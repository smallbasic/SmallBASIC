// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>

#include "platform/mosync/screen.h"
#include "platform/mosync/utils.h"

#define WHITE 15
#define SCROLL_IND 4
#define MAX_HEIGHT 10000
#define TEXT_ROWS 1000

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

Screen::Screen(int x, int y, int width, int height, int fontSize) :
  Rectangle(x, y, width, height),
  font(0),
  fontSize(fontSize),
  charWidth(0),
  charHeight(0),
  pageHeight(0),
  scrollY(0),
  bg(0),
  fg(0),
  curY(0),
  curX(0),
  dirty(0),
  linePadding(0) {
}

Screen::~Screen() {
  Vector_each(Rectangle*, it, buttons) {
    delete (Rectangle *)(*it);
  }
  if (font) {
    maFontDelete(font);
  }
}

// converts ANSI colors to MoSync colors
int Screen::ansiToMosync(long c) {
  int result = c;
  if (c < 0) {
    result = -c;
  } else {
    result = (c > 15) ? colors[WHITE] : colors[c];
  }
  return result;
}

void Screen::draw(bool vscroll) {
  if (vscroll && pageHeight) {
    // display the vertical scrollbar
    int barSize = height * height / pageHeight;
    int barRange = height - (barSize + SCROLL_IND * 2);
    int barTop = SCROLL_IND + (barRange * scrollY / (pageHeight - (height - charHeight)));
    if (barSize < height) {
      maSetColor(fg);
      maLine(x + width - 3, y + barTop, x + width - 3, y + barTop + barSize);
      maLine(x + width - 4, y + barTop, x + width - 4, y + barTop + barSize);
    }
  }
  
  // display the label
  if (label.length()) {
    MAExtent extent = maGetTextSize(label.c_str());
    int w = EXTENT_X(extent);
    int h = EXTENT_Y(extent);
    int top = height - h - h;
    int left = (width - w) / 2;

    maSetColor(GRAY_BG_COL);
    maFillRect(left - 2, top, w + 8, h + 8);
    maSetColor(LABEL_TEXT_COL);
    maDrawText(left, top + 2, label.c_str());
  }
}

void Screen::drawInto(bool background) {
  maSetColor(background ? bg : fg);
}

// remove the button from the list
void Screen::remove(Rectangle *button) {
  Vector_each(Rectangle*, it, buttons) {
    Rectangle *next = (*it);
    if (next == button) {
      buttons.remove(it);
      break;
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

// updated the current font according to accumulated flags
void Screen::setFont(bool bold, bool italic) {
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
    charHeight = EXTENT_Y(extent) + LINE_SPACING;
    trace("charWidth:%d charHeight:%d fontSize:%d", charWidth, charHeight, fontSize);
  }
}

//
// Graphics and text based screen with limited scrollback support
//
GraphicScreen::GraphicScreen(int x, int y, int width, int height, int fontSize) :
  Screen(x, y, width, height, fontSize),
  image(0),
  underline(0),
  invert(0),
  bold(0),
  italic(0),
  imageWidth(width),
  imageHeight(height),
  curYSaved(0),
  curXSaved(0),
  tabSize(0) {
}

GraphicScreen::~GraphicScreen() {
  if (image) {
    maDestroyPlaceholder(image);
  }
}

// calculate the pixel movement for the given cursor position
void GraphicScreen::calcTab() {
  int c = 1;
  int x = curX + 1;
  while (x > tabSize) {
    x -= tabSize;
    c++;
  }
  curX = c * tabSize;
}

bool GraphicScreen::construct() {
  bool result = true;
  image = maCreatePlaceholder();
  if (maCreateDrawableImage(image, imageWidth, imageHeight) == RES_OK) {
    curX = INITXY;
    curY = INITXY;
    tabSize = 40;   // tab size in pixels (160/32 = 5)
    scrollY = 0;
    reset();
  } else {
    result = false;
  }
  return result;
}

void GraphicScreen::clear() {
  drawInto(true);
  maSetColor(bg);
  maFillRect(0, 0, imageWidth, imageHeight);

  curX = INITXY;
  curY = INITXY;
  scrollY = 0;
  pageHeight = 0;

  // cleanup any buttons
  Vector_each(Rectangle*, it, buttons) {
    delete (*it);
  }
  buttons.clear();
  label.clear();
}

void GraphicScreen::draw(bool vscroll) {
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
  
  Screen::draw(vscroll);

  maUpdateScreen();
  maResetBacklight();
  maSetDrawTarget(currentHandle);
  dirty = 0;
}

void GraphicScreen::drawText(const char *text, int len, int x, int lineHeight) {
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

void GraphicScreen::drawInto(bool background) {
  Screen::drawInto(background);
  maSetDrawTarget(image);
  if (!dirty) {
    dirty = maGetMilliSecondCount();
  }
}

int GraphicScreen::getPixel(int x, int y) {
  MARect rc;
  rc.left = x;
  rc.top = y;
  rc.width = 1;
  rc.height = 1;

  int data[1];
  int result = 0;

  maGetImageData(image, &data, &rc, 1);
  result = -(data[0] & 0x00FFFFFF);
  return result;
}

// handles the \n character
void GraphicScreen::newLine(int lineHeight) {
  lineHeight += linePadding;
  linePadding = 0;
  curX = INITXY;
  if (height < MAX_HEIGHT) {
    int offset = curY + (lineHeight * 2);
    if (offset >= height) {
      if (offset >= imageHeight) {
        // extend the base image by another page size
        MAHandle newImage = maCreatePlaceholder();
        int newHeight = imageHeight + height;
        if (maCreateDrawableImage(newImage, imageWidth, newHeight) != RES_OK) {
          // failed to create image
          clear();
          lineHeight = 0;
        } else {
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
      }
      scrollY += lineHeight;
    }
    curY += lineHeight;
    pageHeight += lineHeight;
  } else {
    // overflow
    clear();
  }
}

int GraphicScreen::print(const char *p, int lineHeight) {
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
void GraphicScreen::reset(int argFontSize) {
  curXSaved = 0;
  curYSaved = 0;
  invert = false;
  underline = false;
  bold = false;
  italic = false;
  fg = DEFAULT_COLOR;
  bg = 0;
  if (argFontSize != -1) {
    fontSize = argFontSize;
  }
  setFont(bold, italic);
}

// update the widget to new dimensions
void GraphicScreen::resize(int newWidth, int newHeight, int oldWidth, int oldHeight, int lineHeight) {
  logEntered();
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

    if (curY >= imageHeight) {
      curY = height - lineHeight;
      pageHeight = curY;
    }
    if (curX >= imageWidth) {
      curX = 0;
    }
  }
  scrollY = 0;
  width = newWidth;
  height = newHeight;
}

// handles the given escape character. Returns whether the font has changed
bool GraphicScreen::setGraphicsRendition(char c, int escValue, int lineHeight) {
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

//
// Text based screen with a large scrollback buffer
//
TextScreen::TextScreen(int x, int y, int w, int h, int fontSize) : 
  Screen(x, y, w, h, fontSize),
  rows(TEXT_ROWS),
  cols(0),
  width(0),
  head(NULL),
  tail(NULL) {
}

TextScreen::~TextScreen() {
  delete[] buffer;
}

void TextScreen::calcTab() {
  Row *line = getLine(head);  // pointer to current line
  line->tab();  
}

bool TextScreen::construct() {
  // initialize the buffer
  buffer = new Row[rows];
  return (buffer != NULL);
}

//
// clear the screen
//
void TextScreen::clear() {
  head = tail = 0;
  cols = width = 0;
  getLine(0)->clear();
}

//
// draw the text
//
void TextScreen::draw(bool vscroll) {
  // prepare escape state variables
  bool bold = false;
  bool italic = false;
  bool underline = false;
  bool invert = false;

  // calculate rows to display
  int pageRows = getPageRows();
  int textRows = getTextRows();
  int numRows = textRows < pageRows ? textRows : pageRows;
  int firstRow = tail + scrollY;        // from start plus scroll offset

  // setup the background colour
  MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
  maSetColor(bg);
  maFillRect(x, y, width-1, height-1);
  maSetColor(fg);

  // draw the visible segments
  int pageWidth = 0;
  for (int row = firstRow, rows = 0, nextY = y + lineHeight; 
       rows < numRows; 
       row++, rows++, nextY += lineHeight) {
    Row *line = getLine(row);   // next logical row
    TextSeg *seg = line->head;
    int x = INITXY;
    while (seg != NULL) {
      if (seg->escape(&bold, &italic, &underline, &invert)) {
        setFont(bold, italic);
      }
      int width = seg->width();
      if (seg->str) {
        if (invert) {
          maSetColor(fg);
          maFillRect(x, y - charHeight, width, charHeight);
          maSetColor(bg);
          maDrawText(x, y, seg->str);
          maSetColor(fg);
        } else {
          maDrawText(x, y, seg->str);
        }
      }
      if (underline) {
        maLine(x, y + 1, x + width, y + 1);
      }
      x += width;
      seg = seg->next;
    }
    int rowWidth = line->width();
    if (rowWidth > pageWidth) {
      pageWidth = rowWidth;
    }
  }

  // draw any visible buttons
  Vector_each(Rectangle*, it, buttons) {
    Rectangle *rect = (Rectangle *)(*it);
    if (rect->y >= y + scrollY && 
        rect->y <= y + scrollY + height) {
      rect->draw();
    }
  }

  // draw the base components
  Screen::draw(vscroll);

  maUpdateScreen();
  maResetBacklight();
  maSetDrawTarget(currentHandle);
}

//
// return a pointer to the specified line of the display.
//
Row *TextScreen::getLine(int pos) {
  if (pos < 0) {
    pos += rows;
  }
  if (pos > rows - 1) {
    pos -= rows;
  }

  return &buffer[pos];
}

void TextScreen::newLine(int lineHeight) {
  // scroll by moving logical last line
  if (getTextRows() == rows) {
    tail = (tail + 1 >= rows) ? 0 : tail + 1;
  }
  head = (head + 1 >= rows) ? 0 : head + 1;
  
  // clear the new line
  Row* line = getLine(head);
  line->clear();
}

//
// Creates a new segment then prints the line
//
int TextScreen::print(const char *p, int lineHeight) {
  Row *line = getLine(head);
  TextSeg *segment = new TextSeg();
  line->append(segment);

  int numChars = 1;         // print minimum of one character
  int cx = charWidth;
  int w = width - 1;
  int curX = line->width();

  if (curX + cx >= w) {
    newLine(lineHeight);
    line = getLine(head);
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

  // Print the next (possible) line of text
  segment->setText(p, numChars);

  // save max rows encountered
  int lineWidth = line->width();
  if (lineWidth > width) {
    width = lineWidth;
  }
  
  int lineChars = line->numChars();
  if (lineChars > cols) {
    cols = lineChars;
  }

  return numChars;
}

void TextScreen::reset(int argFontSize) {
  if (argFontSize != -1) {
    fontSize = argFontSize;
  }
  setFont(false, false);
}

void TextScreen::resize(int newWidth, int newHeight, int oldWidth, 
                        int oldHeight, int lineHeight) {
  width = newWidth;
  height = newHeight;
}

//
// performs the ANSI text SGI function.
//
bool TextScreen::setGraphicsRendition(char c, int escValue, int lineHeight) {
  TextSeg *segment = getLine(head)->current();

  switch (c) {
  case 0:
    segment->reset();
    break;

  case 1:                      // Bold on
    segment->set(TextSeg::BOLD, true);
    break;

  case 2:                      // Faint on
    segment->set(TextSeg::BOLD, false);
    break;

  case 3:                      // Italic on
    segment->set(TextSeg::ITALIC, true);
    break;

  case 4:                      // Underscrore
    segment->set(TextSeg::UNDERLINE, true);
    break;

  case 7:                      // reverse video on
    segment->set(TextSeg::INVERT, true);
    break;

  case 21:                     // set bold off
    segment->set(TextSeg::BOLD, false);
    break;

  case 23:
    segment->set(TextSeg::ITALIC, false);
    break;

  case 24:                     // set underline off
    segment->set(TextSeg::UNDERLINE, false);
    break;

  case 27:                     // reverse video off
    segment->set(TextSeg::INVERT, false);
    break;

  case 30:                     // Black
    segment->color = ansiToMosync(0);
    break;

  case 31:                     // Red
    segment->color = ansiToMosync(4);
    break;

  case 32:                     // Green
    segment->color = ansiToMosync(2);
    break;

  case 33:                     // Yellow
    segment->color = ansiToMosync(6);
    break;

  case 34:                     // Blue
    segment->color = ansiToMosync(1);
    break;

  case 35:                     // Magenta
    segment->color = ansiToMosync(5);
    break;

  case 36:                     // Cyan
    segment->color = ansiToMosync(3);
    break;

  case 37:                     // White
    segment->color = ansiToMosync(7);
    break;
  }
}
