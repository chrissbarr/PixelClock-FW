#include "canvas.h"
#include <unity.h>

using namespace canvas;

void setUp(void) {}

void tearDown(void) {}

void test_canvas_initialised_correctly(void) {

    Canvas c(10, 4);
    TEST_ASSERT_EQUAL(10, c.getWidth());
    TEST_ASSERT_EQUAL(4, c.getHeight());
    TEST_ASSERT_EQUAL(40, c.getLength());

    TEST_ASSERT_TRUE(c.getXY(0, 0) == CRGB::Black);
    TEST_ASSERT_FALSE(c.getXY(0, 0) == CRGB::Red);

    c.fill(CRGB::Red);

    TEST_ASSERT_TRUE(c.getXY(0, 0) == CRGB::Red);
    TEST_ASSERT_TRUE(c.getXY(1, 1) == CRGB::Red);

    c.setXY(1, 1, CRGB::Blue);
    TEST_ASSERT_TRUE(c.getXY(1, 1) == CRGB::Blue);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_canvas_initialised_correctly);
    UNITY_END();
    return 0;
}

// Satisfy FastLED external required function. Todo should be a better way.
uint16_t XY(uint8_t x, uint8_t y) { return 0; }