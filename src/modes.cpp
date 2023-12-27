/* Project Scope */
#include "modes.h"
#include "FMTWrapper.h"
#include "display/effects/audiowaterfall.h"
#include "display/effects/bouncingball.h"
#include "display/effects/clockfaces.h"
#include "display/effects/gameoflife.h"
#include "display/effects/gravityfill.h"
#include "display/effects/randomfill.h"
#include "display/effects/spectrumdisplay.h"
#include "display/effects/utilities.h"
#include "display/effects/volumedisplay.h"
#include "display/effects/volumegraph.h"
#include "utility.h"
#include "display/canvas.h"

using namespace printing;

void MainModeFunction::clearAllButtonCallbacks(Button2& button) {
    button.setChangedHandler(nullptr);
    button.setClickHandler(nullptr);
    button.setDoubleClickHandler(nullptr);
    button.setLongClickDetectedHandler(nullptr);
    button.setLongClickHandler(nullptr);
    button.setPressedHandler(nullptr);
    button.setReleasedHandler(nullptr);
    button.setTapHandler(nullptr);
    button.setTripleClickHandler(nullptr);
}

void MainModeFunction::moveInto() {
    clearAllButtonCallbacks(buttons.mode);
    clearAllButtonCallbacks(buttons.select);
    clearAllButtonCallbacks(buttons.left);
    clearAllButtonCallbacks(buttons.right);
    _finished = false;
    this->moveIntoCore();
}

void MainModeFunction::moveIntoCore() {
    buttons.mode.setTapHandler([this](Button2& btn) { _finished = true; });
}

canvas::Canvas MainModeFunction::run() { return this->runCore(); }

void MainModeFunction::moveOut() {
    clearAllButtonCallbacks(buttons.mode);
    clearAllButtonCallbacks(buttons.select);
    clearAllButtonCallbacks(buttons.left);
    clearAllButtonCallbacks(buttons.right);
    this->moveOutCore();
}

Mode_SettingsMenu::Mode_SettingsMenu(const canvas::Canvas& size, ButtonReferences buttons)
    : MainModeFunction("Settings Menu", buttons) {
    menuTextScroller =
        std::make_unique<RepeatingTextScroller>(size, "Placeholder", std::vector<pixel::CRGB>{pixel::CRGB::Red}, 50, 2000, 1);
    menuPages.push_back(std::make_unique<Mode_SettingsMenu_SetTime>(size, buttons));
    menuPages.push_back(std::make_unique<Mode_SettingsMenu_SetBrightness>(size, buttons));
}

void Mode_SettingsMenu::moveIntoCore() {
    menuIndex = 0;
    menuTextScroller->setText(menuPages[menuIndex]->getName());
    menuTextScroller->reset();
    registerButtonCallbacks();
}

void Mode_SettingsMenu::moveOutCore() {
    menuPages[menuIndex]->moveOut(); // todo. Need to have some better way of moving up and down (particularly with
                                     // de/re-registering button callbacks.)
}

void Mode_SettingsMenu::cycleActiveSetting(Button2& btn) {

    using namespace printing;

    print("Switching to next setting...\n");
    print(fmt::format("Previous Setting: {} - {}\n", menuIndex, menuPages[menuIndex]->getName()));

    if (btn == buttons.left) {
        if (menuIndex == 0) {
            menuIndex = menuPages.size() - 1;
        } else {
            menuIndex--;
        }
    } else {
        if (menuIndex == menuPages.size() - 1) {
            menuIndex = 0;
        } else {
            menuIndex++;
        }
    }

    print(fmt::format("New Setting: {} - {}\n", menuIndex, menuPages[menuIndex]->getName()));
    menuTextScroller->setText(menuPages[menuIndex]->getName());
    menuTextScroller->reset();
}

void Mode_SettingsMenu::registerButtonCallbacks() {
    buttons.left.setTapHandler([this](Button2& btn) { cycleActiveSetting(btn); });
    buttons.right.setTapHandler([this](Button2& btn) { cycleActiveSetting(btn); });

    auto moveIntoSetting = [this](Button2& btn) {
        activeMenuPage = menuPages[menuIndex];
        activeMenuPage->moveInto();
    };
    buttons.select.setTapHandler(moveIntoSetting);
    buttons.mode.setTapHandler([this](Button2& btn) { this->_finished = true; });
    print("Registered settings button callbacks\n");
}

canvas::Canvas Mode_SettingsMenu::runCore() {
    canvas::Canvas c;
    if (activeMenuPage) {
        c = activeMenuPage->run();
        if (activeMenuPage->finished()) {
            activeMenuPage->moveOut();
            activeMenuPage.reset();
            registerButtonCallbacks();
            menuTextScroller->reset();
        }
    } else {
        c = menuTextScroller->run();
    }
    return c;
}

Mode_SettingsMenu_SetTime::Mode_SettingsMenu_SetTime(const canvas::Canvas& size, ButtonReferences buttons)
    : MainModeFunction("Set Time", buttons) {
    textscroller = std::make_unique<TextScroller>(size, "12:34:56", std::vector<pixel::CRGB>{pixel::CRGB::White}, 10, 1000, 1);
}

bool Mode_SettingsMenu_SetTime::finished() const { return currentlySettingTimeSegment == TimeSegment::done; }

void Mode_SettingsMenu_SetTime::moveIntoCore() {
    auto changeTimeCallback = [this](Button2& btn) {
        print("Inc/Dec Current Time\n");
        int changeDir = 1;
        if (btn == buttons.left) { changeDir = -1; }

        switch (currentlySettingTimeSegment) {
        case TimeSegment::hour: {
            int changeAmt = changeDir * 60 * 60;
            print(fmt::format("Changing hour by: {}\n", changeAmt));
            this->secondsOffset += changeAmt;
            break;
        }
        case TimeSegment::minute: {
            int changeAmt = changeDir * 60;
            print(fmt::format("Changing minute by: {}\n", changeAmt));
            this->secondsOffset += changeAmt;
            break;
        }
        case TimeSegment::second: {
            int changeAmt = changeDir;
            print(fmt::format("Changing second by: {}\n", changeAmt));
            this->secondsOffset += changeAmt;
            break;
        }
        default: {
            break;
        }
        }
    };
    buttons.left.setTapHandler(changeTimeCallback);
    buttons.left.setLongClickDetectedHandler(changeTimeCallback);
    buttons.left.setLongClickDetectedRetriggerable(true);
    buttons.right.setTapHandler(changeTimeCallback);
    buttons.right.setLongClickDetectedHandler(changeTimeCallback);
    buttons.right.setLongClickDetectedRetriggerable(true);

    auto advanceTimeSegment = [this](Button2& btn) {
        print("Moving to next time segment...\n");

        bool forward = (btn == buttons.select);

        switch (currentlySettingTimeSegment) {
        case TimeSegment::cancel: {
            if (forward) {
                currentlySettingTimeSegment = TimeSegment::hour;
            } else {
                currentlySettingTimeSegment = TimeSegment::done;
            }
            break;
        }
        case TimeSegment::hour: {
            if (forward) {
                currentlySettingTimeSegment = TimeSegment::minute;
            } else {
                currentlySettingTimeSegment = TimeSegment::cancel;
            }
            break;
        }
        case TimeSegment::minute: {
            if (forward) {
                currentlySettingTimeSegment = TimeSegment::second;
            } else {
                currentlySettingTimeSegment = TimeSegment::hour;
            }
            break;
        }
        case TimeSegment::second: {
            if (forward) {
                currentlySettingTimeSegment = TimeSegment::confirm;
            } else {
                currentlySettingTimeSegment = TimeSegment::minute;
            }
            break;
        }
        case TimeSegment::confirm: {
            if (forward) {
                currentlySettingTimeSegment = TimeSegment::done;
                TimeManagerSingleton::get().setTime(TimeManagerSingleton::get().now() + this->secondsOffset);
            } else {
                currentlySettingTimeSegment = TimeSegment::second;
            }
            break;
        }
        default: {
            break;
        }
        }
    };
    buttons.select.setTapHandler(advanceTimeSegment);
    buttons.mode.setTapHandler(advanceTimeSegment);
    currentlySettingTimeSegment = TimeSegment::hour;
    secondsOffset = 0;
    textscroller->reset();
    textscroller->setCurrentOffset(5);
    textscroller->setTargetOffset(5);
}

canvas::Canvas Mode_SettingsMenu_SetTime::runCore() {
    // update the scroller text
    auto times = timeCallbackFunction(TimeManagerSingleton::get().now() + this->secondsOffset);
    std::string timestr = fmt::format("back {:2d}:{:2d}:{:2d} ok", times.hour24, times.minute, times.second);
    textscroller->setText(timestr);

    // move to and highlight the active part of the time
    pixel::CRGB colourSel = pixel::CRGB(pixel::CRGB::Red).fadeLightBy(pixel::scale8(pixel::sin8(millis() / 5), 200));
    // 255 - scale8(sin8(millis()/5), 128), 0, 0);
    pixel::CRGB colourIdle = pixel::CRGB(100, 100, 100);
    switch (currentlySettingTimeSegment) {
    case TimeSegment::cancel: {
        textscroller->setTargetOffset(0);
        textscroller->setColours({
            colourSel,
            colourSel,
            colourSel,
            colourSel,
            colourSel, // back + space
            colourIdle,
            colourIdle,
            colourIdle, // HH:
            colourIdle,
            colourIdle,
            colourIdle, // MM:
            colourIdle,
            colourIdle, // SS
            colourIdle,
            colourIdle,
            colourIdle, // space + ok
        });
        break;
    }
    case TimeSegment::hour: {
        textscroller->setTargetOffset(5);
        textscroller->setColours({
            colourIdle,
            colourIdle,
            colourIdle,
            colourIdle,
            colourIdle, // back + space
            colourSel,
            colourSel,
            colourIdle, // HH:
            colourIdle,
            colourIdle,
            colourIdle, // MM:
            colourIdle,
            colourIdle, // SS
            colourIdle,
            colourIdle,
            colourIdle, // space + ok
        });
        break;
    }
    case TimeSegment::minute: {
        textscroller->setTargetOffset(8);
        textscroller->setColours({
            colourIdle,
            colourIdle,
            colourIdle,
            colourIdle,
            colourIdle, // back + space
            colourIdle,
            colourIdle,
            colourIdle, // HH:
            colourSel,
            colourSel,
            colourIdle, // MM:
            colourIdle,
            colourIdle, // SS
            colourIdle,
            colourIdle,
            colourIdle, // space + ok
        });
        break;
    }
    case TimeSegment::second: {
        textscroller->setTargetOffset(11);
        textscroller->setColours({
            colourIdle,
            colourIdle,
            colourIdle,
            colourIdle,
            colourIdle, // back + space
            colourIdle,
            colourIdle,
            colourIdle, // HH:
            colourIdle,
            colourIdle,
            colourIdle, // MM:
            colourSel,
            colourSel, // SS
            colourIdle,
            colourIdle,
            colourIdle, // space + ok
        });
        break;
    }
    case TimeSegment::confirm: {
        textscroller->setTargetOffset(14);
        textscroller->setColours({
            colourIdle,
            colourIdle,
            colourIdle,
            colourIdle,
            colourIdle, // back + space
            colourIdle,
            colourIdle,
            colourIdle, // HH:
            colourIdle,
            colourIdle,
            colourIdle, // MM:
            colourIdle,
            colourIdle, // SS
            colourIdle,
            colourSel,
            colourSel, // space + ok
        });
        break;
    }
    default: {
        break;
    }
    }
    return textscroller->run();
}

Mode_ClockFace::Mode_ClockFace(ButtonReferences buttons) : MainModeFunction("Clockface", buttons) {
    faces.push_back(std::make_unique<ClockFace_Gravity>([]() { return timeCallbackFunction(TimeManagerSingleton::get().now()); }));
    faces.push_back(std::make_unique<ClockFace_Simple>([]() { return timeCallbackFunction(TimeManagerSingleton::get().now()); }));
    filters.push_back(std::make_unique<RainbowWave>(1.0f, 30, RainbowWave::Direction::horizontal, false));
    filters.push_back(std::make_unique<RainbowWave>(1.0f, 30, RainbowWave::Direction::vertical, false));
    timePrev = timeCallbackFunction();
}

void Mode_ClockFace::moveIntoCore() {
    faces[clockfaceIndex]->reset();

    auto cycleClockface = [this](Button2& btn) {
        clockfaceIndex++;
        if (clockfaceIndex == faces.size()) { clockfaceIndex = 0; }
    };
    buttons.select.setTapHandler(cycleClockface);
    buttons.mode.setTapHandler([this](Button2& btn) { this->_finished = true; });
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

Mode_Effects::Mode_Effects(const canvas::Canvas& size, ButtonReferences buttons)
    : MainModeFunction("Effects", buttons) {
    effects.push_back(std::make_unique<AudioWaterfall>(size));
    effects.push_back(std::make_unique<VolumeGraph>(size));
    effects.push_back(std::make_unique<VolumeDisplay>(size));
    effects.push_back(std::make_unique<SpectrumDisplay>(size));
    effects.push_back(std::make_unique<RandomFill>(size, 100, colourGenerator::randomHSV));
    effects.push_back(std::make_unique<BouncingBall>(size, 100, colourGenerator::cycleHSV));
    effects.push_back(std::make_unique<GravityFill>(size, 25, 25, colourGenerator::randomHSV));

    // print("Pregenerating GoL seeds...\n");
    // uint32_t startTime = millis();
    // golTrainer = std::make_unique<GameOfLife>(size, 0, 0, colourGenerator::white, false);
    // golTrainer->setFadeOnDeath(false);
    // golTrainer->setSeedingMode(true);

    // // run until we have a few initial states
    // while (golTrainer->getSeededCount() < 3) {
    //     while (!golTrainer->finished()) {
    //         golTrainer->run();
    //         // catch any infinite-running seeds
    //         if (golTrainer->getLifespan() > 500) { break; }
    //     }
    //     golTrainer->reset();
    // }
    // print("GoL scores: \n");
    // for (const auto& score : golTrainer->getScores()) {
    //     print(fmt::format("{} - {}\n", score.lifespan, score.seed));
    // }
    // uint32_t stopTime = millis();
    // print(fmt::format("Seeding duration: {}", (stopTime - startTime)));

    // golActual = std::make_shared<GameOfLife>(size, 250, 50, colourGenerator::cycleHSV, false);
    // golActual->setScores(golTrainer->getScores());

    // effects.push_back(golActual);
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
    buttons.mode.setTapHandler([this](Button2& btn) { this->_finished = true; });
}

canvas::Canvas Mode_Effects::runCore() {
    auto c = effects[effectIndex]->run();
    if (effects[effectIndex]->finished()) { effects[effectIndex]->reset(); }

    // if (effects[effectIndex] == golActual) {
    //     golTrainer->run();
    //     if (golTrainer->finished()) {
    //         golTrainer->reset();
    //         if (golTrainer->getIterations() % 100 == 0) {
    //             golActual->setScores(golTrainer->getScores());
    //             Serial.println("GoL scores: ");
    //             for (const auto& score : golActual->getScores()) {
    //                 Serial.print(score.lifespan);
    //                 Serial.print("\t");
    //                 Serial.println(score.seed);
    //             }
    //         }
    //     }
    // }
    return c;
}

ModeManager::ModeManager(const canvas::Canvas& size, ButtonReferences buttons) {
    modes.push_back(std::make_unique<Mode_ClockFace>(buttons));
    modes.push_back(std::make_unique<Mode_Effects>(size, buttons));
    modes.push_back(std::make_unique<Mode_SettingsMenu>(size, buttons));
    modes[modeIndex]->moveInto();
}

canvas::Canvas ModeManager::run() {
    const auto c = modes[modeIndex]->run();
    if (modes[modeIndex]->finished()) { cycleMode(); }
    return c;
}

void ModeManager::cycleMode() {
    using namespace printing;

    print("Switching to next mode...\n");
    print(fmt::format("Previous Mode: {} - {}\n", modeIndex, modes[modeIndex]->getName()));

    modes[modeIndex]->moveOut();
    modeIndex++;
    if (modeIndex == modes.size()) { modeIndex = 0; }
    modes[modeIndex]->moveInto();

    print(fmt::format("New Mode: {} - {}\n", modeIndex, modes[modeIndex]->getName()));
}