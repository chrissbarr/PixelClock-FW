#ifndef volumedisplay_h
#define volumedisplay_h

/* Project Scope */
#include "display/effects/effect.h"
#include <canvas.h>

/* Libraries */
//#include <FastLED.h>

/* C++ Standard Library */
#include <vector>

struct VolumeDisplayColourMap {
    double percentage;
    pixel::CRGB colour;
};

class VolumeDisplay : public DisplayEffect {
public:
    VolumeDisplay(const canvas::Canvas& size);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

private:
    canvas::Canvas _c;
    std::vector<VolumeDisplayColourMap> colourMap;
    bool _finished = false;
};

#endif // volumedisplay_h