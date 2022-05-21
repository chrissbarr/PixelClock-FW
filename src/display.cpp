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

    // Lowercase to uppercase
    if (character >= 'a' && character <= 'z') {
        character -= 32;
    }

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
    if (pixels.getPixelColor(i + pixelOffset) == colour) { filled = false; }
  }
  return filled;
}

bool PixelDisplay::empty(const DisplayRegion& region) const
{
  bool empty = true;
  for (uint8_t x = region.xMin; x < region.xMax; x++) {
    for (uint8_t y = region.yMin; y < region.yMax; y++) {
      if (getXY(x, y) != 0) { empty = false; }
    }
  }
  return empty;
}

bool PixelDisplay::empty() const
{
  bool empty = true;
  for (uint32_t i = 0; i < getSize(); i++) {
    if (pixels.getPixelColor(i + pixelOffset) != 0) { empty = false; }
  }
  return empty;
}

TextScroller::TextScroller(
  PixelDisplay& display, 
  const String& textString, 
  uint16_t timeToHoldAtEnd, 
  bool reverseOnFinish, 
  uint8_t characterSpacing) :
  display(display),
  text(textString),
  timeToHoldAtEnd(timeToHoldAtEnd),
  reverseOnFinish(reverseOnFinish),
  charSpacing(characterSpacing)
  {
    lastUpdateTime = millis();
    currentOffset = 0;
    targetOffset = textString.length() * (3 + characterSpacing) - display.getWidth();
  }

  bool TextScroller::update(uint32_t colour, uint32_t stepDelay)
  {
    if (currentOffset == targetOffset) {
      if (arrivedAtEndTime == 0) {
        arrivedAtEndTime = millis();
      } else {
        if (millis() - arrivedAtEndTime > timeToHoldAtEnd) {
          if (reverseOnFinish && currentOffset != 0) {
            targetOffset = 0;
            arrivedAtEndTime = 0;
          } else {
            finished = true;
          }
        }
      }
    } else {
      if (millis() - lastUpdateTime >= stepDelay) {
        if (targetOffset > currentOffset) {
          currentOffset += 1;
        } else {
          currentOffset -= 1;
        }
        lastUpdateTime = millis();
      }
    }

    display.showCharacters(text, colour, -currentOffset, charSpacing);
    return finished;
  }