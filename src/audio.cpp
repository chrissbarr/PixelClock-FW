/* Project Scope */
#include "audio.h"
#include "FMTWrapper.h"
#include "instrumentation.h"
#include "pinout.h"
#include "utility.h"

/* Libraries */
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
// #define FFT_SPEED_OVER_PRECISION
// #define FFT_SQRT_APPROXIMATION
#include <arduinoFFT.h>
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <numeric>

void read_data_stream(const uint8_t* data, uint32_t length) { Audio::get().a2dp_callback(data, length); }

void Audio::a2dp_callback(const uint8_t* data, uint32_t length) {

    callbackDuration.start();
    audioDuration.start();

    int16_t* samples = (int16_t*)data;
    uint32_t sample_count = length / 2;

    i2sOutput->write(data, length);

    audioDuration.stop();
    volDuration.start();

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

    volDuration.stop();
    fftDuration.start();

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

    fftDuration.stop();
    specDuration.start();

    // Fill the audioSpectrum vector with data.
    auto spectrum = etl::array<float, audioSpectrumBins>();
    spectrum.fill(0);

    float prevMax = 0.0;
    if (!audioCharacteristics->empty()) { prevMax = audioCharacteristics->back().spectrumMax; }

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
        utility::sum_members(*audioCharacteristics, &AudioCharacteristics::spectrumMax) / audioCharacteristics->size();

    float maxScale = 6000;
    float scaleFactor = maxScale / avgMax;

    std::transform(
        spectrum.begin(),
        spectrum.end(),
        spectrum.begin(),
        std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));

    specDuration.stop();

    AudioCharacteristics c{};
    c.volumeLeft = vLeftAvg;
    c.volumeRight = vRightAvg;
    c.spectrumMax = maxThisTime;
    c.spectrum = spectrum;

    xSemaphoreTake(audioCharacteristicsSemaphore, portMAX_DELAY);
    audioCharacteristics->push(c);
    xSemaphoreGive(audioCharacteristicsSemaphore);

    callbackDuration.stop();
}

Audio::Audio() {

    acBuf = (AudioCharacteristics*)ps_calloc(audioHistorySize + 1, sizeof(AudioCharacteristics));
    if (acBuf) {
        Serial.println("PSRAM buffer allocation success!");
        audioCharacteristics = new etl::circular_buffer_ext<AudioCharacteristics>(acBuf, audioHistorySize);
    } else {
        Serial.println("PSRAM buffer allocation fail!");
        audioCharacteristics = new etl::circular_buffer<AudioCharacteristics, audioHistorySize>();
    }
}

Audio::~Audio() {

    if (acBuf) { free(acBuf); }
    free(audioCharacteristics);
}

void Audio::begin() {
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
    // cfg.sample_rate = a2dpSink->sample_rate();
    cfg.channels = 2;
    cfg.bits_per_sample = 16;
    cfg.use_apll = false;
    cfg.rx_tx_mode = audio_tools::RxTxMode::TX_MODE, cfg.sample_rate = 44100,
    cfg.i2s_format = audio_tools::I2SFormat::I2S_STD_FORMAT, cfg.auto_clear = true;
    i2sOutput->begin(cfg);

    Serial.println("after i2s");

    a2dpSink->set_avrc_metadata_callback([](uint8_t id, const uint8_t* text) { avrc_metadata_callback(id, text); });
    a2dpSink->set_stream_reader(read_data_stream, false);
    a2dpSink->start("MyMusic");

    Serial.println("after a2dp");

    FFT = std::make_unique<ArduinoFFT<float>>(vReal, vImag, fftSamples, fftSampleFreq, weighingFactors);

    audioCharacteristicsSemaphore = xSemaphoreCreateMutex();
}

void Audio::update() {

    if (millis() - statReportLastTime > statReportInterval) {

        auto printAndReset = [](std::string name, InstrumentationTrace& t) {
            printing::print(Serial, formatInstrumentationTrace(name, t));
            t.reset();
        };

        printAndReset("Audio Callback - Total", callbackDuration);
        printAndReset("Audio Callback - I2S", audioDuration);
        printAndReset("Audio Callback - Vol", volDuration);
        printAndReset("Audio Callback - FFT", fftDuration);
        printAndReset("Audio Callback - Spectrum", specDuration);

        printing::print(
            Serial,
            fmt::format(
                "Volume: L={:.1f} R={:.1f}\n",
                audioCharacteristics->back().volumeLeft,
                audioCharacteristics->back().volumeRight));

        statReportLastTime = millis();
    }
}