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

//
// EditBuffer
//
EditBuffer::EditBuffer(TextEditInput *in, const char *text) : _in(in) {
  if (text != NULL && text[0]) {
    _len = strlen(text);
    _buffer = new char[_len + 1];
    memcpy(_buffer, text, _len);
    _buffer[_len] = '\0';
  } else {
    _len = 0;
    _buffer = NULL;
  }
}

EditBuffer::~EditBuffer() {
  delete _buffer;
  _buffer = NULL;
}

int EditBuffer::deleteChars(int pos, int num) {
  memmove(&_buffer[pos], &_buffer[pos+num], _len - (pos + num));
  _len -= num;
  return 1;
}

int EditBuffer::insertChars(int pos, char *newtext, int num) {
  _buffer = (char *)realloc(_buffer, _len + num);
  memmove(&_buffer[pos+num], &_buffer[pos], _len - pos);
  memcpy(&_buffer[pos], newtext, num);
  _len += num;
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
  _controlMode(false) {
  stb_textedit_initialize_state(&_state, false);
}

void TextEditInput::close() {
}

void TextEditInput::draw(int x, int y, int w, int h, int chw) {
  maSetColor(getBackground(GRAY_BG_COL));
  maFillRect(x, y, _width, _height);
  maSetColor(_fg);

  StbTexteditRow r;
  int len = _buf._len;
  int i = 0;
  int baseY = 0;
  while (i < len) {
    layout(&r, i);

    if (baseY + r.ymax > h) {
      break;
    }

    if (r.num_chars) {
      maDrawText(x, y + baseY, _buf._buffer + i, r.num_chars);
    }

    if (_state.select_start != _state.select_end) {
      // draw selection
    } else if ((_state.cursor >= i && _state.cursor < i + r.num_chars) ||
               (i + r.num_chars == _buf._len && _state.cursor == _buf._len)) {
      // draw cursor
      int px = x + ((_state.cursor - i) * chw);
      maFillRect(px, y + baseY, chw, _charHeight);
      if (_state.cursor < _buf._len) {
        maSetColor(getBackground(GRAY_BG_COL));
        maDrawText(px, y + baseY, _buf._buffer + _state.cursor, 1);
        maSetColor(_fg);
      }
    }

    i += r.num_chars;
    baseY += _charHeight;
  }
}

bool TextEditInput::edit(int key, int screenWidth, int charWidth) {
  stb_textedit_key(&_buf, &_state, key);
  return true;
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

void TextEditInput::layout(StbTexteditRow *row, int start) {
  int i = start;
  int x2 = _width - _charWidth;
  row->x1 = 0;
  row->num_chars = 0;

  // advance over newlines
  while (i < _buf._len
         && _buf._buffer[i] == STB_TEXTEDIT_NEWLINE) {
    i++;
    row->num_chars++;
  }
  while (i < _buf._len
         && (int)row->x1 < x2
         && _buf._buffer[i] != STB_TEXTEDIT_NEWLINE) {
    row->x1 += _charWidth;
    row->num_chars++;
    i++;
  }
  row->x0 = 0.0f;
  row->ymin = 0.0f;
  row->ymax = row->baseline_y_delta = _charHeight;
}
