// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "common/sbapp.h"
#include "common/keymap.h"
#include "lib/maapi.h"
#include "ui/image_codec.h"
#include "ui/keypad_icons.h"
#include "keypad.h"

constexpr int maxRows = 5;
constexpr int maxCols = 10;
constexpr int defaultPadding = 16;
constexpr double PI = 3.14159;

KeypadTheme modernDarkTheme = {
  ._bg = 0x101114,           // Cool dark slate (true dark with a bluish tint)
  ._key = 0x1c1e22,          // Cool gray keys
  ._keyHighlight = 0x2b2e34, // Slightly brighter for key press
  ._text = 0xe4e6ea,         // Icy white-gray text (bluish white)
  ._outline = 0x2f3137,      // Subtle cool gray outlines
  ._funcKeyBg = 0x8e88aa,    // Muted lavender-gray (soft cool purple)
  ._funcKeyHighlight = 0xc6c2dc, // Light lavender highlight
  ._funcText = 0xffffff,     // Crisp white function key text
};

constexpr RawKey letters[][maxCols] = {
  {{K_CUT, K_CUT}, {K_COPY, K_COPY}, {K_PASTE, K_PASTE}, {K_SAVE, K_SAVE}, {K_RUN, K_RUN}, {K_HELP, K_HELP}},
  {{K_q, K_Q}, {K_w, K_W}, {K_e, K_E}, {K_r, K_R}, {K_t, K_T}, {K_y, K_Y}, {K_u, K_U}, {K_i, K_I}, {K_o, K_O}, {K_p, K_P}},
  {{K_a, K_A}, {K_s, K_S}, {K_d, K_D}, {K_f, K_F}, {K_g, K_G}, {K_h, K_H}, {K_j, K_J}, {K_k, K_K}, {K_l, K_L}},
  {{K_SHIFT, K_SHIFT}, {K_z, K_Z}, {K_x, K_X}, {K_c, K_C}, {K_v, K_V}, {K_b, K_B}, {K_n, K_N}, {K_m, K_M}, {K_BACKSPACE, K_BACKSPACE}},
  {{K_TOGGLE, K_TOGGLE}, {K_SPACE, K_SPACE}, {K_ENTER, K_ENTER}}
};

constexpr RawKey symbols[][maxCols] = {
  {{K_CUT, K_CUT}, {K_COPY, K_COPY}, {K_PASTE, K_PASTE}, {K_SAVE, K_SAVE}, {K_RUN, K_RUN}, {K_HELP, K_HELP}},
  {{K_1, K_EXCLAIM}, {K_2, K_AT}, {K_3, K_HASH}, {K_4, K_DOLLAR}, {K_5, K_PERCENT}, {K_6, K_CARET}, {K_7, K_AMPERSAND}, {K_8, K_ASTERISK}, {K_9, K_LPAREN}, {K_0, K_RPAREN}},
  {{K_BACKTICK, K_TILDE}, {K_MINUS, K_UNDERSCORE}, {K_EQUALS, K_PLUS}, {K_LBRACKET, K_LBRACE}, {K_RBRACKET, K_RBRACE}, {K_BACKSLASH, K_PIPE}, {K_SEMICOLON, K_COLON}, {K_LPAREN, K_QUOTE}, {K_RPAREN, K_COMMA}},
  {{K_SHIFT, K_SHIFT}, {K_LESS, K_COMMA}, {K_GREATER, K_PERIOD}, {K_QUESTION, K_SLASH}, {K_SEMICOLON, K_COLON}, {K_APOSTROPHE, K_QUOTE}, {K_LBRACKET, K_LBRACE}, {K_RBRACKET, K_RBRACE}, {K_BACKSPACE, K_BACKSPACE}},
  {{K_TOGGLE, K_TOGGLE}, {K_SPACE, K_SPACE}, {K_ENTER, K_ENTER}}
};

constexpr int rowLengths[][5] = {
  {6, 10, 9, 9, 3}, // letters
  {6, 10, 9, 9, 3}, // symbols
};

constexpr int rowCharLengths[][5] = {
  {12, 10, 9, 12, 14},// letters
  {12, 10, 9, 12, 14},// letters
};

//
// KeypadImage
//
KeypadImage::KeypadImage() : ImageCodec() {
}

void KeypadImage::draw(int x, int y, int w, int h) const {
  MAPoint2d dstPoint;
  MARect srcRect;
  dstPoint.x = x + (w - _width) / 2;
  dstPoint.y = y + (h - _height) / 2;
  srcRect.left = 0;
  srcRect.top = 0;
  srcRect.width = _width;
  srcRect.height = _height;
  maDrawRGB(&dstPoint, _pixels, &srcRect, 0, _width);
}

//
// KeypadDrawContext
//
KeypadDrawContext::KeypadDrawContext(int charWidth, int charHeight) :
  _charWidth(charWidth),
  _charHeight(charHeight),
  _shiftActive(false),
  _capsLockActive(false),
  _cutImage(),
  _copyImage(),
  _pasteImage(),
  _saveImage(),
  _runImage(),
  _helpImage(),
  _backImage(),
  _enterImage(),
  _searchImage(),
  _shiftImage() {

  if (!_cutImage.decode(img_cut, img_cut_len) ||
      !_copyImage.decode(img_copy, img_copy_len) ||
      !_pasteImage.decode(img_clipboard_paste, img_clipboard_paste_len) ||
      !_saveImage.decode(img_save, img_save_len) ||
      !_runImage.decode(img_bug_play, img_bug_play_len) ||
      !_helpImage.decode(img_book_info_2, img_book_info_2_len) ||
      !_backImage.decode(img_backspace, img_backspace_len) ||
      !_enterImage.decode(img_arrow_enter, img_arrow_enter_len) ||
      !_searchImage.decode(img_search, img_search_len) ||
      !_shiftImage.decode(img_keyboard_shift, img_keyboard_shift_len) ||
      !_toggleImage.decode(img_keyboard, img_keyboard_len)) {
    deviceLog(_cutImage.getLastError());
  }
}

void KeypadDrawContext::toggleShift() {
  _shiftActive = !_shiftActive;
}

bool KeypadDrawContext::useShift(bool specialKey) {
  bool useShift = _shiftActive ^ _capsLockActive;
  if (_shiftActive && !specialKey) {
    _shiftActive = false;
  }
  return useShift;
}

const KeypadImage *KeypadDrawContext::getImage(const KeyCode keycode) const {
  const KeypadImage *result;
  switch (keycode) {
  case K_CUT: result = &_cutImage; break;
  case K_COPY: result = &_copyImage; break;
  case K_PASTE: result = &_pasteImage; break;
  case K_SAVE: result = &_saveImage; break;
  case K_RUN: result = &_runImage; break;
  case K_HELP: result = &_helpImage; break;
  case K_BACKSPACE: result = &_backImage;break;
  case K_ENTER: result = &_enterImage; break;
  case K_SEARCH: result = &_searchImage; break;
  case K_SHIFT: result = &_shiftImage; break;
  case K_TOGGLE: result = &_toggleImage; break;
  default: result = nullptr; break;
  }
  return result;
}

//
// Key
//
Key::Key(const RawKey &k) :
  _key(k._normal),
  _alt(k._shifted) {
  _pressed = false;
  _number = isNumber(k._normal);
  _printable = isPrintable(k._normal);
}

int Key::color(const KeypadTheme *theme, bool shiftActive) const {
  int result;
  if (_pressed || ((_key == K_SHIFT) && shiftActive)) {
    result = !_printable ? theme->_funcKeyHighlight : theme->_keyHighlight;
  } else if (_number) {
    result = theme->_funcKeyHighlight;
  } else {
    result = theme->_text;
  }
  return result;
}

void Key::draw(const KeypadTheme *theme, const KeypadDrawContext *context) const {
  int rc = 5;
  int pad = 2;
  int rx = _x + _w - pad; // right x
  int by = _y + _h - pad; // bottom y
  int lt = _x + rc + pad; // left x (after corner)
  int vt = _y + rc + pad; // top y (after corner)
  int rt = rx - rc;       // right x (before corner)
  int bt = by - rc;       // bottom y (before corner)
  int xcL = _x + rc + pad; // x center for left arcs
  int xcR = rx - rc;       // x center for right arcs
  int ycT = _y + rc + pad; // y center for top arcs
  int ycB = by - rc;       // y center for bottom arcs

  // Set background color
  maSetColor(_printable ? theme->_key : theme->_funcKeyBg);
  maFillRect(_x, _y, _w, _h);

  if (_printable || _pressed) {
    maSetColor(_printable ? theme->_keyHighlight : theme->_funcKeyHighlight);

    // Draw edges (excluding the rounded corners)
    maLine(lt, _y + pad, rt, _y + pad); // top edge
    maLine(_x + pad, vt, _x + pad, bt); // left edge
    maLine(lt, by, rt, by);             // bottom edge
    maLine(rx, vt, rx, bt);            // right edge

    // Draw rounded corners using arcs (quarter circles)
    // Arcs: maArc(xc, yc, r, startAngle, endAngle, aspect)
    double aspect = 1.0; // Circle
    maArc(xcL, ycT, rc, PI, PI * 3 / 2, aspect); // Top-left corner
    maArc(xcR, ycT, rc, PI * 3 / 2, 0, aspect);  // Top-right corner
    maArc(xcR, ycB, rc, 0, PI / 2, aspect);      // Bottom-right corner
    maArc(xcL, ycB, rc, PI / 2, PI, aspect);     // Bottom-left corner
  }

  if (_printable) {
    bool useShift = context->_shiftActive ^ context->_capsLockActive;
    char key[] = {useShift ? _alt : _key, '\0'};
    int xOffset = (_w - context->_charWidth) / 2;
    int yOffset = (_h - context->_charHeight) / 2;
    int textX = _x + xOffset;
    int textY = _y + yOffset;
    maSetColor(color(theme, context->_shiftActive));
    maDrawText(textX, textY, key, 1);
  } else {
    auto *image = context->getImage(_key);
    if (image) {
      image->draw(_x, _y, _w, _h);
    }
  }
}

bool Key::inside(int x, int y) const {
  return (x >= _x &&
          x <= _x + _w &&
          y >= _y &&
          y <= _y + _h);
}

void Key::onClick(bool useShift) {
  auto *event = new MAEvent();
  event->type = EVENT_TYPE_KEY_PRESSED;
  event->nativeKey = 0;
  switch (_key) {
  case K_BACKSPACE:
    event->key = SB_KEY_BACKSPACE;
    break;
  case K_ENTER:
    event->key = SB_KEY_ENTER;
    break;
  case K_SPACE:
    event->key = SB_KEY_SPACE;
    break;
  case K_FIND:
    event->key = SB_KEY_CTRL('f');
    break;
  case K_CUT:
    event->key = SB_KEY_CTRL('x');
    break;
  case K_COPY:
    event->key = SB_KEY_CTRL('c');
    break;
  case K_PASTE:
    event->key = SB_KEY_CTRL('v');
    break;
  case K_SAVE:
    event->key = SB_KEY_CTRL('s');
    break;
  case K_RUN:
    event->key = SB_KEY_CTRL('r');
    break;
  case K_HELP:
    event->key = SB_KEY_F(1);
    break;
  default:
    event->key = _key;
    break;
  }
  maPushEvent(event);
}

//
// Keypad
//
Keypad::Keypad(int charWidth, int charHeight)
  : _posX(0),
    _posY(0),
    _width(0),
    _height(0),
    _context(charWidth, charHeight),
    _theme(&modernDarkTheme),
    _currentLayout(LayoutLetters) {
  generateKeys();
}

int Keypad::outerHeight(int charHeight) {
  return maxRows * ((defaultPadding * 2) + charHeight);
}

void Keypad::generateKeys() {
  _keys.clear();

  const RawKey (*activeLayout)[maxCols];
  switch (_currentLayout) {
  case LayoutLetters:
    activeLayout = letters;
    break;
  default:
    activeLayout = symbols;
    break;
  }

  for (int row = 0; row < maxRows; ++row) {
    int cols = rowLengths[_currentLayout][row];
    for (int col = 0; col < cols; col++) {
      const RawKey &k = activeLayout[row][col];
      if (k._normal != K_NULL) {
        _keys.add(new Key(k));
      }
    }
  }
}

void Keypad::layout(int x, int y, int w, int h) {
  _posX = x;
  _posY = y;
  _width = w;
  _height = h;

  const int charWidth = _context._charWidth;
  const int charHeight = _context._charHeight;

  // start with optimum padding, then reduce to fit width
  int padding = defaultPadding;
  int width = maxCols * (charWidth + (padding * 2));

  while (width > w && padding > 0) {
    padding--;
    width = maxCols * (charWidth + (padding * 2));
  }

  int keyW = charWidth + (padding * 2);
  int keyH = charHeight + (padding * 2);
  int xStart = _posX + ((w - width) / 2);
  int yPos = _posY;
  int index = 0;

  for (int row = 0; row < maxRows; ++row) {
    int cols = rowLengths[_currentLayout][row];
    int chars = rowCharLengths[_currentLayout][row];
    int xPos = xStart;
    if (cols < maxCols) {
      // center narrow row
      int rowWidth = (chars * charWidth) + (cols * padding * 2);
      if (rowWidth > width) {
        xPos -= (rowWidth - width) / 2;
      } else {
        xPos += (width - rowWidth) / 2;
      }
    }
    for (int col = 0; col < cols; col++) {
      if (index >= (int)_keys.size()) {
        break;
      }
      Key *key = _keys[index++];
      int length = key->_printable ? 1 : 2;
      int keyWidth = keyW;
      if (key->_key == K_SPACE) {
        length = 12;
      }
      if (length > 1) {
        keyWidth = (length * charWidth) + (padding * 2);
      }
      key->_x = xPos;
      key->_y = yPos;
      key->_w = keyWidth;
      key->_h = keyH;
      xPos += keyWidth;
    }
    yPos += keyH;
  }
}

void Keypad::clicked(int x, int y, bool pressed) {
  for (auto key : _keys) {
    bool inside = key->inside(x, y);
    key->_pressed = pressed && inside;

    if (pressed && inside) {
      if (key->_key == K_SHIFT) {
        _context.toggleShift();
      } else if (key->_key == K_TOGGLE) {
        _currentLayout = static_cast<KeypadLayout>((_currentLayout + 1) % 2);
        generateKeys();
        layout(_posX, _posY, _width, _height);
        break;
      } else {
        key->onClick(_context.useShift(!key->_printable));
      }
      break;
    }
  }
}

void Keypad::draw() const {
  maSetColor(_theme->_bg);
  maFillRect(_posX, _posY, _width, _height);
  for (const auto &key : _keys) {
    key->draw(_theme, &_context);
  }
}

//
// KeypadInput
//
KeypadInput::KeypadInput(bool floatTop, bool toolbar, int charWidth, int charHeight) :
  FormInput(0, 0, 0, charHeight * 2),
  _floatTop(floatTop),
  _toolbar(toolbar),
  _keypad(nullptr) {
  if (!toolbar) {
    _keypad = new Keypad(charWidth, charHeight);
    _height = Keypad::outerHeight(charHeight);
  }
}

KeypadInput::~KeypadInput() {
  delete _keypad;
}

void KeypadInput::clicked(int x, int y, bool pressed) {
  if (_keypad) {
    _keypad->clicked(x, y, pressed);
  }
}

void KeypadInput::draw(int x, int y, int w, int h, int chw) {
  if (_keypad) {
    _keypad->draw();
  } else {
    maSetColor(modernDarkTheme._bg);
    maFillRect(x, y, _width, _height);
  }
}

void KeypadInput::layout(int x, int y, int w, int h) {
  _x = x;
  _y = _floatTop ? 0 : h;
  _width = w;
  if (_keypad) {
    _keypad->layout(_x, _y, _width, _height);
  }
}
