#ifndef audio_desktop_h
#define audio_desktop_h

#ifdef PIXELCLOCK_DESKTOP

/* Project Scope */
#include "audio/audio.h"

/* Libraries */
#include <SFML/Audio.hpp>
#include <arduinoFFT.h>
#include <etl/array.h>
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <functional>
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

    void registerCallback(std::function<void(const uint8_t* data, uint32_t length)> newCallback) {
        this->callback = newCallback;
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

    // Instrumentation
    std::vector<InstrumentationTrace*> getInstrumentation() override final;

private:
    float vReal[fftSamples];
    float vImag[fftSamples];
    float weighingFactors[fftSamples];

    std::unique_ptr<ArduinoFFT<float>> FFT;

    std::unique_ptr<SFMLRecorder> recorder;

    etl::circular_buffer<AudioCharacteristics, audioHistorySize> audioCharacteristics;

    InstrumentationTrace traceCallbackTotal{"Audio Callback - Overall"};
    InstrumentationTrace traceCallbackVolume{"Audio Callback - Vol"};
    InstrumentationTrace traceCallbackFFT{"Audio Callback - FFT"};
    InstrumentationTrace traceCallbackSpectrum{"Audio Callback - Spectrum"};
};

#endif

#endif // audio_desktop_h