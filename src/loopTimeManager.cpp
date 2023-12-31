/* Project Scope */
#include "loopTimeManager.h"
#include "FMTWrapper.h"
#include "utility.h"

/* Arduino Core */
#include <Arduino.h>

using namespace printing;

LoopTimeManager::LoopTimeManager(uint32_t desiredLoopDuration, uint32_t statReportInterval)
    : desiredLoopDuration(desiredLoopDuration),
      statReportInterval(statReportInterval) {}

void LoopTimeManager::idle() {
    // Manage loop timing

    if (millis() - lastStatReportTime > statReportInterval) {

        printout.start();

        constexpr int nameWidth = 30;
        constexpr int fieldWidth = 15;

        // print timing stats
        printCentred("Timing Statistics", headingWidth);
        print(fmt::format("Time Now: {} ms\n", millis()));

        print(fmt::format(
            "{2:<{0}}{3:<{1}}{4:<{1}}{5:<{1}}{6:<{1}}\n",
            nameWidth,
            fieldWidth,
            "",
            "Hits",
            "Min (us)",
            "Max (us)",
            "Avg (us)"));

        auto printTrace = [&](const InstrumentationTrace& trace) {
            print(fmt::format(
                "{2:<{0}}{3:<{1}}{4:<{1}}{5:<{1}}{6:<{1}}\n",
                nameWidth,
                fieldWidth,
                trace.getName(),
                trace.getHits(),
                trace.getMin(),
                trace.getMax(),
                trace.getAvg()));
        };

        printTrace(loop);
        loop.reset();

        for (auto& c : callbacks) {
            auto traces = c();
            for (auto trace : traces) {
                printTrace(*trace);
                trace->reset();
            }
        }

        printTrace(printout);

// print memory usage stats
#ifndef PIXELCLOCK_DESKTOP

        printCentred("Memory Usage", headingWidth);

        // print header row
        print(fmt::format("{1:<{0}}", nameWidth, "Memory"));
        print(fmt::format("{1:<{0}}", fieldWidth, "free (kB)"));
        print(fmt::format("{1:<{0}}", fieldWidth, "total (kB)"));
        print(fmt::format("{1:<{0}}", fieldWidth, "used (%)"));
        print(fmt::format("{1:<{0}}", fieldWidth, "minfree (kB)"));
        print(fmt::format("{1:<{0}}", fieldWidth, "maxalloc (kB)"));
        print("\n");

        // print heap stats
        print(fmt::format("{1:<{0}}", nameWidth, "Heap"));
        float usedHeapPercentage = 100 * (float(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize());
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

        printout.stop();
        lastStatReportTime = millis();
    }

    loop.stop();
    // ensure we yield at least once
    yield();
    while (millis() - lastLoopTime < desiredLoopDuration) {
        // idle in this loop until desiredLoopDuration has elapsed
        yield();
    }
    loop.start();
    lastLoopTime = millis();
}