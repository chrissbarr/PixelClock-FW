/* Project Scope */
#include "canvas.h"

/* C++ Standard Library */
#include <vector>

namespace canvas {

Canvas::Canvas(uint8_t width, uint8_t height) : width(width), height(height), length(width * height) {
    data = std::vector<CRGB>(length, CRGB::Black);
}

void Canvas::setXY(uint8_t x, uint8_t y, CRGB colour) { data[XYToIndex(x, y)] = colour; }
CRGB Canvas::getXY(uint8_t x, uint8_t y) { return data[XYToIndex(x, y)]; }

void Canvas::fill(CRGB colour) {
    for (auto& v : data) { v = colour; }
}

uint16_t Canvas::XYToIndex(uint8_t x, uint8_t y) const { return (y * width) + x; }

} // namespace canvas