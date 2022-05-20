#ifndef display_h
#define display_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class PixelDisplay {
public:
    PixelDisplay(Adafruit_NeoPixel& pixels, uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset = 0);

    void setXY(uint8_t x, uint8_t y, uint32_t colour);
    uint32_t getXY(uint8_t x, uint8_t y) const;
    void fill(uint32_t colour);

    void showCharacters(const String& string, uint32_t colour, int xOffset, uint8_t spacing = 0);
    void showCharacter(char character, uint32_t colour, int xOffset);

    void update();

    uint8_t getWidth() const { return width; }
    uint8_t getHeight() const { return height; }
    uint32_t getSize() const { return size; }

    bool filled(uint32_t colour = 0) const;
    bool empty() const;


private:
    Adafruit_NeoPixel& pixels;
    const uint8_t width;
    const uint8_t height;
    const uint32_t size;
    const bool serpentine;
    const bool vertical;
    const uint32_t pixelOffset;
    

    uint32_t XYToIndex(uint8_t x, uint8_t y) const;
};

#endif // display_h