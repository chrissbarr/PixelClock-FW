#ifndef display_h
#define display_h

/* C++ Standard Library */
#include <cstdint>

#include "flm_pixeltypes.h"

/* Forward Declarations */
namespace canvas {
class Canvas;
}

class Display {
public:
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual void update(const canvas::Canvas& canvas) = 0;
    virtual uint8_t getWidth() const = 0;
    virtual uint8_t getHeight() const = 0;
    virtual uint32_t getSize() const = 0;
};

void displayDiagnostic(Display& display);

#endif // display_h