/* Project Scope */
#include "display/display.h"
//#include "display/characters.h"
#include "display/displayEffects.h"

PixelDisplay::PixelDisplay(uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset)
    : width(width),
      height(height),
      size(width * height),
      serpentine(serpentine),
      vertical(vertical),
      pixelOffset(pixelOffset) {
    buffer = BufferType(size, 0);
}

PixelDisplay::~PixelDisplay() {}

void PixelDisplay::setIndex(uint32_t index, CRGB colour) {
    if (index < size) { buffer[index] = colour; }
}

CRGB PixelDisplay::getIndex(uint32_t index) const {
    if (index < size) {
        return buffer[index];
    } else {
        return 0;
    }
}

void PixelDisplay::setXY(uint8_t x, uint8_t y, CRGB colour) { buffer[XYToIndex(x, y)] = colour; }

CRGB PixelDisplay::getXY(uint8_t x, uint8_t y) const { return buffer[XYToIndex(x, y)]; }

void PixelDisplay::fill(CRGB colour, const DisplayRegion& region) {
    for (uint8_t x = region.xMin; x <= region.xMax; x++) {
        for (uint8_t y = region.yMin; y <= region.yMax; y++) { setXY(x, y, colour); }
    }
}

void PixelDisplay::fill(CRGB colour) {
    for (uint32_t i = 0; i < size; i++) { buffer[i] = colour; }
}

void PixelDisplay::update() {
    // Serial.println("Update...");
    if (leds) {
        uint8_t* byteToWrite = (uint8_t*)leds;
        for (const CRGB& pixel : buffer) {
            // Serial.println("Writing pixel...");
            *byteToWrite++ = pixel.green;
            *byteToWrite++ = pixel.red;
            *byteToWrite++ = pixel.blue;
            *byteToWrite++ = 0;
        }
        FastLED.show();
    }
}

uint32_t PixelDisplay::XYToIndex(uint8_t x, uint8_t y) const {
    uint16_t i;

    if (serpentine == false) {
        if (vertical == false) {
            i = (y * width) + x;
        } else {
            i = height * (width - (x + 1)) + y;
        }
    }

    if (serpentine == true) {
        if (vertical == false) {
            if (y & 0x01) {
                // Odd rows run backwards
                uint8_t reverseX = (width - 1) - x;
                i = (y * width) + reverseX;
            } else {
                // Even rows run forwards
                i = (y * width) + x;
            }
        } else { // vertical positioning
            if (x & 0x01) {
                i = height * (width - (x + 1)) + y;
            } else {
                i = height * (width - x) - (y + 1);
            }
        }
    }

    return i;
}