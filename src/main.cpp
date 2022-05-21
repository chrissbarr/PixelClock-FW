#include <Adafruit_NeoPixel.h>
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

  Serial.println("fillRandomly");

  if (timeNow - lastSpawnTime >= fillInterval) {
    Serial.println("timeNow");
    if (!display.filled(0, spawnRegion)) {
      Serial.println("NotFilled!");
      bool filledPixel = false;
      while (!filledPixel) {
        uint8_t x = random(spawnRegion.xMin, spawnRegion.xMax + 1);
        uint8_t y = random(spawnRegion.yMin, spawnRegion.yMax + 1);
        Serial.print("X: "); Serial.print(x); Serial.print(" Y: "); Serial.println(y);
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

bool gravityFill(PixelDisplay& display, uint32_t fillInterval, uint32_t moveInterval, bool empty, uint32_t(*colourGenerator)() = colourGenerator_black)
{
  static uint32_t lastMoveTime = 0;
  uint32_t timeNow = millis();

  DisplayRegion spawnZone;
  spawnZone.xMin = 0;
  spawnZone.xMax = display.getWidth() - 1;
  spawnZone.yMin = 0;
  spawnZone.yMax = 0;

  Serial.println("gravityFill");
  if (fillInterval != 0) {
    fillRandomly(display, fillInterval, colourGenerator, spawnZone);
  }

  if (moveInterval != 0) {
    // Move all pixels down
    if (timeNow - lastMoveTime > moveInterval) {
      for (int y = display.getHeight() - 1; y >= 0; y--) {
        for (uint8_t x = 0; x < display.getWidth(); x++) {
          uint32_t cellColour = display.getXY(x, y);
          if (cellColour != 0) {
            // if this is the last row
            if (y == display.getHeight() - 1) {
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

  return !display.filled(0);
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
  pixels.setBrightness(50);

  display.fill(0);
  display.update();
  delay(500);

  //displayDiagnostic(display);


}

void loop()
{

  // switch (effectIndex) {
  //   case 0:
  //     if (fillRandomly(display, 100, colourGenerator_randomHSV)) {
  //       effectIndex++;
  //       display.fill(0);
  //     }
  //     break;
  //   case 1:
  //     if (fillRandomly(display, 10, colourGenerator_randomHSV)) {
  //       effectIndex++;
  //       display.fill(0);
  //     }
  //     break;
  //   case 2:
  //     if (fillRandomly(display, 10, colourGenerator_randomHSV)) {
  //       effectIndex++;
  //       display.fill(0);
  //     }
  //   break;
  //   default:
  //     effectIndex = 0;
  // }

  static int state = 0;

  switch(state) {
    case 0:
      gravityFill(display, 100, 100, false, colourGenerator_randomHSV);
      if (display.filled()) {
        state = 1;
      }
      break;
    case 1:
      gravityFill(display, 0, 100, true, colourGenerator_randomHSV);
      if (display.empty()) {
        state = 0;
      }
      break;
  }


  


  display.update(); 

  // Serial.println(millis());

  // Manage loop timing
  unsigned long loopTime = millis() - lastLoopTime;
  Serial.print("Loop time:" ); Serial.println(loopTime);
  while (millis() - lastLoopTime < loopTime) {}
  lastLoopTime = millis();
}


