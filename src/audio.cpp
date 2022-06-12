#include "audio.h"
#include "pinout.h"

#include "BluetoothA2DPSink.h"
#include "AudioTools.h"

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

extern std::unique_ptr<Audio> audio;

void read_data_stream(const uint8_t *data, uint32_t length)
{
  int16_t *samples = (int16_t*) data;
  uint32_t sample_count = length/2;

  audio->i2sOutput->write(data, length);

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

Audio::Audio() {}
Audio::~Audio() {}

void Audio::begin()
{
  Serial.println("Audio.begin()");
  i2sOutput = std::make_unique<I2SStream>(0);
  a2dpSink = std::make_unique<BluetoothA2DPSink>();

  Serial.println("after constructors");

  auto cfg = i2sOutput->defaultConfig();
  cfg.pin_data = dout;
  cfg.pin_bck = bclk;
  cfg.pin_ws = wclk;
  cfg.buffer_count = 8;
  cfg.buffer_size = 1024;
  cfg.sample_rate = a2dpSink->sample_rate();
  cfg.channels = 2;
  cfg.bits_per_sample = 16;
  i2sOutput->begin(cfg);

  Serial.println("after i2s");


  a2dpSink->set_avrc_metadata_callback([](uint8_t id, const uint8_t *text){ avrc_metadata_callback(id, text); });
  a2dpSink->set_stream_reader(read_data_stream, false);
  a2dpSink->start("MyMusic");

  Serial.println("after a2dp");


}