#include "audio.h"
#include "pinout.h"

#include "BluetoothA2DPSink.h"
#include "AudioTools.h"

// #define FFT_SPEED_OVER_PRECISION
// #define FFT_SQRT_APPROXIMATION
#include <arduinoFFT.h>

#include <numeric>

void read_data_stream(const uint8_t *data, uint32_t length)
{
  Audio::get().a2dp_callback(data, length);
}

void Audio::a2dp_callback(const uint8_t *data, uint32_t length)
{
  int16_t *samples = (int16_t*) data;
  uint32_t sample_count = length/2;

  i2sOutput->write(data, length);

  // int sourceIdx = 0;
  // for (uint32_t i = 0; i < fftSamples; i++) {
    
  //   if (sourceIdx < sample_count) { 
  //     vReal[i] = samples[sourceIdx];
  //   } else {
  //     vReal[i] = 0;
  //   }
  //   vImag[i] = 0;
  //   sourceIdx += 2;
  // }

  // FFT->dcRemoval();
  // FFT->windowing(FFTWindow::Hamming, FFTDirection::Forward);	/* Weigh data */
  // FFT->compute(FFTDirection::Forward); /* Compute FFT */
  // FFT->complexToMagnitude(); /* Compute magnitudes */

  // // Fill the audioSpectrum vector with data. 
  // auto spectrum = std::vector<float>(audioSpectrumBins, 0);

  // float prevMax = 0.0;
  // if (!prevMaxes.empty()) {
  //   prevMax = prevMaxes.back();
  // }

  // // Bin FFT results
  // for (int i = 5; i < (fftSamples / 2) - 1; i++) {
  //   float freq = i * fftFrequencyResolution;
  //   int binIdx = std::floor(freq / audioSpectrumBinWidth);
  //   //Serial.printf("%d\t%f\n", i, vReal[i]);
  //   //int binIdx = i / audioSpectrumBinSize;
  //   if (binIdx < spectrum.size()) {
  //     float val = vReal[i] / audioSpectrumBinSize;

  //     // basic noise filter
  //     if (val > prevMax * 0.02) {
  //       spectrum[binIdx] += val;
  //     }
  //   }
  // }

  // float maxThisTime = *std::max_element(spectrum.begin(), spectrum.end());
  // prevMaxes.push_back(maxThisTime);
  // if (prevMaxes.size() > prevMaxesToKeep) { prevMaxes.pop_front(); }
  // float avgMax = std::accumulate(prevMaxes.begin(), prevMaxes.end(), 0.0) / prevMaxes.size();

  // float maxScale = 6000;
  // float scaleFactor = maxScale / avgMax;
  // //Serial.printf("Scale factor: %f\n", scaleFactor);

  // std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(),
  //   std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));
  // xSemaphoreTake(audioSpectrumSemaphore, portMAX_DELAY);
  // audioSpectrum.push_back(spectrum);
  // if (audioSpectrum.size() > audioSpectrumHistorySize) { audioSpectrum.pop_front(); }
  // xSemaphoreGive(audioSpectrumSemaphore);
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
  cfg.buffer_count = 10;
  cfg.buffer_size = 1024;
  //cfg.sample_rate = a2dpSink->sample_rate();
  cfg.channels = 2;
  cfg.bits_per_sample = 16;
  cfg.use_apll = false;
  cfg.rx_tx_mode = audio_tools::RxTxMode::TX_MODE,
  cfg.sample_rate = 44100,
  cfg.i2s_format = audio_tools::I2SFormat::I2S_STD_FORMAT,
  cfg.auto_clear = true;
  i2sOutput->begin(cfg);

  Serial.println("after i2s");


  a2dpSink->set_avrc_metadata_callback([](uint8_t id, const uint8_t *text){ avrc_metadata_callback(id, text); });
  a2dpSink->set_stream_reader(read_data_stream, false);
  a2dpSink->start("MyMusic");

  Serial.println("after a2dp");

  FFT = std::make_unique<ArduinoFFT<float>>(vReal, vImag, fftSamples, fftSampleFreq, weighingFactors);

  audioSpectrumSemaphore = xSemaphoreCreateMutex();
}