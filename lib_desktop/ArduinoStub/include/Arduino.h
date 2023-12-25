#ifndef arduino_stub_h
#define arduino_stub_h

#include <chrono>

// #ifdef DESKTOP

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

// stub out Arduino functions in x86 environment
inline unsigned long millis() {
    // static unsigned long mil = 0;
    // mil += 10;
    // return mil;
        auto m = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() % 1000;
        return m;
}
inline unsigned long micros() { return 0; }

// #endif

#endif // arduino_stub_h
