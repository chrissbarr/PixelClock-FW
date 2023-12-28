#ifndef bouncingball_h
#define bouncingball_h

/* Project Scope */
#include "display/canvas.h"
#include "display/effects/effect.h"
#include "display/effects/utilities.h"

/* C++ Standard Library */
#include <random>

class BouncingBall : public DisplayEffect {
public:
    BouncingBall(const canvas::Canvas& size, uint32_t updateInterval, colourGenerator::Generator colourGenerator);
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
    colourGenerator::Generator _colourGenerator;
    bool _finished;
    std::minstd_rand rand;
};

#endif // bouncingball_h