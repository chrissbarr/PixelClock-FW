#ifndef arduino_stub_h
#define arduino_stub_h

//#ifdef DESKTOP

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

// stub out Arduino functions in x86 environment
inline unsigned long millis() { return 0; }
inline unsigned long micros() { return 0; }

//#endif

#endif // arduino_stub_h
