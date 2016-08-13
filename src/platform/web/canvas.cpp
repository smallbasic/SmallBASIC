// This file is part of SmallBASIC
//
// Copyright(C) 2001-2016 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "platform/web/canvas.h"

// - write well formed HTML and markup for font, colours etc
// - pass web args to command$
// fixed font

//http://stackoverflow.com/questions/7812514/drawing-a-dot-on-html5-canvas
// http://www.html5canvastutorials.com/tutorials/html5-canvas-lines/
// http://www.html5canvastutorials.com/tutorials/html5-canvas-rectangles/

const char *colors[] = {
  "black",   // 0 black
  "#000080", // 1 blue
  "#008000", // 2 green
  "#008080", // 3 cyan
  "#800000", // 4 red
  "#800080", // 5 magenta
  "#800800", // 6 yellow
  "#c0c0c0", // 7 white
  "#808080", // 8 gray
  "#0000ff", // 9 light blue
  "#00ff00", // 10 light green
  "#00ffff", // 11 light cyan
  "#ff0000", // 12 light red
  "#ff00ff", // 13 light magenta
  "#ffff00", // 14 light yellow
  "white"    // 15 bright white
};

Canvas::Canvas() :
  _html(), _bg(), _fg(),
  _invert(false),
  _underline(false),
  _bold(false),
  _italic(false),
  _curx(0),
  _cury(0) {
}

void Canvas::clearScreen() {
  _html.empty();
}

void Canvas::reset() {
  _invert = false;
  _underline = false;
  _bold = false;
  _italic = false;
  _bg = colors[15];
  _fg = colors[0];
}

void Canvas::setTextColor(long fg, long bg) {
  //labelcolor(getColor(fg));
  //color(getColor(bg)));
}

void Canvas::setColor(long fg) {
  //labelcolor(getColor(fg);
}

void Canvas::setPixel(int x, int y, int c) {
}

void Canvas::setXY(int x, int y) {
}

void Canvas::drawLine(int x1, int y1, int x2, int y2) {
  //setcolor(labelcolor();
  //drawline(x1, y1, x2, y2);
}

void Canvas::drawRectFilled(int x1, int y1, int x2, int y2) {
  //setcolor(labelcolor();
  //fillrect(Rectangle(x1, y1, x2-x1, y2-y1));
}

void Canvas::drawRect(int x1, int y1, int x2, int y2) {
  //setcolor(labelcolor());
  //drawline(x1, y1, x1, y2);
  //drawline(x1, y2, x2, y2);
  //drawline(x2, y2, x2, y1);
  //drawline(x2, y1, x1, y1);
}

/*! Handles the \n character
 */
void Canvas::newLine() {
  _html.append("<br/>");
}

String Canvas::getColor(long c) {
  String result;
  if (c < 0) {
    // assume color is windows style RGB packing
    // RGB(r,g,b) ((COLORREF)((BYTE)(r)|((BYTE)(g) << 8)|((BYTE)(b) << 16)))
    c = -c;
    int b = (c>>16) & 0xFF;
    int g = (c>>8) & 0xFF;
    int r = (c) & 0xFF;
    char buf[8];
    sprintf(buf, "#%x%x%x", b, g, r);
    result.append(buf);
  } else {
    result.append((colors[c > 15 ? 15 : c]));
  }
  return result;
}

/*! Handles the given escape character. Returns whether the font has changed
 */
bool Canvas::setGraphicsRendition(char c, int escValue) {
  switch (c) {
  case ';': // fallthru
  case 'm': // \e[...m  - ANSI terminal
    switch (escValue) {
    case 0:  // reset
      reset();
      break;
    case 1: // set bold on
      _bold = true;
      return true;
    case 2: // set faint on
      break;
    case 3: // set italic on
      _italic = true;
      return true;
    case 4: // set underline on
      _underline = true;
      break;
    case 5: // set blink on
      break;
    case 6: // rapid blink on
      break;
    case 7: // reverse video on
      _invert = true;
      break;
    case 8: // conceal on
      break;
    case 21: // set bold off
      _bold = false;
      return true;
    case 23:
      _italic = false;
      return true;
    case 24: // set underline off
      _underline = false;
      break;
    case 27: // reverse video off
      _invert = false;
      break;
      // colors - 30..37 foreground, 40..47 background
    case 30: // set black fg
      _fg = getColor(0);
      break;
    case 31: // set red fg
      _fg = getColor(4);
      break;
    case 32: // set green fg
      _fg = getColor(2);
      break;
    case 33: // set yellow fg
      _fg = getColor(6);
      break;
    case 34: // set blue fg
      _fg = getColor(1);
      break;
    case 35: // set magenta fg
      _fg = getColor(5);
      break;
    case 36: // set cyan fg
      _fg = getColor(3);
      break;
    case 37: // set white fg
      _fg = getColor(7);
      break;
    case 40: // set black bg
      _bg = getColor(0);
      break;
    case 41: // set red bg
      _bg = getColor(4);
      break;
    case 42: // set green bg
      _bg = getColor(2);
      break;
    case 43: // set yellow bg
      _bg = getColor(6);
      break;
    case 44: // set blue bg
      _bg = getColor(1);
      break;
    case 45: // set magenta bg
      _bg = getColor(5);
      break;
    case 46: // set cyan bg
      _bg = getColor(3);
      break;
    case 47: // set white bg
      _bg = getColor(15);
      break;
    case 48: // subscript on
      break;
    case 49: // superscript
      break;
    };
  }
  return false;
}

/*! Handles the characters following the \e[ sequence. Returns whether a further call
 * is required to complete the process.
 */
bool Canvas::doEscape(unsigned char* &p) {
  int escValue = 0;

  while (isdigit(*p)) {
    escValue = (escValue * 10) + (*p - '0');
    p++;
  }

  if (setGraphicsRendition(*p, escValue)) {
    //setFont();
  }

  if (*p == ';') {
    p++; // next rendition
    return true;
  }
  return false;
}

/*! Prints the contents of the given string onto the backbuffer
 */
void Canvas::print(const char *str) {
  //  setFont();
  unsigned char *p = (unsigned char*)str;
  while (*p) {
    switch (*p) {
    case '\a':
      break;
    case '\xC':
      reset();
      //setcolor(color());
      //fillrect(Rectangle(w(), h()));
      break;
    case '\033':
      // ESC ctrl chars
      if (*(p+1) == '[' ) {
        p += 2;
        while (doEscape(p)) {
          // continue
        }
      }
      break;
    case '\n':
      // new line
      newLine();
      break;
    case '\r':
      break;
    default:
      int numChars = 1; // print minimum of one character
      // print further non-control, non-null characters
      // up to the width of the line
      while (p[numChars] > 31) {
        numChars++;
      }

      if (_invert) {
        //setcolor(labelcolor());
        //fillrect(curX, curY, cx, fontHeight);
        //setcolor(color());
        //drawtext((const char*)p, numChars, float(curX), float(curY+ascent));
      } else {
        //setcolor(color());
        //fillrect(curX, curY, cx, fontHeight);
        //setcolor(labelcolor());
        //drawtext((const char*)p, numChars, float(curX), float(curY+ascent));
      }

      if (_underline) {
        //drawline(curX, curY+ascent+1, curX+cx, curY+ascent+1);
      }

      // advance, allow for p++
      p += numChars-1;
    };

    if (*p == '\0') {
      break;
    }
    p++;
  }
}
