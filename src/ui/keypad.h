// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include "ui/strlib.h"
#include "ui/inputs.h"
#include "lib/maapi.h"

using namespace strlib;

struct KeypadTheme {
  int _bg;
  int _key;
  int _keyHighlight;
  int _text;
  int _outline;
  int _funcKeyBg;
  int _funcKeyHighlight;
  int _funcText;
};

enum KeypadLayout {
  LayoutLetters = 0, LayoutNumbers = 1, LayoutSymbols = 2
};

struct RawKey {
  const char *_normal;
  const char *_shifted;
};

struct Key {
  explicit Key(const RawKey &k);

  int color(const KeypadTheme *theme, bool shiftActive) const;
  void drawButton(const KeypadTheme *theme) const;
  bool inside(int x, int y) const;
  void onClick(bool useShift);

  String _label;
  String _altLabel;
  int _x{};
  int _y{};
  int _w{};
  int _h{};
  int _labelLength;
  bool _pressed;
  bool _number;
  bool _special;
};

struct Keypad {
  Keypad(int charWidth, int charHeight);
  ~Keypad() = default;

  static int outerHeight(int ch) ;
  void clicked(int x, int y, bool pressed);
  void draw() const;
  void layout(int x, int y, int w, int h);

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
  KeypadTheme *_theme;
  KeypadLayout _currentLayout;

  void generateKeys();
  void toggleShift();
};

struct KeypadInput : public FormInput {
  KeypadInput(bool floatTop, bool toolbar, int charWidth, int charHeight);
  ~KeypadInput() override;

  void clicked(int x, int y, bool pressed) override;
  void draw(int x, int y, int w, int h, int chw) override;
  bool floatTop() override { return _floatTop; }
  bool floatBottom() override { return !_floatTop; }
  void layout(int x, int y, int w, int h) override;

private:  
  bool _floatTop;
  bool _toolbar;
  Keypad *_keypad;
};
