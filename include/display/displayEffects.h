#ifndef displayeffects_h
#define displayeffects_h

/* Project Scope */
#include <canvas.h>
// #include "display/display.h"
#include "display/fastled_rgbw.h"
#include "timekeeping.h"

/* Arduino Core */
#include <Arduino.h>

/* C++ Standard Library */
#include <deque>
#include <memory>
#include <set>

// Colour generating functions
inline CRGB colourGenerator_randomHSV() { return CHSV(random8(), 255, 255); }
inline CRGB colourGenerator_cycleHSV() { return CHSV((millis() / 10), 255, 255); }
inline CRGB colourGenerator_black() { return 0; }
inline CRGB colourGenerator_white() { return CRGB::White; }

/**
 * @brief Abstract Base Class for classes that implement an Effect (e.g., a pattern, demo, game, etc. that takes place
 * over time)
 *
 * Effects are expected to manage their own state and timing.
 * Effects may run for some period of time.
 * When an effect has completed one 'cycle' (the definition of which will vary per effect) it will indicate that it is
 * 'finished'. However, the effect should still continue to run (loop, restart, etc.) even if it is 'finished'. The
 * 'finished' flag is intended to be a way to indicate to the caller that this is an appropriate time to move on to
 * another effect.
 */
class DisplayEffect {
public:
    // Runs the effect. Returns true if the effect is considered finished.
    virtual canvas::Canvas run() = 0;
    // Indicates if the effect is finished.
    virtual bool finished() const = 0;
    // Resets the effect to it's initial state
    virtual void reset() = 0;
};

// class DisplayEffectDecorator : public DisplayEffect {
// protected:
//     std::shared_ptr<DisplayEffect> effect;

// public:
//     DisplayEffectDecorator(std::shared_ptr<DisplayEffect> effect) : effect(effect) {}
//     bool run() { return effect->run(); }
//     bool finished() const { return effect->finished(); }
//     void reset() { effect->reset(); }
// };

// class EffectDecorator_Timeout : public DisplayEffectDecorator {
// public:
//     EffectDecorator_Timeout(std::shared_ptr<DisplayEffect> effect, uint32_t timeout)
//         : DisplayEffectDecorator(effect),
//           timeoutDuration(timeout) {}
//     bool run() {
//         effect->run();
//         return finished();
//     }
//     bool finished() const {
//         if (millis() - lastResetTime > timeoutDuration) { return true; }
//         return effect->finished();
//     }
//     void reset() {
//         lastResetTime = millis();
//         effect->reset();
//     }

// private:
//     uint32_t lastResetTime = 0;
//     uint32_t timeoutDuration;
// };

// class TextScroller : public DisplayEffect {
// public:
//     TextScroller(
//         PixelDisplay& display,
//         std::string textString,
//         std::vector<CRGB> colours,
//         uint16_t stepDelay = 100,
//         uint16_t timeToHoldAtEnd = 1000,
//         uint8_t characterSpacing = 1);
//     virtual bool run() override;
//     virtual bool finished() const override { return _finished; }
//     virtual void reset() override {
//         _finished = false;
//         currentOffset = 0;
//         setTargetOffset(0);
//         arrivedAtEndTime = 0;
//     }

//     void setText(const std::string& textIn) { text = textIn; }
//     void setTargetOffset(int targetCharacterIndex = -1);
//     void setCurrentOffset(int targetCharacterIndex = -1);
//     void setColours(std::vector<CRGB> colours) { this->colours = colours; }

// private:
//     PixelDisplay& display;
//     std::string text;
//     std::vector<CRGB> colours;
//     uint16_t timeToHoldAtEnd;
//     uint8_t charSpacing;

//     uint32_t targetOffset;
//     uint32_t currentOffset;
//     uint32_t lastUpdateTime;

//     uint32_t stepDelay;
//     uint32_t arrivedAtEndTime = 0;

//     uint32_t calculateOffset(int charIdx) const;

//     bool _finished = false;
// };

// class RepeatingTextScroller : public TextScroller {
// public:
//     RepeatingTextScroller(
//         PixelDisplay& display,
//         std::string textString,
//         std::vector<CRGB> colours,
//         uint16_t stepDelay = 100,
//         uint16_t timeToHoldAtEnd = 1000,
//         uint8_t characterSpacing = 1);
//     bool run() override;
//     bool finished() const override { return cycles >= 2; }
//     void reset() override {
//         TextScroller::reset();
//         forward = true;
//         TextScroller::setTargetOffset(-1);
//         cycles = 0;
//     }

// private:
//     uint32_t cycles = 0;
//     bool forward = true;
// };

class RandomFill : public DisplayEffect {
public:
    RandomFill(const canvas::Canvas& size, uint32_t fillInterval, CRGB (*colourGenerator)());
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final {
        _finished = false;
        _lastSpawnTime = 0;
        _c.fill(0);
    };

private:
    uint32_t _fillInterval;
    CRGB (*_colourGenerator)();
    bool _finished;
    uint32_t _lastSpawnTime = 0;
    canvas::Canvas _c;
};

// class BouncingBall : public DisplayEffect {
// public:
//     BouncingBall(
//         PixelDisplay& display,
//         uint32_t updateInterval,
//         CRGB (*colourGenerator)(),
//         const DisplayRegion& displayRegion = defaultFull);
//     bool run() override final;
//     bool finished() const override final { return _finished; }
//     void reset() override final;

// private:
//     float ballx;
//     float bally;
//     int xDir;
//     int yDir;
//     PixelDisplay& _display;
//     uint32_t _lastLoopTime;
//     uint32_t _updateInterval;
//     CRGB (*_colourGenerator)();
//     DisplayRegion _displayRegion;
//     bool _finished;
// };

// class Gravity : public DisplayEffect {
// public:
//     enum class Direction { up, down, left, right };

//     Gravity(
//         PixelDisplay& display,
//         uint32_t moveInterval,
//         bool empty,
//         Gravity::Direction direction,
//         const DisplayRegion& displayRegion = defaultFull);
//     bool run() override final;
//     bool finished() const override final { return _finished; }
//     void reset() override final;

//     Direction getDirection() const { return _direction; }
//     void setDirection(Direction direction) { _direction = direction; }

//     void setFallOutOfScreen(bool enabled) { _empty = enabled; }

// private:
//     PixelDisplay& _display;
//     DisplayRegion _displayRegion;
//     bool _finished = false;
//     uint32_t _moveInterval;
//     uint32_t _lastMoveTime = 0;
//     bool _empty;
//     Direction _direction;
// };

// class SpectrumDisplay : public DisplayEffect {
// public:
//     SpectrumDisplay(PixelDisplay& display, uint8_t width, uint32_t decayRate);
//     bool run() override final;
//     bool finished() const override final { return _finished; }
//     void reset() override final;

//     void supplyData(std::vector<float> data);

// private:
//     PixelDisplay& _display;
//     uint8_t _width;

//     // float calculateBarHeight(float val, float valMax, float barMax) const;

//     CRGB colMin;
//     CRGB colMax;

//     std::vector<float> _data;
//     float maxScale = 5000;

//     bool _finished = false;
//     uint32_t _decayRate;
//     uint32_t _lastDecayedTime = 0;
// };

// struct VolumeDisplayColourMap {
//     double percentage;
//     CRGB colour;
// };

// class VolumeDisplay : public DisplayEffect {
// public:
//     VolumeDisplay(PixelDisplay& display);
//     bool run() override final;
//     bool finished() const override final { return _finished; }
//     void reset() override final;

// private:
//     PixelDisplay& _display;
//     uint8_t _width;
//     std::vector<VolumeDisplayColourMap> colourMap;
//     bool _finished = false;
// };

// class VolumeGraph : public DisplayEffect {
// public:
//     VolumeGraph(PixelDisplay& display);
//     bool run() override final;
//     bool finished() const override final { return _finished; }
//     void reset() override final;

// private:
//     PixelDisplay& _display;
//     uint8_t _width;
//     bool _finished = false;
// };

// class AudioWaterfall : public DisplayEffect {
// public:
//     AudioWaterfall(PixelDisplay& display);
//     bool run() override final;
//     bool finished() const override final { return _finished; }
//     void reset() override final;

// private:
//     PixelDisplay& _display;
//     uint8_t _width;
//     bool _finished = false;
// };

class ClockFace_Base : public DisplayEffect {
public:
    ClockFace_Base(std::function<ClockFaceTimeStruct(void)> timeCallbackFunction)
        : timeCallbackFunction(timeCallbackFunction) {}
    virtual canvas::Canvas run() override = 0;
    virtual bool finished() const override = 0;
    virtual void reset() override = 0;

protected:
    std::function<ClockFaceTimeStruct(void)> timeCallbackFunction;
};

class ClockFace_Simple : public ClockFace_Base {
public:
    ClockFace_Simple(std::function<ClockFaceTimeStruct(void)> timeCallbackFunction)
        : ClockFace_Base(timeCallbackFunction) {}
    canvas::Canvas run() override final;
    bool finished() const override final { return false; }
    void reset() override final{};
};

// class ClockFace_Gravity : public ClockFace_Base {
// public:
//     ClockFace_Gravity(PixelDisplay& display, std::function<ClockFaceTimeStruct(void)> timeCallbackFunction);
//     bool run() override final;
//     bool finished() const override final { return false; }
//     void reset() override final;

// private:
//     std::unique_ptr<ClockFace_Simple> clockFace;
//     std::unique_ptr<Gravity> gravityEffect;
//     ClockFaceTimeStruct timePrev;
//     enum class State { stable, fallToBottom, fallOut };
//     State currentState = State::stable;
// };

// bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty,
// uint32_t(*colourGenerator)(), DisplayRegion displayRegion); inline bool gravityFill(PixelDisplay& display, uint32_t
// fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)())
// {
//   return gravityFill(display, fillInterval, moveInterval, empty, colourGenerator, display.getFullDisplayRegion());
// }

// void tetris(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval);

// void displayDiagnostic(PixelDisplay& display);

class FilterMethod {
public:
    virtual void apply(canvas::Canvas& c) const = 0;
};

class HSVTestPattern : public FilterMethod {
public:
    HSVTestPattern(){};
    void apply(canvas::Canvas& c) const override;
};

class SolidColour : public FilterMethod {
public:
    SolidColour(CRGB colour, bool maintainBrightness = true) : colour(colour), maintainBrightness(maintainBrightness) {}
    void apply(canvas::Canvas& c) const override;

private:
    CRGB colour;
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
    void apply(canvas::Canvas& c) const override;

private:
    float speed;
    int width;
    bool maintainBrightness;
    Direction direction;
};

#endif // displayeffects_h