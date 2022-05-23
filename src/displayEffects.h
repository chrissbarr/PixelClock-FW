#ifndef displayeffects_h
#define displayeffects_h

#include "display.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include <deque>

// Colour generating functions
inline uint32_t colourGenerator_randomHSV() { return Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(random(0, 65536))); }
inline uint32_t colourGenerator_cycleHSV() { return Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(millis(), 255, 255)); }
inline uint32_t colourGenerator_black() { return 0; }

template <uint32_t C> uint32_t colourGenerator_fixed() { return C; }

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
    TextScroller(PixelDisplay& display, const String& textString, uint32_t colour, uint16_t timeToHoldAtEnd = 1000, bool reverseOnFinish = false, uint8_t characterSpacing = 1);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final { _finished = false; currentOffset = 0; setTargetOffsetToEnd(); }
private:
    PixelDisplay& display;
    const String text;
    uint32_t colour;
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
    RandomFill(PixelDisplay& display, uint32_t fillInterval, uint32_t(*colourGenerator)(), const DisplayRegion& spawnRegion = defaultFull);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final { _finished = false; _lastSpawnTime = 0; _display.fill(0, _spawnRegion); };

private:
    PixelDisplay& _display;
    uint32_t _fillInterval;
    uint32_t (*_colourGenerator)();
    DisplayRegion _spawnRegion;
    bool _finished;
    uint32_t _lastSpawnTime = 0;
};

class BouncingBall : public DisplayEffect {
public:
    BouncingBall(PixelDisplay& display, float moveSpeed, uint32_t(*colourGenerator)(), const DisplayRegion& displayRegion = defaultFull);
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
    uint32_t (*_colourGenerator)();
    DisplayRegion _displayRegion;
    bool _finished;
    float _moveSpeed = 0;
};

class GameOfLife : public DisplayEffect {
public:
    GameOfLife(PixelDisplay& display, uint32_t updateInterval, uint32_t(*colourGenerator)(), const DisplayRegion& displayRegion = defaultFull);
    bool run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

    void seedDisplay();

private:
    PixelDisplay& _display;
    uint32_t _lastLoopTime;
    uint32_t (*_colourGenerator)();
    DisplayRegion _displayRegion;
    bool _finished;
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

void showTime(PixelDisplay& display, int hour, int minute, uint32_t colour);

void displayDiagnostic(PixelDisplay& display);

#endif //displayeffects_h