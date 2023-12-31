/* Project Scope */
#include "modes/settings.h"
#include "FMTWrapper.h"
#include "display/canvas.h"
#include "utility.h"

using namespace printing;

Mode_SettingsMenu::Mode_SettingsMenu(const canvas::Canvas& size, ButtonReferences buttons)
    : MainModeFunction("Settings Menu", buttons) {
    menuTextScroller = std::make_unique<RepeatingTextScroller>(
        size, "Placeholder", std::vector<flm::CRGB>{flm::CRGB::Red}, 50, 2000, 1);
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

    auto moveIntoSetting = [this]([[maybe_unused]] Button2& btn) {
        activeMenuPage = menuPages[menuIndex];
        activeMenuPage->moveInto();
    };
    buttons.select.setTapHandler(moveIntoSetting);
    buttons.mode.setTapHandler([this]([[maybe_unused]] Button2& btn) { this->_finished = true; });
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
    textscroller =
        std::make_unique<TextScroller>(size, "12:34:56", std::vector<flm::CRGB>{flm::CRGB::White}, 10, 1000, 1);
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
    flm::CRGB colourSel =
        flm::CRGB(flm::CRGB::Red).fadeLightBy(flm::scale8(flm::sin8(static_cast<uint8_t>(millis() / 5)), 200));
    // 255 - scale8(sin8(millis()/5), 128), 0, 0);
    flm::CRGB colourIdle = flm::CRGB(100, 100, 100);
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