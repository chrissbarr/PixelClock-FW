#ifndef display_h
#define display_h

/* Project Scope */
#include "display/fastled_rgbw.h"
#include <canvas.h>

/* Arduino Core */
#include <Arduino.h>

/* C++ Standard Library */
#include <string>
#include <vector>

struct DisplayRegion {
    uint8_t xMin;
    uint8_t xMax;
    uint8_t yMin;
    uint8_t yMax;
};

constexpr bool operator==(const DisplayRegion& lhs, const DisplayRegion& rhs) {
    return lhs.xMin == rhs.xMin && lhs.xMax == rhs.xMax && lhs.yMin == rhs.yMin && lhs.yMax == rhs.yMax;
}

// Set DisplayRegion arguments to this value to indicate that the full display should be used
constexpr DisplayRegion defaultFull = {0, 0, 0, 0};

class PixelDisplay {
public:
    PixelDisplay(uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset = 0);
    ~PixelDisplay();

    void setLEDStrip(CRGB* leds) { this->leds = leds; }
    void update(const canvas::Canvas& canvas);

    uint8_t getWidth() const { return width; }
    uint8_t getHeight() const { return height; }
    uint32_t getSize() const { return size; }

    uint32_t XYToIndex(uint8_t x, uint8_t y) const;

private:
    CRGB* leds = nullptr;
    const uint8_t width;
    const uint8_t height;
    const uint32_t size;
    const bool serpentine;
    const bool vertical;
    const uint32_t pixelOffset;
    std::vector<CRGB> buff;
};

#endif // display_h