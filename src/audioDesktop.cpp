#ifdef PIXELCLOCK_DESKTOP

/* Project Scope */
#include "audioDesktop.h"
#include "FMTWrapper.h"
#include "instrumentation.h"
#include "pinout.h"
#include "utility.h"

/* Libraries */

#include <SFML/Audio.hpp>
// #include "AudioTools.h"
// #include "BluetoothA2DPSink.h"
//  #define FFT_SPEED_OVER_PRECISION
//  #define FFT_SQRT_APPROXIMATION
// #include <arduinoFFT.h>
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <numeric>
#include <cmath>

void AudioDesktop::begin() {

    FFT = std::make_unique<ArduinoFFT<float>>(vReal, vImag, fftSamples, fftSampleFreq, weighingFactors);

    printing::print("AudioDesktop::begin()!\n");
    std::vector<std::string> availableDevices = sf::SoundRecorder::getAvailableDevices();

    printing::print("Available input devices:\n");
    for (const auto& d : availableDevices) { printing::print(fmt::format("  {}\n", d)); }

    std::string inputDevice = availableDevices.back();

    recorder = std::make_unique<SFMLRecorder>();
    recorder->registerCallback([this](const uint8_t* data, uint32_t length) {
        // printing::print("Calback!\n");
        this->a2dp_callback(data, length);
    });

    if (!recorder->setDevice(inputDevice)) { printing::print("Failed to set SFML audio input device!\n"); }
    recorder->start();
}

void AudioDesktop::a2dp_callback(const uint8_t* data, uint32_t length) {

    //callbackDuration.start();
    //audioDuration.start();

    int16_t* samples = (int16_t*)data;
    uint32_t sample_count = length / 2;

    //i2sOutput->write(data, length);

    //audioDuration.stop();
    //volDuration.start();

    float vLeftAvg = 0;
    float vRightAvg = 0;
    constexpr float fullscaleDiv = 1.0 / 32768;

    // calculate average RMS magnitude for L/R channels
    for (uint32_t i = 0; i < sample_count; i += 2) {
        vLeftAvg += samples[i] * samples[i];
        vRightAvg += samples[i + 1] * samples[i + 1];
    }
    int lrSamplesDiv2 = sample_count / 4;
    vLeftAvg = std::sqrt(vLeftAvg / lrSamplesDiv2) * fullscaleDiv;
    vRightAvg = std::sqrt(vRightAvg / lrSamplesDiv2) * fullscaleDiv;

    auto mag2db = [](float mag) -> float {
        if (mag < 1e-3) {
            return -60.0;
        } else {
            return 20 * std::log10(mag);
        }
    };

    // convert to dB
    vLeftAvg = mag2db(vLeftAvg);
    vRightAvg = mag2db(vRightAvg);

    //volDuration.stop();
    //fftDuration.start();

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
    FFT->windowing(FFTWindow::Hamming, FFTDirection::Forward); /* Weigh data */
    FFT->compute(FFTDirection::Forward);                       /* Compute FFT */
    FFT->complexToMagnitude();                                 /* Compute magnitudes */

    //fftDuration.stop();
    //specDuration.start();

    // Fill the audioSpectrum vector with data.
    auto spectrum = etl::array<float, audioSpectrumBins>();
    spectrum.fill(0);

    float prevMax = 0.0;
    if (!audioCharacteristics.empty()) { prevMax = audioCharacteristics.back().spectrumMax; }

    // Bin FFT results
    for (int i = 5; i < (fftSamples / 2) - 1; i++) {
        float freq = i * fftFrequencyResolution;
        int binIdx = std::floor(freq / audioSpectrumBinWidth);
        // int binIdx = i / audioSpectrumBinSize;
        if (binIdx < spectrum.size()) {
            float val = vReal[i] / audioSpectrumBinSize;

            // basic noise filter
            if (val > prevMax * 0.02) { spectrum[binIdx] += val; }
        }
    }

    float maxThisTime = *std::max_element(spectrum.begin(), spectrum.end());
    float avgMax =
        utility::sum_members(audioCharacteristics, &AudioCharacteristics::spectrumMax) / audioCharacteristics.size();

    float maxScale = 6000;
    float scaleFactor = maxScale / avgMax;

    std::transform(
        spectrum.begin(),
        spectrum.end(),
        spectrum.begin(),
        std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));

    //specDuration.stop();

    AudioCharacteristics c{};
    c.volumeLeft = vLeftAvg;
    c.volumeRight = vRightAvg;
    c.spectrumMax = maxThisTime;
    c.spectrum = spectrum;

    //xSemaphoreTake(audioCharacteristicsSemaphore, portMAX_DELAY);
    audioCharacteristics.push(c);
    //xSemaphoreGive(audioCharacteristicsSemaphore);

    //callbackDuration.stop();
}


#endif