#ifndef dummydisplay_h
#define dummydisplay_h

/* Project Scope */
#include "display/display.h"
#include "display/canvas.h"
#include "flm_pixeltypes.h"

/* C++ Standard Library */
#include <cstdint>

class DummyDisplay : public Display {
public:
    DummyDisplay(uint8_t width, uint8_t height) : width(width), height(height), size(width * height) {};
    ~DummyDisplay() {};
    void setBrightness(uint8_t brightness) override final { this->brightness = brightness; }
    void update(const canvas::Canvas& canvas) override final { this->c = canvas; };
    uint8_t getWidth() const override final { return width; }
    uint8_t getHeight() const override final { return height; }
    uint32_t getSize() const override final { return size; }
    uint32_t XYToIndex(uint8_t x, uint8_t y) const;

    canvas::Canvas getCanvas() const { return c; }

private:
    const uint8_t width;
    const uint8_t height;
    const uint32_t size;
    canvas::Canvas c;
    uint8_t brightness{255};
};

#endif // dummydisplay_h