#ifndef textscroller_h
#define textscroller_h

/* Project Scope */
#include "display/canvas.h"
#include "display/effects/effect.h"

/* C++ Standard Library */
#include <string>
#include <vector>

class TextScroller : public DisplayEffect {
public:
    TextScroller(
        const canvas::Canvas& size,
        std::string textString,
        std::vector<flm::CRGB> colours,
        uint16_t stepDelay = 100,
        uint16_t timeToHoldAtEnd = 1000,
        uint8_t characterSpacing = 1);
    virtual canvas::Canvas run() override;
    virtual bool finished() const override { return _finished; }
    virtual void reset() override {
        _finished = false;
        currentOffset = 0;
        setTargetOffset(0);
        arrivedAtEndTime = 0;
    }

    void setText(const std::string& textIn) { text = textIn; }
    void setTargetOffset(int targetCharacterIndex = -1);
    void setCurrentOffset(int targetCharacterIndex = -1);
    void setColours(std::vector<flm::CRGB> colours) { this->colours = colours; }

private:
    canvas::Canvas _c;
    std::string text;
    std::vector<flm::CRGB> colours;
    uint16_t timeToHoldAtEnd;
    uint8_t charSpacing;

    uint32_t targetOffset;
    uint32_t currentOffset;
    uint32_t lastUpdateTime;

    uint32_t stepDelay;
    uint32_t arrivedAtEndTime = 0;

    uint32_t calculateOffset(int charIdx) const;

    bool _finished = false;
};

class RepeatingTextScroller : public TextScroller {
public:
    RepeatingTextScroller(
        const canvas::Canvas& size,
        std::string textString,
        std::vector<flm::CRGB> colours,
        uint16_t stepDelay = 100,
        uint16_t timeToHoldAtEnd = 1000,
        uint8_t characterSpacing = 1);
    canvas::Canvas run() override;
    bool finished() const override { return cycles >= 2; }
    void reset() override {
        TextScroller::reset();
        forward = true;
        TextScroller::setTargetOffset(-1);
        cycles = 0;
    }

private:
    uint32_t cycles = 0;
    bool forward = true;
};

#endif // textscroller_h