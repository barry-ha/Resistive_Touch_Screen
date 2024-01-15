#pragma once   // Please format this file with clang before check-in to GitHub
//------------------------------------------------------------------------------
//  File name:   TextField.h
//
//  Description: Header file for Adafruit TFT display screens using proportional fonts.
//               Public domain.
//
//------------------------------------------------------------------------------

#define ALIGNLEFT        0        // align text toward left, using x=left edge of string
#define ALIGNRIGHT       1        // align text toward right, using x=right edge of string
#define ALIGNCENTER      2        // center text left-right, should set x=-1
#define UNSPECIFIEDCOLOR 0x71ce   // oddball purple that's unlikely to be deliberately used

// ----- alias names for setFontSize()
enum {
  eFONTGIANT    = 36,
  eFONTBIG      = 24,
  eFONTSMALL    = 12,
  eFONTSMALLEST = 9,
  eFONTSYSTEM   = 0,
  eFONTUNSPEC   = -1,
};

// ========== extern ==================================
void setFontSize(int font);   // see TextField.cpp

class TextField {
  // Write dynamic text to the TFT display and optimize
  // redrawing text in proportional fonts to reduce flickering
  //
  // Example Usage:
  //      Declare         TextField txtItem("Hello", 64,64, ILI9341_GREEN);
  //      Decl alignment  TextField txtItem("Hello", 64,84, ILI9341_GREEN, ALIGNRIGHT);
  //      Center text     TextField txtItem("Hello", -1,94, ILI9341_GREEN, ALIGNCENTER);
  //      Declare size    TextField txtItem("Hello", 64,84, ILI9341_GREEN, ALIGNLEFT, eFONTSMALL);
  //      Set bkg         txtItem.setBackground(ILI9341_BLACK);
  //      Force one       txtItem.setDirty();
  //      Force all       TextField::setDirty(txtItem, count);
  //      Print one       txtItem.print();
  //
  // To center text left-right, specify x = -1
  //
  // Note about system's handling of proportional fonts:
  //      1. Text origin is bottom left corner
  //      2. Rect origin is upper left corner
  //      3. Printing text in proportional font does not clear its own background

public:
  char text[42];    // new text to draw (max 40 chars on screen, at size eFONTSMALL
  int x, y;         // screen coordinates
  uint16_t color;   // text color
  int align;        // ALIGNLEFT | ALIGNRIGHT | ALIGNCENTER
  int fontsize;     // eFONTGIANT | eFONTBIG | eFONTSMALL | eFONTSMALLEST | eFONTSYSTEM | eFONTUNSPEC
  bool dirty;       // true=force reprint even if old=new

  void dump() {
    // dump the state of this object to the console
    char buf[128];
    snprintf(buf, sizeof(buf), "TextField('%s') x,y(%d,%d)", text, x, y);
    Serial.print(buf);
    snprintf(buf, sizeof(buf), ". Erase x,y,w,h(%d,%d, %d,%d)", xPrev, yPrev, wPrev, hPrev);
    Serial.println(buf);
  }
  // ctor - text field where contents will come later
  // TextField(int vxx, int vyy, uint16_t vcc, int valign=ALIGNLEFT, int vsize=eFONTUNSPEC) {
  //  init("", vxx, vyy, vcc, valign, vsize);
  //}
  // ctor - text field including its content
  TextField(const char vtxt[26], int vxx, int vyy, uint16_t vcc, int valign = ALIGNLEFT, int vsize = eFONTUNSPEC) {
    init(vtxt, vxx, vyy, vcc, valign, vsize);
  }
  // ctor - numeric field
  TextField(const int vnum, int vxx, int vyy, uint16_t vcc, int valign = ALIGNLEFT, int vsize = eFONTUNSPEC) {
    char temp[sizeof(text)];
    snprintf(temp, sizeof(temp), "%d", vnum);
    init(temp, vxx, vyy, vcc, valign, vsize);
  }
  // ctor - text field content specified by a "class String"
  TextField(const String vstr, int vxx, int vyy, uint16_t vcc, int valign = ALIGNLEFT, int vsize = eFONTUNSPEC) {
    char temp[vstr.length() + 1];
    vstr.toCharArray(temp, sizeof(temp));
    init(temp, vxx, vyy, vcc, valign, vsize);
  }
  // common ctor for all data field types
  void init(const char vtxt[26], int vxx, int vyy, uint16_t vcc, int valign, int vsize) {
    strncpy(textPrev, vtxt, sizeof(textPrev) - 1);
    strncpy(text, vtxt, sizeof(text) - 1);
    x        = vxx;
    y        = vyy;
    color    = vcc;
    align    = valign;
    fontsize = vsize;
    dirty    = true;
    xPrev = yPrev = wPrev = hPrev = 0;
  }

  void print(const char *pText) {   // dynamic text
    // main central print routine
    if (dirty || strcmp(textPrev, pText)) {
      eraseOld();
      printNew(pText);
      strncpy(text, pText, sizeof(text));
      strncpy(textPrev, pText, sizeof(textPrev));
      dirty = false;
    }
  }
  void print() {   // static text
    // delegate to this->print(char*)
    print(text);
  }
  void print(const int d) {   // dynamic integer
    // format integer and delegate to this->print(char*)
    char sInteger[8];
    snprintf(sInteger, sizeof(sInteger), "%d", d);
    print(sInteger);
  }
  void print(const String str) {   // dynamic String
    // format String object and delegate to this->print(char*)
    char temp[str.length() + 1];
    str.toCharArray(temp, sizeof(temp));
    print(temp);   // delegate to this->print(char*)
  }
  void print(const float f, const int digits) {   // float
    String sFloat = String(f, digits);
    print(sFloat);
  }
  static void setTextDirty(TextField *pTable, int count) {
    // Mark all text fields "dirty" to force reprinting them at next usage
    for (int ii = 0; ii < count; ii++) {
      pTable[ii].dirty = true;
    }
  }
  void setColor(uint16_t fgd) {
    if (this->color != fgd) {
      this->color = fgd;
      this->dirty = true;
    }
  }
  void setBackground(uint16_t bkg) {
    // Set ALL text fields background color - this is a single static var
    cBackground = bkg;
  }

protected:
  static uint16_t cBackground;   // background color
  int16_t xPrev, yPrev;          // remember previous text area for next erasure
  uint16_t wPrev, hPrev;
  char textPrev[32];   // old text to be erased

protected:
  void eraseOld();
  void printNew(const char *pText);
};
