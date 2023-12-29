#ifndef arduino_stub_h
#define arduino_stub_h

#include <chrono>

#include <WString.h>

// #ifdef DESKTOP

typedef bool boolean;
typedef uint8_t byte;
typedef unsigned int word;

#define LOW               0x0
#define HIGH              0x1

//GPIO FUNCTIONS
#define INPUT             0x01
// Changed OUTPUT from 0x02 to behave the same as Arduino pinMode(pin,OUTPUT)
// where you can read the state of pin even when it is set as OUTPUT
#define OUTPUT            0x03
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x12
#define ANALOG            0xC0

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

// stub out Arduino functions in x86 environment
inline unsigned long millis() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    auto now = steady_clock::now();
    auto m = duration_cast<milliseconds>(now - start).count();
    return static_cast<unsigned long>(m);
}

inline unsigned long micros() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    auto now = steady_clock::now();
    auto m = duration_cast<microseconds>(now - start).count();
    return static_cast<unsigned long>(m);
}

inline void delay(unsigned long d) {
    auto start = millis();
    while (millis() - start < d) {}
}

inline void pinMode([[maybe_unused]] uint8_t pin, [[maybe_unused]] uint8_t mode) {}
inline void digitalWrite([[maybe_unused]] uint8_t pin, [[maybe_unused]] uint8_t val) {}
inline int digitalRead([[maybe_unused]] uint8_t pin) { return 0; }

inline void yield() {}

// #endif

#endif // arduino_stub_h
