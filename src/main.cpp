// C++ Std Library
#include <memory>
#include <vector>
#include <string>
#include <functional>

// Libraries
#include <SPI.h>
#include <Button2.h>
#include <melody_player.h>
#include <melody_factory.h>

// Project Scope
#include "pinout.h"
#include "display/display.h"
#include "display/displayEffects.h"
#include "display/gameOfLife.h"
#include "display/fastled_rgbw.h"
#include "timekeeping.h"
#include "brightnessSensor.h"
#include "modes.h"

// LED Panel Configuration
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;
constexpr uint16_t dummyLEDCount = getRGBWsize(matrixSize);
CRGB ledsDummyRGBW[dummyLEDCount];
PixelDisplay display(matrixWidth, matrixHeight, false, false);

MelodyPlayer player(buzzerPin, HIGH);

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

std::vector<String> melodies = {
  "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6",
  "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a",
};
//std::unique_ptr<Melody> melodyPlayer;

void setup() {
  delay(1000);
  Serial.begin(250000);
  Serial.println("Serial begin!");
  Wire.begin();
  initialiseTime();

  brightnessSensor = std::make_unique<BrightnessSensor>();
  modeManager = std::make_unique<ModeManager>(display, ButtonReferences{buttonMode, buttonSelect, buttonLeft, buttonRight});

  FastLED.addLeds<WS2812, matrixLEDPin, RGB>(ledsDummyRGBW, dummyLEDCount);
  display.setLEDStrip(ledsDummyRGBW);

  buttonBrightness.setTapHandler(brightnessButton_callback);

  display.fill(0);
  display.update();
  delay(100); 

  //displayDiagnostic(display);
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

  // if (!player.isPlaying()) {
  //   int melodyIdx = random(0, melodies.size());
  //   auto melody = MelodyFactory.loadRtttlString(melodies[melodyIdx].c_str());
  //   player.playAsync(melody);
  //   Serial.println(String(" Title:") + melody.getTitle());
  //   Serial.println(String(" Time unit:") + melody.getTimeUnit());
  // }

  loopTimeManager.idle();
}


