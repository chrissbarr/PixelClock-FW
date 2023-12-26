#ifndef audiodesktop_h
#define audiodesktop_h

#ifdef PIXELCLOCK_DESKTOP

/* Project Scope */
#include "audio.h"
// #include "FMTWrapper.h"
// #include "instrumentation.h"
// #include "utility.h"

#include <SFML/Audio.hpp>

/* Libraries */
#include <arduinoFFT.h>
#include <etl/array.h>
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <functional>
#include <iostream>
#include <memory>

class SFMLRecorder : public sf::SoundRecorder {
public:
    SFMLRecorder() { this->setProcessingInterval(sf::milliseconds(20)); }
    ~SFMLRecorder() {}

    // optional
    virtual bool onStart() { return true; }

    // optional
    virtual void onStop() {}

    virtual bool onProcessSamples(const sf::Int16* samples, std::size_t sampleCount) {
        if (callback) { callback((uint8_t*)(samples), sampleCount); }
        return true;
    }

    void registerCallback(std::function<void(const uint8_t* data, uint32_t length)> callback) {
        this->callback = callback;
    }

private:
    std::function<void(const uint8_t* data, uint32_t length)> callback;
};

class AudioDesktop : public Audio {
public:
    AudioDesktop() {}
    ~AudioDesktop() {
        if (recorder) { recorder->stop(); }
    }

    void begin() override final;
    void update() override final {}
    void a2dp_callback(const uint8_t* data, uint32_t length) override final;
    void lockMutex() override final {}
    void releaseMutex() override final {}
    etl::icircular_buffer<AudioCharacteristics>& getAudioCharacteristicsHistory() override final {
        return audioCharacteristics;
    }

private:
    float vReal[fftSamples];
    float vImag[fftSamples];
    float weighingFactors[fftSamples];

    std::unique_ptr<ArduinoFFT<float>> FFT;

    std::unique_ptr<SFMLRecorder> recorder;

    etl::circular_buffer<AudioCharacteristics, audioHistorySize> audioCharacteristics;
};

#endif

#endif // audiodesktop_h