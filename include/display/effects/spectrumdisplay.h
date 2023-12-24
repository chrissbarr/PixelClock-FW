#ifndef spectrumdisplay_h
#define spectrumdisplay_h

/* Project Scope */
#include "display/effects/effect.h"
#include <canvas.h>

/* Libraries */
#include <FastLED.h>

class SpectrumDisplay : public DisplayEffect {
public:
    SpectrumDisplay(const canvas::Canvas& size);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

private:
    canvas::Canvas _c;
    CRGB colMin;
    CRGB colMax;
    float maxScale = 5000;
    bool _finished = false;
};

#endif // spectrumdisplay_h