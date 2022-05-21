#include <Adafruit_NeoPixel.h>
#include "characters.h"
#include "display.h"

#define LED_PIN  26
#define BRIGHTNESS 255

// Params for width and height
const uint8_t kMatrixWidth = 17;
const uint8_t kMatrixHeight = 5;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

PixelDisplay display(pixels, kMatrixWidth, kMatrixHeight, false, false);

unsigned long lastLoopTime = 0;
constexpr unsigned long loopTime = 25;

enum class Direction {
  up,
  down,
  left,
  right
};

uint32_t colourGenerator_randomHSV() { return Adafruit_NeoPixel::ColorHSV(random(0, 65536)); }
uint32_t colorGenerator_cycleHSV() { 
  return Adafruit_NeoPixel::ColorHSV(millis(), 255, 255);
}
uint32_t colourGenerator_black() { return 0; }

bool fillRandomly(PixelDisplay& display, unsigned long fillInterval, uint32_t(*colourGenerator)() = colourGenerator_black)
{
  static unsigned long lastSpawnTime = 0;
  unsigned long timeNow = millis();

  if (timeNow - lastSpawnTime >= fillInterval) {
    if (!display.filled()) {
      bool filledPixel = false;
      while (!filledPixel) {
        uint8_t x = random(0, display.getWidth());
        uint8_t y = random(0, display.getHeight());
        if (display.getXY(x, y) == uint32_t(0)) {
          display.setXY(x, y, colourGenerator());
          filledPixel = true;
          lastSpawnTime = timeNow;
          Serial.print("Spawn: "); Serial.println(millis());
        }
      }
    } else {
      return true;
    }
  }
  return false;
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

  displayDiagnostic(display);


}

void loop()
{

  switch (effectIndex) {
    case 0:
      if (fillRandomly(display, 100, colourGenerator_randomHSV)) {
        effectIndex++;
        display.fill(0);
      }
      break;
    case 1:
      if (fillRandomly(display, 10, colourGenerator_randomHSV)) {
        effectIndex++;
        display.fill(0);
      }
      break;
    case 2:
      if (fillRandomly(display, 10, colourGenerator_randomHSV)) {
        effectIndex++;
        display.fill(0);
      }
    break;
    default:
      effectIndex = 0;
  }


  display.update(); 

  Serial.println(millis());

  // Manage loop timing
  unsigned long loopTime = millis() - lastLoopTime;
  Serial.print("Loop time:" ); Serial.println(loopTime);
  while (millis() - lastLoopTime < loopTime) {}
  lastLoopTime = millis();
}


