/* Project Scope */
#include "display/display.h"
#include "display/effects/textscroller.h"
#include <canvas.h>

/* Libraries */
#include <FastLED.h>

PixelDisplay::PixelDisplay(uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset)
    : width(width),
      height(height),
      size(width * height),
      serpentine(serpentine),
      vertical(vertical),
      pixelOffset(pixelOffset) {}

PixelDisplay::~PixelDisplay() {}

void PixelDisplay::update(const canvas::Canvas& canvas) {
    // Serial.println("Update...");
    if (leds) {

        auto buff = std::vector<CRGB>(size, 0);

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

void displayDiagnostic(PixelDisplay& display) {
    // Clear display
    canvas::Canvas c(display.getWidth(), display.getHeight());
    c.fill(0);
    display.update(c);
    delay(250);

    // Show Pixel 0
    c.setXY(0, 0, CRGB(255, 0, 0));
    display.update(c);
    delay(250);

    // Solid Red, Green, Blue
    c.fill(CRGB(255, 0, 0));
    display.update(c);
    delay(250);
    c.fill(CRGB(0, 255, 0));
    display.update(c);
    delay(250);
    c.fill(CRGB(0, 0, 255));
    display.update(c);
    delay(250);

    // Move through XY sequentially
    for (uint8_t y = 0; y < c.getHeight(); y++) {
        for (uint8_t x = 0; x < c.getWidth(); x++) {
            c.fill(0);
            c.setXY(x, y, CRGB(100, 0, 0));
            display.update(c);
            delay(1);
        }
    }

    // Scroll short test
    c.fill(0);
    display.update(c);
    auto textScrollTest1 = RepeatingTextScroller(c, "Hello - Testing!", std::vector<CRGB>{CRGB(0, 0, 255)}, 50, 500, 1);
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
        std::vector<CRGB>{CRGB(0, 255, 0)},
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
