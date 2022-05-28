#include "display/displayEffects.h"
#include "display/display.h"

#include <random>

TextScroller::TextScroller(
  PixelDisplay& display,
  String textString,
  CRGB colour,
  uint16_t stepDelay,
  uint16_t timeToHoldAtEnd, 
  bool reverseOnFinish, 
  uint8_t characterSpacing) :
  display(display),
  text(textString),
  colour(colour),
  stepDelay(stepDelay),
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
    display.fill(CRGB::Black);
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

RandomFill::RandomFill(PixelDisplay& display, uint32_t fillInterval, CRGB(*colourGenerator)(), const DisplayRegion& spawnRegion) :
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
        if (_display.getXY(x, y) == CRGB(0)) {
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

BouncingBall::BouncingBall(PixelDisplay& display, uint32_t updateInterval, CRGB(*colourGenerator)(), const DisplayRegion& displayRegion) :
  _display(display), _updateInterval(updateInterval), _colourGenerator(colourGenerator)
{
  if (displayRegion == defaultFull) {
    _displayRegion = display.getFullDisplayRegion();
  } else {
    _displayRegion = displayRegion;
  }
}

void BouncingBall::reset()
{
  ballx = random(_displayRegion.xMin + 1, _displayRegion.xMax);
  bally = random(_displayRegion.yMin + 1, _displayRegion.yMax);
  xDir = 1;
  yDir = 1;
  _finished = false; 
  _lastLoopTime = millis();
  _display.fill(0, _displayRegion);
}

bool BouncingBall::run()
{
  uint32_t millisSinceLastRun = millis() - _lastLoopTime;
  if (millisSinceLastRun > _updateInterval) {
    ballx += xDir;
    bally += yDir;

    if (ballx <= _displayRegion.xMin || ballx >= _displayRegion.xMax) { 
      xDir = -xDir; 
    }
    if (bally <= _displayRegion.yMin || bally >= _displayRegion.yMax) {
      yDir = -yDir; 
    }

    _display.fill(0, _displayRegion);
    _display.setXY(uint8_t(round(ballx)), uint8_t(round(bally)), _colourGenerator());

    _lastLoopTime = millis();
  }
  return true;
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

bool ClockFace::run()
{
  constexpr uint8_t bufSize = 6;
  char c_buf[bufSize];
  auto times = timeCallbackFunction();
  snprintf(c_buf, bufSize, "%2d:%02d", times.hour12, times.minute);
  _display.showCharacters(String(c_buf), CRGB::White, 0, 1);
  return false;
}

void showTime(PixelDisplay& display, int hour, int minute, CRGB colour)
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
  display.setXY(0, 0, CRGB(255, 0, 0));
  display.update();
  delay(250);

  // Solid Red, Green, Blue
  display.fill(CRGB(255, 0, 0));
  display.update();
  delay(250);
  display.fill(CRGB(0, 255, 0));
  display.update();
  delay(250);
  display.fill(CRGB(0, 0, 255));
  display.update();
  delay(250);

  // Move through XY sequentially
  for (uint8_t y = 0; y < display.getHeight(); y++) {
    for (uint8_t x = 0; x < display.getWidth(); x++) {
    display.fill(0);
    display.setXY(x, y, CRGB(100, 0, 0));
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
    CRGB(0, 0, 255),
    50,
    500,
    true,
    1
  );
  while(!textScrollTest1.run()) {
    display.update();
    display.fill(0);
    delay(1);
  }

  // Scroll full character set
  display.fill(0);
  display.update();
  auto textScrollTest = TextScroller(
    display,
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 !\"#$%&'()*+'-./:;<=>?@",
    CRGB(0, 255, 0),
    50,
    500,
    false,
    1
  );
  while(!textScrollTest.run()) {
    display.update();
    display.fill(0);
    delay(1);
  }
  display.fill(0);
  display.update();
}

void HSVTestPattern::apply(PixelDisplay& display) const
{
  auto& buffer = display.getFilterBuffer();
  for (uint8_t x = 0; x < display.getWidth(); x++) {
    for (uint8_t y = 0; y < display.getHeight(); y++) {
    auto index = display.XYToIndex(x, y);
    buffer[index] = CHSV(x * (255 / display.getWidth()), 255, y * (255 / display.getHeight()));
    }
  }
}

void SolidColour::apply(PixelDisplay& display) const
{
  auto& buffer = display.getFilterBuffer();
  for (std::size_t i = 0; i < buffer.size(); i++) {
    if (buffer[i] == CRGB(0)) { continue; }
    buffer[i] = maintainBrightness ? CRGB(colour).nscale8(buffer[i].getAverageLight()) : colour;
  }
}

void RainbowWave::apply(PixelDisplay& display) const
{
  auto& buffer = display.getFilterBuffer();
  static float wheelPos = 0;
  wheelPos += speed;
  int _width;
  if (width == 0) { 
    _width = display.getWidth(); 
  } else {
    _width = width;
  }

  for (uint8_t x = 0; x < display.getWidth(); x++) {
    for (uint8_t y = 0; y < display.getHeight(); y++) {
      uint8_t hue = uint8_t(round(wheelPos)) + ((direction == Direction::horizontal ? x : y) * 256 / width);
      auto index = display.XYToIndex(x, y);
      if (buffer[index] == CRGB(0)) { continue; }
      buffer[index] = CHSV(hue, 255, maintainBrightness ? buffer[index].getAverageLight() : 255);
    }
  }
}