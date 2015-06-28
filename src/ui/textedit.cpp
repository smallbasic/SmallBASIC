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

#define THEME_FOREGROUND 0xa7aebc
#define THEME_BACKGROUND 0x272b33
#define THEME_SELECTION_BACKGROUND 0x3d4350
#define THEME_NUMBER_SELECTION_BACKGROUND 0x2b3039
#define THEME_NUMBER_SELECTION 0xabb2c0
#define THEME_NUMBER 0x484f5f
#define THEME_CURSOR 0xa7aebc
#define THEME_CURSOR_BACKGROUND 0x3875ed

//
// EditTheme
//
EditTheme::EditTheme() :
  _color(THEME_FOREGROUND),
  _background(THEME_BACKGROUND),
  _selection_color(THEME_FOREGROUND),
  _selection_background(THEME_SELECTION_BACKGROUND),
  _number_color(THEME_NUMBER),
  _number_selection_color(THEME_FOREGROUND),
  _number_selection_background(THEME_NUMBER_SELECTION_BACKGROUND),
  _cursor_color(THEME_CURSOR),
  _cursor_background(THEME_CURSOR_BACKGROUND) {
}

EditTheme::EditTheme(int fg, int bg) :
  _color(fg),
  _background(bg),
  _selection_color(bg),
  _selection_background(fg),
  _cursor_color(bg),
  _cursor_background(fg) {
}

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
  FormEditInput(x, y, w, h),
  _buf(this, text),
  _theme(NULL),
  _charWidth(chW),
  _charHeight(chH),
  _marginWidth(0),
  _scroll(0) {
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

  maSetColor(_theme->_background);
  maFillRect(x, y, _width, _height);
  maSetColor(_theme->_color);

  while (i < len) {
    layout(&r, i);
    if (baseY + r.ymax > _height) {
      break;
    }

    if (row++ >= _scroll) {
      int numChars = r.num_chars;
      if (numChars > 0 && _buf._buffer[i + r.num_chars - 1] == '\r') {
        numChars--;
      }
      if (numChars > 0 && _buf._buffer[i + r.num_chars - 1] == '\n') {
        numChars--;
      }
      if (selectStart != selectEnd && i + numChars > selectStart && i < selectEnd) {
        if (numChars) {
          // draw selected text
          int begin = selectStart - i;
          int baseX = _marginWidth;
          if (begin > 0) {
            // initial non-selected chars
            maDrawText(x, y + baseY, _buf._buffer + i, begin);
            baseX += begin * _charWidth;
          } else if (begin < 0) {
            // started on previous row
            selectStart = i;
            begin = 0;
          }

          int count = selectEnd - selectStart;
          if (count > numChars - begin) {
            // fill to end of row
            count = numChars - begin;
            numChars = 0;
          }

          maSetColor(_theme->_selection_background);
          maFillRect(x + baseX, y + baseY, count * _charWidth, _charHeight);
          maSetColor(_theme->_selection_color);
          maDrawText(x + baseX, y + baseY, _buf._buffer + i + begin, count);
          maSetColor(_theme->_color);

          int end = numChars - (begin + count);
          if (end) {
            // trailing non-selected chars
            baseX += count * _charWidth;
            maDrawText(x + baseX, y + baseY, _buf._buffer + i + begin + count, end);
          }
        } else {
          // draw empty row selection
          maSetColor(_theme->_selection_background);
          maFillRect(x + _marginWidth, y + baseY, _charWidth / 2, _charHeight);
        }
      } else if (numChars) {
        maDrawText(x + _marginWidth, y + baseY, _buf._buffer + i, numChars);
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
  maSetColor(_theme->_cursor_background);
  maFillRect(cursorX + _marginWidth, cursorY, chw, _charHeight);
  if (_state.cursor < _buf._len) {
    maSetColor(_theme->_cursor_color);
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
  case SB_KEY_ENTER:
    stb_textedit_key(&_buf, &_state, STB_TEXTEDIT_NEWLINE);
    break;
  case -1:
    result = false;
    break;
  default:
    stb_textedit_key(&_buf, &_state, key);
  }
  if (result) {
    updateScroll();
  }
  return result;
}

void TextEditInput::selectAll() {
  _state.select_start = 0;
  _state.select_end = _buf._len;
}

void TextEditInput::setText(const char *text) {
}

void TextEditInput::clicked(int x, int y, bool pressed) {
  if (pressed) {
    stb_textedit_click(&_buf, &_state, x, y);
  }
}

void TextEditInput::updateField(var_p_t form) {
}

bool TextEditInput::updateUI(var_p_t form, var_p_t field) {
  bool updated = (form && field) ? FormInput::updateUI(form, field) : false;
  if (!_theme) {
    if (_fg == DEFAULT_FOREGROUND && _bg == DEFAULT_BACKGROUND) {
      _theme = new EditTheme();
    } else {
      _theme = new EditTheme(_fg, _bg);
    }
    updated = true;
  }
  return updated;
}

bool TextEditInput::selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) {
  stb_textedit_drag(&_buf, &_state, pt.x, pt.y);
  redraw = true;
  return 1;
}

char *TextEditInput::copy(bool cut) {
  if (cut) {
    stb_textedit_cut(&_buf, &_state);
  }
  return 0;
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
         && _buf._buffer[i] != '\r'
         && _buf._buffer[i] != '\n') {
    row->x1 += _charWidth;
    row->num_chars++;
    i++;
  }

  if (_buf._buffer[i] == '\r') {
    // advance over DOS newline
    row->num_chars++;
    i++;
  }
  if (_buf._buffer[i] == '\n') {
    // advance over newline
    row->num_chars++;
  }
  row->x0 = 0.0f;
  row->ymin = 0.0f;
  row->ymax = row->baseline_y_delta = _charHeight;
}

int TextEditInput::cursorRow() const {
  StbTexteditRow r;
  int len = _buf._len;
  int row = 0;

  for (int i = 0; i < len;) {
    layout(&r, i);
    if (_state.cursor == i + r.num_chars &&
        _buf._buffer[i + r.num_chars - 1] == STB_TEXTEDIT_NEWLINE) {
      // at end of line
      row++;
      break;
    } else if (_state.cursor >= i && _state.cursor < i + r.num_chars) {
      // within line
      break;
    }
    i += r.num_chars;
    if (i < len) {
      row++;
    }
  }
  return row + 1;
}

void TextEditInput::updateScroll() {
  int cursor = cursorRow();
  int pageRows = _height / _charHeight;
  if (cursor + 1 < pageRows) {
    _scroll = 0;
  } else if (cursor > _scroll + pageRows || cursor <= _scroll) {
    // cursor outside current view
    _scroll = cursor - (pageRows / 2);
  }
}

