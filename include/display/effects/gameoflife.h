// #ifndef gameoflife_h
// #define gameoflife_h

// /* Project Scope */
// #include "display/displayEffects.h"

// /* C++ Standard Library */
// #include <deque>
// #include <set>

// struct GoLScore {
//     uint32_t seed;
//     uint16_t lifespan;
// };

// constexpr bool operator<(const GoLScore& x, const GoLScore& y) { return x.lifespan < y.lifespan; }

// class GameOfLife : public DisplayEffect {
// public:
//     GameOfLife(
//         PixelDisplay& display,
//         uint32_t updateInterval,
//         uint32_t fadeInterval,
//         CRGB (*colourGenerator)(),
//         const DisplayRegion& displayRegion = defaultFull,
//         bool wrap = true);
//     bool run() override final;
//     bool finished() const override final { return _finished; }
//     void reset() override final;

//     void setUpdateInterval(uint32_t interval) { _updateInterval = interval; }
//     void setFadeInterval(uint32_t interval) { _fadeInterval = interval; }
//     void setSeedingMode(bool enabled) { _seeding = enabled; }
//     std::size_t getSeededCount() const { return bestScores.size(); }
//     void setFadeOnDeath(bool fade) { _fadeOnDeath = fade; }
//     const std::multiset<GoLScore>& getScores() const { return bestScores; }
//     void setScores(const std::multiset<GoLScore>& scores) { bestScores = scores; }
//     uint32_t getIterations() const { return iterationId; }
//     uint16_t getLifespan() const { return _lifespan; }

//     void seedDisplay();

// private:
//     PixelDisplay& _display;
//     uint32_t _lastLoopTime;
//     CRGB (*_colourGenerator)();
//     DisplayRegion _displayRegion;
//     bool _dead = false;
//     bool _finished;
//     bool _wrap;
//     bool _fadeOnDeath = true;
//     uint32_t _updateInterval;
//     uint32_t _fadeInterval;
//     uint32_t _notUniqueForNSteps;

//     uint32_t _lastSeed = 0;
//     uint16_t _lifespan = 0;

//     std::multiset<GoLScore> bestScores;
//     uint8_t bestScoresToKeep = 20;
//     uint32_t iterationId = 0;
//     bool _seeding = false;

//     uint8_t readBuffer = 0;
//     uint8_t writeBuffer = 1;
//     std::vector<std::vector<CRGB>> buffers;
//     std::deque<std::size_t> bufferHashes;

//     std::size_t hashBuffer(const std::vector<CRGB>& vec) const;
// };

// #endif // gameoflife_h