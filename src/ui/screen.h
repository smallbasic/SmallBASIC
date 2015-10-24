// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SCREEN_H
#define SCREEN_H

#include <config.h>

#include "lib/maapi.h"
#include "ui/strlib.h"
#include "ui/utils.h"
#include "ui/shape.h"
#include "ui/image.h"
#include "ui/inputs.h"

using namespace strlib;

#define LINE_SPACING 0
#define INITXY 2
#define NO_COLOR -1

struct Screen : public Shape {
  Screen(int x, int y, int width, int height, int fontSize);
  virtual ~Screen();

  virtual void calcTab() = 0;
  virtual bool construct() = 0;
  virtual void clear();
  virtual void drawBase(bool vscroll, bool update=true) = 0;
  virtual void drawInto(bool background=false);
  virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
  virtual void drawRect(int x1, int y1, int x2, int y2) = 0;
  virtual void drawRectFilled(int x1, int y1, int x2, int y2) = 0;
  virtual int  getPixel(int x, int y) = 0;
  virtual void newLine(int lineHeight) = 0;
  virtual int  print(const char *p, int lineHeight, bool allChars=false);
  virtual bool setGraphicsRendition(const char c, int escValue, int lineHeight) = 0;
  virtual void setPixel(int x, int y, int c) = 0;
  virtual void reset(int fontSize);
  virtual void resize(int newWidth, int newHeight, int oldWidth,
                      int oldHeight, int lineHeight) = 0;
  virtual void updateFont() = 0;
  virtual int  getMaxHScroll() = 0;

  void add(Shape *button);
  void addImage(ImageDisplay &image);
  int  ansiToMosync(long c);
  void drawLabel();
  void drawShape(Shape *button);
  void drawOverlay(bool vscroll);
  FormInput *getMenu(FormInput *prev, int px, int py);
  FormInput *getNextField(FormInput *field);
  void getScroll(int &x, int &y) { x = _scrollX; y = _scrollY; }
  bool overlaps(int px, int py);
  void remove(Shape *button);
  void removeImage(unsigned imageId);
  void removeInput(FormInput *input);
  void removeInputs() { _inputs.removeAll(); }
  void replaceFont(int type = FONT_TYPE_MONOSPACE);
  void resetScroll() { _scrollX = 0; _scrollY = 0; }
  void setColor(long color);
  void setDirty() { if (!_dirty) { _dirty = maGetMilliSecondCount(); } }
  void setFont(bool bold, bool italic, int size);
  void setScroll(int x, int y) { _scrollX = x; _scrollY = y; }
  void setTextColor(long fg, long bg);
  void updateInputs(var_p_t form);

  MAHandle _font;
  int _fontSize;
  int _fontStyle;
  int _charWidth;
  int _charHeight;
  int _scrollX;
  int _scrollY;
  int _bg, _fg;
  int _curX;
  int _curY;
  int _dirty;
  int _linePadding;
  String _label;
  strlib::List<Shape *> _shapes;
  strlib::List<FormInput *> _inputs;
  strlib::List<ImageDisplay *> _images;
};

struct GraphicScreen : public Screen {
  GraphicScreen(int width, int height, int fontSize);
  virtual ~GraphicScreen();

  void calcTab();
  bool construct();
  void clear();
  void drawBase(bool vscroll, bool update=true);
  void drawInto(bool background=false);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  void imageScroll();
  void imageAppend(MAHandle newImage);
  void newLine(int lineHeight);
  int  print(const char *p, int lineHeight, bool allChars=false);
  void reset(int fontSize);
  bool setGraphicsRendition(const char c, int escValue, int lineHeight);
  void setPixel(int x, int y, int c);
  void resize(int newWidth, int newHeight, int oldWidth,
              int oldHeight, int lineHeight);
  void updateFont() { setFont(_bold, _italic, _fontSize); }
  int  getPixel(int x, int y);
  int  getMaxHScroll() { return 0; }

  MAHandle _image;
  bool _underline;
  bool _invert;
  bool _bold;
  bool _italic;
  int _imageWidth;
  int _imageHeight;
  int _curYSaved;
  int _curXSaved;
  int _tabSize;
};

struct TextSeg {
  enum {
    BOLD = 0x00000001,
    ITALIC = 0x00000002,
    UNDERLINE = 0x00000004,
    INVERT = 0x00000008,
    RESET = 0x00000010,
  };

  // create a new segment
  TextSeg() :
    _str(0),
    _flags(0),
    _color(NO_COLOR),
    _next(0) {}

  ~TextSeg() {
    if (_str) {
      delete[]_str;
    }
  }

  // sets the reset flag
  void reset() {
    set(RESET, true);
  }

  // returns whether the reset flag is set
  bool isReset() const {
    return set(RESET);
  }

  void setText(const char *str, int n) {
    if ((!str || !n)) {
      this->_str = 0;
    } else {
      this->_str = new char[n + 1];
      strncpy(this->_str, str, n);
      this->_str[n] = 0;
    }
  }

  // create a string of n spaces
  void tab(int n) {
    this->_str = new char[n + 1];
    memset(this->_str, ' ', n);
    this->_str[n] = 0;
  }

  // set the flag value
  void set(int f, bool value) {
    if (value) {
      _flags |= f;
    } else {
      _flags &= ~f;
    }
    _flags |= (f << 16);
  }

  // return whether the flag was set (to true or false)
  bool set(int f) const {
    return (_flags & (f << 16));
  }

  // return the flag value if set, otherwise return value
  bool get(int f, bool *value) const {
    bool result = *value;
    if (_flags & (f << 16)) {
      result = (_flags & f);
    }
    return result;
  }

  // width of this segment in pixels
  int width() const {
    MAExtent textSize = maGetTextSize(_str);
    return EXTENT_X(textSize);
  }

  // number of chars in this segment
  int numChars() const {
    return !_str ? 0 : strlen(_str);
  }

  // update font and state variables when set in this segment
  bool escape(bool *bold, bool *italic, bool *underline, bool *invert) {
    *bold = get(BOLD, bold);
    *italic = get(ITALIC, italic);
    *underline = get(UNDERLINE, underline);
    *invert = get(INVERT, invert);
    return set(BOLD) || set(ITALIC);
  }

  char *_str;
  int _flags;
  int _color;
  TextSeg *_next;
};

struct Row {
  Row() : _head(0) {}
  ~Row() {
    clear();
  }

  // append a segment to this row
  void append(TextSeg *node) {
    if (!_head) {
      _head = node;
    } else {
      tail(_head)->_next = node;
    }
    node->_next = 0;
  }

  // clear the contents of this row
  void clear() {
    remove(_head);
    _head = 0;
  }

  TextSeg *next() {
    TextSeg *result = _head;
    if (!result) {
      result = new TextSeg();
      append(result);
    }
    return result;
  }

  // number of characters in this row
  int numChars() const {
    return numChars(this->_head);
  }

  int numChars(TextSeg *next) const {
    int n = 0;
    if (next) {
      n = next->numChars() + numChars(next->_next);
    }
    return n;
  }

  void remove(TextSeg *next) {
    if (next) {
      remove(next->_next);
      delete next;
    }
  }

  // move to the tab position
  void tab() {
    int tabSize = 6;
    int num = numChars(this->_head);
    int pos = tabSize - (num % tabSize);
    if (pos) {
      TextSeg *next = new TextSeg();
      next->tab(pos);
      append(next);
    }
  }

  TextSeg *tail(TextSeg *next) {
    return !next->_next ? next : tail(next->_next);
  }

  int width() const {
    return width(this->_head);
  }

  int width(TextSeg *next) const {
    int n = 0;
    if (next) {
      n = next->width() + width(next->_next);
    }
    return n;
  }

  TextSeg *_head;
};

struct TextScreen : public Screen {
  TextScreen(int width, int height, int fontSize);
  virtual ~TextScreen();

  void calcTab();
  bool construct();
  void clear();
  void drawBase(bool vscroll, bool update=true);
  void drawText(const char *text, int len, int x, int lineHeight);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  int  getPixel(int x, int y) { return 0; }
  void inset(int x, int y, int w, int h, Screen *over);
  void newLine(int lineHeight);
  int  print(const char *p, int lineHeight, bool allChars=false);
  void resize(int newWidth, int newHeight, int oldWidth,
              int oldHeight, int lineHeight);
  bool setGraphicsRendition(const char c, int escValue, int lineHeight);
  void setOver(Screen *over) { _over = over; }
  void setPixel(int x, int y, int c) {}
  void updateFont() {}
  int  getMaxHScroll() { return (_cols * _charWidth) - w(); }

private:
  Row *getLine(int ndx);

  // returns the number of display text rows held in the buffer
  int getTextRows() {
    return 1 + ((_head >= _tail) ? (_head - _tail) : _head + (_rows - _tail));
  }

  // returns the number of rows available for display
  int getPageRows() {
    return (_height - 1) / _charHeight;
  }

  Screen *_over;     // inset over screen
  Shape _inset;      // relative screen size
  Row *_buffer;      // buffer management
  int _head;         // current head of buffer
  int _tail;         // buffer last line
  int _rows;         // total number of rows - size of buffer
  int _cols;         // maximum number of characters in a row
};

#endif
