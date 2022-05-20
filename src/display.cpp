#include <display.h>
#include <Adafruit_NeoPixel.h>

#include "characters.h"

PixelDisplay::PixelDisplay(Adafruit_NeoPixel& pixels, uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset) :
    pixels(pixels), width(width), height(height), size(width * height), serpentine(serpentine), vertical(vertical), pixelOffset(pixelOffset)
{

}

void PixelDisplay::setXY(uint8_t x, uint8_t y, uint32_t colour)
{
    pixels.setPixelColor(XYToIndex(x, y) + pixelOffset, colour);
}

uint32_t PixelDisplay::getXY(uint8_t x, uint8_t y) const
{
    return pixels.getPixelColor(XYToIndex(x, y) + pixelOffset);
}

void PixelDisplay::fill(uint32_t colour)
{
    pixels.fill(colour, pixelOffset, getSize());
}

void PixelDisplay::update() 
{
    pixels.show();
}

void PixelDisplay::showCharacters(const String& string, uint32_t colour, int xOffset, uint8_t spacing)
{
    int xOffsetLocal = 0;
    for (const auto& character : string) {
        showCharacter(character, colour, xOffset + xOffsetLocal);
        xOffsetLocal += 3 + spacing;
    }
}

void PixelDisplay::showCharacter(char character, uint32_t colour, int xOffset)
{
    uint8_t index = 0;
    if (character >= ' ' && character <= 'Z') {
        index = character - ' ';
    }

    for (uint8_t x = 0; x < 3; x++) {
        int xPos = xOffset + x;
        for (uint8_t y = 0; y < 5; y++) {
            if (bitRead(characterFontArray[index][y], 2-x) == 1) {
                if (uint32_t(xPos) >= getWidth()) { continue; }
                if (xPos < 0) { continue; }
                setXY(uint8_t(xPos), y, colour);
            }
        }
    }

    // if (character >= 'a' && character <= 'z') {
    //     index = character - '';
    // }
}

uint32_t PixelDisplay::XYToIndex(uint8_t x, uint8_t y) const
{
  uint16_t i;
  
  if( serpentine == false) {
    if (vertical == false) {
      i = (y * width) + x;
    } else {
      i = height * (width - (x+1))+y;
    }
  }

  if( serpentine == true) {
    if (vertical == false) {
      if( y & 0x01) {
        // Odd rows run backwards
        uint8_t reverseX = (width - 1) - x;
        i = (y * width) + reverseX;
      } else {
        // Even rows run forwards
        i = (y * width) + x;
      }
    } else { // vertical positioning
      if ( x & 0x01) {
        i = height * (width - (x+1))+y;
      } else {
        i = height * (width - x) - (y+1);
      }
    }
  }
  
  return i;
}

bool PixelDisplay::filled(uint32_t colour) const
{
  bool filled = true;
  for (uint32_t i = 0; i < getSize(); i++) {
    if (pixels.getPixelColor(i + pixelOffset) == colour) { filled = false; }
  }
  return filled;
}

bool PixelDisplay::empty() const
{
  bool empty = true;
  for (uint32_t i = 0; i < getSize(); i++) {
    if (pixels.getPixelColor(i + pixelOffset) != 0) { empty = false; }
  }
  return empty;
}