// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FL_TTY_WIDGET
#define FL_TTY_WIDGET

#include <stdlib.h>
#include <string.h>
#include <fltk/Font.h>
#include <fltk/draw.h>
#include <fltk/Group.h>
#include <fltk/Scrollbar.h>

#define SCROLL_W 15
#define SCROLL_H 15
#define HSCROLL_W 80

extern "C" void trace(const char* format, ...);

using namespace fltk;

struct Point {
  int x;
  int y;
};

struct TextSeg {
  enum {
    BOLD = 0x00000001,
    ITALIC = 0x00000002,
    UNDERLINE = 0x00000004,
    INVERSE = 0x00000008,
  };

  // create a new segment
  TextSeg(const char* str, int n) {
    this->str = (!str || !n) ? 0 : strndup(str, n);
    this->flags = 0;
    this->color = NO_COLOR;
  }

  ~TextSeg() {
    if (str) {
      free(str);
    }
  }

  // set the flag value
  void set(int f, bool value) {
    if (value) {
      flags |= f;
    }
    else {
      flags &= ~f;
    }
    flags |= (f << 16);
  }

  // return the flag value if set, otherwise return value
  bool get(int f, bool* value) {
    bool result = *value;
    if (flags & (f << 16)) {
      result = (flags & f);
    }
    return result;
  }

  // width of this segment in pixels
  int width() {
    return !str ? 0 : (int) getwidth(str);
  }
  
  // number of chars in this segment
  int numChars() {
    return !str ? 0 : strlen(str);
  }

  // draw the segment
  int draw(int x, int y, bool* bold, bool* italic, 
           bool* underline, bool* inverse);

  char* str;
  int flags;
  Color color;
  TextSeg* next;
};

struct Row {
  Row(): head(0) {}

  ~Row() {
    clear();
  }

  // append a segment to this row
  void append(TextSeg* node) {
    if (!head) {
      head = node;
    }
    else {
      tail(head)->next = node;
    }
    node->next = 0;
  }

  // clear the contents of this row
  void clear() {
    remove(head);
  }

  // number of characters in this row
  int numChars() {
    return numChars(this->head);
  }

  int numChars(TextSeg* next) {
    int n = 0;
    if (next) {
      n = next->numChars() + numChars(next->next);
    }
    return n;
  }

  void remove(TextSeg* next) {
    if (next) {
      remove(next->next);
      delete next;
    }
  }

  TextSeg* tail(TextSeg* next) {
    return !next->next ? next : tail(next->next);
  }

  int width() {
    return width(this->head);
  }

  int width(TextSeg* next) {
    int n = 0;
    if (next) {
      n = next->width() + width(next->next);
    }
    return n;
  }
  
  TextSeg* head;
};

struct TtyWidget : public Group {
  TtyWidget(int x, int y, int w, int h, int numRows);
  virtual ~TtyWidget();

  // inherited methods
  void draw();
  int handle(int e);
  void layout();

  // public api
  void copySelection();
  void clearScreen();
  void print(const char *str);

private:
  void drawSelection(TextSeg* seg, int row, int x, int y);
  Row* getLine(int ndx);
  int processLine(Row* line, const char* linePtr);
  void setGraphicsRendition(TextSeg* segment, int c);

  // returns the number of display text rows held in the buffer
  int getTextRows() {
    return 1 + ((head >= tail) ? (head - tail) : head + (rows - tail));
  }

  // returns the number of rows available for display
  int getPageRows() {
    return (h() - 1) / lineHeight;
  }

  // buffer management
  Row* buffer;
  int head; // current head of buffer
  int tail; // buffer last line
  int cursor; // current x position at head
  int rows; // total number of rows - size of buffer
  int cols; // maximum number of characters in a row
  int width; // the maximum width of the buffer text in pixels

  // scrollbars
  Scrollbar *vscrollbar;
  Scrollbar *hscrollbar;
  int lineHeight;

  // clipboard handling
  int markX, markY, pointX, pointY;
};

#endif

// $Id$
