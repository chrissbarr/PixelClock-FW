/* Project Scope */
#include "display/effects/filters.h"

/* Libraries */
//#include <FastLED.h>

using namespace pixel;

void HSVTestPattern::apply(canvas::Canvas& c) {
    for (uint8_t x = 0; x < c.getWidth(); x++) {
        for (uint8_t y = 0; y < c.getHeight(); y++) {
            c.setXY(x, y, CHSV(x * (255 / c.getWidth()), 255, y * (255 / c.getHeight())));
        }
    }
}

void SolidColour::apply(canvas::Canvas& c) {
    for (std::size_t i = 0; i < c.getSize(); i++) {
        if (c[i] == CRGB(0)) { continue; }
        c[i] = maintainBrightness ? CRGB(colour).nscale8(c[i].getAverageLight()) : colour;
    }
}

void RainbowWave::apply(canvas::Canvas& c) {
    static float wheelPos = 0;
    wheelPos += speed;
    int _width = width;
    if (_width == 0) { _width = c.getWidth(); }

    for (uint8_t x = 0; x < c.getWidth(); x++) {
        for (uint8_t y = 0; y < c.getHeight(); y++) {
            uint8_t hue = uint8_t(round(wheelPos)) + ((direction == Direction::horizontal ? x : y) * 256 / _width);
            auto index = c.XYToIndex(x, y);
            if (c[index] == CRGB(0)) { continue; }
            c[index] = CHSV(hue, 255, maintainBrightness ? c[index].getAverageLight() : 255);
        }
    }
}