#ifndef pixeldisplay_h
#define pixeldisplay_h

/* Project Scope */
#include "display/display.h"
#include "flm_pixeltypes.h"

/* C++ Standard Library */
#include <cstdint>

/* Forward Declarations */
class CRGB;
namespace canvas {
class Canvas;
}

class PixelDisplay : public Display {
public:
    PixelDisplay(uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset = 0);
    ~PixelDisplay();

    void setBrightness(uint8_t brightness) override final { this->brightness = brightness; }
    void update(const canvas::Canvas& canvas) override final;

    uint8_t getWidth() const override final { return width; }
    uint8_t getHeight() const override final { return height; }
    uint32_t getSize() const override final { return size; }

    uint32_t XYToIndex(uint8_t x, uint8_t y) const;

    // Instrumentation
    std::vector<InstrumentationTrace*> getInstrumentation() override final;

private:
    CRGB* leds = nullptr;
    const uint8_t width;
    const uint8_t height;
    const uint32_t size;
    const bool serpentine;
    const bool vertical;
    const uint32_t pixelOffset;
    uint8_t brightness{255};

    // Instrumentation
    InstrumentationTrace traceUpdateTotal{"Display Update - Overall"};
    InstrumentationTrace traceUpdateLEDWrite{"Display Update - LED Output"};
};

#endif // pixeldisplay_h