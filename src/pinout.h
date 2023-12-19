#ifndef pinout_h
#define pinout_h

/* C++ Standard Library */
#include <cstdint>

// Pinout

namespace pins {

constexpr int16_t matrixLED = 4;
constexpr int16_t button1 = 27;
constexpr int16_t button2 = 33;
constexpr int16_t button3 = 15;
constexpr int16_t button4 = 32;
constexpr int16_t button5 = 14;
constexpr int16_t buzzer = 26;

constexpr int16_t amplifierPower = 25;
constexpr int16_t amplifierReset = 12;
constexpr int16_t amplifierPowerdown = 13;

constexpr int16_t i2sBclk = 21;
constexpr int16_t i2sDout = 5;
constexpr int16_t i2sWclk = 19;

} // namespace pins

#endif // pinout_h