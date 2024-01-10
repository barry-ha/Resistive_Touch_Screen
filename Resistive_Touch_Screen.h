#pragma once   // Please format this file with clang before check-in to GitHub
/*
  File:
    Resistive_Touch_Screen.h

# Resistive Touch Screen Library
    This is an Arduino library for the Adafruit ILI9341 and 4-wire resistive touch screen.
    It will detect touches and make a single report at start of each touch.
    No further detection is reported until the touch is lifted and a new touch begins.

    This library will map touches into screen coordinates, taking into account the screen's orientation.

    This library was developed for the open-source Griduino project, which is a
    driving assistant device for a vehicle's dashboard. https://github.com/barry-ha/Griduino

# Programming Interface
    This class is related to the "Adafruit / Adafruit_TouchScreen" library.

    The public methods are:
    * ctor                 - constructor that requires hardware pin assignments
    * newScreenTap()       - an edge detector to deliver each touch only once
    * setResistanceRange() - configure expected resistance measurements (optional)
    * setScreenSize()      - configure screen width and height (optional)
    * unit_test()          - subroutine that verifies correct mapping for various screen orientations (optional)

    The protected methods are:
    * isTouching()       - which Adafruit did not implement
    * mapTouchToScreen() - which converts resistance measurements into screen coordinates

# Example Usage
    See **touch_demo.ino** for a working example.
    In essence, a minimum program will contain:

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
    It's worthwhile to note the coordinate system axes are different for
    screen drawing and screen touches. This can be the source of some confusion
    during programming, and this library tries to clarify.

    The touchscreen hardware will report raw measurement values as integers 0..1023
    which are proportional to resistance. The values are not literally "ohms" but
    for the sake of discussion this library uses ohms and resistance interchangeably.

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
    Although the Adafruit library provides a convenient **class TSPoint** to hold x,y,z values,
    it can tempt the programmer to use it for everything. It can be the source of program errors
    if it sometimes holds resistance measurements and sometimes screen coordinates.

    To help make programs self-documenting, we provide:

    class ScreenPoint - for screen locations
    class TouchPoint  - for resistance measurements

# Tested with:
    1. Arduino Feather M4 Express (120 MHz SAMD51)
       * https://www.adafruit.com/product/3857

    2. Adafruit 3.2" TFT color LCD display ILI-9341, with 4-wire resistive touchscreen
       * https://www.adafruit.com/product/1743
       * How to:      https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2
       * SPI Wiring:  https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2/spi-wiring-and-test
       * Touchscreen: https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2/resistive-touchscreen

# License
    GNU General Public License v3.0

    Permissions of this strong copyleft license are conditioned on making
    available complete source code of licensed works and modifications,
    which include larger works using a licensed work, under the same license.
    Copyright and license notices must be preserved. Contributors provide
    an express grant of patent rights.
*/
#include <Arduino.h>       // built-in
#include <TouchScreen.h>   // https://github.com/adafruit/Adafruit_TouchScreen

/*
 * PressPoint encapsulates the X,Y, and Z/pressure measurements for a touch.
 * Use "class PressPoint" to document that values are resistance measurements.
 */
class PressPoint : public TSPoint {
public:
  PressPoint(void);
  PressPoint(int16_t x, int16_t y, int16_t z);
};

/*
 * ScreenPoint contains X,Y screen coordinates and the Z/pressure for a touch event.
 * This derives from and is interchangeable with Adafruit's "TSPoint".
 * Use "class ScreenPoint" to document that values are a pixel location on the TFT screen.
 */
class ScreenPoint : public TSPoint {
public:
  ScreenPoint(void);
  ScreenPoint(int16_t x, int16_t y, int16_t z);
};

// ========== Class Resistive_Touch_Screen ==========
class Resistive_Touch_Screen {
public:
  /**
   * @brief Construct a new Resistive Touch Screen object
   *
   * @param x_plus_pin X+ pin.  Must be an analog pin
   * @param y_plus_pin Y+ pin.  Must be an analog pin
   * @param x_minus_pin X- pin. Can be a digital pin
   * @param y_minus_pin Y- pin. Can be a digital pin
   * @param rx  The resistance in ohms between X+ and X- to calibrate pressure sensing
   */
  // clang-format off
  Resistive_Touch_Screen(uint8_t x_plus_pin, uint8_t y_plus_pin, uint8_t x_minus_pin, uint8_t y_minus_pin, uint16_t rx)
    : _x_plus_pin(x_plus_pin)
    , _y_plus_pin(y_plus_pin)
    , _x_minus_pin(x_minus_pin)
    , _y_minus_pin(y_minus_pin)
    , _rx(rx) {}
  // clang-format on

  /**
   * @brief Find leading edge of a screen touch, and returns TRUE only once on initial screen press
   * @brief If true, also return screen coordinates of the touch
   * @brief and then returns FALSE until pressure is released and screen is touched again.
   */
  // orientation = 1 or 3 = ILI9341 screen rotation setting in landscape only
  bool newScreenTap(ScreenPoint *pScreenCoord, uint16_t orientation);

  // getters and setters
  void setResistanceRange(uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max, uint16_t xp_xm) {
    _x_min_ohms = x_min;
    _x_max_ohms = x_max;
    _y_min_ohms = y_min;
    _y_max_ohms = y_max;
    _rx         = xp_xm;   // typ. 310 ohms
  }
  void setScreenSize(uint16_t x_max, uint16_t y_max) {
    _width  = x_max;
    _height = y_max;
  }
  void setThreshhold(uint16_t start_ohms, uint16_t stop_ohms) {
    _start_touch_pressure = start_ohms;
    _stop_touch_pressure  = stop_ohms;
  }
  void unit_test();

protected:
  bool isTouching(void);
  uint16_t pressure(void);
  void mapTouchToScreen(PressPoint touchOhms, ScreenPoint *screenCoord, int orientation);
  int readTouchX(void);
  int readTouchY(void);
  void validateTouch(PressPoint p, ScreenPoint expected, uint16_t o);   // for unit tests

private:
  uint8_t _x_plus_pin, _y_plus_pin, _x_minus_pin, _y_minus_pin, _rx;

  uint16_t _width  = 320;   // Default: screen pixels
  uint16_t _height = 240;

  uint16_t _x_min_ohms = 100;   // Default: Expected range on touchscreen's X-axis readings
  uint16_t _x_max_ohms = 900;
  uint16_t _y_min_ohms = 100;   // Default: Expected range on touchscreen's Y-axis readings
  uint16_t _y_max_ohms = 900;

  uint16_t _start_touch_pressure = 200;   // minimum threshold to detect start of touch
  uint16_t _stop_touch_pressure  = 50;    // maximum threshold to detect end of touch

  // ------- TFT 4-Wire Resistive Touch Screen configuration parameters
  // For touch point precision, we need to know the resistance
  // between X+ and X- Use any multimeter to read it.
  // Here's a default value for starters, averaged from several https://www.adafruit.com/product/1743
  /*
  #define XP_XM_OHMS 310   // Resistance in ohms between X+ and X- to calibrate touch pressure
                            // measure this with an ohmmeter while device is turned off

  #define START_TOUCH_PRESSURE 200   // Minimum pressure threshold considered start of "press"
  #define END_TOUCH_PRESSURE   50    // Maximum pressure threshold required before end of "press"

  #define X_MIN_OHMS 100   // Default: Expected range on touchscreen's X-axis readings
  #define X_MAX_OHMS 900
  #define Y_MIN_OHMS 100   // Default: Expected range on touchscreen's Y-axis readings
  #define Y_MAX_OHMS 900
  */
};
