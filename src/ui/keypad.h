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

#if defined(_SDL)
// for cursor display
#define HAS_HOVER true
#define PADDING 6
#else
#define HAS_HOVER false
#define PADDING 16
#endif

struct KeypadTheme {
  int _bg;
  int _key;
  int _keyHighlight;
  int _text;
  int _outline;
  int _funcKeyBg;
  int _funcKeyHighlight;
};

struct KeypadImage : ImageCodec {
  KeypadImage();
  ~KeypadImage() override = default;
  void draw(int x, int y, int w, int h) const;
};

enum Keyset {
  kLower, kUpper, kNumber, kSymbol, kSize
};

struct RawKey {
  const KeyCode _lower;
  const KeyCode _upper;
  const KeyCode _number;
  const KeyCode _symbol;
};

struct KeypadDrawContext {
  explicit KeypadDrawContext(int charWidth, int charHeight);
  const KeypadImage *getImage(const RawKey &keycode) const;
  void toggle();

  int _charWidth;
  int _charHeight;
  Keyset _keySet;

  KeypadImage _cutImage;
  KeypadImage _copyImage;
  KeypadImage _pasteImage;
  KeypadImage _saveImage;
  KeypadImage _runImage;
  KeypadImage _helpImage;
  KeypadImage _backImage;
  KeypadImage _enterImage;
  KeypadImage _searchImage;
  KeypadImage _toggleImage;
  KeypadImage _lineUpImage;
  KeypadImage _pageUpImage;
  KeypadImage _lineDownImage;  
  KeypadImage _pageDownImage;
};

struct Key {
  explicit Key(const RawKey &k);

  int color(const KeypadTheme *theme) const;
  void draw(const KeypadTheme *theme, const KeypadDrawContext *context) const;
  char getKey(const KeypadDrawContext *context) const;
  bool inside(int x, int y) const;
  void onClick(const KeypadDrawContext *context) const;

  int _x{};
  int _y{};
  int _w{};
  int _h{};
  RawKey _key;
  bool _pressed;
  bool _number;
  bool _printable;
};

struct Keypad {
  Keypad(int charWidth, int charHeight, bool toolbar);
  ~Keypad() = default;

  int outerHeight(int charHeight) const;
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
  bool _toolbar;

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
  void drawHover(int dx, int dy, bool selected) override {};
  bool hasHover() override { return HAS_HOVER; }

private:
  bool _floatTop;
  Keypad *_keypad;
};
