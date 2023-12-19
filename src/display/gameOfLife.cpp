/* Project Scope */
#include "display/gameOfLife.h"

/* C++ Standard Library */
#include <random>

GameOfLife::GameOfLife(
    PixelDisplay& display,
    uint32_t updateInterval,
    uint32_t fadeInterval,
    CRGB (*colourGenerator)(),
    const DisplayRegion& displayRegion,
    bool wrap)
    : _display(display),
      _updateInterval(updateInterval),
      _fadeInterval(fadeInterval),
      _colourGenerator(colourGenerator),
      _wrap(wrap) {
    if (displayRegion == defaultFull) {
        _displayRegion = display.getFullDisplayRegion();
    } else {
        _displayRegion = displayRegion;
    }

    int bufferSize = _display.getSize();
    buffers.push_back(std::vector<CRGB>(bufferSize, CRGB::Black));
    buffers.push_back(std::vector<CRGB>(bufferSize, CRGB::Black));
}

void GameOfLife::reset() {
    _lastLoopTime = 0;
    _finished = false;
    _dead = false;
    _notUniqueForNSteps = 0;

    GoLScore lastScore = {_lastSeed, _lifespan};
    if (!_seeding) {
        Serial.print("GoL Reset. ID: ");
        Serial.print(iterationId);
        Serial.print("\tScore: ");
        Serial.print(lastScore.lifespan);
        Serial.print("\tSeed: ");
        Serial.println(lastScore.seed);
    }
    iterationId++;
    if (_seeding) {
        if (lastScore.lifespan > 10) {
            // Serial.println("saved score");
            bestScores.insert(lastScore);
            if (bestScores.size() > bestScoresToKeep) { bestScores.erase(bestScores.begin()); }
            // Serial.println(bestScores.size());
        }
    }
    _lifespan = 0;
    seedDisplay();
}

void GameOfLife::seedDisplay() {
    //_display.fill(0);
    for (auto& buffer : buffers) { std::fill(buffer.begin(), buffer.end(), 0); }
    readBuffer = 0;
    writeBuffer = 1;

    if (!_seeding && bestScores.size() >= bestScoresToKeep) {
        Serial.println("Using seed from best scores...");
        int randomIndex = random(bestScores.size());
        auto randomScoreToRepeat = (*std::next(bestScores.begin(), randomIndex));
        Serial.print("Using score: ");
        Serial.print(randomScoreToRepeat.lifespan);
        Serial.print("\tSeed: ");
        Serial.println(randomScoreToRepeat.seed);
        _lastSeed = randomScoreToRepeat.seed;
    } else {
        // Serial.println("Using random seed...");
        _lastSeed = micros();
    }

    std::minstd_rand simple_rand;
    simple_rand.seed(_lastSeed);
    std::uniform_int_distribution<uint8_t> dist(0, 10);

    for (uint8_t x = _displayRegion.xMin; x <= _displayRegion.xMax; x++) {
        for (uint8_t y = _displayRegion.yMin; y <= _displayRegion.yMax; y++) {
            int chance = dist(simple_rand);
            if (chance == 0) { buffers[readBuffer][_display.XYToIndex(x, y)] = _colourGenerator(); }
        }
    }
}

bool GameOfLife::run() {
    bool changedDisplay = false;
    if (_dead) {
        if (!_fadeOnDeath) {
            _finished = true;
        } else {
            if (millis() - _lastLoopTime >= _fadeInterval) {
                for (uint8_t x = _displayRegion.xMin; x <= _displayRegion.xMax; x++) {
                    for (uint8_t y = _displayRegion.yMin; y <= _displayRegion.yMax; y++) {
                        buffers[writeBuffer][_display.XYToIndex(x, y)] =
                            buffers[readBuffer][_display.XYToIndex(x, y)].fadeToBlackBy(10);
                    }
                }
                _lastLoopTime = millis();
                changedDisplay = true;
            }
            bool empty = std::all_of(
                buffers[readBuffer].begin(), buffers[readBuffer].end(), [](CRGB i) { return i == CRGB(0); });
            if (empty) { _finished = true; }
        }
    } else {
        if (_seeding || millis() - _lastLoopTime >= _updateInterval) {
            auto neighbourCount = [](uint8_t xPos,
                                     uint8_t yPos,
                                     const PixelDisplay& _display,
                                     const std::vector<CRGB>& buffer,
                                     const DisplayRegion& _region,
                                     bool wrap) -> uint8_t {
                uint8_t alive = 0;
                for (int x = xPos - 1; x <= xPos + 1; x++) {
                    int testX = x;
                    if (testX < _region.xMin) {
                        if (wrap) {
                            testX = _region.xMax;
                        } else {
                            continue;
                        }
                    }
                    if (testX > _region.xMax) {
                        if (wrap) {
                            testX = _region.xMin;
                        } else {
                            continue;
                        }
                    }
                    for (int y = yPos - 1; y <= yPos + 1; y++) {
                        int testY = y;
                        if (testY < _region.yMin) {
                            if (wrap) {
                                testY = _region.yMax;
                            } else {
                                continue;
                            }
                        }
                        if (testY > _region.yMax) {
                            if (wrap) {
                                testY = _region.yMin;
                            } else {
                                continue;
                            }
                        }
                        if (testX == xPos && testY == yPos) { continue; }
                        if (buffer[_display.XYToIndex(testX, testY)] != CRGB(0)) { alive++; }
                    }
                }
                return alive;
            };

            std::fill(buffers[writeBuffer].begin(), buffers[writeBuffer].end(), 0);

            // Serial.println("Neighbours");
            for (uint8_t x = _displayRegion.xMin; x <= _displayRegion.xMax; x++) {
                for (uint8_t y = _displayRegion.yMin; y <= _displayRegion.yMax; y++) {
                    uint8_t neighbours = neighbourCount(x, y, _display, buffers[readBuffer], _displayRegion, _wrap);
                    // Serial.println(neighbours);
                    const CRGB currentVal = buffers[readBuffer][_display.XYToIndex(x, y)];
                    bool currentlyAlive = (currentVal != CRGB(0));
                    if (currentlyAlive) {
                        if (neighbours == 2 || neighbours == 3) {
                            // keep on living
                            buffers[writeBuffer][_display.XYToIndex(x, y)] = currentVal;
                        } else {
                            // kill this cell
                            buffers[writeBuffer][_display.XYToIndex(x, y)] = CRGB(0);
                        }
                    } else {
                        if (neighbours == 3) {
                            // come to life!
                            buffers[writeBuffer][_display.XYToIndex(x, y)] = _colourGenerator();
                        }
                    }
                }
            }

            // copy buffer into display and count living cells
            uint32_t livingCells = 0;
            for (uint32_t i = 0; i < buffers[writeBuffer].size(); i++) {
                if (buffers[writeBuffer][i] != CRGB(0)) { livingCells++; }
            }

            // Serial.print("Living cells this timestep: "); Serial.println(livingCells);
            // Serial.print("State is unique: "); Serial.println(unique);
            // Serial.print("State not unique for N steps: "); Serial.println(_notUniqueForNSteps);

            if (livingCells == 0) {
                // test for simplest death state first to avoid expensive checks later
                _dead = true;
            } else {
                // convert the current state to a hash and compare against previous states
                auto currentStateHash = hashBuffer(buffers[writeBuffer]);
                if (bufferHashes.size() > 0) {
                    bool unique = true;
                    for (const auto& hash : bufferHashes) {
                        if (currentStateHash == hash) { unique = false; }
                    }
                    if (!unique) {
                        _notUniqueForNSteps++;
                    } else {
                        _notUniqueForNSteps = 0;
                    }

                    if (currentStateHash == bufferHashes.back() || _notUniqueForNSteps >= 20) { _dead = true; }
                }
                bufferHashes.push_back(currentStateHash);
                if (bufferHashes.size() > 100) { bufferHashes.pop_front(); }
            }
            _lifespan++;
            _lastLoopTime = millis();
            changedDisplay = true;
        }
    }
    if (changedDisplay) {
        if (!_seeding) {
            for (uint32_t i = 0; i < buffers[writeBuffer].size(); i++) {
                _display.setIndex(i, buffers[writeBuffer][i]);
            }
        }
        uint8_t tempIdx = readBuffer;
        readBuffer = writeBuffer;
        writeBuffer = tempIdx;
    }
    return _finished;
}

std::size_t GameOfLife::hashBuffer(const std::vector<CRGB>& vec) const {
    std::size_t seed = vec.size();
    for (const CRGB& i : vec) {
        uint8_t val = 1;
        if (i == CRGB(0)) { val = 0; }
        seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}