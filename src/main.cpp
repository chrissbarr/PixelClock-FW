// C++ Std Library
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <random>

// Libraries
#include <SPI.h>
#include <Button2.h>
#include <LittleFS.h>

// Project Scope
#include "pinout.h"
#include "display/display.h"
#include "display/displayEffects.h"
#include "display/gameOfLife.h"
#include "display/fastled_rgbw.h"
#include "timekeeping.h"
#include "brightnessSensor.h"
#include "modes.h"
#include "utility.h"
#include "audio.h"

// LED Panel Configuration
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;
constexpr uint16_t dummyLEDCount = getRGBWsize(matrixSize);
CRGB ledsDummyRGBW[dummyLEDCount];
PixelDisplay display(matrixWidth, matrixHeight, false, false);

// Buttons
Button2 buttonMode(buttonPin1, INPUT_PULLUP);
Button2 buttonSelect(buttonPin2, INPUT_PULLUP);
Button2 buttonLeft(buttonPin3, INPUT_PULLUP);
Button2 buttonRight(buttonPin4, INPUT_PULLUP);
Button2 buttonBrightness(buttonPin5, INPUT_PULLUP);

// Modes
std::unique_ptr<ModeManager> modeManager;

//// Brightness Handling
std::unique_ptr<BrightnessSensor> brightnessSensor;
struct BrightnessMode {
  String name;
  std::function<uint8_t()> function;
};

uint8_t brightnessFromSensor()
{
  float maxBrightness = 1.7;
  return uint8_t(constrain(map(brightnessSensor->getBrightness() * 1000, 0, 1700, 0, 255), 1, 255));
}

std::vector<BrightnessMode> brightnessModes = {
  {"High", [](){ return 255; }},
  {"Med", [](){ return 127; }},
  {"Low", [](){ return 10; }},
  {"Auto", brightnessFromSensor}
};
uint8_t brightnessModeIndex = 0;

void brightnessButton_callback(Button2& btn) 
{
  Serial.println("Brightness button callback...");

  Serial.println("Switching to next brightness...");
  Serial.print("Current Brightness Index: "); Serial.println(brightnessModeIndex);
  Serial.print("Current Brightness Name: "); Serial.println(brightnessModes[brightnessModeIndex].name);

  brightnessModeIndex++;
  if (brightnessModeIndex == brightnessModes.size()) {
    brightnessModeIndex = 0;
  }

  Serial.print("New Brightness Index: "); Serial.println(brightnessModeIndex);
  Serial.print("New Brightness Name: "); Serial.println(brightnessModes[brightnessModeIndex].name);
}

// Main loop timing
constexpr uint32_t loopTargetTime = 15;     // Constant loop update rate to target (milliseconds)
constexpr uint32_t reportInterval = 10000;  // Statistics on loop timing will be reported this often (milliseconds)
LoopTimeManager loopTimeManager(loopTargetTime, reportInterval);

// Audio
std::unique_ptr<Audio> audio;


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
  if (!i2cInitialised) {  while (true) {}; }
  utility::listAllI2CDevices(Wire);

  // time
  printTextCentred("Initialising Time", headingWidth);
  initialiseTime();

  // filesystem
  printTextCentred("Initialising Filesystem", headingWidth);
  bool lfsInitialised = LittleFS.begin();
  Serial.printf("%-*s %s\n", textPadding, "LFS Initialisation:", lfsInitialised ? "success" : "failed");
  if (!lfsInitialised) {  while (true) {}; }
  Serial.printf("%-*s %dKB\n", textPadding, "LFS Total Bytes:", LittleFS.totalBytes() / 1024);
  Serial.printf("%-*s %dKB\n", textPadding, "LFS Used Bytes:", LittleFS.usedBytes() / 1024);
  // print all files in FS here?

  printTextCentred("Initialising Light Sensor", headingWidth);
  brightnessSensor = std::make_unique<BrightnessSensor>();

  printTextCentred("Initialising System Modes", headingWidth);
  modeManager = std::make_unique<ModeManager>(display, ButtonReferences{buttonMode, buttonSelect, buttonLeft, buttonRight});

  printTextCentred("Initialising Display", headingWidth);
  FastLED.addLeds<WS2812, matrixLEDPin, RGB>(ledsDummyRGBW, dummyLEDCount);
  display.setLEDStrip(ledsDummyRGBW);
  display.fill(0);
  display.update();
  delay(100); 
  //displayDiagnostic(display);

  printTextCentred("Initialising Input", headingWidth);
  buttonBrightness.setTapHandler(brightnessButton_callback);

  printTextCentred("Initialising Audio", headingWidth);

  audio = std::make_unique<Audio>();
  audio->begin();


  Serial.printf("%-*s %dms\n", textPadding, "Runtime:", millis());
  printSolidLine(headingWidth);
  printTextCentred("Initialisation Completed", headingWidth);
  printSolidLine(headingWidth);

  audioSpectrumSemaphore = xSemaphoreCreateMutex();

  initialiseFFT();

}

void loop()
{
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

  brightnessSensor->update();

  loopTimeManager.idle();

  if (Serial.available()) {
    String receivedCommand = "";
    receivedCommand = Serial.readString();

    std::vector<String> substrings;

    // Split the string into substrings
    while (receivedCommand.length() > 0)
    {
      int index = receivedCommand.indexOf(' ');
      if (index == -1) // No space found
      {
        substrings.push_back(receivedCommand);
        break;
      }
      else
      {
        substrings.push_back(receivedCommand.substring(0, index));
        receivedCommand = receivedCommand.substring(index+1);
      }
    }

    if (!substrings.empty()) {
      Serial.print("Received command: ");
      for (const auto& str : substrings) {
        Serial.printf("%s ", str);
      }
      Serial.printf("\n");

      if (substrings[0] == "T") {
        // YYYY MM DD HH MM SS
        if (substrings.size() == 7) {
          int year = substrings[1].toInt();
          int month = substrings[2].toInt();
          int day = substrings[3].toInt();
          int hour = substrings[4].toInt();
          int min = substrings[5].toInt();
          int sec = substrings[6].toInt();

          TimeElements time;
          time.Year = uint8_t(CalendarYrToTm(year));
          time.Month = uint8_t(month);
          time.Day = uint8_t(day);
          time.Hour = uint8_t(hour);
          time.Minute = uint8_t(min);
          time.Second = uint8_t(sec);
          setTimeGlobally(makeTime(time));
        }
      }
    }
  }

}


