# Resistive Touch Screen Library
This is an Arduino library for the Adafruit ILI9341 and 4-wire resistive touch screen. It will detect touches and make a single report at start of each touch. No further detection is reported until the touch is lifted and a new touch begins.

This library will map touches into screen coordinates, taking into account the screen's orientation.

This library was developed for the open-source Griduino project, which is a driving assistant device for a vehicle's dashboard. https://github.com/barry-ha/Griduino

# Programming Interface
This class is related to the "Adafruit / Adafruit_TouchScreen" library.

The public methods are:

* ctor                 - constructor that requires hardware pin assigments
* newScreenTap()       - an edge detector to deliver each touch only once
* setResistanceRange() - configure expected resistance measurements (optional)
* setScreenSize()      - configure screen width and height (optional)
* unit_test()          - subroutine that verifies correct mapping for various screen orientations (optional)

The protected methods are:

* isTouching()       - which Adafruit did not implement
* mapTouchToScreen() - which converts resistance measurements into screen coordinates

# Example Usage
See **touch_demo.ino** for a working example. In essence, a minimum program will contain:

    #include <Resistive_Touch_Screen.h>

    Resistive_Touch_Screen tsn(PIN_XP, PIN_XM, PIN_YP, PIN_YM, XP_XM_OHMS);
    ScreenPoint screen;   // contains screen coord of touch
    const int r = 2;      // radius of small circle

    void setup() {
      tsn.setScreenSize(tft.width(), tft.height());   // recommended
    }

    void loop() {
      if (tsn.newScreenTap(&screen, tft.getRotation())) {     // if there's touchscreen input
        tft.fillCircle(screen.x, screen.y, r, ILI9341_RED);   // then do something
      }
    }

# Coordinate Systems
It's worthwhile to note the coordinate system axes are different for screen drawing and screen touches. This can be the source of some confusion during programming, and this library tries to clarify.

Physically, the resistive touchscreen is a film layer attached onto the top of the TFT display. The (0,0) touch origin is at one corner, while the TFT display is in another corner. When using setOrientation() command, the TFT's origin moves to a new corner but the touchscreen origin remains in the same place.

The touchscreen hardware will report raw measurement values as integers 0..1023 which are proportional to resistance. The measurements are not literally "ohms" but for the sake of discussion this library uses ohms and resistance interchangeably.

## Screen x,y Coordinate System in Landscape

            x=0 px                        x=320 px
      (0,0)   +-----------------------------+ y=0 px
      Origin  |   x-->                      |
              | y                           |
              | |                           |
              | v                           |
              |                             |
              |                             |
              |                             |
              +-----------------------------+ y=240 px

## TouchScreen X,Y Coordinate System in Landscape

            Y~100 ohms                  Y~900 ohms
              +-----------------------------+ X~900 ohms
              |                             |
              |                             |
              |                             |
              | ^                           |
              | |                           |
              | X                           |
    Origin    |   Y-->                      |
    (100,100) +-----------------------------+ X~100 ohms

## Class Names for Coordinates
Although the Adafruit library provides a convenient **class TSPoint** to hold x,y,z values, it can tempt the programmer to use it for everything. It can be the source of program errors if it sometimes holds resistance measurements and sometimes screen coordinates. 

To help make programs self-documenting, we provide:

* **class ScreenPoint** for screen locations
* **class TouchPoint** for resistance measurements

# Tested with:
1. Arduino Feather M4 Express (120 MHz SAMD51)
   * https://www.adafruit.com/product/3857
2. Adafruit 3.2" TFT color LCD display ILI-9341, with 4-wire resistive touchscreen
   * https://www.adafruit.com/product/1743
   * How to:      https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2
   * SPI Wiring:  https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2/spi-wiring-and-test
   * Touchscreen: https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2/resistive-touchscreen

