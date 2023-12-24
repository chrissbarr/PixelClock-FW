/* Project Scope */
#include "display/effects/gravityfill.h"
#include "display/effects/utilities.h"

/* C++ Standard Library */
#include <memory>

GravityFill::GravityFill(
    const canvas::Canvas& size,
    uint32_t fillInterval,
    uint32_t moveInterval,
    colourGenerator::Generator colourGenerator)
    : _c(size) {
    randomFill = std::make_unique<RandomFill>(size, fillInterval, colourGenerator);
    gravityEffect = std::make_unique<Gravity>(moveInterval, false, Gravity::Direction::down);
    reset();
}

canvas::Canvas GravityFill::run() {

    // Start with previous state
    gravityEffect->setInput(_c);

    // apply gravity effect
    _c = gravityEffect->run();

    if (gravityEffect->finished()) {
        // if gravity effect could detects no movable pixels, spawn new pixel.

        // crop to only top row, this is our spawn region
        auto topRow = canvas::crop(_c, 0, 0, _c.getWidth(), 1);
        randomFill->setInput(topRow);
        topRow = randomFill->run();
        _c = canvas::blit(_c, topRow, 0, 0);

        // unblock gravity effect so it can re-try next loop
        gravityEffect->reset();
    }

    // If there is no space left, effect is finished
    _finished = randomFill->finished();

    return _c;
}
