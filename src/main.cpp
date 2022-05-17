#include <Adafruit_NeoPixel.h>
#include "characters.h"
#include "display.h"

#define LED_PIN  6
#define BRIGHTNESS 255

// Params for width and height
const uint8_t kMatrixWidth = 17;
const uint8_t kMatrixHeight = 5;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

PixelDisplay display(pixels, kMatrixHeight, kMatrixWidth, false, false);


uint8_t hue = 0;
unsigned long lastChangeTime = 0;
unsigned long timeBetweenChanges = 1000;
int currentChar = 0;

void setup() {
  pixels.begin();
  display.fill(0);
  display.update();

  delay(500);

  display.fill(Adafruit_NeoPixel::Color(100, 0, 0));
  display.update();
  delay(500);

  display.fill(Adafruit_NeoPixel::Color(0, 100, 0));
  display.update();
  delay(500);

  display.fill(Adafruit_NeoPixel::Color(0, 0, 100));
  display.update();
  delay(500);

  display.fill(Adafruit_NeoPixel::Color(25, 25, 25));
  display.update();
  delay(500);

  display.fill(Adafruit_NeoPixel::Color(0, 0, 0, 25));
  display.update();
  delay(500);


  for (uint8_t y = 0; y < display.getHeight(); y++) {
    for (uint8_t x = 0; x < display.getWidth(); x++) {
      display.fill(0);
      display.setXY(x, y, Adafruit_NeoPixel::Color(100, 0, 0));
      display.update();
      delay(100);
    }
  }



  
  

  // for (int i = 0; i < NUM_LEDS; i++) {
  //   pixels.fill(0, 0, NUM_LEDS);
  //   pixels.setPixelColor(i, pixels.Color(0, 255, 0));
  //   pixels.show();
  //   delay(100);
  // }
}


void loop()
{
  //FastLED.setBrightness(BRIGHTNESS);
  
  if (millis() - lastChangeTime > timeBetweenChanges) {
    currentChar++;
    if (currentChar >= charCount) {
      currentChar = 0;
    }
    lastChangeTime = millis();
  }

  pixels.fill(0, 0, NUM_LEDS);
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 5; y++) {
      if (bitRead(characterFontArray[currentChar][y], 2-x) == 1) {
          //leds[XY(x, y)] = CRGB::Red;//CHSV(hue, 255, 255);
        //pixels.setPixelColor(XY(x, y), pixels.Color(100, 0, 0));
      }
    }
  }

  pixels.show();
  delay(100);

  //FastLED.show();
  //FastLED.delay(1);

  hue++;
  

  
  
}