/* Project Scope */
#include "display/diagnostic.h"
#include "display/canvas.h"
#include "display/effects/textscroller.h"
#include "flm_pixeltypes.h"

void displayDiagnostic(Display& display) {
    // Clear display
    canvas::Canvas c(display.getWidth(), display.getHeight());
    c.fill(0);
    display.update(c);
    delay(250);

    // Show Pixel 0
    c.setXY(0, 0, flm::CRGB(255, 0, 0));
    display.update(c);
    delay(250);

    // Solid Red, Green, Blue
    c.fill(flm::CRGB(255, 0, 0));
    display.update(c);
    delay(250);
    c.fill(flm::CRGB(0, 255, 0));
    display.update(c);
    delay(250);
    c.fill(flm::CRGB(0, 0, 255));
    display.update(c);
    delay(250);

    // Move through XY sequentially
    for (uint8_t y = 0; y < c.getHeight(); y++) {
        for (uint8_t x = 0; x < c.getWidth(); x++) {
            c.fill(0);
            c.setXY(x, y, flm::CRGB(100, 0, 0));
            display.update(c);
            delay(1);
        }
    }

    // Scroll short test
    c.fill(0);
    display.update(c);
    auto textScrollTest1 =
        RepeatingTextScroller(c, "Hello - Testing!", std::vector<flm::CRGB>{flm::CRGB(0, 0, 255)}, 50, 500, 1);
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
        std::vector<flm::CRGB>{flm::CRGB(0, 255, 0)},
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