#ifndef canvas_h
#define canvas_h

/* Project Scope */
#include "characters.h"
#include "flm_pixeltypes.h"

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

    pixel::CRGB& operator[](std::size_t idx) { return data[idx]; }
    const pixel::CRGB& operator[](std::size_t idx) const { return data[idx]; }

    /* Drawing Functions */
    void setXY(uint8_t x, uint8_t y, pixel::CRGB colour);
    const pixel::CRGB& getXY(uint8_t x, uint8_t y) const;
    void fill(const pixel::CRGB& colour);

    bool containsColour(const pixel::CRGB& colour = 0) const;

    void showCharacters(const std::string& string, const std::vector<pixel::CRGB>& colours, int xOffset, uint8_t spacing = 0);
    void showCharacter(char character, pixel::CRGB colour, int xOffset);
    void showCharacter(const FontGlyph& character, pixel::CRGB colour, int xOffset);

private:
    uint8_t width;
    uint8_t height;
    uint16_t length;
    std::vector<pixel::CRGB> data;
};

Canvas blit(const Canvas& background, const Canvas& foreground, int xOffset, int yOffset);

Canvas crop(const Canvas& input, int startX, int startY, int width, int height);

} // namespace canvas

#endif // canvas_h