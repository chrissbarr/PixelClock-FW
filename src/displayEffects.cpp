#include "displayEffects.h"
#include "display.h"

TextScroller::TextScroller(
  PixelDisplay& display,
  const String& textString,
  uint32_t colour,
  uint16_t timeToHoldAtEnd, 
  bool reverseOnFinish, 
  uint8_t characterSpacing) :
  display(display),
  text(textString),
  colour(colour),
  timeToHoldAtEnd(timeToHoldAtEnd),
  reverseOnFinish(reverseOnFinish),
  charSpacing(characterSpacing)
  {
    lastUpdateTime = millis();
    currentOffset = 0;
    setTargetOffsetToEnd();
  }

  bool TextScroller::run()
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
            _finished = true;
            setTargetOffsetToEnd();
            arrivedAtEndTime = 0;
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
    display.fill(0);
    display.showCharacters(text, colour, -currentOffset, charSpacing);
    return _finished;
  }

  void TextScroller::setTargetOffsetToEnd()
  {
    int end = 0;
    for (char character : text) {
      end += characterFontArray[charToIndex(character)].width + charSpacing;
    }
    targetOffset = end - charSpacing - display.getWidth();
  }

RandomFill::RandomFill(PixelDisplay& display, uint32_t fillInterval, uint32_t(*colourGenerator)(), const DisplayRegion& spawnRegion) :
_display(display), _fillInterval(fillInterval), _colourGenerator(colourGenerator)
{
  if (spawnRegion == defaultFull) {
    _spawnRegion = display.getFullDisplayRegion();
  } else {
    _spawnRegion = spawnRegion;
  }
  reset();
}

bool RandomFill::run()
{
  uint32_t timeNow = millis();

  if (timeNow - _lastSpawnTime >= _fillInterval) {
    if (!_display.filled(0, _spawnRegion)) {
      bool filledPixel = false;
      while (!filledPixel) {
        uint8_t x = random(_spawnRegion.xMin, _spawnRegion.xMax + 1);
        uint8_t y = random(_spawnRegion.yMin, _spawnRegion.yMax + 1);
        //Serial.print("X: "); Serial.print(x); Serial.print(" Y: "); Serial.println(y);
        if (_display.getXY(x, y) == uint32_t(0)) {
          _display.setXY(x, y, _colourGenerator());
          filledPixel = true;
          _lastSpawnTime = timeNow;
        }
      }
    } else {
      _finished = true;
    }
  }
  return _finished;
}

// bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)(), DisplayRegion displayRegion)
// {
//   static uint32_t lastMoveTime = 0;
//   uint32_t timeNow = millis();

//   DisplayRegion spawnZone;
//   spawnZone.xMin = displayRegion.xMin;
//   spawnZone.xMax = displayRegion.xMax;
//   spawnZone.yMin = displayRegion.yMin;
//   spawnZone.yMax = displayRegion.yMin;

//   if (fillInterval != 0) {
//     fillRandomly(display, fillInterval, colourGenerator, spawnZone);
//   }

//   if (moveInterval != 0) {
//     // Move all pixels down
//     if (timeNow - lastMoveTime > moveInterval) {
//       for (int y = displayRegion.yMax; y >= displayRegion.yMin; y--) {
//         for (uint8_t x = displayRegion.xMin; x <= displayRegion.xMax; x++) {
//           uint32_t cellColour = display.getXY(x, y);
//           if (cellColour != 0) {
//             // if this is the last row
//             if (y == displayRegion.yMax) {
//               if (empty) {
//                 display.setXY(x, y, 0);
//               }
//               continue;
//             }
//             if (display.getXY(x, y + 1) == uint32_t(0)) {
//               display.setXY(x, y + 1, cellColour);
//               display.setXY(x, y, 0);
//             }
//           }
//         }
//       }
//       lastMoveTime = timeNow;
//     }  
//   }
//   return !display.filled(0, displayRegion);
// }

// void tetris(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval)
// {
//   gravityFill(display, fillInterval, moveInterval, false, [](){
//     const uint32_t choices[] = {
//       Adafruit_NeoPixel::Color(255, 0, 0),
//       Adafruit_NeoPixel::Color(0, 255, 0),
//       Adafruit_NeoPixel::Color(0, 0, 255),
//     };
//     return choices[random(sizeof(choices)/sizeof(choices[0]))];
//   });

//   bool setFound = false;
//   uint8_t setSize = 3;
//   uint8_t setXCoords[3];
//   uint8_t setYCoords[3];

//   auto checkMatch = [&](uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3) {
//     uint32_t val1 = display.getXY(x1, y1);
//     if (val1 == 0) { return; }
//     uint32_t val2 = display.getXY(x2, y2);
//     uint32_t val3 = display.getXY(x3, y3);

//     if (val1 == val2 && val2 == val3) { 
//       setFound = true;
//       setXCoords[0] = x1;
//       setXCoords[1] = x2;
//       setXCoords[2] = x3;
//       setYCoords[0] = y1;
//       setYCoords[1] = y2;
//       setYCoords[2] = y3;
//     }
//   };

//   for (uint8_t x = 0; x < display.getWidth(); x++) {
//     for (uint8_t y = 0; y < display.getHeight(); y++) {
//       if (x > 0 && x < display.getWidth() - 1) {
//         checkMatch(x - 1, y, x, y, x + 1, y);
//       }
//       if (y > 0 && y < display.getHeight() - 1) {
//         checkMatch(x, y - 1, x, y, x, y + 1);
//       }
//     }
//   }

//   if (setFound) {
//     for (int j = 0; j < setSize; j++) {
//       display.setXY(setXCoords[j], setYCoords[j], 0);
//     }
//   }
// }

void showTime(PixelDisplay& display, int hour, int minute, uint32_t colour)
{
  constexpr uint8_t bufSize = 6;
  char c_buf[bufSize];
  snprintf(c_buf, bufSize, "%2d:%02d", hour, minute);
  display.showCharacters(String(c_buf), colour, 0, 1);
}

void displayDiagnostic(PixelDisplay& display)
{
  // Clear display
  display.fill(0);
  display.update();
  delay(250);

  // Show Pixel 0
  display.setXY(0, 0, Adafruit_NeoPixel::Color(255, 0, 0));
  display.update();
  delay(250);

  // Solid Red, Green, Blue
  display.fill(Adafruit_NeoPixel::Color(255, 0, 0));
  display.update();
  delay(250);
  display.fill(Adafruit_NeoPixel::Color(0, 255, 0));
  display.update();
  delay(250);
  display.fill(Adafruit_NeoPixel::Color(0, 0, 255));
  display.update();
  delay(250);

  // Move through XY sequentially
  for (uint8_t y = 0; y < display.getHeight(); y++) {
    for (uint8_t x = 0; x < display.getWidth(); x++) {
    display.fill(0);
    display.setXY(x, y, Adafruit_NeoPixel::Color(100, 0, 0));
    display.update();
    delay(1);
    }
  }

  // Scroll short test
  display.fill(0);
  display.update();
  auto textScrollTest1 = TextScroller(
    display,
    "Hello - Testing!",
    Adafruit_NeoPixel::Color(0, 0, 255),
    500,
    true,
    1
  );
  while(!textScrollTest1.run()) {
    display.update();
    display.fill(0);
  }

  // Scroll full character set
  display.fill(0);
  display.update();
  auto textScrollTest = TextScroller(
    display,
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 !\"#$%&'()*+'-./:;<=>?@",
    Adafruit_NeoPixel::Color(0, 255, 0),
    500,
    false,
    1
  );
  while(!textScrollTest.run()) {
    display.update();
    display.fill(0);
  }
  display.fill(0);
  display.update();
}