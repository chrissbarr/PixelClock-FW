/* Project Scope */
#include "modes.h"
#include "FMTWrapper.h"
#include "display/gameOfLife.h"
#include "utility.h"

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

void MainModeFunction::run() { this->runCore(); }

void MainModeFunction::moveOut() {
    clearAllButtonCallbacks(buttons.mode);
    clearAllButtonCallbacks(buttons.select);
    clearAllButtonCallbacks(buttons.left);
    clearAllButtonCallbacks(buttons.right);
    this->moveOutCore();
}

Mode_SettingsMenu::Mode_SettingsMenu(PixelDisplay& display, ButtonReferences buttons)
    : MainModeFunction("Settings Menu", display, buttons) {
    menuTextScroller =
        std::make_unique<RepeatingTextScroller>(_display, "Placeholder", std::vector<CRGB>{CRGB::Red}, 50, 2000, 1);
    menuPages.push_back(std::make_unique<Mode_SettingsMenu_SetTime>(display, buttons));
    menuPages.push_back(std::make_unique<Mode_SettingsMenu_SetBrightness>(display, buttons));
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

    print(Serial, "Switching to next setting...\n");
    print(Serial, fmt::format("Previous Setting: {} - {}\n", menuIndex, menuPages[menuIndex]->getName()));

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

    print(Serial, fmt::format("New Setting: {} - {}\n", menuIndex, menuPages[menuIndex]->getName()));
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
    Serial.println("Registered settings button callbacks");
}

void Mode_SettingsMenu::runCore() {
    if (activeMenuPage) {
        activeMenuPage->run();
        if (activeMenuPage->finished()) {
            activeMenuPage->moveOut();
            activeMenuPage.reset();
            registerButtonCallbacks();
            menuTextScroller->reset();
        }
    } else {
        menuTextScroller->run();
    }
}

Mode_SettingsMenu_SetTime::Mode_SettingsMenu_SetTime(PixelDisplay& display, ButtonReferences buttons)
    : MainModeFunction("Set Time", display, buttons) {
    textscroller = std::make_unique<TextScroller>(_display, "12:34:56", std::vector<CRGB>{CRGB::White}, 10, 1000, 1);
}

bool Mode_SettingsMenu_SetTime::finished() const { return currentlySettingTimeSegment == TimeSegment::done; }

void Mode_SettingsMenu_SetTime::moveIntoCore() {
    auto changeTimeCallback = [this](Button2& btn) {
        Serial.println("Inc/Dec Current Time");
        int changeDir = 1;
        if (btn == buttons.left) { changeDir = -1; }

        switch (currentlySettingTimeSegment) {
        case TimeSegment::hour: {
            int changeAmt = changeDir * 60 * 60;
            Serial.print("Changing hour by: ");
            Serial.println(changeAmt);
            this->secondsOffset += changeAmt;
            break;
        }
        case TimeSegment::minute: {
            int changeAmt = changeDir * 60;
            Serial.print("Changing minute by: ");
            Serial.println(changeAmt);
            this->secondsOffset += changeAmt;
            break;
        }
        case TimeSegment::second: {
            int changeAmt = changeDir;
            Serial.print("Changing second by: ");
            Serial.println(changeAmt);
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
        Serial.println("Moving to next time segment...");

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
                setTimeGlobally(now() + this->secondsOffset);
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

void Mode_SettingsMenu_SetTime::runCore() {
    // update the scroller text
    auto times = timeCallbackFunction(now() + this->secondsOffset);
    std::string timestr = fmt::format("back {:2d}:{:2d}:{:2d} ok", times.hour24, times.minute, times.second);
    textscroller->setText(timestr);

    // move to and highlight the active part of the time
    CRGB colourSel = CRGB(CRGB::Red).fadeLightBy(scale8(sin8(millis() / 5), 200));
    // 255 - scale8(sin8(millis()/5), 128), 0, 0);
    CRGB colourIdle = CRGB(100, 100, 100);
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
    textscroller->run();
}

Mode_ClockFace::Mode_ClockFace(PixelDisplay& display, ButtonReferences buttons)
    : MainModeFunction("Clockface", display, buttons) {
    faces.push_back(std::make_unique<ClockFace_Gravity>(_display, []() { return timeCallbackFunction(now()); }));
    faces.push_back(std::make_unique<ClockFace_Simple>(_display, []() { return timeCallbackFunction(now()); }));
    filters.push_back(std::make_unique<RainbowWave>(1, 30, RainbowWave::Direction::horizontal, false));
    filters.push_back(std::make_unique<RainbowWave>(1, 30, RainbowWave::Direction::vertical, false));
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

void Mode_ClockFace::runCore() {
    faces[clockfaceIndex]->run();
    if (faces[clockfaceIndex]->finished()) { faces[clockfaceIndex]->reset(); }

    auto timeNow = timeCallbackFunction();
    if (timeNow.minute != timePrev.minute) {
        filterIndex++;
        if (filterIndex == filters.size()) { filterIndex = 0; }
        lastFilterChangeTime = millis();
    }
    timePrev = timeNow;
    _display.applyFilter(*filters[filterIndex]);
}

Mode_Effects::Mode_Effects(PixelDisplay& display, ButtonReferences buttons)
    : MainModeFunction("Effects", display, buttons) {
    effects.push_back(std::make_unique<AudioWaterfall>(_display));
    effects.push_back(std::make_unique<VolumeGraph>(_display));
    effects.push_back(std::make_unique<VolumeDisplay>(_display));
    effects.push_back(std::make_unique<SpectrumDisplay>(_display, _display.getWidth(), 0));
    effects.push_back(std::make_unique<RandomFill>(_display, 100, colourGenerator_randomHSV));
    effects.push_back(std::make_unique<BouncingBall>(_display, 100, colourGenerator_cycleHSV));

    Serial.println("Pregenerating GoL seeds...");
    uint32_t startTime = millis();
    golTrainer =
        std::make_unique<GameOfLife>(display, 0, 0, colourGenerator_white, display.getFullDisplayRegion(), false);
    golTrainer->setFadeOnDeath(false);
    golTrainer->setSeedingMode(true);

    // run until we have a few initial states
    while (golTrainer->getSeededCount() < 3) {
        while (!golTrainer->finished()) {
            golTrainer->run();
            // catch any infinite-running seeds
            if (golTrainer->getLifespan() > 500) { break; }
        }
        golTrainer->reset();
    }
    Serial.println("GoL scores: ");
    for (const auto& score : golTrainer->getScores()) {
        Serial.print(score.lifespan);
        Serial.print("\t");
        Serial.println(score.seed);
    }
    uint32_t stopTime = millis();
    Serial.print("Seeding duration: ");
    Serial.println(stopTime - startTime);

    golActual =
        std::make_shared<GameOfLife>(display, 250, 50, colourGenerator_cycleHSV, display.getFullDisplayRegion(), false);
    golActual->setScores(golTrainer->getScores());

    effects.push_back(golActual);
}

void Mode_Effects::moveIntoCore() {
    effects[effectIndex]->reset();

    auto cycleHandler = [this](Button2& btn) {
        Serial.println("Switching to next effect...");
        Serial.print("Current Effect Index: ");
        Serial.println(effectIndex);
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
        Serial.print("New Effect Index: ");
        Serial.println(effectIndex);
    };

    buttons.left.setTapHandler(cycleHandler);
    buttons.right.setTapHandler(cycleHandler);
    buttons.mode.setTapHandler([this](Button2& btn) { this->_finished = true; });
}

void Mode_Effects::runCore() {
    effects[effectIndex]->run();
    if (effects[effectIndex]->finished()) { effects[effectIndex]->reset(); }

    if (effects[effectIndex] == golActual) {
        golTrainer->run();
        if (golTrainer->finished()) {
            golTrainer->reset();
            if (golTrainer->getIterations() % 100 == 0) {
                golActual->setScores(golTrainer->getScores());
                Serial.println("GoL scores: ");
                for (const auto& score : golActual->getScores()) {
                    Serial.print(score.lifespan);
                    Serial.print("\t");
                    Serial.println(score.seed);
                }
            }
        }
    }
}

ModeManager::ModeManager(PixelDisplay& display, ButtonReferences buttons) {
    modes.push_back(std::make_unique<Mode_ClockFace>(display, buttons));
    modes.push_back(std::make_unique<Mode_Effects>(display, buttons));
    modes.push_back(std::make_unique<Mode_SettingsMenu>(display, buttons));
    modes[modeIndex]->moveInto();
}

void ModeManager::run() {
    modes[modeIndex]->run();
    if (modes[modeIndex]->finished()) { cycleMode(); }
}

void ModeManager::cycleMode() {
    using namespace printing;

    print(Serial, "Switching to next mode...\n");
    print(Serial, fmt::format("Previous Mode: {} - {}\n", modeIndex, modes[modeIndex]->getName()));

    modes[modeIndex]->moveOut();
    modeIndex++;
    if (modeIndex == modes.size()) { modeIndex = 0; }
    modes[modeIndex]->moveInto();

    print(Serial, fmt::format("New Mode: {} - {}\n", modeIndex, modes[modeIndex]->getName()));
}