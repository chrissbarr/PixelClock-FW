#ifndef displayeffects_h
#define displayeffects_h

#include "timekeeping.h"
#include "display/display.h"
#include <Arduino.h>
#include "display/fastled_rgbw.h"

#include <deque>
#include <memory>
#include <set>

// Colour generating functions
inline CRGB colourGenerator_randomHSV() { return CHSV(random8(), 255, 255); }
inline CRGB colourGenerator_cycleHSV() { return CHSV((millis() / 10), 255, 255); }
inline CRGB colourGenerator_black() { return 0; }
inline CRGB colourGenerator_white() { return CRGB::White; }

/**
 * @brief Abstract Base Class for classes that implement an Effect (e.g., a pattern, demo, game, etc. that takes place over time)
 * 
 * Effects are expected to manage their own state and timing.
 * Effects may run for some period of time. 
 * When an effect has completed one 'cycle' (the definition of which will vary per effect) it will indicate that it is 'finished'.
 * However, the effect should still continue to run (loop, restart, etc.) even if it is 'finished'.
 * The 'finished' flag is intended to be a way to indicate to the caller that this is an appropriate time to move on to another effect.
 */
class DisplayEffect {
public:
    // Runs the effect. Returns true if the effect is considered finished.
    virtual bool run() = 0;
    // Indicates if the effect is finished.
    virtual bool finished() const = 0;
    // Resets the effect to it's initial state
    virtual void reset() = 0;
};

class DisplayEffectDecorator : public DisplayEffect {
protected:
    std::shared_ptr<DisplayEffect> effect;
public:
    DisplayEffectDecorator(std::shared_ptr<DisplayEffect> effect) : effect(effect) {}
    bool run() { return effect->run(); }
    bool finished() const { return effect->finished(); }
    void reset() { effect->reset(); }
};

class EffectDecorator_Timeout : public DisplayEffectDecorator {
public:
    EffectDecorator_Timeout(std::shared_ptr<DisplayEffect> effect, uint32_t timeout) : DisplayEffectDecorator(effect), timeoutDuration(timeout) {}
    bool run() 
    { 
        effect->run(); 
        return finished();
    }
    bool finished() const 
    { 
        if (millis() - lastResetTime > timeoutDuration) {
            return true;
        }
        return effect->finished(); 
    }
    void reset() 
    { 
        lastResetTime = millis();
        effect->reset();
    }
private:
    uint32_t lastResetTime = 0;
    uint32_t timeoutDuration;
};

class TextScroller : public DisplayEffect {
public:
    TextScroller(PixelDisplay& display, String textString, CRGB colour, uint16_t stepDelay = 100, uint16_t timeToHoldAtEnd = 1000, bool reverseOnFinish = false, uint8_t characterSpacing = 1);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final { _finished = false; currentOffset = 0; setTargetOffsetToEnd(); }

    void setText(const String& textIn) { text = textIn; }
private:
    PixelDisplay& display;
    String text;
    CRGB colour;
    uint16_t timeToHoldAtEnd;
    bool reverseOnFinish;
    uint8_t charSpacing;

    uint32_t targetOffset;
    uint32_t currentOffset;
    uint32_t lastUpdateTime;

    uint32_t stepDelay;
    uint32_t arrivedAtEndTime = 0;

    bool _finished = false;

    void setTargetOffsetToEnd();
};

class RandomFill : public DisplayEffect {
public:
    RandomFill(PixelDisplay& display, uint32_t fillInterval, CRGB(*colourGenerator)(), const DisplayRegion& spawnRegion = defaultFull);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final { _finished = false; _lastSpawnTime = 0; _display.fill(0, _spawnRegion); };

private:
    PixelDisplay& _display;
    uint32_t _fillInterval;
    CRGB (*_colourGenerator)();
    DisplayRegion _spawnRegion;
    bool _finished;
    uint32_t _lastSpawnTime = 0;
};

class BouncingBall : public DisplayEffect {
public:
    BouncingBall(PixelDisplay& display, uint32_t updateInterval, CRGB(*colourGenerator)(), const DisplayRegion& displayRegion = defaultFull);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

private:
    float ballx;
    float bally;
    int xDir;
    int yDir;
    PixelDisplay& _display;
    uint32_t _lastLoopTime;
    uint32_t _updateInterval;
    CRGB (*_colourGenerator)();
    DisplayRegion _displayRegion;
    bool _finished;
};

class ClockFace : public DisplayEffect {
public:
    ClockFace(PixelDisplay& display, ClockFaceTimeStruct (*timeCallbackFunction)()) : _display(display), timeCallbackFunction(timeCallbackFunction) {}
    bool run() override final;
    bool finished() const override final { return false; }
    void reset() override final { };

private:
    PixelDisplay& _display;
    ClockFaceTimeStruct (*timeCallbackFunction)();
};

class Gravity : public DisplayEffect {
public:

    enum class Direction {
        up, 
        down,
        left,
        right
    };

    Gravity(PixelDisplay& display, uint32_t moveInterval, bool empty, Gravity::Direction direction, const DisplayRegion& displayRegion = defaultFull);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

    Direction getDirection() const { return _direction; }
    void setDirection(Direction direction) { _direction = direction; }
private:
    PixelDisplay& _display;
    DisplayRegion _displayRegion;
    bool _finished = false;
    uint32_t _moveInterval;
    uint32_t _lastMoveTime = 0;
    bool _empty;
    Direction _direction;
};

// bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)(), DisplayRegion displayRegion);
// inline bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)())
// {
//   return gravityFill(display, fillInterval, moveInterval, empty, colourGenerator, display.getFullDisplayRegion());
// }

// void tetris(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval);

void displayDiagnostic(PixelDisplay& display);

class FilterMethod {
public:
    virtual void apply(PixelDisplay& display) const = 0;
};

class HSVTestPattern : public FilterMethod {
public:
    HSVTestPattern() {};
    void apply(PixelDisplay& display) const override;
};

class SolidColour : public FilterMethod {
public:
    SolidColour(CRGB colour, bool maintainBrightness = true) : colour(colour), maintainBrightness(maintainBrightness) {}
    void apply(PixelDisplay& display) const override;
private:
    CRGB colour;
    bool maintainBrightness;
};

class RainbowWave : public FilterMethod {
public:
    enum Direction {
        horizontal,
        vertical
    };
    RainbowWave(float speed, int width, Direction direction = Direction::horizontal, bool maintainBrightness = true) : speed(speed), width(width), direction(direction), maintainBrightness(maintainBrightness) {}
    void apply(PixelDisplay& display) const override;

private:
    float speed;
    int width;
    bool maintainBrightness;
    Direction direction;
};

#endif //displayeffects_h