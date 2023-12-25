/* Project Scope */
#include "audio.h"
#include "brightnessSensor.h"
#include "display/display.h"
#include "modes.h"
#include "pinout.h"
#include "serialCommands.h"
#include "timekeeping.h"
#include "utility.h"

/* Libraries */
#include "FMTWrapper.h"
#include <Button2.h>
#include <LittleFS.h>
#include <SPI.h>

/* C++ Standard Library */
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace printing;

// LED Panel Configuration
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
PixelDisplay display(matrixWidth, matrixHeight, false, false);
canvas::Canvas baseCanvas(matrixWidth, matrixHeight);

// Buttons
Button2 buttonMode(pins::button1, INPUT_PULLUP);
Button2 buttonSelect(pins::button2, INPUT_PULLUP);
Button2 buttonLeft(pins::button3, INPUT_PULLUP);
Button2 buttonRight(pins::button4, INPUT_PULLUP);
Button2 buttonBrightness(pins::button5, INPUT_PULLUP);

// Modes
std::unique_ptr<ModeManager> modeManager;

//// Brightness Handling
std::unique_ptr<BrightnessSensor> brightnessSensor;
struct BrightnessMode {
    std::string name;
    std::function<uint8_t()> function;
};

uint8_t brightnessFromSensor() {
    return uint8_t(constrain(map(brightnessSensor->getBrightness() * 1000, 0, 1700, 0, 255), 1, 255));
}

std::vector<BrightnessMode> brightnessModes = {
    {"High", []() { return 255; }},
    {"Med", []() { return 127; }},
    {"Low", []() { return 10; }},
    {"Auto", brightnessFromSensor},
};
uint8_t brightnessModeIndex = 0;

void brightnessButton_callback(Button2& btn) {
    print(Serial, "Brightness button callback...\n");
    print(Serial, "Switching to next brightness...\n");
    print(Serial, fmt::format("Current Brightness Index: {}\n", brightnessModeIndex));
    print(Serial, fmt::format("Current Brightness Name: {}\n", brightnessModes[brightnessModeIndex].name));

    brightnessModeIndex++;
    if (brightnessModeIndex == brightnessModes.size()) { brightnessModeIndex = 0; }

    print(Serial, fmt::format("New Brightness Index: {}\n", brightnessModeIndex));
    print(Serial, fmt::format("New Brightness Name: {}\n", brightnessModes[brightnessModeIndex].name));
}

// Main loop timing
constexpr uint32_t loopTargetTime = 15;    // Constant loop update rate to target (milliseconds)
constexpr uint32_t reportInterval = 10000; // Statistics on loop timing will be reported this often (milliseconds)
LoopTimeManager loopTimeManager(loopTargetTime, reportInterval);

void setup() {
    delay(100);
    Serial.begin(921600);

    print(Serial, "\n\n");
    printSolidLine(Serial, headingWidth);
    printCentred(Serial, "Pixel Clock Firmware Start", headingWidth);
    printSolidLine(Serial, headingWidth);

    // system
    printCentred(Serial, "System Information", headingWidth);
    print(Serial, fmt::format("{1:<{0}} {2}\n", textPadding, "ESP Chip Model:", ESP.getChipModel()));
    print(Serial, fmt::format("{1:<{0}} {2}\n", textPadding, "ESP Chip Rev:", ESP.getChipCores()));
    print(Serial, fmt::format("{1:<{0}} {2} MHz\n", textPadding, "ESP CPU Freq:", ESP.getCpuFreqMHz()));
    print(
        Serial, fmt::format("{1:<{0}} {2}\n", textPadding, "ESP Flash Mode:", fmt::underlying(ESP.getFlashChipMode())));
    print(Serial, fmt::format("{1:<{0}} {2} kB\n", textPadding, "ESP Flash Size:", ESP.getFlashChipSize() / 1024));
    print(Serial, fmt::format("{1:<{0}} {2}\n", textPadding, "ESP Flash Speed:", ESP.getFlashChipSpeed()));
    print(Serial, fmt::format("{1:<{0}} {2}\n", textPadding, "ESP SDK Version:", ESP.getSdkVersion()));
    bool psramEnabled = psramInit();
    print(Serial, fmt::format("{1:<{0}} {2}\n", textPadding, "ESP PSRAM Enabled:", psramEnabled));

    // firmware
    printCentred(Serial, "Firmware Information", headingWidth);
    print(Serial, fmt::format("{1:<{0}} {2}\n", textPadding, "Firmware MD5:", ESP.getSketchMD5()));
    print(Serial, fmt::format("{1:<{0}} {2} kB\n", textPadding, "Firmware Size:", ESP.getSketchSize() / 1024));
    print(Serial, fmt::format("{1:<{0}} {2}\n", textPadding, "Available Space:", ESP.getFreeSketchSpace() / 1024));

    // I2C
    printCentred(Serial, "Initialising I2C", headingWidth);
    bool i2cInitialised = Wire.begin();
    print(
        Serial,
        fmt::format("{1:<{0}} {2}\n", textPadding, "I2C Initialisation:", i2cInitialised ? "success" : "failed"));
    if (!i2cInitialised) {
        while (true) {}
    }
    const auto devices = utility::scanI2CDevices(Wire);
    for (const auto& d : devices) {
        print(Serial, fmt::format("{1:<{0}} {2:#X}\n", textPadding, "Found I2C Device:", d));
    }

    // time
    printCentred(Serial, "Initialising Time", headingWidth);
    initialiseTime();

    // filesystem
    printCentred(Serial, "Initialising Filesystem", headingWidth);
    bool lfsInitialised = LittleFS.begin();
    print(
        Serial,
        fmt::format("{1:<{0}} {2}\n", textPadding, "LFS Initialisation:", lfsInitialised ? "success" : "failed"));
    if (!lfsInitialised) {
        while (true) {}
    }
    print(Serial, fmt::format("{1:<{0}} {2} kB\n", textPadding, "LFS Total Bytes:", LittleFS.totalBytes() / 1024));
    print(Serial, fmt::format("{1:<{0}} {2} kB\n", textPadding, "LFS Used Bytes:", LittleFS.usedBytes() / 1024));
    // print all files in FS here?

    printCentred(Serial, "Initialising Light Sensor", headingWidth);
    brightnessSensor = std::make_unique<BrightnessSensor>();

    printCentred(Serial, "Initialising System Modes", headingWidth);
    baseCanvas.fill(pixel::CRGB::Black);
    modeManager =
        std::make_unique<ModeManager>(baseCanvas, ButtonReferences{buttonMode, buttonSelect, buttonLeft, buttonRight});
    printCentred(Serial, "Initialising Display", headingWidth);
    display.update(baseCanvas);
    delay(100);
    // displayDiagnostic(display);

    printCentred(Serial, "Initialising Input", headingWidth);
    buttonBrightness.setTapHandler(brightnessButton_callback);

    printCentred(Serial, "Initialising Audio", headingWidth);
    Audio::get().begin();

    print(Serial, fmt::format("{1:<{0}} {2} ms\n", textPadding, "Runtime:", millis()));
    printSolidLine(Serial, headingWidth);
    printCentred(Serial, "Initialisation Completed", headingWidth);
    printSolidLine(Serial, headingWidth);
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
    display.setBrightness(brightnessModes[brightnessModeIndex].function());
    display.update(out);

    Audio::get().update();

    brightnessSensor->update();

    processSerialCommands();

    loopTimeManager.idle();
}
