// C++ Std Library
#include <memory>
#include <vector>
#include <string>

// Libraries
#include <SPI.h>
#include <FastLED.h>
#include <TimeLib.h>
#include <RTClib.h>
#include <Button2.h>
#include <TSL2591I2C.h>

// Project Scope
#include "characters.h"
#include "display.h"
#include "displayEffects.h"
#include "fastled_rgbw.h"

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

// Buttons

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

TSL2591I2C tsl2591;

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

// Timekeeping
RTC_DS3231 rtc;

ClockFaceTimeStruct timeCallbackFunction() {
  DateTime now = rtc.now();
  ClockFaceTimeStruct val;
  val.hour = now.hour();
  val.minute = now.minute();
  val.second = now.second();
  return val;
}

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

void initialiseLightSensor()
{
  Wire.begin();
  if (!tsl2591.begin())
	{
		Serial.println("begin() failed. check the connection to your TSL2591.");
		while (1);
	}

	Serial.print("sensor ID: "); Serial.println(tsl2591.getID(), HEX);

	tsl2591.resetToDefaults();

	//set channel
	tsl2591.setChannel(TSL2591MI::TSL2591_CHANNEL_0);

	//set gain
	tsl2591.setGain(TSL2591MI::TSL2591_GAIN_MED);

	//set integration time
	tsl2591.setIntegrationTime(TSL2591MI::TSL2591_INTEGRATION_TIME_100ms);

  if (!tsl2591.measure()) {
    Serial.println("could not start measurement. ");
    return;
  }

  while (!tsl2591.hasValue()) {
    delay(1);
  }

	Serial.print("Irradiance: "); Serial.print(tsl2591.getIrradiance(), 7); Serial.println(" W / m^2");
	Serial.print("Brightness: "); Serial.print(tsl2591.getBrightness(), 7); Serial.println(" lux");
  
}

void setup() {
  delay(1000);
  Serial.begin(250000);
  Serial.println("Serial begin!");

  initialiseLightSensor();

  FastLED.addLeds<WS2812, matrixLEDPin, RGB>(ledsDummyRGBW, dummyLEDCount);
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

  displayDiagnostic(display);

  displayEffects.push_back(std::make_unique<GameOfLife>(display, 100, colourGenerator_cycleHSV, display.getFullDisplayRegion(), false));
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
  //display.fill(0);
  //display.applyFilter(HSVTestPattern());
  //showTime(display, hourFormat12(), minute(), CRGB::Red);


  //display.applyFilter(*(filterConfigs[filterIndex].filter));
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
  }
  lastLoopTime = millis();

  if (tsl2591.hasValue()) {
    Serial.print("Irradiance: "); Serial.print(tsl2591.getIrradiance(), 7); Serial.println(" W / m^2");
    Serial.print("Brightness: "); Serial.print(tsl2591.getBrightness(), 7); Serial.println(" lux");
    float maxBrightness = 1.7;
    uint8_t brightness = uint8_t(constrain(map(tsl2591.getBrightness() * 1000, 0, 1700, 0, 255), 1, 255));
    FastLED.setBrightness(brightness);
    tsl2591.measure();
  }

}


