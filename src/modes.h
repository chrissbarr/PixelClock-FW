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
  Mode_ClockFace(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton) : 
  MainModeFunction("Clockface", display, selectButton, leftButton, rightButton) {
    faces.push_back(std::make_unique<ClockFace>(_display, timeCallbackFunction));
  }
protected:
  void moveIntoCore() override final {
    faces[clockfaceIndex]->reset();
  }
  void runCore() override final {
    faces[clockfaceIndex]->run();
    if (faces[clockfaceIndex]->finished()) {
      faces[clockfaceIndex]->reset();
    }
  }
  void moveOutCore() override final {}
private:
  std::vector<std::unique_ptr<DisplayEffect>> faces;
  uint8_t clockfaceIndex = 0;
};

class Mode_SettingsMenu : public MainModeFunction
{
public:
  Mode_SettingsMenu(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton) 
  : MainModeFunction("Settings Menu", display, selectButton, leftButton, rightButton) {
    menuTextScroller = std::make_unique<TextScroller>(_display, "Placeholder", CRGB::Red, 100, 1000, true, 1);
    menuPages = {
      {"Set Time"},
      {"Set Date"},
      {"Set Brightness"}
    };
  }
protected:
  void moveIntoCore() override final;
  void runCore() override final;
  void moveOutCore() override final {}
private:
  void cycleActiveSetting(Button2& btn);
  std::unique_ptr<TextScroller> menuTextScroller;
  struct MenuPage {
    String scrollerText;
  };

  std::vector<MenuPage> menuPages;
  uint8_t menuIndex = 0;
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