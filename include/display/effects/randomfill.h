#ifndef randomfill_h
#define randomfill_h

/* Project Scope */
#include "display/effects/effect.h"
#include <canvas.h>

/* Libraries */
#include <FastLED.h>

class RandomFill : public DisplayEffect {
public:
    RandomFill(const canvas::Canvas& size, uint32_t fillInterval, CRGB (*colourGenerator)());
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final {
        _finished = false;
        _lastSpawnTime = 0;
        _c.fill(0);
    };

    void setInput(const canvas::Canvas& c) { _c = c; }

private:
    uint32_t _fillInterval;
    CRGB (*_colourGenerator)();
    bool _finished;
    uint32_t _lastSpawnTime = 0;
    canvas::Canvas _c;
};

#endif // randomfill_h