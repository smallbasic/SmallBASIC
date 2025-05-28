// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include "ui/strlib.h"
#include "ui/theme.h"

using namespace strlib;

enum KeypadLayout {
  LayoutLetters = 0, LayoutNumbers = 1, LayoutSymbols = 2
};

struct RawKey {
  const char *_normal;
  const char *_shifted;
};

struct Key {
  Key(const RawKey &k);

  int color(EditTheme *theme, bool shiftActive);
  void drawButton(EditTheme *theme);
  bool inside(int x, int y) const;

  String _label;
  String _altLabel;
  int _x;
  int _y;
  int _w;
  int _h;
  int _labelLength;
  bool _pressed;
  bool _number;
  bool _special;
};

struct Keypad {
  Keypad(int charWidth, int charHeight);
  ~Keypad() = default;

  bool visible() const { return _visible; }
  int  outerHeight(int ch) const;
  void clicked(int x, int y, bool pressed);
  void draw();
  void hide() { _visible = false; }
  void layout(int x, int y, int w, int h);
  void show() { _visible = true; }
  void setTheme(EditTheme *theme) { _theme = theme; }

  private:
  int _posX;
  int _posY;
  int _width;
  int _height;
  int _charWidth;
  int _charHeight;
  strlib::List<Key *> _keys;
  bool _shiftActive;
  bool _capsLockActive;
  bool _visible;
  EditTheme *_theme;
  KeypadLayout _currentLayout;

  void generateKeys();
  void toggleShift();
};
