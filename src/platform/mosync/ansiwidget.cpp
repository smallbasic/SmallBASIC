// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform/mosync/ansiwidget.h"
#include "platform/mosync/utils.h"

/* class AnsiWidget
 
  Displays ANSI escape codes. 

  Escape sequences start with the characters ESC (ASCII 27d / 1Bh / 033o ) 
  and [ (left bracket). This sequence is called CSI for 
  "Control Sequence Introducer".

  For more information about ANSI code see:
  http://en.wikipedia.org/wiki/ANSI_escape_code
  http://www.uv.tietgen.dk/staff/mlha/PC/Soft/Prog/BAS/VB/Function.html
  http://bjh21.me.uk/all-escapes/all-escapes.txt

  Supported control codes:
  \t      tab (20 px)
  \a      beep
  \r      return
  \n      next line
  \xC     clear screen
  \e[K    clear to end of line
  \e[0m   reset all attributes to their defaults
  \e[1m   set bold on
  \e[4m   set underline on
  \e[7m   reverse video
  \e[21m  set bold off
  \e[24m  set underline off
  \e[27m  set reverse off
*/

#define MAX_PENDING_UPDATES 10
#define V_HEIGHT 5
#define V_WIDTH  1
#define INITXY 2
#define BLACK  0
#define BLUE   1
#define GREEN  2
#define WHITE  15

static int colors[] = {
  0x000000, // 0 black
  0x000080, // 1 blue
  0x008000, // 2 green
  0x008080, // 3 cyan
  0x800000, // 4 red
  0x800080, // 5 magenta
  0x808000, // 6 yellow
  0xC0C0C0, // 7 white
  0x808080, // 8 gray
  0x0000FF, // 9 light blue
  0x00FF00, // 10 light green
  0x00FFFF, // 11 light cyan
  0xFF0000, // 12 light red
  0xFF00FF, // 13 light magenta
  0xFFFF00, // 14 light yellow
  0xFFFFFF  // 15 bright white
};

// Workaround for API's which don't take a length argument
struct TextBuffer {
  TextBuffer(const char *s, int len) : 
    str(s), len(len) {
    c = str[len];
    ((char *)str)[len] = 0;
  }

  ~TextBuffer() {
    ((char *)str)[len] = c;
  }

  const char *str;
  char c;
  int len;
};

Hyperlink::Hyperlink(const char *url, const char *label, 
                     int x, int y, int w, int h) :
  url(url),
  label(label),
  pressed(false),
  x(x),
  y(y),
  w(w),
  h(h) {
}

void Hyperlink::draw() {
  maSetColor(pressed ? colors[BLUE] : colors[GREEN]);
  maDrawText(x, y, !label.size() ? url.c_str() : label.c_str());
  maLine(x, y + h - 1, x + w, y + h - 1);
}

bool Hyperlink::overlaps(MAPoint2d pt) {
  bool outside = (pt.x < x || pt.y < y || pt.x > x + w || pt.y > y + h);
  return !outside;
}

AnsiWidget::AnsiWidget(int width, int height) :
  image(0),
  font(0),
  bg(0),
  fg(0),
  underline(0),
  invert(0),
  bold(0),
  italic(0),
  curY(0),
  curX(0),
  curYSaved(0),
  curXSaved(0),
  tabSize(0),
  textSize(0),
  dirty(0),
  width(width),
  height(height),
  virtualWidth(width * V_WIDTH),
  virtualHeight(height * V_HEIGHT),
  scrollY(0),
  touchX(-1),
  touchY(-1),
  touchMode(0),
  hyperlinkListener(0),
  activeLink(0) {
}

bool AnsiWidget::construct() {
  bool result = false;
  image = maCreatePlaceholder();
  if (image && RES_OK == maCreateDrawableImage(image, virtualWidth, virtualHeight)) {
    reset(true);
    clearScreen();
    result = true;
  }
  return result;
}

// widget clean up
AnsiWidget::~AnsiWidget() {
  maDestroyPlaceholder(image);
  if (font) {
    maFontDelete(font);
  }
}

// create audible beep sound
void AnsiWidget::beep() const {
  //  QApplication::beep();
  // http://www.mosync.com/documentation/manualpages/using-audio-api
}

// clear the offscreen buffer
void AnsiWidget::clearScreen() {
  reset(true);
  maSetDrawTarget(image);
  maSetColor(bg);
  maFillRect(0, 0, width, height);
  flush(dirty = true);
}

// draws the given image onto the offscreen buffer
void AnsiWidget::drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h) {
  maSetDrawTarget(image);
  maSetColor(fg);

  // TODO - draw image

  flush(false);
}

// draw a line onto the offscreen buffer
void AnsiWidget::drawLine(int x1, int y1, int x2, int y2) {
  maSetDrawTarget(image);
  maSetColor(fg);
  maLine(x1, y1, x2, y2);
  flush(false);
}

// draw a rectangle onto the offscreen buffer
void AnsiWidget::drawRect(int x1, int y1, int x2, int y2) {
  maSetDrawTarget(image);
  maSetColor(fg);
  maLine(x1, y1, x2, y1); // top
  maLine(x1, y2, x2, y2); // bottom
  maLine(x1, y1, x1, y2); // left
  maLine(x2, y1, x2, y2); // right
  flush(false);
}

// draw a filled rectangle onto the offscreen buffer
void AnsiWidget::drawRectFilled(int x1, int y1, int x2, int y2) {
  maSetDrawTarget(image);
  maSetColor(fg);
  maFillRect(x1, y1, x2 - x1, y2 - y1);
  flush(false);
}

// display and pending images changed
void AnsiWidget::flush(bool force) {
  bool update = false;
  if (force) {
    update = dirty;
  } else {
    update = (++dirty == MAX_PENDING_UPDATES);
  }

  if (update) {
    MARect srcRect;
    MAPoint2d dstPoint;
    srcRect.left = 0;
    srcRect.top = scrollY;
    srcRect.width = width;
    srcRect.height = height;
    dstPoint.x = 0;
    dstPoint.y = 0;

    maSetDrawTarget(HANDLE_SCREEN);    
    maDrawImageRegion(image, &srcRect, &dstPoint, TRANS_NONE);
    maUpdateScreen();
    maResetBacklight();
    dirty = 0;
  }
}

// returns the color of the pixel at the given xy location
int AnsiWidget::getPixel(int x, int y) {
  MARect rc;
  rc.left = x;
  rc.top = y;
  rc.width = 1;
  rc.height = 1;

  int result[1];

  maGetImageData(image, &result, &rc, 1);
  return result[0];
}

// Returns the height in pixels using the current font setting
int AnsiWidget::textHeight(void) {
  return EXTENT_Y(maGetTextSize("Q@"));
}

// returns the width in pixels using the current font setting
int AnsiWidget::textWidth(const char *str, int len) {
  int result;
  if (len != -1) {
    TextBuffer text(str, len);
    result = EXTENT_X(maGetTextSize(text.str));
  } else {
    result = EXTENT_X(maGetTextSize(str));
  }
  return result;
}

// prints the contents of the given string onto the backbuffer
void AnsiWidget::print(const char *str) {
  maSetDrawTarget(image);
  maSetColor(fg);

  int len = strlen(str);
  if (len <= 0) {
    return;
  }

  int lineHeight = textHeight();

  // copy the string to allow length manipulation
  char *buffer = new char[len + 1];
  strncpy(buffer, str, len);
  buffer[len] = 0;
  char *p = buffer;

  while (*p) {
    switch (*p) {
    case '\a':                 // beep
      beep();
      break;
    case '\t':
      curX = calcTab(curX + 1);
      break;
    case '\xC':
      clearScreen();
      break;
    case '\033':               // ESC ctrl chars
      if (*(p + 1) == '[') {
        p += 2;
        while (doEscape(p)) {
          // continue
        }
      }
      break;
    case '\n':                 // new line
      newLine();
      break;
    case '\r':                 // return
      curX = INITXY;           // erasing the line will clear any previous text
      break;
    default:
      int numChars = 1;         // print minimum of one character
      int cx = charWidth(*p);
      int w = width - 1;

      if (curX + cx >= w) {
        newLine();
      }
      // print further non-control, non-null characters 
      // up to the width of the line
      while (p[numChars] > 31) {
        cx += charWidth(*p);
        if (curX + cx < w) {
          numChars++;
        } else {
          break;
        }
      }

      // erase the background
      maSetColor(invert ? this->fg : this->bg);
      maFillRect(curX, curY, cx, lineHeight);

      // draw the text buffer
      maSetColor(invert ? this->bg : this->fg);
      maDrawText(curX, curY, TextBuffer((const char *)p, numChars).str);

      if (underline) {
        maLine(curX, curY + lineHeight - 1, curX + cx, curY + lineHeight - 1);
      }
      // advance
      p += numChars - 1;        // allow for p++ 
      curX += cx;
    };

    if (*p == '\0') {
      break;
    }
    p++;
  }

  // cleanup
  delete [] buffer;
  flush(false);
}

// update the widget to new dimensions
void AnsiWidget::resize(int newWidth, int newHeight) {
  if (newWidth > virtualWidth || newHeight > virtualHeight) {
    // screen is larger than existing virtual size
    int newVWidth = newHeight * V_WIDTH;
    int newVHeight = newHeight * V_HEIGHT;

    MAHandle newImage = maCreatePlaceholder();
    maCreateDrawableImage(newImage, newVWidth, newVHeight);
    
    MARect srcRect;
    MAPoint2d dstPoint;

    // overlay the old image onto the new image
    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.width = (virtualWidth > newVWidth ? newVWidth : virtualWidth);
    srcRect.height = (virtualHeight > newVHeight ? newVHeight : virtualHeight);
    dstPoint.x = 0;
    dstPoint.y = 0;
    
    maSetDrawTarget(newImage);
    maDrawImageRegion(image, &srcRect, &dstPoint, TRANS_NONE);
    
    maDestroyPlaceholder(image);
    image = newImage;
    width = newWidth;
    height = newHeight;
    virtualWidth = newVWidth;
    virtualHeight = newVHeight;
    
    if (curY >= height) {
      curY = height - textHeight();
    }
    if (curX >= width) {
      curX = 0;
    }
    
    maSetDrawTarget(HANDLE_SCREEN);
    maDrawImage(image, 0, 0);
    maUpdateScreen();
  }
}

// sets the current drawing color
void AnsiWidget::setColor(long fg) {
  this->fg = ansiToMosync(fg);
}

// sets the pixel to the given color at the given xy location
void AnsiWidget::setPixel(int x, int y, int c) {
  maSetDrawTarget(image);
  maSetColor(c);
  maPlot(x, y);
}

// sets the current text drawing color
void AnsiWidget::setTextColor(long fg, long bg) {
  this->bg = ansiToMosync(bg);
  this->fg = ansiToMosync(fg);
}

// resets mouse mode to false
void AnsiWidget::resetMouse() {
  touchX = touchY = -1;
  touchMode = false;
}

// sets mouse mode on or off
void AnsiWidget::setMouseMode(bool flag) {
  touchMode = flag;
}

// handler for pointer touch events
void AnsiWidget::pointerTouchEvent(MAEvent &event) {
  touchX = event.point.x;
  touchY = event.point.y;

  Vector_each(Hyperlink*, it, hyperlinks) {
    if ((*it)->overlaps(event.point)) {
      maSetDrawTarget(image);
      activeLink = (*it);
      activeLink->pressed = true;
      activeLink->draw();
      flush(dirty = true);
      break;
    }
  }
}

// handler for pointer move events
void AnsiWidget::pointerMoveEvent(MAEvent &event) {
  if (activeLink != NULL) {
    bool pressed = activeLink->overlaps(event.point);
    if (pressed != activeLink->pressed) {
      maSetDrawTarget(image);
      activeLink->pressed = pressed;
      activeLink->draw();
      flush(dirty = true);
    }
  } else {
    // scroll up/down
    int lineHeight = textHeight();
    int vscroll = (touchY - event.point.y) / lineHeight;
    if (vscroll > 0 && vscroll < virtualHeight - height) {
      scrollY = vscroll;
      touchX = event.point.x;
      touchY = event.point.y;
      flush(dirty = true);
    }
  }
}

// handler for pointer release events
void AnsiWidget::pointerReleaseEvent(MAEvent &event) {
  if (activeLink != NULL && activeLink->pressed) {
    maSetDrawTarget(image);
    maSetColor(fg);
    activeLink->pressed = false;
    activeLink->draw();
    flush(dirty = true);
    if (hyperlinkListener) {
      hyperlinkListener->linkClicked(activeLink->url.c_str());
    }
  }
  touchX = touchY = -1;
}

// converts ANSI colors to FLTK colors
int AnsiWidget::ansiToMosync(long c) {
  int result = c;
  if (c < 0) {
    result = -c;
  } else {
    result = (c > 16) ? colors[WHITE] : colors[c];
  }
  return result;
}

// calculate the pixel movement for the given cursor position
int AnsiWidget::calcTab(int x) const {
  int c = 1;
  while (x > tabSize) {
    x -= tabSize;
    c++;
  }
  return c * tabSize;
}

// calculate the pixel width of the given character
int AnsiWidget::charWidth(char c) {
  char measure[] = { c, 0 };
  int result = EXTENT_X(maGetTextSize(measure));
  return result;
}

// creates a hyperlink, eg // ^[ hwww.foo.com|title|hover;More text
void AnsiWidget::createLink(char *&p, bool execLink) {
  Vector<String *> *items = getItems(p);
  const char *url = items->size() > 0 ? (*items)[0]->c_str() : "";
  const char *text = items->size() > 1 ? (*items)[1]->c_str() : "";

  if (execLink && hyperlinkListener) {
    hyperlinkListener->linkClicked(url);
  } else {
    MAExtent textSize = maGetTextSize(text);
    int w = EXTENT_X(textSize) + 2;
    int h = EXTENT_Y(textSize) + 2;
    int x = curX;
    int y = curY;
    if (curX + w >= width) {
      w = width - curX; // clipped
      newLine();
    } else {
      curX += w;      
    }
    Hyperlink *link = new Hyperlink(url, text, x, y, w, h);
    hyperlinks.add(link);
    link->draw();
  }

  deleteItems(items);
}

// cleanup the string list created in getItems()
void AnsiWidget::deleteItems(Vector<String *> *items) {
  Vector_each(String*, it, *items) {
    delete (*it);
  }
  delete items;
}

// handles the characters following the \e[ sequence. Returns whether a further call
// is required to complete the process.
bool AnsiWidget::doEscape(char *&p) {
  int escValue = 0;

  while (isdigit(*p)) {
    escValue = (escValue *10) + (*p - '0');
    p++;
  }

  if (*p == ' ') {
    p++;
    switch (*p) {
    case 'C':
      // GSS Graphic Size Selection
      textSize = escValue;
      updateFont();
      break;
    case 'h':
      createLink(p, false);
      break;
    case 'H':
      createLink(p, true);
      break;
    case 'p':
    case 'P':
      setPopupMode(escValue);
      break;
    case 'a':
    case 'A':
      showAlert(p);
      break;
    }
  } else if (setGraphicsRendition(*p, escValue)) {
    updateFont();
  }

  bool result = false;
  if (*p == ';') {
    result = true;
    p++;                        // next rendition
  }
  return result;
}

// returns list of strings extracted from the semi-colon separated input string
Vector<String *> *AnsiWidget::getItems(char *&p) {
  Vector<String *> *result = new Vector<String *>();
  char *next = p + 1;
  bool eot = false;
  int index = 0;

  while (*p && !eot) {
    p++;
    switch (*p) {
    case '\033':
    case '\n':
    case ';':
      eot = true;
      // fallthru

    case '|':
      result->add(new String((const char *)next, (p - next)));
      next = p + 1;
      break;

    default:
      break;
    }
  }
  return result;
}

// handles the \n character
void AnsiWidget::newLine() {
  int lineHeight = textHeight();
  int offset = curY + (lineHeight * 2);
  curX = INITXY;

  if (offset >= height) {
    if (offset >= virtualHeight - height) {
      MAHandle newImage = maCreatePlaceholder();
      maCreateDrawableImage(newImage, virtualWidth, virtualHeight);
      
      MARect srcRect;
      MAPoint2d dstPoint;
      
      srcRect.left = 0;
      srcRect.top = lineHeight;
      srcRect.width = virtualWidth;
      srcRect.height = virtualHeight - lineHeight;
      dstPoint.x = 0;
      dstPoint.y = 0;
      
      maSetDrawTarget(newImage);
      maDrawImageRegion(image, &srcRect, &dstPoint, TRANS_NONE);
      maSetColor(bg);
      maFillRect(0, virtualHeight - lineHeight, virtualWidth, lineHeight);
      
      maSetDrawTarget(image);
      maDrawImage(newImage, 0, 0);
      maDestroyPlaceholder(newImage);
    } else {
      scrollY += lineHeight;
      curY += lineHeight;
    }
  } else {
    curY += lineHeight;
  }
}

// reset the current drawing variables
void AnsiWidget::reset(bool init) {
  if (init) {
    curY = INITXY;              // allow for input control border
    curX = INITXY;
    tabSize = 40;               // tab size in pixels (160/32 = 5)

    // cleanup any hyperlinks
    Vector_each(Hyperlink*, it, hyperlinks) {
      delete (*it);
    }
    hyperlinks.clear();
  }

  curYSaved = 0;
  curXSaved = 0;
  invert = false;
  underline = false;
  bold = false;
  italic = false;
  fg = DEFAULT_COLOR;
  bg = 0;
  textSize = height / 40;
  updateFont();
}

// handles the given escape character. Returns whether the font has changed
bool AnsiWidget::setGraphicsRendition(char c, int escValue) {
  switch (c) {
  case 'K':
    maSetColor(this->bg);      // \e[K - clear to eol 
    maFillRect(curX, curY, width - curX, textHeight());
    break;
  case 'G':                    // move to column
    curX = escValue;
    break;
  case 'T':                    // non-standard: move to n/80th of screen width
    curX = escValue * width / 80;
    break;
  case 's':                    // save cursor position
    curYSaved = curX;
    curXSaved = curY;
    break;
  case 'u':                    // restore cursor position
    curX = curYSaved;
    curY = curXSaved;
    break;
  case ';':                    // fallthru
  case 'm':                    // \e[...m - ANSI terminal
    switch (escValue) {
    case 0:                    // reset
      reset(false);
      break;
    case 1:                    // set bold on
      bold = true;
      return true;
    case 2:                    // set faint on
      break;
    case 3:                    // set italic on
      italic = true;
      return true;
    case 4:                    // set underline on
      underline = true;
      break;
    case 5:                    // set blink on
      break;
    case 6:                    // rapid blink on
      break;
    case 7:                    // reverse video on
      invert = true;
      break;
    case 8:                    // conceal on
      break;
    case 21:                   // set bold off
      bold = false;
      return true;
    case 23:
      italic = false;
      return true;
    case 24:                   // set underline off
      underline = false;
      break;
    case 27:                   // reverse video off
      invert = false;
      break;
      // colors - 30..37 foreground, 40..47 background
    case 30:                   // set black fg
      this->fg = ansiToMosync(0);
      break;
    case 31:                   // set red fg
      this->fg = ansiToMosync(4);
      break;
    case 32:                   // set green fg
      this->fg = ansiToMosync(2);
      break;
    case 33:                   // set yellow fg
      this->fg = ansiToMosync(6);
      break;
    case 34:                   // set blue fg
      this->fg = ansiToMosync(1);
      break;
    case 35:                   // set magenta fg
      this->fg = ansiToMosync(5);
      break;
    case 36:                   // set cyan fg
      this->fg = ansiToMosync(3);
      break;
    case 37:                   // set white fg
      this->fg = ansiToMosync(7);
      break;
    case 40:                   // set black bg
      this->bg = ansiToMosync(0);
      break;
    case 41:                   // set red bg
      this->bg = ansiToMosync(4);
      break;
    case 42:                   // set green bg
      this->bg = ansiToMosync(2);
      break;
    case 43:                   // set yellow bg
      this->bg = ansiToMosync(6);
      break;
    case 44:                   // set blue bg
      this->bg = ansiToMosync(1);
      break;
    case 45:                   // set magenta bg
      this->bg = ansiToMosync(5);
      break;
    case 46:                   // set cyan bg
      this->bg = ansiToMosync(3);
      break;
    case 47:                   // set white bg
      this->bg = ansiToMosync(15);
      break;
    case 48:                   // subscript on
      break;
    case 49:                   // superscript
      break;
    };
  }
  return false;
}

// commence or end displaying into the popup image
void AnsiWidget::setPopupMode(int mode) {

}

// display an alert box - eg // ^[ aAlert!!;
void AnsiWidget::showAlert(char *&p) {
  Vector<String *> *items = getItems(p);

  const char *title = items->size() > 0 ? (*items)[0]->c_str() : "";
  const char *message = items->size() > 1 ? (*items)[1]->c_str() : "";
  const char *button1 = items->size() > 2 ? (*items)[2]->c_str() : "";
  const char *button2 = items->size() > 3 ? (*items)[3]->c_str() : "";
  const char *button3 = items->size() > 4 ? (*items)[4]->c_str() : "";

  maAlert(title, message, button1, button2, button3);
  deleteItems(items);
}

// updated the current font according to accumulated flags
void AnsiWidget::updateFont() {
  if (font) {
    maFontDelete(font);
  }
  int style = FONT_STYLE_NORMAL;
  if (italic) {
    style |= FONT_STYLE_ITALIC;
  }
  if (bold) {
    style |= FONT_STYLE_BOLD;
  }

  font = maFontLoadDefault(FONT_TYPE_MONOSPACE, style, textSize);
  maFontSetCurrent(font);
}
