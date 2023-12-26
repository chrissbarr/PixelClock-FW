/* Project Scope */
#include "loopTimeManager.h"
#include "FMTWrapper.h"
#include "utility.h"

#include <Arduino.h>

using namespace printing;

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
            fmt::format(
                "Loop Timing Statistics (Min - Max - Avg): {} - {} - {:.2f}\n", loopTimeMin, loopTimeMax, loopTimeAvg));
        // Serial.print("FastLED FPS:" ); Serial.println(FastLED.getFPS());

        // print memory usage stats
        #ifndef PIXELCLOCK_DESKTOP
        float usedHeapPercentage = 100 * (float(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize());
        uint8_t fieldWidth = 15;
        print(fmt::format("{1:<{0}}", fieldWidth, "Memory"));
        print(fmt::format("{1:<{0}}", fieldWidth, "free (kB)"));
        print(fmt::format("{1:<{0}}", fieldWidth, "total (kB)"));
        print(fmt::format("{1:<{0}}", fieldWidth, "used (%)"));
        print(fmt::format("{1:<{0}}", fieldWidth, "minfree (kB)"));
        print(fmt::format("{1:<{0}}", fieldWidth, "maxalloc (kB)"));
        print("\n");

        // print heap stats
        print(fmt::format("{1:<{0}}", fieldWidth, "Heap"));
        print(fmt::format("{1:<{0}}", fieldWidth, ESP.getFreeHeap() / 1024));
        print(fmt::format("{1:<{0}}", fieldWidth, ESP.getHeapSize() / 1024));
        print(fmt::format("{1:<{0}.2f}", fieldWidth, usedHeapPercentage));
        print(fmt::format("{1:<{0}}", fieldWidth, ESP.getMinFreeHeap() / 1024));
        print(fmt::format("{1:<{0}}", fieldWidth, ESP.getMaxAllocHeap() / 1024));
        print("\n");

        // print psram stats
        float usedPsramPercentage = 100 * (float(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize());
        print(fmt::format("{1:<{0}}", fieldWidth, "PSRAM"));
        print(fmt::format("{1:<{0}}", fieldWidth, ESP.getFreePsram() / 1024));
        print(fmt::format("{1:<{0}}", fieldWidth, ESP.getPsramSize() / 1024));
        print(fmt::format("{1:<{0}.2f}", fieldWidth, usedPsramPercentage));
        print(fmt::format("{1:<{0}}", fieldWidth, ESP.getMinFreePsram() / 1024));
        print(fmt::format("{1:<{0}}", fieldWidth, ESP.getMaxAllocPsram() / 1024));
        print("\n");
        #endif

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
