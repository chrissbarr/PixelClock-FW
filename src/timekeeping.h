#ifndef timekeeping_h
#define timekeeping_h

#include <Arduino.h>
#include <TimeLib.h>
#include <RTClib.h>

#include <memory>

struct ClockFaceTimeStruct {

    uint8_t hour12;
    uint8_t hour24;
    uint8_t minute;
    uint8_t second;
};

ClockFaceTimeStruct timeCallbackFunction();

constexpr char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Provide the RTC time to the Time library.
time_t time_provider();

bool initialiseTime();

bool initialiseRTC();


#endif