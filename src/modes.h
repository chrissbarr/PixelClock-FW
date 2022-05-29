#ifndef modes_h
#define modes_h

#include <Arduino.h>

#include <Button2.h>

#include "display/display.h"
#include "display/displayEffects.h"

#include <memory>
#include <vector>

class MainModeFunction
{
private:
  void clearAllButtonCallbacks(Button2& button);
protected:
  virtual void moveIntoCore() = 0;
  virtual void moveOutCore() = 0;
  virtual void runCore() = 0;
public:
  MainModeFunction(String name, PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton) : 
  _name(name),
  _display(display),
  selectButton(selectButton),
  leftButton(leftButton),
  rightButton(rightButton)
  {}

  void moveInto()
  {
    clearAllButtonCallbacks(selectButton);
    clearAllButtonCallbacks(leftButton);
    clearAllButtonCallbacks(rightButton);
    this->moveIntoCore();
  }

  void run() 
  {
    this->runCore();
  }

  void moveOut()
  {
    this->moveOutCore();
  }

  virtual bool finished() const { return false; }

  String getName() const { return _name; }

protected:
  PixelDisplay& _display;
  Button2& selectButton;
  Button2& leftButton;
  Button2& rightButton;
  String _name;
};

class Mode_ClockFace : public MainModeFunction
{
public:
  Mode_ClockFace(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton);
protected:
  void moveIntoCore() override final {
    faces[clockfaceIndex]->reset();
  }
  void runCore() override final;
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

class Mode_Effects : public MainModeFunction
{
public:
  Mode_Effects(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton);
protected:
  void moveIntoCore() override final;
  void runCore() override final;
  void moveOutCore() override final {}
private:
  std::vector<std::unique_ptr<DisplayEffect>> effects;
  uint8_t effectIndex = 0;
};

class Mode_SettingsMenu : public MainModeFunction
{
public:
  Mode_SettingsMenu(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton);
protected:
  void moveIntoCore() override final;
  void runCore() override final;
  void moveOutCore() override final;
private:
  void cycleActiveSetting(Button2& btn);
  void registerButtonCallbacks();
  std::unique_ptr<TextScroller> menuTextScroller;
  std::vector<std::shared_ptr<MainModeFunction>> menuPages;
  std::shared_ptr<MainModeFunction> activeMenuPage = nullptr;
  uint8_t menuIndex = 0;
};

class Mode_SettingsMenu_SetTime : public MainModeFunction
{
public:
  Mode_SettingsMenu_SetTime(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton);
  bool finished() const override;
protected:
  void moveIntoCore() override final;
  void runCore() override final;
  void moveOutCore() override final {}
private:
  int secondsOffset = 0;
  std::unique_ptr<TextScroller> textscroller;
  enum class TimeSegment {
    hour,
    minute,
    second,
    done
  };
  TimeSegment currentlySettingTimeSegment = TimeSegment::hour;
};

class Mode_SettingsMenu_SetBrightness : public MainModeFunction
{
public:
  Mode_SettingsMenu_SetBrightness(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton) 
  : MainModeFunction("Set Brightness", display, selectButton, leftButton, rightButton) {}
protected:
  void moveIntoCore() override final {}
  void runCore() override final {}
  void moveOutCore() override final {}
};

class ModeManager
{
public:
  ModeManager(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton);
  void cycleMode();
  void run();

private:
  std::vector<std::unique_ptr<MainModeFunction>> modes;
  uint8_t modeIndex = 0;
};


#endif // modes_h