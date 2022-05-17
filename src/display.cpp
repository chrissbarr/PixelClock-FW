#include <display.h>
#include <Adafruit_NeoPixel.h>

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