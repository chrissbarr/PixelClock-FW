/* Project Scope */
#include "modes/clockface.h"
#include "FMTWrapper.h"
#include "display/canvas.h"
#include "display/effects/clockfaces.h"
#include "utility.h"

using namespace printing;

Mode_ClockFace::Mode_ClockFace(ButtonReferences buttons) : MainModeFunction("Clockface", buttons) {
    auto timeCallback = []() { return timeCallbackFunction(TimeManagerSingleton::get().now()); };
    faces.push_back(std::make_unique<ClockFace_GravityFill>(
        timeCallback, std::make_unique<GravityFillTemplate>(GravityFillTemplate::FillMode::leftRightPerRow)));
    faces.push_back(std::make_unique<ClockFace_GravityFill>(
        timeCallback, std::make_unique<GravityFillTemplate>(GravityFillTemplate::FillMode::leftRightPerCol)));
    faces.push_back(std::make_unique<ClockFace_GravityFill>(
        timeCallback, std::make_unique<GravityFillTemplate>(GravityFillTemplate::FillMode::random)));
    faces.push_back(std::make_unique<ClockFace_Gravity>(timeCallback));
    faces.push_back(std::make_unique<ClockFace_Simple>(timeCallback));
    filters.push_back(std::make_unique<RainbowWave>(50.0f, 30, RainbowWave::Direction::horizontal, false));
    filters.push_back(std::make_unique<RainbowWave>(50.0f, 30, RainbowWave::Direction::vertical, false));
    timePrev = timeCallbackFunction();
}

void Mode_ClockFace::moveIntoCore() {
    faces[clockfaceIndex]->reset();

    auto cycleClockface = [this]([[maybe_unused]] Button2& btn) {
        clockfaceIndex++;
        if (clockfaceIndex == faces.size()) { clockfaceIndex = 0; }
        faces[clockfaceIndex]->reset();
    };
    buttons.select.setTapHandler(cycleClockface);
    buttons.mode.setTapHandler([this]([[maybe_unused]] Button2& btn) { this->_finished = true; });
}

canvas::Canvas Mode_ClockFace::runCore() {
    auto c = faces[clockfaceIndex]->run();
    if (faces[clockfaceIndex]->finished()) { faces[clockfaceIndex]->reset(); }

    auto timeNow = timeCallbackFunction();

    // Every minute, change filter
    if (timeNow.minute != timePrev.minute) {
        filterIndex++;
        if (filterIndex == filters.size()) { filterIndex = 0; }
        lastFilterChangeTime = millis();
    }

    // Every hour, switch to random clockface
    // todo - only do this after previous clockface has finished exit transition
    if (timeNow.hour24 != timePrev.hour24) {
        std::uniform_int_distribution<std::size_t> dist(0, faces.size() - 1);
        clockfaceIndex = dist(rand);
        printing::print(fmt::format("Mode_ClockFace switching to new face randomly. New index: {}\n", clockfaceIndex));
        faces[clockfaceIndex]->reset();
    }

    timePrev = timeNow;

    if (filterIndex < filters.size() && filters[filterIndex]) { filters[filterIndex]->apply(c); }

    return c;
}
