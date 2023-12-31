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
    effects.push_back(std::make_unique<AudioWaterfall>(size));
    effects.push_back(std::make_unique<VolumeGraph>(size));
    effects.push_back(std::make_unique<VolumeDisplay>(size));
    effects.push_back(std::make_unique<SpectrumDisplay>(size));
    effects.push_back(std::make_unique<RandomFill>(size, 100, colourGenerator::randomHSV));
    effects.push_back(std::make_unique<BouncingBall>(size, 100, colourGenerator::cycleHSV));
    effects.push_back(std::make_unique<GravityFill>(size, 25, 25, colourGenerator::randomHSV));
    effects.push_back(std::make_unique<GameOfLife>(size, 250, 5, colourGenerator::cycleHSV, false));
    auto gol = std::make_unique<GameOfLife>(size, 250, 5, colourGenerator::white, false);
    gol->setFilter(std::make_unique<RainbowWave>(1.0f, 30, RainbowWave::Direction::horizontal, true));
    effects.push_back(std::move(gol));
}

void Mode_Effects::moveIntoCore() {
    effects[effectIndex]->reset();

    auto cycleHandler = [this](Button2& btn) {
        print("Switching to next effect...\n");
        print(fmt::format("Current Effect Index: {}\n", effectIndex));
        if (btn == buttons.left) {
            if (effectIndex == 0) {
                effectIndex = effects.size() - 1;
            } else {
                effectIndex--;
            }
        } else {
            if (effectIndex == effects.size() - 1) {
                effectIndex = 0;
            } else {
                effectIndex++;
            }
        }
        print(fmt::format("New Effect Index: {}\n", effectIndex));
    };

    buttons.left.setTapHandler(cycleHandler);
    buttons.right.setTapHandler(cycleHandler);
    buttons.mode.setTapHandler([this]([[maybe_unused]] Button2& btn) { this->_finished = true; });
}

canvas::Canvas Mode_Effects::runCore() {
    auto c = effects[effectIndex]->run();
    if (effects[effectIndex]->finished()) { effects[effectIndex]->reset(); }
    return c;
}