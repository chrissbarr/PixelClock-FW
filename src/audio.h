#ifndef audiofft_h
#define audiofft_h

/* Project Scope */
#include "instrumentation.h"

/* Libraries */
#include "arduinoFFT.h"
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <deque>
#include <memory>
#include <vector>

inline void avrc_metadata_callback(uint8_t id, const uint8_t* text) {
    Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
}

// class ArduinoFFT;
class BluetoothA2DPSink;
namespace audio_tools {
class I2SStream;
}

constexpr int fftSamples = 2048;
constexpr int fftSampleFreq = 44100;
constexpr int fftBandwidth = fftSampleFreq / 2;                    // Bandwidth / Nyquist Freq
constexpr int fftPeriod = 1000 * fftSamples / fftSampleFreq;       // Duration / period of FFT (ms)
constexpr int fftFrequencyResolution = fftSampleFreq / fftSamples; // Width of each measurement result (Hz)

constexpr int audioSpectrumBins = 17;
constexpr int audioSpectrumMaxFreq = 3000;
constexpr int audioSpectrumBinWidth = audioSpectrumMaxFreq / audioSpectrumBins;
constexpr int audioSpectrumHistorySize = 3;
constexpr int audioSpectrumBinSize = (fftSamples / 4) / audioSpectrumBins;

void read_data_stream(const uint8_t* data, uint32_t length);

struct AudioCharacteristics {
    float volumeLeft;
    float volumeRight;
    float spectrumMax;
};

class Audio {
public:
    static Audio& get() {
        static Audio instance;
        return instance;
    }

    Audio(const Audio&) = delete;
    void operator=(const Audio&) = delete;

    void begin();

    void update();

    void a2dp_callback(const uint8_t* data, uint32_t length);

    std::deque<std::vector<float>>& getAudioSpectrum() { return audioSpectrum; }
    SemaphoreHandle_t getAudioSpectrumSemaphore() { return audioSpectrumSemaphore; }

    etl::icircular_buffer<AudioCharacteristics>& getAudioCharacteristicsHistory() { return audioCharacteristics; }

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

    InstrumentationTrace callbackDuration;
    InstrumentationTrace audioDuration;
    InstrumentationTrace volDuration;
    InstrumentationTrace fftDuration;

    uint32_t statReportInterval = 5000;
    uint32_t statReportLastTime = 0;

    etl::circular_buffer<AudioCharacteristics, 10> audioCharacteristics;
};

#endif // audiofft_h