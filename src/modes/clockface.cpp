/* Project Scope */
#include "modes/clockface.h"
#include "FMTWrapper.h"
#include "display/canvas.h"
#include "display/effects/clockfaces.h"
#include "utility.h"

using namespace printing;

Mode_ClockFace::Mode_ClockFace(ButtonReferences buttons) : MainModeFunction("Clockface", buttons) {
    faces.push_back(
        std::make_unique<ClockFace_Gravity>([]() { return timeCallbackFunction(TimeManagerSingleton::get().now()); }));
    faces.push_back(
        std::make_unique<ClockFace_Simple>([]() { return timeCallbackFunction(TimeManagerSingleton::get().now()); }));
    filters.push_back(std::make_unique<RainbowWave>(50.0f, 30, RainbowWave::Direction::horizontal, false));
    filters.push_back(std::make_unique<RainbowWave>(50.0f, 30, RainbowWave::Direction::vertical, false));
    timePrev = timeCallbackFunction();
}

void Mode_ClockFace::moveIntoCore() {
    faces[clockfaceIndex]->reset();

    auto cycleClockface = [this]([[maybe_unused]] Button2& btn) {
        clockfaceIndex++;
        if (clockfaceIndex == faces.size()) { clockfaceIndex = 0; }
    };
    buttons.select.setTapHandler(cycleClockface);
    buttons.mode.setTapHandler([this]([[maybe_unused]] Button2& btn) { this->_finished = true; });
}

canvas::Canvas Mode_ClockFace::runCore() {
    auto c = faces[clockfaceIndex]->run();
    if (faces[clockfaceIndex]->finished()) { faces[clockfaceIndex]->reset(); }

    auto timeNow = timeCallbackFunction();
    if (timeNow.minute != timePrev.minute) {
        filterIndex++;
        if (filterIndex == filters.size()) { filterIndex = 0; }
        lastFilterChangeTime = millis();
    }
    timePrev = timeNow;

    if (filterIndex < filters.size() && filters[filterIndex]) { filters[filterIndex]->apply(c); }

    return c;
}
