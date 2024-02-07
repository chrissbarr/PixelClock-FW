/* Project Scope */
#include "display/canvas.h"

/* Libraries */
#include <gtest/gtest.h>

using namespace canvas;

void setUp(void) {}

void tearDown(void) {}

TEST(CanvasTestSuite, CanvasBasics) {

    Canvas c(10, 4);
    EXPECT_EQ(10, c.getWidth());
    EXPECT_EQ(4, c.getHeight());
    EXPECT_EQ(40, c.getSize());

    EXPECT_EQ(c.getXY(0, 0), flm::CRGB::Black);
    EXPECT_NE(c.getXY(0, 0), flm::CRGB::Red);

    c.fill(flm::CRGB::Red);

    EXPECT_EQ(c.getXY(0, 0), flm::CRGB::Red);
    EXPECT_EQ(c.getXY(1, 1), flm::CRGB::Red);

    c.setXY(1, 1, flm::CRGB::Blue);
    EXPECT_EQ(c.getXY(1, 1), flm::CRGB::Blue);
}
