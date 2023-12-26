#ifndef audiofft_h
#define audiofft_h

/* Project Scope */

/* Libraries */
#include <etl/circular_buffer.h>
#include <etl/array.h>

/* C++ Standard Library */


constexpr int fftSamples = 2048;
constexpr int fftSampleFreq = 44100;
constexpr int fftBandwidth = fftSampleFreq / 2;                    // Bandwidth / Nyquist Freq
constexpr int fftPeriod = 1000 * fftSamples / fftSampleFreq;       // Duration / period of FFT (ms)
constexpr int fftFrequencyResolution = fftSampleFreq / fftSamples; // Width of each measurement result (Hz)

constexpr int audioSpectrumBins = 17;
constexpr int audioSpectrumMaxFreq = 3000;
constexpr int audioSpectrumBinWidth = audioSpectrumMaxFreq / audioSpectrumBins;
constexpr int audioSpectrumBinSize = (fftSamples / 4) / audioSpectrumBins;

constexpr int audioHistorySize = 100;

struct AudioCharacteristics {
    float volumeLeft;  // Volume of last chunk left channel (RMS dBFS)
    float volumeRight; // Volume of last chunk right channel (RMS dBFS)
    float spectrumMax;
    etl::array<float, audioSpectrumBins> spectrum;
};

class Audio {
public:
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void a2dp_callback(const uint8_t* data, uint32_t length) = 0;
    virtual void lockMutex() = 0;
    virtual void releaseMutex() = 0;
    virtual etl::icircular_buffer<AudioCharacteristics>& getAudioCharacteristicsHistory() = 0;
};

class AudioSingleton {
public:
    static Audio& get();
    AudioSingleton(const AudioSingleton&) = delete;
    void operator=(const AudioSingleton&) = delete;
};


#endif // audiofft_h