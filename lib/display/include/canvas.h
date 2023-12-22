#ifndef canvas_h
#define canvas_h

/* Libraries */
#include "FastLED.h"

/* C++ Standard Library */
#include <cstdint>
#include <vector>

namespace canvas {

class Canvas {
public:
    Canvas(uint8_t width, uint8_t height);
    uint16_t XYToIndex(uint8_t x, uint8_t y) const;
    void setXY(uint8_t x, uint8_t y, CRGB colour);
    CRGB getXY(uint8_t x, uint8_t y);
    void fill(CRGB colour);

    uint8_t getWidth() const { return width; }
    uint8_t getHeight() const { return height; }
    uint16_t getLength() const { return length; }

private:
    const uint8_t width;
    const uint8_t height;
    const uint16_t length;
    std::vector<CRGB> data;
};

} // namespace canvas

#endif // canvas_h