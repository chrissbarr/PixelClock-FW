/* Project Scope */
#include "utility.h"
#include "FMTWrapper.h"

#ifndef PIXELCLOCK_DESKTOP
#include <Wire.h>
#endif
/* C++ Standard Library */
#include <vector>
#ifdef PIXELCLOCK_DESKTOP
#include <iostream>
#endif

namespace utility {

#ifndef PIXELCLOCK_DESKTOP
std::vector<uint8_t> scanI2CDevices(TwoWire& wire) {
    std::vector<uint8_t> devices;
    for (uint8_t address = 1; address < 127; address++) {
        wire.beginTransmission(address);
        if (wire.endTransmission() == 0) { devices.push_back(address); }
    }
    return devices;
}
#endif

float mapNumericRange(float input, float fromMin, float fromMax, float toMin, float toMax) {
    float slope = 1.0f * (toMax - toMin) / (fromMax - fromMin);
    float output = toMin + std::round(slope * (input - fromMin));
    return output;
}

} // namespace utility

namespace printing {

void print(const std::string& s) {
#ifdef PIXELCLOCK_DESKTOP
    std::cout << s;
#else
    Serial.print(s.c_str());
#endif
}

void printSolidLine(uint8_t width) { print(fmt::format("{1:-^{0}}\n", width, "")); }

void printCentred(const std::string& s, uint8_t width) {
    print(fmt::format("{1:-^{0}}\n", width, fmt::format(" {} ", s)));
}

} // namespace printing