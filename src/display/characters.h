#ifndef characters_h
#define characters_h

#include <Arduino.h>

typedef uint8_t charMapType[5];
struct FontGlyph {
  uint8_t glyph[5];
  uint8_t width;
};

constexpr uint8_t charToIndex(char character)
{
  //Lowercase to uppercase
  if (character >= 'a' && character <= 'z') {
    return character - 32 - ' ';
  } 
  if (character >= ' ' && character <= 'Z') {
    return character - ' ';
  }
  return 0;
}

constexpr FontGlyph characterFontArray[] = {
  {   // space
    {
      B000,
      B000,
      B000,
      B000,
      B000
    }, 
    3
  },
  {   // !
    {
      B010,
      B010,
      B010,
      B000,
      B010
    }, 
    3
  },
  {   // "
    {
      B011,
      B110,
      B000,
      B000,
      B000
    },
    3
  },
  {   // #
    {
      B101,
      B111,
      B101,
      B111,
      B101
    },
    3,
  },
  {   // $
    {
      B111,
      B110,
      B111,
      B011,
      B111
    },
    3,
  },
  {   // %
    {
      B100,
      B001,
      B010,
      B100,
      B001
    },
    3,
  },
  {   // &
    {
      B111,
      B101,
      B111,
      B101,
      B110
    },
    3,
  },
  {   // '
    {
      B010,
      B010,
      B000,
      B000,
      B000
    },
    3,
  },
  {   // (
    {
      B010,
      B100,
      B100,
      B100,
      B010
    },
    3,
  },
  {   // )
    {
      B010,
      B001,
      B001,
      B001,
      B010
    },
    3,
  },
  {   // *
    {
      B000,
      B101,
      B010,
      B101,
      B000
    },
    3,
  },
  {   // +
    {
      B000,
      B010,
      B111,
      B010,
      B000
    },
    3,
  },
  {   // '
    {
      B010,
      B010,
      B000,
      B000,
      B000
    },
    3,
  },
  {   // -
    {
      B000,
      B000,
      B111,
      B000,
      B000
    },
    3,
  },
  {   // .
    {
      B000,
      B000,
      B000,
      B000,
      B010
    },
    3,
  },
  {   // /
    {
      B001,
      B010,
      B010,
      B010,
      B100
    },
    3,
  },
  {   // 0
    {
      B111,
      B101,
      B101,
      B101,
      B111
    },
    3,
  },
  {   // 1
    {
      B010,
      B110,
      B010,
      B010,
      B111
    },
    3,
  },
  {   // 2
    {
      B111,
      B001,
      B111,
      B100,
      B111
    },
    3,
  },
  {   // 3
    {
      B111,
      B001,
      B011,
      B001,
      B111
    },
    3,
  },
  {   // 4
    {
      B101,
      B101,
      B111,
      B001,
      B001
    },
    3,
  },
  {   // 5
    {
      B111,
      B100,
      B111,
      B001,
      B111
    },
    3,
  },
  {   // 6
    {
      B111,
      B100,
      B111,
      B101,
      B111
    },
    3,
  },
  {   // 7
    {
      B111,
      B001,
      B010,
      B100,
      B100
    },
    3,
  },
  {   // 8
    {
      B111,
      B101,
      B111,
      B101,
      B111
    },
    3,
  },
  {   // 9
    {
      B111,
      B101,
      B111,
      B001,
      B001
    },
    3,
  },
  {   // :
    {
      B0,
      B1,
      B0,
      B1,
      B0
    },
    1,
  },
  {   // ;
    {
      B000,
      B010,
      B000,
      B010,
      B100
    },
    3,
  },
  {   // <
    {
      B000,
      B010,
      B100,
      B010,
      B000
    },
    3,
  },
  {   // =
    {
      B000,
      B111,
      B000,
      B111,
      B000
    },
    3,
  },
  {   // >
    {
      B000,
      B010,
      B001,
      B010,
      B000
    },
    3,
  },
  {   // ?
    {
      B111,
      B001,
      B011,
      B000,
      B010
    },
    3,
  },
  {   // @
    {
      B111,
      B001,
      B111,
      B101,
      B111
    },
    3,
  },
  {   // A
    {
      B010,
      B101,
      B111,
      B101,
      B101
    },
    3,
  },
  {   // B
    {
      B110,
      B101,
      B110,
      B101,
      B110
    },
    3,
  },
  {   // C
    {
      B011,
      B100,
      B100,
      B100,
      B011
    },
    3,
  },
  {   // D
    {
      B110,
      B101,
      B101,
      B101,
      B110
    },
    3,
  },
  {   // E
    {
      B111,
      B100,
      B110,
      B100,
      B111
    },
    3,
  },
  {   // F
    {
      B111,
      B100,
      B110,
      B100,
      B100
    },
    3,
  },
  {   // G
    {
      B011,
      B100,
      B101,
      B101,
      B011
    },
    3,
  },
  {   // H
    {
      B101,
      B101,
      B111,
      B101,
      B101
    },
    3,
  },
  {   // I
    {
      B111,
      B010,
      B010,
      B010,
      B111
    },
    3,
  },
  {   // J
    {
      B001,
      B001,
      B001,
      B101,
      B111
    },
    3,
  },
  {   // K
    {
      B101,
      B101,
      B110,
      B101,
      B101
    },
    3,
  },
  {   // L
    {
      B100,
      B100,
      B100,
      B100,
      B111
    },
    3,
  },
  {   // M
    {
      B10001,
      B11011,
      B10101,
      B10001,
      B10001
    },
    5,
  },
  {   // N
    {
      B001,
      B101,
      B111,
      B101,
      B100
    },
    3,
  },
  {   // O
    {
      B111,
      B101,
      B101,
      B101,
      B111
    },
    3,
  },
  {   // P
    {
      B111,
      B101,
      B111,
      B100,
      B100
    },
    3,
  },
  {   // Q
    {
      B111,
      B101,
      B101,
      B111,
      B011
    },
    3,
  },
  {   // R
    {
      B111,
      B101,
      B111,
      B110,
      B101
    },
    3,
  },
  {   // S
    {
      B111,
      B100,
      B111,
      B001,
      B111
    },
    3,
  },
  {   // T
    {
      B111,
      B010,
      B010,
      B010,
      B010
    },
    3,
  },
  {   // U
    {
      B101,
      B101,
      B101,
      B101,
      B111
    },
    3,
  },
  {   // V
    {
      B101,
      B101,
      B101,
      B101,
      B010
    },
    3,
  },
  {   // W
    {
      B101,
      B101,
      B101,
      B111,
      B101
    },
    3,
  },
  {   // X
    {
      B101,
      B101,
      B010,
      B101,
      B101
    },
    3,
  },
  {   // Y
    {
      B101,
      B101,
      B010,
      B010,
      B010
    },
    3,
  },
  {   // Z
    {
      B111,
      B001,
      B010,
      B100,
      B111
    },
    3,
  },
};

#endif // characters_h