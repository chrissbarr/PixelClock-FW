/* Project Scope */
#include "display/canvas.h"

/* Arduino Core */
#include <assert.h>

/* C++ Standard Library */
#include <vector>

namespace canvas {

Canvas::Canvas(int width, int height) : width(width), height(height), length(width * height) {
    data = std::vector<flm::CRGB>(length, flm::CRGB::Black);
}

void Canvas::setXY(int x, int y, flm::CRGB colour) { data[XYToIndex(x, y)] = colour; }

const flm::CRGB& Canvas::getXY(int x, int y) const { return data[XYToIndex(x, y)]; }

void Canvas::fill(const flm::CRGB& colour) {
    for (auto& v : data) { v = colour; }
}

std::size_t Canvas::XYToIndex(int x, int y) const {
    assert(((y * width) + x) < data.size());
    return (y * width) + x;
}

void Canvas::showCharacters(
    const std::string& string, const std::vector<flm::CRGB>& colours, int xOffset, uint8_t spacing) {
    int xOffsetLocal = 0;
    int colourIndex = 0;
    for (const auto& character : string) {
        FontGlyph g = characterFontArray[charToIndex(character)];
        showCharacter(g, colours[colourIndex], xOffset + xOffsetLocal);
        xOffsetLocal += g.width + spacing;
        colourIndex++;
        if (colourIndex >= colours.size()) { colourIndex = 0; }
        if (xOffset + xOffsetLocal > getWidth()) { break; }
    }
}

void Canvas::showCharacter(char character, flm::CRGB colour, int xOffset) {
    showCharacter(characterFontArray[charToIndex(character)], colour, xOffset);
}

void Canvas::showCharacter(const FontGlyph& character, flm::CRGB colour, int xOffset) {
    for (uint8_t x = 0; x < character.width; x++) {
        int xPos = xOffset + x;
        for (uint8_t y = 0; y < 5; y++) {
            if (bitRead(character.glyph[y], character.width - 1 - x) == 1) {
                if (xPos >= getWidth()) { continue; }
                if (xPos < 0) { continue; }
                setXY(xPos, y, colour);
            }
        }
    }
}

bool Canvas::containsColour(const flm::CRGB& colour) const {
    bool contains = false;
    for (std::size_t i = 0; i < getSize(); i++) {
        if (this->operator[](i) == colour) {
            contains = true;
            break;
        }
    }
    return contains;
}

Canvas blit(const Canvas& background, const Canvas& foreground, int xOffset, int yOffset) {

    // create new canvas to hold result
    Canvas c(background);

    for (int x = 0; x < c.getWidth(); x++) {
        for (int y = 0; y < c.getHeight(); y++) {
            int xInFore = x - xOffset;
            int yInFore = y - yOffset;
            if (xInFore >= 0 && xInFore < foreground.getWidth() && yInFore >= 0 && yInFore < foreground.getHeight()) {
                c.setXY(x, y, foreground.getXY(xInFore, yInFore));
            }
        }
    }

    return c;
}

Canvas crop(const Canvas& input, int startX, int startY, int width, int height) {

    // create new canvas to hold result
    Canvas c(width, height);

    // copy cropped region from input
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) { c.setXY(x, y, input.getXY(x + startX, y + startY)); }
    }

    return c;
}

} // namespace canvas