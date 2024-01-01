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
    std::function<ClockFaceTimeStruct(void)> timeCallbackFunction, FillMode fillMode)
    : ClockFace_Base(timeCallbackFunction),
      fillMode(fillMode) {
    gravityEffect = std::make_unique<Gravity>(25, false, Gravity::Direction::down);
}

void ClockFace_GravityFill::reset() {
    gravityEffect->reset();
    timePrev = timeCallbackFunction();
    currentState = State::empty;
}

canvas::Canvas ClockFace_GravityFill::run() {
    auto timeNow = timeCallbackFunction();

    switch (currentState) {
    case State::stable: {
        if (timePrev.minute != timeNow.minute) { currentState = State::emptying; }
        break;
    }
    case State::emptying: {
        _c.fill(flm::CRGB::Black);
        currentState = State::empty;
        break;
    }
    case State::empty: {
        // generate target canvas for current time
        auto times = timeCallbackFunction();
        std::string timestr = fmt::format("{:2d}:{:02d}", times.hour12, times.minute);
        targetTextCanvas = canvas::Canvas(17, 5);
        targetTextCanvas.fill(0);
        targetTextCanvas.showCharacters(timestr, {flm::CRGB::White}, 0, 1);

        _c = canvas::Canvas(targetTextCanvas);
        _c.fill(0);

        spawnCol = 0;
        spawnRow = 0;
        spawnColDir = 1;

        // configure gravity fill
        gravityEffect->setFallOutOfScreen(false);
        gravityEffect->setValidPixelsMask(targetTextCanvas);

        currentState = State::filling;
        break;
    }
    case State::filling: {
        if (_c == targetTextCanvas) {
            currentState = State::stable;
            break;
        }

        gravityEffect->setInput(_c);

        // apply gravity effect
        _c = gravityEffect->run();

        if (gravityEffect->finished()) {
            // if gravity effect could detects no movable pixels, spawn new pixel.

            // create list of columns that need new pixels
            auto pixelsPerColumn = [](const canvas::Canvas& c) {
                std::vector<int> pixelsPerCol;
                pixelsPerCol.reserve(c.getWidth());
                for (int x = 0; x < c.getWidth(); x++) {
                    pixelsPerCol.push_back(0);
                    for (int y = 0; y < c.getHeight(); y++) {
                        if (c.getXY(x, y) != 0) { pixelsPerCol.back()++; }
                    }
                }
                return pixelsPerCol;
            };

            std::vector<int> pixelsPerColTarget = pixelsPerColumn(targetTextCanvas);
            std::vector<int> pixelsPerColCurrent = pixelsPerColumn(_c);

            std::vector<int> colsNeedingNewPixel;
            for (int i = 0; i < pixelsPerColTarget.size(); i++) {
                if (pixelsPerColCurrent[i] < pixelsPerColTarget[i]) { colsNeedingNewPixel.push_back(i); }
            }

            if (colsNeedingNewPixel.empty()) { break; }

            int spawnX = -1;

            switch (fillMode) {
            case FillMode::random: {
                // select a column needing a pixel at random
                std::uniform_int_distribution<std::size_t> dist(0, colsNeedingNewPixel.size() - 1);
                std::size_t randomIndex = dist(rand);
                spawnX = colsNeedingNewPixel[randomIndex];
                break;
            }
            case FillMode::leftRightPerCol: {
                // left to right one col at a time
                for (int x = 0; x < _c.getWidth(); x++) {
                    bool xNeedsPixels = false;
                    for (const auto& col : colsNeedingNewPixel) {
                        if (col == x) {
                            xNeedsPixels = true;
                            break;
                        }
                    }
                    if (xNeedsPixels) {
                        spawnX = x;
                        break;
                    }
                }
                break;
            }
            case FillMode::leftRightPerRow: {
                // left to right y ordered
                if (spawnCol < 0 || spawnCol >= _c.getWidth()) {
                    spawnColDir = -spawnColDir;
                    spawnCol += spawnColDir;
                    spawnRow++;
                }
                if (spawnRow >= _c.getHeight()) {
                    spawnCol = 0;
                    spawnRow = 0;
                    spawnColDir = 1;
                    // finished?
                }

                if ((_c.getXY(spawnCol, _c.getHeight() - spawnRow - 1) == 0) !=
                    (targetTextCanvas.getXY(spawnCol, _c.getHeight() - spawnRow - 1) == 0)) {
                    spawnX = spawnCol;
                }
                spawnCol += spawnColDir;
                break;
            }
            }

            // spawn the pixel
            if (spawnX >= 0) { _c.setXY(spawnX, 0, flm::CRGB::White); }

            // unblock gravity effect so it can re-try next loop
            gravityEffect->reset();
        }
        break;
    }
    }

    timePrev = timeNow;
    return _c;
}