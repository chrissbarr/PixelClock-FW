#ifndef instrumentation_h
#define instrumentation_h

/* C++ Standard Library */
#include <cstdint>

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
    uint32_t min{};
    uint32_t max{};
    bool empty{true};
};

#endif // instrumentation_h