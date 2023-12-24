/* Project Scope */
#include "display/effects/randomfill.h"

/* Arduino Core */
#include <Arduino.h>

/* C++ Standard Library */
#include <random>

RandomFill::RandomFill(const canvas::Canvas& size, uint32_t fillInterval, CRGB (*colourGenerator)())
    : _c(size),
      _fillInterval(fillInterval),
      _colourGenerator(colourGenerator) {
    reset();
}

canvas::Canvas RandomFill::run() {
    uint32_t timeNow = millis();

    if (timeNow - _lastSpawnTime >= _fillInterval) {
        if (_c.containsColour(CRGB::Black)) {
            bool filledPixel = false;
            while (!filledPixel) {
                uint8_t x = random(0, _c.getWidth());
                uint8_t y = random(0, _c.getHeight());
                if (_c.getXY(x, y) == CRGB::Black) {
                    _c.setXY(x, y, _colourGenerator());
                    filledPixel = true;
                    _lastSpawnTime = timeNow;
                }
            }
        } else {
            _finished = true;
        }
    }
    return _c;
}