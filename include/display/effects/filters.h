#ifndef filters_h
#define filters_h

/* Project Scope */
#include <canvas.h>

#include "flm_pixeltypes.h"

class FilterMethod {
public:
    virtual void apply(canvas::Canvas& c) = 0;
};

class HSVTestPattern : public FilterMethod {
public:
    HSVTestPattern(){};
    void apply(canvas::Canvas& c) override;
};

class SolidColour : public FilterMethod {
public:
    SolidColour(pixel::CRGB colour, bool maintainBrightness = true) : colour(colour), maintainBrightness(maintainBrightness) {}
    void apply(canvas::Canvas& c) override;

private:
    pixel::CRGB colour;
    bool maintainBrightness;
};

class RainbowWave : public FilterMethod {
public:
    enum Direction { horizontal, vertical };
    RainbowWave(float speed, int width, Direction direction = Direction::horizontal, bool maintainBrightness = true)
        : speed(speed),
          width(width),
          direction(direction),
          maintainBrightness(maintainBrightness) {}
    void apply(canvas::Canvas& c) override;

private:
    float speed;
    int width;
    bool maintainBrightness;
    Direction direction;
};

#endif // filters_h