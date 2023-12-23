/* Project Scope */
#include "display/displayEffects.h"
#include "EMA.h"
#include "FMTWrapper.h"
#include "audio.h"
#include "display/display.h"
#include "utility.h"

/* C++ Standard Library */
#include <random>

TextScroller::TextScroller(
    const canvas::Canvas& size,
    std::string textString,
    std::vector<CRGB> colours,
    uint16_t stepDelay,
    uint16_t timeToHoldAtEnd,
    uint8_t characterSpacing)
    : _c(size),
      text(textString),
      colours(colours),
      stepDelay(stepDelay),
      timeToHoldAtEnd(timeToHoldAtEnd),
      charSpacing(characterSpacing) {
    lastUpdateTime = millis();
    currentOffset = 0;
    setTargetOffset(0);
}

canvas::Canvas TextScroller::run() {
    if (currentOffset == targetOffset) {
        if (arrivedAtEndTime == 0) {
            arrivedAtEndTime = millis();
        } else {
            if (millis() - arrivedAtEndTime > timeToHoldAtEnd) {
                _finished = true;
                arrivedAtEndTime = 0;
            }
        }
    } else {
        if (millis() - lastUpdateTime >= stepDelay) {
            if (targetOffset > currentOffset) {
                currentOffset += 1;
            } else if (targetOffset < currentOffset) {
                currentOffset -= 1;
            }
            lastUpdateTime = millis();
        }
    }
    _c.fill(CRGB::Black);
    _c.showCharacters(text, colours, -currentOffset, charSpacing);
    return _c;
}

void TextScroller::setTargetOffset(int targetCharacterIndex) {
    targetOffset = calculateOffset(targetCharacterIndex);
    _finished = false;
}

void TextScroller::setCurrentOffset(int targetCharacterIndex) {
    currentOffset = calculateOffset(targetCharacterIndex);
    _finished = false;
}

uint32_t TextScroller::calculateOffset(int targetCharIndex) const {
    int end = 0;
    int charIndex = 0;
    for (const char& character : text) {
        if (targetCharIndex != -1 && charIndex == targetCharIndex) { break; }
        end += characterFontArray[charToIndex(character)].width + charSpacing;
        charIndex++;
    }
    if (targetCharIndex == -1) { end -= (charSpacing + _c.getWidth()); }
    return end;
}

RepeatingTextScroller::RepeatingTextScroller(
    const canvas::Canvas& size,
    std::string textString,
    std::vector<CRGB> colours,
    uint16_t stepDelay,
    uint16_t timeToHoldAtEnd,
    uint8_t characterSpacing)
    : TextScroller(size, textString, colours, stepDelay, timeToHoldAtEnd, characterSpacing) {
    TextScroller::setTargetOffset(-1);
}

canvas::Canvas RepeatingTextScroller::run() {
    auto c = TextScroller::run();
    bool scrollerFinished = TextScroller::finished();
    if (scrollerFinished) {
        if (forward) {
            TextScroller::setTargetOffset(0);
            forward = false;
        } else {
            TextScroller::setTargetOffset(-1);
            forward = true;
        }
        cycles++;
    }

    return c;
}

RandomFill::RandomFill(const canvas::Canvas& size, uint32_t fillInterval, CRGB (*colourGenerator)())
    : _c(size),
      _fillInterval(fillInterval),
      _colourGenerator(colourGenerator) {
    reset();
}

canvas::Canvas RandomFill::run() {
    uint32_t timeNow = millis();

    if (timeNow - _lastSpawnTime >= _fillInterval) {
        if (_c.containsColour(CRGB::Black)) {
            bool filledPixel = false;
            while (!filledPixel) {
                uint8_t x = random(0, _c.getWidth());
                uint8_t y = random(0, _c.getHeight());
                if (_c.getXY(x, y) == CRGB::Black) {
                    _c.setXY(x, y, _colourGenerator());
                    filledPixel = true;
                    _lastSpawnTime = timeNow;
                }
            }
        } else {
            _finished = true;
        }
    }
    return _c;
}

BouncingBall::BouncingBall(const canvas::Canvas& size, uint32_t updateInterval, CRGB (*colourGenerator)())
    : _c(size),
      _updateInterval(updateInterval),
      _colourGenerator(colourGenerator) {
    reset();
}

void BouncingBall::reset() {
    const int spawnInFromBorder = 1;
    ballx = random(spawnInFromBorder, _c.getWidth() - 1 - spawnInFromBorder);
    bally = random(spawnInFromBorder, _c.getHeight() - 1 - spawnInFromBorder);
    xDir = 1;
    yDir = 1;
    _finished = false;
    _lastLoopTime = millis();
    _c.fill(CRGB::Black);
}

canvas::Canvas BouncingBall::run() {
    uint32_t millisSinceLastRun = millis() - _lastLoopTime;
    if (millisSinceLastRun > _updateInterval) {
        ballx += xDir;
        bally += yDir;

        if (ballx <= 0 || ballx >= _c.getWidth() - 1) { xDir = -xDir; }
        if (bally <= 0 || bally >= _c.getHeight() - 1) { yDir = -yDir; }

        _c.fill(CRGB::Black);
        uint8_t pixelx = uint8_t(round(ballx));
        uint8_t pixely = uint8_t(round(bally));
        // printing::print(Serial, fmt::format("Ball (x={:.2f}({}), y={:.2f}({}))\n", ballx, pixelx, bally, pixely));
        _c.setXY(pixelx, pixely, _colourGenerator());

        _lastLoopTime = millis();
    }
    return _c;
}

Gravity::Gravity(uint32_t moveInterval, bool empty, Gravity::Direction direction)
    : _moveInterval(moveInterval),
      _empty(empty),
      _direction(direction) {}

void Gravity::reset() {
    _lastMoveTime = millis();
    _finished = false;
}

canvas::Canvas Gravity::run() {
    uint32_t timenow = millis();
    if (timenow - _lastMoveTime > _moveInterval) {
        bool anyPixelsMovedThisUpdate = false;

        int xMax = _c.getWidth() - 1;
        int yMax = _c.getHeight() - 1;
        int xMin = 0;
        int yMin = 0;

        auto movePixel = [&](canvas::Canvas& c, int x, int y, int xMove, int yMove, bool empty) {
            auto currentIndex = c.XYToIndex(x, y);
            CRGB cellColour = c[currentIndex];

            if (cellColour != CRGB(0)) {
                // if this is the last row
                if ((yMove == 1 && y == yMax) || (yMove == -1 && y == yMin) || (xMove == 1 && x == xMin) ||
                    (xMove == -1 && x == xMin)) {
                    if (empty) {
                        c[currentIndex] = 0;
                        return true;
                    }
                    return false;
                }
                auto moveIntoIndex = c.XYToIndex(x + xMove, y + yMove);
                if (c[moveIntoIndex] == CRGB(0)) {
                    c[moveIntoIndex] = cellColour;
                    c[currentIndex] = 0;
                    return true;
                }
            }
            return false;
        };

        switch (_direction) {
        case Gravity::Direction::down: {
            for (int y = yMax; y >= yMin; y--) {
                for (int x = xMin; x <= xMax; x++) {
                    if (movePixel(_c, x, y, 0, 1, _empty)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::up: {
            for (int y = yMin; y <= yMax; y++) {
                for (int x = xMin; x <= xMax; x++) {
                    if (movePixel(_c, x, y, 0, -1, _empty)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::left: {
            for (int y = yMin; y <= yMax; y++) {
                for (int x = xMin; x <= xMax; x++) {
                    if (movePixel(_c, x, y, -1, 0, _empty)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::right: {
            for (int y = yMin; y <= yMax; y++) {
                for (int x = xMax; x >= xMin; x--) {
                    if (movePixel(_c, x, y, 1, 0, _empty)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        }

        if (!anyPixelsMovedThisUpdate) { _finished = true; }
        _lastMoveTime = timenow;
    }
    return _c;
}

SpectrumDisplay::SpectrumDisplay(const canvas::Canvas& size) : _c(size) {
    colMin = CRGB::Blue;
    colMax = CRGB::Purple;
}

void SpectrumDisplay::reset() { _finished = false; }

float calculateBarHeight(float val, float valMin, float valMax, float barMax) {
    float barMin = 0;
    float height = std::clamp((val - valMin) * (barMax - barMin) / (valMax - valMin) + barMin, barMin, barMax);
    return height;
}

canvas::Canvas SpectrumDisplay::run() {
    xSemaphoreTake(Audio::get().getAudioCharacteristicsSemaphore(), portMAX_DELAY);

    // average this many spectrums max
    const std::size_t maxSamplesToAvg = 5;
    std::vector<float> totals;
    const auto& hist = Audio::get().getAudioCharacteristicsHistory();
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
            std::bind(std::multiplies<float>(), std::placeholders::_1, 1.0 / samplesToAvg));
    }
    xSemaphoreGive(Audio::get().getAudioCharacteristicsSemaphore());

    uint8_t vertMax = _c.getHeight();
    if (!totals.empty()) {
        for (uint8_t x = 0; x < totals.size(); x++) {
            if (x >= _c.getWidth()) { break; }
            // we need to scale the value from 0 - valMax to 0 - vertMax
            auto barHeight = calculateBarHeight(totals[x], 0, maxScale, vertMax);
            for (int y = 0; y < _c.getHeight(); y++) {
                CRGB colour = CRGB::Black;
                if (y <= barHeight) {
                    colour = CRGB(colMin).lerp16(colMax, fract16((barHeight / float(vertMax)) * 65535));
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

VolumeDisplay::VolumeDisplay(const canvas::Canvas& size) : _c(size) {
    colourMap.push_back({0, CRGB::Green});
    colourMap.push_back({0.4, CRGB::Yellow});
    colourMap.push_back({0.6, CRGB::Red});
}

void VolumeDisplay::reset() { _finished = false; }

canvas::Canvas VolumeDisplay::run() {

    auto& audioHist = Audio::get().getAudioCharacteristicsHistory();

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
            CRGB colour = CRGB::Black;

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
            CRGB colour = CRGB::Black;
            if (yIdx <= barHeight) { colour = CRGB::Blue; }
            _c.setXY(xIdx, _c.getHeight() - 1 - yIdx, colour);
        }
        xIdx -= 1;
        if (xIdx < 0) { break; }
    }

    return _c;
}

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
                CRGB colour = CRGB::Red;
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

// bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty,
// uint32_t(*colourGenerator)(), DisplayRegion displayRegion)
// {
//   static uint32_t lastMoveTime = 0;
//   uint32_t timeNow = millis();

//   DisplayRegion spawnZone;
//   spawnZone.xMin = displayRegion.xMin;
//   spawnZone.xMax = displayRegion.xMax;
//   spawnZone.yMin = displayRegion.yMin;
//   spawnZone.yMax = displayRegion.yMin;

//   if (fillInterval != 0) {
//     fillRandomly(display, fillInterval, colourGenerator, spawnZone);
//   }

//   if (moveInterval != 0) {
//     // Move all pixels down
//     if (timeNow - lastMoveTime > moveInterval) {
//       for (int y = displayRegion.yMax; y >= displayRegion.yMin; y--) {
//         for (uint8_t x = displayRegion.xMin; x <= displayRegion.xMax; x++) {
//           uint32_t cellColour = display.getXY(x, y);
//           if (cellColour != 0) {
//             // if this is the last row
//             if (y == displayRegion.yMax) {
//               if (empty) {
//                 display.setXY(x, y, 0);
//               }
//               continue;
//             }
//             if (display.getXY(x, y + 1) == uint32_t(0)) {
//               display.setXY(x, y + 1, cellColour);
//               display.setXY(x, y, 0);
//             }
//           }
//         }
//       }
//       lastMoveTime = timeNow;
//     }
//   }
//   return !display.filled(0, displayRegion);
// }

// void tetris(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval)
// {
//   gravityFill(display, fillInterval, moveInterval, false, [](){
//     const uint32_t choices[] = {
//       Adafruit_NeoPixel::Color(255, 0, 0),
//       Adafruit_NeoPixel::Color(0, 255, 0),
//       Adafruit_NeoPixel::Color(0, 0, 255),
//     };
//     return choices[random(sizeof(choices)/sizeof(choices[0]))];
//   });

//   bool setFound = false;
//   uint8_t setSize = 3;
//   uint8_t setXCoords[3];
//   uint8_t setYCoords[3];

//   auto checkMatch = [&](uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3) {
//     uint32_t val1 = display.getXY(x1, y1);
//     if (val1 == 0) { return; }
//     uint32_t val2 = display.getXY(x2, y2);
//     uint32_t val3 = display.getXY(x3, y3);

//     if (val1 == val2 && val2 == val3) {
//       setFound = true;
//       setXCoords[0] = x1;
//       setXCoords[1] = x2;
//       setXCoords[2] = x3;
//       setYCoords[0] = y1;
//       setYCoords[1] = y2;
//       setYCoords[2] = y3;
//     }
//   };

//   for (uint8_t x = 0; x < display.getWidth(); x++) {
//     for (uint8_t y = 0; y < display.getHeight(); y++) {
//       if (x > 0 && x < display.getWidth() - 1) {
//         checkMatch(x - 1, y, x, y, x + 1, y);
//       }
//       if (y > 0 && y < display.getHeight() - 1) {
//         checkMatch(x, y - 1, x, y, x, y + 1);
//       }
//     }
//   }

//   if (setFound) {
//     for (int j = 0; j < setSize; j++) {
//       display.setXY(setXCoords[j], setYCoords[j], 0);
//     }
//   }
// }

canvas::Canvas ClockFace_Simple::run() {
    auto times = timeCallbackFunction();
    std::string timestr = fmt::format("{:2d}:{:2d}", times.hour12, times.minute);
    auto c = canvas::Canvas(17, 5);
    c.fill(0);
    c.showCharacters(timestr, {CRGB::White}, 0, 1);
    return c;
}

ClockFace_Gravity::ClockFace_Gravity(std::function<ClockFaceTimeStruct(void)> timeCallbackFunction)
    : ClockFace_Base(timeCallbackFunction) {
    gravityEffect = std::make_unique<Gravity>(500, false, Gravity::Direction::down);
    clockFace = std::make_unique<ClockFace_Simple>(timeCallbackFunction);
}

void ClockFace_Gravity::reset() {
    gravityEffect->reset();
    clockFace->reset();
    timePrev = timeCallbackFunction();
    currentState = State::stable;
}

canvas::Canvas ClockFace_Gravity::run() {
    auto timeNow = timeCallbackFunction();

    switch (currentState) {
    case State::stable:
        if (timePrev.minute != timeNow.minute) {
            currentState = State::fallToBottom;
            gravityEffect->reset();
            gravityEffect->setInput(_c);
            gravityEffect->setFallOutOfScreen(false);
        } else {
            _c = clockFace->run();
        }
        break;
    case State::fallToBottom:
        _c = gravityEffect->run();
        if (gravityEffect->finished()) {
            currentState = State::fallOut;
            gravityEffect->setFallOutOfScreen(true);
            gravityEffect->reset();
        }
        break;
    case State::fallOut:
        _c = gravityEffect->run();
        if (gravityEffect->finished()) { currentState = State::stable; }
        break;
    }

    timePrev = timeNow;
    return _c;
}

void displayDiagnostic(PixelDisplay& display) {
    // Clear display
    canvas::Canvas c(display.getWidth(), display.getHeight());
    c.fill(0);
    display.update(c);
    delay(250);

    // Show Pixel 0
    c.setXY(0, 0, CRGB(255, 0, 0));
    display.update(c);
    delay(250);

    // Solid Red, Green, Blue
    c.fill(CRGB(255, 0, 0));
    display.update(c);
    delay(250);
    c.fill(CRGB(0, 255, 0));
    display.update(c);
    delay(250);
    c.fill(CRGB(0, 0, 255));
    display.update(c);
    delay(250);

    // Move through XY sequentially
    for (uint8_t y = 0; y < c.getHeight(); y++) {
        for (uint8_t x = 0; x < c.getWidth(); x++) {
            c.fill(0);
            c.setXY(x, y, CRGB(100, 0, 0));
            display.update(c);
            delay(1);
        }
    }

    // Scroll short test
    c.fill(0);
    display.update(c);
    auto textScrollTest1 = RepeatingTextScroller(c, "Hello - Testing!", std::vector<CRGB>{CRGB(0, 0, 255)}, 50, 500, 1);
    while (!textScrollTest1.finished()) {
        display.update(textScrollTest1.run());
        delay(1);
    }

    // Scroll full character set
    c.fill(0);
    display.update(c);
    auto textScrollTest = RepeatingTextScroller(
        c,
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 !\"#$%&'()*+'-./:;<=>?@",
        std::vector<CRGB>{CRGB(0, 255, 0)},
        50,
        500,
        1);
    while (!textScrollTest.finished()) {
        display.update(textScrollTest.run());
        delay(1);
    }
    c.fill(0);
    display.update(c);
}

void HSVTestPattern::apply(canvas::Canvas& c) {
    for (uint8_t x = 0; x < c.getWidth(); x++) {
        for (uint8_t y = 0; y < c.getHeight(); y++) {
            c.setXY(x, y, CHSV(x * (255 / c.getWidth()), 255, y * (255 / c.getHeight())));
        }
    }
}

void SolidColour::apply(canvas::Canvas& c) {
    for (std::size_t i = 0; i < c.getSize(); i++) {
        if (c[i] == CRGB(0)) { continue; }
        c[i] = maintainBrightness ? CRGB(colour).nscale8(c[i].getAverageLight()) : colour;
    }
}

void RainbowWave::apply(canvas::Canvas& c) {
    static float wheelPos = 0;
    wheelPos += speed;
    int _width = width;
    if (_width == 0) { _width = c.getWidth(); }

    for (uint8_t x = 0; x < c.getWidth(); x++) {
        for (uint8_t y = 0; y < c.getHeight(); y++) {
            uint8_t hue = uint8_t(round(wheelPos)) + ((direction == Direction::horizontal ? x : y) * 256 / _width);
            auto index = c.XYToIndex(x, y);
            if (c[index] == CRGB(0)) { continue; }
            c[index] = CHSV(hue, 255, maintainBrightness ? c[index].getAverageLight() : 255);
        }
    }
}