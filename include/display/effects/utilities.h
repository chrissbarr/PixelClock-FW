#ifndef displayutilities_h
#define displayutilities_h

/* Libraries */
#include <FastLED.h>

/* C++ Standard Library */
#include <functional>

float calculateBarHeight(float val, float valMin, float valMax, float barMax);

namespace colourGenerator {

using Generator = std::function<CRGB()>;

// Colour generating functions
inline Generator randomHSV = []() { return CHSV(random8(), 255, 255); };
inline Generator cycleHSV = []() { return CHSV((millis() / 10), 255, 255); };
inline Generator black = []() { return 0; };
inline Generator white = []() { return CRGB::White; };

} // namespace colourGenerator

#endif // displayutilities_h