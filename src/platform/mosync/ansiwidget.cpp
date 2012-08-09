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
#include <wchar.h>

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
  \xC     clear screen (new page)
  \e[K    clear to end of line
  \e[0m   reset all attributes to their defaults
  \e[1m   set bold on
  \e[4m   set underline on
  \e[7m   reverse video
  \e[21m  set bold off
  \e[24m  set underline off
  \e[27m  set reverse off
*/

#define MAX_PENDING 250
#define MAX_HEIGHT  10000
#define SCROLL_OFFS 10
#define SCROLL_IND 4
#define INITXY 2
#define BLACK  0
#define BLUE   1
#define GREEN  2
#define WHITE  15
#define BUTTON_PADDING 8
#define SWIPE_MAX_TIMER 6000
#define SWIPE_DELAY_STEP 250
#define SWIPE_SCROLL_FAST 20
#define SWIPE_SCROLL_SLOW 10
#define SWIPE_TRIGGER_FAST 40
#define SWIPE_TRIGGER_SLOW 80

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

// converts ANSI colors to FLTK colors
int ansiToMosync(long c) {
  int result = c;
  if (c < 0) {
    result = -c;
  } else {
    result = (c > 15) ? colors[WHITE] : colors[c];
  }
  return result;
}

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

Button::Button(Screen *screen, const char* action, const char *label,
               int x, int y, int w, int h) :
  action(action),
  label(label),
  pressed(false),
  bg(screen->bg),
  fg(screen->fg),
  x(x),
  y(y),
  w(w),
  h(h) {
}

bool Button::overlaps(MAPoint2d pt, int scrollX, int scrollY) {
  return !(OUTSIDE_RECT(pt.x, pt.y, x - scrollX, y - scrollY, w, h));
}

BlockButton::BlockButton(Screen *screen, const char *action, const char *label,
                         int x, int y, int w, int h) :
  Button(screen, action, label, x, y, w, h) {
}

void BlockButton::draw() {
  int r = x+w;
  int b = y+h;

  maSetColor(0x505050);
  maFillRect(x, y, w, h);

  maSetColor(pressed ? bg : fg);
  maLine(x, y, r-1, y); // top
  maLine(x, y, x, b);   // left

  maSetColor(pressed ? fg : bg);
  maLine(x, b, r, b); // bottom
  maLine(r, y, r, b); // right

  maSetColor(fg);
  maDrawText(x + 4, y + 4, label.c_str());
}

TextButton::TextButton(Screen *screen, const char *action, const char *label,
                       int x, int y, int w, int h) :
  Button(screen, action, label, x, y, w, h) {
}

void TextButton::draw() {
  maSetColor(pressed ? bg : fg);
  maDrawText(x, y, label.c_str());
  maLine(x, y + h + 1, x + w, y + h + 1);
}

Screen::Screen(int x, int y, int width, int height) :
  image(0),
  font(0),
  underline(0),
  invert(0),
  bold(0),
  italic(0),
  bg(0),
  fg(0),
  x(x),
  y(y),
  width(width),
  height(height),
  imageWidth(width),
  imageHeight(height),
  pageHeight(0),
  scrollY(0),
  curY(0),
  curX(0),
  curYSaved(0),
  curXSaved(0),
  tabSize(0),
  fontSize(0),
  charWidth(0),
  charHeight(0),
  dirty(0),
  linePadding(0) {
}

Screen::~Screen() {
  if (image) {
    maDestroyPlaceholder(image);
  }
  if (font) {
    maFontDelete(font);
  }
  Vector_each(Button*, it, buttons) {
    delete (Button*)(*it);
  }
}

// calculate the pixel movement for the given cursor position
void Screen::calcTab() {
  int c = 1;
  int x = curX + 1;
  while (x > tabSize) {
    x -= tabSize;
    c++;
  }
  curX = c * tabSize;
}

bool Screen::construct() {
  image = maCreatePlaceholder();
  bool result = (image && RES_OK ==
                 maCreateDrawableImage(image, imageWidth, imageHeight));
  if (result) {
    curX = INITXY;
    curY = INITXY;
    tabSize = 40;   // tab size in pixels (160/32 = 5)
    scrollY = 0;
    reset();
  }
  return result;
}

void Screen::clear() {
  drawInto(true);
  maSetColor(bg);
  maFillRect(0, 0, imageWidth, imageHeight);

  curX = INITXY;
  curY = INITXY;
  scrollY = 0;
  pageHeight = 0;

  // cleanup any buttons
  Vector_each(Button*, it, buttons) {
    delete (*it);
  }
  buttons.clear();
}

void Screen::draw(bool vscroll) {
  MARect srcRect;
  MAPoint2d dstPoint;
  srcRect.left = 0;
  srcRect.top = scrollY;
  srcRect.width = width;
  srcRect.height = height;
  dstPoint.x = x;
  dstPoint.y = y;

  MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
  maDrawImageRegion(image, &srcRect, &dstPoint, TRANS_NONE);

  if (vscroll && pageHeight) {
    // display the vertical scrollbar
    int barSize = height * height / pageHeight;
    int barRange = height - (barSize + SCROLL_IND * 2);
    int barTop = SCROLL_IND + (barRange * scrollY / (pageHeight - (height - SCROLL_OFFS)));
    if (barSize < height) {
      maSetColor(fg);
      maLine(x + width - 3, y + barTop, x + width - 3, y + barTop + barSize);
      maLine(x + width - 4, y + barTop, x + width - 4, y + barTop + barSize);
    }
  }

  maUpdateScreen();
  maResetBacklight();
  maSetDrawTarget(currentHandle);
  dirty = 0;
}

void Screen::drawInto(bool background) {
  maSetDrawTarget(image);
  maSetColor(background ? bg : fg);
  dirty = maGetMilliSecondCount();
}

void Screen::drawText(const char *text, int len, int x, int lineHeight) {
  // erase the background
  maSetColor(invert ? fg : bg);
  maFillRect(curX, curY, x, lineHeight);

  // draw the text buffer
  maSetColor(invert ? bg : fg);
  maDrawText(curX, curY, TextBuffer(text, len).str);

  if (underline) {
    maLine(curX, curY + lineHeight - 1, curX + x, curY + lineHeight - 1);
  }
}

// handles the \n character
void Screen::newLine(int lineHeight) {
  lineHeight += linePadding;
  linePadding = 0;
  curX = INITXY;
  if (height < MAX_HEIGHT) {
    int offset = curY + (lineHeight * 2);
    if (offset >= height) {
      if (offset >= imageHeight) {
        // extend the base image by another page size
        MAHandle newImage = maCreatePlaceholder();
        maCreateDrawableImage(newImage, imageWidth, imageHeight + height);

        MARect srcRect;
        MAPoint2d dstPoint;

        srcRect.left = 0;
        srcRect.top = 0;
        srcRect.width = imageWidth;
        srcRect.height = imageHeight;
        dstPoint.x = 0;
        dstPoint.y = 0;

        maSetDrawTarget(newImage);
        maDrawImageRegion(image, &srcRect, &dstPoint, TRANS_NONE);

        // clear the new segment
        maSetColor(bg);
        maFillRect(0, imageHeight, imageWidth, imageHeight + height);
        imageHeight += height;

        // cleanup the old image
        maDestroyPlaceholder(image);
        image = newImage;
      }
      scrollY += lineHeight;
    }
    curY += lineHeight;
    pageHeight += lineHeight;
  } else {
    // overflow
    curY = INITXY;
    pageHeight = 0;
  }
}

int Screen::print(const char *p, int lineHeight) {
  int numChars = 1;         // print minimum of one character
  int cx = charWidth;
  int w = width - 1;

  if (curX + cx >= w) {
    newLine(lineHeight);
  }

  // print further non-control, non-null characters
  // up to the width of the line
  while (p[numChars] > 31) {
    cx += charWidth;
    if (curX + cx < w) {
      numChars++;
    } else {
      break;
    }
  }

  drawText(p, numChars, cx, lineHeight);

  curX += cx;
  return numChars;
}

// reset the current drawing variables
void Screen::reset() {
  curXSaved = 0;
  curYSaved = 0;
  invert = false;
  underline = false;
  bold = false;
  italic = false;
  fg = DEFAULT_COLOR;
  bg = 0;
  fontSize = EXTENT_Y(maGetScrSize()) / 40;
  updateFont();
}

// update the widget to new dimensions
void Screen::resize(int newWidth, int newHeight, int oldWidth, int oldHeight, int lineHeight) {
  bool fullscreen = ((width - x) == oldWidth && (height - y) == oldHeight);
  if (fullscreen && (newWidth > imageWidth || newHeight > imageHeight)) {
    // screen is larger than existing virtual size
    MARect srcRect;
    MAPoint2d dstPoint;
    MAHandle newImage = maCreatePlaceholder();
    int newImageWidth = max(newWidth, imageWidth);
    int newImageHeight = max(newHeight, imageHeight);

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.width = min(imageWidth, newImageWidth);
    srcRect.height = min(imageHeight, newImageHeight);
    dstPoint.x = 0;
    dstPoint.y = 0;

    maCreateDrawableImage(newImage, newImageWidth, newImageHeight);
    maSetDrawTarget(newImage);
    maSetColor(bg);
    maFillRect(0, 0, newImageWidth, newImageHeight);
    maDrawImageRegion(image, &srcRect, &dstPoint, TRANS_NONE);
    maDestroyPlaceholder(image);

    image = newImage;
    imageWidth = newImageWidth;
    imageHeight = newImageHeight;
    width = newWidth;
    height = newHeight;

    if (curY >= imageHeight) {
      curY = height - lineHeight;
      pageHeight = curY;
    }
    if (curX >= imageWidth) {
      curX = 0;
    }
  }
}

void Screen::setColor(long color) {
  fg = ansiToMosync(color);
}

void Screen::setTextColor(long foreground, long background) {
  bg = ansiToMosync(background);
  fg = ansiToMosync(foreground);
}

// handles the given escape character. Returns whether the font has changed
bool Screen::setGraphicsRendition(char c, int escValue, int lineHeight) {
  switch (c) {
  case 'K':
    maSetColor(bg);            // \e[K - clear to eol
    maFillRect(curX, curY, width - curX, lineHeight);
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
      reset();
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
      fg = ansiToMosync(0);
      break;
    case 31:                   // set red fg
      fg = ansiToMosync(4);
      break;
    case 32:                   // set green fg
      fg = ansiToMosync(2);
      break;
    case 33:                   // set yellow fg
      fg = ansiToMosync(6);
      break;
    case 34:                   // set blue fg
      fg = ansiToMosync(1);
      break;
    case 35:                   // set magenta fg
      fg = ansiToMosync(5);
      break;
    case 36:                   // set cyan fg
      fg = ansiToMosync(3);
      break;
    case 37:                   // set white fg
      fg = ansiToMosync(7);
      break;
    case 40:                   // set black bg
      bg = ansiToMosync(0);
      break;
    case 41:                   // set red bg
      bg = ansiToMosync(4);
      break;
    case 42:                   // set green bg
      bg = ansiToMosync(2);
      break;
    case 43:                   // set yellow bg
      bg = ansiToMosync(6);
      break;
    case 44:                   // set blue bg
      bg = ansiToMosync(1);
      break;
    case 45:                   // set magenta bg
      bg = ansiToMosync(5);
      break;
    case 46:                   // set cyan bg
      bg = ansiToMosync(3);
      break;
    case 47:                   // set white bg
      bg = ansiToMosync(15);
      break;
    case 48:                   // subscript on
      break;
    case 49:                   // superscript
      break;
    };
  }
  return false;
}

// updated the current font according to accumulated flags
void Screen::updateFont() {
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

  font = maFontLoadDefault(FONT_TYPE_MONOSPACE, style, fontSize);

  if (font == -1) {
    trace("maFontLoadDefault failed: style=%d size=%d", style, fontSize);
  } else {
    maFontSetCurrent(font);

    MAExtent extent = maGetTextSize("W");
    charWidth = EXTENT_X(extent);
    charHeight = 4 + EXTENT_Y(extent);
    trace("charWidth:%d charHeight:%d fontSize:%d %d", charWidth, charHeight, fontSize);
  }
}

AnsiWidget::AnsiWidget(ButtonListener *listener, int width, int height) :
  back(NULL),
  front(NULL),
  saved(NULL),
  width(width),
  height(height),
  touchX(-1),
  touchY(-1),
  moveTime(0),
  moveDown(false),
  touchMode(false),
  buttonListener(listener),
  activeLink(NULL) {
  for (int i = 0; i < MAX_SCREENS; i++) {
    screens[i] = NULL;
  }
  trace("width: %d height: %d", width, height);
}

bool AnsiWidget::construct() {
  bool result = false;
  back = new Screen(0, 0, width, height);
  if (back && back->construct()) {
    screens[0] = front = back;
    clearScreen();
    result = true;
  }
  return result;
}

// widget clean up
AnsiWidget::~AnsiWidget() {
  for (int i = 0; i < MAX_SCREENS; i++) {
    delete screens[i];
  }
}

// create audible beep sound
void AnsiWidget::beep() const {
  // http://www.mosync.com/documentation/manualpages/using-audio-api
}

// clear the offscreen buffer
void AnsiWidget::clearScreen() {
  back->clear();
  flush(true);
}

// draws the given image onto the offscreen buffer
void AnsiWidget::drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h) {
  back->drawInto();
  // TODO - draw image

  flush(false);
}

// draw a line onto the offscreen buffer
void AnsiWidget::drawLine(int x1, int y1, int x2, int y2) {
  back->drawInto();
  maLine(x1, y1, x2, y2);
  flush(false);
}

// draw a rectangle onto the offscreen buffer
void AnsiWidget::drawRect(int x1, int y1, int x2, int y2) {
  back->drawInto();
  maLine(x1, y1, x2, y1); // top
  maLine(x1, y2, x2, y2); // bottom
  maLine(x1, y1, x1, y2); // left
  maLine(x2, y1, x2, y2); // right
  flush(false);
}

// draw a filled rectangle onto the offscreen buffer
void AnsiWidget::drawRectFilled(int x1, int y1, int x2, int y2) {
  back->drawInto();
  maFillRect(x1, y1, x2 - x1, y2 - y1);
  flush(false);
}

// display and pending images changed
void AnsiWidget::flush(bool force, bool vscroll) {
  bool update = false;
  if (front == back) {
    if (force) {
      update = back->dirty;
    } else if (back->dirty) {
      update = (maGetMilliSecondCount() - back->dirty >= MAX_PENDING);
    }
    if (update) {
      front->draw(vscroll);
    }
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

  maGetImageData(back->image, &result, &rc, 1);
  return result[0];
}

// Returns the height in pixels using the current font setting
int AnsiWidget::textHeight(void) {
  return back->charHeight;
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
  int len = strlen(str);
  if (len) {
    back->drawInto();

    int lineHeight = textHeight();

    // copy the string to allow length manipulation
    char *buffer = new char[len + 1];
    strncpy(buffer, str, len);
    buffer[len] = 0;
    char *p = buffer;

    while (*p) {
      switch (*p) {
      case '\a':   // beep
        beep();
        break;
      case '\t':
        back->calcTab();
        break;
      case '\003': // end of text
        flush(true);
        break;
      case '\xC':
        clearScreen();
        break;
      case '\033': // ESC ctrl chars
        if (*(p + 1) == '[') {
          p += 2;
          while (doEscape(p, lineHeight)) {
            // continue
          }
        }
        break;
      case '\034':
        // file separator
        break;
      case '\n':   // new line
        back->newLine(lineHeight);
        break;
      case '\r':   // return
        back->curX = INITXY;     // erasing the line will clear any previous text
        break;
      default:
        p += back->print(p, lineHeight) - 1; // allow for p++
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
}

// update the widget to new dimensions
void AnsiWidget::resize(int newWidth, int newHeight) {
  int lineHeight = textHeight();
  for (int i = 0; i < MAX_SCREENS; i++) {
    if (screens[i]) {
      screens[i]->resize(newWidth, newHeight, width, height, lineHeight);
      if (screens[i] != back) {
        screens[i]->draw(false);
      }
    }
  }
  width = newWidth;
  height = newHeight;
  back->draw(false);
}

// sets the current drawing color
void AnsiWidget::setColor(long fg) {
  back->setColor(fg);
}

// sets the pixel to the given color at the given xy location
void AnsiWidget::setPixel(int x, int y, int c) {
  back->drawInto();
  maSetColor(ansiToMosync(c));
  maPlot(x, y);
}

// sets the current text drawing color
void AnsiWidget::setTextColor(long fg, long bg) {
  back->setTextColor(fg, bg);
}

// return whether any of the screens contain a hyperlink
bool AnsiWidget::hasUI() {
  bool result = false;
  for (int i = 0; i < MAX_SCREENS && !result; i++) {
    if (screens[i] != NULL && screens[i]->buttons.size() > 0) {
      result = true;
    }
  }
  return result;
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
  if (!OUTSIDE_RECT(event.point.x, event.point.y,
                    back->x, back->y,
                    back->width, back->height)) {
    touchX = event.point.x;
    touchY = event.point.y;

    Vector_each(Button*, it, back->buttons) {
      if ((*it)->overlaps(event.point, 0, back->scrollY)) {
        back->drawInto();
        activeLink = (*it);
        activeLink->pressed = true;
        activeLink->draw();
        flush(true);
        break;
      }
    }
  }
}

// handler for pointer move events
void AnsiWidget::pointerMoveEvent(MAEvent &event) {
  if (activeLink != NULL) {
    bool pressed = activeLink->overlaps(event.point, 0, back->scrollY);
    if (pressed != activeLink->pressed) {
      back->drawInto();
      activeLink->pressed = pressed;
      activeLink->draw();
      flush(true);
    }
  } else {
    // scroll up/down
    if (!OUTSIDE_RECT(event.point.x, event.point.y,
                      back->x, back->y,
                      back->width, back->height)) {
      int vscroll = back->scrollY + (touchY - event.point.y);
      int maxScroll = back->curY - (back->height - SCROLL_OFFS);
      if (vscroll < 0) {
        vscroll = 0;
      } else if (vscroll > maxScroll) {
        vscroll = maxScroll;
      }
      if (vscroll != back->scrollY && maxScroll > 0) {
        moveTime = maGetMilliSecondCount();
        moveDown = (back->scrollY < vscroll);
        back->drawInto();
        back->scrollY = vscroll;
        touchX = event.point.x;
        touchY = event.point.y;
        flush(true, true);
      }
    }
  }
}

// handler for pointer release events
void AnsiWidget::pointerReleaseEvent(MAEvent &event) {
  if (activeLink != NULL && activeLink->pressed) {
    back->drawInto();
    activeLink->pressed = false;
    activeLink->draw();
    flush(true);
    if (buttonListener) {
      buttonListener->buttonClicked(activeLink->action.c_str());
    }
  } else {
    int maxScroll = back->curY - (back->height - SCROLL_OFFS);
    if (touchY != -1 && maxScroll > 0) {
      back->drawInto();
      int start = maGetMilliSecondCount();
      if (start - moveTime < SWIPE_TRIGGER_SLOW) {
        // swiped
        MAEvent event;
        int elapsed = 0;
        int vscroll = back->scrollY;
        int scrollSize = (start - moveTime < SWIPE_TRIGGER_FAST) ? 
                         SWIPE_SCROLL_FAST : SWIPE_SCROLL_SLOW;
        int swipeStep = SWIPE_DELAY_STEP;

        while (elapsed < SWIPE_MAX_TIMER) {
          if (maGetEvent(&event) && event.type == EVENT_TYPE_POINTER_PRESSED) {
            scrollSize = 1;
          }
          elapsed += (maGetMilliSecondCount() - start);
          if (elapsed > swipeStep && scrollSize > 1) {
            // step down to a lesser scroll amount
            scrollSize -= 1;
            swipeStep += SWIPE_DELAY_STEP;
          }
          if (scrollSize == 1) {
            maWait(20);
          }
          vscroll += moveDown ? scrollSize : -scrollSize;
          if (vscroll < 0) {
            vscroll = 0;
          } else if (vscroll > maxScroll) {
            vscroll = maxScroll;
          }
          if (vscroll != back->scrollY) {
            back->dirty = true;
            back->scrollY = vscroll;
            flush(true, true);
          } else {
            break;
          }
        }
      }
      flush(true);
      moveTime = 0;
    }
  }
  touchX = touchY = -1;
  activeLink = NULL;
}

// creates a hyperlink, eg // ^[ hwww.foo.com|title;More text
void AnsiWidget::createLink(char *&p, bool execLink, bool button) {
  Vector<String *> *items = getItems(p);
  const char *action = items->size() > 0 ? (*items)[0]->c_str() : "";
  const char *text = items->size() > 1 ? (*items)[1]->c_str() : action;

  if (execLink && buttonListener) {
    buttonListener->buttonClicked(action);
  } else {
    MAExtent textSize = maGetTextSize(text);
    int w = EXTENT_X(textSize) + 2;
    int h = EXTENT_Y(textSize) + 2;
    int x = back->curX;
    int y = back->curY;
    
    if (button) {
      w += BUTTON_PADDING;
      h += BUTTON_PADDING;
      back->linePadding = BUTTON_PADDING;
    }
    if (back->curX + w >= width) {
      w = width - back->curX; // clipped
      back->newLine(EXTENT_Y(textSize));
    } else {
      back->curX += w;
    }
    Button *link;
    if (button) {
      link = new BlockButton(back, action, text, x, y, w, h);
    } else {
      link = new TextButton(back, action, text, x, y, w, h);
    }
    back->buttons.add(link);
    link->draw();
  }

  deleteItems(items);
}

// create an options dialog
void AnsiWidget::createOptionsBox(char *&p) {
  Vector<String *> *items = getItems(p);
  if (items->size()) {
    // calculate the size of the options buffer
    int optionsBytes = sizeof(int);
    Vector_each(String*, it, *items) {
      const char *str = (*it)->c_str();
      optionsBytes += (strlen(str) + 1) * sizeof(wchar);
    }

    // create the options buffer
    char *buffer = new char[optionsBytes];
    *(int *)buffer = items->size();
    wchar_t *dst = (wchar_t *)(buffer + sizeof(int));

    Vector_each(String*, it, *items) {
      const char *str = (*it)->c_str();
      int len = strlen(str);
      swprintf(dst, len + 1, L"%hs", str);
      dst[len] = 0;
      dst += (len + 1);
    }
    maOptionsBox(L"SmallBASIC", NULL, L"Close", (MAAddress)buffer, optionsBytes);
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
bool AnsiWidget::doEscape(char *&p, int textHeight) {
  int escValue = 0;

  while (isdigit(*p)) {
    escValue = (escValue *10) + (*p - '0');
    p++;
  }

  if (*p == ' ') {
    p++;
    switch (*p) {
    case 'A':
      showAlert(p);
      break;
    case 'B':
      createLink(p, false, true);
      break;
    case 'C':
      // GSS Graphic Size Selection
      back->fontSize = escValue;
      back->updateFont();
      break;
    case 'H':
      createLink(p, false, false);
      break;
    case 'h':
      createLink(p, true, false);
      break;
    case 'O':
      createOptionsBox(p);
      break;
    case 'X': // double buffering - transpose write and display screens
      swapScreens();
      break;
    case 'R': // remove a screen
      removeScreen(p);
      break;
    case 'S': // select new write and display screens
      if (selectScreen(p, true)) {
        front = back;
        flush(true);
      }
      break;
    case 's': // restore the saved write and display screens
      if (saved) {
        back = front = saved;
        back->drawInto();
        flush(true);
        saved = NULL;
      }
      break;
    case 'W': // select new write screen
      selectScreen(p, false);
      break;
    case 'w': // restore write screen
      if (front) {
        back = front;
      }
      break;
    }
  } else if (back->setGraphicsRendition(*p, escValue, textHeight)) {
    back->updateFont();
  }

  bool result = false;
  if (*p == ';') {
    result = true;
    p++;                        // next rendition
  }
  return result;
}

// returns list of strings extracted from the vertical-bar separated input string
Vector<String *> *AnsiWidget::getItems(char *&p) {
  Vector<String *> *result = new Vector<String *>();
  char *next = p + 1;
  bool eot = false;
  int index = 0;

  while (*p && !eot) {
    p++;
    switch (*p) {
    case '\034':
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

// remove the specified screen
void AnsiWidget::removeScreen(char *&p) {
  Vector<String *> *items = getItems(p);
  int n = items->size() > 0 ? atoi((*items)[0]->c_str()) : 0;
  if (n < 1 || n >= MAX_SCREENS) {
    print("ERR screen#");
  } else if (screens[n] != NULL) {
    if (back = screens[n]) {
      back = screens[0];
    }
    if (front = screens[n]) {
      front = screens[0];
    }
    delete screens[n];
    screens[n] = NULL;
  }
  deleteItems(items);
}

// select the specified screen
 bool AnsiWidget::selectScreen(char *&p, bool setSaved) {
  Vector<String *> *items = getItems(p);
  int n = items->size() > 0 ? atoi((*items)[0]->c_str()) : 0;
  int x = items->size() > 1 ? atoi((*items)[1]->c_str()) : 0;
  int y = items->size() > 2 ? atoi((*items)[2]->c_str()) : 0;
  int w = items->size() > 3 ? atoi((*items)[3]->c_str()) : width;
  int h = items->size() > 4 ? atoi((*items)[4]->c_str()) : height;

  bool result = true;
  flush(true);

  if (n < 0 || n >= MAX_SCREENS) {
    print("ERR screen#");
    result = false;
  } else if (screens[n] != NULL) {
    if (setSaved && back != screens[n]) {
      saved = back;
    }
    back = screens[n];
    back->drawInto();
  } else {
    if (setSaved) {
      saved = back;
    }
    back = new Screen(x, y, w, h);
    if (back && back->construct()) {
      screens[n] = back;
      back->drawInto();
      back->clear();
    } else {
      trace("Failed to create screen %d", n);
      result = false;
    }
  }

  deleteItems(items);
  return result;
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

// transpose the front and back screens
void AnsiWidget::swapScreens() {
  if (front == back) {
    if (screens[1] != NULL) {
      front = screens[1];
    } else {
      front = new Screen(0, 0, width, height);
      if (front && front->construct()) {
        screens[1] = front;
      } else {
        trace("Failed to create screen");
      }
    }
  } else {
    Screen *tmp = front;
    front = back;
    back = tmp;
    if (front->dirty) {
      front->draw(false);
    }
  }
}
