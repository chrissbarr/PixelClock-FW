#ifndef gameoflife_h
#define gameoflife_h

/* Project Scope */
#include "FMTWrapper.h"
#include "display/canvas.h"
#include "display/effects/effect.h"
#include "display/effects/utilities.h"

/* Libraries */
#include <etl/circular_buffer.h>

/* C++ Standard Library */
#include <random>
#include <set>
#include <vector>

struct GOLRules {
    int width;
    int height;
    bool wrap;
    int staleStateStepsAllowed;
};

class GameOfLifeGame {
public:
    GameOfLifeGame(GOLRules rules, uint32_t seed);
    void tick();
    bool getAlive() const { return alive; }
    uint32_t getLifespan() const { return lifespan; }
    std::vector<uint32_t>& getData() { return data; }
    GOLRules& getRules() { return rules; }
    std::size_t getSeed() { return seed; }
    std::size_t XYToIndex(int x, int y) const;

private:
    int neighbourCount(int xPos, int yPos, int width, int height, const std::vector<uint32_t>& data, bool wrap) const;
    std::size_t hashState(const std::vector<uint32_t>& state) const;

    uint32_t seed;
    GOLRules rules;
    std::vector<uint32_t> data;
    bool alive{};
    std::minstd_rand rand;

    uint32_t currentTick{0};
    uint32_t lifespan{0};

    etl::circular_buffer<std::size_t, 100> previousStateHashes;
    uint32_t notUniqueForNSteps = 0;
};

struct GoLScore {
    uint32_t seed;
    uint32_t lifespan;
};

template <> class fmt::formatter<GoLScore> {
public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename Context> constexpr auto format(const GoLScore& s, Context& ctx) const {
        return format_to(ctx.out(), "(Seed = {:10d}, Lifespan = {:04d})", s.seed, s.lifespan);
    }
};

constexpr bool operator<(const GoLScore& x, const GoLScore& y) { return x.lifespan < y.lifespan; }

class GameOfLife : public DisplayEffect {
public:
    GameOfLife(
        const canvas::Canvas& size,
        uint32_t updateInterval,
        uint32_t fadeInterval,
        colourGenerator::Generator colourGenerator,
        bool wrap = true);
    canvas::Canvas run() override final;
    bool finished() const override final { return _finished; }
    void reset() override final;

    void setUpdateInterval(uint32_t interval) { _updateInterval = interval; }
    void setFadeInterval(uint32_t interval) { _fadeInterval = interval; }
    // void setSeedingMode(bool enabled) { _seeding = enabled; }
    // std::size_t getSeededCount() const { return bestScores.size(); }
    void setFadeOnDeath(bool fade) { _fadeOnDeath = fade; }
    const std::multiset<GoLScore>& getScores() const { return bestScores; }
    void setScores(const std::multiset<GoLScore>& scores) { bestScores = scores; }
    // uint32_t getIterations() const { return iterationId; }
    // uint16_t getLifespan() const { return _lifespan; }

private:
    canvas::Canvas _c;
    uint32_t _lastLoopTime;
    colourGenerator::Generator _colourGenerator;
    bool _finished;
    bool _wrap;
    bool _fadeOnDeath = true;
    uint32_t _updateInterval;
    uint32_t _fadeInterval;

    std::multiset<GoLScore> bestScores;
    uint8_t bestScoresToKeep = 20;

    std::minstd_rand rand;
    std::unique_ptr<GameOfLifeGame> game;
};

#endif // gameoflife_h