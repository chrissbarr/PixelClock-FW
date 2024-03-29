#ifdef PIXELCLOCK_DESKTOP

/* Project Scope */
#include "audio/desktop.h"
#include "FMTWrapper.h"
#include "utility.h"

/* Libraries */
#include <SFML/Audio.hpp>
#include <arduinoFFT.h>
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <cmath>
#include <numeric>

void AudioDesktop::begin() {

    FFT = std::make_unique<ArduinoFFT<float>>(vReal, vImag, fftSamples, float(fftSampleFreq), weighingFactors);

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
    recorder->setChannelCount(2);
    recorder->start(44100);
}

void AudioDesktop::a2dp_callback(const uint8_t* data, uint32_t length) {

    traceCallbackTotal.start();

    // printing::print(fmt::format("Channels: {}\n", recorder->getChannelCount()));

    int16_t* samples = (int16_t*)data;
    uint32_t sample_count = length / 2;

    for (uint32_t i = 0; i < sample_count; i++) { audioSamplesBuffer.push(samples[i]); }

    traceCallbackTotal.stop();
}

void AudioDesktop::update() {

    const int samplesToProcess = 2048;
    while (audioSamplesBuffer.size() >= samplesToProcess) {

        auto& samples = audioSamplesBuffer;

        traceCallbackVolume.start();
        float vLeftAvg = 0;
        float vRightAvg = 0;
        constexpr float fullscaleDiv = 1.0 / 32768;

        // calculate average RMS magnitude for L/R channels
        for (uint32_t i = 0; i < samplesToProcess; i += 2) {
            vLeftAvg += samples[i] * samples[i];
            vRightAvg += samples[i + 1] * samples[i + 1];
        }
        int lrSamplesDiv2 = samplesToProcess / 4;
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

        traceCallbackVolume.stop();
        traceCallbackFFT.start();

        int sourceIdx = 0;
        for (uint32_t i = 0; i < fftSamples; i++) {

            if (sourceIdx < samplesToProcess) {
                // convert stereo samples to mono
                vReal[i] = static_cast<float>((uint32_t(samples[sourceIdx]) + samples[sourceIdx + 1]) / 2);
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

        traceCallbackFFT.stop();
        traceCallbackSpectrum.start();

        // Fill the audioSpectrum vector with data.
        auto spectrum = etl::array<float, audioSpectrumBins>();
        spectrum.fill(0);

        float prevMax = 0.0;
        if (!audioCharacteristics.empty()) { prevMax = audioCharacteristics.back().spectrumMax; }

        // Bin FFT results
        for (int i = 5; i < (fftSamples / 2) - 1; i++) {
            float freq = static_cast<float>(i * fftFrequencyResolution);
            int binIdx = static_cast<int>(std::floor(freq / audioSpectrumBinWidth));
            // int binIdx = i / audioSpectrumBinSize;
            if (binIdx < spectrum.size()) {
                float val = vReal[i] / audioSpectrumBinSize;

                // basic noise filter
                if (val > prevMax * 0.02) { spectrum[binIdx] += val; }
            }
        }

        float maxThisTime = *std::max_element(spectrum.begin(), spectrum.end());
        float avgMax = utility::sum_members(audioCharacteristics, &AudioCharacteristics::spectrumMax) /
                       audioCharacteristics.size();

        float maxScale = 6000;
        float scaleFactor = maxScale / avgMax;

        std::transform(
            spectrum.begin(),
            spectrum.end(),
            spectrum.begin(),
            std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));

        traceCallbackSpectrum.stop();

        AudioCharacteristics c{};
        c.volumeLeft = vLeftAvg;
        c.volumeRight = vRightAvg;
        c.spectrumMax = maxThisTime;
        c.spectrum = spectrum;

        audioCharacteristics.push(c);

        samples.pop(samplesToProcess);
    }
}

std::vector<InstrumentationTrace*> AudioDesktop::getInstrumentation() {
    std::vector<InstrumentationTrace*> vec;
    vec.reserve(4);
    vec.push_back(&traceCallbackTotal);
    vec.push_back(&traceCallbackVolume);
    vec.push_back(&traceCallbackFFT);
    vec.push_back(&traceCallbackSpectrum);
    return vec;
}

#endif