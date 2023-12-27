#ifndef spectrumdisplay_h
#define spectrumdisplay_h

/* Project Scope */
#include "display/canvas.h"
#include "display/effects/effect.h"

class SpectrumDisplay : public DisplayEffect {
public:
    SpectrumDisplay(const canvas::Canvas& size);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

private:
    canvas::Canvas _c;
    pixel::CRGB colMin;
    pixel::CRGB colMax;
    float maxScale = 5000;
    bool _finished = false;
};

#endif // spectrumdisplay_h