/* Project Scope */
#include "display/effects/spectrumdisplay.h"
#include "audio.h"
#include "display/effects/utilities.h"

/* C++ Standard Library */
#include <algorithm>
#include <functional>
#include <vector>

SpectrumDisplay::SpectrumDisplay(const canvas::Canvas& size) : _c(size) {
    colMin = pixel::CRGB::Blue;
    colMax = pixel::CRGB::Purple;
}

void SpectrumDisplay::reset() { _finished = false; }

canvas::Canvas SpectrumDisplay::run() {
    AudioSingleton::get().lockMutex();

    // average this many spectrums max
    const std::size_t maxSamplesToAvg = 5;
    std::vector<float> totals;
    const auto& hist = AudioSingleton::get().getAudioCharacteristicsHistory();
    if (!hist.empty()) {
        totals = std::vector<float>(hist.back().spectrum.size(), 0);

        // limit averages to maximum and available
        const std::size_t samplesToAvg = std::min(maxSamplesToAvg, hist.size());

        // iterate in reverse order so we always average N latest
        int idx = 0;
        for (auto it = hist.rbegin(); it != hist.rend(); ++it) {
            // add values for this spectrum to total sum
            for (int i = 0; i < it->spectrum.size(); i++) { totals[i] += it->spectrum[i]; }
            idx++;
            if (idx == samplesToAvg) { break; }
        }

        // divide summed values by N samples
        std::transform(
            totals.begin(),
            totals.end(),
            totals.begin(),
            std::bind(std::multiplies<float>(), std::placeholders::_1, 1.0f / samplesToAvg));
    }
    AudioSingleton::get().releaseMutex();

    uint8_t vertMax = _c.getHeight();
    if (!totals.empty()) {
        for (uint8_t x = 0; x < totals.size(); x++) {
            if (x >= _c.getWidth()) { break; }
            // we need to scale the value from 0 - valMax to 0 - vertMax
            auto barHeight = calculateBarHeight(totals[x], 0, maxScale, vertMax);
            for (int y = 0; y < _c.getHeight(); y++) {
                pixel::CRGB colour = pixel::CRGB::Black;
                if (y <= barHeight) {
                    colour = pixel::CRGB(colMin).lerp16(colMax, pixel::fract16((barHeight / float(vertMax)) * 65535));
                }
                if (y == std::floor(barHeight)) {
                    float remainder = barHeight - std::floor(barHeight);
                    colour = colour.scale8(uint8_t(remainder * 255));
                }
                _c.setXY(x, _c.getHeight() - 1 - y, colour);
            }
        }
    }
    return _c;
}
