/* FastLED_RGBW
 *
 * Hack to enable SK6812 RGBW strips to work with FastLED.
 *
 * Original code by Jim Bumgardner (http://krazydad.com).
 * Modified by David Madison (http://partsnotincluded.com).
 *
 */

#ifndef FastLED_RGBW_h
#define FastLED_RGBW_h

#define FASTLED_INTERNAL
#include <Arduino.h>
#include <FastLED.h>

constexpr uint16_t getRGBWsize(uint16_t nleds) {
    uint16_t nbytes = nleds * 4;
    if (nbytes % 3 > 0) {
        return nbytes / 3 + 1;
    } else {
        return nbytes / 3;
    }
}

#endif