#ifndef clockfaces_h
#define clockfaces_h

/* Project Scope */
#include "display/canvas.h"
#include "display/effects/effect.h"
#include "display/effects/gravity.h"
#include "display/effects/gravityfill.h"
#include "timekeeping.h"

/* C++ Standard Library */
#include <functional>
#include <memory>
#include <random>

class ClockFace_Base : public DisplayEffect {
public:
    ClockFace_Base(std::function<ClockFaceTimeStruct(void)> timeCallbackFunction)
        : timeCallbackFunction(timeCallbackFunction) {}
    virtual canvas::Canvas run() override = 0;
    virtual bool finished() const override = 0;
    virtual void reset() override = 0;

protected:
    std::function<ClockFaceTimeStruct(void)> timeCallbackFunction;
};

class ClockFace_Simple : public ClockFace_Base {
public:
    ClockFace_Simple(std::function<ClockFaceTimeStruct(void)> timeCallbackFunction)
        : ClockFace_Base(timeCallbackFunction) {}
    canvas::Canvas run() override final;
    bool finished() const override final { return false; }
    void reset() override final{};
};

class ClockFace_Gravity : public ClockFace_Base {
public:
    ClockFace_Gravity(std::function<ClockFaceTimeStruct(void)> timeCallbackFunction);
    canvas::Canvas run() override final;
    bool finished() const override final { return false; }
    void reset() override final;

private:
    canvas::Canvas _c;
    std::unique_ptr<ClockFace_Simple> clockFace;
    std::unique_ptr<Gravity> gravityEffect;
    ClockFaceTimeStruct timePrev;
    enum class State { stable, fallToBottom, fallOut };
    State currentState = State::stable;
};

class ClockFace_GravityFill : public ClockFace_Base {
public:
    ClockFace_GravityFill(
        std::function<ClockFaceTimeStruct(void)> timeCallbackFunction,
        std::unique_ptr<GravityFillTemplate> gravFillTemplate);
    canvas::Canvas run() override final;
    bool finished() const override final { return false; }
    void reset() override final;

private:
    canvas::Canvas _c;
    ClockFaceTimeStruct timePrev;
    std::unique_ptr<GravityFillTemplate> gravFill;
};

#endif // clockfaces_h