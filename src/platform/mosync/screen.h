// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef SCREEN_H
#define SCREEN_H

#include <maapi.h>
#include <MAUtil/String.h>
#include <MAUtil/Vector.h>

#include "platform/mosync/utils.h"

#define INITXY 2
#define LINE_SPACING 4
#define NO_COLOR -1
#define DEFAULT_COLOR  0xa1a1a1
#define GRAY_BG_COL    0x383f42
#define LABEL_TEXT_COL 0xebebeb

using namespace MAUtil;

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

struct Shape {
  Shape(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
  virtual ~Shape() {}
  virtual void draw() {}

  int x, y, width, height;
};

struct Screen : public Shape {
  Screen(int x, int y, int width, int height, int fontSize);
  virtual ~Screen();
  
  virtual void calcTab() = 0;
  virtual bool construct() = 0;
  virtual void clear();
  virtual void draw(bool vscroll);
  virtual void drawInto(bool background=false);
  virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
  virtual void drawRect(int x1, int y1, int x2, int y2) = 0;
  virtual void drawRectFilled(int x1, int y1, int x2, int y2) = 0;
  virtual int getPixel(int x, int y) = 0;
  virtual void newLine(int lineHeight) = 0;
  virtual int print(const char *p, int lineHeight);
  virtual bool setGraphicsRendition(char c, int escValue, int lineHeight) = 0;
  virtual void setPixel(int x, int y, int c) = 0;
  virtual void reset(int fontSize = -1);
  virtual void resize(int newWidth, int newHeight, int oldWidth, 
                      int oldHeight, int lineHeight) = 0;
  virtual void updateFont() = 0;

  int ansiToMosync(long c);
  void add(Shape *button) { shapes.add(button); }
  void remove(Shape *button);
  void setColor(long color);
  void setTextColor(long fg, long bg);
  void setFont(bool bold, bool italic);

  MAHandle font;
  int fontSize;
  int charWidth;
  int charHeight;
  int scrollY;
  int bg, fg;
  int curX;
  int curY;
  int dirty;
  int linePadding;
  Vector <Shape *>shapes;
  String label;
};

struct GraphicScreen : public Screen {
  GraphicScreen(int x, int y, int width, int height, int fontSize);
  virtual ~GraphicScreen();

  void calcTab();
  bool construct();
  void clear();
  void draw(bool vscroll);
  void drawInto(bool background=false);
  void newLine(int lineHeight);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  int print(const char *p, int lineHeight);
  void reset(int fontSize = -1);
  bool setGraphicsRendition(char c, int escValue, int lineHeight);
  void setPixel(int x, int y, int c);
  void resize(int newWidth, int newHeight, int oldWidth, int oldHeight, int lineHeight);
  void updateFont() { setFont(bold, italic); }
  int getPixel(int x, int y);

  MAHandle image;
  bool underline;
  bool invert;
  bool bold;
  bool italic;
  int imageWidth;
  int imageHeight;
  int curYSaved;
  int curXSaved;
  int tabSize;
};

struct TextSeg {
  enum {
    BOLD = 0x00000001,
    ITALIC = 0x00000002,
    UNDERLINE = 0x00000004,
    INVERT = 0x00000008,
  };

  // create a new segment
  TextSeg() :
    str(0),
    flags(0),
    color(NO_COLOR),
    next(0) {}
  
  ~TextSeg() {
    if (str) {
      delete[]str;
    }
  }

  // reset all flags
  void reset() {
    set(BOLD, false);
    set(ITALIC, false);
    set(UNDERLINE, false);
    set(INVERT, false);
  }

  void setText(const char *str, int n) {
    if ((!str || !n)) {
      this->str = 0;
    } else {
      this->str = new char[n + 1];
      strncpy(this->str, str, n);
      this->str[n] = 0;
    }
  }

  // create a string of n spaces
  void tab(int n) {
    this->str = new char[n + 1];
    memset(this->str, ' ', n);
    this->str[n] = 0;
  }

  // set the flag value
  void set(int f, bool value) {
    if (value) {
      flags |= f;
    } else {
      flags &= ~f;
    }
    flags |= (f << 16);
  }

  // return whether the flag was set (to true or false)
  bool set(int f) {
    return (flags & (f << 16));
  }

  // return the flag value if set, otherwise return value
  bool get(int f, bool *value) {
    bool result = *value;
    if (flags & (f << 16)) {
      result = (flags & f);
    }
    return result;
  }

  // width of this segment in pixels
  int width() {
    return get_text_width(str);
  }

  // number of chars in this segment
  int numChars() {
    return !str ? 0 : strlen(str);
  }

  // update font and state variables when set in this segment
  bool escape(bool *bold, bool *italic, bool *underline, bool *invert) {
    *bold = get(BOLD, bold);
    *italic = get(ITALIC, italic);
    *underline = get(UNDERLINE, underline);
    *invert = get(INVERT, invert);

    if (this->color != NO_COLOR) {
      maSetColor(this->color);
    }

    return set(BOLD) || set(ITALIC);
  }

  char *str;
  int flags;
  int color;
  TextSeg *next;
};

struct Row {
  Row() : head(0) {} 
  ~Row() {
    clear();
  }

  // append a segment to this row
  void append(TextSeg *node) {
    if (!head) {
      head = node;
    } else {
      tail(head)->next = node;
    }
    node->next = 0;
  }

  // clear the contents of this row
  void clear() {
    remove(head);
    head = 0;
  }

  // number of characters in this row
  int numChars() {
    return numChars(this->head);
  }

  int numChars(TextSeg *next) {
    int n = 0;
    if (next) {
      n = next->numChars() + numChars(next->next);
    }
    return n;
  }

  void remove(TextSeg *next) {
    if (next) {
      remove(next->next);
      delete next;
    }
  }

  // move to the tab position
  void tab() {
    int tabSize = 6;
    int num = numChars(this->head);
    int pos = tabSize - (num % tabSize);
    if (pos) {
      TextSeg *next = new TextSeg();
      next->tab(pos);
      append(next);
    }
  }

  TextSeg *tail(TextSeg *next) {
    return !next->next ? next : tail(next->next);
  }

  int width() {
    return width(this->head);
  }

  int width(TextSeg *next) {
    int n = 0;
    if (next) {
      n = next->width() + width(next->next);
    }
    return n;
  }

  TextSeg *head;
};

struct TextScreen : public Screen {
  TextScreen(int x, int y, int w, int h, int fontSize);
  virtual ~TextScreen();

  void calcTab();
  bool construct();
  void clear();
  void draw(bool vscroll);
  void drawText(const char *text, int len, int x, int lineHeight);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  int  getPixel(int x, int y) { return 0; }
  void newLine(int lineHeight);
  int  print(const char *p, int lineHeight);
  void resize(int newWidth, int newHeight, int oldWidth, 
              int oldHeight, int lineHeight);
  bool setGraphicsRendition(char c, int escValue, int lineHeight);
  void setPixel(int x, int y, int c) {}
  void updateFont() {}

private:
  Row *getLine(int ndx);

  // returns the number of display text rows held in the buffer
  int getTextRows() {
    return 1 + ((head >= tail) ? (head - tail) : head + (rows - tail));
  }

  // returns the number of rows available for display
  int getPageRows() {
    return (height - 1) / charHeight;
  }

  // buffer management
  Row *buffer;
  int head;         // current head of buffer
  int tail;         // buffer last line
  int rows;         // total number of rows - size of buffer
  int cols;         // maximum number of characters in a row
};

#endif
