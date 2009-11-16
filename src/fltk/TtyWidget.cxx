// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <fltk/damage.h>
#include <fltk/events.h>
#include <fltk/CheckButton.h>

#include "TtyWidget.h"

#define BASE_FONT_SIZE 12
#define BASE_FONT COURIER

//
// TextSeg - a segment of text between escape chars
//
int TextSeg::draw(int x, int y, bool* bold, bool* italic, 
                  bool* underline, bool* inverse) {

  // update state variables when explicitely set in this segment
  *bold = get(BOLD, bold);
  *italic = get(ITALIC, italic);
  *underline = get(UNDERLINE, underline);
  *inverse = get(INVERSE, inverse);

  int next_x = x;

  if (str) {
    if (this->color != NO_COLOR) {
      fltk::setcolor(this->color);
    }

    Font* font = BASE_FONT;
    if (*bold) {
      font = font->bold();
    }
    if (*italic) {
      font = font->italic();
    }
    fltk::setfont(font, BASE_FONT_SIZE);

    drawtext(str, x, y);
    next_x += width();
  }

  if (*underline) {
    int ascent = (int)getascent();
    drawline(x, y+ascent+1, x+next_x, y+ascent+1);
  }

  return next_x;
}

void scrollbar_callback(Widget* scrollBar, void* ttyWidget) {
  ((TtyWidget *) ttyWidget)->redraw(DAMAGE_ALL);
}

//
// initialize variables used by I/O window.
//
TtyWidget::TtyWidget(int x, int y, int w, int h, int numRows) :
  Group(x, y, w, h, 0) {

  // initialize the buffer
  buffer = new Row[numRows];
  rows = numRows;
  cols = 0;
  width = 0;
  head = 0;
  tail = 0;
  cursor = 0;

  fltk::setfont(BASE_FONT, BASE_FONT_SIZE);
  lineHeight = (int) (getascent() + getdescent());

  begin();
  vscrollbar = new Scrollbar(w - SCROLL_W, 1, SCROLL_W, h);
  vscrollbar->set_vertical();
  vscrollbar->user_data(this);
  vscrollbar->callback(scrollbar_callback);

  hscrollbar = new Scrollbar(w - HSCROLL_W - SCROLL_W, 1, HSCROLL_W, SCROLL_H);
  vscrollbar->user_data(this);
  vscrollbar->callback(scrollbar_callback);

  end();
}

TtyWidget::~TtyWidget() {
  delete[] buffer;
}

//
// The I/O window paint procedure.
//
void TtyWidget::draw() {
  // get the text drawing rectangle
  Rectangle rc = Rectangle(0, 0, w(), h()+1);
  if (vscrollbar->visible()) {
    rc.move_r(-vscrollbar->w());
  }

  // prepare escape state variables
  bool bold = false;
  bool italic = false;
  bool underline = false;
  bool inverse = false;

  // calculate rows to display
  int pageRows = getPageRows();
  int textRows = getTextRows();
  int vscroll = vscrollbar->value();
  int hscroll = hscrollbar->value();
  int numRows = textRows < pageRows ? textRows : pageRows;
  int firstRow = tail + vscroll;

  // setup the background colour
  setcolor(color());
  fillrect(rc);
  push_clip(rc);
  setcolor(BLACK);
  drawline(0, 0, w(), 0);
  setcolor(labelcolor());

  int pageWidth = 0;
  for (int row = firstRow, rows = 0, y = rc.y() + lineHeight;
       rows < numRows; row++, rows++, y += lineHeight) {
    Row* line = getLine(row); // next logical row
    TextSeg* seg = line->head;
    int x = (rc.x() + 2) - hscroll;
    while (seg != NULL) {
      x += seg->draw(x, y, &bold, &italic, &underline, &inverse);
      x += hscroll;
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
// process messages for the I/O window.
//
int TtyWidget::handle(int e) {
  switch (e) {
  case PUSH:
    break;

  case MOVE:
    break;

  case RELEASE:
    break;
  }
  return Group::handle(e);
}

//
// update the scrollbar position
//
void TtyWidget::layout() {
  int pageRows = getPageRows();
  int textRows = getTextRows();
  int hscrollX = w() - HSCROLL_W;
  int hscrollW = w() - 4;

  if (textRows > pageRows && h() > SCROLL_W) {
    vscrollbar->set_visible();
    vscrollbar->value(vscrollbar->value(), pageRows, 0, textRows);
    vscrollbar->resize(w() - SCROLL_W, 1, SCROLL_W, h());
    hscrollX -= SCROLL_W;
    hscrollW -= SCROLL_W;
  }
  else {
    vscrollbar->clear_visible();
    vscrollbar->value(0);
  }

  if (width > hscrollW) {
    hscrollbar->set_visible();
    hscrollbar->value(hscrollbar->value(), hscrollW, 0, width);
    hscrollbar->resize(hscrollX, 1, HSCROLL_W, SCROLL_H);
  }
  else {
    hscrollbar->clear_visible();
  }

}

//
// clear the screen
//
void TtyWidget::clearScreen() {

}

//
// Process incoming text to Stdio window.
//
void TtyWidget::print(const char *str) {
  int strLength = strlen(str);
  Row* line = getLine(head); // pointer to current line

  // need the current font set to calculate text widths
  fltk::setfont(BASE_FONT, BASE_FONT_SIZE);

  // scan the text, handle any special characters, and display the rest.
  for (int i = 0; i < strLength; i++) {
    // check for telnet IAC codes
    switch (str[i]) {
    case '\r': // return
      // move to the start of the line
      cursor = 0;
      break;

    case '\a':
      // beep!
      break;

    case '\n': // new line
      // scroll by moving logical last line
      head = (head + 1 >= rows) ? 0 : head + 1;
      if (head == tail) {
        tail++;
      }

      // clear the new line
      line = getLine(head);
      line->clear();
      break;

    case '\b': // backspace
      // move back one space
      if (cursor > 0) {
        redraw(DAMAGE_ALL);
      }
      break;

    case '\t':
      cursor += 8;
      break;

    default:
      i += processLine(line, &str[i]);
    } // end case
  }

  vscrollbar->value(getTextRows() - getPageRows());
}

//
// return a pointer to the specified line of the display.
//
Row* TtyWidget::getLine(int pos) {
  if (pos < 0) {
    pos += rows;
  }
  if (pos > rows - 1) {
    pos -= rows;
  }

  return &buffer[pos];
}

//
// Interpret ANSI escape codes in linePtr and return number of chars consumed
//
int TtyWidget::processLine(Row* line, const char* linePtr) {
  const char* linePtrNext = linePtr;

  // Walk the line of text until an escape char or end-of-line
  // is encountered.
  while (*linePtr > 31) {
    linePtr++;
  }

  // Print the next (possible) line of text
  if (*linePtrNext != '\0') {
    TextSeg* segment = new TextSeg(linePtrNext, (linePtr - linePtrNext));
    line->append(segment);

    // save max rows encountered
    int lineWidth = line->width();
    if (lineWidth > width) {
      width = lineWidth;
    }

    int lineChars = line->numChars();
    if (lineChars > cols) {
      cols = lineChars;
    }

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
            escaped = false;  // fall through

          case ';':  // Parameter seperator
            setGraphicsRendition(segment, param);
            param = 0;
            break;

          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
            // Numeric OK; continue till delimeter or illegal char
            param = (param * 10) + (*linePtr - '0');
            break;

          default: // Illegal character - reset
            segment->flags = 0;
            escaped = false;
          }
          linePtr += 1;
        } // while escaped and not null
      }
    }
    else {
      // next special char was not escape
      linePtr--;
    }
  }
  return (linePtr - linePtrNext);
}

//
// performs the ANSI text SGI function.
//
void TtyWidget::setGraphicsRendition(TextSeg* segment, int c) {
  switch (c) {
  case 1:  // Bold on
    segment->set(TextSeg::BOLD, true);
    break;

  case 2:  // Faint on
    segment->set(TextSeg::BOLD, false);
    break;

  case 3:  // Italic on
    segment->set(TextSeg::ITALIC, true);
    break;

  case 4:  // Underscrore
    segment->set(TextSeg::UNDERLINE, true);
    break;

  case 7: // reverse video on
    segment->set(TextSeg::INVERSE, true);
    break;

  case 21: // set bold off
    segment->set(TextSeg::BOLD, false);
    break;

  case 23:
    segment->set(TextSeg::ITALIC, false);
    break;

  case 24: // set underline off
    segment->set(TextSeg::UNDERLINE, false);
    break;

  case 27: // reverse video off
    segment->set(TextSeg::INVERSE, false);
    break;

  case 30: // Black
    segment->color = BLACK;
    break;

  case 31: // Red
    segment->color = RED;
    break;

  case 32: // Green
    segment->color = GREEN;
    break;

  case 33: // Yellow
    segment->color = YELLOW;
    break;

  case 34: // Blue
    segment->color = BLUE;
    break;

  case 35: // Magenta
    segment->color = MAGENTA;
    break;

  case 36: // Cyan
    segment->color = CYAN;
    break;

  case 37: // White
    segment->color = WHITE;
    break;
  }
}

// End of "$Id$".


