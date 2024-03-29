// Please format this file with clang before check-in to GitHub
/*
  File:     touch_demo.ino

  Purpose:  This demo program draws a dot where the screen is touched.
            Public domain.
*/

#include <Resistive_Touch_Screen.h>   // https://github.com/barry-ha/Resistive_Touch_Screen
#include <Adafruit_ILI9341.h>         // TFT color display library

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
                         // However, if you set non-zero, you might get +/- 16-bit
                         // readings like I did. Run TFT_Touch_Scope to find out.
#define X_MIN_OHMS 100   // X-axis expected minimum reading
#define X_MAX_OHMS 900   // X-axis expected maximum reading
#define Y_MIN_OHMS 100   // Y-axis expected minimum reading
#define Y_MAX_OHMS 900   // Y-axis expected maximum reading

// ---------- Constructor
Resistive_Touch_Screen tsn(PIN_XP, PIN_YP, PIN_XM, PIN_YM, XP_XM_OHMS);

// ----- define the TFT hardware
#define TFT_BL       4     // TFT backlight
#define TFT_CS       5     // TFT chip select pin
#define TFT_DC       12    // TFT display/command pin
#define SCREENWIDTH  320   //
#define SCREENHEIGHT 240   //

// ----- create an instance of the TFT Display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

ScreenPoint screen;     // screen coordinates of touch
const int radius = 2;   // size of small circle

//=========== setup ============================================
void setup() {
  // ----- init Serial port
  while (!Serial) {
    delay(10);
  }
  delay(500);
  Serial.println("Resistive Touch Screen Demo");

  // ----- init TFT display
  tft.begin();                     // initialize TFT display
  tft.setRotation(1);              // 1=landscape (default is 0=portrait)
  tft.fillScreen(ILI9341_BLACK);   // note that "begin()" does not clear screen
  tft.println("Resistive Touch Screen Demo");
  tft.println("Touch me");

  // ----- init touchscreen
  tsn.setScreenSize(tft.width(), tft.height());                                         // required
  tsn.setResistanceRange(X_MIN_OHMS, X_MAX_OHMS, Y_MIN_OHMS, Y_MAX_OHMS, XP_XM_OHMS);   // optional, for overriding defaults
  tsn.unit_test();                                                                      // optional, for debug
}

void loop() {
  // ----- retrieve touch on screen
  if (tsn.newScreenTap(&screen, tft.getRotation())) {          // if there's touchscreen input
    tft.fillCircle(screen.x, screen.y, radius, ILI9341_RED);   // then do something
  }
}
