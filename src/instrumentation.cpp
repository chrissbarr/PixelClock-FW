/* Project Scope */
#include "instrumentation.h"
#include "FMTWrapper.h"

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
    sum += value;
    sumSamples++;
    if (empty) {
        min = value;
        max = value;
        empty = false;
    } else {
        if (value < min) { min = value; }
        if (value > max) { max = value; }
    }
    avg = sum / sumSamples;
}

void InstrumentationTrace::reset() {
    min = 0;
    max = 0;
    avg = 0;
    sum = 0;
    sumSamples = 0;
    empty = true;
}

std::string formatInstrumentationTrace(std::string name, const InstrumentationTrace& trace) {
    return fmt::format(
        "{:<20} (Min - Max - Avg): {:5d} - {:5d} - {:5d}\n", name, trace.getMin(), trace.getMax(), trace.getAvg());
};
