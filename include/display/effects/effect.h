#ifndef effect_h
#define effect_h

/* Project Scope */
#include "display/display.h"
#include <canvas.h>

/* Libraries */
#include <FastLED.h>

/* Arduino Core */
#include <Arduino.h>

/* C++ Standard Library */
#include <memory>

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

class DisplayEffectDecorator : public DisplayEffect {
protected:
    std::shared_ptr<DisplayEffect> effect;

public:
    DisplayEffectDecorator(std::shared_ptr<DisplayEffect> effect) : effect(effect) {}
    canvas::Canvas run() { return effect->run(); }
    bool finished() const { return effect->finished(); }
    void reset() { effect->reset(); }
};

class EffectDecorator_Timeout : public DisplayEffectDecorator {
public:
    EffectDecorator_Timeout(std::shared_ptr<DisplayEffect> effect, uint32_t timeout)
        : DisplayEffectDecorator(effect),
          timeoutDuration(timeout) {}
    canvas::Canvas run() { return effect->run(); }
    bool finished() const {
        if (millis() - lastResetTime > timeoutDuration) { return true; }
        return effect->finished();
    }
    void reset() {
        lastResetTime = millis();
        effect->reset();
    }

private:
    uint32_t lastResetTime = 0;
    uint32_t timeoutDuration;
};

#endif // effect_h