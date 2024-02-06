#ifndef audio_esp32_h
#define audio_esp32_h

/* Project Scope */
#include "FMTWrapper.h"
#include "audio/audio.h"
#include "instrumentation.h"
#include "utility.h"

/* Libraries */
#include "arduinoFFT.h"
#include <TAS5822.h>
#include <etl/array.h>
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <memory>

inline void avrc_metadata_callback(uint8_t id, const uint8_t* text) {
    printing::print(fmt::format("==> AVRC metadata rsp: attribute id {:#X}, {}\n", id, (char*)(text)));
}

// class ArduinoFFT;
class BluetoothA2DPSink;
namespace audio_tools {
class I2SStream;
}

void read_data_stream(const uint8_t* data, uint32_t length);

class AudioESP32 : public Audio {
public:
    AudioESP32();
    ~AudioESP32();

    // Audio methods
    void begin() override final;
    void update() override final;
    void a2dp_callback(const uint8_t* data, uint32_t length) override final;
    void lockMutex() override final;
    void releaseMutex() override final;
    etl::icircular_buffer<AudioCharacteristics>& getAudioCharacteristicsHistory() override final {
        return *audioCharacteristics;
    }

    // Instrumentation
    std::vector<InstrumentationTrace*> getInstrumentation() override final;

    void audioProcessingTask();

private:
    std::unique_ptr<ArduinoFFT<float>> FFT;
    std::unique_ptr<BluetoothA2DPSink> a2dpSink;
    std::unique_ptr<audio_tools::I2SStream> i2sOutput;
    float vReal[fftSamples];
    float vImag[fftSamples];
    float weighingFactors[fftSamples];

    SemaphoreHandle_t audioCharacteristicsSemaphore;

    uint32_t statReportInterval = 5000;
    uint32_t statReportLastTime = 0;

    AudioCharacteristics* acBuf;
    etl::icircular_buffer<AudioCharacteristics>* audioCharacteristics;

    std::size_t audioBufferSize = 8192;
    int16_t* audioBufferRaw;
    etl::icircular_buffer<int16_t>* audioBuffer;

    // Amplifier
    std::unique_ptr<TAS5822::TAS5822<TwoWire>> amplifier;

    // Instrumentation
    InstrumentationTrace traceCallbackTotal{"Audio Callback - Overall"};
    InstrumentationTrace traceCallbackI2S{"Audio Callback - I2S"};
    InstrumentationTrace traceCallbackBuffer{"Audio Callback - Buffer Fill"};
    InstrumentationTrace traceProcessVol{"Audio Processing - Vol"};
    InstrumentationTrace traceProcessFFT{"Audio Processing - FFT"};
    InstrumentationTrace traceProcessSpectrum{"Audio Processing - Spectrum"};
    std::vector<InstrumentationTrace*> traces;

    TaskHandle_t audioProcessingTaskHandle = nullptr;
};

#endif // audio_esp32_h