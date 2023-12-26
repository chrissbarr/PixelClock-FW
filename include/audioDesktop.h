#ifndef audioesp32_h
#define audioesp32_h

/* Project Scope */
#include "audio.h"
// #include "FMTWrapper.h"
// #include "instrumentation.h"
// #include "utility.h"

/* Libraries */
// #include "arduinoFFT.h"
#include <etl/array.h>
#include <etl/circular_buffer.h>

/* C++ Standard Library */
// #include <memory>

class AudioDesktop : public Audio {
public:
    AudioDesktop() {}
    ~AudioDesktop() {}

    void begin() override final {}
    void update() override final {}
    void a2dp_callback(const uint8_t* data, uint32_t length) override final {}
    void lockMutex() override final {}
    void releaseMutex() override final {}
    etl::icircular_buffer<AudioCharacteristics>& getAudioCharacteristicsHistory() override final {
        return audioCharacteristics;
    }

private:
    // float vReal[fftSamples];
    // float vImag[fftSamples];
    // float weighingFactors[fftSamples];

    etl::circular_buffer<AudioCharacteristics, audioHistorySize> audioCharacteristics;
};

#endif // audioesp32_h