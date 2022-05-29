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

ClockFaceTimeStruct timeCallbackFunction(time_t time);
ClockFaceTimeStruct timeCallbackFunction();

constexpr char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

bool initialiseTime();

bool initialiseRTC();

// Sets the time to the RTC (if available) and time library
void setTimeGlobally(uint32_t timeToSet);


#endif