/* Project Scope */
#include "display/effects/gameoflife.h"
#include "FMTWrapper.h"
#include "utility.h"

/* C++ Standard Library */
#include <random>

GameOfLife::GameOfLife(
    const canvas::Canvas& size,
    uint32_t updateInterval,
    uint32_t fadeInterval,
    colourGenerator::Generator colourGenerator,
    bool wrap)
    : _c(size),
      _updateInterval(updateInterval),
      _fadeInterval(fadeInterval),
      _colourGenerator(colourGenerator),
      _wrap(wrap) {

    int bufferSize = _c.getSize();
    buffers.push_back(std::vector<pixel::CRGB>(bufferSize, pixel::CRGB::Black));
    buffers.push_back(std::vector<pixel::CRGB>(bufferSize, pixel::CRGB::Black));
}

void GameOfLife::reset() {
    _lastLoopTime = 0;
    _finished = false;
    _dead = false;
    _notUniqueForNSteps = 0;

    GoLScore lastScore = {_lastSeed, _lifespan};
    if (!_seeding) {
        printing::print(
            Serial,
            fmt::format("GoL Reset. ID: {}, Score: {}, Seed: {}\n", iterationId, lastScore.lifespan, lastScore.seed));
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
        printing::print(
            Serial, fmt::format("Using score: {}, Seed: {}\n", randomScoreToRepeat.lifespan, randomScoreToRepeat.seed));
        _lastSeed = randomScoreToRepeat.seed;
    } else {
        // Serial.println("Using random seed...");
        _lastSeed = micros();
    }

    std::minstd_rand simple_rand;
    simple_rand.seed(_lastSeed);
    std::uniform_int_distribution<uint8_t> dist(0, 10);

    int xMin = 0;
    int xMax = _c.getWidth() - 1;
    int yMin = 0;
    int yMax = _c.getHeight() - 1;

    for (uint8_t x = xMin; x <= xMax; x++) {
        for (uint8_t y = yMin; y <= yMax; y++) {
            int chance = dist(simple_rand);
            if (chance == 0) { buffers[readBuffer][_c.XYToIndex(x, y)] = _colourGenerator(); }
        }
    }
}

canvas::Canvas GameOfLife::run() {
    bool changedDisplay = false;

    int xMin = 0;
    int xMax = _c.getWidth() - 1;
    int yMin = 0;
    int yMax = _c.getHeight() - 1;

    if (_dead) {
        if (!_fadeOnDeath) {
            _finished = true;
        } else {
            if (millis() - _lastLoopTime >= _fadeInterval) {

                for (uint8_t x = xMin; x <= xMax; x++) {
                    for (uint8_t y = yMin; y <= yMax; y++) {
                        buffers[writeBuffer][_c.XYToIndex(x, y)] =
                            buffers[readBuffer][_c.XYToIndex(x, y)].fadeToBlackBy(10);
                    }
                }
                _lastLoopTime = millis();
                changedDisplay = true;
            }
            bool empty = std::all_of(
                buffers[readBuffer].begin(), buffers[readBuffer].end(), [](pixel::CRGB i) { return i == pixel::CRGB(0); });
            if (empty) { _finished = true; }
        }
    } else {
        if (_seeding || millis() - _lastLoopTime >= _updateInterval) {
            auto neighbourCount = [](uint8_t xPos,
                                     uint8_t yPos,
                                     const canvas::Canvas& _c,
                                     const std::vector<pixel::CRGB>& buffer,
                                     bool wrap) -> uint8_t {
                uint8_t alive = 0;

                int xMin = 0;
                int xMax = _c.getWidth() - 1;
                int yMin = 0;
                int yMax = _c.getHeight() - 1;

                for (int x = xPos - 1; x <= xPos + 1; x++) {
                    int testX = x;
                    if (testX < xMin) {
                        if (wrap) {
                            testX = xMax;
                        } else {
                            continue;
                        }
                    }
                    if (testX > xMax) {
                        if (wrap) {
                            testX = xMin;
                        } else {
                            continue;
                        }
                    }
                    for (int y = yPos - 1; y <= yPos + 1; y++) {
                        int testY = y;
                        if (testY < yMin) {
                            if (wrap) {
                                testY = yMax;
                            } else {
                                continue;
                            }
                        }
                        if (testY > yMax) {
                            if (wrap) {
                                testY = yMin;
                            } else {
                                continue;
                            }
                        }
                        if (testX == xPos && testY == yPos) { continue; }
                        if (buffer[_c.XYToIndex(testX, testY)] != pixel::CRGB(0)) { alive++; }
                    }
                }
                return alive;
            };

            std::fill(buffers[writeBuffer].begin(), buffers[writeBuffer].end(), 0);

            // Serial.println("Neighbours");
            for (uint8_t x = xMin; x <= xMax; x++) {
                for (uint8_t y = yMin; y <= yMax; y++) {
                    uint8_t neighbours = neighbourCount(x, y, _c, buffers[readBuffer], _wrap);
                    // Serial.println(neighbours);
                    const pixel::CRGB currentVal = buffers[readBuffer][_c.XYToIndex(x, y)];
                    bool currentlyAlive = (currentVal != pixel::CRGB(0));
                    if (currentlyAlive) {
                        if (neighbours == 2 || neighbours == 3) {
                            // keep on living
                            buffers[writeBuffer][_c.XYToIndex(x, y)] = currentVal;
                        } else {
                            // kill this cell
                            buffers[writeBuffer][_c.XYToIndex(x, y)] = pixel::CRGB(0);
                        }
                    } else {
                        if (neighbours == 3) {
                            // come to life!
                            buffers[writeBuffer][_c.XYToIndex(x, y)] = _colourGenerator();
                        }
                    }
                }
            }

            // copy buffer into display and count living cells
            uint32_t livingCells = 0;
            for (uint32_t i = 0; i < buffers[writeBuffer].size(); i++) {
                if (buffers[writeBuffer][i] != pixel::CRGB(0)) { livingCells++; }
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
            for (uint32_t i = 0; i < buffers[writeBuffer].size(); i++) { _c[i] = buffers[writeBuffer][i]; }
        }
        uint8_t tempIdx = readBuffer;
        readBuffer = writeBuffer;
        writeBuffer = tempIdx;
    }
    return _c;
}

std::size_t GameOfLife::hashBuffer(const std::vector<pixel::CRGB>& vec) const {
    std::size_t seed = vec.size();
    for (const pixel::CRGB& i : vec) {
        uint8_t val = 1;
        if (i == pixel::CRGB(0)) { val = 0; }
        seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}