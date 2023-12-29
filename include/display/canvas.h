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
    Canvas(int width, int height);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getSize() const { return length; }
    std::size_t XYToIndex(int x, int y) const;

    flm::CRGB& operator[](std::size_t idx) { return data[idx]; }
    const flm::CRGB& operator[](std::size_t idx) const { return data[idx]; }

    /* Drawing Functions */
    void setXY(int x, int y, flm::CRGB colour);
    const flm::CRGB& getXY(int x, int y) const;
    void fill(const flm::CRGB& colour);

    bool containsColour(const flm::CRGB& colour = 0) const;

    void showCharacters(const std::string& string, const std::vector<flm::CRGB>& colours, int xOffset, uint8_t spacing = 0);
    void showCharacter(char character, flm::CRGB colour, int xOffset);
    void showCharacter(const FontGlyph& character, flm::CRGB colour, int xOffset);

private:
    int width;
    int height;
    int length;
    std::vector<flm::CRGB> data;
};

Canvas blit(const Canvas& background, const Canvas& foreground, int xOffset, int yOffset);

Canvas crop(const Canvas& input, int startX, int startY, int width, int height);

} // namespace canvas

#endif // canvas_h