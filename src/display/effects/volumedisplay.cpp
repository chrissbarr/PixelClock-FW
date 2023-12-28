/* Project Scope */
#include "display/effects/volumedisplay.h"
#include "EMA.h"
#include "FMTWrapper.h"
#include "audio/audio.h"
#include "display/display.h"
#include "display/effects/utilities.h"
#include "utility.h"

VolumeDisplay::VolumeDisplay(const canvas::Canvas& size) : _c(size) {
    colourMap.push_back({0, flm::CRGB::Green});
    colourMap.push_back({0.4, flm::CRGB::Yellow});
    colourMap.push_back({0.6, flm::CRGB::Red});
}

void VolumeDisplay::reset() { _finished = false; }

canvas::Canvas VolumeDisplay::run() {

    auto& audioHist = AudioSingleton::get().getAudioCharacteristicsHistory();

    float vLeft = -60;
    float vRight = -60;

    if (!audioHist.empty()) {
        utility::EMA leftAvg(0.8);
        utility::EMA rightAvg(0.8);
        for (const auto& v : audioHist) {
            leftAvg.update(v.volumeLeft);
            rightAvg.update(v.volumeRight);
        }
        vLeft = leftAvg.getValue();
        vRight = rightAvg.getValue();
    }
    // printing::print(Serial, fmt::format("Volume: L={:.1f} R={:.1f}\n", vLeft, vRight));

    uint8_t horMax = _c.getWidth();

    float leftBarHeight = calculateBarHeight(vLeft, -40.0, 0.0, horMax);
    float rightBarHeight = calculateBarHeight(vRight, -40.0, 0.0, horMax);

    auto drawBar = [&](float barHeight, int y) {
        for (int x = 0; x < _c.getWidth(); x++) {
            flm::CRGB colour = flm::CRGB::Black;

            float pct = float(x) / horMax;

            if (x <= barHeight) {
                for (const auto& m : colourMap) {
                    if (pct > m.percentage) { colour = m.colour; }
                }
            }
            if (x == std::floor(barHeight)) {
                float remainder = barHeight - std::floor(barHeight);
                colour = colour.scale8(uint8_t(remainder * 255));
            }
            _c.setXY(x, y, colour);
        }
    };

    _c.fill(0);
    drawBar(leftBarHeight, 0);
    drawBar(leftBarHeight, 1);
    drawBar(rightBarHeight, 3);
    drawBar(rightBarHeight, 4);

    return _c;
}
