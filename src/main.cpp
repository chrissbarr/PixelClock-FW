// C++ Std Library
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <random>

// Libraries
#include <SPI.h>
#include <Button2.h>
#include <LittleFS.h>

#include "BluetoothA2DPSink.h"
#include <arduinoFFT.h>


// Project Scope
#include "pinout.h"
#include "display/display.h"
#include "display/displayEffects.h"
#include "display/gameOfLife.h"
#include "display/fastled_rgbw.h"
#include "timekeeping.h"
#include "brightnessSensor.h"
#include "modes.h"
#include "utility.h"

BluetoothA2DPSink a2dp_sink;

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

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
}

std::unique_ptr<SpectrumDisplay> specDis;

#define FFT_SPEED_OVER_PRECISION
#define FFT_SQRT_APPROXIMATION

constexpr int fftSamples = 2048;
constexpr int fftSampleFreq = 44100 * 1;
constexpr int binWidthHertz = fftSampleFreq / fftSamples;
float vReal[fftSamples];
float vImag[fftSamples];
float weighingFactors[fftSamples];

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, fftSamples, fftSampleFreq, weighingFactors);

constexpr int prevMaxesToKeep = 100;
std::deque<float> prevMaxes;

void read_data_stream(const uint8_t *data, uint32_t length)
{
  int16_t *samples = (int16_t*) data;
  uint32_t sample_count = length/2;
  Serial.printf("Sample count: %d\n", sample_count);
  for (uint32_t i = 0; i < fftSamples; i++) {
    
    if (i < sample_count) { 
      vReal[i] = samples[i];
    } else {
      vReal[i] = 0;
    }
    
    vImag[i] = 0;
    //Serial.println(vReal[i] / 1000);
  }
  //Serial.println("Input--------");


  FFT.dcRemoval();
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);	/* Weigh data */
  FFT.compute(FFTDirection::Forward); /* Compute FFT */
  FFT.complexToMagnitude(); /* Compute magnitudes */

  constexpr int reducedBins = 15;
  auto binnedData = std::vector<float>(reducedBins, 0);
  constexpr int binSize = (fftSamples / 32) / reducedBins;

  // Analyse FFT results
  for (int i = 2; i < (fftSamples/4); i++) {
    int binIdx = i / binSize;
    if (binIdx < binnedData.size()) {
      binnedData[binIdx] += vReal[i] / binSize;
    }
  }

  float maxThisTime = *std::max_element(binnedData.begin(), binnedData.end());
  prevMaxes.push_back(maxThisTime);
  if (prevMaxes.size() > prevMaxesToKeep) { prevMaxes.pop_front(); }
  float avgMax = std::accumulate(prevMaxes.begin(), prevMaxes.end(), 0.0) / prevMaxes.size();

  float maxScale = 4000;
  float scaleFactor = maxScale / avgMax;
  Serial.printf("Scale factor: %f\n", scaleFactor);

  std::transform(binnedData.begin(), binnedData.end(), binnedData.begin(),
    std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));

  // for (const auto& d : binnedData) {
  //   for (int j = 0; j < d / (maxScale * 10); j++) {
  //     Serial.printf("=");
  //   }
  //   Serial.println();
  // }
  // Serial.println("--------");



  specDis->supplyData(binnedData);
}

void setup() {
  delay(100);
  Serial.begin(500000);

  using namespace utility::printFormatting;

  printSolidLine(headingWidth);
  printTextCentred("Pixel Clock Firmware Start", headingWidth);
  printSolidLine(headingWidth);

  // system
  printTextCentred("System Information", headingWidth);
  Serial.printf("%-*s %s\n", textPadding, "ESP Chip Model:", ESP.getChipModel());
  Serial.printf("%-*s %d\n", textPadding, "ESP Chip Rev:", ESP.getChipRevision());
  Serial.printf("%-*s %d\n", textPadding, "ESP Chip Cores:", ESP.getChipCores());
  Serial.printf("%-*s %d\n", textPadding, "ESP CPU Freq:", ESP.getCpuFreqMHz());
  Serial.printf("%-*s %d\n", textPadding, "ESP Flash Mode:", ESP.getFlashChipMode());
  Serial.printf("%-*s %dKB\n", textPadding, "ESP Flash Size:", ESP.getFlashChipSize() / 1024);
  Serial.printf("%-*s %d\n", textPadding, "ESP Flash Speed:", ESP.getFlashChipSpeed());
  Serial.printf("%-*s %s\n", textPadding, "ESP SDK Version:", ESP.getSdkVersion());

  // firmware
  printTextCentred("Firmware Information", headingWidth);
  Serial.printf("%-*s %s\n", textPadding, "Firmware MD5:", ESP.getSketchMD5().c_str());
  Serial.printf("%-*s %dKB\n", textPadding, "Size:", ESP.getSketchSize() / 1024);
  Serial.printf("%-*s %dKB\n", textPadding, "Available space:", ESP.getFreeSketchSpace() / 1024);
  
  // I2C
  printTextCentred("Initialising I2C", headingWidth);
  bool i2cInitialised = Wire.begin();
  Serial.printf("%-*s %s\n", textPadding, "I2C Initialisation:", i2cInitialised ? "success" : "failed");
  if (!i2cInitialised) {  while (true) {}; }
  utility::listAllI2CDevices(Wire);

  // time
  printTextCentred("Initialising Time", headingWidth);
  initialiseTime();

  // filesystem
  printTextCentred("Initialising Filesystem", headingWidth);
  bool lfsInitialised = LittleFS.begin();
  Serial.printf("%-*s %s\n", textPadding, "LFS Initialisation:", lfsInitialised ? "success" : "failed");
  if (!lfsInitialised) {  while (true) {}; }
  Serial.printf("%-*s %dKB\n", textPadding, "LFS Total Bytes:", LittleFS.totalBytes() / 1024);
  Serial.printf("%-*s %dKB\n", textPadding, "LFS Used Bytes:", LittleFS.usedBytes() / 1024);
  // print all files in FS here?

  printTextCentred("Initialising Light Sensor", headingWidth);
  brightnessSensor = std::make_unique<BrightnessSensor>();

  printTextCentred("Initialising System Modes", headingWidth);
  modeManager = std::make_unique<ModeManager>(display, ButtonReferences{buttonMode, buttonSelect, buttonLeft, buttonRight});

  printTextCentred("Initialising Display", headingWidth);
  FastLED.addLeds<WS2812, matrixLEDPin, RGB>(ledsDummyRGBW, dummyLEDCount);
  display.setLEDStrip(ledsDummyRGBW);
  display.fill(0);
  display.update();
  delay(100); 
  //displayDiagnostic(display);

  printTextCentred("Initialising Input", headingWidth);
  buttonBrightness.setTapHandler(brightnessButton_callback);

  printTextCentred("Initialising Audio", headingWidth);

  i2s_pin_config_t my_pin_config = {
    .bck_io_num = bclk,
    .ws_io_num = wclk,
    .data_out_num = dout,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100,
      .bits_per_sample = (i2s_bits_per_sample_t)16,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 1024,
      .use_apll = false,
      .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.set_i2s_config(i2s_config);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_stream_reader(read_data_stream);
  a2dp_sink.start("MyMusic");

  Serial.printf("%-*s %dms\n", textPadding, "Runtime:", millis());
  printSolidLine(headingWidth);
  printTextCentred("Initialisation Completed", headingWidth);
  printSolidLine(headingWidth);

  specDis = std::make_unique<SpectrumDisplay>(display, display.getWidth(), 0);
  specDis->reset();
}

void loop()
{
  // update buttons
  buttonMode.loop();
  buttonBrightness.loop();
  buttonSelect.loop();
  buttonLeft.loop();
  buttonRight.loop();

  //modeManager->run();

  FastLED.setBrightness(brightnessModes[brightnessModeIndex].function());
  FastLED.setDither(1);

  specDis->run();

  display.update();

  brightnessSensor->update();

  loopTimeManager.idle();

}


