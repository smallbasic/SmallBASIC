// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#define STB_TEXTEDIT_CHARTYPE char

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lib/stb_textedit.h"
#include "ui/inputs.h"
#include "common/smbas.h"
#include "common/keymap.h"

struct TextEditInput;

struct EditBuffer {
  char *_buffer;
  int _len;
  TextEditInput *_in;

  EditBuffer(TextEditInput *in, const char *text);
  virtual ~EditBuffer();

  int deleteChars(int pos, int num);
  int insertChars(int pos, char *newtext, int num);
};

struct TextEditInput : public FormInput {
  TextEditInput(const char *text, int chW, int chH, int x, int y, int w, int h);
  virtual ~TextEditInput() {}

  void close();
  void draw(int x, int y, int w, int h, int chw);
  bool edit(int key, int screenWidth, int charWidth);
  const char *getText() const { return _buf._buffer; }
  int  getControlKey(int key);
  bool getControlMode() const { return _controlMode; }
  void setControlMode(bool cursorMode) { _controlMode = cursorMode; }
  void setText(const char *text);
  void setFocus();
  void clicked(int x, int y, bool pressed);
  void updateField(var_p_t form);
  bool selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw);
  int padding(bool) const { return 0; }
  char *copy(bool cut);
  void cut();
  void paste(char *text);
  void layout(StbTexteditRow *row, int start_i);
  int charWidth(int k, int i) { return _charWidth; }

private:
  EditBuffer _buf;
  STB_TexteditState _state;
  int _charWidth;
  int _charHeight;
  bool _controlMode;
};

#define STB_TEXTEDIT_STRING       EditBuffer
#define STB_TEXTEDIT_K_LEFT       SB_KEY_LEFT
#define STB_TEXTEDIT_K_RIGHT      SB_KEY_RIGHT
#define STB_TEXTEDIT_K_UP         SB_KEY_UP
#define STB_TEXTEDIT_K_DOWN       SB_KEY_DOWN
#define STB_TEXTEDIT_K_LINESTART  SB_KEY_HOME
#define STB_TEXTEDIT_K_LINEEND    SB_KEY_END
#define STB_TEXTEDIT_K_TEXTSTART  SB_KEY_CTRL('a')
#define STB_TEXTEDIT_K_TEXTEND    SB_KEY_CTRL('e')
#define STB_TEXTEDIT_K_DELETE     SB_KEY_DELETE
#define STB_TEXTEDIT_K_BACKSPACE  SB_KEY_BACKSPACE
#define STB_TEXTEDIT_K_UNDO       SB_KEY_CTRL('z')
#define STB_TEXTEDIT_K_REDO       SB_KEY_CTRL('y')
#define STB_TEXTEDIT_K_INSERT     SB_KEY_INSERT
#define STB_TEXTEDIT_K_WORDLEFT   SB_KEY_CTRL(SB_KEY_LEFT)
#define STB_TEXTEDIT_K_WORDRIGHT  SB_KEY_CTRL(SB_KEY_RIGHT)
#define STB_TEXTEDIT_K_PGUP       SB_KEY_PGUP
#define STB_TEXTEDIT_K_PGDOWN     SB_KEY_PGDN
#define STB_TEXTEDIT_K_SHIFT      0x40000000
#define STB_TEXTEDIT_K_CONTROL    0xF100
#define STB_TEXTEDIT_KEYTOTEXT(k) k
#define STB_TEXTEDIT_NEWLINE      '\n'
#define STB_TEXTEDIT_STRINGLEN(o) o->_len
#define STB_TEXTEDIT_GETCHAR(o,i) o->_buffer[i]
#define STB_TEXTEDIT_GETWIDTH(o,n,i)      o->_in->charWidth(n, i)
#define STB_TEXTEDIT_LAYOUTROW(r,o,n)     o->_in->layout(r, n)
#define STB_TEXTEDIT_DELETECHARS(o,i,n)   o->deleteChars(i, n)
#define STB_TEXTEDIT_INSERTCHARS(o,i,c,n) o->insertChars(i, c, n)

#endif

