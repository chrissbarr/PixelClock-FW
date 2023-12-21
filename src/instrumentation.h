#ifndef instrumentation_h
#define instrumentation_h

/* C++ Standard Library */
#include <cstdint>
#include <string>

class InstrumentationTrace {
public:
    InstrumentationTrace();
    void start();
    void stop();
    void update(uint32_t value);
    uint32_t getMin() const { return min; }
    uint32_t getMax() const { return max; }
    uint32_t getAvg() const { return avg; }
    void reset();

private:
    uint32_t started{};
    uint32_t avg{};
    uint64_t sum{};
    uint32_t sumSamples{};
    uint32_t min{};
    uint32_t max{};
    bool empty{true};
};

std::string formatInstrumentationTrace(std::string name, const InstrumentationTrace& trace);

#endif // instrumentation_h