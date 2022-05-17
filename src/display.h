#ifndef display_h
#define display_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class PixelDisplay {
public:
    PixelDisplay(Adafruit_NeoPixel& pixels, uint32_t width, uint32_t height, bool serpentine, bool vertical);

    void setXY(uint8_t x, uint8_t y, uint32_t colour);
    void fill(uint32_t colour);

    void update();

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    uint32_t getSize() const { return width * height; }


private:
    const uint32_t width;
    const uint32_t height;
    const bool serpentine;
    const bool vertical;
    Adafruit_NeoPixel& pixels;

    uint32_t XYToIndex(uint8_t x, uint8_t y);
};

#endif // display_h