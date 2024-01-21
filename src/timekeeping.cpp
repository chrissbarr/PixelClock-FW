/* Project Scope */
#include "timekeeping.h"
#include "FMTWrapper.h"
#include "utility.h"

/* Libraries */
#include <fmt/chrono.h>
#ifndef PIXELCLOCK_DESKTOP
#include <RTClib.h>
#include <TimeLib.h>
#endif

/* C++ Standard Library */
#ifdef PIXELCLOCK_DESKTOP
#include <chrono>
#endif

using namespace printing;

#ifndef PIXELCLOCK_DESKTOP
bool TimeManagerEmbedded::initialise() {
    if (initialiseRTC()) {
        // Set Time to sync from RTC
        syncRTC();

        if (timeStatus() != timeSet) {
            print("Unable to sync with the RTC.\n");
        } else {
            print("RTC has set the system time.\n");
        }
    } else {
        print("Setting time to placeholder value.\n");
        ::setTime(11, 55, 50, 1, 1, 2022);
    }
    return true;
}

bool TimeManagerEmbedded::initialiseRTC() {

    print("Initialising RTC: \n");

    rtc = std::make_unique<RTC_DS3231>();

    if (!rtc->begin()) {
        print("Error!\n");
        rtc.reset();
        return false;
    } else {
        print("Success!\n");

        if (rtc->lostPower()) {
            print("RTC has lost power and time needs to be set!\n");
        } else {
            print("RTC reports it has not lost power.\n");
        }

        print("RTC has time: \n");

        DateTime now = rtc->now();
        print(fmt::format(
            "{:04d}/{:02d}/{:02d} ({}) : {:02d}:{:02d}:{:02d}\n",
            now.year(),
            now.month(),
            now.day(),
            daysOfTheWeek[now.dayOfTheWeek()],
            now.hour(),
            now.minute(),
            now.second()));

        print(fmt::format("since midnight 1/1/1970 = {}s = {}d\n", now.unixtime(), now.unixtime() / 86400L));
        return true;
    }
}

std::time_t TimeManagerEmbedded::now() const { return std::time_t(::now()); }

void TimeManagerEmbedded::setTime(std::time_t newTime) {
    if (rtc) {
        print("Setting RTC time...\n");
        rtc->adjust(newTime);
    }
    print("Setting time...\n");
    ::setTime(newTime);
}

void TimeManagerEmbedded::update() {
    auto timeNow = now();
    if (timeNow - syncLast > syncInterval) {
        syncRTC();
        syncLast = timeNow;
    }
}

void TimeManagerEmbedded::syncRTC() {
    if (!rtc) { return; }
    ::setTime(time_t(rtc->now().unixtime()));
}

#endif

#ifdef PIXELCLOCK_DESKTOP
bool TimeManagerDesktop::initialise() { return true; }
std::time_t TimeManagerDesktop::now() const {
    const auto tnow = std::chrono::system_clock::now();
    const std::time_t tc = std::chrono::system_clock::to_time_t(tnow);
    return tc;
}
void TimeManagerDesktop::setTime([[maybe_unused]] std::time_t newTime) {}
void TimeManagerDesktop::update() {}
#endif

int hour24to12(int hour) {
    if (hour == 0) {
        return 12; // 12 midnight
    } else if (hour > 12) {
        return hour - 12;
    } else {
        return hour;
    }
}

ClockFaceTimeStruct timeCallbackFunction(std::time_t time) {
    // extract elements of time into struct
    std::tm caltime = fmt::localtime(time);
    ClockFaceTimeStruct val{};
    val.hour12 = static_cast<uint8_t>(hour24to12(caltime.tm_hour));
    val.hour24 = static_cast<uint8_t>(caltime.tm_hour);
    val.minute = static_cast<uint8_t>(caltime.tm_min);
    val.second = static_cast<uint8_t>(caltime.tm_sec);
    return val;
}

ClockFaceTimeStruct timeCallbackFunction() { return timeCallbackFunction(TimeManagerSingleton::get().now()); }