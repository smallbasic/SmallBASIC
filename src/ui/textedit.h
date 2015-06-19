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

struct EditBuffer {
  char *_buffer;
  int _len;

  EditBuffer(const char *text);
  virtual ~EditBuffer();
  
  void layout(StbTexteditRow *row, int start_i);
  int deleteChars(int pos, int num);
  int insertChars(int pos, char *newtext, int num);
};

struct TextEditInput : public FormInput {
  TextEditInput(const char *text, int x, int y, int w, int h);
  virtual ~TextEditInput() {}

  void close();
  void draw(int x, int y, int w, int h, int chw);
  bool edit(int key, int screenWidth, int charWidth);
  const char *getText() const { return _str._buffer; }
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
  
private:
  EditBuffer _str;
  STB_TexteditState _state;
  int _scroll;
  int _mark;
  int _point;
  bool _grow;
  bool _controlMode;
};

#define STB_TEXTEDIT_STRING      EditBuffer
#define KEYDOWN_BIT              0x80000000
#define STB_TEXTEDIT_K_LEFT      (KEYDOWN_BIT | 1) // actually use VK_LEFT, SDLK_LEFT, etc
#define STB_TEXTEDIT_K_RIGHT     (KEYDOWN_BIT | 2) // VK_RIGHT
#define STB_TEXTEDIT_K_UP        (KEYDOWN_BIT | 3) // VK_UP
#define STB_TEXTEDIT_K_DOWN      (KEYDOWN_BIT | 4) // VK_DOWN
#define STB_TEXTEDIT_K_LINESTART (KEYDOWN_BIT | 5) // VK_HOME
#define STB_TEXTEDIT_K_LINEEND   (KEYDOWN_BIT | 6) // VK_END
#define STB_TEXTEDIT_K_TEXTSTART (STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_TEXTEND   (STB_TEXTEDIT_K_LINEEND   | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_DELETE    (KEYDOWN_BIT | 7) // VK_DELETE
#define STB_TEXTEDIT_K_BACKSPACE (KEYDOWN_BIT | 8) // VK_BACKSPACE
#define STB_TEXTEDIT_K_UNDO      (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | 'z')
#define STB_TEXTEDIT_K_REDO      (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | 'y')
#define STB_TEXTEDIT_K_INSERT    (KEYDOWN_BIT | 9) // VK_INSERT
#define STB_TEXTEDIT_K_WORDLEFT  (STB_TEXTEDIT_K_LEFT  | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_WORDRIGHT (STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_PGUP      (KEYDOWN_BIT | 10) // VK_PGUP -- not implemented
#define STB_TEXTEDIT_K_PGDOWN    (KEYDOWN_BIT | 11) // VK_PGDOWN -- not implemented
#define STB_TEXTEDIT_K_SHIFT     0x40000000
#define STB_TEXTEDIT_K_CONTROL   0x20000000
#define STB_TEXTEDIT_NEWLINE     '\n'
#define STB_TEXTEDIT_STRINGLEN(tc)    ((tc)->_len)
#define STB_TEXTEDIT_GETWIDTH(tc,n,i) (1) // quick hack for monospaced
#define STB_TEXTEDIT_KEYTOTEXT(key)   (((key) & KEYDOWN_BIT) ? 0 : (key))
#define STB_TEXTEDIT_GETCHAR(tc,i)    ((tc)->_buffer[i])
#define STB_TEXTEDIT_LAYOUTROW(r,obj,n)     obj->layout(r,n)
#define STB_TEXTEDIT_DELETECHARS(obj,i,n)   obj->deleteChars(i, n)
#define STB_TEXTEDIT_INSERTCHARS(obj,i,c,n) obj->insertChars(i, c, n)

#endif

