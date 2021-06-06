// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>

#include "ui/screen.h"

#define WHITE 15
#define SCROLL_IND 4
#define MAX_HEIGHT 10000
#define TEXT_ROWS 1000

#define DRAW_SHAPE \
  Shape *rect = (*it); \
  if (rect->_y >= _scrollY && \
      rect->_y <= _scrollY + _height) \
    rect->draw(_x + rect->_x, _y + rect->_y - _scrollY, w(), h(), _charWidth)

int compareZIndex(const void *p1, const void *p2) {
  ImageDisplay **i1 = (ImageDisplay **)p1;
  ImageDisplay **i2 = (ImageDisplay **)p2;
  return (*i1)->_zIndex < (*i2)->_zIndex ? -1 : (*i1)->_zIndex == (*i2)->_zIndex ? 0 : 1;
}

bool Shape::isFullScreen() const {
  MAExtent screenSize = maGetScrSize();
  return _width == EXTENT_X(screenSize) && _height == EXTENT_Y(screenSize);
}

Screen::Screen(int x, int y, int width, int height, int fontSize) :
  Shape(x, y, width, height),
  _font(0),
  _fontSize(fontSize),
  _fontStyle(0),
  _charWidth(0),
  _charHeight(0),
  _scrollX(0),
  _scrollY(0),
  _bg(0),
  _fg(0),
  _curX(INITXY),
  _curY(INITXY),
  _dirty(0),
  _linePadding(0) {
}

Screen::~Screen() {
  if (_font) {
    maFontDelete(_font);
  }
}

// converts ANSI colors to MoSync colors
int Screen::ansiToMosync(long c) {
  int result;
  if (c < 0) {
    result = -c;
  } else {
    result = (c > 15) ? colors[WHITE] : colors[c];
  }
  return result;
}

void Screen::add(Shape *button) {
  _shapes.add(button);
  if (button->_x + button->_width > _curX) {
    _curX = button->_x + button->_width;
  }
  if (button->_y > _curY) {
    _curY = button->_y;
  }
}

void Screen::addImage(ImageDisplay &image) {
  bool exists = false;
  List_each(ImageDisplay *, it, _images) {
    ImageDisplay *next = (*it);
    if (next->_id == image._id) {
      exists = true;
      next->copyImage(image);
      break;
    }
  }
  if (!exists) {
    _images.add(new ImageDisplay(image));
  }
  _images.sort(compareZIndex);
  setDirty();
}

void Screen::clear() {
  _curX = INITXY;
  _curY = INITXY;
  _scrollX = 0;
  _scrollY = 0;

  // cleanup any shapes
  _shapes.removeAll();
  _inputs.removeAll();
  _images.removeAll();
  _label.clear();
}

void Screen::drawLabel() {
  if (!_label.empty()) {
    int labelLen = _label.length();
    int w = _charWidth * (labelLen + 2);
    int h = _charHeight + 2;
    int top = _height - h;
    int left = (_width - w) / 2;
    int textY = top + ((h - _charHeight) / 2);

    maSetColor(0xbfbfbf);
    maFillRect(left, top, w, h);
    maSetColor(0xe5e5e5);
    maLine(left, top, left + w, top);
    maSetColor(0x737373);
    maLine(left, top + h - 1, left + w, top + h - 1);
    maLine(left + w, top + 1, left + w, top + h - 1);
    maSetColor(0x403c44);
    maDrawText(left + _charWidth, textY, _label.c_str(), labelLen);
  }
}

void Screen::drawMenu() {
  static const char dot[] = {'\260', '\0'};
  int gap = _charHeight / 3;
  int left = _width - _charWidth - (_charWidth / 2);
  int top = (_height - _charHeight) + gap;
  maSetColor(_fg);
  maDrawText(left, top, dot, 1);
  maDrawText(left, top - gap, dot, 1);
  maDrawText(left, top - gap - gap, dot, 1);
}

void Screen::drawShape(Shape *rect) {
  if (rect != NULL &&
      rect->_y >= _scrollY &&
      rect->_y + rect->_height <= _scrollY + _height) {
    rect->draw(_x + rect->_x, _y + rect->_y - _scrollY, w(), h(), _charWidth);

    List_each(FormInput *, it, _inputs) {
      FormInput *input = (*it);
      if (input->isVisible() &&
          input->isDrawTop() &&
          input->_y >= _scrollY - _height &&
          input->_y + input->_height <= _scrollY + _height) {
        input->draw(_x + input->_x, _y + input->_y - _scrollY, w(), h(), _charWidth);
      }
    }
  }
}

void Screen::drawOverlay(bool vscroll) {
  // draw any visible shapes
  List_each(Shape *, it, _shapes) {
    DRAW_SHAPE;
  }

  List_each(ImageDisplay *, it, _images) {
    DRAW_SHAPE;
  }

  FormInput *drawTop = NULL;
  List_each(FormInput *, it, _inputs) {
    FormInput *input = (*it);
    if (input->_y >= _scrollY - _height &&
        input->isVisible()) {
      if (input->isDrawTop()) {
        drawTop = input;
      } else {
        input->draw(_x + input->_x, _y + input->_y - _scrollY, w(), h(), _charWidth);
      }
    }
  }
  if (drawTop != NULL) {
    drawTop->draw(_x + drawTop->_x, _y + drawTop->_y - _scrollY, w(), h(), _charWidth);
  }

  if (vscroll && _curY) {
    // display the vertical scrollbar
    int pageHeight = _curY + _charHeight + _charHeight;
    int barSize = _height * _height / pageHeight;

    if (barSize < _height) {
      int barRange = _height - (barSize + SCROLL_IND * 2);
      int barTop = SCROLL_IND + (barRange * _scrollY / (pageHeight - _height));
      int barBottom = _y + barTop + barSize;
      if (barBottom + SCROLL_IND > _height) {
        barBottom = _height - SCROLL_IND;
      }
      maSetColor(_fg);
      maLine(_x + _width - 3, _y + barTop, _x + _width - 3, barBottom);
      maLine(_x + _width - 4, _y + barTop, _x + _width - 4, barBottom);
    }
  }

  drawLabel();

#if defined(_FLTK)
  drawMenu();
#else
  if ((!_inputs.empty() || !_label.empty()) && isFullScreen()) {
    // draw the menu widget when in UI mode
    drawMenu();
  }
#endif
}

void Screen::drawInto(bool background) {
  maSetColor(background ? _bg : _fg);
  setDirty();
}

int Screen::getIndex(FormInput *input) const {
  int index;
  if (input == NULL) {
    index = -1;
  } else {
    index = 0;
    List_each(FormInput *, it, _inputs) {
      FormInput *next = (*it);
      if (next == input) {
        break;
      } else {
        index++;
      }
    }
  }
  return index;
}

FormInput *Screen::getMenu(FormInput *prev, int px, int py) {
  FormInput *result = _inputs[0];
  if (result != NULL && overlaps(px, py)) {
    int item = (py - _y) / result->_height;
    result = _inputs[item];
  } else {
    result = NULL;
  }
  if (result != prev) {
    MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
    if (prev != NULL) {
      prev->_pressed = false;
      drawShape(prev);
    }
    if (result != NULL) {
      result->_pressed = true;
      drawShape(result);
    }
    maUpdateScreen();
    maSetDrawTarget(currentHandle);
  }
  return result;
}

FormInput *Screen::getNextMenu(FormInput *prev, bool up) {
  int index;
  if (prev == NULL) {
    index = 0;
  } else {
    index = getIndex(prev) + (up ? -1 : 1);
  }
  FormInput *next = prev;
  if (index > -1 && index < _inputs.size()) {
    MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
    next = _inputs.get(index);
    next->_pressed = true;
    drawShape(next);
    if (prev != NULL) {
      prev->_pressed = false;
      drawShape(prev);
    }
    maUpdateScreen();
    maSetDrawTarget(currentHandle);
  }
  return next;
}

void Screen::layoutInputs(int newWidth, int newHeight) {
  List_each(FormInput *, it, _inputs) {
    FormInput *r1 = (*it);
    r1->layout(newWidth, newHeight);
  }
}

// whether the point overlaps the label text
bool Screen::overLabel(int px, int py) {
  bool result;
  if (!_label.empty()) {
    int w = _charWidth * (_label.length() + 2);
    int h = _charHeight + 2;
    int top = _height - h;
    int left = (_width - w) / 2;
    result = (!OUTSIDE_RECT(px, py, left, top, w, h));
  } else {
    result = false;
  }
  return result;
}

// whether the point overlaps the menu widget
bool Screen::overMenu(int px, int py) {
  int w = _charWidth * 3;
  int h = _charHeight * 2;
  return (!OUTSIDE_RECT(px, py, _width - w, _height - h, w, h));
}

// whether the given point overlaps with the screen rectangle
bool Screen::overlaps(int px, int py) {
  return (!OUTSIDE_RECT(px, py, _x, _y, _width, _height));
}

int Screen::print(const char *p, int lineHeight, bool allChars) {
  // print minimum of one character
  int numChars = 1;
  int cx = _charWidth;
  int w = _width - 1;

  // print further non-control, non-null characters
  // up to the width of the line
  while (p[numChars] > 31) {
    cx += _charWidth;
    if (allChars || _curX + cx < w) {
      numChars++;
    } else {
      break;
    }
  }

  _curX += cx;
  return numChars;
}

// remove the button from the list
void Screen::remove(Shape *button) {
  List_each(Shape *, it, _shapes) {
    Shape *next = (*it);
    if (next == button) {
      _shapes.remove(it);
      setDirty();
      break;
    }
  }
}

// remove the image from the list
bool Screen::removeInput(FormInput *input) {
  bool result = false;
  List_each(FormInput *, it, _inputs) {
    FormInput *next = (*it);
    if (next == input) {
      _inputs.remove(it);
      result = true;
      break;
    }
  }
  return result;
}

// remove the image from the list
void Screen::removeImage(unsigned imageId) {
  List_each(ImageDisplay *, it, _images) {
    ImageDisplay *next = (*it);
    if (next->_id == imageId) {
      _images.remove(it);
      setDirty();
      break;
    }
  }
}

void Screen::replaceFont(int type) {
  logEntered();
  if (_font) {
    maFontDelete(_font);
  }
  _font = maFontLoadDefault(type, _fontStyle, _fontSize);
  if (_font != -1) {
    maFontSetCurrent(_font);
    MAExtent extent = maGetTextSize("W");
    _charWidth = EXTENT_X(extent);
    _charHeight = EXTENT_Y(extent) + LINE_SPACING;
  } else {
    trace("maFontLoadDefault failed: style=%d size=%d", _fontStyle, _fontSize);
  }
}

void Screen::reset(int fontSize) {
  _fg = DEFAULT_FOREGROUND;
  _bg = DEFAULT_BACKGROUND;
  setFont(false, false, fontSize);
}

void Screen::setColor(long color) {
  _fg = ansiToMosync(color);
}

void Screen::setTextColor(long foreground, long background) {
  _bg = ansiToMosync(background);
  _fg = ansiToMosync(foreground);
}

// updated the current font according to accumulated flags
void Screen::setFont(bool bold, bool italic, int size) {
  int style = FONT_STYLE_NORMAL;
  int type = FONT_TYPE_MONOSPACE;

  if (italic) {
    style |= FONT_STYLE_ITALIC;
    type = FONT_TYPE_SERIF;
  }
  if (bold) {
    style |= FONT_STYLE_BOLD;
    if (!italic) {
      type = FONT_TYPE_SANS_SERIF;
    }
  }

  if ((style != _fontStyle || size != _fontSize) || !_font) {
    _fontStyle = style;
    _fontSize = size;
    replaceFont(type);
  }
}

FormInput *Screen::getNextField(FormInput *field) {
  FormInput *result = NULL;
  bool setNext = false;
  List_each(FormInput *, it, _inputs) {
    FormInput *next = (*it);
    if (!next->isNoFocus()) {
      if (result == NULL) {
        // set result to first item
        result = next;
        if (field == NULL) {
          // no next item
          break;
        }
      }
      if (setNext) {
        result = next;
        break;
      } else if (next == field) {
        setNext = true;
      }
    }
  }
  return result;
}

void Screen::updateInputs(var_p_t form, bool setVars) {
  List_each(FormInput *, it, _inputs) {
    FormInput *next = (*it);
    if (setVars) {
      next->updateField(form);
    } else {
      var_p_t field = next->getField(form);
      if (field == NULL) {
        _inputs.remove(it);
        delete next;
        setDirty();
        it--;
      } else if (next->updateUI(form, field)) {
        setDirty();
      }
    }
  }
}

//
// Graphics and text based screen with limited scrollback support
//
GraphicScreen::GraphicScreen(int width, int height, int fontSize) :
  Screen(0, 0, width, height, fontSize),
  _image(0),
  _underline(0),
  _invert(0),
  _bold(0),
  _italic(0),
  _imageWidth(width),
  _imageHeight(height),
  _curYSaved(0),
  _curXSaved(0),
  _tabSize(40) {  // tab size in pixels (160/32 = 5)
}

GraphicScreen::~GraphicScreen() {
  if (_image) {
    maDestroyPlaceholder(_image);
  }
}

// calculate the pixel movement for the given cursor position
void GraphicScreen::calcTab() {
  int c = 1;
  int x = _curX + 1;
  while (x > _tabSize) {
    x -= _tabSize;
    c++;
  }
  _curX = c * _tabSize;
}

bool GraphicScreen::construct() {
  bool result = true;
  _image = maCreatePlaceholder();
  if (maCreateDrawableImage(_image, _imageWidth, _imageHeight) == RES_OK) {
    reset(_fontSize);
  } else {
    result = false;
  }
  return result;
}

void GraphicScreen::clear() {
  drawInto(true);
  maSetColor(_bg);
  maFillRect(0, 0, _imageWidth, _imageHeight);
  Screen::clear();
}

void GraphicScreen::drawArc(int xc, int yc, double r, double start, double end, double aspect) {
  drawInto();
  maArc(xc, yc, r, start, end, aspect);
}

void GraphicScreen::drawBase(bool vscroll, bool update) {
  MARect srcRect;
  MAPoint2d dstPoint;
  srcRect.left = 0;
  srcRect.top = _scrollY;
  srcRect.width = _width;
  srcRect.height = _height;
  dstPoint.x = _x;
  dstPoint.y = _y;
  MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
  maDrawImageRegion(_image, &srcRect, &dstPoint, TRANS_NONE);

  drawOverlay(vscroll);
  _dirty = 0;
  if (update) {
    maUpdateScreen();
  }
  maSetDrawTarget(currentHandle);
}

void GraphicScreen::drawEllipse(int xc, int yc, int rx, int ry, int fill) {
  drawInto();
  maEllipse(xc, yc, rx, ry, fill);
}

void GraphicScreen::drawImage(ImageDisplay &image) {
  drawInto();
  image.draw(image._x, image._y, image._width, image._height, 0);
}

void GraphicScreen::drawInto(bool background) {
  maSetDrawTarget(_image);
  Screen::drawInto(background);
}

void GraphicScreen::drawLine(int x1, int y1, int x2, int y2) {
  drawInto();
  maLine(x1, y1, x2, y2);
}

void GraphicScreen::drawRect(int x1, int y1, int x2, int y2) {
  drawInto();
  maLine(x1, y1, x2, y1); // top
  maLine(x1, y2, x2, y2); // bottom
  maLine(x1, y1, x1, y2); // left
  maLine(x2, y1, x2, y2); // right
}

void GraphicScreen::drawRectFilled(int x1, int y1, int x2, int y2) {
  drawInto();
  maFillRect(x1, y1, x2 - x1, y2 - y1);
}

// returns the color of the pixel at the given xy location
int GraphicScreen::getPixel(int x, int y) {
  MARect rc;
  rc.left = x;
  rc.top = y;
  rc.width = 1;
  rc.height = 1;
  int data[1];
  int result;

  if (x < 0 || y < 0) {
    rc.left = x < 0 ? -x : x;
    rc.top = y < 0 ? -y : y;
    drawBase(false);
    maGetImageData(HANDLE_SCREEN, &data, &rc, 1);
  } else {
    maGetImageData(_image, &data, &rc, 1);
  }
  result = -(data[0] & 0x00FFFFFF);
  return result;
}

// extend the image to allow for additional content on the newline
void GraphicScreen::imageAppend(MAHandle newImage) {
  MARect srcRect;
  MAPoint2d dstPoint;

  srcRect.left = 0;
  srcRect.top = 0;
  srcRect.width = _imageWidth;
  srcRect.height = _imageHeight;
  dstPoint.x = 0;
  dstPoint.y = 0;

  maSetDrawTarget(newImage);
  maDrawImageRegion(_image, &srcRect, &dstPoint, TRANS_NONE);

  // clear the new segment
  maSetColor(_bg);
  maFillRect(0, _imageHeight, _imageWidth, _imageHeight + _height);
  _imageHeight += _height;

  // cleanup the old image
  maDestroyPlaceholder(_image);
  _image = newImage;
}

// scroll back the image to allow for additioal content on the newline
void GraphicScreen::imageScroll() {
  MAHandle newImage = maCreatePlaceholder();
  int newHeight = _imageHeight;
  if (maCreateDrawableImage(newImage, _imageWidth, newHeight) == RES_OK) {
    MARect srcRect;
    MAPoint2d dstPoint;
    int scrollBack = _height;
    int copiedHeight = _imageHeight - scrollBack;

    srcRect.left = 0;
    srcRect.top = scrollBack;
    srcRect.width = _imageWidth;
    srcRect.height = copiedHeight;
    dstPoint.x = 0;
    dstPoint.y = 0;

    maSetDrawTarget(newImage);
    maDrawImageRegion(_image, &srcRect, &dstPoint, TRANS_NONE);

    // clear the new segment
    maSetColor(_bg);
    maFillRect(0, copiedHeight, _imageWidth, scrollBack);

    // cleanup the old image
    maDestroyPlaceholder(_image);
    _image = newImage;
    _scrollY -= scrollBack;
    _curY -= scrollBack;
  } else {
    // unable to create duplicate
    maDestroyPlaceholder(newImage);
    clear();
  }
}

// handles the \n character
void GraphicScreen::newLine(int lineHeight) {
  lineHeight += _linePadding;
  _linePadding = 0;
  _curX = INITXY;
  if (_height < MAX_HEIGHT) {
    int offset = _curY + (lineHeight * 2);
    if (offset >= _height) {
      if (offset >= _imageHeight) {
        // extend the base image by another page size
        MAHandle newImage = maCreatePlaceholder();
        int newHeight = _imageHeight + _height;
        if (maCreateDrawableImage(newImage, _imageWidth, newHeight) != RES_OK) {
          // maximum image size reached
          maDestroyPlaceholder(newImage);
          imageScroll();
          lineHeight = 0;
        } else {
          imageAppend(newImage);
        }
      }
      _scrollY += lineHeight;
    }
    _curY += lineHeight;
  } else {
    // overflow
    clear();
  }
}

int GraphicScreen::print(const char *p, int lineHeight, bool allChars) {
  if (_curX + _charWidth >= _width - 1) {
    newLine(lineHeight);
  }

  int cx = _curX;
  int numChars = Screen::print(p, lineHeight);

  // erase the background
  maSetColor(_invert ? _fg : _bg);
  maFillRect(cx, _curY, _curX-cx, lineHeight);

  // draw the text buffer
  maSetColor(_invert ? _bg : _fg);
  maDrawText(cx, _curY, p, numChars);

  if (_underline) {
    maLine(cx, _curY + lineHeight - 2, _curX, _curY + lineHeight - 2);
  }

  return numChars;
}

// reset the current drawing variables
void GraphicScreen::reset(int fontSize) {
  Screen::reset(fontSize);
  _curXSaved = 0;
  _curYSaved = 0;
  _invert = false;
  _underline = false;
  _bold = false;
  _italic = false;
}

// update the widget to new dimensions
void GraphicScreen::resize(int newWidth, int newHeight, int oldWidth,
                           int oldHeight, int lineHeight) {
  logEntered();
  bool fullscreen = ((_width - _x) == oldWidth && (_height - _y) == oldHeight);
  if (fullscreen && (newWidth > _imageWidth || newHeight > _imageHeight)) {
    // screen is larger than existing virtual size
    MARect srcRect;
    MAPoint2d dstPoint;
    MAHandle newImage = maCreatePlaceholder();
    int newImageWidth = MAX(newWidth, _imageWidth);
    int newImageHeight = MAX(newHeight, _imageHeight);

    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.width = MIN(_imageWidth, newImageWidth);
    srcRect.height = MIN(_imageHeight, newImageHeight);
    dstPoint.x = 0;
    dstPoint.y = 0;

    if (maCreateDrawableImage(newImage, newImageWidth, newImageHeight) == RES_OK) {
      maSetDrawTarget(newImage);
      maSetColor(_bg);
      maFillRect(0, 0, newImageWidth, newImageHeight);
      maDrawImageRegion(_image, &srcRect, &dstPoint, TRANS_NONE);
      maDestroyPlaceholder(_image);
    } else {
      // cannot resize - alert and abort
      deviceLog("Failed to resize to %d %d", newImageWidth, newImageHeight);
      abort();
    }

    _image = newImage;
    _imageWidth = newImageWidth;
    _imageHeight = newImageHeight;

    if (_curY >= _imageHeight) {
      _curY = _height - lineHeight;
    }
    if (_curX >= _imageWidth) {
      _curX = 0;
    }
  }
  _scrollY = 0;
  _width = newWidth;
  _height = newHeight;
  if (!fullscreen) {
    drawBase(false);
  }
  layoutInputs(newWidth, newHeight);
}

void GraphicScreen::updateFont(int size) {
  setFont(_bold, _italic, size > 0 ? size : _fontSize);
}

// handles the given escape character. Returns whether the font has changed
bool GraphicScreen::setGraphicsRendition(const char c, int escValue, int lineHeight) {
  switch (c) {
  case 'K':
    maSetColor(_bg);            // \e[K - clear to eol
    maFillRect(_curX, _curY, _width - _curX, lineHeight);
    break;
  case 'G':                    // move to column
    _curX = escValue * _charWidth;
    break;
  case 's':                    // save cursor position
    _curYSaved = _curX;
    _curXSaved = _curY;
    break;
  case 'u':                    // restore cursor position
    _curX = _curYSaved;
    _curY = _curXSaved;
    break;
  case ';':                    // fallthru
  case 'm':                    // \e[...m - ANSI terminal
    switch (escValue) {
    case 0:                    // reset
      reset(_fontSize);
      break;
    case 1:                    // set bold on
      _bold = true;
      return true;
    case 2:                    // set faint on
      break;
    case 3:                    // set italic on
      _italic = true;
      return true;
    case 4:                    // set underline on
      _underline = true;
      break;
    case 5:                    // set blink on
      break;
    case 6:                    // rapid blink on
      break;
    case 7:                    // reverse video on
      _invert = true;
      break;
    case 8:                    // conceal on
      break;
    case 21:                   // set bold off
      _bold = false;
      return true;
    case 23:
      _italic = false;
      return true;
    case 24:                   // set underline off
      _underline = false;
      break;
    case 27:                   // reverse video off
      _invert = false;
      break;
      // colors - 30..37 foreground, 40..47 background
    case 30:                   // set black fg
      _fg = ansiToMosync(0);
      break;
    case 31:                   // set red fg
      _fg = ansiToMosync(4);
      break;
    case 32:                   // set green fg
      _fg = ansiToMosync(2);
      break;
    case 33:                   // set yellow fg
      _fg = ansiToMosync(6);
      break;
    case 34:                   // set blue fg
      _fg = ansiToMosync(1);
      break;
    case 35:                   // set magenta fg
      _fg = ansiToMosync(5);
      break;
    case 36:                   // set cyan fg
      _fg = ansiToMosync(3);
      break;
    case 37:                   // set white fg
      _fg = ansiToMosync(7);
      break;
    case 40:                   // set black bg
      _bg = ansiToMosync(0);
      break;
    case 41:                   // set red bg
      _bg = ansiToMosync(4);
      break;
    case 42:                   // set green bg
      _bg = ansiToMosync(2);
      break;
    case 43:                   // set yellow bg
      _bg = ansiToMosync(6);
      break;
    case 44:                   // set blue bg
      _bg = ansiToMosync(1);
      break;
    case 45:                   // set magenta bg
      _bg = ansiToMosync(5);
      break;
    case 46:                   // set cyan bg
      _bg = ansiToMosync(3);
      break;
    case 47:                   // set white bg
      _bg = ansiToMosync(15);
      break;
    case 48:                   // subscript on
      break;
    case 49:                   // superscript
      break;
    };
  }
  return false;
}

void GraphicScreen::setPixel(int x, int y, int c) {
  drawInto();
  maSetColor(ansiToMosync(c));
  maPlot(x, y);
}

struct LineShape : Shape {
  LineShape(int x, int y, int w, int h) : Shape(x, y, w, h) {}
  void draw(int ax, int ay, int, int, int) {
    maLine(_x, _y, _width, _height);
  }
};

struct RectShape : Shape {
  RectShape(int x, int y, int w, int h) : Shape(x, y, w, h) {}
  void draw(int ax, int ay, int, int, int) {
    int x1 = _x;
    int y1 = _y;
    int x2 = _x + _width;
    int y2 = _y + _width;
    maLine(x1, y1, x2, y1); // top
    maLine(x1, y2, x2, y2); // bottom
    maLine(x1, y1, x1, y2); // left
    maLine(x2, y1, x2, y2); // right
  }
};

struct RectFilledShape : Shape {
  RectFilledShape(int x, int y, int w, int h) : Shape(x, y, w, h) {}
  void draw(int ax, int ay, int, int, int) {
    maFillRect(_x, _y, _width, _height);
  }
};

//
// Text based screen with a large scrollback buffer
//
TextScreen::TextScreen(int width, int height, int fontSize) :
  Screen(0, 0, width, height, fontSize),
  _over(NULL),
  _inset(0, 0, 0, 0),
  _buffer(NULL),
  _head(0),
  _tail(0),
  _rows(TEXT_ROWS),
  _cols(0) {
}

TextScreen::~TextScreen() {
  delete[] _buffer;
}

void TextScreen::calcTab() {
  Row *line = getLine(_head);  // pointer to current line
  line->tab();
}

bool TextScreen::construct() {
  reset(_fontSize);
  _buffer = new Row[_rows];
  return (_buffer != NULL);
}

//
// clear the screen
//
void TextScreen::clear() {
  _head = _tail = _cols = 0;
  getLine(0)->clear();
  Screen::clear();
}

//
// draw the text
//
void TextScreen::drawBase(bool vscroll, bool update) {
  // prepare escape state variables
  bool bold = false;
  bool italic = false;
  bool underline = false;
  bool invert = false;
  int color = DEFAULT_FOREGROUND;

  // calculate rows to display
  int pageRows = getPageRows();
  int textRows = getTextRows();
  int numRows = textRows < pageRows ? textRows : pageRows;
  int firstRow = _tail + (_scrollY / _charHeight);
  int yoffs = _scrollY % _charHeight; // smooth scrolling offset

  // prevent drawing beyond available text
  if (numRows > textRows - firstRow) {
    numRows = textRows - firstRow;
  }

  if (_over != NULL && _over != this) {
    _over->drawBase(vscroll, false);
  }

  // setup the background colour
  MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN_BUFFER);
  maSetColor(_bg);
  maFillRect(_x, _y, _width, _height);
  maSetColor(color);

  // draw the visible segments
  int pageWidth = 0;
  for (int row = firstRow, rows = 0, py = _y - yoffs;
       rows < numRows;
       row++, rows++, py += _charHeight) {
    Row *line = getLine(row);   // next logical row
    TextSeg *seg = line->_head;
    int px = (_x + INITXY) - _scrollX;
    while (seg != NULL) {
      if (seg->escape(&bold, &italic, &underline, &invert)) {
        setFont(bold, italic, _fontSize);
      } else if (seg->isReset()) {
        reset(_fontSize);
        bold = false;
        italic = false;
        underline = false;
        invert = false;
        color = DEFAULT_FOREGROUND;
        maSetColor(color);
      }
      if (seg->_color != NO_COLOR) {
        color = seg->_color;
        maSetColor(color);
      }
      int width = seg->width();
      if (seg->_str) {
        if (invert) {
          maSetColor(_fg);
          maFillRect(px, py, width, _charHeight);
          maSetColor(_bg);
          maDrawText(px, py, seg->_str, seg->numChars());
          maSetColor(color);
        } else {
          maDrawText(px, py, seg->_str, seg->numChars());
        }
        if (underline) {
          maLine(px, py + _charHeight, width, py + _charHeight);
        }
      }
      px += width;
      seg = seg->_next;
    }
    int rowWidth = line->width();
    if (rowWidth > pageWidth) {
      pageWidth = rowWidth;
    }
  }

  // draw the base components
  drawOverlay(vscroll);
  _dirty = 0;
  maUpdateScreen();
  maSetDrawTarget(currentHandle);
}

void TextScreen::drawLine(int x1, int y1, int x2, int y2) {
  add(new LineShape(x1, y1, x2, y2));
}

void TextScreen::drawRect(int x1, int y1, int x2, int y2) {
  add(new RectShape(x1, y1, x2, y2));
}

void TextScreen::drawRectFilled(int x1, int y1, int x2, int y2) {
  add(new RectFilledShape(x1, y1, x2 - x1, y2 - y1));
}

//
// return a pointer to the specified line of the display.
//
Row *TextScreen::getLine(int pos) {
  if (pos < 0) {
    pos += _rows;
  }
  if (pos > _rows - 1) {
    pos -= _rows;
  }

  return &_buffer[pos];
}

void TextScreen::inset(int x, int y, int w, int h, Screen *over) {
  _x = over->_x;
  _y = over->_y;
  _width = over->_width;
  _height = over->_height;
  _inset._x = x;
  _inset._y = y;
  _inset._width = w;
  _inset._height = h;
  _over = over;
  resize(_width, _height, 0, 0, 0);
}

void TextScreen::newLine(int lineHeight) {
  // scroll by moving logical last line
  if (getTextRows() == _rows) {
    _tail = (_tail + 1 >= _rows) ? 0 : _tail + 1;
  }
  _head = (_head + 1 >= _rows) ? 0 : _head + 1;

  // clear the new line
  Row* line = getLine(_head);
  line->clear();

  lineHeight += _linePadding;
  _linePadding = 0;

  _curX = INITXY;
  _curY += lineHeight;
}

void TextScreen::resize(int newWidth, int newHeight, int, int, int) {
  if (_inset._width != 0 && _inset._height != 0) {
    _x = newWidth * _inset._x / 100;
    _y = newHeight * _inset._y / 100;
    _width = (newWidth * _inset._width / 100) - _x;
    _height = (newHeight * _inset._height / 100) - _y;
  } else {
    _width = newWidth;
    _height = newHeight;
  }
  layoutInputs(newWidth, newHeight);
}

//
// Creates a new segment then prints the line
//
int TextScreen::print(const char *p, int lineHeight, bool allChars) {
  Row *line = getLine(_head);
  TextSeg *segment = new TextSeg();
  line->append(segment);

  int numChars = Screen::print(p, lineHeight, true);

  // Print the next (possible) line of text
  segment->setText(p, numChars);

  // remember the maximum line length
  if (numChars > _cols) {
    _cols = numChars;
  }

  return numChars;
}

//
// performs the ANSI text SGI function.
//
bool TextScreen::setGraphicsRendition(const char c, int escValue, int lineHeight) {
  if (c == ';' || c == 'm') {
    Row *line = getLine(_head);
    TextSeg *segment = line->next();

    if (segment->_flags || segment->_color != NO_COLOR) {
      // avoid overwriting existing flags
      segment = new TextSeg();
      line->append(segment);
    }

    switch (escValue) {
    case 0:
      segment->reset();
      reset(_fontSize);
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
      _fg = segment->_color = ansiToMosync(0);
      break;

    case 31:                     // Red
      _fg = segment->_color = ansiToMosync(4);
      break;

    case 32:                     // Green
      _fg = segment->_color = ansiToMosync(2);
      break;

    case 33:                     // Yellow
      _fg = segment->_color = ansiToMosync(6);
      break;

    case 34:                     // Blue
      _fg = segment->_color = ansiToMosync(1);
      break;

    case 35:                     // Magenta
      _fg = segment->_color = ansiToMosync(5);
      break;

    case 36:                     // Cyan
      _fg = segment->_color = ansiToMosync(3);
      break;

    case 37:                     // White
      _fg = segment->_color = ansiToMosync(7);
      break;
    }
  }
  return false;
}

