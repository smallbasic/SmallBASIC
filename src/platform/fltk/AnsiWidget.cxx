//
// Copyright 2001-2006 by Bill Spitzak and others.
// Original code Copyright Chris Warren-Smith.  Permission to distribute under
// the LGPL for the FLTK library granted by Chris Warren-Smith.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <fltk3/Image.h>
#include <fltk3/rgbImage.h>
#include <fltk3/draw.h>
#include <fltk3/Rectangle.h>
#include <fltk3/Group.h>
#include <fltk3/ask.h>

#if defined(WIN32)
#include <wingdi.h>
//extern HDC fl_bitmap_dc;
#else
#include <fltk3/x.h>
#endif

#define INITXY 2

#include "AnsiWidget.h"

using namespace fltk3;

/*! \class fltk3::AnsiWidget

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

static Color colors[] = {
  BLACK,                        // 0 black
  rgb_color(0, 0, 128),             // 1 blue
  rgb_color(0, 128, 0),             // 2 green
  rgb_color(0, 128, 128),           // 3 cyan
  rgb_color(128, 0, 0),             // 4 red
  rgb_color(128, 0, 128),           // 5 magenta
  rgb_color(128, 128, 0),           // 6 yellow
  rgb_color(192, 192, 192),         // 7 white
  rgb_color(128, 128, 128),         // 8 gray
  rgb_color(0, 0, 255),             // 9 light blue
  rgb_color(0, 255, 0),             // 10 light green
  rgb_color(0, 255, 255),           // 11 light cyan
  rgb_color(255, 0, 0),             // 12 light red
  rgb_color(255, 0, 255),           // 13 light magenta
  rgb_color(255, 255, 0),           // 14 light yellow
  WHITE                         // 15 bright white
};

// TODO: fixme
#define begin_offscreen()                       \
  initImage();                                

/*! Standard constructor for a widget.
 */
AnsiWidget::AnsiWidget(int x, int y, int w, int h, int defsize) : 
  Widget(x, y, w, h, 0) {
  labelsize(float (defsize));   // default size
  textsize(float (defsize));    // settable size
  init();
  reset();
  img = 0;
  resized = false;
}

/*! widget clean up
 */
AnsiWidget::~AnsiWidget() {
  destroyImage();
}

/*! clean up the offscreen buffer
 */
void AnsiWidget::destroyImage() {
  if (img) {
    //img->destroy();
    // TODO: fixme
    delete img;
    img = 0;
  }
}

/*! offscreen widget initialisation
  can only be called following Fl::check() or Fl::run()
*/
void AnsiWidget::initImage() {
  if (img == 0) {
    //img = new Image(w(), h());
    //    GSave gsave;
    // TODO: fixme
    //img->make_current();
    setcolor(color());
    rectf(0, 0, w(), h());
    setFont();
  }
}

/*! widget initialisation
 */
void AnsiWidget::init() {
  curY = INITXY;                // allow for input control border
  curX = INITXY;
  tabSize = 40;                 // tab size in pixels (160/32 = 5)
}

/*! reset the current drawing variables
 */
void AnsiWidget::reset() {
  curYSaved = 0;
  curXSaved = 0;
  invert = false;
  underline = false;
  bold = false;
  italic = false;
  color(WHITE);                 // bg
  labelcolor(BLACK);            // fg
  labelfont(COURIER);
  textsize(labelsize());
  setFont();
}

/*! handle resize changes
 */
void AnsiWidget::resize(int x, int y, int w, int h) {
  // TODO: fixme
  //  if (img && (layout_damage() & LAYOUT_WH)) {
  //    // can't use GSave here in X
  //    resized = true;
  //  }
  //  Widget::layout();
}

/*! output the offscreen buffer
 */
void AnsiWidget::draw() {
  // ensure this widget has lowest z-order
  int siblings = parent()->children();
  for (int n = 0; n < siblings; n++) {
    Widget *w = parent()->child(n);
    if (w != this) {
      w->redraw();
    }
  }
  if (img) {
    if (resized) {
      int W = img->w();
      int H = img->h();
      if (w() > W) {
        W = w();
      }
      if (h() > H) {
        H = h();
      }
      // TODO: fixme
      // Image *old = img;
      //img = new Image(W, H);
      //GSave gsave;
      //img->make_current();
      setcolor(color());
      setFont();
      rectf(0, 0, W, H);
      //old->draw(Rectangle(old->w(), old->h()));
      //old->destroy();
      //delete old;
      resized = false;
    }
    push_clip(0, 0, w(), h());
    //img->draw(Rectangle(img->w(), img->h()));
    pop_clip();
  } else {
    setcolor(color());
    rectf(0, 0, w(), h());
  }
}

/*! clear the offscreen buffer
 */
void AnsiWidget::clearScreen() {
  if (img != 0) {
    init();
    begin_offscreen();
    setcolor(color());
    rectf(0, 0, w(), h());
    redraw();
  }
}

/*! sets the current text drawing color
 */
void AnsiWidget::setTextColor(long fg, long bg) {
  labelcolor(ansiToFltk(fg));
  color(ansiToFltk(bg));
}

/*! sets the current drawing color
 */
void AnsiWidget::setColor(long fg) {
  labelcolor(ansiToFltk(fg));
}

/*! draw a line onto the offscreen buffer
 */
void AnsiWidget::drawLine(int x1, int y1, int x2, int y2) {
  begin_offscreen();
  setcolor(labelcolor());
  drawline(x1, y1, x2, y2);
  redraw();
}

/*! draw a filled rectangle onto the offscreen buffer
 */
void AnsiWidget::drawRectFilled(int x1, int y1, int x2, int y2) {
  begin_offscreen();
  setcolor(labelcolor());
  rectf(x1, y1, x2 - x1, y2 - y1);
  redraw();
}

/*! draw a rectangle onto the offscreen buffer
 */
void AnsiWidget::drawRect(int x1, int y1, int x2, int y2) {
  begin_offscreen();
  setcolor(labelcolor());
  drawline(x1, y1, x1, y2);
  drawline(x1, y2, x2, y2);
  drawline(x2, y2, x2, y1);
  drawline(x2, y1, x1, y1);
  redraw();
}

/*! draws the given image onto the offscreen buffer
 */
void AnsiWidget::drawImage(Image *image, int x, int y, int sx, int sy, int width, int height) {
  begin_offscreen();
  // TODO: fixme
  //  image->draw(Rectangle(sx, sy, width, height), // from
  //              Rectangle(x, y, width, height));  // to
  redraw();
}

/*! save the offscreen buffer to the given filename
 */
void AnsiWidget::saveImage(const char *filename, int x, int y, int width, int height) {
  if (width == 0) {
    width = w();
  }
  if (height == 0) {
    height = h();
  }
  uchar *pixels = (uchar *) malloc(width * height * 3);
  begin_offscreen();
  // TODO: fixme
  //readimage(pixels, RGB, Rectangle(x, y, width, height));
  //fltk3::rgbImage jpg(pixels, RGB, width, height);
  // jpg.write_jpeg(filename);
  // TODO: re-enable jpeg write function
  free((void *)pixels);
}

/*! sets the pixel to the given color at the given xy location
 */
void AnsiWidget::setPixel(int x, int y, int c) {
  begin_offscreen();
#if defined(WIN32)
  if (c < 0) {
    // TODO: fixme ::SetPixel(fl_bitmap_dc, x, y, -c);
  } else {
    setcolor(ansiToFltk(c));
    drawpoint(x, y);
  }
#else
  setcolor(ansiToFltk(c));
  drawpoint(x, y);
#endif
  redraw();
}

/*! returns the color of the pixel at the given xy location
 */
int AnsiWidget::getPixel(int x, int y) {
#if USE_X11
  XImage *image = XGetImage(fltk3::xdisplay, xwindow, x, y, 1, 1, AllPlanes, ZPixmap);
  if (image) {
    int color = XGetPixel(image, 0, 0);
    XDestroyImage(image);
    return -color;
  }
#elif defined(_WIN32)
  begin_offscreen();
  // needs to return a -ve number to distiguish from basic 16 color values
  // unpacked in later calls to ansiToFltk()
  // TODO: fixme return -int (::GetPixel(fl_bitmap_dc, x, y));
#elif defined(__APPLE__)
  // TODO !
#endif
  return 0;
}

/*! create audible beep sound
 */
void AnsiWidget::beep() const {
  fltk3::beep(fltk3::BEEP_MESSAGE);
}

/*! Returns the width in pixels using the current font setting
 */
int AnsiWidget::textWidth(const char *s) {
  begin_offscreen();
  setFont();
  return (int)getwidth(s);
}

/*! Returns the height in pixels using the current font setting
 */
int AnsiWidget::textHeight(void) {
  begin_offscreen();
  setFont();
  return (int)(getascent() + getdescent());
}

/*! callback for scrollrect
 */
void eraseBottomLine(void *data, const fltk3::Rectangle &r) {
  AnsiWidget *out = (AnsiWidget *) data;
  setcolor(out->color());
  fillrect(r);
}

/*! Handles the \n character
 */
void AnsiWidget::newLine() {
  int height = h();
  int fontHeight = (int)(getascent() + getdescent());

  curX = INITXY;
  if (curY + (fontHeight * 2) >= height) {
    // TODO: fixme
    //scrollrect(Rectangle(w(), height), 0, -fontHeight, eraseBottomLine, this);
  } else {
    curY += fontHeight;
  }
}

/*! Calculate the pixel movement for the given cursor position
 */
int AnsiWidget::calcTab(int x) const {
  int c = 1;
  while (x > tabSize) {
    x -= tabSize;
    c++;
  }
  return c * tabSize;
}

/*! Converts ANSI colors to FLTK colors
 */
Color AnsiWidget::ansiToFltk(long c) {
  if (c < 0) {
    // assume color is windows style RGB packing
    // RGB(r,g,b) ((COLORREF)((BYTE)(r)|((BYTE)(g) << 8)|((BYTE)(b) << 16)))
    c = -c;
    int r = (c >> 16) & 0xFF;
    int g = (c >> 8) & 0xFF;
    int b = (c) & 0xFF;
    return fltk3::rgb_color(r, g, b);
  }

  return (c > 16) ? WHITE : colors[c];
}

/*! Handles the given escape character. Returns whether the font has changed
 */
bool AnsiWidget::setGraphicsRendition(char c, int escValue) {
  switch (c) {
  case 'K':                    // \e[K - clear to eol
    setcolor(color());
    rectf(curX, curY, w() - curX, (int)(getascent() + getdescent()));
    break;
  case 'G':                    // move to column
    curX = escValue;
    break;
  case 'T':                    // non-standard: move to n/80th of screen width
    curX = escValue * w() / 80;
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
      labelcolor(ansiToFltk(0));
      break;
    case 31:                   // set red fg
      labelcolor(ansiToFltk(4));
      break;
    case 32:                   // set green fg
      labelcolor(ansiToFltk(2));
      break;
    case 33:                   // set yellow fg
      labelcolor(ansiToFltk(6));
      break;
    case 34:                   // set blue fg
      labelcolor(ansiToFltk(1));
      break;
    case 35:                   // set magenta fg
      labelcolor(ansiToFltk(5));
      break;
    case 36:                   // set cyan fg
      labelcolor(ansiToFltk(3));
      break;
    case 37:                   // set white fg
      labelcolor(ansiToFltk(7));
      break;
    case 40:                   // set black bg
      color(ansiToFltk(0));
      break;
    case 41:                   // set red bg
      color(ansiToFltk(4));
      break;
    case 42:                   // set green bg
      color(ansiToFltk(2));
      break;
    case 43:                   // set yellow bg
      color(ansiToFltk(6));
      break;
    case 44:                   // set blue bg
      color(ansiToFltk(1));
      break;
    case 45:                   // set magenta bg
      color(ansiToFltk(5));
      break;
    case 46:                   // set cyan bg
      color(ansiToFltk(3));
      break;
    case 47:                   // set white bg
      color(ansiToFltk(15));
      break;
    case 48:                   // subscript on
      break;
    case 49:                   // superscript
      break;
    };
  }
  return false;
}

/*! Handles the characters following the \e[ sequence. Returns whether a further call
 * is required to complete the process.
 */
bool AnsiWidget::doEscape(unsigned char *&p) {
  int escValue = 0;

  while (isdigit(*p)) {
    escValue = (escValue * 10) + (*p - '0');
    p++;
  }

  if (*p == ' ') {
    p++;
    switch (*p) {
    case 'C':
      // GSS Graphic Size Selection
      textsize(escValue);
      setFont();
      break;
    }
  } else if (setGraphicsRendition(*p, escValue)) {
    setFont();
  }

  if (*p == ';') {
    p++;                        // next rendition
    return true;
  }
  return false;
}

/*! Prepares to display text according to accumulated flags
 */
void AnsiWidget::setFont() {
  //  fltk3::Font *font = labelfont();
  //  if (bold) {
  //    font = font->bold();
  //  }
  //  if (italic) {
  //    font = font->italic();
  //  }
  //  setfont(font, textsize());
  // TODO: fixme
}

/*! Prints the contents of the given string onto the backbuffer
 */
void AnsiWidget::print(const char *str) {
  int len = strlen(str);
  if (len <= 0) {
    return;
  }

  begin_offscreen();
  setFont();
  int ascent = (int)getascent();
  int fontHeight = (int)(ascent + getdescent());
  unsigned char *p = (unsigned char *)str;

  while (*p) {
    switch (*p) {
    case '\a':                 // beep
      beep();
      break;
    case '\t':
      curX = calcTab(curX + 1);
      break;
    case '\xC':
      init();
      setcolor(color());
      rectf(0, 0, w(), h());
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
      curX = INITXY;
      setcolor(color());
      rectf(0, curY, w(), fontHeight);
      break;
    default:
      int numChars = 1;         // print minimum of one character
      int cx = (int)getwidth((const char *)p, 1);
      int width = w() - 1;

      if (curX + cx >= width) {
        newLine();
      }
      // print further non-control, non-null characters
      // up to the width of the line
      while (p[numChars] > 31) {
        cx += (int)getwidth((const char *)p + numChars, 1);
        if (curX + cx < width) {
          numChars++;
        } else {
          break;
        }
      }

      if (invert) {
        setcolor(labelcolor());
        rectf(curX, curY, cx, fontHeight);
        setcolor(color());
        drawtext((const char *)p, numChars, float (curX), float (curY + ascent));
      } else {
        setcolor(color());
        rectf(curX, curY, cx, fontHeight);
        setcolor(labelcolor());
        drawtext((const char *)p, numChars, float (curX), float (curY + ascent));
      }

      if (underline) {
        drawline(curX, curY + ascent + 1, curX + cx, curY + ascent + 1);
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

  redraw();
}

/*!
 */
int AnsiWidget::handle(int e) {
  if (e == FOCUS) {
    return 2;
  }
  return Widget::handle(e);
}

