#ifndef displayutilities_h
#define displayutilities_h

/* Project Scope */
#include <flm_lib8tion.h>
#include <flm_pixeltypes.h>

/* C++ Standard Library */
#include <functional>

float calculateBarHeight(float val, float valMin, float valMax, float barMax);

namespace colourGenerator {

using Generator = std::function<flm::CRGB()>;

// Colour generating functions
inline Generator randomHSV = []() { return flm::CHSV(flm::random8(), 255, 255); };
inline Generator cycleHSV = []() { return flm::CHSV(static_cast<uint8_t>(millis() / 10), 255, 255); };
inline Generator black = []() { return 0; };
inline Generator white = []() { return flm::CRGB::White; };

} // namespace colourGenerator

#endif // displayutilities_h