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
    PixelDisplay& display,
    std::string textString,
    std::vector<CRGB> colours,
    uint16_t stepDelay,
    uint16_t timeToHoldAtEnd,
    uint8_t characterSpacing)
    : display(display),
      text(textString),
      colours(colours),
      stepDelay(stepDelay),
      timeToHoldAtEnd(timeToHoldAtEnd),
      charSpacing(characterSpacing) {
    lastUpdateTime = millis();
    currentOffset = 0;
    setTargetOffset(0);
}

bool TextScroller::run() {
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
    display.fill(CRGB::Black);
    display.showCharacters(text, colours, -currentOffset, charSpacing);
    return _finished;
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
    if (targetCharIndex == -1) { end -= (charSpacing + display.getWidth()); }
    return end;
}

RepeatingTextScroller::RepeatingTextScroller(
    PixelDisplay& display,
    std::string textString,
    std::vector<CRGB> colours,
    uint16_t stepDelay,
    uint16_t timeToHoldAtEnd,
    uint8_t characterSpacing)
    : TextScroller(display, textString, colours, stepDelay, timeToHoldAtEnd, characterSpacing) {
    TextScroller::setTargetOffset(-1);
}

bool RepeatingTextScroller::run() {
    bool scrollerFinished = TextScroller::run();
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

    return finished();
}

RandomFill::RandomFill(
    PixelDisplay& display, uint32_t fillInterval, CRGB (*colourGenerator)(), const DisplayRegion& spawnRegion)
    : _display(display),
      _fillInterval(fillInterval),
      _colourGenerator(colourGenerator) {
    if (spawnRegion == defaultFull) {
        _spawnRegion = display.getFullDisplayRegion();
    } else {
        _spawnRegion = spawnRegion;
    }
    reset();
}

bool RandomFill::run() {
    uint32_t timeNow = millis();

    if (timeNow - _lastSpawnTime >= _fillInterval) {
        if (!_display.filled(0, _spawnRegion)) {
            bool filledPixel = false;
            while (!filledPixel) {
                uint8_t x = random(_spawnRegion.xMin, _spawnRegion.xMax + 1);
                uint8_t y = random(_spawnRegion.yMin, _spawnRegion.yMax + 1);
                // Serial.print("X: "); Serial.print(x); Serial.print(" Y: "); Serial.println(y);
                if (_display.getXY(x, y) == CRGB(0)) {
                    _display.setXY(x, y, _colourGenerator());
                    filledPixel = true;
                    _lastSpawnTime = timeNow;
                }
            }
        } else {
            _finished = true;
        }
    }
    return _finished;
}

BouncingBall::BouncingBall(
    PixelDisplay& display, uint32_t updateInterval, CRGB (*colourGenerator)(), const DisplayRegion& displayRegion)
    : _display(display),
      _updateInterval(updateInterval),
      _colourGenerator(colourGenerator) {
    if (displayRegion == defaultFull) {
        _displayRegion = display.getFullDisplayRegion();
    } else {
        _displayRegion = displayRegion;
    }
}

void BouncingBall::reset() {
    ballx = random(_displayRegion.xMin + 1, _displayRegion.xMax);
    bally = random(_displayRegion.yMin + 1, _displayRegion.yMax);
    xDir = 1;
    yDir = 1;
    _finished = false;
    _lastLoopTime = millis();
    _display.fill(0, _displayRegion);
}

bool BouncingBall::run() {
    uint32_t millisSinceLastRun = millis() - _lastLoopTime;
    if (millisSinceLastRun > _updateInterval) {
        ballx += xDir;
        bally += yDir;

        if (ballx <= _displayRegion.xMin || ballx >= _displayRegion.xMax) { xDir = -xDir; }
        if (bally <= _displayRegion.yMin || bally >= _displayRegion.yMax) { yDir = -yDir; }

        _display.fill(0, _displayRegion);
        _display.setXY(uint8_t(round(ballx)), uint8_t(round(bally)), _colourGenerator());

        _lastLoopTime = millis();
    }
    return true;
}

Gravity::Gravity(
    PixelDisplay& display,
    uint32_t moveInterval,
    bool empty,
    Gravity::Direction direction,
    const DisplayRegion& displayRegion)
    : _display(display),
      _moveInterval(moveInterval),
      _empty(empty),
      _direction(direction) {
    if (displayRegion == defaultFull) {
        _displayRegion = display.getFullDisplayRegion();
    } else {
        _displayRegion = displayRegion;
    }
}

void Gravity::reset() {
    _lastMoveTime = millis();
    _finished = false;
}

bool Gravity::run() {
    uint32_t timenow = millis();
    if (timenow - _lastMoveTime > _moveInterval) {
        bool anyPixelsMovedThisUpdate = false;

        auto movePixel = [](PixelDisplay& display,
                            const DisplayRegion& displayRegion,
                            int x,
                            int y,
                            int xMove,
                            int yMove,
                            bool empty) {
            auto currentIndex = display.XYToIndex(x, y);
            CRGB cellColour = display.getIndex(currentIndex);
            if (cellColour != CRGB(0)) {
                // if this is the last row
                if ((yMove == 1 && y == displayRegion.yMax) || (yMove == -1 && y == displayRegion.yMin) ||
                    (xMove == 1 && x == displayRegion.xMax) || (xMove == -1 && x == displayRegion.yMin)) {
                    if (empty) {
                        display.setIndex(currentIndex, 0);
                        return true;
                    }
                    return false;
                }
                auto moveIntoIndex = display.XYToIndex(x + xMove, y + yMove);
                if (display.getIndex(moveIntoIndex) == CRGB(0)) {
                    display.setIndex(moveIntoIndex, cellColour);
                    display.setIndex(currentIndex, 0);
                    return true;
                }
            }
            return false;
        };

        switch (_direction) {
        case Gravity::Direction::down: {
            for (int y = _displayRegion.yMax; y >= _displayRegion.yMin; y--) {
                for (int x = _displayRegion.xMin; x <= _displayRegion.xMax; x++) {
                    if (movePixel(_display, _displayRegion, x, y, 0, 1, _empty)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::up: {
            for (int y = _displayRegion.yMin; y <= _displayRegion.yMax; y++) {
                for (int x = _displayRegion.xMin; x <= _displayRegion.xMax; x++) {
                    if (movePixel(_display, _displayRegion, x, y, 0, -1, _empty)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::left: {
            for (int y = _displayRegion.yMin; y <= _displayRegion.yMax; y++) {
                for (int x = _displayRegion.xMin; x <= _displayRegion.xMax; x++) {
                    if (movePixel(_display, _displayRegion, x, y, -1, 0, _empty)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        case Gravity::Direction::right: {
            for (int y = _displayRegion.yMin; y <= _displayRegion.yMax; y++) {
                for (int x = _displayRegion.xMax; x >= _displayRegion.xMin; x--) {
                    if (movePixel(_display, _displayRegion, x, y, 1, 0, _empty)) { anyPixelsMovedThisUpdate = true; }
                }
            }
            break;
        }
        }

        if (!anyPixelsMovedThisUpdate) { _finished = true; }
        _lastMoveTime = timenow;
    }
    return _finished;
}

SpectrumDisplay::SpectrumDisplay(PixelDisplay& display, uint8_t width, uint32_t decayRate)
    : _display(display),
      _width(width),
      _decayRate(decayRate) {
    colMin = CRGB::Blue;
    colMax = CRGB::Purple;
}

void SpectrumDisplay::reset() {
    _data = std::vector<float>(_width, 0);
    _finished = false;
}

float calculateBarHeight(float val, float valMin, float valMax, float barMax) {
    float barMin = 0;
    float height = std::clamp((val - valMin) * (barMax - barMin) / (valMax - valMin) + barMin, barMin, barMax);
    return height;
}

bool SpectrumDisplay::run() {
    xSemaphoreTake(Audio::get().getAudioCharacteristicsSemaphore(), portMAX_DELAY);

    // average this many spectrums max
    const std::size_t maxSamplesToAvg = 5;

    const auto& hist = Audio::get().getAudioCharacteristicsHistory();
    if (!hist.empty()) {
        std::vector<float> totals = std::vector<float>(hist.back().spectrum.size(), 0);

        // limit averages to maximum and available
        const std::size_t samplesToAvg = std::min(maxSamplesToAvg, hist.size());

        // iterate in reverse order so we always average N latest
        int idx = 0;
        for(auto it = hist.rbegin(); it != hist.rend(); ++it) {
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
        supplyData(totals);
    }
    xSemaphoreGive(Audio::get().getAudioCharacteristicsSemaphore());

    uint8_t vertMax = _display.getHeight();
    if (!_data.empty()) {
        for (uint8_t x = 0; x < _data.size(); x++) {
            if (x >= _display.getWidth()) { break; }
            // we need to scale the value from 0 - valMax to 0 - vertMax
            auto barHeight = calculateBarHeight(_data[x], 0, maxScale, vertMax);
            for (int y = 0; y < _display.getHeight(); y++) {
                CRGB colour = CRGB::Black;
                if (y <= barHeight) {
                    colour = CRGB(colMin).lerp16(colMax, fract16((barHeight / float(vertMax)) * 65535));
                }
                if (y == std::floor(barHeight)) {
                    float remainder = barHeight - std::floor(barHeight);
                    colour = colour.scale8(uint8_t(remainder * 255));
                }
                _display.setXY(x, _display.getHeight() - 1 - y, colour);
            }
        }
    }
    return finished();
}

void SpectrumDisplay::supplyData(std::vector<float> data) { _data = data; }

VolumeDisplay::VolumeDisplay(PixelDisplay& display) : _display(display) {
    colourMap.push_back({0, CRGB::Green});
    colourMap.push_back({0.4, CRGB::Yellow});
    colourMap.push_back({0.6, CRGB::Red});
}

void VolumeDisplay::reset() { _finished = false; }

bool VolumeDisplay::run() {

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

    uint8_t horMax = _display.getWidth();

    float leftBarHeight = calculateBarHeight(vLeft, -40.0, 0.0, horMax);
    float rightBarHeight = calculateBarHeight(vRight, -40.0, 0.0, horMax);

    auto drawBar = [&](float barHeight, int y) {
        for (int x = 0; x < _display.getWidth(); x++) {
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
            _display.setXY(x, y, colour);
        }
    };

    _display.fill(0);
    drawBar(leftBarHeight, 0);
    drawBar(leftBarHeight, 1);
    drawBar(rightBarHeight, 3);
    drawBar(rightBarHeight, 4);

    return finished();
}

VolumeGraph::VolumeGraph(PixelDisplay& display) : _display(display) {}

void VolumeGraph::reset() { _finished = false; }

bool VolumeGraph::run() {

    _display.fill(0);
    auto& audioHist = Audio::get().getAudioCharacteristicsHistory();

    float volMin = 0;
    float volMax = -60;
    for (auto it = audioHist.rbegin(); it != audioHist.rend(); ++it) {
        float vol = (it->volumeLeft + it->volumeRight) / 2;
        if (vol > volMax) { volMax = vol; }
        if (vol < volMin) { volMin = vol; }
    }

    int xIdx = _display.getWidth() - 1;
    for (auto it = audioHist.rbegin(); it != audioHist.rend(); ++it) {
        float vol = (it->volumeLeft + it->volumeRight) / 2;
        float barHeight = calculateBarHeight(vol, volMin * 0.9, volMax * 0.9, 5);
        for (int yIdx = 0; yIdx < _display.getHeight(); yIdx++) {
            CRGB colour = CRGB::Black;
            if (yIdx <= barHeight) { colour = CRGB::Blue; }
            _display.setXY(xIdx, _display.getHeight() - 1 - yIdx, colour);
        }
        xIdx -= 1;
        if (xIdx < 0) { break; }
    }

    return finished();
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

bool ClockFace_Simple::run() {
    auto times = timeCallbackFunction();
    std::string timestr = fmt::format("{:2d}:{:2d}", times.hour12, times.minute);
    _display.fill(0);
    _display.showCharacters(timestr, {CRGB::White}, 0, 1);
    return false;
}

ClockFace_Gravity::ClockFace_Gravity(
    PixelDisplay& display, std::function<ClockFaceTimeStruct(void)> timeCallbackFunction)
    : ClockFace_Base(display, timeCallbackFunction) {
    gravityEffect = std::make_unique<Gravity>(display, 500, false, Gravity::Direction::down);
    clockFace = std::make_unique<ClockFace_Simple>(display, timeCallbackFunction);
}

void ClockFace_Gravity::reset() {
    gravityEffect->reset();
    clockFace->reset();
    timePrev = timeCallbackFunction();
    currentState = State::stable;
}

bool ClockFace_Gravity::run() {
    auto timeNow = timeCallbackFunction();

    switch (currentState) {
    case State::stable:
        if (timePrev.minute != timeNow.minute) {
            currentState = State::fallToBottom;
            gravityEffect->reset();
            gravityEffect->setFallOutOfScreen(false);
        } else {
            clockFace->run();
        }
        break;
    case State::fallToBottom:
        gravityEffect->run();
        if (gravityEffect->finished()) {
            currentState = State::fallOut;
            gravityEffect->setFallOutOfScreen(true);
            gravityEffect->reset();
        }
        break;
    case State::fallOut:
        gravityEffect->run();
        if (gravityEffect->finished()) { currentState = State::stable; }
        break;
    }

    timePrev = timeNow;
    return false;
}

void displayDiagnostic(PixelDisplay& display) {
    // Clear display
    display.fill(0);
    display.update();
    delay(250);

    // Show Pixel 0
    display.setXY(0, 0, CRGB(255, 0, 0));
    display.update();
    delay(250);

    // Solid Red, Green, Blue
    display.fill(CRGB(255, 0, 0));
    display.update();
    delay(250);
    display.fill(CRGB(0, 255, 0));
    display.update();
    delay(250);
    display.fill(CRGB(0, 0, 255));
    display.update();
    delay(250);

    // Move through XY sequentially
    for (uint8_t y = 0; y < display.getHeight(); y++) {
        for (uint8_t x = 0; x < display.getWidth(); x++) {
            display.fill(0);
            display.setXY(x, y, CRGB(100, 0, 0));
            display.update();
            delay(1);
        }
    }

    // Scroll short test
    display.fill(0);
    display.update();
    auto textScrollTest1 =
        RepeatingTextScroller(display, "Hello - Testing!", std::vector<CRGB>{CRGB(0, 0, 255)}, 50, 500, 1);
    while (!textScrollTest1.run()) {
        display.update();
        display.fill(0);
        delay(1);
    }

    // Scroll full character set
    display.fill(0);
    display.update();
    auto textScrollTest = RepeatingTextScroller(
        display,
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 !\"#$%&'()*+'-./:;<=>?@",
        std::vector<CRGB>{CRGB(0, 255, 0)},
        50,
        500,
        1);
    while (!textScrollTest.run()) {
        display.update();
        display.fill(0);
        delay(1);
    }
    display.fill(0);
    display.update();
}

void HSVTestPattern::apply(PixelDisplay& display) const {
    auto& buffer = display.getFilterBuffer();
    for (uint8_t x = 0; x < display.getWidth(); x++) {
        for (uint8_t y = 0; y < display.getHeight(); y++) {
            auto index = display.XYToIndex(x, y);
            buffer[index] = CHSV(x * (255 / display.getWidth()), 255, y * (255 / display.getHeight()));
        }
    }
}

void SolidColour::apply(PixelDisplay& display) const {
    auto& buffer = display.getFilterBuffer();
    for (std::size_t i = 0; i < buffer.size(); i++) {
        if (buffer[i] == CRGB(0)) { continue; }
        buffer[i] = maintainBrightness ? CRGB(colour).nscale8(buffer[i].getAverageLight()) : colour;
    }
}

void RainbowWave::apply(PixelDisplay& display) const {
    auto& buffer = display.getFilterBuffer();
    static float wheelPos = 0;
    wheelPos += speed;
    int _width = width;
    if (_width == 0) { _width = display.getWidth(); }

    for (uint8_t x = 0; x < display.getWidth(); x++) {
        for (uint8_t y = 0; y < display.getHeight(); y++) {
            uint8_t hue = uint8_t(round(wheelPos)) + ((direction == Direction::horizontal ? x : y) * 256 / _width);
            auto index = display.XYToIndex(x, y);
            if (buffer[index] == CRGB(0)) { continue; }
            buffer[index] = CHSV(hue, 255, maintainBrightness ? buffer[index].getAverageLight() : 255);
        }
    }
}