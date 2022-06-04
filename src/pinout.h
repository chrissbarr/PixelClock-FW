#ifndef pinout_h
#define pinout_h

#include <cstdint>

// Pinout
constexpr int16_t matrixLEDPin = 4;
constexpr int16_t buttonPin1 = 27;
constexpr int16_t buttonPin2 = 33;
constexpr int16_t buttonPin3 = 15;
constexpr int16_t buttonPin4 = 32;
constexpr int16_t buttonPin5 = 14;
constexpr int16_t buzzerPin = 26;

constexpr int16_t amplifierPowerPin = 25;
constexpr int16_t amplifierResetPin = 12;
constexpr int16_t amplifierPowerdownPin = 13;

constexpr int bclk = 21;
constexpr int dout = 5;
constexpr int wclk = 19;

#endif // pinout_h