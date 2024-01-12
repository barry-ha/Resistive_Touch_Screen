// Please format this file with clang before check-in to GitHub
/*
  Touchscreen Calibrator - Touch screen with X, Y (resistance) and Z (pressure) readings

  Software: Barry Hansen, K7BWH, barry@k7bwh.com, Seattle, WA

  Purpose:  Interactive display of the effects of Touch Screen calibration settings.
            This program shows the current compiled touch settings alongside the
            real-time measurements as you touch the screen.

  Usage:    1. Compile and run the program
            2. Tap gently with a stylus around the center, then work your way around the screen edges
            3. Each tap leaves a dot, showing how resistance measurements are mapped to screen area
            4. Edit source code "Touch configuration parameters" to adjust the mapping
            5. Recompile as needed and run this again
            6. Once you're satisifed with the values, use them in your own project
            7. If you're using library Resistive_Touch_Screen,
               then use your new values to override its built-in defaults:
               setResistanceRange(X_MIN_OHMS, X_MAX_OHMS, Y_MIN_OHMS, Y_MAX_OHMS, XP_XM_OHMS);

  Adjusting X_MIN_OHMS:
            Run program and touch near bottom of screen.
            If dots are below your touch, increase X_MIN_OHMS
            if dots are above your touch, decrease X_MIN_OHMS
  Adjusting X_MAX_OHMS:
            Run program and touch near top of screen.
            If dots are below your touch, increase X_MIN_OHMS
            if dots are above your touch, decrease X_MIN_OHMS

  Coordinate system:
            X and Y are in terms of the native touchscreen coordinates, which corresponds to
            native portrait mode (screen rotation 0). The touchscreen does not "rotate" when
            the TFT display changes to a new orientation.

  Dead zone:
            Many of the Adafruit touchscreen displays tested have a dead triangle near
            maximum X,Y measured resistances. I've tested more than a dozen screens in 2023.
            As you design your screen layout, we recommend NOT placing touchscreen
            controls in the bottom right (max X and Y) corner.

  Tested with:
         1. Arduino Feather M4 Express (120 MHz SAMD51)     https://www.adafruit.com/product/3857

         2. Adafruit 3.2" TFT color LCD display ILI-9341    https://www.adafruit.com/product/1743
            How to:      https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2
            SPI Wiring:  https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2/spi-wiring-and-test
            Touchscreen: https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2/resistive-touchscreen
*/

#include <Resistive_Touch_Screen.h>   // https://github.com/barry-ha/Resistive_Touch_Screen
#include <Adafruit_ILI9341.h>         // TFT color display library
#include <TouchScreen.h>              // Adafruit / Adafruit_Touchscreen library
#include <elapsedMillis.h>            // Scheduling intervals in main loop
#include "TextField.h"                // Helper for showing text on TFT display

// ---------- Touch Screen pins, depends on wiring from CPU to Touch Screen
#define PIN_XP A3   // Touchscreen X+ can be a digital pin
#define PIN_XM A4   // Touchscreen X- must be an analog pin, use "An" notation
#define PIN_YP A5   // Touchscreen Y+ must be an analog pin, use "An" notation
#define PIN_YM 9    // Touchscreen Y- can be a digital pin

// ---------- Touch Screen configuration
#define XP_XM_OHMS 310             // Resistance in ohms between X+ and X- to calibrate touch pressure
                                   // measure this with an ohmmeter while device is turned off
#define X_MIN_OHMS           150   // Default: Expected range on touchscreen's X-axis readings
#define X_MAX_OHMS           900
#define Y_MIN_OHMS           100   // Default: Expected range on touchscreen's Y-axis readings
#define Y_MAX_OHMS           900
#define START_TOUCH_PRESSURE 200   // Minimum pressure threshold considered start of "press"
#define END_TOUCH_PRESSURE   50    // Maximum pressure threshold required before end of "press"

// ---------- Constructor
Resistive_Touch_Screen tsn(PIN_XP, PIN_YP, PIN_XM, PIN_YM, XP_XM_OHMS);   // For single-point touches:
TouchScreen ts(PIN_XP, PIN_YP, PIN_XM, PIN_YM, XP_XM_OHMS);               // For continuous touch information

// ------- Identity for splash screen and console --------
#define PROGRAM_NAME "Touchscreen Calibrator"

// ----- define the TFT hardware
#define TFT_BL       4     // TFT backlight
#define TFT_CS       5     // TFT chip select pin
#define TFT_DC       12    // TFT display/command pin
#define SCREENWIDTH  320   //
#define SCREENHEIGHT 240   //

// ----- alias names for SCREEN_ROTATION
enum Rotation {
  PORTRAIT          = 0,   //   0 degrees = portrait
  LANDSCAPE         = 1,   //  90 degrees = landscape
  FLIPPED_PORTRAIT  = 2,   // 180 degrees = portrait flipped 180-degrees
  FLIPPED_LANDSCAPE = 3,   // 270 degrees = landscape flipped 180-degrees
};

// ----- create an instance of the TFT Display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// ----- color scheme
// RGB 565 true color: https://chrishewett.com/blog/true-rgb565-colour-picker/
#define cBACKGROUND 0x00A            // 0,   0,  10 = darker than ILI9341_NAVY, but not black
#define cLABEL      ILI9341_GREEN    //
#define cVALUE      ILI9341_YELLOW   // 255, 255, 0
#define cTEXTCOLOR  0x67FF           // rgb(102,255,255) = hsl(180,100,70%)

void clearScreen() {
  tft.fillScreen(cBACKGROUND);
}

/* ========== main screen ====================================
        xul
  yul..+-:-----------------------------------------+
       | Touch Screen Calibrator           800 max | 1
       |                                        :  | 2
       |                                        :  | 3   ^
       |                                        :  | 4   |
       | Current pressure = ...            ...  X  | 5   X-axis
       | Pressure threshold = 200               :  | 6   |
       |                                        :  | 7   v
       |                                        :  | 8
       |                                   240 min | 9
       | 320              ...                  760 | 10
       | min ~ ~ ~ ~ ~ ~ ~ Y ~ ~ ~ ~ ~ ~ ~ ~ ~ max | 11
       +-------------------:--:------------:-----:-+
  pixel coords:          x1 x2           x3    x4

  touch coords:      <-- Y-axis -->
*/
#define cXGROUP ILI9341_MAGENTA
#define cYGROUP ILI9341_GREENYELLOW

const int xul   = 24;                     // upper left corner of screen text
const int yul   = 0;                      //
const int ht    = 20;                     // row height
const int xData = 12;                     // upper left corner of screen text
const int x1    = SCREENWIDTH / 2 - 10;   // = 320/2 = 160
const int x2    = x1 + 48;
const int x3    = SCREENWIDTH - 80;
const int x4    = SCREENWIDTH - 8;
const int row1  = yul + ht;
const int row2  = row1 + ht;
const int row3  = row2 + ht;
const int row4  = row3 + ht;
const int row5  = row4 + ht;
const int row6  = row5 + ht;
const int row7  = row6 + ht;
const int row8  = row7 + ht;
const int row9  = row8 + ht;
const int row10 = row9 + ht + 6;   // a little extra room for bottom two rows
const int row11 = row10 + ht;

// clang-format off
TextField txtScreen[] = {
  // row 1
    {X_MAX_OHMS,   xData, row1, cXGROUP},                   // [1]
  // row 2
    {PROGRAM_NAME,  -1, row2, cTEXTCOLOR, ALIGNCENTER},     // [0]
  // row 3
  // row 4
  // row 5
    {"Touch pressure:",    x2, row5, cLABEL, ALIGNRIGHT},   // [2]
    {"ppp",             x2+12, row5, cVALUE, ALIGNLEFT},    // [3] current pressure value
  // row 6
    {"X   ",            xData, row6, cXGROUP, ALIGNLEFT},   // [4]
    {"xxx",             xData, row7, cVALUE},               // [5] current X value
  // row 7
  // row 8
  // row 9
    {X_MIN_OHMS,        xData, row10, cXGROUP},             // [6]
  // row 10
    {"Y",                -1, row10, cYGROUP, ALIGNCENTER},  // [7]
  // row 11
    {Y_MIN_OHMS,        xData, row11, cYGROUP},             // [8]
    {"yyy",                x1, row11, cVALUE },             // [9] current Y value
    {Y_MAX_OHMS,           x4, row11, cYGROUP, ALIGNRIGHT}, // [10]
};
const int numScreenFields = sizeof(txtScreen) / sizeof(TextField);
// clang-format on

// ========== main screen ====================================
void startScreen() {

  clearScreen();                             // clear screen
  txtScreen[0].setBackground(cBACKGROUND);   // set background for all TextFields
  setFontSize(eFONTSMALLEST);                // small font but not tiny like the default system font

  TextField::setTextDirty(txtScreen, numScreenFields);   // make sure all fields are updated
  for (int ii = 0; ii < numScreenFields; ii++) {         // now update all the fields
    txtScreen[ii].print();
  }
}

void updateScreen(TSPoint tp) {
  txtScreen[5].print(tp.x);   // current X value
  txtScreen[9].print(tp.y);   // current Y value
  txtScreen[3].print(tp.z);   // current pressure value
}

void labelAxis() {
  const int xV = x3 - 4;      // screen X coord of vertical line
  const int yH = row9 + 10;   // screen Y coord of horizontal line

  struct TwoPoints {
    int x1, y1;
    int x2, y2;
    uint16_t color;
  };

  const int yA = SCREENHEIGHT - 5;   // screen Y coord of left/rht arrow
  const int xA = 5;                  // screen X coord of up/down arrow

  // clang-format off
#define nSolidLines 6
  TwoPoints solidLines[nSolidLines] = {
      {    xul, yA,     200, yA, cYGROUP},   // right arrow
      {200 -10, yA - 2, 200, yA, cYGROUP},   // top arrowhead
      {200 -10, yA + 2, 200, yA, cYGROUP},   // bot arrowhead

      { xA,  240-24,  xA, 80, cXGROUP},      // up arrow
      { xA-2, 80+10,  xA, 80, cXGROUP},      // left side arrowhead
      { xA+2, 80+10,  xA, 80, cXGROUP},      // right side arrowhead
  };
  // clang-format on

  for (int ii = 0; ii < nSolidLines; ii++) {   // draw the lines and arrows
    TwoPoints item = solidLines[ii];
    tft.drawLine(item.x1, item.y1, item.x2, item.y2, item.color);
  }
}

//=========== setup ============================================
void setup() {

  // ----- init TFT backlight
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, 255);   // start at full brightness

  // ----- init TFT display
  tft.begin();                   // initialize TFT display
  tft.setRotation(LANDSCAPE);    // 1=landscape (default is 0=portrait)
  tft.fillScreen(cBACKGROUND);   // note that "begin()" does not clear screen

  // ----- init serial monitor (do not "Serial.print" before this, it won't show up in console)
  Serial.begin(115200);   // init for debugging in the Arduino IDE

  // now that Serial is ready and connected (or we gave up)...
  Serial.println(PROGRAM_NAME);                        // Report our program name to console
  Serial.println("Compiled " __DATE__ " " __TIME__);   // Report our compiled date
  Serial.println(__FILE__);                            // Report our source code file name

  // ----- init touchscreen
  tsn.setScreenSize(tft.width(), tft.height());                                         // required
  tsn.setResistanceRange(X_MIN_OHMS, X_MAX_OHMS, Y_MIN_OHMS, Y_MAX_OHMS, XP_XM_OHMS);   // optional, for overriding defaults
  tsn.unit_test();                                                                      // optional, for debug

  startScreen();   // show all the initial text on the screen
  labelAxis();     // show all the lines and arrows
}

//=========== main work loop ===================================
elapsedMillis refreshTimer;

void loop() {

  // if the the screen was touched, show where
  ScreenPoint screen;
  if (tsn.newScreenTap(&screen, tft.getRotation())) {

    const int radius = 1;
    tft.fillCircle(screen.x, screen.y, radius, ILI9341_RED);
  }

  // continuously update the "touch" location and pressure
  // but slow it down enough to read the values
  if (refreshTimer > 200) {
    refreshTimer = 0;

    TSPoint p = ts.getPoint();
    updateScreen(p);
  }
}
