#ifndef modes_effects_h
#define modes_effects_h

/* Project Scope */
#include "display/effects/effect.h"
#include "modes/modes.h"

/* C++ Standard Library */
#include <memory>
#include <vector>

struct EffectId {
    std::string name;
    std::shared_ptr<DisplayEffect> ptr;
};

class Mode_Effects : public MainModeFunction {
public:
    Mode_Effects(const canvas::Canvas& size, ButtonReferences buttons);

protected:
    void moveIntoCore() override final;
    canvas::Canvas runCore() override final;
    void moveOutCore() override final {}

private:
    std::vector<EffectId> effects;
    std::size_t effectIndex = 0;

    int transitionDir = 0;
    float transitionPercentage = 0.0;

    enum class State {
        Stable,
        Transition,
    };

    uint32_t lastLoopTime = 0;

    State currentState = State::Stable;
};

#endif // modes_effects_h