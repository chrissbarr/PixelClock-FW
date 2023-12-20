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

void printTextCentred(const char* text, uint8_t width);
void printSolidLine(uint8_t width);

} // namespace printFormatting

void listAllI2CDevices(TwoWire& wire);

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