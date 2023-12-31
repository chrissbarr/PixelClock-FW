/* Project Scope */
#include "modes/modes.h"
#include "FMTWrapper.h"
#include "display/canvas.h"
#include "modes/clockface.h"
#include "modes/effects.h"
#include "modes/settings.h"
#include "utility.h"

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
    buttons.mode.setTapHandler([this]([[maybe_unused]] Button2& btn) { _finished = true; });
}

canvas::Canvas MainModeFunction::run() { return this->runCore(); }

void MainModeFunction::moveOut() {
    clearAllButtonCallbacks(buttons.mode);
    clearAllButtonCallbacks(buttons.select);
    clearAllButtonCallbacks(buttons.left);
    clearAllButtonCallbacks(buttons.right);
    this->moveOutCore();
}

ModeManager::ModeManager(const canvas::Canvas& size, ButtonReferences buttons) {
    modes.push_back(std::make_unique<Mode_ClockFace>(buttons));
    modes.push_back(std::make_unique<Mode_Effects>(size, buttons));
    modes.push_back(std::make_unique<Mode_SettingsMenu>(size, buttons));
    modes[modeIndex]->moveInto();
}

canvas::Canvas ModeManager::run() {
    traceRunTotal.start();
    const auto c = modes[modeIndex]->run();
    if (modes[modeIndex]->finished()) { cycleMode(); }
    traceRunTotal.stop();
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