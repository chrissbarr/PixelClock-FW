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

enum class Direction {
  up,
  down,
  left,
  right
};

uint32_t colourGenerator_randomHSV() { return Adafruit_NeoPixel::ColorHSV(random(0, 65536)); }
uint32_t colourGenerator_black() { return 0; }

void gravityFill(Direction fallDir, int rate, bool empty, uint32_t(*colourGenerator)() = colourGenerator_black)
{
  uint8_t chance = random(rate);
  if (chance == 0) {
    uint8_t x = random(0, display.getWidth());
    uint8_t y = 0;//random(0, display.getWidth());
    if (display.getXY(x, y) == uint32_t(0)) {
      display.setXY(x, y, colourGenerator());
    }
  }
  
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
}

void tetris()
{
  while (!display.filled()) {
    gravityFill(Direction::down, 1, false, []() {
      int r = random(6);
      switch (r) {
        case 0:
          return Adafruit_NeoPixel::Color(255, 0, 0);
          break;
        case 1:
          return Adafruit_NeoPixel::Color(0, 255, 0);
          break;
        case 2:
          return Adafruit_NeoPixel::Color(0, 0, 255);
          break;
        case 3:
          return Adafruit_NeoPixel::Color(127, 127, 0);
          break;
        case 4:
          return Adafruit_NeoPixel::Color(0, 127, 127);
          break;
        case 5:
          return Adafruit_NeoPixel::Color(127, 0, 127);
          break;
        default:
          return Adafruit_NeoPixel::Color(255, 255, 255);
          break;
      }
    });
    delay(50);
    display.update();

    bool setFound = false;
    uint8_t setSize = 3;
    uint8_t setXCoords[3];
    uint8_t setYCoords[3];

    for (uint8_t y = 0; y < display.getHeight(); y++) {
      for (uint8_t x = 1; x < display.getWidth() - 1; x++) {
        if (display.getXY(x, y) != 0) {
          if (display.getXY(x - 1, y) == display.getXY(x, y) && display.getXY(x, y) == display.getXY(x + 1, y)) {
            setFound = true;
            setXCoords[0] = x - 1;
            setXCoords[1] = x;
            setXCoords[2] = x + 1;
            setYCoords[0] = y;
            setYCoords[1] = y;
            setYCoords[2] = y;
          }
        }
      }
    }

    for (uint8_t x = 0; x < display.getWidth(); x++) {
      for (uint8_t y = 1; y < display.getHeight() - 1; y++) {
        if (display.getXY(x, y) != 0) {
          if (display.getXY(x, y - 1) == display.getXY(x, y) && display.getXY(x, y) == display.getXY(x, y + 1)) {
            setFound = true;
            setXCoords[0] = x;
            setXCoords[1] = x;
            setXCoords[2] = x;
            setYCoords[0] = y - 1;
            setYCoords[1] = y;
            setYCoords[2] = y + 1;
          }
        }
      }
    }

    if (setFound) {
      uint32_t original = display.getXY(setXCoords[0], setYCoords[0]);
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < setSize; j++) {
          display.setXY(setXCoords[j], setYCoords[j], original);
        }
        display.update();
        delay(250);
        for (int j = 0; j < setSize; j++) {
          display.setXY(setXCoords[j], setYCoords[j], 0);
        }
        display.update();
        delay(250);
      }
    }
  }
  delay(1000);

  while(!display.empty()) {
    gravityFill(Direction::down, 0, true);
    delay(250);
    display.update();
  }
  
  display.fill(0);
  display.update();
}

void setup() {
  pixels.begin();
  pixels.setBrightness(50);
  display.fill(0);
  display.update();

  delay(500);

  tetris();

  // for (int i = 0; i < 10; i++) {
  //   while (!display.filled()) {
  //     gravityFill(Direction::down, 1, false, colourGenerator_randomHSV);
  //     delay(50);
  //     display.update();
  //   }
  //   delay(1000);
    
  //   uint32_t colour = 0;//pixels.Color(0, 0, 0, 255);
  //   display.showCharacter('1', colour, 0);
  //   display.showCharacter('2', colour, 4);
  //   display.showCharacter(':', colour, 7);
  //   display.showCharacter('3', colour, 10);
  //   display.showCharacter('4', colour, 14);

  //   display.update();
  //   delay(1000);

  //   while(!display.empty()) {
  //     gravityFill(Direction::down, 100000, true);
  //     delay(500);
  //     display.update();
  //   }
  //   delay(500);
  // }
  



  // for (int i = 0; i < 10; i++) {
  //   while(!display.filled()) {
  //     bool filledPixel = false;
  //     while (!filledPixel) {
  //       uint8_t x = random(0, display.getWidth());
  //       uint8_t y = random(0, display.getWidth());
  //       if (display.getXY(x, y) == uint32_t(0)) {
  //         display.setXY(x, y, pixels.ColorHSV(random(0, 655236)));
  //         filledPixel = true;
  //       }
  //     }
  //     display.update();
  //   }
  //   display.fill(0);
  //   display.update();
  //   delay(100);
  // }




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



  for (uint32_t i = 0; i < 65536; i += 100) {
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