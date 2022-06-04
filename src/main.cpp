// C++ Std Library
#include <memory>
#include <vector>
#include <string>
#include <functional>

// Libraries
#include <SPI.h>
#include <Button2.h>
#include <LittleFS.h>

#include <AudioOutputI2S.h>
#include <AudioGeneratorMP3.h>
#include <AudioFileSourceLittleFS.h>
#include "AudioFileSourceID3.h"

// Project Scope
#include "pinout.h"
#include "display/display.h"
#include "display/displayEffects.h"
#include "display/gameOfLife.h"
#include "display/fastled_rgbw.h"
#include "timekeeping.h"
#include "brightnessSensor.h"
#include "modes.h"

AudioFileSourceLittleFS *file;
AudioOutputI2S *dac;
AudioGeneratorMP3 *mp3;
AudioFileSourceID3 *id3;

// LED Panel Configuration
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;
constexpr uint16_t dummyLEDCount = getRGBWsize(matrixSize);
CRGB ledsDummyRGBW[dummyLEDCount];
PixelDisplay display(matrixWidth, matrixHeight, false, false);

// Buttons
Button2 buttonMode(buttonPin1, INPUT_PULLUP);
Button2 buttonSelect(buttonPin2, INPUT_PULLUP);
Button2 buttonLeft(buttonPin3, INPUT_PULLUP);
Button2 buttonRight(buttonPin4, INPUT_PULLUP);
Button2 buttonBrightness(buttonPin5, INPUT_PULLUP);

// Modes
std::unique_ptr<ModeManager> modeManager;

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

// Main loop timing
constexpr uint32_t loopTargetTime = 15;     // Constant loop update rate to target (milliseconds)
constexpr uint32_t reportInterval = 10000;  // Statistics on loop timing will be reported this often (milliseconds)
LoopTimeManager loopTimeManager(loopTargetTime, reportInterval);

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }
  
  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}

void setup() {
  delay(1000);
  Serial.begin(250000);
  Serial.println("Serial begin!");
  Serial.print("ESP Chip Model:  "); Serial.println(ESP.getChipModel());
  Serial.print("ESP Chip Rev:    "); Serial.println(ESP.getChipRevision());
  Serial.print("ESP Chip Cores:  "); Serial.println(ESP.getChipCores());
  Serial.print("ESP CPU Freq:    "); Serial.println(ESP.getCpuFreqMHz());
  Serial.print("ESP Flash Mode:  "); Serial.println(ESP.getFlashChipMode());
  Serial.print("ESP Flash Size:  "); Serial.println(ESP.getFlashChipSize());
  Serial.print("ESP Flash Speed: "); Serial.println(ESP.getFlashChipSpeed());
  Serial.print("ESP SDK Version: "); Serial.println(ESP.getSdkVersion());
  Serial.print("Firmware MD5:    "); Serial.println(ESP.getSketchMD5());
  Serial.print("F/W size:        "); Serial.println(ESP.getSketchSize());
  Serial.print("F/W free space:  "); Serial.println(ESP.getFreeSketchSpace());

  Wire.begin();
  initialiseTime();

  Serial.print("LittleFS: "); Serial.println(LittleFS.begin());
  Serial.print("LittleFS Total Bytes: "); Serial.println(LittleFS.totalBytes());
  Serial.print("LittleFS Used Bytes: "); Serial.println(LittleFS.usedBytes());
  Serial.println(LittleFS.exists("/tetris.mp3"));

  brightnessSensor = std::make_unique<BrightnessSensor>();
  modeManager = std::make_unique<ModeManager>(display, ButtonReferences{buttonMode, buttonSelect, buttonLeft, buttonRight});

  FastLED.addLeds<WS2812, matrixLEDPin, RGB>(ledsDummyRGBW, dummyLEDCount);
  display.setLEDStrip(ledsDummyRGBW);

  buttonBrightness.setTapHandler(brightnessButton_callback);

  display.fill(0);
  display.update();
  delay(100); 

  //displayDiagnostic(display);

  audioLogger = &Serial;
  file = new AudioFileSourceLittleFS("/tetris.mp3");
  id3 = new AudioFileSourceID3(file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  dac = new AudioOutputI2S(0, 0, 8, AudioOutputI2S::APLL_ENABLE);
  dac->SetPinout(bclk, wclk, dout);
  dac->SetGain(0.1);
  mp3 = new AudioGeneratorMP3();
  Serial.printf("BEGIN...\n");
  mp3->begin(id3, dac);
}

void loop()
{
  // update buttons
  buttonMode.loop();
  buttonBrightness.loop();
  buttonSelect.loop();
  buttonLeft.loop();
  buttonRight.loop();

  modeManager->run();

  FastLED.setBrightness(brightnessModes[brightnessModeIndex].function());
  FastLED.setDither(1);
  display.update();

  brightnessSensor->update();

  loopTimeManager.idle();

  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  } else {
    Serial.printf("MP3 done\n");
    delay(1000);
  }
}


