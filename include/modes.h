#ifndef modes_h
#define modes_h

/* Project Scope */
#include "display/display.h"
#include "display/effects/effect.h"
#include "display/effects/filters.h"
#include "display/effects/textscroller.h"
#include "timekeeping.h"
#include <canvas.h>

/* Libraries */
#include <Button2.h>

/* Arduino Core */
#include <Arduino.h>

/* C++ Standard Library */
#include <memory>
#include <string>
#include <vector>

class GameOfLife;

struct ButtonReferences {
    Button2& mode;
    Button2& select;
    Button2& left;
    Button2& right;
};

class MainModeFunction {
public:
    MainModeFunction(std::string name, ButtonReferences buttons) : _name(name), buttons(buttons) {}
    // should be called by the parent when moving into this mode
    void moveInto();
    // should be called by the parent when this mode is active
    canvas::Canvas run();
    // should be called by the parent when moving out of this mode
    void moveOut();
    // indicates that this mode is ready to exit/return
    virtual bool finished() const { return _finished; }
    // get the name of this mode
    std::string getName() const { return _name; }

protected:
    virtual void moveIntoCore();
    virtual void moveOutCore() = 0;
    virtual canvas::Canvas runCore() = 0;
    bool _finished = false;
    ButtonReferences buttons;
    std::string _name;

private:
    void clearAllButtonCallbacks(Button2& button);
};

class Mode_ClockFace : public MainModeFunction {
public:
    Mode_ClockFace(ButtonReferences buttons);

protected:
    void moveIntoCore() override final;
    canvas::Canvas runCore() override final;
    void moveOutCore() override final {}

private:
    std::vector<std::unique_ptr<DisplayEffect>> faces;
    uint8_t clockfaceIndex = 0;
    std::vector<std::unique_ptr<FilterMethod>> filters;
    uint8_t filterIndex = 0;
    uint32_t lastFilterChangeTime = 0;
    uint32_t filterChangePeriod = 10000;
    ClockFaceTimeStruct timePrev;
};

class Mode_Effects : public MainModeFunction {
public:
    Mode_Effects(const canvas::Canvas& size, ButtonReferences buttons);

protected:
    void moveIntoCore() override final;
    canvas::Canvas runCore() override final;
    void moveOutCore() override final {}

private:
    std::vector<std::shared_ptr<DisplayEffect>> effects;
    uint8_t effectIndex = 0;

    // std::unique_ptr<GameOfLife> golTrainer;
    // std::shared_ptr<GameOfLife> golActual;
};

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
    uint8_t menuIndex = 0;
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

class ModeManager {
public:
    ModeManager(const canvas::Canvas& size, ButtonReferences buttons);
    void cycleMode();
    canvas::Canvas run();

private:
    std::vector<std::unique_ptr<MainModeFunction>> modes;
    uint8_t modeIndex = 0;
};

#endif // modes_h