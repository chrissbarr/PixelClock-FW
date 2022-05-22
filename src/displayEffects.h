#ifndef displayeffects_h
#define displayeffects_h

#include "display.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Colour generating functions
inline uint32_t colourGenerator_randomHSV() { return Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(random(0, 65536))); }
inline uint32_t colorGenerator_cycleHSV() { return Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(millis(), 255, 255)); }
inline uint32_t colourGenerator_black() { return 0; }

bool fillRandomly(PixelDisplay& display, uint32_t fillInterval, uint32_t(*colourGenerator)(), const DisplayRegion& spawnRegion);
inline bool fillRandomly(PixelDisplay& display, uint32_t fillInterval, uint32_t(*colourGenerator)())
{
  return fillRandomly(display, fillInterval, colourGenerator, display.getFullDisplayRegion());
}

bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)(), DisplayRegion displayRegion);
inline bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)())
{
  return gravityFill(display, fillInterval, moveInterval, empty, colourGenerator, display.getFullDisplayRegion());
}

void tetris(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval);

void showTime(PixelDisplay& display, int hour, int minute, uint32_t colour);

#endif //displayeffects_h