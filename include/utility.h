#ifndef utility_h
#define utility_h

/* Arduino Core */
#include <Arduino.h>
#ifndef PIXELCLOCK_DESKTOP
#include <Wire.h>
#endif

/* C++ Standard Library */
#include <iterator>
#include <numeric>
#include <string>

namespace utility {

#ifndef PIXELCLOCK_DESKTOP
std::vector<uint8_t> scanI2CDevices(TwoWire& wire);
#endif

template <class T, class M> auto sum_members(const T& container, M member) {
    // Deduce the type of the member
    // It's important to specify what type to use in the sum
    using t_element = decltype(*std::begin(container));
    using t_value = decltype(std::declval<t_element>().*member);

    return std::accumulate(
        std::begin(container), std::end(container), t_value{}, [member](const auto sum, const auto& next) {
            return sum + (next.*member);
        });
}

float mapNumericRange(float input, float fromMin, float fromMax, float toMin, float toMax);

} // namespace utility

namespace printing {

constexpr uint8_t headingWidth = 40;
constexpr uint8_t textPadding = 20;

void print(const std::string& s);
void printCentred(const std::string& s, uint8_t width);
void printSolidLine(uint8_t width);

} // namespace printing

#endif // utility_h