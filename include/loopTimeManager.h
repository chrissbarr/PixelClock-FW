#ifndef looptimemanager_h
#define looptimemanager_h

/* Project Scope */
#include "instrumentation.h"

/* C++ Standard Library */
#include <cstdint>
#include <functional>
#include <vector>

class LoopTimeManager {
public:
    LoopTimeManager(uint32_t desiredLoopDuration, uint32_t statReportInterval);
    void idle();

    void registerTraceCallback(std::function<std::vector<InstrumentationTrace*>()> callback) {
        callbacks.push_back(callback);
    }

private:
    const uint32_t desiredLoopDuration;
    uint32_t lastLoopTime = 0;
    const uint32_t statReportInterval;
    uint32_t lastStatReportTime = 0;

    InstrumentationTrace loop{"Overall Loop"};
    InstrumentationTrace printout{"Instrumentation Printout"};

    std::vector<std::function<std::vector<InstrumentationTrace*>()>> callbacks;
};

#endif