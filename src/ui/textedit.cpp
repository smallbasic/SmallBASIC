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
// EditString
//
EditString::~EditString() {
  delete _buffer;
  _buffer = NULL;
}

void EditString::layout(StbTexteditRow *row, int start_i) {
  int remaining_chars = _len - start_i;
  row->num_chars = remaining_chars > 20 ? 20 : remaining_chars; // should do real word wrap here
  row->x0 = 0;
  row->x1 = 20; // need to account for actual size of characters
  row->baseline_y_delta = 1.25;
  row->ymin = -1;
  row->ymax =  0;
}

int EditString::deleteChars(int pos, int num) {
  memmove(&_buffer[pos], &_buffer[pos+num], _len - (pos + num));
  _len -= num;
  return 1;
}

int EditString::insertChars(int pos, char *newtext, int num) {
  _buffer = (char *)realloc(_buffer, _len + num);
  memmove(&_buffer[pos+num], &_buffer[pos], _len - pos);
  memcpy(&_buffer[pos], newtext, num);
  _len += num;
  return 1;
}

//
// TextEditInput
//
TextEditInput::TextEditInput(const char *text, int x, int y, int w, int h) :
  FormInput(x, y, w, h),
  _str(text),
  _scroll(0),
  _mark(-1),
  _point(0),
  _controlMode(false) {
  stb_textedit_initialize_state(&_state, false);
}

void TextEditInput::close() {
}

void TextEditInput::draw(int x, int y, int w, int h, int chw) {
}

bool TextEditInput::edit(int key, int screenWidth, int charWidth) {
  stb_textedit_key(&_str, &_state, key);
  return 0;
}

int TextEditInput::getControlKey(int key) {
  return 0;
}

void TextEditInput::setText(const char *text) {
}

void TextEditInput::setFocus() {
}

void TextEditInput::clicked(int x, int y, bool pressed) {
// static void stb_textedit_click(STB_TEXTEDIT_STRING *str, STB_TexteditState *state, float x, float y)
}

void TextEditInput::updateField(var_p_t form) {
}

bool TextEditInput::selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) {
// static void stb_textedit_drag(STB_TEXTEDIT_STRING *str, STB_TexteditState *state, float x, float y)
  return 0;
}

char *TextEditInput::copy(bool cut) {
// static int stb_textedit_cut(STB_TEXTEDIT_STRING *str, STB_TexteditState *state)
  return 0;
}

void TextEditInput::cut() {
// static int stb_textedit_cut(STB_TEXTEDIT_STRING *str, STB_TexteditState *state)
}

void TextEditInput::paste(char *text) {
// static int stb_textedit_paste(STB_TEXTEDIT_STRING *str, STB_TexteditState *state, STB_TEXTEDIT_CHARTYPE const *ctext, int len)
}

EditString::EditString(const char *text) {
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

