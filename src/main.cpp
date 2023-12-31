/* Project Scope */
#include "audio/audio.h"
#include "brightnessSensor.h"
#include "loopTimeManager.h"
#include "modes.h"
#include "pinout.h"
#include "timekeeping.h"
#include "utility.h"
#ifdef PIXELCLOCK_DESKTOP
#include "display/dummydisplay.h"
#include "utility/sfml.h"
#include <SFML/Graphics.hpp>
#else
#include "display/pixeldisplay.h"
#include "serialCommands.h"
#endif

/* Libraries */
#include "FMTWrapper.h"
#include <Button2.h>
#ifndef PIXELCLOCK_DESKTOP
#include <LittleFS.h>
#include <SPI.h>
#endif

/* C++ Standard Library */
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

using namespace printing;

// LED Panel Configuration
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
std::unique_ptr<Display> display;
canvas::Canvas baseCanvas(matrixWidth, matrixHeight);

// Buttons
#ifdef PIXELCLOCK_DESKTOP
Button2 buttonMode;
Button2 buttonSelect;
Button2 buttonLeft;
Button2 buttonRight;
Button2 buttonBrightness;
#else
Button2 buttonMode(pins::button1, INPUT_PULLUP);
Button2 buttonSelect(pins::button2, INPUT_PULLUP);
Button2 buttonLeft(pins::button3, INPUT_PULLUP);
Button2 buttonRight(pins::button4, INPUT_PULLUP);
Button2 buttonBrightness(pins::button5, INPUT_PULLUP);
#endif

// Modes
std::unique_ptr<ModeManager> modeManager;

//// Brightness Handling
std::unique_ptr<BrightnessSensor> brightnessSensor;
uint8_t brightnessFromSensor() {
    return uint8_t(std::clamp(
        uint8_t(utility::mapNumericRange(brightnessSensor->getBrightness() * 1000, 0, 1700, 0, 255)),
        uint8_t(1),
        uint8_t(255)));
}

struct BrightnessMode {
    std::string name;
    std::function<uint8_t()> function;
};
std::vector<BrightnessMode> brightnessModes = {
    {"High", []() { return 255; }}, //
    {"Med", []() { return 127; }},  //
    {"Low", []() { return 10; }},   //
    {"Auto", brightnessFromSensor}, //
};
uint8_t brightnessModeIndex = 0;

void brightnessButton_callback(Button2& btn) {
    print("Brightness button callback...\n");
    print("Switching to next brightness...\n");
    print(fmt::format("Current Brightness Index: {}\n", brightnessModeIndex));
    print(fmt::format("Current Brightness Name: {}\n", brightnessModes[brightnessModeIndex].name));

    brightnessModeIndex++;
    if (brightnessModeIndex == brightnessModes.size()) { brightnessModeIndex = 0; }

    print(fmt::format("New Brightness Index: {}\n", brightnessModeIndex));
    print(fmt::format("New Brightness Name: {}\n", brightnessModes[brightnessModeIndex].name));
}

// Main loop timing
constexpr uint32_t loopTargetTime = 15;    // Constant loop update rate to target (milliseconds)
constexpr uint32_t reportInterval = 10000; // Statistics on loop timing will be reported this often (milliseconds)
LoopTimeManager loopTimeManager(loopTargetTime, reportInterval);

void setup() {
    delay(100);
#ifndef PIXELCLOCK_DESKTOP
    Serial.begin(921600);
#endif

    print("\n\n");
    printSolidLine(headingWidth);
    printCentred("Pixel Clock Firmware Start", headingWidth);
    printSolidLine(headingWidth);

    // system
    printCentred("System Information", headingWidth);
#ifndef PIXELCLOCK_DESKTOP
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "ESP Chip Model:", ESP.getChipModel()));
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "ESP Chip Rev:", ESP.getChipCores()));
    print(fmt::format("{1:<{0}} {2} MHz\n", textPadding, "ESP CPU Freq:", ESP.getCpuFreqMHz()));
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "ESP Flash Mode:", fmt::underlying(ESP.getFlashChipMode())));
    print(fmt::format("{1:<{0}} {2} kB\n", textPadding, "ESP Flash Size:", ESP.getFlashChipSize() / 1024));
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "ESP Flash Speed:", ESP.getFlashChipSpeed()));
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "ESP SDK Version:", ESP.getSdkVersion()));
    bool psramEnabled = psramInit();
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "ESP PSRAM Enabled:", psramEnabled));
#endif

    // firmware
    printCentred("Firmware Information", headingWidth);
#ifndef PIXELCLOCK_DESKTOP
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "Firmware MD5:", ESP.getSketchMD5()));
    print(fmt::format("{1:<{0}} {2} kB\n", textPadding, "Firmware Size:", ESP.getSketchSize() / 1024));
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "Available Space:", ESP.getFreeSketchSpace() / 1024));
#endif

    // I2C
#ifndef PIXELCLOCK_DESKTOP
    printCentred("Initialising I2C", headingWidth);
    bool i2cInitialised = Wire.begin();
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "I2C Initialisation:", i2cInitialised ? "success" : "failed"));
    if (!i2cInitialised) {
        while (true) {}
    }
    const auto devices = utility::scanI2CDevices(Wire);
    for (const auto& d : devices) { print(fmt::format("{1:<{0}} {2:#X}\n", textPadding, "Found I2C Device:", d)); }
#endif

    // time
    printCentred("Initialising Time", headingWidth);
    TimeManagerSingleton::get().initialise();

    // filesystem
#ifndef PIXELCLOCK_DESKTOP
    printCentred("Initialising Filesystem", headingWidth);
    bool lfsInitialised = LittleFS.begin();
    print(fmt::format("{1:<{0}} {2}\n", textPadding, "LFS Initialisation:", lfsInitialised ? "success" : "failed"));
    if (!lfsInitialised) {
        while (true) {}
    }
    print(fmt::format("{1:<{0}} {2} kB\n", textPadding, "LFS Total Bytes:", LittleFS.totalBytes() / 1024));
    print(fmt::format("{1:<{0}} {2} kB\n", textPadding, "LFS Used Bytes:", LittleFS.usedBytes() / 1024));
    // print all files in FS here?
#endif

    printCentred("Initialising Light Sensor", headingWidth);
#ifdef PIXELCLOCK_DESKTOP
    brightnessSensor = std::make_unique<BrightnessSensorDummy>(1.0f);
#else
    brightnessSensor = std::make_unique<BrightnessSensorTSL2591>();
#endif

    printCentred("Initialising System Modes", headingWidth);
    baseCanvas.fill(flm::CRGB::Black);
    modeManager =
        std::make_unique<ModeManager>(baseCanvas, ButtonReferences{buttonMode, buttonSelect, buttonLeft, buttonRight});

    printCentred("Initialising Display", headingWidth);
#ifdef PIXELCLOCK_DESKTOP
    display = std::make_unique<DummyDisplay>(matrixWidth, matrixHeight);
#else
    display = std::make_unique<PixelDisplay>(matrixWidth, matrixHeight, false, false);
#endif
    display->update(baseCanvas);
    delay(100);
    // displayDiagnostic(display);

    printCentred("Initialising Input", headingWidth);
#ifdef PIXELCLOCK_DESKTOP
    buttonMode.begin(VIRTUAL_PIN);
    buttonSelect.begin(VIRTUAL_PIN);
    buttonLeft.begin(VIRTUAL_PIN);
    buttonRight.begin(VIRTUAL_PIN);
    buttonBrightness.begin(VIRTUAL_PIN);
#endif
    buttonBrightness.setTapHandler(brightnessButton_callback);

    printCentred("Initialising Audio", headingWidth);
    AudioSingleton::get().begin();
    loopTimeManager.registerTraceCallback([](){ return AudioSingleton::get().getInstrumentation(); });

    print(fmt::format("{1:<{0}} {2} ms\n", textPadding, "Runtime:", millis()));
    printSolidLine(headingWidth);
    printCentred("Initialisation Completed", headingWidth);
    printSolidLine(headingWidth);
}

void loop() {
    // update buttons
    buttonMode.loop();
    buttonBrightness.loop();
    buttonSelect.loop();
    buttonLeft.loop();
    buttonRight.loop();

    auto c = modeManager->run();
    auto out = canvas::blit(baseCanvas, c, 0, 0);

    display->setBrightness(brightnessModes[brightnessModeIndex].function());
    display->update(out);

    brightnessSensor->update();

    AudioSingleton::get().update();

    // processSerialCommands();

    TimeManagerSingleton::get().update();
    loopTimeManager.idle();
}

#if defined PIXELCLOCK_DESKTOP
int main(int argc, char** argv) {

    print("PixelClock main!\n");
    setup();

    sf::Font font;
    if (!font.loadFromFile("data/Roboto-Regular.ttf")) { print("Font not found!\n"); }

    std::vector<Button> buttons;
    buttons.reserve(10);

    buttons.push_back({"Mode", {200, 100}, 40, sf::Color::Black, sf::Color::White});
    buttons.back().setFont(font);
    buttonMode.setButtonStateFunction([&]() { return buttons[0].isClicked(); });

    buttons.push_back({"Select", {200, 100}, 40, sf::Color::Black, sf::Color::White});
    buttons.back().setFont(font);
    buttonSelect.setButtonStateFunction([&]() { return buttons[1].isClicked(); });

    buttons.push_back({"Left", {200, 100}, 40, sf::Color::Black, sf::Color::White});
    buttons.back().setFont(font);
    buttonLeft.setButtonStateFunction([&]() { return buttons[2].isClicked(); });

    buttons.push_back({"Right", {200, 100}, 40, sf::Color::Black, sf::Color::White});
    buttons.back().setFont(font);
    buttonRight.setButtonStateFunction([&]() { return buttons[3].isClicked(); });

    auto window = sf::RenderWindow{{800u, 400u}, "PixelClock-FW"};
    window.setFramerateLimit(300);

    while (window.isOpen()) {
        for (auto event = sf::Event{}; window.pollEvent(event);) {
            if (event.type == sf::Event::Closed) { window.close(); }

            if (event.type == sf::Event::MouseButtonPressed) {
                for (auto& btn : buttons) {
                    if (btn.isMouseOver(window)) {
                        btn.click();
                        print(fmt::format("Button clicked! ({})\n", btn.getName()));
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                for (auto& btn : buttons) { btn.release(); }
            }
        }

        window.clear();

        loop();

        auto t = canvasToTex2(static_cast<DummyDisplay*>(display.get())->getCanvas());
        sf::Sprite sprite(t);
        sprite.setPosition(sf::Vector2f(0.f, 0.f));

        sf::Vector2f targetSize = window.getView().getSize();
        targetSize.y -= 100;

        sprite.setScale(targetSize.x / sprite.getLocalBounds().width, targetSize.y / sprite.getLocalBounds().height);
        window.draw(sprite);

        int btnCount = buttons.size();
        int windowWidth = window.getView().getSize().x;
        int widthPerButton = windowWidth / btnCount;
        int idx = 0;
        for (auto& btn : buttons) {
            btn.setSize({float(widthPerButton), 100});
            btn.setPosition({float(idx * widthPerButton), 300});
            idx++;
        }

        for (auto& btn : buttons) { btn.drawTo(window); }

        window.display();
    }

    return 0;
}
#endif