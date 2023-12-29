/* Project Scope */
#include "display/effects/filters.h"

using namespace flm;

void HSVTestPattern::apply(canvas::Canvas& c) {
    for (int x = 0; x < c.getWidth(); x++) {
        for (int y = 0; y < c.getHeight(); y++) {
            auto colour = CHSV(
                static_cast<uint8_t>(x * (255 / c.getWidth())),
                static_cast<uint8_t>(255),
                static_cast<uint8_t>(y * (255 / c.getHeight())));
            c.setXY(x, y, colour);
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

    for (int x = 0; x < c.getWidth(); x++) {
        for (int y = 0; y < c.getHeight(); y++) {
            uint8_t hue =
                uint8_t(round(wheelPos)) + uint8_t((direction == Direction::horizontal ? x : y) * 256 / _width);
            auto index = c.XYToIndex(x, y);
            if (c[index] == CRGB(0)) { continue; }
            c[index] = CHSV(hue, 255, maintainBrightness ? c[index].getAverageLight() : 255);
        }
    }
}