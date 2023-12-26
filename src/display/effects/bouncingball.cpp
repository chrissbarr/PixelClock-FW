/* Project Scope */
#include "display/effects/bouncingball.h"
#include "display/effects/utilities.h"

/* C++ Standard Library */
#include <random>

BouncingBall::BouncingBall(
    const canvas::Canvas& size, uint32_t updateInterval, colourGenerator::Generator colourGenerator)
    : _c(size),
      _updateInterval(updateInterval),
      _colourGenerator(colourGenerator) {
    reset();
    rand.seed(0);
}

void BouncingBall::reset() {
    const int spawnInFromBorder = 1;

    std::uniform_int_distribution<uint16_t> horDist(spawnInFromBorder, _c.getWidth() - 1 - spawnInFromBorder);
    std::uniform_int_distribution<uint16_t> vertDist(spawnInFromBorder, _c.getHeight() - 1 - spawnInFromBorder);

    ballx = horDist(rand);
    bally = vertDist(rand);
    xDir = 1;
    yDir = 1;
    _finished = false;
    _lastLoopTime = millis();
    _c.fill(pixel::CRGB::Black);
}

canvas::Canvas BouncingBall::run() {
    uint32_t millisSinceLastRun = millis() - _lastLoopTime;
    if (millisSinceLastRun > _updateInterval) {
        ballx += xDir;
        bally += yDir;

        if (ballx <= 0 || ballx >= _c.getWidth() - 1) { xDir = -xDir; }
        if (bally <= 0 || bally >= _c.getHeight() - 1) { yDir = -yDir; }

        _c.fill(pixel::CRGB::Black);
        uint8_t pixelx = uint8_t(round(ballx));
        uint8_t pixely = uint8_t(round(bally));
        // printing::print(Serial, fmt::format("Ball (x={:.2f}({}), y={:.2f}({}))\n", ballx, pixelx, bally, pixely));
        _c.setXY(pixelx, pixely, _colourGenerator());

        _lastLoopTime = millis();
    }
    return _c;
}