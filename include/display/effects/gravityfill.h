#ifndef gravityfill_h
#define gravityfill_h

/* Project Scope */
#include "display/canvas.h"
#include "display/effects/effect.h"
#include "display/effects/gravity.h"
#include "display/effects/randomfill.h"
#include "display/effects/utilities.h"

/* C++ Standard Library */
#include <memory>
#include <random>

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

class GravityFillTemplate : public DisplayEffect {
public:
    enum class FillMode { random, leftRightPerCol, leftRightPerRow };

    GravityFillTemplate(FillMode fillMode);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;
    void setTemplate(const canvas::Canvas& c) { templateCanvas = c; }

private:
    std::unique_ptr<Gravity> gravityEffect;

    bool _finished;

    canvas::Canvas _c;
    canvas::Canvas templateCanvas;

    enum class State { empty, filling, stable };
    State currentState = State::empty;

    FillMode fillMode{FillMode::leftRightPerCol};
    int spawnCol{0};
    int spawnRow{0};
    int spawnColDir = 1;

    std::minstd_rand rand;
};

#endif // gravityfill_h