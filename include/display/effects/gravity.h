#ifndef gravity_h
#define gravity_h

/* Project Scope */
#include "display/canvas.h"
#include "display/effects/effect.h"

class Gravity : public DisplayEffect {
public:
    enum class Direction { up, down, left, right };

    Gravity(uint32_t moveInterval, bool empty, Gravity::Direction direction);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

    Direction getDirection() const { return _direction; }
    void setDirection(Direction direction) { _direction = direction; }
    void setFallOutOfScreen(bool enabled) { _empty = enabled; }
    bool getFallOutOfScreen() const { return _empty; }
    void setInput(const canvas::Canvas& c) { _c = c; }
    void setValidPixelsMask(const canvas::Canvas& c) { validPixelsMask = c; }

private:
    canvas::Canvas _c;
    canvas::Canvas validPixelsMask;
    bool _finished = false;
    uint32_t _moveInterval;
    uint32_t _lastMoveTime = 0;
    bool _empty;
    Direction _direction;
};

#endif // gravity_h