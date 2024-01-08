/* Project Scope */
#include "audio/ESP32.h"
#include "FMTWrapper.h"
#include "instrumentation.h"
#include "pinout.h"
#include "utility.h"

/* Libraries */
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include <arduinoFFT.h>
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <numeric>

void read_data_stream(const uint8_t* data, uint32_t length) { AudioSingleton::get().a2dp_callback(data, length); }

void AudioESP32::a2dp_callback(const uint8_t* data, uint32_t length) {

    traceCallbackTotal.start();
    traceCallbackI2S.start();
    i2sOutput->write(data, length);
    traceCallbackI2S.stop();

    traceCallbackBuffer.start();
    int16_t* samples = (int16_t*)data;
    const uint32_t sample_count = length / 2;
    for (uint32_t i = 0; i < sample_count; i++) { audioBuffer->push(samples[i]); }
    traceCallbackBuffer.stop();

    if (audioProcessingTaskHandle) {
        xTaskNotifyGive(audioProcessingTaskHandle);
    }

    traceCallbackTotal.stop();
}

AudioESP32::AudioESP32() {

    acBuf = (AudioCharacteristics*)ps_calloc(audioHistorySize + 1, sizeof(AudioCharacteristics));
    if (acBuf) {
        printing::print("PSRAM buffer allocation success!\n");
        audioCharacteristics = new etl::circular_buffer_ext<AudioCharacteristics>(acBuf, audioHistorySize);
    } else {
        printing::print("PSRAM buffer allocation fail!\n");
        audioCharacteristics = new etl::circular_buffer<AudioCharacteristics, audioHistorySize>();
    }

    audioBufferRaw = (int16_t*)ps_calloc(audioBufferSize + 1, sizeof(int16_t));
    if (audioBufferRaw) {
        printing::print("PSRAM buffer allocation success!\n");
    } else {
        printing::print("PSRAM buffer allocation fail!\n");
        audioBufferRaw = (int16_t*)calloc(audioBufferSize + 1, sizeof(int16_t));
    }
    audioBuffer = new etl::circular_buffer_ext<int16_t>(audioBufferRaw, audioBufferSize);

    traces.reserve(6);
    traces.push_back(&traceCallbackTotal);
    traces.push_back(&traceCallbackI2S);
    traces.push_back(&traceCallbackBuffer);
    traces.push_back(&traceProcessVol);
    traces.push_back(&traceProcessFFT);
    traces.push_back(&traceProcessSpectrum);

    printing::print("Creating audio processing task... ");
    auto taskCreated = xTaskCreatePinnedToCore(
        [](void* o) {
            while (1) { static_cast<AudioESP32*>(o)->audioProcessingTask(); }
        },                          // Function to implement the task
        "AudioUpdate",              // Name of the task
        2048,                       // Stack size in words
        this,                       // Task input parameter
        10,                         // Priority of the task
        &audioProcessingTaskHandle, // Task handle.
        0                           // Core where the task should run
    );

    if (taskCreated == pdPASS) {
        printing::print("success!\n");
    } else {
        printing::print("failure!\n");
    }
}

AudioESP32::~AudioESP32() {

    if (acBuf) { free(acBuf); }
    free(audioCharacteristics);

    if (audioBufferRaw) { free(audioBufferRaw); }
    free(audioBuffer);
}

void AudioESP32::begin() {
    printing::print("Audio.begin()\n");
    i2sOutput = std::make_unique<I2SStream>(0);
    a2dpSink = std::make_unique<BluetoothA2DPSink>();

    auto cfg = i2sOutput->defaultConfig();
    cfg.pin_data = pins::i2sDout;
    cfg.pin_bck = pins::i2sBclk;
    cfg.pin_ws = pins::i2sWclk;
    cfg.buffer_count = 5;
    cfg.buffer_size = 1024;
    // cfg.sample_rate = a2dpSink->sample_rate();
    cfg.channels = 2;
    cfg.bits_per_sample = 16;
    cfg.use_apll = false;
    cfg.rx_tx_mode = audio_tools::RxTxMode::TX_MODE, cfg.sample_rate = 44100,
    cfg.i2s_format = audio_tools::I2SFormat::I2S_STD_FORMAT, cfg.auto_clear = true;
    i2sOutput->begin(cfg);

    printing::print("after i2s\n");

    a2dpSink->set_avrc_metadata_callback([](uint8_t id, const uint8_t* text) { avrc_metadata_callback(id, text); });
    a2dpSink->set_stream_reader(read_data_stream, false);
    a2dpSink->start("MyMusic");

    printing::print("after a2dp\n");

    FFT = std::make_unique<ArduinoFFT<float>>(vReal, vImag, fftSamples, fftSampleFreq, weighingFactors);

    audioCharacteristicsSemaphore = xSemaphoreCreateMutex();
}

void AudioESP32::audioProcessingTask() {

    uint32_t thread_notification = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // printing::print("audio task notified...\n");
    // printing::print(fmt::format("AudioESP32::audioProcessingTask() - buffer = {}\n", audioBuffer->size()));

    const int samplesToProcess = 2048;
    while (audioBuffer->size() >= samplesToProcess) {

        // printing::print(fmt::format(
        //     "AudioESP32::audioProcessingTask(), buffer {} >= {}, processing buffer...\n", audioBuffer->size(),
        //     samplesToProcess));

        auto& samples = *audioBuffer;

        traceProcessVol.start();

        float vLeftAvg = 0;
        float vRightAvg = 0;

        // calculate average RMS magnitude for L/R channels
        for (uint32_t i = 0; i < samplesToProcess; i += 2) {
            vLeftAvg += samples[i] * samples[i];
            vRightAvg += samples[i + 1] * samples[i + 1];
        }

        constexpr float fullscaleDiv = 1.0 / 32768;
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

        traceProcessVol.stop();
        traceProcessFFT.start();

        int sourceIdx = 0;
        for (uint32_t i = 0; i < fftSamples; i++) {

            if (sourceIdx < samplesToProcess) {
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

        traceProcessFFT.stop();
        traceProcessSpectrum.start();

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
        float avgMax = utility::sum_members(*audioCharacteristics, &AudioCharacteristics::spectrumMax) /
                       audioCharacteristics->size();

        float maxScale = 6000;
        float scaleFactor = maxScale / avgMax;

        std::transform(
            spectrum.begin(),
            spectrum.end(),
            spectrum.begin(),
            std::bind(std::multiplies<float>(), std::placeholders::_1, scaleFactor));

        traceProcessSpectrum.stop();

        AudioCharacteristics c{};
        c.volumeLeft = vLeftAvg;
        c.volumeRight = vRightAvg;
        c.spectrumMax = maxThisTime;
        c.spectrum = spectrum;

        xSemaphoreTake(audioCharacteristicsSemaphore, portMAX_DELAY);
        audioCharacteristics->push(c);
        xSemaphoreGive(audioCharacteristicsSemaphore);

        samples.pop(samplesToProcess);
    }

    // printing::print(fmt::format(
    //     "AudioESP32::audioProcessingTask(), stack high water mark = {}\n", uxTaskGetStackHighWaterMark(nullptr)));
}

void AudioESP32::update() {

    if (millis() - statReportLastTime > statReportInterval) {

        printing::print(fmt::format(
            "Volume: L={:.1f} R={:.1f}\n",
            audioCharacteristics->back().volumeLeft,
            audioCharacteristics->back().volumeRight));

        statReportLastTime = millis();
    }
}

void AudioESP32::lockMutex() { xSemaphoreTake(audioCharacteristicsSemaphore, portMAX_DELAY); }
void AudioESP32::releaseMutex() { xSemaphoreGive(audioCharacteristicsSemaphore); }

std::vector<InstrumentationTrace*> AudioESP32::getInstrumentation() { return traces; }