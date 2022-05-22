#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <RTClib.h>
#include <Button2.h>


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
unsigned long lastLoopTime = 0;
constexpr unsigned long loopTime = 25;

// Timekeeping
RTC_DS3231 rtc;

enum class Mode {
  DisplayTime,
  SetTime,
  Demo
};
Mode currentMode = Mode::DisplayTime;


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
  pixels.setBrightness(10);

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
  delay(500); 

  //displayDiagnostic(display);
}

void loop()
{

  // update buttons
  for (Button2 button : buttons) {
    button.loop();
  }

  time_t currentTime = now();
  switch (currentMode) {
    case Mode::DisplayTime:
    {
      display.fill(0);
      showTime(display, hourFormat12(currentTime), minute(currentTime), colorGenerator_cycleHSV());
      break;
    }
    case Mode::SetTime:
    {
      display.fill(0);
      if (second(currentTime) % 1) {
        showTime(display, hourFormat12(currentTime), minute(currentTime), colorGenerator_cycleHSV());
      }
      break;
    }
    case Mode::Demo:
    {
      tetris(display, 100, 100);
      if (display.filled()) { display.fill(0); }
    }
  }

  // update display
  display.update();
  //delay(1);
  //delayMicroseconds(100);
  pixels.fill(0);
  pixels.show();
  delay(2);


  // Manage loop timing
  unsigned long loopTime = millis() - lastLoopTime;
  //Serial.print("Loop time:" ); Serial.println(loopTime);
  while (millis() - lastLoopTime < loopTime) {}
  lastLoopTime = millis();
}


