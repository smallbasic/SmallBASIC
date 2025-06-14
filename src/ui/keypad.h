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
#include "ui/image_codec.h"
#include "ui/keycode.h"

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

struct KeypadImage : ImageCodec {
  KeypadImage();
  ~KeypadImage() = default;
  void draw(int x, int y, int w, int h) const;
};

struct KeypadDrawContext {
  explicit KeypadDrawContext(int charWidth, int charHeight);
  void toggleShift();
  bool useShift(bool specialKey);
  const KeypadImage *getImage(const KeyCode keycode) const;

  int _charWidth;
  int _charHeight;
  bool _shiftActive;
  bool _capsLockActive;

  KeypadImage _cutImage;
  KeypadImage _copyImage;
  KeypadImage _pasteImage;
  KeypadImage _saveImage;
  KeypadImage _runImage;
  KeypadImage _helpImage;
  KeypadImage _backImage;
  KeypadImage _enterImage;
  KeypadImage _searchImage;
  KeypadImage _shiftImage;
  KeypadImage _toggleImage;
};

enum KeypadLayout {
  LayoutLetters = 0, LayoutNumbers = 1, LayoutSymbols = 2
};

struct RawKey {
  const KeyCode _normal;
  const KeyCode _shifted;
};

struct Key {
  explicit Key(const RawKey &k);

  int color(const KeypadTheme *theme, bool shiftActive) const;
  void draw(const KeypadTheme *theme, const KeypadDrawContext *context) const;
  bool inside(int x, int y) const;
  void onClick(bool useShift);

  int _x{};
  int _y{};
  int _w{};
  int _h{};
  KeyCode _key;
  KeyCode _alt;
  bool _pressed;
  bool _number;
  bool _printable;
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
  strlib::List<Key *> _keys;
  KeypadTheme *_theme;
  KeypadDrawContext _context;
  KeypadLayout _currentLayout;

  void generateKeys();
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
