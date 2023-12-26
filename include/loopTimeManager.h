#ifndef looptimemanager_h
#define looptimemanager_h

/* C++ Standard Library */
#include <cstdint>

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