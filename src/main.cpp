// C++ Std Library
#include <memory>
#include <vector>
#include <string>
#include <functional>

// Libraries
#include <SPI.h>

#include <Button2.h>


#include <melody_player.h>
#include <melody_factory.h>

// Project Scope
#include "pinout.h"
#include "display/display.h"
#include "display/displayEffects.h"
#include "display/gameOfLife.h"
#include "display/fastled_rgbw.h"
#include "timekeeping.h"
#include "brightnessSensor.h"

// LED Panel Configuration
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;
constexpr uint16_t dummyLEDCount = getRGBWsize(matrixSize);
CRGB ledsDummyRGBW[dummyLEDCount];
PixelDisplay display(matrixWidth, matrixHeight, false, false);

MelodyPlayer player(buzzerPin, HIGH);

// Buttons
Button2 buttonMode(buttonPin1, INPUT_PULLUP);
Button2 buttonSelect(buttonPin2, INPUT_PULLUP);
Button2 buttonLeft(buttonPin3, INPUT_PULLUP);
Button2 buttonRight(buttonPin4, INPUT_PULLUP);
Button2 buttonBrightness(buttonPin5, INPUT_PULLUP);

void clearAllButtonCallbacks(Button2& button)
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

//// Brightness Handling
std::unique_ptr<BrightnessSensor> brightnessSensor;
struct BrightnessMode {
  String name;
  std::function<uint8_t()> function;
};

uint8_t brightnessFromSensor()
{
  float maxBrightness = 1.7;
  return uint8_t(constrain(map(brightnessSensor->getBrightness() * 1000, 0, 1700, 0, 255), 1, 255));
}

std::vector<BrightnessMode> brightnessModes = {
  {"High", [](){ return 255; }},
  {"Med", [](){ return 127; }},
  {"Low", [](){ return 10; }},
  {"Auto", brightnessFromSensor}
};
uint8_t brightnessModeIndex = 0;

class MainModeFunction
{
protected:
  virtual void moveIntoCore() = 0;
  virtual void moveOutCore() = 0;
  virtual void runCore() = 0;
public:
  MainModeFunction(String name) : _name(name) {}
  void moveInto()
  {
    clearAllButtonCallbacks(buttonSelect);
    clearAllButtonCallbacks(buttonLeft);
    clearAllButtonCallbacks(buttonRight);
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

private:

  String _name;
};

class Mode_ClockFace : public MainModeFunction
{
public:
  Mode_ClockFace() : MainModeFunction("Clockface") {
    faces.push_back(std::make_unique<ClockFace>(display, timeCallbackFunction));
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
  Mode_SettingsMenu() : MainModeFunction("Settings Menu") {
    menuTextScroller = std::make_unique<TextScroller>(display, "Placeholder", CRGB::Red, 250, 1000, true, 1);
    menuPages = {
      {"Set Time"},
      {"Set Date"},
      {"Set Brightness"}
    };
  }
protected:
  void moveIntoCore() override final {
    menuTextScroller->reset();
    menuIndex = 0;
  }
  void runCore() override final {
    menuTextScroller->setText(menuPages[menuIndex].scrollerText);
    menuTextScroller->run();
  }
  void moveOutCore() override final {}
private:
  std::unique_ptr<TextScroller> menuTextScroller;
  struct MenuPage {
    String scrollerText;
  };

  std::vector<MenuPage> menuPages;
  uint8_t menuIndex = 0;
};

std::vector<std::unique_ptr<MainModeFunction>> mainModes;
uint8_t modeIndex = 0;

void brightnessButton_callback(Button2& btn) 
{
  Serial.println("Brightness button callback...");

  Serial.println("Switching to next brightness...");
  Serial.print("Current Brightness Index: "); Serial.println(brightnessModeIndex);
  Serial.print("Current Brightness Name: "); Serial.println(brightnessModes[brightnessModeIndex].name);

  brightnessModeIndex++;
  if (brightnessModeIndex == brightnessModes.size()) {
    brightnessModeIndex = 0;
  }

  Serial.print("New Brightness Index: "); Serial.println(brightnessModeIndex);
  Serial.print("New Brightness Name: "); Serial.println(brightnessModes[brightnessModeIndex].name);
}

void click(Button2& btn) {
  Serial.println("Button click callback...");
  if (btn == buttonMode) {
    Serial.println("BTN0");
    Serial.println("Switching to next mode...");
    Serial.print("Current Mode Index: "); Serial.println(modeIndex);
    Serial.print("Current Mode Name: "); Serial.println(mainModes[modeIndex]->getName());

    mainModes[modeIndex]->moveOut();
    modeIndex++;
    if (modeIndex == mainModes.size()) {
      modeIndex = 0;
    }
    mainModes[modeIndex]->moveInto();

    Serial.print("New Mode Index: "); Serial.println(modeIndex);
    Serial.print("New Mode Name: "); Serial.println(mainModes[modeIndex]->getName());

  } 
  if (btn == buttonBrightness) {
    Serial.println("B clicked");
  }
}



// Main loop timing
uint32_t lastLoopTime = 0;
constexpr uint32_t loopTargetTime = 15;
uint32_t lastReportTime = 0;
constexpr uint32_t reportInterval = 5000;

float avgTime = 0;
uint16_t minTime = std::numeric_limits<uint16_t>::max();
uint16_t maxTime = 0;

constexpr float approxRollingAverage(float avg, float newSample, int N) 
{
  avg -= avg / N;
  avg += newSample / N;
  return avg;
}

std::vector<std::shared_ptr<DisplayEffect>> displayEffects;
std::size_t effectIndex = 0;

struct FilterConfig {
  std::unique_ptr<FilterMethod> filter;
  String description;
};
std::vector<FilterConfig> filterConfigs;
std::size_t filterIndex = 0;
uint32_t lastFilterChangeTime = 0;
uint32_t filterChangePeriod = 3000;

std::unique_ptr<GameOfLife> golTrainer;
std::shared_ptr<GameOfLife> golActual;

void setup() {
  delay(1000);
  Serial.begin(250000);
  Serial.println("Serial begin!");

  Wire.begin();

  brightnessSensor = std::make_unique<BrightnessSensor>();

  initialiseTime();
  delay(1000);

  mainModes.push_back(std::make_unique<Mode_ClockFace>());
  mainModes.push_back(std::make_unique<Mode_SettingsMenu>());

  FastLED.addLeds<WS2812, matrixLEDPin, RGB>(ledsDummyRGBW, dummyLEDCount);
  display.setLEDStrip(ledsDummyRGBW);

  Serial.println("Loading melody...");
  const char melodyString[] = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
  // create a melody
  Melody melody = MelodyFactory.loadRtttlString(melodyString);

  // get basic info about the melody
  Serial.println(String(" Title:") + melody.getTitle());
  Serial.println(String(" Time unit:") + melody.getTimeUnit());

  Serial.print("Start playing in blocking mode... ");
  //player.play(melody);
  Serial.println("Melody ends!");

  buttonMode.setClickHandler(click);
  buttonBrightness.setTapHandler(brightnessButton_callback);

  display.fill(0);
  display.update();
  delay(100); 

  //displayDiagnostic(display);

  Serial.println("Pregenerating GoL seeds...");
  uint32_t startTime = millis();




  // start time tracking for main loop
  lastLoopTime = millis();
}

void loop()
{

  // update buttons
  buttonMode.loop();
  buttonBrightness.loop();

  mainModes[modeIndex]->run();

  FastLED.setBrightness(brightnessModes[brightnessModeIndex].function());
  FastLED.setDither(1);
  display.update();

  brightnessSensor->update();

  // Manage loop timing
  unsigned long loopTime = millis() - lastLoopTime;

  avgTime = approxRollingAverage(avgTime, float(loopTime), 1000);
  if (loopTime > maxTime) { maxTime = loopTime; }
  if (loopTime < minTime) { minTime = loopTime; }

  if (millis() - lastReportTime > reportInterval) {
    Serial.println("Loop Timing Statistics");
    Serial.print("Avg time:" ); Serial.println(avgTime);
    Serial.print("Min time:" ); Serial.println(minTime);
    Serial.print("Max time:" ); Serial.println(maxTime);
    Serial.print("FastLED FPS:" ); Serial.println(FastLED.getFPS());
    lastReportTime = millis();
    minTime = std::numeric_limits<uint16_t>::max();
    maxTime = 0;
  }

  yield();
  while (millis() - lastLoopTime < loopTargetTime) {
    yield();
  }
  lastLoopTime = millis();
}


