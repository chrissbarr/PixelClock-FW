#include "timekeeping.h"

std::unique_ptr<RTC_DS3231> rtc;

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
        Serial.printf(
            "Loop Timing Statistics (Min - Max - Avg): %d - %d - %.2f \n", loopTimeMin, loopTimeMax, loopTimeAvg);
        // Serial.print("FastLED FPS:" ); Serial.println(FastLED.getFPS());

        // print memory usage stats
        float usedHeapPercentage = 100 * (float(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize());
        uint8_t fieldWidth = 15;
        Serial.printf("%-*s", fieldWidth, "Memory");
        Serial.printf("%-*s", fieldWidth, "free (kB)");
        Serial.printf("%-*s", fieldWidth, "total (kB)");
        Serial.printf("%-*s", fieldWidth, "used (%)");
        Serial.printf("%-*s", fieldWidth, "minfree (kB)");
        Serial.printf("%-*s", fieldWidth, "maxalloc (kB)");
        Serial.printf("\n");

        // print heap stats
        Serial.printf("%-*s", fieldWidth, "Heap");
        Serial.printf("%-*d", fieldWidth, ESP.getFreeHeap() / 1024);
        Serial.printf("%-*d", fieldWidth, ESP.getHeapSize() / 1024);
        Serial.printf("%-*.2f", fieldWidth, usedHeapPercentage);
        Serial.printf("%-*d", fieldWidth, ESP.getMinFreeHeap() / 1024);
        Serial.printf("%-*d", fieldWidth, ESP.getMaxAllocHeap() / 1024);
        Serial.printf("\n");

        // print psram stats
        float usedPsramPercentage = 100 * (float(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize());
        Serial.printf("%-*s", fieldWidth, "PSRAM");
        Serial.printf("%-*d", fieldWidth, ESP.getFreePsram() / 1024);
        Serial.printf("%-*d", fieldWidth, ESP.getPsramSize() / 1024);
        Serial.printf("%-*.2f", fieldWidth, usedPsramPercentage);
        Serial.printf("%-*d", fieldWidth, ESP.getMinFreePsram() / 1024);
        Serial.printf("%-*d", fieldWidth, ESP.getMaxAllocPsram() / 1024);
        Serial.printf("\n");

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
