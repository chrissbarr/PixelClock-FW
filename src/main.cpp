// C++ Std Library
#include <memory>
#include <vector>
#include <string>

// Libraries
#include <SPI.h>
#include <FastLED.h>

#include <Button2.h>
#include <TSL2591I2C.h>

#include <melody_player.h>
#include <melody_factory.h>

// Project Scope
#include "display/display.h"
#include "display/displayEffects.h"
#include "display/gameOfLife.h"
#include "display/fastled_rgbw.h"
#include "timekeeping.h"

// Pinout
constexpr int16_t matrixLEDPin = 4;
constexpr int16_t buttonPin1 = 27;
constexpr int16_t buttonPin2 = 33;
constexpr int16_t buttonPin3 = 15;
constexpr int16_t buttonPin4 = 32;
constexpr int16_t buttonPin5 = 14;
constexpr int16_t buzzerPin = 26;

// LED Panel Configuration
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;
constexpr uint16_t dummyLEDCount = getRGBWsize(matrixSize);
CRGB ledsDummyRGBW[dummyLEDCount];
PixelDisplay display(matrixWidth, matrixHeight, false, false);

MelodyPlayer player(buzzerPin, HIGH);


// Buttons

// Button2 buttons[] = {
//   Button2(buttonPin1),
//   Button2(buttonPin2),
//   Button2(buttonPin3),
//   Button2(buttonPin4),
//   Button2(buttonPin5)
// };

// void click(Button2& btn) {
//     if (btn == buttons[0]) {
//       Serial.println("A clicked");
//     } 
//     if (btn == buttons[1]) {
//       Serial.println("B clicked");
//     }
// }

TSL2591I2C tsl2591;
bool lightSensorActive = false;
uint32_t lightSensorLastPollTime = 0;
uint32_t lightSensorPollInterval = 200;

// Main loop timing
uint32_t lastLoopTime = 0;
constexpr uint32_t loopTargetTime = 15;
uint32_t lastReportTime = 0;
constexpr uint32_t reportInterval = 5000;

float avgTime = 0;
uint16_t minTime = std::numeric_limits<uint16_t>::max();
uint16_t maxTime = 0;

constexpr float approxRollingAverage(float avg, float newSample, int N) 
{
  avg -= avg / N;
  avg += newSample / N;
  return avg;
}

enum class Mode {
  DisplayTime,
  SetTime,
  Demo
};
Mode currentMode = Mode::Demo;

std::vector<std::shared_ptr<DisplayEffect>> displayEffects;
std::size_t effectIndex = 0;

struct FilterConfig {
  std::unique_ptr<FilterMethod> filter;
  String description;
};
std::vector<FilterConfig> filterConfigs;
std::size_t filterIndex = 0;
uint32_t lastFilterChangeTime = 0;
uint32_t filterChangePeriod = 3000;

bool initialiseLightSensor()
{
  Wire.begin();
  Serial.print("Initialising TSL2591: ");
  if (!tsl2591.begin())	{
    Serial.println("Error!");
    return false;
  } else {
    Serial.println("Success!");
    Serial.print("Sensor ID: "); Serial.println(tsl2591.getID(), HEX);
    tsl2591.resetToDefaults();
    tsl2591.setChannel(TSL2591MI::TSL2591_CHANNEL_0);
    tsl2591.setGain(TSL2591MI::TSL2591_GAIN_MED);
    tsl2591.setIntegrationTime(TSL2591MI::TSL2591_INTEGRATION_TIME_100ms);

    Serial.println("Performing test measurement...");

    if (!tsl2591.measure()) {
      Serial.println("Could not start measurement. ");
      return false;
    }

    while (!tsl2591.hasValue()) {
      delay(1);
    }

    Serial.print("Irradiance: "); Serial.print(tsl2591.getIrradiance(), 7); Serial.println(" W / m^2");
    Serial.print("Brightness: "); Serial.print(tsl2591.getBrightness(), 7); Serial.println(" lux");
    return true;
  }
}

std::unique_ptr<GameOfLife> golTrainer;
std::shared_ptr<GameOfLife> golActual;

void setup() {
  delay(1000);
  Serial.begin(250000);
  Serial.println("Serial begin!");

  initialiseTime();
  delay(1000);

  // pinMode(buttonPin1, INPUT_PULLUP);
  // pinMode(buttonPin2, INPUT_PULLUP);
  // pinMode(buttonPin3, INPUT_PULLUP);
  // pinMode(buttonPin4, INPUT_PULLUP);
  // pinMode(buttonPin5, INPUT_PULLUP);
  // while (true) {
  //   Serial.print(digitalRead(buttonPin1));
  //   Serial.print(digitalRead(buttonPin2));
  //   Serial.print(digitalRead(buttonPin3));
  //   Serial.print(digitalRead(buttonPin4));
  //   Serial.print(digitalRead(buttonPin5));
  //   Serial.println();
  //   yield();
  //   delay(100);
  // }

  lightSensorActive = initialiseLightSensor();

  FastLED.addLeds<WS2812, matrixLEDPin, RGB>(ledsDummyRGBW, dummyLEDCount);
  display.setLEDStrip(ledsDummyRGBW);

  Serial.println("Loading melody...");
  const char melodyString[] = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
  // create a melody
  Melody melody = MelodyFactory.loadRtttlString(melodyString);

  // get basic info about the melody
  Serial.println(String(" Title:") + melody.getTitle());
  Serial.println(String(" Time unit:") + melody.getTimeUnit());

  Serial.print("Start playing in blocking mode... ");
  player.play(melody);
  Serial.println("Melody ends!");



  // for (Button2 button : buttons) {
  //   //button.setClickHandler(click);
  // }


  display.fill(0);
  display.update();
  delay(100); 

  //displayDiagnostic(display);

  Serial.println("Pregenerating GoL seeds...");
  uint32_t startTime = millis();

  // construct an instance of GoL that will be used to seed interesting states
  golTrainer = std::make_unique<GameOfLife>(display, 0, 0, colourGenerator_white, display.getFullDisplayRegion(), false);
  golTrainer->setFadeOnDeath(false);
  golTrainer->setSeedingMode(true);

  // run until we have a few initial states
  while (golTrainer->getSeededCount() < 3) {
    while (!golTrainer->finished()) {
      golTrainer->run();
      // catch any infinite-running seeds
      if (golTrainer->getLifespan() > 500) { break; }
    }
    golTrainer->reset();
  }
  Serial.println("GoL scores: ");
  for (const auto& score : golTrainer->getScores()) {
    Serial.print(score.lifespan); Serial.print("\t"); Serial.println(score.seed);
  }
  uint32_t stopTime = millis();
  Serial.print("Seeding duration: "); Serial.println(stopTime - startTime);

  golActual = std::make_shared<GameOfLife>(display, 250, 50, colourGenerator_cycleHSV, display.getFullDisplayRegion(), false);
  golActual->setScores(golTrainer->getScores());


  //displayEffects.push_back(golActual);
  //displayEffects.push_back(std::make_unique<EffectDecorator_Timeout>(std::make_shared<BouncingBall>(display, 250, colourGenerator_cycleHSV), 10000));
  displayEffects.push_back(std::make_unique<EffectDecorator_Timeout>(std::make_shared<ClockFace>(display, timeCallbackFunction), 1000));

  //displayEffects.push_back(std::make_unique<BouncingBall>(display, 250, colourGenerator_cycleHSV));
  //displayEffects.push_back(std::make_unique<TextScroller>(display, "Test Text Scroller", Adafruit_NeoPixel::Color(255, 0, 0), 1000, true, 1));
  // //displayEffects.push_back(std::make_unique<TextScroller>(display, "Another Test! 1234:5678", Adafruit_NeoPixel::Color(0, 0, 255), 1000, true, 1));
  //displayEffects.push_back(std::make_unique<RandomFill>(display, 100, colourGenerator_randomHSV));
  displayEffects.front()->reset();

  //filterConfigs.push_back({std::make_unique<SolidColour>(CRGB::Red, true), "SolidColour(CRGB::Red, true)"});
  //filterConfigs.push_back({std::make_unique<SolidColour>(CRGB::Red, false), "SolidColour(CRGB::Red, false)"});
  //filterConfigs.push_back({std::make_unique<SolidColour>(CRGB::Cyan, true), "SolidColour(CRGB::Cyan, true)"});
  //filterConfigs.push_back({std::make_unique<SolidColour>(CRGB::Cyan, false), "SolidColour(CRGB::Cyan, false)"});
  filterConfigs.push_back({std::make_unique<RainbowWave>(0.2, 10, RainbowWave::Direction::horizontal, true), "RainbowWave(0.2, 10, horizontal, true)"});
  //filterConfigs.push_back({std::make_unique<RainbowWave>(0.2, 10, RainbowWave::Direction::horizontal, false), "RainbowWave(0.2, 10, horizontal, false)"});
  filterConfigs.push_back({std::make_unique<RainbowWave>(0.2, 10, RainbowWave::Direction::vertical, true), "RainbowWave(0.2, 10, vertical, true)"});
  //filterConfigs.push_back({std::make_unique<RainbowWave>(0.2, 10, RainbowWave::Direction::vertical, false), "RainbowWave(0.2, 10, vertical, false)"});




  // start time tracking for main loop
  lastLoopTime = millis();
}

TextScroller settingsMenuTextScroller_settime(display, "Set Time", 1000, true, 1);

void loop()
{

  // update buttons
  // for (Button2 button : buttons) {
  //   button.loop();
  // }

  // time_t currentTime = now();
  // switch (currentMode) {
  //   case Mode::DisplayTime:
  //   {
  //     display.fill(0);
  //     showTime(display, hourFormat12(currentTime), minute(currentTime), colourGenerator_cycleHSV());
  //     break;
  //   }
  //   case Mode::SetTime:
  //   {
  //     display.fill(0);
  //     if (second(currentTime) % 1) {
  //       showTime(display, hourFormat12(currentTime), minute(currentTime), colourGenerator_cycleHSV());
  //     }
  //     break;
  //   }
  //   case Mode::Demo:
  //   {
  //     display.fill(0);
  //     settingsMenuTextScroller_settime.run();
  //     //tetris(display, 100, 100);
  //     //if (display.filled()) { display.fill(0); }
  //   }
  // }

  display.fill(0);
  if (displayEffects[effectIndex]->finished()) {
    Serial.println("Effect finished!");
    effectIndex++;
    if (effectIndex >= displayEffects.size()) {
      effectIndex = 0;
    }
    displayEffects[effectIndex]->reset();
  }
  displayEffects[effectIndex]->run();

  // update display
  
  //display.applyFilter(HSVTestPattern());
  //showTime(display, hourFormat12(), minute(), CRGB::Red);


  display.applyFilter(*(filterConfigs[filterIndex].filter));
  if (millis() - lastFilterChangeTime > filterChangePeriod) {
    filterIndex++;
    if (filterIndex >= filterConfigs.size()) {
      filterIndex = 0;
    }
    Serial.print("Filter: "); Serial.println(filterConfigs[filterIndex].description);
    lastFilterChangeTime = millis();
  }

  //display.applyFilter(SolidColour(CRGB::Purple));
  FastLED.setDither(1);
  //display.applyFilter(SolidColour(CRGB(1, 0, 0), false));
  //FastLED.setBrightness(100);
  display.update();
  //delay(1);
  //display.fill(0);
  //display.update();
  //delay(20);

  if (lightSensorActive && millis() - lightSensorLastPollTime > lightSensorPollInterval) {
    if (tsl2591.hasValue()) {
      float brightness = tsl2591.getBrightness();
      float irradiance = tsl2591.getIrradiance();
      //Serial.print("Irradiance: "); Serial.print(irradiance, 7); Serial.println(" W / m^2");
      //Serial.print("Brightness: "); Serial.print(brightness, 7); Serial.println(" lux");
      float maxBrightness = 1.7;
      uint8_t newBrightness = uint8_t(constrain(map(brightness * 1000, 0, 1700, 0, 255), 1, 255));
      //Serial.print("Brightness set to: "); Serial.println(newBrightness);
      FastLED.setBrightness(newBrightness);
      //tsl2591.measure();
    }
    lightSensorLastPollTime = millis();
  }

  // Manage loop timing
  unsigned long loopTime = millis() - lastLoopTime;

  avgTime = approxRollingAverage(avgTime, float(loopTime), 1000);
  if (loopTime > maxTime) { maxTime = loopTime; }
  if (loopTime < minTime) { minTime = loopTime; }

  if (millis() - lastReportTime > reportInterval) {
    Serial.println("Loop Timing Statistics");
    Serial.print("Avg time:" ); Serial.println(avgTime);
    Serial.print("Min time:" ); Serial.println(minTime);
    Serial.print("Max time:" ); Serial.println(maxTime);
    Serial.print("FastLED FPS:" ); Serial.println(FastLED.getFPS());
    lastReportTime = millis();
    minTime = std::numeric_limits<uint16_t>::max();
    maxTime = 0;
  }

  yield();
  while (millis() - lastLoopTime < loopTargetTime) {
    yield();
    //FastLED.show();

    //Serial.println("Running trainer...");
    golTrainer->run();
    if (golTrainer->finished()) {
      //Serial.println("Resetting trainer...");
      golTrainer->reset();

      if (golTrainer->getIterations() % 1000 == 0) {
        golActual->setScores(golTrainer->getScores());
        Serial.println("GoL scores: ");
        for (const auto& score : golActual->getScores()) {
          Serial.print(score.lifespan); Serial.print("\t"); Serial.println(score.seed);
        }
      }
    }


    
  }
  lastLoopTime = millis();
}


