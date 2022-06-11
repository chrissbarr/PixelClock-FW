#ifndef audiofft_h
#define audiofft_h

#define FFT_SPEED_OVER_PRECISION
#define FFT_SQRT_APPROXIMATION

#include <arduinoFFT.h>

#include <memory>
#include <deque>
#include <vector>

constexpr int fftSamples = 512;
constexpr int fftSampleFreq = 44100 * 1;
constexpr int binWidthHertz = fftSampleFreq / fftSamples;
extern float vReal[fftSamples];
extern float vImag[fftSamples];
extern float weighingFactors[fftSamples];

constexpr int audioSpectrumBins = 17;
constexpr int audioSpectrumHistorySize = 10;
constexpr int audioSpectrumBinSize = (fftSamples / 32) / audioSpectrumBins;
extern std::deque<std::vector<float>> audioSpectrum;
extern SemaphoreHandle_t audioSpectrumSemaphore;

extern std::unique_ptr<ArduinoFFT<float>> FFT;

constexpr int prevMaxesToKeep = 100;
extern std::deque<float> prevMaxes;

void initialiseFFT();

void read_data_stream(const uint8_t *data, uint32_t length);

#endif // audiofft_h