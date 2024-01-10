// Please format this file with clang before check-in to GitHub
/*
  File:     basic_interface.ino

  Purpose:  Illustrate constructing the object and calling its methods.
            This example invokes the class and confirms it compiles successfully.
            This is not intended to be functional or useful.
            Public domain.
*/

#include <Resistive_Touch_Screen.h>   // https://github.com/barry-ha/Resistive_Touch_Screen

// ---------- Touch Screen pins, depends on wiring from CPU to Touch Screen
#define PIN_XP A3   // Touchscreen X+ can be a digital pin
#define PIN_XM A4   // Touchscreen X- must be an analog pin, use "An" notation
#define PIN_YP A5   // Touchscreen Y+ must be an analog pin, use "An" notation
#define PIN_YM 9    // Touchscreen Y- can be a digital pin

// ---------- Touch Screen configuration
#define XP_XM_OHMS 310   // Resistance in ohms between X+ and X- to calibrate touch pressure
                         // measure this with an ohmmeter while device is turned off
#define X_MIN_OHMS 100   // Default: Expected range on touchscreen's X-axis readings
#define X_MAX_OHMS 900
#define Y_MIN_OHMS 100   // Default: Expected range on touchscreen's Y-axis readings
#define Y_MAX_OHMS 900

#define START_TOUCH_PRESSURE 200   // Minimum pressure threshold considered start of "press"
#define END_TOUCH_PRESSURE   50    // Maximum pressure threshold required before end of "press"

void setup() {
  // ----- example constructors
  Resistive_Touch_Screen tsn(PIN_XP, PIN_XM, PIN_YP, PIN_YM, XP_XM_OHMS);

  PressPoint press1;
  PressPoint press2(1, 2, 3);   // resistance

  ScreenPoint loc1;
  ScreenPoint loc2(4, 5, 6);   // screen coordinates

  // ----- init touchscreen
  tsn.setScreenSize(320, 240);                                                          // required
  tsn.setResistanceRange(X_MIN_OHMS, X_MAX_OHMS, Y_MIN_OHMS, Y_MAX_OHMS, XP_XM_OHMS);   // optional, for overriding defaults
  tsn.setThreshhold(START_TOUCH_PRESSURE, END_TOUCH_PRESSURE);                          // optional, for overriding defaults
  tsn.unit_test();                                                                      // optional, for debug

  // uint16_t p = tsn.pressure();     // protected function is not available in public interface
  // int xTouch = tsn.readTouchX();   // protected function is not available in public interface
  // int yTouch = tsn.readTouchY();   // protected function is not available in public interface

  // ----- retrieve touch on screen
  ScreenPoint screenCoord;
  bool bb = tsn.newScreenTap(&screenCoord, 0);
}

void loop() {
}