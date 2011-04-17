// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <QApplication>
#include <QRect>
#include <QPaintEvent>

#include <stdio.h>
#include "ansiwidget.h"

/*! \class AnsiWidget
 
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

#define INITXY 2

static QColor colors[] = {
  Qt::black,        // 0 black
  Qt::darkBlue,     // 1 blue
  Qt::darkGreen,    // 2 green
  Qt::darkCyan,     // 3 cyan
  Qt::darkRed,      // 4 red
  Qt::darkMagenta,  // 5 magenta
  Qt::darkYellow,   // 6 yellow
  Qt::lightGray,    // 7 white
  Qt::gray,         // 8 gray
  Qt::blue,         // 9 light blue
  Qt::green,        // 10 light green
  Qt::cyan,         // 11 light cyan
  Qt::red,          // 12 light red
  Qt::magenta,      // 13 light magenta
  Qt::yellow,       // 14 light yellow
  Qt::white         // 15 bright white
};

AnsiWidget::AnsiWidget(QWidget *parent) : QWidget(parent), img(0) {
  reset(true);
  initImage();
}

/*! widget clean up
 */
AnsiWidget::~AnsiWidget() {
  destroyImage();
}

/*! create audible beep sound
 */
void AnsiWidget::beep() const {
  QApplication::beep();
}

/*! clear the offscreen buffer
 */
void AnsiWidget::clearScreen() {
  reset(false);
  QPainter painter(this->img);
  painter.fillRect(this->geometry(), this->bg);
  update();
}

/*! draws an arc onto the offscreen buffer
 */
void AnsiWidget::drawArc(int xc, int yc, double r, 
                         double start, double end, double aspect) {
  QPainter painter(this->img);
  update();
}

/*! draws an ellipse onto the offscreen buffer
 */
void AnsiWidget::drawEllipse(int xc, int yc, int xr, int yr, 
                             double aspect, int fill) {
  QPainter painter(this->img);
  update();
}

/*! draws the given image onto the offscreen buffer
 */
void AnsiWidget::drawImage(QImage* image, int x, int y, int sx, int sy, int w, int h) {
  QPainter painter(this->img);
  painter.drawImage(x, y, *image, sx, sy, w, h); 
}

/*! draw a line onto the offscreen buffer
 */
void AnsiWidget::drawLine(int x1, int y1, int x2, int y2) {
  QPainter painter(this->img);
  painter.setPen(this->fg);
  painter.drawLine(x1, y1, x2, y2);
}

/*! draw a rectangle onto the offscreen buffer
 */
void AnsiWidget::drawRect(int x1, int y1, int x2, int y2) {
  QPainter painter(this->img);
  painter.setPen(this->fg);
  painter.drawRect(x1, y1, x2-x1, y2-y1);
}

/*! draw a filled rectangle onto the offscreen buffer
 */
void AnsiWidget::drawRectFilled(int x1, int y1, int x2, int y2) {
  QPainter painter(this->img);
  painter.fillRect(x1, y1, x2-x1, y2-y1, this->fg);
}

/*! returns the color of the pixel at the given xy location
 */
QRgb AnsiWidget::getPixel(int x, int y) {
  return img->copy(x, y, 1, 1).toImage().pixel(0, 0);
}

/*! Returns the height in pixels using the current font setting
 */
int AnsiWidget::textHeight(void) {
  QFontMetrics fm = fontMetrics();
  return fm.ascent() + fm.descent();
}

/*! Returns the width in pixels using the current font setting
 */
int AnsiWidget::textWidth(const char* s) {
  QFontMetrics fm = fontMetrics();
  return fm.width(s);
}

/*! Prints the contents of the given string onto the backbuffer
 */
void AnsiWidget::print(const char *str) {
  int len = strlen(str);
  if (len <= 0) {
    return;
  }

  QFontMetrics fm = fontMetrics();
  int ascent = fm.ascent();
  int fontHeight = fm.ascent() + fm.descent();
  unsigned char *p = (unsigned char*)str;

  while (*p) {
    switch (*p) {
    case '\a':   // beep
      beep();
      break;
    case '\t':
      curX = calcTab(curX+1);
      break;
    case '\xC':
      {
        reset(false);
        QPainter painter(this->img);
        painter.fillRect(this->geometry(), this->bg);
      }
      break;
    case '\033':  // ESC ctrl chars
      if (*(p+1) == '[' ) {
        p += 2;
        while (doEscape(p)) {
          // continue
        }
      }
      break;
    case '\n': // new line
      newLine();
      break;
    case '\r': // return
      {
        curX = INITXY;
        QPainter painter(this->img);
        painter.fillRect(0, curY, width(), fontHeight, this->bg);
      }
      break;
    default:
      int numChars = 1; // print minimum of one character
      int cx = fontMetrics().width((const char*) p, 1);
      int w = width() - 1;

      if (curX + cx >= w) {
        newLine();
      }

      // print further non-control, non-null characters 
      // up to the width of the line
      while (p[numChars] > 31) {
        cx += fontMetrics().width((const char*) p + numChars, 1);
        if (curX + cx < w) {
          numChars++;
        } 
        else {
          break;
        }
      }

      QPainter painter(this->img);
      painter.setFont(font());
      painter.setBackground(invert ? this->fg : this->bg);
      painter.setPen(invert ? this->bg : this->fg);
      painter.fillRect(curX, curY, cx, fontHeight, invert ? this->fg : this->bg);
      painter.drawText(curX, curY + ascent, QString::fromUtf8((const char*)p, numChars));

      if (underline) {
        painter.drawLine(curX, curY+ascent+1, curX+cx, curY+ascent+1);
      }
            
      // advance
      p += numChars-1; // allow for p++ 
      curX += cx;
    };
        
    if (*p == '\0') {
      break;
    }
    p++;
  }

  update();
}

/*! save the offscreen buffer to the given filename
 */
void AnsiWidget::saveImage(const char* filename, int x, int y, int w, int h) {
  if (w == 0) {
    w = width();
  }
  if (h == 0) {
    h = height();
  }
  
  img->copy(x, y, w, h).save(filename);
}

/*! sets the current drawing color
 */
void AnsiWidget::setColor(long fg) {
  this->fg = ansiToQt(fg);
}

/*! sets the pixel to the given color at the given xy location
 */
void AnsiWidget::setPixel(int x, int y, int c) {
  QPainter painter(this->img);
  painter.setPen(c);
  painter.drawPoint(x, y);
  update(x, y, 1, 1);
}

/*! sets the current text drawing color
 */
void AnsiWidget::setTextColor(long fg, long bg) {
  this->bg = ansiToQt(bg);
  this->fg = ansiToQt(fg);
}

/*! sets mouse mode on or off
 */
void AnsiWidget::setMouseMode(bool flag) {
  mouseMode = flag;
  setMouseTracking(flag);
}

/*! resets mouse mode to false
 */
void AnsiWidget::resetMouse() {
  pointX = pointY = markX = markY = 0;
  mouseMode = false;
  setMouseTracking(false);
}

/*! public slot - copy selected text to the clipboard
 */
void AnsiWidget::copySelection() {
}

/*! public slot - find the next text item
 */
void AnsiWidget::findNextText() {
}

/*! public slot - find text
 */
void AnsiWidget::findText() {
}

/*! public slot - select all text
 */
void AnsiWidget::selectAll() {
}

/*! Converts ANSI colors to FLTK colors
 */
QColor AnsiWidget::ansiToQt(long c) {
  if (c < 0) {
    // assume color is windows style RGB packing
    // RGB(r,g,b) ((COLORREF)((BYTE)(r)|((BYTE)(g) << 8)|((BYTE)(b) << 16)))
    c = -c;
    int r = (c>>16) & 0xFF;
    int g = (c>>8) & 0xFF;
    int b = (c) & 0xFF;
    return QColor(r, g, b);
  }

  return (c > 16) ? Qt::white : colors[c];
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

/*! clean up the offscreen buffer
 */
void AnsiWidget::destroyImage() {
  if (img) {
    delete img;
    img = 0;
  }
}

/*! Handles the characters following the \e[ sequence. Returns whether a further call
 * is required to complete the process.
 */
bool AnsiWidget::doEscape(unsigned char* &p) {
  int escValue = 0;

  while (isdigit(*p)) {
    escValue = (escValue * 10) + (*p - '0');
    p++;
  }

  if (*p == ' ') {
    p++;
    switch (*p) {
    case 'C':
      // GSS  Graphic Size Selection
      textSize = escValue;
      updateFont();
      break;
    }
  } 
  else if (setGraphicsRendition(*p, escValue)) {
    updateFont();
  }
    
  if (*p == ';') {
    p++; // next rendition
    return true;
  }
  return false;
}

/*! widget initialisation
 */
void AnsiWidget::initImage() {
  if (img == NULL) {
    img = new QPixmap(parentWidget()->width(), parentWidget()->height());
    img->fill(this->bg);
  }
}

/*! Handles the \n character
 */
void AnsiWidget::newLine() {
  int h = height();
  int fontHeight = textHeight();

  curX = INITXY;
  if (curY + (fontHeight * 2) >= h) {
    QRegion exposed;
    img->scroll(0, -fontHeight, 0, 0, width(), h, &exposed);

    QPainter painter(this->img);
    painter.fillRect(exposed.boundingRect(), this->bg);

    update();
  } 
  else {
    curY += fontHeight;
  }
}

/*! reset the current drawing variables
 */
void AnsiWidget::reset(bool init) {
  if (init) {
    curY = INITXY; // allow for input control border
    curX = INITXY;
    tabSize = 40; // tab size in pixels (160/32 = 5)
    markX = markY = pointX = pointY = 0;
    copyMode = false;
  }

  curYSaved = 0;
  curXSaved = 0;
  invert = false;
  underline = false;
  bold = false;
  italic = false;
  fg = Qt::black;
  bg = Qt::white;
  textSize = 10;
  updateFont();
}

/*! Handles the given escape character. Returns whether the font has changed
 */
bool AnsiWidget::setGraphicsRendition(char c, int escValue) {
  switch (c) {
  case 'K': 
    { // \e[K - clear to eol 
      QPainter painter(this->img);
      painter.fillRect(curX, curY, width() - curX, textHeight(), this->bg);
    }
    break;
  case 'G': // move to column
    curX = escValue;
    break;
  case 'T': // non-standard: move to n/80th of screen width
    curX = escValue * width() / 80;
    break;
  case 's': // save cursor position
    curYSaved = curX;
    curXSaved = curY;
    break;
  case 'u': // restore cursor position
    curX = curYSaved;
    curY = curXSaved;
    break;
  case ';': // fallthru
  case 'm': // \e[...m  - ANSI terminal
    switch (escValue) {
    case 0:  // reset
      reset(false);
      break;
    case 1: // set bold on
      bold = true;
      return true;
    case 2: // set faint on
      break;
    case 3: // set italic on
      italic = true;
      return true;
    case 4: // set underline on
      underline = true;
      break;
    case 5: // set blink on
      break;
    case 6: // rapid blink on
      break;
    case 7: // reverse video on
      invert = true;
      break;
    case 8: // conceal on
      break;
    case 21: // set bold off
      bold = false;
      return true;
    case 23:
      italic = false;
      return true;
    case 24: // set underline off
      underline = false;
      break;
    case 27: // reverse video off
      invert = false;
      break;
      // colors - 30..37 foreground, 40..47 background
    case 30: // set black fg
      this->fg = ansiToQt(0);
      break;
    case 31: // set red fg
      this->fg = ansiToQt(4);
      break;
    case 32: // set green fg
      this->fg = ansiToQt(2);
      break;
    case 33: // set yellow fg
      this->fg = ansiToQt(6);
      break;
    case 34: // set blue fg
      this->fg = ansiToQt(1);
      break;
    case 35: // set magenta fg
      this->fg = ansiToQt(5);
      break;
    case 36: // set cyan fg
      this->fg = ansiToQt(3);
      break;
    case 37: // set white fg
      this->fg = ansiToQt(7);
      break;
    case 40: // set black bg
      this->bg = ansiToQt(0);
      break;
    case 41: // set red bg
      this->bg = ansiToQt(4);
      break;
    case 42: // set green bg
      this->bg = ansiToQt(2);
      break;
    case 43: // set yellow bg
      this->bg = ansiToQt(6);
      break;
    case 44: // set blue bg
      this->bg = ansiToQt(1);
      break;
    case 45: // set magenta bg
      this->bg = ansiToQt(5);
      break;
    case 46: // set cyan bg
      this->bg = ansiToQt(3);
      break;
    case 47: // set white bg
      this->bg = ansiToQt(15);
      break;
    case 48: // subscript on
      break;
    case 49: // superscript
      break;
    };                        
  }
  return false;
}

/*! Updated the current font according to accumulated flags
 */
void AnsiWidget::updateFont() {
  QFont font = QFont("TypeWriter");
  font.setFixedPitch(true);
  font.setPointSize(textSize);
  font.setBold(bold);
  font.setItalic(italic);
  this->setFont(font);
}

void AnsiWidget::mouseMoveEvent(QMouseEvent* event) {
  pointX = event->x();
  pointY = event->y();
  if (copyMode) {
    update();
  }
  if (mouseMode && mouseListener) {
    mouseListener->mouseMoveEvent(event->button());
  }
}

void AnsiWidget::mousePressEvent(QMouseEvent* event) {
  bool selected = (markX != pointX || markY != pointY);
  markX = pointX = event->x();
  markY = pointY = event->y();
  if (mouseMode && selected) {
    update();
  }
  if (copyMode && mouseListener) {
    mouseListener->mousePressEvent();
  }
}

void AnsiWidget::mouseReleaseEvent(QMouseEvent* event) {
  bool selected = (markX != pointX || markY != pointY);
  if (copyMode && selected) {
    update();
  }
  if (mouseMode && mouseListener) {
    mouseListener->mousePressEvent();
  }
}

void AnsiWidget::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  QRect dirtyRect = event->rect();
  painter.drawPixmap(dirtyRect, *img, dirtyRect);

  // draw the mouse selection
  if (copyMode && (markX != pointX || markY != pointY)) {
    painter.setPen(Qt::DashDotDotLine);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.drawRect(markX, markY, pointX - markX, pointY - markY);
  }
}

void AnsiWidget::resizeEvent(QResizeEvent* event) {
  int imgW = img->width();
  int imgH = img->height();

  if (width() > imgW) {
    imgW = width();
  }
  
  if (height() > imgH) {
    imgH = height();
  }
  
  QPixmap* old = img;
  img = new QPixmap(imgW, imgH);
  QPainter painter(img);
  painter.fillRect(0, 0, imgW, imgH, this->bg);
  painter.drawPixmap(0, 0, old->width(), old->height(), *old);
  delete old;

  QWidget::resizeEvent(event);
}

// End of "$Id$".
