// This file is part of SmallBASIC
//
// Copyright(C) 2001-2016 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "platform/web/canvas.h"

const char *colors[] = {
  "#0",      // 0 black
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
  "#fff"     // 15 bright white
};

#define DEFAULT_FOREGROUND -0xa1a1a1
#define DEFAULT_BACKGROUND 0

Canvas::Canvas() :
  _html(),
  _script(),
  _bg(),
  _fg(),
  _invert(false),
  _underline(false),
  _bold(false),
  _italic(false),
  _spanLevel(false),
  _curx(0),
  _cury(0) {
  _bgBody = getColor(DEFAULT_BACKGROUND);
  _fgBody = getColor(DEFAULT_FOREGROUND);
}

String Canvas::getPage() {
  String result;
  result.append("<!DOCTYPE HTML><html><head><style>")
    .append(" body { margin: 0px; padding: 2px; font-family: monospace;")
    .append("  background-color:").append(_bgBody).append(";")
    .append("  color:").append(_fgBody).append(";}\n")
    .append(" span.underline { text-decoration: underline; }\n")
    .append(" span.bold { font-weight: bold; }\n")
    .append(" span.italic { text-style: italic; }\n")
    .append(" #_canvas { position:absolute; left:0px; top:0px;} ")
    .append("\n</style><title>SmallBASIC</title></head><body>")
    .append("<canvas id='_canvas'></canvas>\n")
    .append("<script type=text/javascript>\n")
    .append("var canvas = document.getElementById('_canvas');\n")
    .append("canvas.width = window.innerWidth;\n")
    .append("canvas.height = window.innerHeight;\n")
    .append("var ctx = canvas.getContext('2d');\n")
    .append("ctx.textBaseline = 'hanging';\n")
    .append("ctx.textBaseline = 'hanging';\n")
    .append("ctx.font = '14pt courier';\n")
    .append("var m = ctx.measureText('Q');\n")
    .append("function t(s, x, y) {\n")
    .append("  ctx.fillText(s, x*m.width, y*20);\n")
    .append("}\n")
    .append("function ln(x1, y1, x2, y2, c) {\n")
    .append("  ctx.beginPath();\n")
    .append("  ctx.moveTo(x1,y1);\n")
    .append("  ctx.lineTo(x2,y2);\n")
    .append("  ctx.lineWidth = 1;\n")
    .append("  ctx.strokeStyle=c;\n")
    .append("  ctx.stroke();\n")
    .append("}\n")
    .append("function rcf(x1, y1, x2, y2, c) {\n")
    .append("  ctx.beginPath();\n")
    .append("  ctx.rect(x1, y1, x2, y2);\n")
    .append("  ctx.fillStyle=c;\n")
    .append("  ctx.fill();\n")
    .append("}\n")
    .append("function rc(x1, y2, x2, y2, c) {\n")
    .append("  ctx.beginPath();\n")
    .append("  ctx.rect(x1, y1, x2, y2);\n")
    .append("  ctx.strokeStyle=c;\n")
    .append("  ctx.stroke();\n")
    .append("}\n")
    .append(_script)
    .append("</script>\n")
    .append(_html);
  for (int i = 0; i < _spanLevel; i++) {
    result.append("</span>");
  }
  result.append("</body></html>");
  return result;
}

void Canvas::clearScreen() {
  _html.empty();
  _script.empty();
  _spanLevel = 0;
  _curx = _cury = 0;
  _bgBody = _bg;
  _fgBody = _fg;
}

void Canvas::reset() {
  resetStyle();
  clearScreen();
}

void Canvas::setTextColor(long fg, long bg) {
  _fg = getColor(fg);
  _bg = getColor(bg);
}

void Canvas::setColor(long fg) {
  _fg = getColor(fg);
}

//http://stackoverflow.com/questions/7812514/drawing-a-dot-on-html5-canvas
void Canvas::setPixel(int x, int y, int c) {
  _script.append("ctx.fillRect(").append(x).append(",").append(y).append(",1,1);");
}

void Canvas::setXY(int x, int y) {
  _curx = x;
  _cury = y;
}

void Canvas::drawLine(int x1, int y1, int x2, int y2) {
  _script.append("ln(")
    .append(x1).append(",")
    .append(y1).append(",")
    .append(x2).append(",")
    .append(y2).append(",'")
    .append(_fg).append("');\n");
}

void Canvas::drawRectFilled(int x1, int y1, int x2, int y2) {
  _script.append("rcf(")
    .append(x1).append(",")
    .append(y1).append(",")
    .append(x2).append(",")
    .append(y2).append(",'")
    .append(_fg).append("');\n");
}

void Canvas::drawRect(int x1, int y1, int x2, int y2) {
  _script.append("rc(")
    .append(x1).append(",")
    .append(y1).append(",")
    .append(x2).append(",")
    .append(y1).append(",'")
    .append(_fg).append("');\n");
}

/*! Prints the contents of the given string onto the backbuffer
 */
void Canvas::print(const char *str) {
  unsigned char *p = (unsigned char*)str;
  while (*p) {
    switch (*p) {
    case '\a':
      break;
    case '\xC':
      resetStyle();
      break;
    case '\033':
      // ESC ctrl chars
      if (*(p+1) == '[' ) {
        p += 2;
        String bg = _bg;
        String fg = _fg;
        bool bold = _bold;
        bool italic = _italic;
        while (doEscape(p)) {
          // continue
        }
        if (!bg.equals(_bg) || !fg.equals(_fg)) {
          if (_bg.equals(_bgBody) && _fg.equals(_fgBody)) {
            printEndSpan();
          } else {
            printColorSpan(_bg, _fg);
          }
        }
        if (_bold != bold) {
          if (!_bold) {
            printEndSpan();
          } else {
            printSpan("bold");
          }
        }
        if (_italic != italic) {
          if (!_italic) {
            printEndSpan();
          } else {
            printSpan("italic");
          }
        }
      }
      break;
    case '\n':
      // new line
      newLine();
      break;
    case '\r':
      _curx = 0;
      break;
    default:
      int numChars = 1;
      while (p[numChars] > 31) {
        _curx++;
        numChars++;
      }

      if (_invert) {
        printColorSpan(_fg, _bg);
      }
      if (_underline) {
        printSpan("underline");
      }
      drawText((const char *)p, numChars);
      if (_invert) {
        printEndSpan();
      }
      if (_underline) {
        printEndSpan();
      }

      // advance, allow for p++
      p += numChars - 1;
    };
    if (*p == '\0') {
      break;
    }
    p++;
  }
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

  setGraphicsRendition(*p, escValue);

  if (*p == ';') {
    p++; // next rendition
    return true;
  }
  return false;
}

void Canvas::drawText(const char *str, int len, bool canvas) {
  if (canvas) {
    _script.append("t('").append(str, len).append("', ")
      .append(_curx).append(", ").append(_cury).append(");\n");
  } else {
    _html.append(str, len);
  }
}

String Canvas::getColor(long c) {
  String result;
  if (c < 0) {
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

/*! Handles the \n character
 */
void Canvas::newLine() {
  _html.append("<br/>");
  _cury++;
  _curx = 0;
}

void Canvas::printColorSpan(String &bg, String &fg) {
  _spanLevel++;
  _html.append("<span style='background-color:")
    .append(bg).append("; color:").append(fg).append("'>");
}

void Canvas::printEndSpan() {
  if (_spanLevel) {
    _spanLevel--;
    _html.append("</span>");
  }
}

void Canvas::printSpan(const char *clazz) {
  _spanLevel++;
  _html.append("<span class=").append(clazz).append(">");
}

void Canvas::resetStyle() {
  for (int i = 0; i < _spanLevel; i++) {
    _html.append("</span>");
  }
  _spanLevel = 0;
  _invert = false;
  _underline = false;
  _bold = false;
  _italic = false;
  _bg = _bgBody;
  _fg = _fgBody;
}

/*! Handles the given escape character. Returns whether the style has changed
 */
void Canvas::setGraphicsRendition(char c, int escValue) {
  switch (c) {
  case ';': // fallthru
  case 'm': // \e[...m  - ANSI terminal
    switch (escValue) {
    case 0:  // reset
      resetStyle();
      break;
    case 1: // set bold on
      _bold = true;
      break;
    case 2: // set faint on
      break;
    case 3: // set italic on
      _italic = true;
      break;
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
      break;
    case 23:
      _italic = false;
      break;
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
}
