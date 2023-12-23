#ifndef display_h
#define display_h

/* Project Scope */
#include "display/fastled_rgbw.h"

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

using BufferType = std::vector<CRGB>;
//class FilterMethod;

class PixelDisplay {
public:
    PixelDisplay(uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset = 0);
    ~PixelDisplay();

    void setIndex(uint32_t index, CRGB colour);
    CRGB getIndex(uint32_t index) const;
    void setXY(uint8_t x, uint8_t y, CRGB colour);
    CRGB getXY(uint8_t x, uint8_t y) const;
    void fill(CRGB colour, const DisplayRegion& region);
    void fill(CRGB colour);

    //void showCharacters(const std::string& string, const std::vector<CRGB>& colours, int xOffset, uint8_t spacing = 0);
    //void showCharacter(char character, CRGB colour, int xOffset);
    //void showCharacter(const FontGlyph& character, CRGB colour, int xOffset);

    void setLEDStrip(CRGB* leds) { this->leds = leds; }
    void update();

    uint8_t getWidth() const { return width; }
    uint8_t getHeight() const { return height; }
    uint32_t getSize() const { return size; }

    //bool filled(CRGB colour, const DisplayRegion& region) const;
    //bool filled(CRGB colour = 0) const;
    //bool empty(const DisplayRegion& region) const;
    //bool empty() const;

    //const DisplayRegion& getFullDisplayRegion() const { return fullDisplay; }

    uint32_t XYToIndex(uint8_t x, uint8_t y) const;

    //BufferType& getFilterBuffer() { return filterBuffer; }
    //BufferType& getOutputBuffer() { return (filterApplied ? getFilterBuffer() : buffer); }

    //void applyFilter(const FilterMethod& filter);

private:
    CRGB* leds = nullptr;
    const uint8_t width;
    const uint8_t height;
    const uint32_t size;
    const bool serpentine;
    const bool vertical;
    const uint32_t pixelOffset;

    BufferType buffer;
};

#endif // display_h