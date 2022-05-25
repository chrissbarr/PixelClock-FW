#ifndef displayeffects_h
#define displayeffects_h

#include "display.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <FastLED.h>

#include <deque>

// Colour generating functions
inline CRGB colourGenerator_randomHSV() { return CHSV(random8(), 255, 255); }
inline CRGB colourGenerator_cycleHSV() { return CHSV((millis() / 10), 255, 255); }
inline CRGB colourGenerator_black() { return 0; }

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

class TextScroller : public DisplayEffect {
public:
    TextScroller(PixelDisplay& display, const String& textString, CRGB colour, uint16_t timeToHoldAtEnd = 1000, bool reverseOnFinish = false, uint8_t characterSpacing = 1);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final { _finished = false; currentOffset = 0; setTargetOffsetToEnd(); }
private:
    PixelDisplay& display;
    const String text;
    CRGB colour;
    uint16_t timeToHoldAtEnd;
    bool reverseOnFinish;
    uint8_t charSpacing;

    uint32_t targetOffset;
    uint32_t currentOffset;
    uint32_t lastUpdateTime;

    uint32_t stepDelay = 100;
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

class GameOfLife : public DisplayEffect {
public:
    GameOfLife(PixelDisplay& display, uint32_t updateInterval, CRGB(*colourGenerator)(), const DisplayRegion& displayRegion = defaultFull, bool wrap = true);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

    void seedDisplay();

private:
    PixelDisplay& _display;
    uint32_t _lastLoopTime;
    CRGB (*_colourGenerator)();
    DisplayRegion _displayRegion;
    bool _dead;
    bool _finished;
    bool _wrap;
    uint32_t _updateInterval;
    uint32_t _notUniqueForNSteps;

    std::vector<uint32_t> nextBuffer;
    std::deque<std::size_t> bufferHashes;

    std::size_t hashBuffer(const std::vector<uint32_t>& vec) const; 
};

// bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)(), DisplayRegion displayRegion);
// inline bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)())
// {
//   return gravityFill(display, fillInterval, moveInterval, empty, colourGenerator, display.getFullDisplayRegion());
// }

// void tetris(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval);

void showTime(PixelDisplay& display, int hour, int minute, CRGB colour);

void displayDiagnostic(PixelDisplay& display);

class FilterMethod {
public:
    virtual void apply(PixelDisplay& display) const = 0;
};

class SolidColour : public FilterMethod {
public:
    SolidColour(CRGB colour) : colour(colour) {}
    void apply(PixelDisplay& display) const;
private:
    CRGB colour;
};

class RainbowWave : public FilterMethod {
public:
    RainbowWave(float speed, int width) : speed(speed), width(width) {}
    void apply(PixelDisplay& display) const;
private:
    float speed;
    int width;
};

void filterSolidColour(PixelDisplay& display, CRGB colour);
void filterRainbowWave(PixelDisplay& display, int speed, int width = 0);

#endif //displayeffects_h