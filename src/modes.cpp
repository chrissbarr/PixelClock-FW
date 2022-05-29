  #include "modes.h"
  #include "display/gameOfLife.h"

void MainModeFunction::clearAllButtonCallbacks(Button2& button)
{
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

Mode_SettingsMenu::Mode_SettingsMenu(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton) 
  : MainModeFunction("Settings Menu", display, selectButton, leftButton, rightButton) 
{
  menuTextScroller = std::make_unique<TextScroller>(_display, "Placeholder", CRGB::Red, 50, 2000, true, 1);
  menuPages.push_back(std::make_unique<Mode_SettingsMenu_SetTime>(display, selectButton, leftButton, rightButton));
  menuPages.push_back(std::make_unique<Mode_SettingsMenu_SetBrightness>(display, selectButton, leftButton, rightButton));
}

void Mode_SettingsMenu::moveIntoCore()
{
  menuTextScroller->setText(menuPages[menuIndex]->getName());
  menuTextScroller->reset();
  menuIndex = 0;
  registerButtonCallbacks();
}

void Mode_SettingsMenu::cycleActiveSetting(Button2& btn)
{
  Serial.println("Switching to next setting...");
  Serial.print("Current Setting Index: "); Serial.println(menuIndex);
  Serial.print("Current Setting Name: "); Serial.println(menuPages[menuIndex]->getName());

  if (btn == leftButton) {
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

  Serial.print("New Setting Index: "); Serial.println(menuIndex);
  Serial.print("New Setting Name: "); Serial.println(menuPages[menuIndex]->getName());
  menuTextScroller->setText(menuPages[menuIndex]->getName());
  menuTextScroller->reset();
}

void Mode_SettingsMenu::registerButtonCallbacks()
{
  leftButton.setTapHandler([this](Button2& btn) { cycleActiveSetting(btn); });
  rightButton.setTapHandler([this](Button2& btn) { cycleActiveSetting(btn); });

  auto moveIntoSetting = [this](Button2& btn) {
    activeMenuPage = menuPages[menuIndex];
    activeMenuPage->moveInto();
  };
  selectButton.setTapHandler(moveIntoSetting);
  Serial.println("Registered settings button callbacks");
}

void Mode_SettingsMenu::runCore() 
{
  if (activeMenuPage) {
    activeMenuPage->run();
    if (activeMenuPage->finished()) {
      activeMenuPage->moveOut();
      activeMenuPage.reset();
      registerButtonCallbacks();
    }
  } else {
    menuTextScroller->run();
  }
}

Mode_SettingsMenu_SetTime::Mode_SettingsMenu_SetTime(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton) 
  : MainModeFunction("Set Time", display, selectButton, leftButton, rightButton)
{
  auto modifiedTimeCallback = [this]()->ClockFaceTimeStruct {
    return timeCallbackFunction(now() + this->secondsOffset);
  };
  clockface = std::make_unique<ClockFace>(_display, modifiedTimeCallback);
}

bool Mode_SettingsMenu_SetTime::finished() const
{
  return currentlySettingTimeSegment == TimeSegment::done;
}

void Mode_SettingsMenu_SetTime::moveIntoCore()
{
  auto changeTimeCallback = [this](Button2& btn) {
    Serial.println("Inc/Dec Current Time");
    int changeDir = 1;
    if (btn == leftButton) {
      changeDir = -1;
    } 

    switch (currentlySettingTimeSegment) {
      case TimeSegment::hour:
      {
        int changeAmt = changeDir * 60 * 60;
        Serial.print("Changing hour by: "); Serial.println(changeAmt);
        this->secondsOffset += changeAmt;
        break;
      }
      case TimeSegment::minute:
      {
        int changeAmt = changeDir * 60;
        Serial.print("Changing minute by: "); Serial.println(changeAmt);
        this->secondsOffset += changeAmt;
        break;
      }
      case TimeSegment::second:
      {
        int changeAmt = changeDir;
        Serial.print("Changing second by: "); Serial.println(changeAmt);
        this->secondsOffset += changeAmt;
        break;
      }
    }
  };
  leftButton.setTapHandler(changeTimeCallback);
  rightButton.setTapHandler(changeTimeCallback);

  auto advanceTimeSegment = [this](Button2& btn) {
    Serial.println("Moving to next time segment...");

    switch (currentlySettingTimeSegment) {
      case TimeSegment::hour:
      {
        currentlySettingTimeSegment = TimeSegment::minute;
        break;
      }
      case TimeSegment::minute:
      {
        currentlySettingTimeSegment = TimeSegment::second;
        break;
      }
      case TimeSegment::second:
      {
        currentlySettingTimeSegment = TimeSegment::done;
        break;
      }
    }
  };
  selectButton.setTapHandler(advanceTimeSegment);
  currentlySettingTimeSegment = TimeSegment::hour;

  secondsOffset = 0;
}

void Mode_SettingsMenu_SetTime::runCore()
{
  clockface->run();
}

Mode_ClockFace::Mode_ClockFace(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton) : 
  MainModeFunction("Clockface", display, selectButton, leftButton, rightButton) {
  faces.push_back(std::make_unique<ClockFace>(_display, [](){ return timeCallbackFunction(now()); }));
  filters.push_back(std::make_unique<RainbowWave>(1, 30, RainbowWave::Direction::horizontal, false));
  filters.push_back(std::make_unique<RainbowWave>(1, 30, RainbowWave::Direction::vertical, false));
  timePrev = timeCallbackFunction();
}

void Mode_ClockFace::runCore()
{
  faces[clockfaceIndex]->run();
  if (faces[clockfaceIndex]->finished()) {
    faces[clockfaceIndex]->reset();
  }

  auto timeNow = timeCallbackFunction();
  if (timeNow.minute != timePrev.minute) {
    filterIndex++;
    if (filterIndex == filters.size()) {
      filterIndex = 0;
    }
    lastFilterChangeTime = millis();
  }
  timePrev = timeNow;
  _display.applyFilter(*filters[filterIndex]);
}

Mode_Effects::Mode_Effects(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton) : 
  MainModeFunction("Effects", display, selectButton, leftButton, rightButton) {
  effects.push_back(std::make_unique<RandomFill>(_display, 100, colourGenerator_randomHSV));
  effects.push_back(std::make_unique<BouncingBall>(_display, 100, colourGenerator_cycleHSV));
  effects.push_back(std::make_unique<GameOfLife>(_display, 100, 50, colourGenerator_cycleHSV, _display.getFullDisplayRegion(), false));
}

void Mode_Effects::moveIntoCore()
{
  effects[effectIndex]->reset();

  auto cycleHandler = [this](Button2& btn) {
    Serial.println("Switching to next effect...");
    Serial.print("Current Effect Index: "); Serial.println(effectIndex);
    if (btn == leftButton) {
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
    Serial.print("New Effect Index: "); Serial.println(effectIndex);
  };

  leftButton.setTapHandler(cycleHandler);
  rightButton.setTapHandler(cycleHandler);
}

void Mode_Effects::runCore()
{
  effects[effectIndex]->run();
  if (effects[effectIndex]->finished()) {
    // effectIndex++;
    // if (effectIndex == effects.size()) {
    //   effectIndex = 0;
    // }
    effects[effectIndex]->reset();
  }
}

ModeManager::ModeManager(PixelDisplay& display, Button2& selectButton, Button2& leftButton, Button2& rightButton)
{
  modes.push_back(std::make_unique<Mode_ClockFace>(display, selectButton, leftButton, rightButton));
  modes.push_back(std::make_unique<Mode_Effects>(display, selectButton, leftButton, rightButton));
  modes.push_back(std::make_unique<Mode_SettingsMenu>(display, selectButton, leftButton, rightButton));
}

void ModeManager::run()
{
  modes[modeIndex]->run();
}

void ModeManager::cycleMode()
{
  Serial.println("Switching to next mode...");
  Serial.print("Current Mode Index: "); Serial.println(modeIndex);
  Serial.print("Current Mode Name: "); Serial.println(modes[modeIndex]->getName());

  modes[modeIndex]->moveOut();
  modeIndex++;
  if (modeIndex == modes.size()) {
  modeIndex = 0;
  }
  modes[modeIndex]->moveInto();

  Serial.print("New Mode Index: "); Serial.println(modeIndex);
  Serial.print("New Mode Name: "); Serial.println(modes[modeIndex]->getName());
}