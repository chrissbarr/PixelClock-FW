/* Project Scope */
#include "instrumentation.h"

/* Arduino Core */
#include <Arduino.h>

/* C++ Standard Library */
#include <numeric>
#include <string>

InstrumentationTrace::InstrumentationTrace() { reset(); }

void InstrumentationTrace::start() { started = micros(); }

void InstrumentationTrace::stop() {
    uint32_t duration = micros() - started;
    update(duration);
}

void InstrumentationTrace::update(uint32_t value) {
    if (empty) {
        min = value;
        max = value;
        avg = value;
        empty = false;
    } else {
        if (value < min) { min = value; }
        if (value > max) { max = value; }
        // todo better
        avg = (avg + value) / 2;
    }
}

void InstrumentationTrace::reset() {
    min = 0;
    max = 0;
    avg = 0;
    empty = true;
}

std::string formatInstrumentationTrace(std::string name, const InstrumentationTrace& trace) {
    constexpr uint8_t bufSize = 100;
    char c_buf[bufSize];
    snprintf(
        c_buf,
        bufSize,
        "%-20s (Min - Max - Avg): %5d - %5d - %5d\n",
        name.c_str(),
        trace.getMin(),
        trace.getMax(),
        trace.getAvg());
    std::string val(c_buf);
    return val;
};
