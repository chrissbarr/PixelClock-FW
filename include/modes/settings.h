#ifndef modes_settings_h
#define modes_settings_h

/* Project Scope */
#include "display/display.h"
#include "display/effects/effect.h"
#include "display/effects/filters.h"
#include "display/effects/textscroller.h"
#include "modes/modes.h"

/* C++ Standard Library */
#include <memory>
#include <string>
#include <vector>

class Mode_SettingsMenu : public MainModeFunction {
public:
    Mode_SettingsMenu(const canvas::Canvas& size, ButtonReferences buttons);

protected:
    void moveIntoCore() override final;
    canvas::Canvas runCore() override final;
    void moveOutCore() override final;

private:
    void cycleActiveSetting(Button2& btn);
    void registerButtonCallbacks();
    std::unique_ptr<TextScroller> menuTextScroller;
    std::vector<std::shared_ptr<MainModeFunction>> menuPages;
    std::shared_ptr<MainModeFunction> activeMenuPage = nullptr;
    std::size_t menuIndex = 0;
};

class Mode_SettingsMenu_SetTime : public MainModeFunction {
public:
    Mode_SettingsMenu_SetTime(const canvas::Canvas& size, ButtonReferences buttons);
    bool finished() const override;

protected:
    void moveIntoCore() override final;
    canvas::Canvas runCore() override final;
    void moveOutCore() override final {}

private:
    int secondsOffset = 0;
    std::unique_ptr<TextScroller> textscroller;
    enum class TimeSegment { cancel, hour, minute, second, confirm, done };
    TimeSegment currentlySettingTimeSegment = TimeSegment::hour;
};

class Mode_SettingsMenu_SetBrightness : public MainModeFunction {
public:
    Mode_SettingsMenu_SetBrightness(const canvas::Canvas& size, ButtonReferences buttons)
        : MainModeFunction("Set Brightness", buttons) {}

protected:
    void moveIntoCore() override final {}
    canvas::Canvas runCore() override final { return canvas::Canvas(); }
    void moveOutCore() override final {}
};

#endif // modes_settings_h