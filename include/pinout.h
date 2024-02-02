#ifndef pinout_h
#define pinout_h

/* C++ Standard Library */
#include <cstdint>

// Pinout

namespace pins {

// Configuration
#define PCB_REV 2
#define MATRIX_TYPE_APA102
//#define MATRIX_TYPE_SK6812RGBW

#if PCB_REV == 1

constexpr int16_t matrixLEDData = 4;
constexpr int16_t matrixLEDClock = -1;

constexpr int16_t button1 = 27;
constexpr int16_t button2 = 33;
constexpr int16_t button3 = 15;
constexpr int16_t button4 = 32;
constexpr int16_t button5 = 14;
constexpr int16_t buzzer = 26;

constexpr int16_t amplifierPower = 25;
constexpr int16_t amplifierReset = 12;

constexpr int16_t i2sBclk = 21;
constexpr int16_t i2sDout = 5;
constexpr int16_t i2sWclk = 19;

#endif

#if PCB_REV == 2

constexpr int16_t matrixLEDData = 23;
constexpr int16_t matrixLEDClock = 18;

constexpr int16_t button1 = 35;
constexpr int16_t button2 = 34;
constexpr int16_t button3 = 39;
constexpr int16_t button4 = 36;
constexpr int16_t button5 = 4;
constexpr int16_t buzzer = 25;

constexpr int16_t amplifierReset = 26;
constexpr int16_t amplifierPowerdown = 13;

constexpr int16_t i2sBclk = 27;
constexpr int16_t i2sDout = 32;
constexpr int16_t i2sWclk = 33;

#endif

} // namespace pins

#endif // pinout_h