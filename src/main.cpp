#include <SPI.h>
#include <FastLED.h>
#include <fastled_rgbw.h>

#include <TimeLib.h>
#include <RTClib.h>
#include <Button2.h>
#include <memory>
#include <vector>
#include <string>


#include "characters.h"
#include "display.h"
#include "displayEffects.h"

// LED Panel Configuration
constexpr int16_t matrixLEDPin = 14;
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;
constexpr uint16_t dummyLEDCount = getRGBWsize(matrixSize);
CRGB ledsDummyRGBW[dummyLEDCount];
PixelDisplay display(matrixWidth, matrixHeight, false, false);

// Buttons
constexpr int16_t buttonPin1 = 13;
constexpr int16_t buttonPin2 = 15;
constexpr int16_t buttonPin3 = 0;
constexpr int16_t buttonPin4 = 16;
constexpr int16_t buttonPin5 = 2;
Button2 buttons[] = {
  Button2(buttonPin1),
  Button2(buttonPin2),
  Button2(buttonPin3),
  Button2(buttonPin4),
  Button2(buttonPin5)
};

void click(Button2& btn) {
    if (btn == buttons[0]) {
      Serial.println("A clicked");
    } 
    if (btn == buttons[1]) {
      Serial.println("B clicked");
    }
}

// Main loop timing
uint32_t lastLoopTime = 0;
constexpr uint32_t loopTargetTime = 5;
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

// Timekeeping
RTC_DS3231 rtc;

enum class Mode {
  DisplayTime,
  SetTime,
  Demo
};
Mode currentMode = Mode::Demo;

std::vector<std::unique_ptr<DisplayEffect>> displayEffects;
std::size_t effectIndex = 0;

struct FilterConfig {
  std::unique_ptr<FilterMethod> filter;
  String description;
};
std::vector<FilterConfig> filterConfigs;
std::size_t filterIndex = 0;
uint32_t lastFilterChangeTime = 0;
uint32_t filterChangePeriod = 3000;

constexpr char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Provide the RTC time to the Time library.
time_t time_provider() {
    return rtc.now().unixtime();
}

bool initialiseRTC()
{
  Serial.print("Initialising RTC: ");

  if (! rtc.begin()) {
    Serial.println("Error!");
    return false;
  } else {
    Serial.println("Success!");

    if (rtc.lostPower()) {
      Serial.println("RTC has lost power and time needs to be set!");
    } else {
      Serial.println("RTC reports it has not lost power.");
    }

    DateTime now = rtc.now();
    Serial.println("RTC has time: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
    return true;
  }
}

void setup() {
  delay(1000);
  Serial.begin(250000);
  Serial.println("Serial begin!");

  FastLED.addLeds<SK6812, matrixLEDPin, RGB>(ledsDummyRGBW, dummyLEDCount);
  display.setLEDStrip(ledsDummyRGBW);

  if (initialiseRTC()) {
    // Set Time to sync from RTC
    setSyncProvider(time_provider);
    setSyncInterval(60);

    if(timeStatus() != timeSet) {
     Serial.println("Unable to sync with the RTC.");
    } else {
     Serial.println("RTC has set the system time"); 
    }
  } else {
    Serial.println("Setting time to placeholder value.");
    setTime(11,55,50,1,1,2022);
  }

  for (Button2 button : buttons) {
    //button.setClickHandler(click);
  }


  display.fill(0);
  display.update();
  delay(100); 

  //displayDiagnostic(display);

  displayEffects.push_back(std::make_unique<GameOfLife>(display, 100, colourGenerator_cycleHSV, display.getFullDisplayRegion(), false));
  //displayEffects.push_back(std::make_unique<BouncingBall>(display, 250, colourGenerator_cycleHSV));
  //displayEffects.push_back(std::make_unique<TextScroller>(display, "Test Text Scroller", Adafruit_NeoPixel::Color(255, 0, 0), 1000, true, 1));
  // //displayEffects.push_back(std::make_unique<TextScroller>(display, "Another Test! 1234:5678", Adafruit_NeoPixel::Color(0, 0, 255), 1000, true, 1));
  //displayEffects.push_back(std::make_unique<RandomFill>(display, 100, colourGenerator_randomHSV));
  displayEffects.front()->reset();

  filterConfigs.push_back({std::make_unique<SolidColour>(CRGB::Red, true), "SolidColour(CRGB::Red, true)"});
  filterConfigs.push_back({std::make_unique<SolidColour>(CRGB::Red, false), "SolidColour(CRGB::Red, false)"});
  filterConfigs.push_back({std::make_unique<SolidColour>(CRGB::Cyan, true), "SolidColour(CRGB::Cyan, true)"});
  filterConfigs.push_back({std::make_unique<SolidColour>(CRGB::Cyan, false), "SolidColour(CRGB::Cyan, false)"});
  filterConfigs.push_back({std::make_unique<RainbowWave>(0.2, 10, RainbowWave::Direction::horizontal, true), "RainbowWave(0.2, 10, horizontal, true)"});
  filterConfigs.push_back({std::make_unique<RainbowWave>(0.2, 10, RainbowWave::Direction::horizontal, false), "RainbowWave(0.2, 10, horizontal, false)"});
  filterConfigs.push_back({std::make_unique<RainbowWave>(0.2, 10, RainbowWave::Direction::vertical, true), "RainbowWave(0.2, 10, vertical, true)"});
  filterConfigs.push_back({std::make_unique<RainbowWave>(0.2, 10, RainbowWave::Direction::vertical, false), "RainbowWave(0.2, 10, vertical, false)"});




  // start time tracking for main loop
  lastLoopTime = millis();
}

TextScroller settingsMenuTextScroller_settime(display, "Set Time", 1000, true, 1);

void loop()
{

  // update buttons
  for (Button2 button : buttons) {
    button.loop();
  }

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

  // if (displayEffects[effectIndex]->finished()) {
  //   Serial.println("Effect finished!");
  //   effectIndex++;
  //   if (effectIndex >= displayEffects.size()) {
  //     effectIndex = 0;
  //   }
  //   displayEffects[effectIndex]->reset();
  // }
  // displayEffects[effectIndex]->run();

  // update display
  display.fill(0);
  display.applyFilter(HSVTestPattern());

  display.applyFilter(*(filterConfigs[filterIndex].filter));
  if (millis() - lastFilterChangeTime > filterChangePeriod) {
    filterIndex++;
    if (filterIndex >= filterConfigs.size()) {
      filterIndex = 0;
    }
    Serial.print("Filter: "); Serial.println(filterConfigs[filterIndex].description);
    lastFilterChangeTime = millis();
  }

  //showTime(display, hourFormat12(), minute(), CRGB::Red);
  //display.applyFilter(SolidColour(CRGB::Purple));
  //display.applyFilter(RainbowWave(0.1, 100));
  FastLED.setBrightness(255);
  display.update();

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

  while (millis() - lastLoopTime < loopTargetTime) {
    yield();
    FastLED.show();
  }
  lastLoopTime = millis();
}


