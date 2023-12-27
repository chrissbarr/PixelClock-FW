#ifndef displayutilities_h
#define displayutilities_h

/* Project Scope */
#include <flm_pixeltypes.h>
#include <flm_lib8tion.h>

/* C++ Standard Library */
#include <functional>

float calculateBarHeight(float val, float valMin, float valMax, float barMax);

namespace colourGenerator {

using Generator = std::function<pixel::CRGB()>;

// Colour generating functions
inline Generator randomHSV = []() { return pixel::CHSV(pixel::random8(), 255, 255); };
inline Generator cycleHSV = []() { return pixel::CHSV((millis() / 10), 255, 255); };
inline Generator black = []() { return 0; };
inline Generator white = []() { return pixel::CRGB::White; };

} // namespace colourGenerator

#endif // displayutilities_h