#ifndef timekeeping_h
#define timekeeping_h

/* Libraries */
#ifndef PIXELCLOCK_DESKTOP
#include <RTClib.h>
#include <TimeLib.h>
#endif

/* C++ Standard Library */
#include <ctime>
#include <memory>

struct ClockFaceTimeStruct {

    uint8_t hour12;
    uint8_t hour24;
    uint8_t minute;
    uint8_t second;
};

class TimeManager {
public:
    virtual bool initialise() = 0;
    virtual std::time_t now() const = 0;
    virtual void setTime(std::time_t newTime) = 0;
    virtual void update() = 0;
};

#ifndef PIXELCLOCK_DESKTOP
class TimeManagerEmbedded : public TimeManager {
public:
    bool initialise() override final;
    std::time_t now() const override final;
    void setTime(std::time_t newTime) override final;
    void update() override final;

private:
    std::unique_ptr<RTC_DS3231> rtc;
    bool initialiseRTC();
    void syncRTC();
    uint32_t syncInterval{60};
    uint32_t syncLast{0};
};
#else
class TimeManagerDesktop : public TimeManager {
public:
    bool initialise() override final;
    std::time_t now() const override final;
    void setTime(std::time_t newTime) override final;
    void update() override final;
};
#endif

class TimeManagerSingleton {
public:
    static TimeManager& get() {
#ifdef PIXELCLOCK_DESKTOP
        static TimeManagerDesktop instance;
#else
        static TimeManagerEmbedded instance;
#endif
        return instance;
    }

    // delete constructors
    TimeManagerSingleton(const TimeManagerSingleton&) = delete;
    void operator=(const TimeManagerSingleton&) = delete;
};

ClockFaceTimeStruct timeCallbackFunction(std::time_t time);
ClockFaceTimeStruct timeCallbackFunction();

constexpr char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#endif