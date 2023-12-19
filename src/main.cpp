/* Project Scope */
#include "audio.h"
#include "brightnessSensor.h"
#include "display/display.h"
#include "display/displayEffects.h"
#include "display/fastled_rgbw.h"
#include "display/gameOfLife.h"
#include "modes.h"
#include "pinout.h"
#include "serialCommands.h"
#include "timekeeping.h"
#include "utility.h"

/* Libraries */
#include <Button2.h>
#include <LittleFS.h>
#include <SPI.h>

/* C++ Standard Library */
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>

// LED Panel Configuration
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;
constexpr uint16_t dummyLEDCount = getRGBWsize(matrixSize);
CRGB ledsDummyRGBW[dummyLEDCount];
PixelDisplay display(matrixWidth, matrixHeight, false, false);

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
    String name;
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
    Serial.println("Brightness button callback...");

    Serial.println("Switching to next brightness...");
    Serial.print("Current Brightness Index: ");
    Serial.println(brightnessModeIndex);
    Serial.print("Current Brightness Name: ");
    Serial.println(brightnessModes[brightnessModeIndex].name);

    brightnessModeIndex++;
    if (brightnessModeIndex == brightnessModes.size()) { brightnessModeIndex = 0; }

    Serial.print("New Brightness Index: ");
    Serial.println(brightnessModeIndex);
    Serial.print("New Brightness Name: ");
    Serial.println(brightnessModes[brightnessModeIndex].name);
}

// Main loop timing
constexpr uint32_t loopTargetTime = 15;    // Constant loop update rate to target (milliseconds)
constexpr uint32_t reportInterval = 10000; // Statistics on loop timing will be reported this often (milliseconds)
LoopTimeManager loopTimeManager(loopTargetTime, reportInterval);

void setup() {
    delay(100);
    Serial.begin(921600);

    using namespace utility::printFormatting;

    printSolidLine(headingWidth);
    printTextCentred("Pixel Clock Firmware Start", headingWidth);
    printSolidLine(headingWidth);

    // system
    printTextCentred("System Information", headingWidth);
    Serial.printf("%-*s %s\n", textPadding, "ESP Chip Model:", ESP.getChipModel());
    Serial.printf("%-*s %d\n", textPadding, "ESP Chip Rev:", ESP.getChipRevision());
    Serial.printf("%-*s %d\n", textPadding, "ESP Chip Cores:", ESP.getChipCores());
    Serial.printf("%-*s %d\n", textPadding, "ESP CPU Freq:", ESP.getCpuFreqMHz());
    Serial.printf("%-*s %d\n", textPadding, "ESP Flash Mode:", ESP.getFlashChipMode());
    Serial.printf("%-*s %dKB\n", textPadding, "ESP Flash Size:", ESP.getFlashChipSize() / 1024);
    Serial.printf("%-*s %d\n", textPadding, "ESP Flash Speed:", ESP.getFlashChipSpeed());
    Serial.printf("%-*s %s\n", textPadding, "ESP SDK Version:", ESP.getSdkVersion());

    // firmware
    printTextCentred("Firmware Information", headingWidth);
    Serial.printf("%-*s %s\n", textPadding, "Firmware MD5:", ESP.getSketchMD5().c_str());
    Serial.printf("%-*s %dKB\n", textPadding, "Size:", ESP.getSketchSize() / 1024);
    Serial.printf("%-*s %dKB\n", textPadding, "Available space:", ESP.getFreeSketchSpace() / 1024);

    // I2C
    printTextCentred("Initialising I2C", headingWidth);
    bool i2cInitialised = Wire.begin();
    Serial.printf("%-*s %s\n", textPadding, "I2C Initialisation:", i2cInitialised ? "success" : "failed");
    if (!i2cInitialised) {
        while (true) {}
    }
    utility::listAllI2CDevices(Wire);

    // time
    printTextCentred("Initialising Time", headingWidth);
    initialiseTime();

    // filesystem
    printTextCentred("Initialising Filesystem", headingWidth);
    bool lfsInitialised = LittleFS.begin();
    Serial.printf("%-*s %s\n", textPadding, "LFS Initialisation:", lfsInitialised ? "success" : "failed");
    if (!lfsInitialised) {
        while (true) {}
    }
    Serial.printf("%-*s %dKB\n", textPadding, "LFS Total Bytes:", LittleFS.totalBytes() / 1024);
    Serial.printf("%-*s %dKB\n", textPadding, "LFS Used Bytes:", LittleFS.usedBytes() / 1024);
    // print all files in FS here?

    printTextCentred("Initialising Light Sensor", headingWidth);
    brightnessSensor = std::make_unique<BrightnessSensor>();

    printTextCentred("Initialising System Modes", headingWidth);
    modeManager =
        std::make_unique<ModeManager>(display, ButtonReferences{buttonMode, buttonSelect, buttonLeft, buttonRight});

    printTextCentred("Initialising Display", headingWidth);
    FastLED.addLeds<WS2812, pins::matrixLED, RGB>(ledsDummyRGBW, dummyLEDCount);
    display.setLEDStrip(ledsDummyRGBW);
    display.fill(0);
    display.update();
    delay(100);
    // displayDiagnostic(display);

    printTextCentred("Initialising Input", headingWidth);
    buttonBrightness.setTapHandler(brightnessButton_callback);

    printTextCentred("Initialising Audio", headingWidth);
    Audio::get().begin();

    Serial.printf("%-*s %lums\n", textPadding, "Runtime:", millis());
    printSolidLine(headingWidth);
    printTextCentred("Initialisation Completed", headingWidth);
    printSolidLine(headingWidth);
}

void loop() {
    // update buttons
    buttonMode.loop();
    buttonBrightness.loop();
    buttonSelect.loop();
    buttonLeft.loop();
    buttonRight.loop();

    modeManager->run();

    FastLED.setBrightness(brightnessModes[brightnessModeIndex].function());
    FastLED.setDither(1);

    display.update();

    Audio::get().update();

    brightnessSensor->update();

    processSerialCommands();

    loopTimeManager.idle();
}
