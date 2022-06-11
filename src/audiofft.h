#ifndef audiofft_h
#define audiofft_h

// #define FFT_SPEED_OVER_PRECISION
// #define FFT_SQRT_APPROXIMATION

#include <arduinoFFT.h>

#include <memory>
#include <deque>
#include <vector>

constexpr int fftSamples = 2048;
constexpr int fftSampleFreq = 44100;
constexpr int fftBandwidth = fftSampleFreq / 2;                 // Bandwidth / Nyquist Freq
constexpr int fftPeriod = 1000 * fftSamples / fftSampleFreq;    // Duration / period of FFT (ms)
constexpr int fftFrequencyResolution = fftSampleFreq / fftSamples;  // Width of each measurement result (Hz)

extern float vReal[fftSamples];
extern float vImag[fftSamples];
extern float weighingFactors[fftSamples];

constexpr int audioSpectrumBins = 17;
constexpr int audioSpectrumMaxFreq = 3000;
constexpr int audioSpectrumBinWidth = audioSpectrumMaxFreq / audioSpectrumBins;
constexpr int audioSpectrumHistorySize = 3;
constexpr int audioSpectrumBinSize = (fftSamples / 4) / audioSpectrumBins;
extern std::deque<std::vector<float>> audioSpectrum;
extern SemaphoreHandle_t audioSpectrumSemaphore;

extern std::unique_ptr<ArduinoFFT<float>> FFT;

constexpr int prevMaxesToKeep = 200;
extern std::deque<float> prevMaxes;

void initialiseFFT();

void read_data_stream(const uint8_t *data, uint32_t length);

#endif // audiofft_h