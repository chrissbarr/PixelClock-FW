#ifndef display_h
#define display_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "characters.h"

#include <vector>

struct DisplayRegion {
    uint8_t xMin;
    uint8_t xMax;
    uint8_t yMin;
    uint8_t yMax;
};

constexpr bool operator==(const DisplayRegion& lhs, const DisplayRegion& rhs)
{
    return lhs.xMin == rhs.xMin
    && lhs.xMax == rhs.xMax
    && lhs.yMin == rhs.yMin
    && lhs.yMax == rhs.yMax;
}

// Set DisplayRegion arguments to this value to indicate that the full display should be used
constexpr DisplayRegion defaultFull = {0, 0, 0, 0};

class PixelDisplay {
public:
    PixelDisplay(Adafruit_NeoPixel& pixels, uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset = 0);
    ~PixelDisplay();

    void setIndex(uint32_t index, uint32_t colour);
    uint32_t getIndex(uint32_t index) const;
    void setXY(uint8_t x, uint8_t y, uint32_t colour);
    uint32_t getXY(uint8_t x, uint8_t y) const;
    void fill(uint32_t colour, const DisplayRegion& region);
    void fill(uint32_t colour);

    void showCharacters(const String& string, uint32_t colour, int xOffset, uint8_t spacing = 0);
    void showCharacter(char character, uint32_t colour, int xOffset);
    void showCharacter(const FontGlyph& character, uint32_t colour, int xOffset);

    void update();

    uint8_t getWidth() const { return width; }
    uint8_t getHeight() const { return height; }
    uint32_t getSize() const { return size; }

    bool filled(uint32_t colour, const DisplayRegion& region) const;
    bool filled(uint32_t colour = 0) const;
    bool empty(const DisplayRegion& region) const;
    bool empty() const;

    const DisplayRegion& getFullDisplayRegion() const { return fullDisplay; }

    uint32_t XYToIndex(uint8_t x, uint8_t y) const;

    void preFilter();
    std::vector<uint32_t>& getFilterBuffer() { return filterBuffer; }

private:
    Adafruit_NeoPixel& pixels;
    const uint8_t width;
    const uint8_t height;
    const uint32_t size;
    const bool serpentine;
    const bool vertical;
    const uint32_t pixelOffset;

    std::vector<uint32_t> buffer;
    std::vector<uint32_t> filterBuffer;
    bool filterApplied = false;

    DisplayRegion fullDisplay;

    
};

#endif // display_h