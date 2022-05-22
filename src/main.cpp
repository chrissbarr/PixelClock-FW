#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>

#include "characters.h"
#include "display.h"

// LED Panel Configuration
constexpr int16_t matrixLEDPin = 26;
constexpr uint8_t matrixWidth = 17;
constexpr uint8_t matrixHeight = 5;
constexpr uint8_t matrixSize = matrixWidth * matrixHeight;

Adafruit_NeoPixel pixels(matrixSize, matrixLEDPin, NEO_GRBW + NEO_KHZ800);

PixelDisplay display(pixels, matrixWidth, matrixHeight, false, false);

unsigned long lastLoopTime = 0;
constexpr unsigned long loopTime = 25;

enum class Direction {
  up,
  down,
  left,
  right
};

uint32_t colourGenerator_randomHSV() { return Adafruit_NeoPixel::ColorHSV(random(0, 65536)); }
uint32_t colorGenerator_cycleHSV() { return Adafruit_NeoPixel::ColorHSV(millis(), 255, 255); }
uint32_t colourGenerator_black() { return 0; }

bool fillRandomly(PixelDisplay& display, uint32_t fillInterval, uint32_t(*colourGenerator)(), const DisplayRegion& spawnRegion)
{
  static uint32_t lastSpawnTime = 0;
  uint32_t timeNow = millis();

  if (timeNow - lastSpawnTime >= fillInterval) {
    if (!display.filled(0, spawnRegion)) {
      bool filledPixel = false;
      while (!filledPixel) {
        uint8_t x = random(spawnRegion.xMin, spawnRegion.xMax + 1);
        uint8_t y = random(spawnRegion.yMin, spawnRegion.yMax + 1);
        //Serial.print("X: "); Serial.print(x); Serial.print(" Y: "); Serial.println(y);
        if (display.getXY(x, y) == uint32_t(0)) {
          display.setXY(x, y, colourGenerator());
          filledPixel = true;
          lastSpawnTime = timeNow;
        }
      }
    } else {
      return true;
    }
  }
  return false;
}

bool fillRandomly(PixelDisplay& display, uint32_t fillInterval, uint32_t(*colourGenerator)())
{
  return fillRandomly(display, fillInterval, colourGenerator, display.getFullDisplayRegion());
}

bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)(), DisplayRegion displayRegion)
{
  static uint32_t lastMoveTime = 0;
  uint32_t timeNow = millis();

  DisplayRegion spawnZone;
  spawnZone.xMin = displayRegion.xMin;
  spawnZone.xMax = displayRegion.xMax;
  spawnZone.yMin = displayRegion.yMin;
  spawnZone.yMax = displayRegion.yMin;

  if (fillInterval != 0) {
    fillRandomly(display, fillInterval, colourGenerator, spawnZone);
  }

  if (moveInterval != 0) {
    // Move all pixels down
    if (timeNow - lastMoveTime > moveInterval) {
      for (int y = displayRegion.yMax; y >= displayRegion.yMin; y--) {
        for (uint8_t x = displayRegion.xMin; x <= displayRegion.xMax; x++) {
          uint32_t cellColour = display.getXY(x, y);
          if (cellColour != 0) {
            // if this is the last row
            if (y == displayRegion.yMax) {
              if (empty) {
                display.setXY(x, y, 0);
              }
              continue;
            }
            if (display.getXY(x, y + 1) == uint32_t(0)) {
              display.setXY(x, y + 1, cellColour);
              display.setXY(x, y, 0);
            }
          }
        }
      }
      lastMoveTime = timeNow;
    }  
  }
  return !display.filled(0, displayRegion);
}

bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)())
{
  return gravityFill(display, fillInterval, moveInterval, empty, colourGenerator, display.getFullDisplayRegion());
}

void tetris(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval)
{
  gravityFill(display, fillInterval, moveInterval, false, [](){
    const uint32_t choices[] = {
      Adafruit_NeoPixel::Color(255, 0, 0),
      Adafruit_NeoPixel::Color(0, 255, 0),
      Adafruit_NeoPixel::Color(0, 0, 255),
    };
    return choices[random(sizeof(choices)/sizeof(choices[0]))];
  });

  bool setFound = false;
  uint8_t setSize = 3;
  uint8_t setXCoords[3];
  uint8_t setYCoords[3];

  auto checkMatch = [&](uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3) {
    uint32_t val1 = display.getXY(x1, y1);
    if (val1 == 0) { return; }
    uint32_t val2 = display.getXY(x2, y2);
    uint32_t val3 = display.getXY(x3, y3);

    if (val1 == val2 && val2 == val3) { 
      setFound = true;
      setXCoords[0] = x1;
      setXCoords[1] = x2;
      setXCoords[2] = x3;
      setYCoords[0] = y1;
      setYCoords[1] = y2;
      setYCoords[2] = y3;
    }
  };

  for (uint8_t x = 0; x < display.getWidth(); x++) {
    for (uint8_t y = 0; y < display.getHeight(); y++) {
      if (x > 0 && x < display.getWidth() - 1) {
        checkMatch(x - 1, y, x, y, x + 1, y);
      }
      if (y > 0 && y < display.getHeight() - 1) {
        checkMatch(x, y - 1, x, y, x, y + 1);
      }
    }
  }

  if (setFound) {
    for (int j = 0; j < setSize; j++) {
      display.setXY(setXCoords[j], setYCoords[j], 0);
    }
  }
}

void showTime(PixelDisplay& display)
{
  uint32_t colour = colorGenerator_cycleHSV();
  time_t t = now();
  constexpr uint8_t bufSize = 3;
  char c_buf[bufSize];

  // hour
  int hr = hourFormat12(t);
  snprintf(c_buf, bufSize, "%2d", hr);
  display.showCharacters(String(c_buf), colour, 0, 1);

  // colon
  display.showCharacter(':', colour, 7);

  // minute
  int min = minute(t);
  snprintf(c_buf, bufSize, "%02d", min);
  display.showCharacters(c_buf, colour, 10, 1);
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
    "Hello - Testing",
    500,
    true,
    1
  );
  while(!textScrollTest1.update(colorGenerator_cycleHSV(), 50)) {
    display.update();
    display.fill(0);
  }
  

  // Scroll full character set
  display.fill(0);
  display.update();
  auto textScrollTest = TextScroller(
    display,
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 !\"#$%&'()*+'-./:;<=>?@",
    500,
    false,
    1
  );
  while(!textScrollTest.update(Adafruit_NeoPixel::Color(0, 25, 0), 10)) {
    display.update();
    display.fill(0);
  }
  display.fill(0);
  display.update();
}

int effectIndex = 0;

void setup() {
  Serial.begin(250000);
  Serial.println("Serial begin!");
  pixels.begin();
  pixels.setBrightness(100);

  display.fill(0);
  display.update();
  delay(500);

  setTime(12,30,0,1,1,2022);

  //displayDiagnostic(display);


}

void loop()
{

  // DisplayRegion randRegion;
  // randRegion.xMin = 10;
  // randRegion.xMax = 16;
  // randRegion.yMin = 0;
  // randRegion.yMax = 4;

  // switch (effectIndex) {
  //   case 0:
  //     if (fillRandomly(display, 100, colourGenerator_randomHSV, randRegion)) {
  //       effectIndex++;
  //       display.fill(0, randRegion);
  //     }
  //     break;
  //   case 1:
  //     if (fillRandomly(display, 10, colourGenerator_randomHSV, randRegion)) {
  //       effectIndex++;
  //       display.fill(0, randRegion);
  //     }
  //     break;
  //   case 2:
  //     if (fillRandomly(display, 10, colourGenerator_randomHSV, randRegion)) {
  //       effectIndex++;
  //       display.fill(0, randRegion);
  //     }
  //   break;
  //   default:
  //     effectIndex = 0;
  // }

  // static int state = 0;
  // switch(state) {
  //   case 0:
  //     gravityFill(display, 100, 100, false, colourGenerator_randomHSV);
  //     if (display.filled(0)) {
  //       state = 1;
  //     }
  //     break;
  //   case 1:
  //     gravityFill(display, 0, 100, true, colourGenerator_randomHSV);
  //     if (display.empty()) {
  //       state = 0;
  //     }
  //     break;
  // }

  //tetris(display, 100, 100);
  //if (display.filled()) { display.fill(0); }


  display.fill(0);
  showTime(display);
  display.update(); 

  // Serial.println(millis());

  // Manage loop timing
  unsigned long loopTime = millis() - lastLoopTime;
  //Serial.print("Loop time:" ); Serial.println(loopTime);
  while (millis() - lastLoopTime < loopTime) {}
  lastLoopTime = millis();
}


