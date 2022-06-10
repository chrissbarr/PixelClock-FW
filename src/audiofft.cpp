#include "audiofft.h"

#include <numeric>

float vReal[fftSamples];
float vImag[fftSamples];
float weighingFactors[fftSamples];
std::vector<float> audioSpectrum;
SemaphoreHandle_t audioSpectrumSemaphore;

std::unique_ptr<ArduinoFFT<float>> FFT;
std::deque<float> prevMaxes;

void initialiseFFT()
{
    FFT = std::make_unique<ArduinoFFT<float>>(vReal, vImag, fftSamples, fftSampleFreq, weighingFactors);
}

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


  FFT->dcRemoval();
  FFT->windowing(FFTWindow::Hamming, FFTDirection::Forward);	/* Weigh data */
  FFT->compute(FFTDirection::Forward); /* Compute FFT */
  FFT->complexToMagnitude(); /* Compute magnitudes */

  // Fill the audioSpectrum vector with data. 
  xSemaphoreTake(audioSpectrumSemaphore, portMAX_DELAY);
  audioSpectrum = std::vector<float>(audioSpectrumBins, 0);

  // Analyse FFT results
  for (int i = 2; i < (fftSamples/4); i++) {
    int binIdx = i / audioSpectrumBinSize;
    if (binIdx < audioSpectrum.size()) {
      audioSpectrum[binIdx] += vReal[i] / audioSpectrumBinSize;
    }
  }

  float maxThisTime = *std::max_element(audioSpectrum.begin(), audioSpectrum.end());
  prevMaxes.push_back(maxThisTime);
  if (prevMaxes.size() > prevMaxesToKeep) { prevMaxes.pop_front(); }
  float avgMax = std::accumulate(prevMaxes.begin(), prevMaxes.end(), 0.0) / prevMaxes.size();

  float maxScale = 4000;
  float scaleFactor = maxScale / avgMax;
  Serial.printf("Scale factor: %f\n", scaleFactor);

  std::transform(audioSpectrum.begin(), audioSpectrum.end(), audioSpectrum.begin(),
    std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));
  
  xSemaphoreGive(audioSpectrumSemaphore);
}