#ifndef bouncingball_h
#define bouncingball_h

/* Project Scope */
#include "display/effects/effect.h"
#include <canvas.h>

class BouncingBall : public DisplayEffect {
public:
    BouncingBall(const canvas::Canvas& size, uint32_t updateInterval, CRGB (*colourGenerator)());
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

private:
    canvas::Canvas _c;
    float ballx;
    float bally;
    int xDir;
    int yDir;
    uint32_t _lastLoopTime;
    uint32_t _updateInterval;
    CRGB (*_colourGenerator)();
    bool _finished;
};

#endif // bouncingball_h