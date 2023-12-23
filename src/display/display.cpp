/* Project Scope */
#include "display/display.h"
#include "display/displayEffects.h"

PixelDisplay::PixelDisplay(uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset)
    : width(width),
      height(height),
      size(width * height),
      serpentine(serpentine),
      vertical(vertical),
      pixelOffset(pixelOffset),
      buff(size, 0) {}

PixelDisplay::~PixelDisplay() {}

void PixelDisplay::update(const canvas::Canvas& canvas) {
    // Serial.println("Update...");
    if (leds) {

        for (int x = 0; x < canvas.getWidth(); x++) {
            if (x >= getWidth()) { break; }
            for (int y = 0; y < canvas.getHeight(); y++) {
                if (y >= getHeight()) { break; }
                buff[XYToIndex(x, y)] = canvas.getXY(x, y);
            }
        }

        uint8_t* byteToWrite = (uint8_t*)leds;

        for (const CRGB& pixel : buff) {
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