// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include <fltk/damage.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/CheckButton.h>

#include "platform/fltk/TtyWidget.h"

//
// TtyWidget constructor
//
TtyWidget::TtyWidget(int x, int y, int w, int h, int numRows) : 
  Group(x, y, w, h, 0) {

  // initialize the buffer
  buffer = new TtyRow[numRows];
  rows = numRows;
  cols = width = 0;
  head = tail = 0;
  markX = markY = pointX = pointY = 0;

  setfont(COURIER, 12);
  scrollLock = false;

  begin();
  // vertical scrollbar scrolls in row units
  vscrollbar = new Scrollbar(w - SCROLL_W, 1, SCROLL_W, h);
  vscrollbar->set_vertical();
  vscrollbar->user_data(this);

  // horizontal scrollbar scrolls in pixel units
  hscrollbar = new Scrollbar(w - HSCROLL_W - SCROLL_W, 1, HSCROLL_W, SCROLL_H);
  vscrollbar->user_data(this);

  end();
}

TtyWidget::~TtyWidget() {
  delete[]buffer;
}

//
// draw the text
//
void TtyWidget::draw() {
  // get the text drawing rectangle
  Rectangle rc = Rectangle(0, 0, w(), h() + 1);
  if (vscrollbar->visible()) {
    rc.move_r(-vscrollbar->w());
  }
  // prepare escape state variables
  bool bold = false;
  bool italic = false;
  bool underline = false;
  bool invert = false;

  // calculate rows to display
  int pageRows = getPageRows();
  int textRows = getTextRows();
  int vscroll = vscrollbar->value();
  int hscroll = hscrollbar->value();
  int numRows = textRows < pageRows ? textRows : pageRows;
  int firstRow = tail + vscroll;        // from start plus scroll offset

  // setup the background colour
  setcolor(color());
  fillrect(rc);
  push_clip(rc);
  setcolor(BLACK);
  drawline(0, 0, w(), 0);
  setcolor(labelcolor());
  setfont(labelfont(), (int)labelsize());

  int pageWidth = 0;
  for (int row = firstRow, rows = 0, y = rc.y() + lineHeight; rows < numRows; row++, rows++, y += lineHeight) {
    TtyRow *line = getLine(row);   // next logical row
    TtyTextSeg *seg = line->head;
    int x = 2 - hscroll;
    while (seg != NULL) {
      if (seg->escape(&bold, &italic, &underline, &invert)) {
        setfont(bold, italic);
      }
      drawSelection(seg, NULL, row, x, y);
      int width = seg->width();
      if (seg->str) {
        if (invert) {
          setcolor(labelcolor());
          fillrect(x, (y - lineHeight) + (int)getdescent(), width, lineHeight);
          setcolor(color());
          drawtext(seg->str, x, y);
          setcolor(labelcolor());
        } else {
          drawtext(seg->str, x, y);
        }
      }
      if (underline) {
        drawline(x, y + 1, x + width, y + 1);
      }
      x += width;
      seg = seg->next;
    }
    int rowWidth = line->width();
    if (rowWidth > pageWidth) {
      pageWidth = rowWidth;
    }
  }

  // draw scrollbar controls
  if (pageWidth > w()) {
    draw_child(*hscrollbar);
  }
  pop_clip();
  draw_child(*vscrollbar);
}

//
// draw the background for selected text
//
void TtyWidget::drawSelection(TtyTextSeg *seg, strlib::String *s, int row, int x, int y) {
  if (markX != pointX || markY != pointY) {
    Rectangle rc(0, y - (int)getascent(), 0, lineHeight);
    int r1 = markY;
    int r2 = pointY;
    int x1 = markX;
    int x2 = pointX;

    if (r1 > r2) {
      r1 = pointY;
      r2 = markY;
      x1 = pointX;
      x2 = markX;
    }
    if (r1 == r2 && x1 > x2) {
      x1 = pointX;
      x2 = markX;
    }
    if (row > r1 && row < r2) {
      // entire row
      rc.x(x);
      rc.w(seg->width());
      if (s) {
        s->append(seg->str);
      }
    } else if (row == r1 && (r2 > r1 || x < x2)) {
      // top selection row
      int i = 0;
      int len = seg->numChars();

      // find start of selection
      while (x < x1 && i < len) {
        x += (int)getwidth(seg->str + (i++), 1);
      }
      rc.x(x);

      // select rest of line when r2>r1
      while ((r2 > r1 || x < x2) && i < len) {
        if (s) {
          s->append(seg->str[i]);
        }
        x += (int)getwidth(seg->str + (i++), 1);
      }
      rc.set_r(x);
    } else if (row == r2) {
      // bottom selection row
      rc.x(x);

      // select rest of line when r2>r1
      int i = 0;
      int len = seg->numChars();
      while (x < x2 && i < len) {
        if (s) {
          s->append(seg->str[i]);
        }
        x += (int)getwidth(seg->str + (i++), 1);
      }
      rc.set_r(x);
    }

    if (!s && !rc.empty()) {
      setcolor(YELLOW);
      fillrect(rc);
      setcolor(labelcolor());
    }
  }
}

//
// process mouse messages
//
int TtyWidget::handle(int e) {
  static bool leftButtonDown = false;
  switch (e) {
  case PUSH:
    if ((!vscrollbar->visible() || !event_inside(*vscrollbar)) &&
        (!hscrollbar->visible() || !event_inside(*hscrollbar))) {
      bool selected = (markX != pointX || markY != pointY);
      if (selected && event_button() == RightButton) {
        // right click to copy selection
        copySelection();
      }
      markX = pointX = event_x();
      markY = pointY = rowEvent();
      if (selected) {
        // draw end selection
        redraw(DAMAGE_HIGHLIGHT);
      }
      leftButtonDown = true;
      return 1;                 // become belowmouse to receive RELEASE event
    }
    break;

  case DRAG:
  case MOVE:
    if (leftButtonDown) {
      pointX = event_x();
      pointY = rowEvent();
      redraw(DAMAGE_HIGHLIGHT);
      if (vscrollbar->visible()) {
        // drag to scroll up or down
        int value = vscrollbar->value();
        if (event_y() < 0 && value > 0) {
          vscrollbar->value(value - 1);
        } else if ((event_y() > h()) && (value + getPageRows() < getTextRows())) {
          vscrollbar->value(value + 1);
        }
      }
    }
    return 1;

  case RELEASE:
    leftButtonDown = false;
    return 1;

  case MOUSEWHEEL:
    if (vscrollbar->visible()) {
      return vscrollbar->handle(e);
    }
    break;
  }

  return Group::handle(e);
}

//
// update scrollbar positions
//
void TtyWidget::layout() {
  int pageRows = getPageRows();
  int textRows = getTextRows();
  int hscrollX = w() - HSCROLL_W;
  int hscrollW = w() - 4;

  if (textRows > pageRows && h() > SCROLL_W) {
    vscrollbar->set_visible();
    int value = vscrollbar->value();
    if (value + pageRows > textRows) {
      // prevent value from extending beyond the buffer range
      value = textRows - pageRows;
    }
    vscrollbar->resize(w() - SCROLL_W, 1, SCROLL_W, h());
    vscrollbar->value(value, pageRows, 0, textRows);
    hscrollX -= SCROLL_W;
    hscrollW -= SCROLL_W;
  } else {
    vscrollbar->clear_visible();
    vscrollbar->value(0);
  }

  if (width > hscrollW) {
    hscrollbar->set_visible();
    hscrollbar->resize(hscrollX, 1, HSCROLL_W, SCROLL_H);
    hscrollbar->value(hscrollbar->value(), hscrollW, 0, width);
  } else {
    hscrollbar->clear_visible();
    hscrollbar->value(0);
  }
}

//
// copy selected text to the clipboard
//
bool TtyWidget::copySelection() {
  int hscroll = hscrollbar->value();
  bool bold = false;
  bool italic = false;
  bool underline = false;
  bool invert = false;
  int r1 = markY;
  int r2 = pointY;

  if (r1 > r2) {
    r1 = pointY;
    r2 = markY;
  }

  strlib::String selection;

  for (int row = r1; row <= r2; row++) {
    TtyRow *line = getLine(row);   // next logical row
    TtyTextSeg *seg = line->head;
    int x = 2 - hscroll;
    strlib::String rowText;
    while (seg != NULL) {
      if (seg->escape(&bold, &italic, &underline, &invert)) {
        setfont(bold, italic);
      }
      drawSelection(seg, &rowText, row, x, 0);
      x += seg->width();
      seg = seg->next;
    }
    if (rowText.length()) {
      selection.append(rowText);
      selection.append("\n");
    }
  }

  bool result = selection.length() > 0;
  if (result) {
    const char *copy = selection.c_str();
    fltk::copy(copy, strlen(copy), true);
  }
  return result;
}

//
// clear the screen
//
void TtyWidget::clearScreen() {
  head = tail = 0;
  cols = width = 0;
  markX = markY = pointX = pointY = 0;
  getLine(0)->clear();
  vscrollbar->value(0);
  vscrollbar->hide();
  hscrollbar->value(0);
  hscrollbar->hide();
  redraw();
}

//
// process incoming text
//
void TtyWidget::print(const char *str) {
  int strLength = strlen(str);
  TtyRow *line = getLine(head);    // pointer to current line

  // need the current font set to calculate text widths
  fltk::setfont(labelfont(), labelsize());

  // scan the text, handle any special characters, and display the rest.
  for (int i = 0; i < strLength; i++) {
    // check for telnet IAC codes
    switch (str[i]) {
    case '\r':                 // return
      // move to the start of the line
      break;

    case '\a':
      // beep!
      break;

    case '\n':                 // new line
      // scroll by moving logical last line
      if (getTextRows() == rows) {
        tail = (tail + 1 >= rows) ? 0 : tail + 1;
      }
      head = (head + 1 >= rows) ? 0 : head + 1;

      // clear the new line
      line = getLine(head);
      line->clear();
      break;

    case '\b':                 // backspace
      break;

    case '\t':
      line->tab();
      break;

    case '\xC':
      clearScreen();
      break;

    default:
      i += processLine(line, &str[i]);
    }                           // end case
  }

  if (!scrollLock) {
    vscrollbar->value(getTextRows() - getPageRows());
  }
  // schedule a layout and redraw
  relayout();
  redraw();
}

//
// return a pointer to the specified line of the display.
//
TtyRow *TtyWidget::getLine(int pos) {
  if (pos < 0) {
    pos += rows;
  }
  if (pos > rows - 1) {
    pos -= rows;
  }

  return &buffer[pos];
}

//
// interpret ANSI escape codes in linePtr and return number of chars consumed
//
int TtyWidget::processLine(TtyRow *line, const char *linePtr) {
  TtyTextSeg *segment = new TtyTextSeg();
  line->append(segment);

  const char *linePtrStart = linePtr;

  // Determine if we are at an end-of-line or an escape
  bool escaped = false;
  if (*linePtr == '\033') {
    linePtr++;

    if (*linePtr == '[') {
      escaped = true;
      linePtr++;
    }

    if (escaped) {
      int param = 0;
      while (*linePtr != '\0' && escaped) {
        // Walk the escape sequence
        switch (*linePtr) {
        case 'm':
          escaped = false;      // fall through

        case ';':              // Parameter seperator
          setGraphicsRendition(segment, param);
          param = 0;
          break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          // Numeric OK; continue till delimeter or illegal char
          param = (param * 10) + (*linePtr - '0');
          break;

        default:               // Illegal character - reset
          segment->flags = 0;
          escaped = false;
          break;
        }
        linePtr += 1;
      }                         // while escaped and not null
    }
  }

  const char *linePtrNext = linePtr;

  // Walk the line of text until an escape char or end-of-line
  // is encountered.
  while (*linePtr > 31) {
    linePtr++;
  }

  // Print the next (possible) line of text
  if (*linePtrNext != '\0' && linePtrNext != linePtr) {
    segment->setText(linePtrNext, (linePtr - linePtrNext));

    // save max rows encountered
    int lineWidth = line->width();
    if (lineWidth > width) {
      width = lineWidth;
    }

    int lineChars = line->numChars();
    if (lineChars > cols) {
      cols = lineChars;
    }
  }
  // return the number of eaten chars (less 1)
  return linePtr == linePtrStart ? 0 : (linePtr - linePtrStart) - 1;
}

//
// performs the ANSI text SGI function.
//
void TtyWidget::setGraphicsRendition(TtyTextSeg *segment, int c) {
  switch (c) {
  case 0:
    segment->reset();
    break;

  case 1:                      // Bold on
    segment->set(TtyTextSeg::BOLD, true);
    break;

  case 2:                      // Faint on
    segment->set(TtyTextSeg::BOLD, false);
    break;

  case 3:                      // Italic on
    segment->set(TtyTextSeg::ITALIC, true);
    break;

  case 4:                      // Underscrore
    segment->set(TtyTextSeg::UNDERLINE, true);
    break;

  case 7:                      // reverse video on
    segment->set(TtyTextSeg::INVERT, true);
    break;

  case 21:                     // set bold off
    segment->set(TtyTextSeg::BOLD, false);
    break;

  case 23:
    segment->set(TtyTextSeg::ITALIC, false);
    break;

  case 24:                     // set underline off
    segment->set(TtyTextSeg::UNDERLINE, false);
    break;

  case 27:                     // reverse video off
    segment->set(TtyTextSeg::INVERT, false);
    break;

  case 30:                     // Black
    segment->color = BLACK;
    break;

  case 31:                     // Red
    segment->color = RED;
    break;

  case 32:                     // Green
    segment->color = GREEN;
    break;

  case 33:                     // Yellow
    segment->color = YELLOW;
    break;

  case 34:                     // Blue
    segment->color = BLUE;
    break;

  case 35:                     // Magenta
    segment->color = MAGENTA;
    break;

  case 36:                     // Cyan
    segment->color = CYAN;
    break;

  case 37:                     // White
    segment->color = WHITE;
    break;
  }
}

//
// update the current drawing font
//
void TtyWidget::setfont(bool bold, bool italic) {
  Font *font = labelfont();
  if (bold) {
    font = font->bold();
  }
  if (italic) {
    font = font->italic();
  }
  fltk::setfont(font, labelsize());
}

//
// update the current drawing font and remember the face/size
//
void TtyWidget::setfont(Font *font, int size) {
  if (font) {
    labelfont(font);
  }
  if (size) {
    labelsize(size);
  }
  fltk::setfont(labelfont(), labelsize());
  lineHeight = (int)(getascent() + getdescent());
}

