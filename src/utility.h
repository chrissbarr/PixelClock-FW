#ifndef utility_h
#define utility_h

#include <Arduino.h>
#include <Wire.h>

namespace utility {

namespace printFormatting {

constexpr uint8_t headingWidth = 40;
constexpr uint8_t textPadding = 20;

void printTextCentred(const char* text, uint8_t width);
void printSolidLine(uint8_t width);

} // namespace printFormatting

void listAllI2CDevices(TwoWire& wire);

} // namespace utility

#endif // utility_h