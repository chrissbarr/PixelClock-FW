/* Project Scope */
#include "modes/effects.h"
#include "FMTWrapper.h"
#include "display/canvas.h"
#include "display/effects/audiowaterfall.h"
#include "display/effects/bouncingball.h"
#include "display/effects/gameoflife.h"
#include "display/effects/gravityfill.h"
#include "display/effects/randomfill.h"
#include "display/effects/spectrumdisplay.h"
#include "display/effects/utilities.h"
#include "display/effects/volumedisplay.h"
#include "display/effects/volumegraph.h"
#include "utility.h"

using namespace printing;

Mode_Effects::Mode_Effects(const canvas::Canvas& size, ButtonReferences buttons)
    : MainModeFunction("Effects", buttons) {
    effects.push_back({"Audio Waterfall", std::make_unique<AudioWaterfall>(size)});
    effects.push_back({"Volume Graph", std::make_unique<VolumeGraph>(size)});
    effects.push_back({"Volume Display", std::make_unique<VolumeDisplay>(size)});
    effects.push_back({"Spectrum Display", std::make_unique<SpectrumDisplay>(size)});
    effects.push_back({"Random Fill", std::make_unique<RandomFill>(size, 100, colourGenerator::randomHSV)});
    effects.push_back({"Bouncing Ball", std::make_unique<BouncingBall>(size, 100, colourGenerator::cycleHSV)});
    effects.push_back({"Gravity Fill", std::make_unique<GravityFill>(size, 25, 25, colourGenerator::randomHSV)});
    effects.push_back({"GoL - 1", std::make_unique<GameOfLife>(size, 250, 5, colourGenerator::cycleHSV, false)});
    auto gol = std::make_unique<GameOfLife>(size, 250, 5, colourGenerator::white, false);
    gol->setFilter(std::make_unique<RainbowWave>(1.0f, 30, RainbowWave::Direction::horizontal, true));
    effects.push_back({"GoL - 2", std::move(gol)});
}

void Mode_Effects::moveIntoCore() {
    effects[effectIndex].ptr->reset();

    auto cycleHandler = [&](Button2& btn) {
        if (currentState != State::Transition) {
            if (btn == buttons.left) {
                transitionDir = -1;
            } else {
                transitionDir = 1;
            }
            print(fmt::format("Switching to next effect. Direction: {:+d}\n", transitionDir));
            currentState = State::Transition;
        }
    };

    buttons.left.setTapHandler(cycleHandler);
    buttons.right.setTapHandler(cycleHandler);
    buttons.mode.setTapHandler([this]([[maybe_unused]] Button2& btn) { this->_finished = true; });
}

canvas::Canvas Mode_Effects::runCore() {

    uint32_t millisSinceLastRun = millis() - lastLoopTime;
    float tdelta = static_cast<float>(millisSinceLastRun) / 1000;
    lastLoopTime = millis();

    canvas::Canvas c;
    switch (currentState) {
    case State::Stable: {
        c = effects[effectIndex].ptr->run();
        if (effects[effectIndex].ptr->finished()) { effects[effectIndex].ptr->reset(); }
        break;
    }
    case State::Transition: {
        print(fmt::format("State::Transition - {} - {}\n", transitionPercentage, transitionDir));
        std::size_t effectIdxLeft = (effects.size() - 1 + effectIndex) % effects.size();
        std::size_t effectIdxRight = (effects.size() + 1 + effectIndex) % effects.size();
        print(fmt::format("Left Current Right - {} {} {}\n", effectIdxLeft, effectIndex, effectIdxRight));
        canvas::Canvas cl = effects[effectIdxLeft].ptr->run();
        canvas::Canvas cm = effects[effectIndex].ptr->run();
        canvas::Canvas cr = effects[effectIdxRight].ptr->run();
        canvas::Canvas cb(cm);
        cb.fill(0);

        const float transitionDuration = 0.2f; // seconds
        transitionPercentage += (1.0f / transitionDuration) * tdelta;

        auto easeInOutCubic = [](float t) -> float {
            if (t < 0.5) {
                return 4 * t * t * t;
            } else {
                float f = ((2 * t) - 2);
                return 0.5f * f * f * f + 1;
            }
        };

        float transitionPositionEased = easeInOutCubic(transitionPercentage);

        int xOffset = static_cast<int>(std::round((transitionPositionEased * -transitionDir) * cb.getWidth()));

        cb = canvas::blit(cb, cl, xOffset - cb.getWidth(), 0);
        cb = canvas::blit(cb, cm, xOffset, 0);
        cb = canvas::blit(cb, cr, xOffset + cb.getWidth(), 0);

        if (transitionPercentage >= 1.0) {
            effectIndex = (effects.size() + transitionDir + effectIndex) % effects.size();
            print(fmt::format(
                "Effect Transition Finished, New Effect = {} - {}\n", effectIndex, effects[effectIndex].name));
            currentState = State::Stable;
            transitionPercentage = 0;
        }

        c = cb;
        break;
    }
    }

    return c;
}