#ifndef instrumentation_h
#define instrumentation_h

/* C++ Standard Library */
#include <cstdint>
#include <string>
#include <vector>

class InstrumentationTrace {
public:
    InstrumentationTrace(std::string name);
    void start();
    void stop();
    void update(uint32_t value);
    uint32_t getMin() const { return min; }
    uint32_t getMax() const { return max; }
    uint32_t getAvg() const { return avg; }
    uint32_t getHits() const { return hits; }
    const std::string& getName() const { return name; }
    void reset();

private:
    std::string name;
    uint32_t started{};
    uint32_t avg{};
    uint64_t sum{};
    uint32_t sumSamples{};
    uint32_t hits{};
    uint32_t min{};
    uint32_t max{};
    bool empty{true};
};

class Instrumented {
public:
    virtual std::vector<InstrumentationTrace*> getInstrumentation() = 0;
};

std::string formatInstrumentationTrace(std::string name, const InstrumentationTrace& trace);

#endif // instrumentation_h