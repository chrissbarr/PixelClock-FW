#ifndef display_h
#define display_h

/* Project Scope */
#include "flm_pixeltypes.h"
#include "instrumentation.h"

/* C++ Standard Library */
#include <cstdint>

/* Forward Declarations */
namespace canvas {
class Canvas;
}

class Display : public Instrumented {
public:
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual void update(const canvas::Canvas& canvas) = 0;
    virtual uint8_t getWidth() const = 0;
    virtual uint8_t getHeight() const = 0;
    virtual uint32_t getSize() const = 0;
};

void displayDiagnostic(Display& display);

#endif // display_h