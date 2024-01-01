/* Project Scope */
#include "display/effects/clockfaces.h"
#include "FMTWrapper.h"
#include "utility.h"

/* C++ Standard Library */
#include <memory>
#include <string>

canvas::Canvas ClockFace_Simple::run() {
    auto times = timeCallbackFunction();
    std::string timestr = fmt::format("{:2d}:{:02d}", times.hour12, times.minute);
    auto c = canvas::Canvas(17, 5);
    c.fill(0);
    c.showCharacters(timestr, {flm::CRGB::White}, 0, 1);
    return c;
}

ClockFace_Gravity::ClockFace_Gravity(std::function<ClockFaceTimeStruct(void)> timeCallbackFunction)
    : ClockFace_Base(timeCallbackFunction) {
    gravityEffect = std::make_unique<Gravity>(500, false, Gravity::Direction::down);
    clockFace = std::make_unique<ClockFace_Simple>(timeCallbackFunction);
}

void ClockFace_Gravity::reset() {
    gravityEffect->reset();
    clockFace->reset();
    timePrev = timeCallbackFunction();
    currentState = State::stable;
}

canvas::Canvas ClockFace_Gravity::run() {
    auto timeNow = timeCallbackFunction();

    switch (currentState) {
    case State::stable:
        if (timePrev.minute != timeNow.minute) {
            currentState = State::fallToBottom;
            gravityEffect->reset();
            gravityEffect->setInput(_c);
            gravityEffect->setFallOutOfScreen(false);
        } else {
            _c = clockFace->run();
        }
        break;
    case State::fallToBottom:
        _c = gravityEffect->run();
        if (gravityEffect->finished()) {
            currentState = State::fallOut;
            gravityEffect->setFallOutOfScreen(true);
            gravityEffect->reset();
        }
        break;
    case State::fallOut:
        _c = gravityEffect->run();
        if (gravityEffect->finished()) { currentState = State::stable; }
        break;
    }

    timePrev = timeNow;
    return _c;
}

ClockFace_GravityFill::ClockFace_GravityFill(
    std::function<ClockFaceTimeStruct(void)> timeCallbackFunction,
    std::unique_ptr<GravityFillTemplate> gravFillTemplate)
    : ClockFace_Base(timeCallbackFunction),
      gravFill(std::move(gravFillTemplate)) {}

void ClockFace_GravityFill::reset() {

    canvas::Canvas newTemplate(17, 5);
    newTemplate.fill(0);
    auto times = timeCallbackFunction();
    std::string timestr = fmt::format("{:2d}:{:02d}", times.hour12, times.minute);
    newTemplate.showCharacters(timestr, {flm::CRGB::White}, 0, 1);
    gravFill->setTemplate(newTemplate);
    gravFill->reset();
    timePrev = timeCallbackFunction();
}

canvas::Canvas ClockFace_GravityFill::run() {
    auto timeNow = timeCallbackFunction();

    if (gravFill->finished()) {
        if (timePrev.minute != timeNow.minute) { reset(); }
    }
    _c = gravFill->run();

    timePrev = timeNow;
    return _c;
}