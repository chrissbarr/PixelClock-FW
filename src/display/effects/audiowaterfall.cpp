/* Project Scope */
#include "display/effects/audiowaterfall.h"
#include "audio.h"

AudioWaterfall::AudioWaterfall(const canvas::Canvas& size) : _c(size) {}

void AudioWaterfall::reset() { _finished = false; }

canvas::Canvas AudioWaterfall::run() {

    _c.fill(0);

    xSemaphoreTake(Audio::get().getAudioCharacteristicsSemaphore(), portMAX_DELAY);
    const auto& hist = Audio::get().getAudioCharacteristicsHistory();
    if (!hist.empty()) {

        int xIdx = _c.getWidth() - 1;
        for (auto it = hist.rbegin(); it != hist.rend(); ++it) {
            for (int yIdx = 0; yIdx < _c.getHeight(); yIdx++) {
                float val = it->spectrum.at(yIdx);
                val = val / 8000;
                pixel::CRGB colour = pixel::CRGB::Red;
                colour = colour.scale8(uint8_t(val * 255));
                _c.setXY(xIdx, _c.getHeight() - 1 - yIdx, colour);
            }
            xIdx -= 1;
            if (xIdx < 0) { break; }
        }
    }
    xSemaphoreGive(Audio::get().getAudioCharacteristicsSemaphore());

    return _c;
}
