#ifndef characters_h
#define characters_h

#include <Arduino.h>

typedef uint8_t charMapType[5];

const int charCount = 4;
constexpr charMapType characterFontArray[] = {
  {   // A
    B010,
    B101,
    B111,
    B101,
    B101
  },
  {   // B
    B110,
    B101,
    B110,
    B101,
    B110
  },
  {   // C
    B011,
    B100,
    B100,
    B100,
    B011
  },
  {   // D
    B110,
    B101,
    B101,
    B101,
    B110
  },
};

#endif // characters_h