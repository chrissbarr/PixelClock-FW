#include <display.h>
#include <Adafruit_NeoPixel.h>

#include "characters.h"

PixelDisplay::PixelDisplay(Adafruit_NeoPixel& pixels, uint32_t width, uint32_t height, bool serpentine, bool vertical) :
    pixels(pixels), width(width), height(height), serpentine(serpentine), vertical(vertical)
{

}

void PixelDisplay::setXY(uint8_t x, uint8_t y, uint32_t colour)
{
    pixels.setPixelColor(XYToIndex(x, y), colour);
}

void PixelDisplay::fill(uint32_t colour)
{
    pixels.fill(colour, 0, width * height);
}

void PixelDisplay::update() 
{
    pixels.show();
}

void PixelDisplay::showCharacters(String string, uint32_t colour, int xOffset, uint8_t spacing)
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
        for (uint8_t y = 0; y < 5; y++) {
            if (bitRead(characterFontArray[index][y], 2-x) == 1) {
                int xPos = xOffset + x;
                if (xPos >= getWidth()) { continue; }
                if (xPos < 0) { continue; }
                setXY(uint8_t(xPos), y, colour);
            }
        }
    }

    // if (character >= 'a' && character <= 'z') {
    //     index = character - '';
    // }
}

uint32_t PixelDisplay::XYToIndex( uint8_t x, uint8_t y)
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