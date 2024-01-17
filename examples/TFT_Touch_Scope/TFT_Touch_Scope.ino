// Please format this file with clang before check-in to GitHub
/*
  Touchscreen Oscilloscope - Graph X,Y,Z touchscreen measurements

  Software: Barry Hansen, K7BWH, barry@k7bwh.com, Seattle, WA

  Purpose:  Graph X,Y,Z touchscreen measurements over time.
            You can try different touchscreen drivers to see how they perform.

  Library Dependencies:
            Adafruit / Adafruit_Touchscreen - if you want to test stock touchscreen library
            barry-ha / Resistive_Touch_Screen - if you want to test Barry's touchscreen library
*/

#include <Adafruit_ILI9341.h>   // TFT color display library
#include <elapsedMillis.h>      // Scheduling intervals in main loop
#include "TextField.h"          // Helper for showing text on TFT display

// ---------- Touch Screen pins, depends on wiring from CPU to Touch Screen
#define PIN_XP A3   // Touchscreen X+ can be a digital pin
#define PIN_XM A4   // Touchscreen X- must be an analog pin, use "An" notation
#define PIN_YP A5   // Touchscreen Y+ must be an analog pin, use "An" notation
#define PIN_YM 9    // Touchscreen Y- can be a digital pin

// ---------- Touch Screen configuration
#define XP_XM_OHMS 0     // Set this to zero to receive raw resistance measurements 0..1023
                         // Adafruit suggests entering resistance in ohms between
                         // X+ and X- to calibrate touch pressure, as measured
                         // with an ohmmeter while device is turned off.
                         // However, if you set XP_XM_OHMS to non-zero, you might get
                         // wild +/- 16-bit readings like I did. Run this "scope" to find out.
#define X_MIN_OHMS 150   // Default: Expected range on touchscreen's X-axis readings
#define X_MAX_OHMS 900
#define Y_MIN_OHMS 100   // Default: Expected range on touchscreen's Y-axis readings
#define Y_MAX_OHMS 900
// #define START_TOUCH_PRESSURE 200   // Minimum pressure threshold considered start of "press"
// #define END_TOUCH_PRESSURE   50    // Maximum pressure threshold required before end of "press"

// ---------- Constructor
// Test 1
// #include <TouchScreen.h>   // Adafruit / Adafruit_Touchscreen library
// #define LIBRARY "Adafruit_Touchscreen"
// TouchScreen ts(PIN_XP, PIN_YP, PIN_XM, PIN_YM, XP_XM_OHMS);

// Test 2
#include <Resistive_Touch_Screen.h>   // https://github.com/barry-ha/Resistive_Touch_Screen
#define LIBRARY "Resistive_Touch_Screen"
Resistive_Touch_Screen ts(PIN_XP, PIN_YP, PIN_XM, PIN_YM, XP_XM_OHMS);

// ------- Identity for splash screen and console --------
#define PROGRAM_NAME "Touchscreen Oscilloscope"

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
#define cLIBCOLOR   0x3adf           // rgb(7,22,31)

void clearScreen() {
  tft.fillScreen(cBACKGROUND);
}

/* ========== main screen ====================================
        xul
  yul..+-:-----------------------------------------+row
       |        Touch Screen Oscilloscope          | 1
       | 0 ======================================= | .. canvasTop
       | 1023         X =   123 magenta          : | 2
       | :            Y =  1023 yellow           : | 3
       | :            Z =   345 red              : | 4
       | :                                       : | 5
       | :    ---                                : | 6
       | : ---   ---                             : | 7
       | :          ---              -----       : | 8
       | :             ---        ---     ----   : | 9
       | :                --------            ---: | 10
       | 0 ================<time>================= | 11.. canvasBottom
       +-:------------:-------:------------------:-+
     canvasLeft     xLabel  xValue          canvasRight
*/
#define colorX ILI9341_MAGENTA
#define colorY ILI9341_GREENYELLOW
#define colorZ ILI9341_RED

const int xul    = 20;                     // leftmost screen text
const int yul    = 0;                      //
const int ht     = 20;                     // row height
const int xLabel = (320 / 2);              //
const int xValue = xLabel + 60;            //
const int x1     = SCREENWIDTH / 2 - 10;   // = 320/2 = 160
const int x2     = x1 + 48;
const int x3     = SCREENWIDTH - 80;
const int x4     = SCREENWIDTH - 8;
const int row1   = yul + ht;
const int row2   = row1 + ht;
const int row3   = row2 + ht;
const int row4   = row3 + ht;
const int row5   = row4 + ht;
const int row6   = row5 + ht;
const int row7   = row6 + ht;
const int row8   = row7 + ht;
const int row9   = row8 + ht;
const int row10  = row9 + ht + 6;   // a little extra room for bottom two rows
const int row11  = row10 + ht;
const int row12  = row11 + 10;

// clang-format off
TextField txtScreen[] = {
  // row 1
    {PROGRAM_NAME, -1, row1, cTEXTCOLOR, ALIGNCENTER},   // [0]
  // row 2
    {"X =",    xLabel, row2, colorX, ALIGNRIGHT},        // [1]
    {"xxx",    xValue, row2, colorX, ALIGNRIGHT},        // [2] current X value
  // row 3
    {"Y =",    xLabel, row3, colorY, ALIGNRIGHT},        // [3]
    {"yyy",    xValue, row3, colorY, ALIGNRIGHT},        // [4] current X value
  // row 4
    {"Z =",    xLabel, row4, colorZ, ALIGNRIGHT},        // [5]
    {"zzz",    xValue, row4, colorZ, ALIGNRIGHT},        // [6] current X value
  // row 5
  // row 6
  // row 7
    {"X",            1,row7, colorX, ALIGNLEFT },        // [7]
  // row 8
  // row 9
  // row 10
    {LIBRARY,      xul, row10, cLIBCOLOR, ALIGNLEFT},    // [8]
  // row 11
    {"0",            1, row12, cTEXTCOLOR},       // [8] origin (0,0)
    {"Y",          132, row12, colorY},           // [9]
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

// repaint part of the screen where the graph might have overwritten our labels
void redrawLabels() {
  txtScreen[1].dirty = true;
  txtScreen[3].dirty = true;
  txtScreen[5].dirty = true;

  txtScreen[1].print();
  txtScreen[3].print();
  txtScreen[5].print();
}

void showMeasurementText(TSPoint tp) {
  char msg[64];
  snprintf(msg, sizeof(msg), "%4d", tp.x);   // current X value, 0..1023
  txtScreen[2].print(msg);
  snprintf(msg, sizeof(msg), "%4d", tp.y);   // current Y value, 0..1023
  txtScreen[4].print(msg);
  snprintf(msg, sizeof(msg), "%4d", tp.z);   // current Z (pressure) value, 0..1023
  txtScreen[6].print(msg);
}

struct TwoPoints {
  int x1, y1;
  int x2, y2;
  uint16_t color;
};

void labelAxis() {
  const int yUD = row7 + 4;           // screen Y coord of top tip of up/down arrow
  const int xUD = 6;                  // screen X coord of up/down arrow centerline
  const int xLR = 124;                // screen Y coord of rightmost tip of left/right arrow
  const int yLR = SCREENHEIGHT - 7;   // screen Y coord of left/rht arrow centerline

  // clang-format off
  TwoPoints solidLines[] = {
      { xUD, SCREENHEIGHT-24, xUD, yUD, colorX},      // up-down arrow
      { xUD-2, row7+16,       xUD, yUD, colorX},      // left side arrowhead
      { xUD+2, row7+16,       xUD, yUD, colorX},      // right side arrowhead

      {    20, yLR,     xLR, yLR, colorY},   // left-right arrow
      {xLR-12, yLR-2,   xLR, yLR, colorY},   // top arrowhead
      {xLR-12, yLR+2,   xLR, yLR, colorY},   // bot arrowhead

  };
  // clang-format on
  int nSolidLines = sizeof(solidLines) / sizeof(TwoPoints);

  for (int ii = 0; ii < nSolidLines; ii++) {   // draw the lines and arrows
    TwoPoints item = solidLines[ii];
    tft.drawLine(item.x1, item.y1, item.x2, item.y2, item.color);
  }
};

// ========== graphing functions ====================================

// ----- screen canvas work area
const int canvasLeft   = 18;
const int canvasRight  = SCREENWIDTH - 12;
const int canvasTop    = row1 + 4;
const int canvasBottom = SCREENHEIGHT - 21;
void drawCanvasOutline() {

  TwoPoints border[] = {
      {canvasLeft - 1, canvasTop - 1, canvasLeft - 1, canvasBottom + 1, colorZ},       // left edge
      {canvasRight + 1, canvasTop - 1, canvasRight + 1, canvasBottom + 1, colorZ},     // right edge
      {canvasLeft - 1, canvasTop - 1, canvasRight + 1, canvasTop - 1, colorZ},         // top edge
      {canvasLeft - 1, canvasBottom + 1, canvasRight + 1, canvasBottom + 1, colorZ},   // bottom edge
  };
  const int nBorder = sizeof(border) / sizeof(TwoPoints);

  for (int ii = 0; ii < nBorder; ii++) {   // draw the lines and arrows
    TwoPoints item = border[ii];
    tft.drawLine(item.x1, item.y1, item.x2, item.y2, item.color);
  }
}

// ----- data definition for circular buffer
struct Measurement {
  TSPoint latest;     // latest measurement
  TSPoint previous;   // help detect changes, to optimize erasing/drawing onto screen
};

Measurement history[canvasRight - canvasLeft];   // remember touchscreen measurements
int current = 0;                                 // index of the most recent item saved

const int totalSize  = sizeof(history);          // bytes
const int recordSize = sizeof(Measurement);      // bytes
const int capacity   = totalSize / recordSize;   // max number of records

// ----- add measurement to end of circular buffer
void saveMeasurement(TSPoint p) {
  current                 = (current + 1) % capacity;
  history[current].latest = p;
}

// ----- plot one point
// horizontal scale is time, e.g., current index
// vertical scale is measured value
void graphMeasurementItem(int index) {
  TSPoint latest = history[index].latest;
  TSPoint prev   = history[index].previous;
  int screenx    = canvasLeft + index;

  if (latest == prev) {
    // do nothing, screen has not changed
  } else {

    graphPoint(screenx, prev.x, latest.x, colorX);
    graphPoint(screenx, prev.y, latest.y, colorY);
    graphPoint(screenx, prev.z, latest.z, colorZ);

    history[index].previous = latest;
  }

  // draw a marker moving along bottom axis to show active measurement
  if (index == 0) {
    sprite(canvasRight - 1, canvasBottom, cBACKGROUND);   // erase prev at rhs
  } else {
    sprite(screenx - 1, canvasBottom, cBACKGROUND);   // erase prev
  }
  sprite(screenx, canvasBottom, ILI9341_WHITE);   // draw new
}

void sprite(int x, int y, int color) {
  if (false) {
    // circle marker
    int radius  = 2;
    int offset  = 2 + radius;   // allow room for canvas border + top half of circle
    int screeny = y + offset;   // centerline of marker
    tft.drawCircle(x, screeny, radius, color);
  } else {
    // triangle marker, corners are clockwise from lower left
    int height = 3;
    int offset = 2 + height;   // allow room for canvas border + height of triangle
    int x0     = x - 1;
    int y0     = y + offset;   // baseline of marker
    int x1     = x;
    int y1     = y0 - height;
    int x2     = x + 1;
    int y2     = y0;   // baseline of marker
    tft.drawTriangle(x0, y0, x1, y1, x2, y2, color);
  }
}

void graphPoint(int ii, int mold, int mnew, int c) {
  // erase old point
  int screeny = map(mold, 0, 1023, canvasBottom - 3, canvasTop + 3);
  tft.drawPixel(ii, screeny, cBACKGROUND);
  tft.drawPixel(ii, screeny - 1, cBACKGROUND);

  // draw new point
  screeny = map(mnew, 0, 1023, canvasBottom - 3, canvasTop + 3);
  tft.drawPixel(ii, screeny, c);
  tft.drawPixel(ii, screeny - 1, c);   // 2 pixels tall for visibility
}

//=========== setup ============================================
void setup() {

  // ----- init TFT backlight
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, 255);   // start at maximum brightness

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

  startScreen();         // show all the initial text on the screen
  labelAxis();           // show all the lines and arrows
  drawCanvasOutline();   // draw border outside canvas area
}

//=========== main work loop ===================================

elapsedMillis refreshTimer;
void loop() {

  // continuously read and update the touch measurement graph
  // use a timer to slow it down enough to read the values
  if (refreshTimer > 20) {
    refreshTimer = 0;

    // TSPoint p = getMeasurement_1();
    TSPoint p = ts.getPoint();
    saveMeasurement(p);
    showMeasurementText(p);
    graphMeasurementItem(current);

    // repaint part of the screen where the graph might have overwritten our labels
    if (current == xLabel) {
      redrawLabels();
    }
  }
}
