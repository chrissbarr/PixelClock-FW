#include "audiofft.h"

#include <numeric>

float vReal[fftSamples];
float vImag[fftSamples];
float weighingFactors[fftSamples];
std::deque<std::vector<float>> audioSpectrum;
SemaphoreHandle_t audioSpectrumSemaphore;

std::unique_ptr<ArduinoFFT<float>> FFT;
std::deque<float> prevMaxes;

float prevMax = 0.0;

void initialiseFFT()
{
    FFT = std::make_unique<ArduinoFFT<float>>(vReal, vImag, fftSamples, fftSampleFreq, weighingFactors);
}

void read_data_stream(const uint8_t *data, uint32_t length)
{
  int16_t *samples = (int16_t*) data;
  uint32_t sample_count = length/2;

  int sourceIdx = 0;
  for (uint32_t i = 0; i < fftSamples; i++) {
    
    if (sourceIdx < sample_count) { 
      vReal[i] = samples[sourceIdx];
    } else {
      vReal[i] = 0;
    }
    vImag[i] = 0;
    sourceIdx += 2;
  }

  FFT->dcRemoval();
  FFT->windowing(FFTWindow::Hamming, FFTDirection::Forward);	/* Weigh data */
  FFT->compute(FFTDirection::Forward); /* Compute FFT */
  FFT->complexToMagnitude(); /* Compute magnitudes */

  // Fill the audioSpectrum vector with data. 
  xSemaphoreTake(audioSpectrumSemaphore, portMAX_DELAY);
  audioSpectrum.push_back(std::vector<float>(audioSpectrumBins, 0));
  if (audioSpectrum.size() > audioSpectrumHistorySize) { audioSpectrum.pop_front(); }

  // Bin FFT results
  for (int i = 5; i < (fftSamples / 2) - 1; i++) {
    float freq = i * fftFrequencyResolution;
    int binIdx = std::floor(freq / audioSpectrumBinWidth);
    //Serial.printf("%d\t%f\n", i, vReal[i]);
    //int binIdx = i / audioSpectrumBinSize;
    if (binIdx < audioSpectrum.back().size()) {
      float val = vReal[i] / audioSpectrumBinSize;

      // basic noise filter
      if (val > prevMax * 0.02) {
        audioSpectrum.back()[binIdx] += val;
      }
    }
  }

  float maxThisTime = *std::max_element(audioSpectrum.back().begin(), audioSpectrum.back().end());
  prevMaxes.push_back(maxThisTime);
  if (prevMaxes.size() > prevMaxesToKeep) { prevMaxes.pop_front(); }
  float avgMax = std::accumulate(prevMaxes.begin(), prevMaxes.end(), 0.0) / prevMaxes.size();
  prevMax = avgMax;

  float maxScale = 6000;
  float scaleFactor = maxScale / avgMax;
  //Serial.printf("Scale factor: %f\n", scaleFactor);

  std::transform(audioSpectrum.back().begin(), audioSpectrum.back().end(), audioSpectrum.back().begin(),
    std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));
  
  xSemaphoreGive(audioSpectrumSemaphore);
}