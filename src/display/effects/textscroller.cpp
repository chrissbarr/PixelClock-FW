/* Project Scope */
#include "display/effects/textscroller.h"
#include "FMTWrapper.h"
#include "display/display.h"
#include "utility.h"

/* C++ Standard Library */
#include <string>
#include <vector>

TextScroller::TextScroller(
    const canvas::Canvas& size,
    std::string textString,
    std::vector<flm::CRGB> colours,
    uint16_t stepDelay,
    uint16_t timeToHoldAtEnd,
    uint8_t characterSpacing)
    : _c(size),
      text(textString),
      colours(colours),
      stepDelay(stepDelay),
      timeToHoldAtEnd(timeToHoldAtEnd),
      charSpacing(characterSpacing) {
    lastUpdateTime = millis();
    currentOffset = 0;
    setTargetOffset(0);
}

canvas::Canvas TextScroller::run() {
    if (currentOffset == targetOffset) {
        if (arrivedAtEndTime == 0) {
            arrivedAtEndTime = millis();
        } else {
            if (millis() - arrivedAtEndTime > timeToHoldAtEnd) {
                _finished = true;
                arrivedAtEndTime = 0;
            }
        }
    } else {
        if (millis() - lastUpdateTime >= stepDelay) {
            if (targetOffset > currentOffset) {
                currentOffset += 1;
            } else if (targetOffset < currentOffset) {
                currentOffset -= 1;
            }
            lastUpdateTime = millis();
        }
    }
    _c.fill(flm::CRGB::Black);
    _c.showCharacters(text, colours, -currentOffset, charSpacing);
    return _c;
}

void TextScroller::setTargetOffset(int targetCharacterIndex) {
    targetOffset = calculateOffset(targetCharacterIndex);
    _finished = false;
}

void TextScroller::setCurrentOffset(int targetCharacterIndex) {
    currentOffset = calculateOffset(targetCharacterIndex);
    _finished = false;
}

uint32_t TextScroller::calculateOffset(int targetCharIndex) const {
    int end = 0;
    int charIndex = 0;
    for (const char& character : text) {
        if (targetCharIndex != -1 && charIndex == targetCharIndex) { break; }
        end += characterFontArray[charToIndex(character)].width + charSpacing;
        charIndex++;
    }
    if (targetCharIndex == -1) { end -= (charSpacing + _c.getWidth()); }
    return end;
}

RepeatingTextScroller::RepeatingTextScroller(
    const canvas::Canvas& size,
    std::string textString,
    std::vector<flm::CRGB> colours,
    uint16_t stepDelay,
    uint16_t timeToHoldAtEnd,
    uint8_t characterSpacing)
    : TextScroller(size, textString, colours, stepDelay, timeToHoldAtEnd, characterSpacing) {
    TextScroller::setTargetOffset(-1);
}

canvas::Canvas RepeatingTextScroller::run() {
    auto c = TextScroller::run();
    bool scrollerFinished = TextScroller::finished();
    if (scrollerFinished) {
        if (forward) {
            TextScroller::setTargetOffset(0);
            forward = false;
        } else {
            TextScroller::setTargetOffset(-1);
            forward = true;
        }
        cycles++;
    }

    return c;
}