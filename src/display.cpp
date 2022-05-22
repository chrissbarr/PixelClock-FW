#include <display.h>
#include <Adafruit_NeoPixel.h>
#include "characters.h"

PixelDisplay::PixelDisplay(Adafruit_NeoPixel& pixels, uint8_t width, uint8_t height, bool serpentine, bool vertical, uint32_t pixelOffset) :
    pixels(pixels), width(width), height(height), size(width * height), serpentine(serpentine), vertical(vertical), pixelOffset(pixelOffset)
{
  fullDisplay.xMin = 0;
  fullDisplay.xMax = width - 1;
  fullDisplay.yMin = 0;
  fullDisplay.yMax = height - 1;

  buffer = new uint32_t[size] {0};
}

PixelDisplay::~PixelDisplay()
{
  delete [] buffer;
}

void PixelDisplay::setXY(uint8_t x, uint8_t y, uint32_t colour)
{
    buffer[XYToIndex(x, y)] = colour;
}

uint32_t PixelDisplay::getXY(uint8_t x, uint8_t y) const
{
    return buffer[XYToIndex(x, y)];
}

void PixelDisplay::fill(uint32_t colour, const DisplayRegion& region)
{
  for (uint8_t x = region.xMin; x <= region.xMax; x++) {
    for (uint8_t y = region.yMin; y <= region.yMax; y++) {
      setXY(x, y, colour);
    }
  }
}

void PixelDisplay::fill(uint32_t colour)
{
  for (uint32_t i = 0; i < size; i++) {
    buffer[i] = colour;
  }
}

void PixelDisplay::update() 
{
  for (uint32_t i = 0; i < size; i++) {
    pixels.setPixelColor(i + pixelOffset, buffer[i]);
  }
  pixels.show();
}

void PixelDisplay::showCharacters(const String& string, uint32_t colour, int xOffset, uint8_t spacing)
{
    int xOffsetLocal = 0;
    for (const auto& character : string) {
        FontGlyph g = characterFontArray[charToIndex(character)];
        showCharacter(g, colour, xOffset + xOffsetLocal);
        xOffsetLocal += g.width + spacing;
        if (xOffset + xOffsetLocal > getWidth()) { break; }
    }
}

void PixelDisplay::showCharacter(char character, uint32_t colour, int xOffset)
{
    showCharacter(characterFontArray[charToIndex(character)], colour, xOffset);
}

void PixelDisplay::showCharacter(const FontGlyph& character, uint32_t colour, int xOffset)
{
    for (uint8_t x = 0; x < character.width; x++) {
        int xPos = xOffset + x;
        for (uint8_t y = 0; y < 5; y++) {
            if (bitRead(character.glyph[y], character.width-1-x) == 1) {
                if (uint32_t(xPos) >= getWidth()) { continue; }
                if (xPos < 0) { continue; }
                setXY(uint8_t(xPos), y, colour);
            }
        }
    }
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

bool PixelDisplay::filled(uint32_t colour, const DisplayRegion& region) const
{
  bool filled = true;
  for (uint8_t x = region.xMin; x <= region.xMax; x++) {
    for (uint8_t y = region.yMin; y <= region.yMax; y++) {
      if (getXY(x, y) == colour) { filled = false; }
    }
  }
  return filled;
}

bool PixelDisplay::filled(uint32_t colour) const
{
  bool filled = true;
  for (uint32_t i = 0; i < getSize(); i++) {
    if (buffer[i] == colour) { filled = false; }
  }
  return filled;
}

bool PixelDisplay::empty(const DisplayRegion& region) const
{
  bool empty = true;
  for (uint8_t x = region.xMin; x <= region.xMax; x++) {
    for (uint8_t y = region.yMin; y <= region.yMax; y++) {
      if (getXY(x, y) != 0) { empty = false; }
    }
  }
  return empty;
}

bool PixelDisplay::empty() const
{
  bool empty = true;
  for (uint32_t i = 0; i < getSize(); i++) {
    if (buffer[i] != 0) { empty = false; }
  }
  return empty;
}