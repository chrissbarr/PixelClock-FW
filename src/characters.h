#ifndef characters_h
#define characters_h

#include <Arduino.h>

typedef uint8_t charMapType[5];

const int charCount = 4;
constexpr charMapType characterFontArray[] = {
  {   // space
    B000,
    B000,
    B000,
    B000,
    B000
  },
  {   // !
    B010,
    B010,
    B010,
    B000,
    B010
  },
  {   // "
    B011,
    B110,
    B000,
    B000,
    B000
  },
  {   // #
    B101,
    B111,
    B101,
    B111,
    B101
  },
  {   // $
    B111,
    B110,
    B111,
    B011,
    B111
  },
  {   // %
    B100,
    B001,
    B010,
    B100,
    B001
  },
  {   // &
    B111,
    B101,
    B111,
    B101,
    B110
  },
  {   // '
    B010,
    B010,
    B000,
    B000,
    B000
  },
  {   // (
    B010,
    B100,
    B100,
    B100,
    B010
  },
  {   // )
    B010,
    B001,
    B001,
    B001,
    B010
  },
  {   // *
    B000,
    B101,
    B010,
    B101,
    B000
  },
  {   // +
    B000,
    B010,
    B111,
    B010,
    B000
  },
  {   // '
    B010,
    B010,
    B000,
    B000,
    B000
  },
  {   // -
    B000,
    B000,
    B111,
    B000,
    B000
  },
  {   // .
    B000,
    B000,
    B000,
    B000,
    B010
  },
  {   // /
    B001,
    B010,
    B010,
    B010,
    B100
  },
  {   // 0
    B111,
    B101,
    B101,
    B101,
    B111
  },
  {   // 1
    B010,
    B110,
    B010,
    B010,
    B111
  },
  {   // 2
    B111,
    B001,
    B111,
    B100,
    B111
  },
  {   // 3
    B111,
    B001,
    B011,
    B001,
    B111
  },
  {   // 4
    B101,
    B101,
    B111,
    B001,
    B001
  },
  {   // 5
    B111,
    B100,
    B111,
    B001,
    B111
  },
  {   // 6
    B111,
    B100,
    B111,
    B101,
    B111
  },
  {   // 7
    B111,
    B001,
    B010,
    B100,
    B100
  },
  {   // 8
    B111,
    B101,
    B111,
    B101,
    B111
  },
  {   // 9
    B111,
    B101,
    B111,
    B001,
    B001
  },
  {   // :
    B000,
    B010,
    B000,
    B010,
    B000
  },
  {   // ;
    B000,
    B010,
    B000,
    B010,
    B100
  },
  {   // <
    B000,
    B010,
    B100,
    B010,
    B000
  },
  {   // =
    B000,
    B111,
    B000,
    B111,
    B000
  },
  {   // >
    B000,
    B010,
    B001,
    B010,
    B000
  },
  {   // ?
    B111,
    B001,
    B011,
    B000,
    B010
  },
  {   // @
    B111,
    B001,
    B111,
    B101,
    B111
  },
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
  {   // E
    B111,
    B100,
    B110,
    B100,
    B111
  },
  {   // F
    B111,
    B100,
    B110,
    B100,
    B100
  },
  {   // G
    B011,
    B100,
    B101,
    B101,
    B011
  },
  {   // H
    B101,
    B101,
    B111,
    B101,
    B101
  },
  {   // I
    B111,
    B010,
    B010,
    B010,
    B111
  },
  {   // J
    B001,
    B001,
    B001,
    B101,
    B111
  },
  {   // K
    B101,
    B101,
    B110,
    B101,
    B101
  },
  {   // L
    B100,
    B100,
    B100,
    B100,
    B111
  },
  {   // M
    B101,
    B111,
    B101,
    B101,
    B101
  },
  {   // N
    B001,
    B101,
    B111,
    B101,
    B100
  },
  {   // O
    B111,
    B101,
    B101,
    B101,
    B111
  },
  {   // P
    B111,
    B101,
    B111,
    B100,
    B100
  },
  {   // Q
    B111,
    B101,
    B101,
    B111,
    B011
  },
  {   // R
    B111,
    B101,
    B111,
    B110,
    B101
  },
  {   // S
    B111,
    B100,
    B111,
    B001,
    B111
  },
  {   // T
    B111,
    B010,
    B010,
    B010,
    B010
  },
  {   // U
    B101,
    B101,
    B101,
    B101,
    B111
  },
  {   // V
    B101,
    B101,
    B101,
    B101,
    B010
  },
  {   // W
    B101,
    B101,
    B101,
    B111,
    B101
  },
  {   // X
    B101,
    B101,
    B010,
    B101,
    B101
  },
  {   // Y
    B101,
    B101,
    B010,
    B010,
    B010
  },
  {   // Z
    B111,
    B001,
    B010,
    B100,
    B111
  },
};

#endif // characters_h