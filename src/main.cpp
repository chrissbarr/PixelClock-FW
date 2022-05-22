#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <RTClib.h>

#include "characters.h"
#include "display.h"
#include "displayEffects.h"

// LED Panel Configuration
constexpr int16_t matrixLEDPin = 14;
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;

Adafruit_NeoPixel pixels(matrixSize, matrixLEDPin, NEO_GRBW + NEO_KHZ800);

PixelDisplay display(pixels, matrixWidth, matrixHeight, false, false);

unsigned long lastLoopTime = 0;
constexpr unsigned long loopTime = 25;

RTC_DS3231 rtc;

void displayDiagnostic(PixelDisplay& display)
{
  // Clear display
  display.fill(0);
  display.update();
  delay(250);

  // Show Pixel 0
  display.setXY(0, 0, Adafruit_NeoPixel::Color(255, 0, 0));
  display.update();
  delay(250);

  // Solid Red, Green, Blue
  display.fill(Adafruit_NeoPixel::Color(255, 0, 0));
  display.update();
  delay(250);
  display.fill(Adafruit_NeoPixel::Color(0, 255, 0));
  display.update();
  delay(250);
  display.fill(Adafruit_NeoPixel::Color(0, 0, 255));
  display.update();
  delay(250);

  // Move through XY sequentially
  for (uint8_t y = 0; y < display.getHeight(); y++) {
    for (uint8_t x = 0; x < display.getWidth(); x++) {
    display.fill(0);
    display.setXY(x, y, Adafruit_NeoPixel::Color(100, 0, 0));
    display.update();
    delay(1);
    }
  }

  // Scroll short test
  display.fill(0);
  display.update();
  auto textScrollTest1 = TextScroller(
    display,
    "Hello - Testing!",
    500,
    true,
    1
  );
  while(!textScrollTest1.update(colorGenerator_cycleHSV(), 50)) {
    display.update();
    display.fill(0);
  }
  

  // Scroll full character set
  display.fill(0);
  display.update();
  auto textScrollTest = TextScroller(
    display,
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 !\"#$%&'()*+'-./:;<=>?@",
    500,
    false,
    1
  );
  while(!textScrollTest.update(Adafruit_NeoPixel::Color(0, 25, 0), 10)) {
    display.update();
    display.fill(0);
  }
  display.fill(0);
  display.update();
}

int effectIndex = 0;

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
  pixels.begin();
  pixels.setBrightness(100);

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


  display.fill(0);
  display.update();
  delay(500); 

  //displayDiagnostic(display);
}

void loop()
{

  // DisplayRegion randRegion;
  // randRegion.xMin = 10;
  // randRegion.xMax = 16;
  // randRegion.yMin = 0;
  // randRegion.yMax = 4;

  // switch (effectIndex) {
  //   case 0:
  //     if (fillRandomly(display, 100, colourGenerator_randomHSV, randRegion)) {
  //       effectIndex++;
  //       display.fill(0, randRegion);
  //     }
  //     break;
  //   case 1:
  //     if (fillRandomly(display, 10, colourGenerator_randomHSV, randRegion)) {
  //       effectIndex++;
  //       display.fill(0, randRegion);
  //     }
  //     break;
  //   case 2:
  //     if (fillRandomly(display, 10, colourGenerator_randomHSV, randRegion)) {
  //       effectIndex++;
  //       display.fill(0, randRegion);
  //     }
  //   break;
  //   default:
  //     effectIndex = 0;
  // }

  // static int state = 0;
  // switch(state) {
  //   case 0:
  //     gravityFill(display, 100, 100, false, colourGenerator_randomHSV);
  //     if (display.filled(0)) {
  //       state = 1;
  //     }
  //     break;
  //   case 1:
  //     gravityFill(display, 0, 100, true, colourGenerator_randomHSV);
  //     if (display.empty()) {
  //       state = 0;
  //     }
  //     break;
  // }

  static int state = 0;
  static int minPrev;
  static uint32_t startedWaitingTime = 0;
  time_t currentTime = now();
  switch(state) {
    case 0:
      display.fill(0);
      showTime(display, hourFormat12(currentTime), minute(currentTime), colorGenerator_cycleHSV());
      minPrev = minute(currentTime);
      state = 1;
      break;
    case 1:
      if (minPrev != minute(currentTime)) {
        state = 2;
        break;
      }
      display.fill(0);
      showTime(display, hourFormat12(currentTime), minute(currentTime), colorGenerator_cycleHSV());
      break;
    case 2:
      // text fall off screen
      gravityFill(display, 0, 100, true, colourGenerator_randomHSV);
      if (startedWaitingTime == 0 && display.empty()) {
        startedWaitingTime = millis();
      }
      if (startedWaitingTime != 0 && millis() - startedWaitingTime > 1000) {
        state = 0;
        startedWaitingTime = 0;
      }
      break;
  }

  //tetris(display, 100, 100);
  //if (display.filled()) { display.fill(0); }


  
  display.update(); 

  // Serial.println(millis());

  // Manage loop timing
  unsigned long loopTime = millis() - lastLoopTime;
  //Serial.print("Loop time:" ); Serial.println(loopTime);
  while (millis() - lastLoopTime < loopTime) {}
  lastLoopTime = millis();
}


