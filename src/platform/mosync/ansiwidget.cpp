// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>
#include <ctype.h>

#include "platform/mosync/ansiwidget.h"
#include "platform/mosync/utils.h"

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
#define BLACK  0
#define WHITE  15

static int colors[] = {
  0x000000,                     // 0 black
  0x000080,                     // 1 blue
  0x008000,                     // 2 green
  0x008080,                     // 3 cyan
  0x800000,                     // 4 red
  0x800080,                     // 5 magenta
  0x808000,                     // 6 yellow
  0xC0C0C0,                     // 7 white
  0x808080,                     // 8 gray
  0x0000FF,                     // 9 light blue
  0x00FF00,                     // 10 light green
  0x00FFFF,                     // 11 light cyan
  0xFF0000,                     // 12 light red
  0xFF00FF,                     // 13 light magenta
  0xFFFF00,                     // 14 light yellow
  0xFFFFFF                      // 15 bright white
};

/*
 * Workaround for API's which don't take a length argument
 */
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

/*
 * Handles drawing to a backbuffer
 */
struct Backbuffer {
  MAHandle image;
  
  Backbuffer(MAHandle image, int color) : image(image) {
    maSetDrawTarget(image);
    maSetColor(color);
  }

  ~Backbuffer() {
    maSetDrawTarget(HANDLE_SCREEN);
    maDrawImage(image, 0, 0);
    maUpdateScreen();
    maResetBacklight();
  }
};

/*
struct HyperlinkButton : public QAbstractButton {
  HyperlinkButton(QString url, QString text, QWidget *parent) : 
    QAbstractButton(parent) {
    this->url = url;
    setText(text);
  };

  void paintEvent(QPaintEvent *event) {
    AnsiWidget *out = (AnsiWidget *) parent();
    QPainter painter(this);
    QFontMetrics fm = fontMetrics();
    int width = fm.width(text());
    int y = fm.ascent();

    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(out->font());
    painter.setPen(isDown() ? Qt::blue : underMouse() ? Qt::red : Qt::darkGreen);

    painter.drawText(0, y, text());
    if (underMouse()) {
      maLine(0, y + 2, width, y + 2);
    }
  }

  QString url;
};
*/

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
  scrollSize(0),
  width(width),
  height(height),
  mouseMode(0),
  listener(0) {
}

bool AnsiWidget::construct() {
  bool result = false;
  image = maCreatePlaceholder();
  if (image && RES_OK == maCreateDrawableImage(image, width, height)) {
    reset(true);
    clearScreen();
    result = true;
  }
  return result;
}

/*! widget clean up
 */
AnsiWidget::~AnsiWidget() {
  maDestroyPlaceholder(image);
  if (font) {
    maFontDelete(font);
  }
}

/*! create audible beep sound
 */
void AnsiWidget::beep() const {
  //  QApplication::beep();
  // http://www.mosync.com/documentation/manualpages/using-audio-api
}

/*! clear the offscreen buffer
 */
void AnsiWidget::clearScreen() {
  reset(true);
  Backbuffer backbuffer(image, bg);
  maFillRect(0, 0, width, height);
}

/*! draws the given image onto the offscreen buffer
 */
void AnsiWidget::drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h) {
  Backbuffer backbuffer(image, fg);

  //  painter.drawImage(x, y, *image, sx, sy, w, h);
}

/*! draw a line onto the offscreen buffer
 */
void AnsiWidget::drawLine(int x1, int y1, int x2, int y2) {
  Backbuffer backbuffer(image, fg);
  maLine(x1, y1, x2, y2);
}

/*! draw a rectangle onto the offscreen buffer
 */
void AnsiWidget::drawRect(int x1, int y1, int x2, int y2) {
  Backbuffer backbuffer(image, fg);
  maLine(x1, y1, x2, y1); // top
  maLine(x1, y2, x2, y2); // bottom
  maLine(x1, y1, x1, y2); // left
  maLine(x2, y1, x2, y2); // right
}

/*! draw a filled rectangle onto the offscreen buffer
 */
void AnsiWidget::drawRectFilled(int x1, int y1, int x2, int y2) {
  Backbuffer backbuffer(image, fg);
  maFillRect(x1, y1, x2 - x1, y2 - y1);
}

/*! returns the color of the pixel at the given xy location
 */
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

/*! Returns the height in pixels using the current font setting
 */
int AnsiWidget::textHeight(void) {
  return EXTENT_Y(maGetTextSize("Q@"));
}

/*! Returns the width in pixels using the current font setting
 */
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

/*! Prints the contents of the given string onto the backbuffer
 */
void AnsiWidget::print(const char *str) {
  Backbuffer backBuffer(image, fg);
  int len = strlen(str);
  if (len <= 0) {
    return;
  }

  int fontHeight = textHeight();
  int ascent = 0;

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
      curX = INITXY;
      maSetColor(this->bg);
      maFillRect(0, curY, width, fontHeight);
      break;
    default:
      int numChars = 1;         // print minimum of one character
      int cx = textWidth((const char *)p, 1);
      int w = width - 1;

      if (curX + cx >= w) {
        newLine();
      }
      // print further non-control, non-null characters 
      // up to the width of the line
      while (p[numChars] > 31) {
        cx += textWidth((const char *)p + numChars, 1);
        if (curX + cx < w) {
          numChars++;
        } else {
          break;
        }
      }

      // erase the background
      maSetColor(invert ? this->fg : this->bg);
      maFillRect(curX, curY, cx, fontHeight);

      // draw the text buffer
      maSetColor(invert ? this->bg : this->fg);
      maDrawText(curX, curY + ascent, TextBuffer((const char *)p, numChars).str);

      if (underline) {
        maLine(curX, curY + ascent + 1, curX + cx, curY + ascent + 1);
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
}

/*! Display the contents of the back buffer
 */
void AnsiWidget::refresh() {
  Backbuffer(image, bg);
}

/*! Update the widget to new dimensions
 */
void AnsiWidget::resize(int newWidth, int newHeight) {
  MAHandle newImage = maCreatePlaceholder();
  maCreateDrawableImage(newImage, newWidth, newHeight);
  
  MARect srcRect;
  MAPoint2d dstPoint;
  
  srcRect.left = 0;
  srcRect.top = 0;
  srcRect.width = (width > newWidth ? newWidth : width);
  srcRect.height = (height > newHeight ? newHeight : height);
  dstPoint.x = 0;
  dstPoint.y = 0;

  maSetDrawTarget(newImage);
  maDrawImageRegion(image, &srcRect, &dstPoint, TRANS_NONE);
  
  maDestroyPlaceholder(image);
  image = newImage;
  width = newWidth;
  height = newHeight;

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

/*! sets the current drawing color
 */
void AnsiWidget::setColor(long fg) {
  this->fg = ansiToMosync(fg);
}

/*! sets the pixel to the given color at the given xy location
 */
void AnsiWidget::setPixel(int x, int y, int c) {
  Backbuffer backbuffer(image, c);
  maPlot(x, y);
}

/*! sets the current text drawing color
 */
void AnsiWidget::setTextColor(long fg, long bg) {
  this->bg = ansiToMosync(bg);
  this->fg = ansiToMosync(fg);
}

/*! sets the number of scrollback lines
 */
void AnsiWidget::setScrollSize(int scrollSize) {
  this->scrollSize = scrollSize;
}

/*! sets mouse mode on or off
 */
void AnsiWidget::setMouseMode(bool flag) {
  mouseMode = flag;
}

/*! resets mouse mode to false
 */
void AnsiWidget::resetMouse() {
  pointX = pointY = markX = markY = 0;
  mouseMode = false;
}

/*! public slot - copy selected text to the clipboard
 */
void AnsiWidget::copySelection() {
}

/*! public slot - a hyperlink has been clicked
 */
void AnsiWidget::linkClicked() {
  //HyperlinkButton *button = (HyperlinkButton *) sender();
  if (listener) {
    // listener->loadPath(button->url, true, true);
  }
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
int AnsiWidget::ansiToMosync(long c) {
  int result = c;
  if (c < 0) {
    // assume color is windows style RGB packing
    // RGB(r,g,b) ((COLORREF)((BYTE)(r)|((BYTE)(g) << 8)|((BYTE)(b) << 16)))
    result = -c;
  } else {
    result = (c > 16) ? colors[WHITE] : colors[c];
  }

  return result;
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

/*! Creates a hyperlink, eg // ^[ hwww.foo.com:title:hover;More text
 */
void AnsiWidget::createLink(char *&p, bool execLink) {
  /*
  QString url;
  QString text;
  QString tooltip;

  unsigned char *next = p + 1;
  bool eot = false;
  int segment = 0;

  while (*p && !eot) {
    p++;

    switch (*p) {
    case '\033':
    case '\n':
    case ':':
      eot = true;
      // fallthru

    case ';':
      switch (segment++) {
      case 0:
        url = QString::fromUtf8((const char *)next, (p - next));
        text = tooltip = url;
        break;
      case 1:
        text = QString::fromUtf8((const char *)next, (p - next));
        tooltip = text;
        break;
      case 2:
        tooltip = QString::fromUtf8((const char *)next, (p - next));
        eot = true;
        break;
      default:
        break;
      }
      next = p + 1;
      break;

    default:
      break;
    }
  }

  if (execLink && listener) {
    listener->loadPath(url, true, true);
  } else {
    HyperlinkButton *button = new HyperlinkButton(url, text, this);
    QFontMetrics fm = fontMetrics();
    int width = fm.width(text) + 2;
    int height = fm.ascent() + fm.descent() + 4;

    button->setGeometry(curX, curY, width, height);
    button->setToolTip(tooltip);
    button->connect(button, SIGNAL(clicked(bool)), this, SLOT(linkClicked()));
    button->setCursor(Qt::PointingHandCursor);
    button->show();

    hyperlinks.append(button);
    curX += width;
  }
*/
}

/*! Handles the characters following the \e[ sequence. Returns whether a further call
 * is required to complete the process.
 */
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
    }
  } else if (setGraphicsRendition(*p, escValue)) {
    updateFont();
  }

  if (*p == ';') {
    p++;                        // next rendition
    return true;
  }
  return false;
}

/*! Handles the \n character
 */
void AnsiWidget::newLine() {
  int fontHeight = textHeight();
  curX = INITXY;
  if (curY + (fontHeight * 2) >= height) {
    MAHandle newImage = maCreatePlaceholder();
    maCreateDrawableImage(newImage, width, height);

    MARect srcRect;
    MAPoint2d dstPoint;

    srcRect.left = 0;
    srcRect.top = fontHeight;
    srcRect.width = width;
    srcRect.height = height - fontHeight;
    dstPoint.x = 0;
    dstPoint.y = 0;

    maSetDrawTarget(newImage);
    maDrawImageRegion(image, &srcRect, &dstPoint, TRANS_NONE);
    maSetColor(bg);
    maFillRect(0, height - fontHeight, width, fontHeight);

    maSetDrawTarget(image);
    maDrawImage(newImage, 0, 0);
    maDestroyPlaceholder(newImage);
  } else {
    curY += fontHeight;
  }
}

/*! reset the current drawing variables
 */
void AnsiWidget::reset(bool init) {
  if (init) {
    curY = INITXY;              // allow for input control border
    curX = INITXY;
    tabSize = 40;               // tab size in pixels (160/32 = 5)
    markX = markY = pointX = pointY = 0;
    copyMode = false;
  }

  // cleanup any hyperlinks
  //  int n = hyperlinks.size();
  //  for (int i = 0; i < n; i++) {
  //    QAbstractButton *nextObject = hyperlinks.at(i);
  //    delete nextObject;
  //  }
  //  hyperlinks.clear();

  curYSaved = 0;
  curXSaved = 0;
  invert = false;
  underline = false;
  bold = false;
  italic = false;
  fg = 0xffba00;
  bg = colors[BLACK];
  textSize = height / 40;
  updateFont();
}

/*! Handles the given escape character. Returns whether the font has changed
 */
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

/*! Updated the current font according to accumulated flags
 */
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

/*
void AnsiWidget::mouseMoveEvent(QMouseEvent *event) {
  pointX = event->x();
  pointY = event->y();
  if (copyMode) {
    update();
  }
  if (mouseMode && listener) {
    listener->mouseMoveEvent(event->button());
  }
}

void AnsiWidget::mousePressEvent(QMouseEvent *event) {
  bool selected = (markX != pointX || markY != pointY);
  markX = pointX = event->x();
  markY = pointY = event->y();
  if (mouseMode && selected) {
    update();
  }
  if (copyMode && listener) {
    listener->mousePressEvent();
  }
}

void AnsiWidget::mouseReleaseEvent(QMouseEvent *) {
  bool selected = (markX != pointX || markY != pointY);
  if (copyMode && selected) {
    update();
  }
  if (mouseMode && listener) {
    listener->mousePressEvent();
  }
}

*/
