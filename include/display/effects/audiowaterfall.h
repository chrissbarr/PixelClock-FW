#ifndef audiowaterfall_h
#define audiowaterfall_h

/* Project Scope */
#include "display/effects/effect.h"
#include <canvas.h>

class AudioWaterfall : public DisplayEffect {
public:
    AudioWaterfall(const canvas::Canvas& size);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

private:
    canvas::Canvas _c;
    bool _finished = false;
};

#endif // audiowaterfall_h