#ifndef modes_clockface_h
#define modes_clockface_h

/* Project Scope */
#include "display/effects/effect.h"
#include "display/effects/filters.h"
#include "modes/modes.h"

/* C++ Standard Library */
#include <memory>
#include <string>
#include <vector>
#include <random>

class Mode_ClockFace : public MainModeFunction {
public:
    Mode_ClockFace(ButtonReferences buttons);

protected:
    void moveIntoCore() override final;
    canvas::Canvas runCore() override final;
    void moveOutCore() override final {}

private:
    std::vector<std::unique_ptr<DisplayEffect>> faces;
    std::size_t clockfaceIndex = 0;
    std::vector<std::unique_ptr<FilterMethod>> filters;
    std::size_t filterIndex = 0;
    uint32_t lastFilterChangeTime = 0;
    uint32_t filterChangePeriod = 10000;
    ClockFaceTimeStruct timePrev;
    std::minstd_rand rand;
};

#endif // modes_clockface_h