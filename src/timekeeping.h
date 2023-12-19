#ifndef timekeeping_h
#define timekeeping_h

#include <Arduino.h>
#include <RTClib.h>
#include <TimeLib.h>

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

class LoopTimeManager {
public:
    LoopTimeManager(uint32_t desiredLoopDuration, uint32_t statReportInterval);
    void idle();

private:
    const uint32_t desiredLoopDuration;
    uint32_t lastLoopTime = 0;
    const uint32_t statReportInterval;
    uint32_t lastStatReportTime = 0;
    float loopTimeAvg = 0;
    uint16_t loopTimeMin = 0;
    uint16_t loopTimeMax = 0;
    constexpr float approxRollingAverage(float avg, float newSample, int N) const;
};

#endif