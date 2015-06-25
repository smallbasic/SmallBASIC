// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ui/textedit.h"
#include "ui/inputs.h"
#include "ui/utils.h"

#define STB_TEXTEDIT_IS_SPACE(ch) isspace(ch)
#define STB_TEXTEDIT_IMPLEMENTATION
#include "lib/stb_textedit.h"

#define GROW_SIZE 128

//
// EditBuffer
//
EditBuffer::EditBuffer(TextEditInput *in, const char *text) :
  _buffer(NULL),
  _len(0),
  _size(0),
  _in(in) {
  if (text != NULL && text[0]) {
    _len = strlen(text);
    _size = _len + 1;
    _buffer = (char *)malloc(_size);
    memcpy(_buffer, text, _len);
    _buffer[_len] = '\0';
  }
}

EditBuffer::~EditBuffer() {
  delete _buffer;
  _buffer = NULL;
  _len = _size = 0;
}

int EditBuffer::deleteChars(int pos, int num) {
  memmove(&_buffer[pos], &_buffer[pos+num], _len - (pos + num));
  _len -= num;
  return 1;
}

int EditBuffer::insertChars(int pos, char *newtext, int num) {
  int required = _len + num + 1;
  if (required >= _size) {
    _size += (required + GROW_SIZE);
    _buffer = (char *)realloc(_buffer, _size);
  }
  memmove(&_buffer[pos + num], &_buffer[pos], _len - pos);
  memcpy(&_buffer[pos], newtext, num);
  _len += num;
  _buffer[_len] = '\0';
  return 1;
}

//
// TextEditInput
//
TextEditInput::TextEditInput(const char *text, int chW, int chH,
                             int x, int y, int w, int h) :
  FormInput(x, y, w, h),
  _buf(this, text),
  _charWidth(chW),
  _charHeight(chH),
  _marginWidth(0),
  _scroll(0),
  _controlMode(false) {
  stb_textedit_initialize_state(&_state, false);
}

void TextEditInput::close() {
}

void TextEditInput::draw(int x, int y, int w, int h, int chw) {
  StbTexteditRow r;
  int len = _buf._len;
  int i = 0;
  int baseY = 0;
  int cursorX = x;
  int cursorY = y;
  int row = 0;
  int selectStart = MIN(_state.select_start, _state.select_end);
  int selectEnd = MAX(_state.select_start, _state.select_end);

  maSetColor(getBackground(GRAY_BG_COL));
  maFillRect(x, y, _width, _height);
  maSetColor(_fg);
  
  while (i < len) {
    layout(&r, i);
    if (baseY + r.ymax > _height) {
      break;
    }

    if (row++ >= _scroll) {
      int numChars = r.num_chars;
      if (numChars > 0 && _buf._buffer[i + r.num_chars - 1] == STB_TEXTEDIT_NEWLINE) { 
        numChars--;
      }
      
      if (numChars) {
        if (selectStart != selectEnd && i >= selectStart && i < selectEnd) {
          // draw selection
          int start = selectStart - i;
          int baseX = _marginWidth;
          if (start > 0) {
            // initial non-selected chars
            maDrawText(x, y + baseY, _buf._buffer + i, start);
            baseX += start * _charWidth;
          } else if (start < 0) {
            // started on previous row
            selectStart = i;
            start = 0;
          }

          int count = selectEnd - selectStart;
          if (count > numChars) {
            count = numChars;
          }
          
          maSetColor(_fg);
          maFillRect(x + baseX, y + baseY, count * _charWidth, _charHeight);
          maSetColor(getBackground(GRAY_BG_COL));
          maDrawText(x + baseX, y + baseY, _buf._buffer + i + start, count);
          maSetColor(_fg);

          if (count < numChars) {
            // trailing non-selected chars
            baseX += count * _charWidth;
            start += count;
            maDrawText(x + baseX, y + baseY, _buf._buffer + i + start, numChars - count);
          }
        } else {
          maDrawText(x + _marginWidth, y + baseY, _buf._buffer + i, numChars);
        }
      }
      
      if ((_state.cursor >= i && _state.cursor < i + r.num_chars) ||
          (i + r.num_chars == _buf._len && _state.cursor == _buf._len)) {
        // set cursor position
        if (_state.cursor == i + r.num_chars && 
            _buf._buffer[i + r.num_chars - 1] == STB_TEXTEDIT_NEWLINE) {
          // place cursor on newline
          cursorX = x;
          cursorY = y + baseY + _charHeight;
        } else {
          cursorX = x + ((_state.cursor - i) * chw);
          cursorY = y + baseY;
        }
      }
      baseY += _charHeight;
    }
    i += r.num_chars;
  }

  // draw cursor
  maFillRect(cursorX + _marginWidth, cursorY, chw, _charHeight);
  if (_state.cursor < _buf._len) {
    maSetColor(getBackground(GRAY_BG_COL));
    maDrawText(cursorX + _marginWidth, cursorY, _buf._buffer + _state.cursor, 1);
  }
}

bool TextEditInput::edit(int key, int screenWidth, int charWidth) {
  bool result = true;
  switch (key) {
  case SB_KEY_PGUP:
    break;
  case SB_KEY_PGDN:
    break;
  case -1:
    result = false;
    break;
  default:
    stb_textedit_key(&_buf, &_state, key);
    _scroll = getScroll();
  }
  return result;
}

int TextEditInput::getControlKey(int key) {
  return 0;
}

void TextEditInput::setText(const char *text) {
}

void TextEditInput::setFocus() {
}

void TextEditInput::clicked(int x, int y, bool pressed) {
  stb_textedit_click(&_buf, &_state, x, y);
}

void TextEditInput::updateField(var_p_t form) {
}

bool TextEditInput::selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) {
  stb_textedit_drag(&_buf, &_state, pt.x, pt.y);
  return 1;
}

char *TextEditInput::copy(bool cut) {
  if (cut) {
    stb_textedit_cut(&_buf, &_state);
  }
  return 0;
}

void TextEditInput::cut() {
  stb_textedit_cut(&_buf, &_state);
}

void TextEditInput::paste(char *text) {
  stb_textedit_paste(&_buf, &_state, text, strlen(text));
}

void TextEditInput::layout(StbTexteditRow *row, int start) const {
  int i = start;
  int len = _buf._len;
  int x2 = _width - _charWidth - _marginWidth;
  row->x1 = 0;
  row->num_chars = 0;

  // advance to newline or rectangle edge
  while (i < len
         && (int)row->x1 < x2
         && _buf._buffer[i] != STB_TEXTEDIT_NEWLINE) {
    row->x1 += _charWidth;
    row->num_chars++;
    i++;
  }
  if (_buf._buffer[i] == STB_TEXTEDIT_NEWLINE) {
    // advance over newline
    row->num_chars++;
  }
  row->x0 = 0.0f;
  row->ymin = 0.0f;
  row->ymax = row->baseline_y_delta = _charHeight;
}

int TextEditInput::getScroll() const {
  StbTexteditRow r;
  int len = _buf._len;
  int i = 0;
  int row = 0;
  
  while (i < len) {
    layout(&r, i);
    row++;
    if (_state.cursor == i + r.num_chars && 
        _buf._buffer[i + r.num_chars - 1] == STB_TEXTEDIT_NEWLINE) {
      row++;
      break;
    } else if (_state.cursor >= i && _state.cursor < i + r.num_chars) {
      break;
    }
    i += r.num_chars;
  }

  int rows = _height / _charHeight;
  return row < rows ? 0 : (row - rows);
}
