#ifndef audiofft_h
#define audiofft_h

#include "arduinoFFT.h"

#include <memory>
#include <deque>
#include <vector>

inline void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
}

//class ArduinoFFT;
class BluetoothA2DPSink;
namespace audio_tools {
class I2SStream;
}

constexpr int fftSamples = 2048;
constexpr int fftSampleFreq = 44100;
constexpr int fftBandwidth = fftSampleFreq / 2;                 // Bandwidth / Nyquist Freq
constexpr int fftPeriod = 1000 * fftSamples / fftSampleFreq;    // Duration / period of FFT (ms)
constexpr int fftFrequencyResolution = fftSampleFreq / fftSamples;  // Width of each measurement result (Hz)

constexpr int audioSpectrumBins = 17;
constexpr int audioSpectrumMaxFreq = 3000;
constexpr int audioSpectrumBinWidth = audioSpectrumMaxFreq / audioSpectrumBins;
constexpr int audioSpectrumHistorySize = 3;
constexpr int audioSpectrumBinSize = (fftSamples / 4) / audioSpectrumBins;

constexpr int prevMaxesToKeep = 200;

void read_data_stream(const uint8_t *data, uint32_t length);

struct CallbackDiagnostic {
  uint32_t callbackDuration;
  uint32_t fftDuration;
  uint32_t sampleCount;
};

struct Volume {
  float left;
  float right;
};

class Audio {
public:
  static Audio& get()
  {
    static Audio instance;
    return instance;
  }

  Audio(Audio const&) = delete;
  void operator=(Audio const&)  = delete;

  void begin();

  void update();

  void a2dp_callback(const uint8_t *data, uint32_t length);

  std::deque<std::vector<float>>& getAudioSpectrum() { return audioSpectrum; }
  SemaphoreHandle_t getAudioSpectrumSemaphore() { return audioSpectrumSemaphore; }

  std::deque<Volume>& getVolumeHistory() { return volumeHistory; }




private:
  Audio();
  ~Audio();

  std::unique_ptr<ArduinoFFT<float>> FFT;
  std::unique_ptr<BluetoothA2DPSink> a2dpSink;
  std::unique_ptr<audio_tools::I2SStream> i2sOutput;
  float vReal[fftSamples];
  float vImag[fftSamples];
  float weighingFactors[fftSamples];

  SemaphoreHandle_t audioSpectrumSemaphore;
  std::deque<std::vector<float>> audioSpectrum;

  std::deque<float> prevMaxes;

  std::deque<CallbackDiagnostic> callbackDiagnostics;
  uint32_t statReportInterval = 5000;
  uint32_t statReportLastTime = 0;

  int16_t volumeHistorySize = 100;
  std::deque<Volume> volumeHistory;
};

#endif // audiofft_h