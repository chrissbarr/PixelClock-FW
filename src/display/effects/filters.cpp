/* Project Scope */
#include "display/effects/filters.h"

/* C++ Standard Library */
#include <cmath>

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
    for (auto& p : c) {
        if (p == CRGB(0)) { continue; }
        p = maintainBrightness ? CRGB(colour).nscale8(p.getAverageLight()) : colour;
    }
}

void RainbowWave::apply(canvas::Canvas& c) {

    position += speed;
    while (position >= 256) { position -= 256; }
    while (position < 0) { position += 256; }

    int _width = width;
    if (_width == 0) { _width = c.getWidth(); }
    float invWidth = 256.f / _width;

    for (int x = 0; x < c.getWidth(); x++) {
        for (int y = 0; y < c.getHeight(); y++) {

            auto index = c.XYToIndex(x, y);
            if (c[index] == CRGB(0)) { continue; }

            float pixelHue = position + ((direction == Direction::horizontal ? x : y) * invWidth);
            uint8_t hue = static_cast<uint8_t>(std::round(pixelHue));

            c[index] = CHSV(hue, 255, maintainBrightness ? c[index].getAverageLight() : 255);
        }
    }
}