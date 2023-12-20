/* Project Scope */
#include "instrumentation.h"

/* Arduino Core */
#include <Arduino.h>

/* C++ Standard Library */
#include <numeric>

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
