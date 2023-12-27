#ifndef audio_esp32_h
#define audio_esp32_h

/* Project Scope */
#include "audio/audio.h"
#include "FMTWrapper.h"
#include "instrumentation.h"
#include "utility.h"

/* Libraries */
#include "arduinoFFT.h"
#include <etl/circular_buffer.h>
#include <etl/array.h>

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

    void begin() override final;
    void update() override final;
    void a2dp_callback(const uint8_t* data, uint32_t length) override final;
    void lockMutex() override final;
    void releaseMutex() override final;
    etl::icircular_buffer<AudioCharacteristics>& getAudioCharacteristicsHistory() override final { return *audioCharacteristics; }

private:

    std::unique_ptr<ArduinoFFT<float>> FFT;
    std::unique_ptr<BluetoothA2DPSink> a2dpSink;
    std::unique_ptr<audio_tools::I2SStream> i2sOutput;
    float vReal[fftSamples];
    float vImag[fftSamples];
    float weighingFactors[fftSamples];

    SemaphoreHandle_t audioCharacteristicsSemaphore;

    InstrumentationTrace callbackDuration;
    InstrumentationTrace audioDuration;
    InstrumentationTrace volDuration;
    InstrumentationTrace fftDuration;
    InstrumentationTrace specDuration;

    uint32_t statReportInterval = 5000;
    uint32_t statReportLastTime = 0;

    AudioCharacteristics* acBuf;
    etl::icircular_buffer<AudioCharacteristics>* audioCharacteristics;
};

#endif // audio_esp32_h