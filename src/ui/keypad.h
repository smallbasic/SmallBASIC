// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include <memory>
#include "ui/strlib.h"
#include "ui/inputs.h"
#include "ui/image_codec.h"
#include "ui/keycode.h"

using namespace strlib;

#if defined(_SDL)
// for cursor display
#define HAS_HOVER true
#define PLATFORM_PADDING 0.5
#else
#define HAS_HOVER false
#define PLATFORM_PADDING 1.1
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
  kLower, kNumber, kSymbol, kUpper, kSize
};

struct RawKey {
  const KeyCode _lower;
  const KeyCode _number;
  const KeyCode _symbol;
  const KeyCode _upper;
};

struct KeypadDrawContext {
  explicit KeypadDrawContext(int charWidth, int charHeight);
  const KeypadImage *getImage(const RawKey &keycode) const;
  KeyCode getKey(RawKey rawKey) const;
  void layoutHeight(int padding);
  void onClick(RawKey key);
  void toggle();
  void upper();

  int _charWidth;
  int _charHeight;
  int _imageSize;
  bool _punctuation;
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
  KeypadImage _tagImage;
  KeypadImage _leftImage;
  KeypadImage _rightImage;
};

struct Key {
  explicit Key(const RawKey &k);

  int color(const KeypadTheme *theme) const;
  void draw(const KeypadTheme *theme, const KeypadDrawContext *context, bool pressed) const;
  bool inside(int x, int y) const;
  void onClick(KeypadDrawContext *context) const;

  int _x{};
  int _y{};
  int _w{};
  int _h{};
  int _xEnd{};
  int _yEnd{};
  RawKey _key;
  bool _printable;
};

enum KeypadLayoutStyle {
  kNarrow, kWide
};

struct KeypadLayout {
  virtual ~KeypadLayout();
  virtual RawKey getRawKey(int row, int col) const = 0;
  virtual KeypadLayoutStyle getKeypadLayoutStyle() const = 0;
  virtual int getMaxCols() const = 0;
  virtual int getMaxRows() const = 0;
  virtual int getRowLength(int row) const = 0;
  virtual int getSpaceCols() const = 0;
  virtual bool isCentered(int row) const = 0;
};

struct Keypad {
  Keypad(int screenWidth, int charWidth, int charHeight, bool toolbar);
  ~Keypad() = default;

  void clicked(int x, int y, bool pressed);
  void draw() const;
  void layout(int x, int y, int w, int h);
  int layoutHeight(int screenHeight);
  void selectLayout();

  private:
  void generateKeys();

  int _posX;
  int _posY;
  int _width;
  int _height;
  int _padding;
  bool _toolbar;
  Key *_pressed;
  strlib::List<Key *> _keys;
  KeypadTheme *_theme;
  KeypadDrawContext _context;
  std::unique_ptr<KeypadLayout> _layout;
};

struct KeypadInput : public FormInput {
  KeypadInput(int screenWidth, bool floatTop, bool toolbar, int charWidth, int charHeight);
  ~KeypadInput() override;

  void clicked(int x, int y, bool pressed) override;
  void draw(int x, int y, int w, int h, int chw) override;
  bool floatTop() override { return _floatTop; }
  bool floatBottom() override { return !_floatTop; }
  void layout(int x, int y, int w, int h) override;
  int  layoutHeight(int screenHeight) override;
  void drawHover(int dx, int dy, bool selected) override {};
  bool hasHover() override { return HAS_HOVER; }

  private:
  bool _floatTop;
  Keypad *_keypad;
};
