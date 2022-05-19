#ifndef display_h
#define display_h

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class PixelDisplay {
public:
    PixelDisplay(Adafruit_NeoPixel& pixels, uint32_t width, uint32_t height, bool serpentine, bool vertical);

    void setXY(uint8_t x, uint8_t y, uint32_t colour);
    uint32_t getXY(uint8_t x, uint8_t y) const;
    void fill(uint32_t colour);

    void showCharacters(const String& string, uint32_t colour, int xOffset, uint8_t spacing = 0);
    void showCharacter(char character, uint32_t colour, int xOffset);

    void update();

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    uint32_t getSize() const { return size; }

    bool filled() const;


private:
    const uint32_t width;
    const uint32_t height;
    const uint32_t size;
    const bool serpentine;
    const bool vertical;
    Adafruit_NeoPixel& pixels;

    uint32_t XYToIndex(uint8_t x, uint8_t y) const;
};

#endif // display_h