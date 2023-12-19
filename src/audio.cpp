#include "audio.h"
#include "pinout.h"

#include "BluetoothA2DPSink.h"
#include "AudioTools.h"

//#define FFT_SPEED_OVER_PRECISION
//#define FFT_SQRT_APPROXIMATION
#include <arduinoFFT.h>

#include <melody_player.h>
#include <melody_factory.h>

#include <numeric>

void read_data_stream(const uint8_t *data, uint32_t length)
{
  Audio::get().a2dp_callback(data, length);
}

void Audio::a2dp_callback(const uint8_t *data, uint32_t length)
{
  uint32_t callbackStart = micros();

  int16_t *samples = (int16_t*) data;
  uint32_t sample_count = length/2;

  i2sOutput->write(data, length);

  float vLeftAvg = 0;
  float vRightAvg = 0;

  // calculate average RMS magnitude for L/R channels
  for (uint32_t i = 0; i < sample_count; i+=2) {
    vLeftAvg += samples[i] * samples[i];
    vRightAvg += samples[i + 1] * samples[i + 1];
  }
  int lrSamples = sample_count / 2;
  vLeftAvg = std::sqrt(vLeftAvg / lrSamples);
  vRightAvg = std::sqrt(vRightAvg / lrSamples);

  // convert to decibels
  vLeftAvg = 20 * std::log10(vLeftAvg);
  vRightAvg = 20 * std::log10(vRightAvg);

  // store avg volume in history
  volumeHistory.push_back({vLeftAvg, vRightAvg});
  if (volumeHistory.size() > volumeHistorySize) {
    volumeHistory.pop_front();
  }

  uint32_t fftStart = micros();

  int sourceIdx = 0;
  for (uint32_t i = 0; i < fftSamples; i++) {
    
    if (sourceIdx < sample_count) { 
      // convert stereo samples to mono
      vReal[i] = (uint32_t(samples[sourceIdx]) + samples[sourceIdx + 1]) / 2;
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
  auto spectrum = std::vector<float>(audioSpectrumBins, 0);

  float prevMax = 0.0;
  if (!prevMaxes.empty()) {
    prevMax = prevMaxes.back();
  }

  // Bin FFT results
  for (int i = 5; i < (fftSamples / 2) - 1; i++) {
    float freq = i * fftFrequencyResolution;
    int binIdx = std::floor(freq / audioSpectrumBinWidth);
    //Serial.printf("%d\t%f\n", i, vReal[i]);
    //int binIdx = i / audioSpectrumBinSize;
    if (binIdx < spectrum.size()) {
      float val = vReal[i] / audioSpectrumBinSize;

      // basic noise filter
      if (val > prevMax * 0.02) {
        spectrum[binIdx] += val;
      }
    }
  }

  float maxThisTime = *std::max_element(spectrum.begin(), spectrum.end());
  prevMaxes.push_back(maxThisTime);
  if (prevMaxes.size() > prevMaxesToKeep) { prevMaxes.pop_front(); }
  float avgMax = std::accumulate(prevMaxes.begin(), prevMaxes.end(), 0.0) / prevMaxes.size();

  float maxScale = 6000;
  float scaleFactor = maxScale / avgMax;
  //Serial.printf("Scale factor: %f\n", scaleFactor);

  std::transform(spectrum.begin(), spectrum.end(), spectrum.begin(),
    std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));
  xSemaphoreTake(audioSpectrumSemaphore, portMAX_DELAY);
  audioSpectrum.push_back(spectrum);
  if (audioSpectrum.size() > audioSpectrumHistorySize) { audioSpectrum.pop_front(); }
  xSemaphoreGive(audioSpectrumSemaphore);

  uint32_t fftEnd = micros();
  uint32_t callbackEnd = micros();

  uint32_t callbackDuration = callbackEnd - callbackStart;
  uint32_t fftDuration = fftEnd - fftStart;

  callbackDiagnostics.push_back({callbackDuration, fftDuration, sample_count});
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
  cfg.pin_data = pins::i2sDout;
  cfg.pin_bck = pins::i2sBclk;
  cfg.pin_ws = pins::i2sWclk;
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

  Serial.println("Begin melody library generation...");
  melodies["status_button"] = MelodyFactory.loadRtttlString("status_button: d=16,o=5,b=112:f");
  melodies["status_good"] = MelodyFactory.loadRtttlString("status_button: d=16,o=5,b=112:c,f");
  melodies["status_bad"] = MelodyFactory.loadRtttlString("status_button: d=16,o=5,b=112:f,c");

  melodies["music_pokemon"] = MelodyFactory.loadRtttlString("Pokemon:d=16,o=5,b=112:32p,f,a#,c6,c#6,c6,c#6,d#6,2f6,a#,c6,8c#6,8f6,8d#6,32c#.6,32d#.6,32c#.6,8c6,8g#.,f,a#,c6,c#6,c6,c#6,d#6,2f6,8a#,c#6,8f6,a,d#6,4g#6");
  melodies["music_tetris"] = MelodyFactory.loadRtttlString("korobyeyniki:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a");
  Serial.println("Completed melody library generation!");

  melodyPlayer = std::make_unique<MelodyPlayer>(pins::buzzer);
  melodyPlayer->playAsync(melodies["music_tetris"]);
}

void Audio::update()
{
  if (millis() - statReportLastTime > statReportInterval && !callbackDiagnostics.empty()) {

    float callbackAvg = 0;
    uint16_t callbackMin = std::numeric_limits<uint16_t>::max();
    uint16_t callbackMax = 0;

    float fftAvg = 0;
    uint16_t fftMin = std::numeric_limits<uint16_t>::max();
    uint16_t fftMax = 0;

    float samplesAvg = 0;
    uint16_t samplesMin = std::numeric_limits<uint16_t>::max();
    uint16_t samplesMax = 0;

    for (const auto& stats : callbackDiagnostics) {
      callbackAvg += stats.callbackDuration;
      if (stats.callbackDuration > callbackMax) { callbackMax = stats.callbackDuration; }
      if (stats.callbackDuration < callbackMin) { callbackMin = stats.callbackDuration; }

      fftAvg += stats.fftDuration;
      if (stats.fftDuration > fftMax) { fftMax = stats.fftDuration; }
      if (stats.fftDuration < fftMin) { fftMin = stats.fftDuration; }

      samplesAvg += stats.sampleCount;
      if (stats.sampleCount > samplesMax) { samplesMax = stats.sampleCount; }
      if (stats.sampleCount < samplesMin) { samplesMin = stats.sampleCount; }
    }

    callbackAvg = callbackAvg / callbackDiagnostics.size();
    fftAvg = fftAvg / callbackDiagnostics.size();
    samplesAvg = samplesAvg / callbackDiagnostics.size();
    callbackDiagnostics.clear();

    Serial.printf("A2DP Callback Statistics (Min - Max - Avg): %d - %d - %.2f \n", callbackMin, callbackMax, callbackAvg);
    Serial.printf("A2DP FFT Statistics      (Min - Max - Avg): %d - %d - %.2f \n", fftMin, fftMax, fftAvg);    
    Serial.printf("A2DP Sample Count        (Min - Max - Avg): %d - %d - %.2f \n", samplesMin, samplesMax, samplesAvg);

    statReportLastTime = millis();
  }
}

void Audio::playStatusSound(StatusSound sound)
{
  switch (sound) {
    case StatusSound::ButtonClick:
      melodyPlayer->playAsync(melodies["status_button"]);
      break;
    case StatusSound::Confirm:
      melodyPlayer->playAsync(melodies["status_good"]);
      break;
    case StatusSound::Cancel:
      melodyPlayer->playAsync(melodies["status_bad"]);
      break;
  }

}
