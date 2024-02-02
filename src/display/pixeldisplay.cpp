/* Project Scope */
#include "display/canvas.h"
#include "display/effects/textscroller.h"
#include "display/pixeldisplay.h"
#include "pinout.h"

#include "flm_pixeltypes.h"

/* Libraries */
#define FASTLED_ALL_PINS_HARDWARE_SPI
#include <FastLED.h>

/* Hack to enable SK6812 RGBW strips to work with FastLED.
 *
 * Original code by Jim Bumgardner (http://krazydad.com).
 * Modified by David Madison (http://partsnotincluded.com).
 */
constexpr uint16_t getRGBWsize(uint16_t nleds) {
    uint16_t nbytes = nleds * 4;
    if (nbytes % 3 > 0) {
        return nbytes / 3 + 1;
    } else {
        return nbytes / 3;
    }
}

PixelDisplay::PixelDisplay(uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset)
    : width(width),
      height(height),
      size(width * height),
      serpentine(serpentine),
      vertical(vertical),
      pixelOffset(pixelOffset) {

#ifdef MATRIX_TYPE_SK6812RGBW
    uint16_t dummyLEDCount = getRGBWsize(size);
    leds = (CRGB*)calloc(dummyLEDCount, sizeof(CRGB));
    FastLED.addLeds<WS2812, pins::matrixLEDData, RGB>(leds, dummyLEDCount);
#endif

#ifdef MATRIX_TYPE_APA102
    leds = (CRGB*)calloc(size, sizeof(CRGB));
    FastLED.addLeds<APA102HD, pins::matrixLEDData, pins::matrixLEDClock, BGR>(leds, size);
#endif
}

PixelDisplay::~PixelDisplay() { free(leds); }

void PixelDisplay::update(const canvas::Canvas& canvas) {

    traceUpdateTotal.start();
    // Serial.println("Update...");
    if (leds) {

        auto buff = std::vector<flm::CRGB>(size, 0);

        for (int x = 0; x < canvas.getWidth(); x++) {
            if (x >= getWidth()) { break; }
            for (int y = 0; y < canvas.getHeight(); y++) {
                if (y >= getHeight()) { break; }
                buff[XYToIndex(x, y)] = canvas.getXY(x, y);
            }
        }

        uint8_t* byteToWrite = (uint8_t*)leds;

        for (const flm::CRGB& pixel : buff) {
#ifdef MATRIX_TYPE_APA102
            *byteToWrite++ = pixel.red;
            *byteToWrite++ = pixel.green;
            *byteToWrite++ = pixel.blue;
#endif
#ifdef MATRIX_TYPE_SK6812RGBW
            *byteToWrite++ = pixel.green;
            *byteToWrite++ = pixel.red;
            *byteToWrite++ = pixel.blue;
            *byteToWrite++ = 0;
#endif
        }

        traceUpdateLEDWrite.start();
        FastLED.setBrightness(brightness);
        FastLED.setDither(1);
        FastLED.show();
        traceUpdateLEDWrite.stop();
    }

    traceUpdateTotal.stop();
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

std::vector<InstrumentationTrace*> PixelDisplay::getInstrumentation() {
    std::vector<InstrumentationTrace*> vec;
    vec.reserve(2);
    vec.push_back(&traceUpdateTotal);
    vec.push_back(&traceUpdateLEDWrite);
    return vec;
}