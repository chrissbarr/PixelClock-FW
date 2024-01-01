/* Project Scope */
#include "display/effects/gravityfill.h"
#include "display/effects/utilities.h"

/* C++ Standard Library */
#include <memory>

GravityFill::GravityFill(
    const canvas::Canvas& size,
    uint32_t fillInterval,
    uint32_t moveInterval,
    colourGenerator::Generator colourGenerator)
    : _c(size) {
    randomFill = std::make_unique<RandomFill>(size, fillInterval, colourGenerator);
    gravityEffect = std::make_unique<Gravity>(moveInterval, false, Gravity::Direction::down);
    reset();
}

canvas::Canvas GravityFill::run() {

    // Start with previous state
    gravityEffect->setInput(_c);

    // apply gravity effect
    _c = gravityEffect->run();

    if (gravityEffect->finished()) {
        // if gravity effect could detects no movable pixels, spawn new pixel.

        // crop to only top row, this is our spawn region
        auto topRow = canvas::crop(_c, 0, 0, _c.getWidth(), 1);
        randomFill->setInput(topRow);
        topRow = randomFill->run();
        _c = canvas::blit(_c, topRow, 0, 0);

        // unblock gravity effect so it can re-try next loop
        gravityEffect->reset();
    }

    // If there is no space left, effect is finished
    _finished = randomFill->finished();

    return _c;
}

GravityFillTemplate::GravityFillTemplate(FillMode fillMode) : fillMode(fillMode) {
    gravityEffect = std::make_unique<Gravity>(25, false, Gravity::Direction::down);
    reset();
}

void GravityFillTemplate::reset() {
    gravityEffect->reset();
    currentState = State::empty;
}

canvas::Canvas GravityFillTemplate::run() {

    switch (currentState) {
    case State::stable: {
        _finished = true;
        break;
    }
    case State::empty: {

        _c = canvas::Canvas(templateCanvas);
        _c.fill(0);

        spawnCol = 0;
        spawnRow = 0;
        spawnColDir = 1;

        // configure gravity fill
        gravityEffect->setFallOutOfScreen(false);
        gravityEffect->setValidPixelsMask(templateCanvas);

        currentState = State::filling;
        break;
    }
    case State::filling: {
        if (_c == templateCanvas) {
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

            std::vector<int> pixelsPerColTarget = pixelsPerColumn(templateCanvas);
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
                    (templateCanvas.getXY(spawnCol, _c.getHeight() - spawnRow - 1) == 0)) {
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

    return _c;
}