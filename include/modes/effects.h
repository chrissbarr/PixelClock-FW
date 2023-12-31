#ifndef modes_effects_h
#define modes_effects_h

/* Project Scope */
#include "display/effects/effect.h"
#include "modes/modes.h"

/* C++ Standard Library */
#include <memory>
#include <vector>

class Mode_Effects : public MainModeFunction {
public:
    Mode_Effects(const canvas::Canvas& size, ButtonReferences buttons);

protected:
    void moveIntoCore() override final;
    canvas::Canvas runCore() override final;
    void moveOutCore() override final {}

private:
    std::vector<std::shared_ptr<DisplayEffect>> effects;
    std::size_t effectIndex = 0;
};

#endif // modes_effects_h