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

PixelDisplay display(pixels, kMatrixWidth, kMatrixHeight, false, false);


uint8_t hue = 0;
unsigned long lastChangeTime = 0;
unsigned long timeBetweenChanges = 1000;
int currentChar = 0;

void setup() {
  pixels.begin();
  pixels.setBrightness(255);
  display.fill(0);
  display.update();

  delay(500);

  display.fill(Adafruit_NeoPixel::Color(100, 0, 0));
  display.update();
  delay(250);

  display.fill(Adafruit_NeoPixel::Color(0, 100, 0));
  display.update();
  delay(250);

  display.fill(Adafruit_NeoPixel::Color(0, 0, 100));
  display.update();
  delay(250);

  display.fill(Adafruit_NeoPixel::Color(25, 25, 25));
  display.update();
  delay(250);

  display.fill(Adafruit_NeoPixel::Color(0, 0, 0, 25));
  display.update();
  delay(250);

  display.fill(0);
  display.setXY(0, 0, Adafruit_NeoPixel::Color(0, 0, 100));
  display.setXY(1, 0, Adafruit_NeoPixel::Color(0, 0, 100));
  display.setXY(2, 0, Adafruit_NeoPixel::Color(0, 0, 100));
  display.setXY(1, 1, Adafruit_NeoPixel::Color(0, 0, 100));
  display.setXY(1, 2, Adafruit_NeoPixel::Color(0, 0, 100));
  display.update();
  delay(250);

  for (uint8_t y = 0; y < display.getHeight(); y++) {
    for (uint8_t x = 0; x < display.getWidth(); x++) {
      display.fill(0);
      display.setXY(x, y, Adafruit_NeoPixel::Color(100, 0, 0));
      display.update();
      delay(1);
    }
  }

  display.fill(0);
  display.showCharacter('A', Adafruit_NeoPixel::Color(100, 0, 0), 0);
  display.showCharacter('B', Adafruit_NeoPixel::Color(100, 0, 0), 3);
  display.showCharacter('C', Adafruit_NeoPixel::Color(100, 0, 0), 6);
  display.update();

  for (int i = 0; i < 20; i++) {
    display.fill(0);
    display.showCharacter('D', Adafruit_NeoPixel::Color(0, 25, 0), i);
    display.update();
    delay(100);
  }

  for (uint32_t i = 0; i < 65536; i += 1) {
    uint32_t colour = pixels.ColorHSV(i, 255, 255);
    display.fill(0);
    display.showCharacter('1', colour, 0);
    display.showCharacter('2', colour, 4);
    display.showCharacter(':', colour, 7);
    display.showCharacter('3', colour, 10);
    display.showCharacter('4', colour, 14);
    display.update();
    //delay(1);
  }
}

void loop()
{
  for (int i = 0; i < 300; i++) {
    display.fill(0);
    display.showCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 !\"#$%&'()*+'-./:;<=>?@", Adafruit_NeoPixel::Color(0, 25, 0), -i, 1);
    display.update();
    delay(250);
  }
}