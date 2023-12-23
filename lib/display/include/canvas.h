#ifndef canvas_h
#define canvas_h

/* Project Scope */
#include "characters.h"

/* Libraries */
#include "FastLED.h"

/* C++ Standard Library */
#include <cstdint>
#include <string>
#include <vector>

namespace canvas {

class Canvas {
public:
    Canvas() : Canvas(0, 0) {}
    Canvas(uint8_t width, uint8_t height);

    uint8_t getWidth() const { return width; }
    uint8_t getHeight() const { return height; }
    uint16_t getSize() const { return length; }
    uint16_t XYToIndex(uint8_t x, uint8_t y) const;

    CRGB& operator[](std::size_t idx) { return data[idx]; }
    const CRGB& operator[](std::size_t idx) const { return data[idx]; }

    /* Drawing Functions */
    void setXY(uint8_t x, uint8_t y, CRGB colour);
    const CRGB& getXY(uint8_t x, uint8_t y) const;
    void fill(const CRGB& colour);

    bool containsColour(const CRGB& colour = 0) const;

    void showCharacters(const std::string& string, const std::vector<CRGB>& colours, int xOffset, uint8_t spacing = 0);
    void showCharacter(char character, CRGB colour, int xOffset);
    void showCharacter(const FontGlyph& character, CRGB colour, int xOffset);

private:
    uint8_t width;
    uint8_t height;
    uint16_t length;
    std::vector<CRGB> data;
};

Canvas blit(const Canvas& background, const Canvas& foreground, int xOffset, int yOffset);

} // namespace canvas

#endif // canvas_h