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
      _wrap(wrap) {}

void GameOfLife::reset() {
    _lastLoopTime = 0;
    _finished = false;

    printing::print("GoL Reset\n");

    if (game) {
        // collect stats from last run
        GoLScore prevScore{};
        prevScore.seed = game->getSeed();
        prevScore.lifespan = game->getLifespan();
        printing::print(fmt::format("GoL Last Game Score: {}\n", prevScore));
    }

    GoLRules rules{};
    rules.height = _c.getHeight();
    rules.width = _c.getWidth();
    rules.wrap = _wrap;
    rules.staleStateStepsAllowed = 20;

    const uint32_t millisecondsAllowedForSeedSimulation = 50;
    const uint32_t seedSimStartTime = millis();
    uint32_t seedSimEndTime = seedSimStartTime;
    uint32_t seedSimDuration = 0;
    uint32_t iterations = 0;

    printing::print(fmt::format("GoL Running Seed Simulation, start = {} ms\n", seedSimStartTime));

    while (seedSimDuration < millisecondsAllowedForSeedSimulation) {

        // create a dummy game and run it until it dies (or times out)
        auto seedGame = std::make_unique<GameOfLifeGame>(rules, millis());
        const uint32_t maxLifespan = 5000;
        while (seedGame->getAlive() && seedGame->getLifespan() < maxLifespan) { seedGame->tick(); }

        // collect stats from dummy game
        GoLScore score{};
        score.seed = seedGame->getSeed();
        score.lifespan = seedGame->getLifespan();
        // add them to the best scores container
        bestScores.insert(score);
        // if container at max capacity, remove the lowest score
        if (bestScores.size() > bestScoresToKeep) { bestScores.erase(bestScores.begin()); }
        // printing::print(
        //     fmt::format("GoL Seed Simulation Score: {}\n", score));

        seedSimEndTime = millis();
        seedSimDuration = seedSimEndTime - seedSimStartTime;
        iterations++;
    }

    printing::print(
        fmt::format("GoL Seed Simulation Finished, duration = {} ms, iterations = {}\n", seedSimDuration, iterations));

    printing::print("GoL Saved Scores: \n");
    for (const auto& s : bestScores) { printing::print(fmt::format("  {}\n", s)); }

    // pick a random score from the best scores container to use for the actual display game
    std::uniform_int_distribution<std::size_t> dist(0, bestScores.size() - 1);
    std::size_t randomIndex = dist(rand);
    auto randomScoreToRepeat = (*std::next(bestScores.begin(), randomIndex));
    printing::print(fmt::format("GoL Repeating Score: {}\n", randomScoreToRepeat));
    game = std::make_unique<GameOfLifeGame>(rules, randomScoreToRepeat.seed);

    _c = canvas::Canvas(rules.width, rules.height);
    _c.fill(flm::CRGB::Black);
}

canvas::Canvas GameOfLife::run() {

    // this shouldn't happen, exit early if it does
    if (!game) {
        _finished = true;
        return _c;
    }

    if (game->getAlive()) {
        if (millis() - _lastLoopTime >= _updateInterval) {

            game->tick();

            int width = game->getRules().width;
            int height = game->getRules().height;
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    const auto val = game->getData().at(game->XYToIndex(x, y));
                    if (val == 0) { _c.setXY(x, y, flm::CRGB::Black); }
                    if (val != 0) {
                        if (_c.getXY(x, y) == flm::CRGB::Black) { _c.setXY(x, y, _colourGenerator()); }
                    }
                }
            }

            _lastLoopTime = millis();
        }

    } else {
        if (!_fadeOnDeath) {
            _finished = true;
        } else {
            if (millis() - _lastLoopTime >= _fadeInterval) {

                // fade all pixels
                for (std::size_t i = 0; i < _c.getSize(); i++) { _c[i] = _c[i].fadeToBlackBy(10); }
                _lastLoopTime = millis();
            }
            bool empty = true;
            for (std::size_t i = 0; i < _c.getSize(); i++) {
                if (_c[i] != flm::CRGB(0)) {
                    empty = false;
                    break;
                }
            }

            if (empty) { _finished = true; }
        }
    }

    if (_filter) {
        canvas::Canvas temp(_c);
        _filter->apply(temp);
        return temp;
    }

    return _c;
}

GameOfLifeGame::GameOfLifeGame(GoLRules rules, uint32_t seed) : rules(rules), seed(seed) {
    // zero the data
    data = std::vector<uint32_t>(rules.width * rules.height, 0);

    // setup RNG
    rand.seed(seed);

    // seed initial state
    std::uniform_int_distribution<int> dist(0, 10);
    for (auto& v : data) {
        int chance = dist(rand);
        if (chance == 0) { v = 1; }
    }

    alive = true;
}

void GameOfLifeGame::tick() {

    currentTick++;

    auto newData = std::vector<uint32_t>(rules.width * rules.height, 0);

    int xMin = 0;
    int xMax = rules.width - 1;
    int yMin = 0;
    int yMax = rules.height - 1;

    for (int x = xMin; x <= xMax; x++) {
        for (int y = yMin; y <= yMax; y++) {
            const int neighbours = neighbourCount(x, y, rules.width, rules.height, data, rules.wrap);
            const auto currentVal = data[XYToIndex(x, y)];
            bool currentlyAlive = (currentVal != 0);

            if (currentlyAlive) {
                if (neighbours == 2 || neighbours == 3) {
                    // keep on living
                    newData[XYToIndex(x, y)] = currentVal;
                } else {
                    // kill this cell
                    newData[XYToIndex(x, y)] = 0;
                }
            } else {
                if (neighbours == 3) {
                    // come to life!
                    newData[XYToIndex(x, y)] = currentTick;
                }
            }
        }
    }

    data = newData;

    // count living cells
    uint32_t livingCells = 0;
    for (std::size_t i = 0; i < data.size(); i++) {
        if (data[i] != 0) { livingCells++; }
    }

    // test for simplest death state first to avoid expensive checks later
    if (livingCells == 0) { alive = false; }

    // test for more complex death states (frozen or repetitive pattern)
    if (alive) {

        lifespan = currentTick;

        // convert the current state to a hash and compare against previous states
        auto currentStateHash = hashState(data);
        if (previousStateHashes.size() > 0) {
            bool unique = true;
            for (const auto& hash : previousStateHashes) {
                if (currentStateHash == hash) { unique = false; }
            }
            if (!unique) {
                notUniqueForNSteps++;
            } else {
                notUniqueForNSteps = 0;
            }

            // if state is identical two steps in a row, then we are frozen.
            if (currentStateHash == previousStateHashes.back()) { alive = false; }

            // if there has been no new state for X steps, consider stale.
            if (notUniqueForNSteps >= rules.staleStateStepsAllowed) { alive = false; }
        }
        previousStateHashes.push(currentStateHash);
    }
}

std::size_t GameOfLifeGame::XYToIndex(int x, int y) const { return (y * rules.width) + x; }

int GameOfLifeGame::neighbourCount(
    int xPos, int yPos, int width, int height, const std::vector<uint32_t>& dataIn, bool wrap) const {
    uint8_t aliveCount = 0;

    const int xMin = 0;
    const int xMax = width - 1;
    const int yMin = 0;
    const int yMax = height - 1;

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
            if (dataIn.at(XYToIndex(testX, testY)) != 0) { aliveCount++; }
        }
    }
    return aliveCount;
}

std::size_t GameOfLifeGame::hashState(const std::vector<uint32_t>& state) const {
    std::size_t seedVal = state.size();
    for (const auto i : state) {
        uint8_t val = 0;
        if (i > 0) { val = 1; }
        seedVal ^= val + 0x9e3779b9 + (seedVal << 6) + (seedVal >> 2);
    }
    return seedVal;
}