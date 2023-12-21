/* Project Scope */
#include "utility.h"
#include "FMTWrapper.h"

/* C++ Standard Library */
#include <vector>

namespace utility {

namespace printFormatting {

void printSolidLine(Print& p, uint8_t width) { print(p, fmt::format("{1:-^{0}}\n", width, "")); }

void printCentred(Print& p, const std::string& s, uint8_t width) {
    print(p, fmt::format("{1:-^{0}}\n", width, fmt::format(" {} ", s)));
}

} // namespace printFormatting

std::vector<uint8_t> scanI2CDevices(TwoWire& wire) {
    std::vector<uint8_t> devices;
    for (uint8_t address = 1; address < 127; address++) {
        wire.beginTransmission(address);
        if (wire.endTransmission() == 0) { devices.push_back(address); }
    }
    return devices;
}

} // namespace utility