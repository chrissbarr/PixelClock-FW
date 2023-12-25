/* Project Scope */
#include "display/effects/volumegraph.h"
#include "EMA.h"
#include "FMTWrapper.h"
#include "audio.h"
#include "display/display.h"
#include "display/effects/utilities.h"
#include "utility.h"

VolumeGraph::VolumeGraph(const canvas::Canvas& size) : _c(size) {}

void VolumeGraph::reset() { _finished = false; }

canvas::Canvas VolumeGraph::run() {

    _c.fill(0);
    auto& audioHist = Audio::get().getAudioCharacteristicsHistory();

    float volMin = 0;
    float volMax = -60;
    for (auto it = audioHist.rbegin(); it != audioHist.rend(); ++it) {
        float vol = (it->volumeLeft + it->volumeRight) / 2;
        if (vol > volMax) { volMax = vol; }
        if (vol < volMin) { volMin = vol; }
    }

    int xIdx = _c.getWidth() - 1;
    for (auto it = audioHist.rbegin(); it != audioHist.rend(); ++it) {
        float vol = (it->volumeLeft + it->volumeRight) / 2;
        float barHeight = calculateBarHeight(vol, volMin * 0.9, volMax * 0.9, 5);
        for (int yIdx = 0; yIdx < _c.getHeight(); yIdx++) {
            pixel::CRGB colour = pixel::CRGB::Black;
            if (yIdx <= barHeight) { colour = pixel::CRGB::Blue; }
            _c.setXY(xIdx, _c.getHeight() - 1 - yIdx, colour);
        }
        xIdx -= 1;
        if (xIdx < 0) { break; }
    }

    return _c;
}
