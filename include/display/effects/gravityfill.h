#ifndef gravityfill_h
#define gravityfill_h

/* Project Scope */
#include "display/effects/effect.h"
#include "display/effects/gravity.h"
#include "display/effects/randomfill.h"
#include "display/effects/utilities.h"
#include <canvas.h>

/* Libraries */
#include <FastLED.h>

/* C++ Standard Library */
#include <memory>

class GravityFill : public DisplayEffect {
public:
    GravityFill(
        const canvas::Canvas& size,
        uint32_t fillInterval,
        uint32_t moveInterval,
        colourGenerator::Generator colourGenerator);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final {
        randomFill->reset();
        gravityEffect->reset();
        _finished = false;
        _c.fill(0);
    };

private:
    std::unique_ptr<RandomFill> randomFill;
    std::unique_ptr<Gravity> gravityEffect;

    colourGenerator::Generator _colourGenerator;
    bool _finished;

    canvas::Canvas _c;
};

#endif // gravityfill_h