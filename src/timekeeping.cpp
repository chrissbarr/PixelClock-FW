/* Project Scope */
#include "timekeeping.h"
#include "FMTWrapper.h"
#include "utility.h"

std::unique_ptr<RTC_DS3231> rtc;
using namespace printing;

bool initialiseTime() {
    if (initialiseRTC()) {
        // Set Time to sync from RTC
        setSyncProvider([]() { return time_t(rtc->now().unixtime()); });
        setSyncInterval(60);

        if (timeStatus() != timeSet) {
            Serial.println("Unable to sync with the RTC.");
        } else {
            Serial.println("RTC has set the system time");
        }
    } else {
        Serial.println("Setting time to placeholder value.");
        setTime(11, 55, 50, 1, 1, 2022);
    }
    return true;
}

bool initialiseRTC() {
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

        Serial.println("RTC has time: ");

        DateTime now = rtc->now();
        print(
            Serial,
            fmt::format(
                "{:04d}/{:02d}/{:02d} ({}) : {:02d}:{:02d}:{:02d}\n",
                now.year(),
                now.month(),
                now.day(),
                daysOfTheWeek[now.dayOfTheWeek()],
                now.hour(),
                now.minute(),
                now.second()));

        print(Serial, fmt::format("since midnight 1/1/1970 = {}s = {}d\n", now.unixtime(), now.unixtime() / 86400L));
        return true;
    }
}

ClockFaceTimeStruct timeCallbackFunction(time_t time) {
    // extract elements of time into struct
    ClockFaceTimeStruct val;
    val.hour12 = hourFormat12(time);
    val.hour24 = hour(time);
    val.minute = minute(time);
    val.second = second(time);
    return val;
}

ClockFaceTimeStruct timeCallbackFunction() { return timeCallbackFunction(now()); }

void setTimeGlobally(uint32_t timeToSet) {
    Serial.println("Setting time...");
    if (rtc) {
        Serial.println("Setting RTC time...");
        rtc->adjust(timeToSet);
    }
    setTime(timeToSet);
}

LoopTimeManager::LoopTimeManager(uint32_t desiredLoopDuration, uint32_t statReportInterval)
    : desiredLoopDuration(desiredLoopDuration),
      statReportInterval(statReportInterval) {
    loopTimeMin = std::numeric_limits<uint16_t>::max();
    loopTimeMax = 0;
}

void LoopTimeManager::idle() {
    // Manage loop timing
    uint32_t loopTime = millis() - lastLoopTime;

    loopTimeAvg = approxRollingAverage(loopTimeAvg, float(loopTime), 1000);
    if (loopTime > loopTimeMax) { loopTimeMax = loopTime; }
    if (loopTime < loopTimeMin) { loopTimeMin = loopTime; }

    if (millis() - lastStatReportTime > statReportInterval) {
        // print timing stats
        print(
            Serial,
            fmt::format(
                "Loop Timing Statistics (Min - Max - Avg): {} - {} - {:.2f}\n", loopTimeMin, loopTimeMax, loopTimeAvg));
        // Serial.print("FastLED FPS:" ); Serial.println(FastLED.getFPS());

        // print memory usage stats
        float usedHeapPercentage = 100 * (float(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize());
        uint8_t fieldWidth = 15;
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, "Memory"));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, "free (kB)"));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, "total (kB)"));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, "used (%)"));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, "minfree (kB)"));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, "maxalloc (kB)"));
        print(Serial, "\n");

        // print heap stats
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, "Heap"));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, ESP.getFreeHeap() / 1024));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, ESP.getHeapSize() / 1024));
        print(Serial, fmt::format("{1:<{0}.2f}", fieldWidth, usedHeapPercentage));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, ESP.getMinFreeHeap() / 1024));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, ESP.getMaxAllocHeap() / 1024));
        print(Serial, "\n");

        // print psram stats
        float usedPsramPercentage = 100 * (float(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize());
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, "PSRAM"));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, ESP.getFreePsram() / 1024));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, ESP.getPsramSize() / 1024));
        print(Serial, fmt::format("{1:<{0}.2f}", fieldWidth, usedPsramPercentage));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, ESP.getMinFreePsram() / 1024));
        print(Serial, fmt::format("{1:<{0}}", fieldWidth, ESP.getMaxAllocPsram() / 1024));
        print(Serial, "\n");

        lastStatReportTime = millis();
        loopTimeMin = std::numeric_limits<uint16_t>::max();
        loopTimeMax = 0;
    }

    // ensure we yield at least once
    yield();
    while (millis() - lastLoopTime < desiredLoopDuration) {
        // idle in this loop until desiredLoopDuration has elapsed
        yield();
    }
    lastLoopTime = millis();
}

constexpr float LoopTimeManager::approxRollingAverage(float avg, float newSample, int N) const {
    avg -= avg / N;
    avg += newSample / N;
    return avg;
}
