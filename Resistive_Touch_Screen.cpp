// Please format this file with clang before check-in to GitHub
/*
  File:     Resistive_Touch_Screen.h

  Purpose:  Arduino library for the Adafruit ILI9341 and 4-wire resistive touch screen.
            It provides "newScreenTap()" which is an edge detector to deliver each touch only once.

  License:  GNU General Public License v3.0

            Permissions of this strong copyleft license are conditioned on making
            available complete source code of licensed works and modifications,
            which include larger works using a licensed work, under the same license.
            Copyright and license notices must be preserved. Contributors provide
            an express grant of patent rights.
*/

#include <Resistive_Touch_Screen.h>

/*
 * Default constructor PressPoint and ScreenPoint objects
 */
PressPoint::PressPoint(void) { x = y = z = 0; }
ScreenPoint::ScreenPoint(void) { x = y = z = 0; }

/**
 * @brief Construct a new TSPoint::TSPoint object
 *
 * @param x0 The point's X value
 * @param y0 The point's Y value
 * @param z0 The point's Z value
 */
PressPoint::PressPoint(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}
ScreenPoint::ScreenPoint(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}

// "isTouching()" is defined in Adafruit's touch.h but is not implemented Adafruit's TouchScreen library
// Note - For Griduino, if this function takes longer than 8 msec it can cause erratic GPS readings
// so we recommend against using https://forum.arduino.cc/index.php?topic=449719.0
bool Resistive_Touch_Screen::isTouching(void) {
  static bool button_state = false;
  uint16_t pres_val        = pressure();

  if ((button_state == false) && (pres_val > START_TOUCH_PRESSURE)) {
    button_state = true;
    // Serial.print(". pressed, pressure = ");   // debug
    // Serial.println(pres_val);                 // debug
  }

  if ((button_state == true) && (pres_val < END_TOUCH_PRESSURE)) {
    button_state = false;
    // Serial.print(". released, pressure = ");   // debug
    // Serial.println(pres_val);                  // debug
  }

  return button_state;
}

// find leading edge of a screen touch, nonblocking
// returns TRUE only once on initial screen press
// and then returns FALSE until pressure is released and screen is touched again
// if true, also return screen coordinates of the touch
// orientation = 1 or 3 = ILI9341 screen rotation setting in landscape only
bool Resistive_Touch_Screen::newScreenTap(ScreenPoint *screen, uint16_t orientation) {

  static bool gTouching = false;   // keep track of previous state

  bool result = false;   // assume no touch
  if (gTouching) {
    // the touch was previously processed, so ignore continued pressure until they let go
    if (!isTouching()) {
      // Touching ==> Not Touching transition
      gTouching = false;
    }
  } else {
    // here, we know the screen was not being touched in the last pass,
    // so look for a new touch on this pass
    // Our replacement "isTouching" function has built-in hysteresis to debounce
    if (isTouching()) {
      gTouching = true;
      result    = true;

      // touchscreen point object has (x,y,z) coordinates, where x,y = resistance, and z = pressure
      PressPoint touchOhms;
      touchOhms.x = readTouchX();
      touchOhms.y = readTouchY();
      touchOhms.z = pressure();

      // convert resistance measurements into screen pixel coords
      mapTouchToScreen(touchOhms, screen, orientation);
    } else {
      // do nothing - wait for next start of touch
    }
  }
  // delay(10);   // no delay: code above completely handles debouncing without blocking the loop
  return result;
}

/**
 * @brief Convert from X+,Y+ resistance measurements to screen coordinates
 * @param touchOhms = resistance readings from touchscreen
 * @param screenCoord = result of converting touchOhms into screen coordinates
 **/
//
void Resistive_Touch_Screen::mapTouchToScreen(PressPoint touchOhms, ScreenPoint *screenCoord, int orientation) {
  //
  // Some measured readings in landscape orientation were:
  //   +---------------------+ X=876
  //   |                     |
  //   |                     |
  //   |                     |
  //   +---------------------+ X=160
  //  Y=110                Y=892
  //
  // Typical measured pressures=200..600

  switch (orientation) {
  case 1:   // LANDSCAPE
    // setRotation(1) = landscape orientation = x-,y-axis exchanged
    //               map(value        in_min,in_max,          out_min,out_max)
    screenCoord->x = map(touchOhms.y, Y_MIN_OHMS, Y_MAX_OHMS, 0, _width);
    screenCoord->y = map(touchOhms.x, X_MAX_OHMS, X_MIN_OHMS, 0, _height);
    screenCoord->z = touchOhms.z;
    break;

  case 3:   // FLIPPED_LANDSCAPE
    // setRotation(3) = upside down landscape orientation = x-,y-axis exchanged
    //               map(value        in_min,in_max,          out_min,out_max)
    screenCoord->x = map(touchOhms.y, X_MAX_OHMS, X_MIN_OHMS, 0, _width);
    screenCoord->y = map(touchOhms.x, Y_MIN_OHMS, Y_MAX_OHMS, 0, _height);
    screenCoord->z = touchOhms.z;
    break;

  default:
    Serial.println("Portrait orientation is not implemented.");
    screenCoord->x = random(0, _width);
    screenCoord->y = random(0, _height);
    screenCoord->z = touchOhms.z;
    break;
  }

  // debug
  /*
  char temp[128];
  snprintf(temp, sizeof(temp),   // report resistance measurements
           ". PressPoint (x,y,z) = (%d, %d, %d) ohms",
           touchOhms.x, touchOhms.y, touchOhms.z);
  Serial.println(temp);
  snprintf(temp, sizeof(temp),   // report screen coordinates
           ". screenCoord (x,y,z) = (%d, %d, %d) pixels",
           screenCoord->x, screenCoord->y, screenCoord->z);
  Serial.println(temp);
  /* */

  // keep all touches within boundaries of the screen coordinates
  screenCoord->x = constrain(screenCoord->x, 0, _width);
  screenCoord->y = constrain(screenCoord->y, 0, _height);

  return;
}

/**
 * (Copied from Adafruit_Touchscreen)
 *
 * @brief Read the touch event's X value
 *
 * @return int the X measurement
 */
int Resistive_Touch_Screen::readTouchX(void) {
  pinMode(_y_plus_pin, INPUT);
  pinMode(_y_minus_pin, INPUT);
  digitalWrite(_y_plus_pin, LOW);
  digitalWrite(_y_minus_pin, LOW);

  pinMode(_x_plus_pin, OUTPUT);
  digitalWrite(_x_plus_pin, HIGH);
  pinMode(_x_minus_pin, OUTPUT);
  digitalWrite(_x_minus_pin, LOW);

  return (1023 - analogRead(_y_plus_pin));
}

/**
 * (Copied from Adafruit_Touchscreen)
 *
 * @brief Read the touch event's Y value
 *
 * @return int the Y measurement
 */
int Resistive_Touch_Screen::readTouchY(void) {
  pinMode(_x_plus_pin, INPUT);
  pinMode(_x_minus_pin, INPUT);
  digitalWrite(_x_plus_pin, LOW);
  digitalWrite(_x_minus_pin, LOW);

  pinMode(_y_plus_pin, OUTPUT);
  digitalWrite(_y_plus_pin, HIGH);
  pinMode(_y_minus_pin, OUTPUT);
  digitalWrite(_y_minus_pin, LOW);

  return (1023 - analogRead(_x_minus_pin));
}

// 2020-05-03 CraigV and barry@k7bwh.com
/**
 * @brief Read the touch event's Z/pressure value
 *
 * @return int the Z measurement
 */
uint16_t Resistive_Touch_Screen::pressure(void) {
  // Set X+ to ground
  pinMode(_x_plus_pin, OUTPUT);
  digitalWrite(_x_plus_pin, LOW);

  // Set Y- to VCC
  pinMode(_y_minus_pin, OUTPUT);
  digitalWrite(_y_minus_pin, HIGH);

  // Hi-Z X- and Y+
  digitalWrite(_x_minus_pin, LOW);
  pinMode(_x_minus_pin, INPUT);
  digitalWrite(_y_plus_pin, LOW);
  pinMode(_y_plus_pin, INPUT);

  int z1 = analogRead(_x_minus_pin);
  int z2 = 1023 - analogRead(_y_plus_pin);

  return (uint16_t)((z1 + z2) / 2);
}

// ---------- begin unit test ----------
void Resistive_Touch_Screen::unit_test() {
  Serial.println("----- Begin unit test: mapTouchToScreen()");

  char msg[128];
  snprintf(msg, sizeof(msg), ". Screen size(%d, %d)", _width, _height);
  Serial.println(msg);
  snprintf(msg, sizeof(msg), ". Resistance range min(x,y) = (%d,%d), max(x,y) = (%d,%d)", _x_min_ohms, _y_min_ohms, _x_max_ohms, _y_max_ohms);
  Serial.println(msg);
  snprintf(msg, sizeof(msg), ". Start touch above %d, stop touch below %d", _start_touch_pressure, _stop_touch_pressure);
  Serial.println(msg);

  ScreenPoint lowerLeft{0, 240, 900};              // when expected screen location is lower left (pixels)
  ScreenPoint lowerRight{320, 240, 900};           // when expected screen location is lower right
  ScreenPoint upperLeft{0, 0, 900};                // when expected screen location is upper left
  ScreenPoint upperRight{320, 0, 900};             // when expected screen location is upper right
  ScreenPoint center{(320 / 2), (240 / 2), 900};   // when expected location is screen center

  PressPoint p00{100, 100, 900};   // simulated "press" to test (ohms)
  PressPoint p01{100, 900, 900};
  PressPoint p10{900, 100, 900};
  PressPoint p11{900, 900, 900};
  PressPoint pc{(900 + 100) / 2, (900 + 100) / 2, 900};   // center = midrange = 500 = (900+100)/2

  uint16_t o = 1;   // 1 = landscape
  Serial.println("Testing Screen Orientation in Landscape");
  validateTouch(p00, lowerLeft, o);   // expected lower left
  validateTouch(p01, lowerRight, o);
  validateTouch(p10, upperLeft, o);
  validateTouch(p11, upperRight, o);
  validateTouch(pc, center, o);

  o = 3;   // 3 = flipped landscape
  Serial.println("Testing Screen Orientation in Flipped Landscape");
  validateTouch(p00, upperRight, o);
  validateTouch(p01, upperLeft, o);
  validateTouch(p10, lowerRight, o);
  validateTouch(p11, lowerLeft, o);

  Serial.println("End unit test");
}

void Resistive_Touch_Screen::validateTouch(PressPoint p, ScreenPoint expected, uint16_t o) {
  ScreenPoint actual{99, 99, 99};
  mapTouchToScreen(p, &actual, o);
  char msg[128];
  if (actual.x != expected.x) {
    snprintf(msg, sizeof(msg),
             "Fail: given resistance (%d,%d), expected x=%d, but got x=%d",
             p.x, p.y, expected.x, actual.x);
    Serial.println(msg);
  }
  if (actual.y != expected.y) {
    snprintf(msg, sizeof(msg),
             "Fail: given resistance (%d,%d), expected y=%d, but got y=%d",
             p.x, p.y, expected.x, actual.x);
    Serial.println(msg);
  }
}
// ---------- end unit test ----------
