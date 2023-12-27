/* Project Scope */
#include "display/pixeldisplay.h"
#include "display/effects/textscroller.h"
#include "pinout.h"
#include "display/canvas.h"

#include "flm_pixeltypes.h"


/* Libraries */
//#define FASTLED_NAMESPACE_BEGIN namespace NSFastLED {
//#define FASTLED_NAMESPACE_END }
//#define FASTLED_USING_NAMESPACE using namespace NSFastLED;
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

        uint16_t dummyLEDCount = getRGBWsize(size);
        leds = (CRGB*)calloc(dummyLEDCount, sizeof(CRGB));
        FastLED.addLeds<WS2812, pins::matrixLED, RGB>(leds, dummyLEDCount);

      }

PixelDisplay::~PixelDisplay() {
    free(leds);
}

void PixelDisplay::update(const canvas::Canvas& canvas) {
    // Serial.println("Update...");
    if (leds) {

        auto buff = std::vector<pixel::CRGB>(size, 0);

        for (int x = 0; x < canvas.getWidth(); x++) {
            if (x >= getWidth()) { break; }
            for (int y = 0; y < canvas.getHeight(); y++) {
                if (y >= getHeight()) { break; }
                buff[XYToIndex(x, y)] = canvas.getXY(x, y);
            }
        }

        uint8_t* byteToWrite = (uint8_t*)leds;

        for (const pixel::CRGB& pixel : buff) {
            // Serial.println("Writing pixel...");
            *byteToWrite++ = pixel.green;
            *byteToWrite++ = pixel.red;
            *byteToWrite++ = pixel.blue;
            *byteToWrite++ = 0;
        }

        FastLED.setBrightness(brightness);
        FastLED.setDither(1);
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

void displayDiagnostic(PixelDisplay& display) {
    // Clear display
    canvas::Canvas c(display.getWidth(), display.getHeight());
    c.fill(0);
    display.update(c);
    delay(250);

    // Show Pixel 0
    c.setXY(0, 0, pixel::CRGB(255, 0, 0));
    display.update(c);
    delay(250);

    // Solid Red, Green, Blue
    c.fill(pixel::CRGB(255, 0, 0));
    display.update(c);
    delay(250);
    c.fill(pixel::CRGB(0, 255, 0));
    display.update(c);
    delay(250);
    c.fill(pixel::CRGB(0, 0, 255));
    display.update(c);
    delay(250);

    // Move through XY sequentially
    for (uint8_t y = 0; y < c.getHeight(); y++) {
        for (uint8_t x = 0; x < c.getWidth(); x++) {
            c.fill(0);
            c.setXY(x, y, pixel::CRGB(100, 0, 0));
            display.update(c);
            delay(1);
        }
    }

    // Scroll short test
    c.fill(0);
    display.update(c);
    auto textScrollTest1 = RepeatingTextScroller(c, "Hello - Testing!", std::vector<pixel::CRGB>{pixel::CRGB(0, 0, 255)}, 50, 500, 1);
    while (!textScrollTest1.finished()) {
        display.update(textScrollTest1.run());
        delay(1);
    }

    // Scroll full character set
    c.fill(0);
    display.update(c);
    auto textScrollTest = RepeatingTextScroller(
        c,
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 !\"#$%&'()*+'-./:;<=>?@",
        std::vector<pixel::CRGB>{pixel::CRGB(0, 255, 0)},
        50,
        500,
        1);
    while (!textScrollTest.finished()) {
        display.update(textScrollTest.run());
        delay(1);
    }
    c.fill(0);
    display.update(c);
}
