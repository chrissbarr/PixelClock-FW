#ifndef utility_h
#define utility_h

/* Arduino Core */
#include <Arduino.h>
#include <Wire.h>

/* C++ Standard Library */
#include <iterator>
#include <numeric>

namespace utility {

namespace printFormatting {

constexpr uint8_t headingWidth = 40;
constexpr uint8_t textPadding = 20;

inline void print(Print& p, const std::string& s) { p.print(s.c_str()); }

void printCentred(Print& p, const std::string& s, uint8_t width);
void printSolidLine(Print& p, uint8_t width);

} // namespace printFormatting

std::vector<uint8_t> scanI2CDevices(TwoWire& wire);

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

} // namespace utility

#endif // utility_h