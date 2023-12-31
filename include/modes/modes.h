#ifndef modes_modes_h
#define modes_modes_h

/* Project Scope */
#include "display/canvas.h"
#include "display/display.h"
#include "display/effects/effect.h"
#include "display/effects/filters.h"
#include "display/effects/textscroller.h"
#include "instrumentation.h"
#include "timekeeping.h"

/* Libraries */
#include <Button2.h>

/* C++ Standard Library */
#include <memory>
#include <string>
#include <vector>

struct ButtonReferences {
    Button2& mode;
    Button2& select;
    Button2& left;
    Button2& right;
};

class MainModeFunction {
public:
    MainModeFunction(std::string name, ButtonReferences buttons) : _name(name), buttons(buttons) {}
    // should be called by the parent when moving into this mode
    void moveInto();
    // should be called by the parent when this mode is active
    canvas::Canvas run();
    // should be called by the parent when moving out of this mode
    void moveOut();
    // indicates that this mode is ready to exit/return
    virtual bool finished() const { return _finished; }
    // get the name of this mode
    std::string getName() const { return _name; }

protected:
    virtual void moveIntoCore();
    virtual void moveOutCore() = 0;
    virtual canvas::Canvas runCore() = 0;
    bool _finished = false;
    ButtonReferences buttons;
    std::string _name;

private:
    void clearAllButtonCallbacks(Button2& button);
};

class ModeManager : public Instrumented {
public:
    ModeManager(const canvas::Canvas& size, ButtonReferences buttons);
    void cycleMode();
    canvas::Canvas run();

    // Instrumentation
    std::vector<InstrumentationTrace*> getInstrumentation() override final { return {&traceRunTotal}; }

private:
    std::vector<std::unique_ptr<MainModeFunction>> modes;
    uint8_t modeIndex = 0;

    // Instrumentation
    InstrumentationTrace traceRunTotal{"Mode Run - Overall"};
};

#endif // modes_modes_h