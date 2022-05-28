#include "timekeeping.h"

std::unique_ptr<RTC_DS3231> rtc;

// Provide the RTC time to the Time library.
time_t time_provider() {
  Serial.println("time provider called!");
  return rtc->now().unixtime();
}

bool initialiseTime()
{
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
  return true;
}

bool initialiseRTC()
{
  Serial.print("Initialising RTC: ");

  rtc = std::make_unique<RTC_DS3231>();

  if (!rtc->begin()) {
    Serial.println("Error!");
    return false;
  } else {
    Serial.println("Success!");

    if (rtc->lostPower()) {
      Serial.println("RTC has lost power and time needs to be set!");
    } else {
      Serial.println("RTC reports it has not lost power.");
    }

    DateTime now = rtc->now();
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

ClockFaceTimeStruct timeCallbackFunction()
{
  ClockFaceTimeStruct val;
  // get the time once, just in case it changes during this function
  time_t time = now();

  // extract elements of time into struct
  val.hour12 = hourFormat12(time);
  val.hour24 = hour(time);
  val.minute = minute(time);
  val.second = second(time);

  return val;
}