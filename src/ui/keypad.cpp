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

// the size at which the tablet layout is selected
constexpr int WIDE_LAYOUT = 600;

// the size of PNGs as defined in keypad/build.sh
constexpr int IMAGE_SIZE = 30;

// PI
constexpr double PI = 3.14159;

// padding size based on character height
constexpr double PADDING_FACTOR = PLATFORM_PADDING;

// maximum keyboard height as based on screen height
constexpr double MAX_HEIGHT_FACTOR = 0.48;

// image height based on available rectangle
constexpr double IMAGE_SIZE_FACTOR = 0.65;

// IMAGE_SIZE_FACTOR for the tag image
constexpr double TAG_IMAGE_SIZE_FACTOR = 0.42;

// IMAGE_SIZE_FACTOR for the page up/down images
constexpr double PAGE_IMAGE_SIZE_FACTOR = 0.52;

// https://materialui.co/colors
KeypadTheme MODERN_DARK_THEME = {
  ._bg = 0x121212,           // Material UI standard dark background
  ._key = 0x1e1e1e,          // Slightly raised surface color
  ._keyHighlight = 0x2c2c2c, // Key press highlight (low elevation overlay)
  ._text = 0xffffff,         // High contrast white text
  ._outline = 0x2a2a2a,      // Subtle key outlines (very low elevation)
  ._funcKeyBg = 0x263238,    // Blue Grey 800
  ._funcKeyHighlight = 0x90a4ae, // Blue Grey 300 (matching accent highlight)
};

// compact layout for mobile devices in portrait mode
namespace MobileKeypadLayout {
  constexpr int ROW_LENGTHS[] = {7, 10, 9, 9, 9};
  constexpr int MAX_ROWS = 5;
  constexpr int MAX_COLS = 10;
  constexpr int QWERTY_ROW = 1;
  constexpr int ASDF_ROW = 2;
  constexpr int SPACE_COLS = 3;
  constexpr RawKey KEYS[MAX_ROWS][MAX_COLS] = {
    // Toolbar (mobile)
    {
      {K_CUT, K_CUT, K_CUT, K_CUT},
      {K_COPY, K_COPY, K_COPY, K_COPY},
      {K_PASTE, K_PASTE, K_PASTE, K_PASTE},
      {K_SEARCH, K_SEARCH, K_SEARCH, K_SEARCH},
      {K_SAVE, K_SAVE, K_SAVE, K_SAVE},
      {K_RUN, K_RUN, K_RUN, K_RUN},
      {K_HELP, K_HELP, K_HELP, K_HELP},
      {K_NULL},
      {K_NULL},
      {K_NULL}
    },
    // QWERTY (mobile)
    {
      {K_q, K_1, K_1, K_Q},
      {K_w, K_2, K_2, K_W},
      {K_e, K_3, K_3, K_E},
      {K_r, K_4, K_4, K_R},
      {K_t, K_5, K_5, K_T},
      {K_y, K_6, K_6, K_Y},
      {K_u, K_7, K_7, K_U},
      {K_i, K_8, K_8, K_I},
      {K_o, K_9, K_9, K_O},
      {K_p, K_0, K_0, K_P}
    },
    // ASDF (mobile)
    {
      {K_a, K_COMMA, K_HASH, K_A},
      {K_s, K_EQUALS, K_SEMICOLON, K_S},
      {K_d, K_LPAREN, K_QUESTION, K_D},
      {K_f, K_RPAREN, K_AMPERSAND, K_F},
      {K_g, K_QUOTE, K_DOLLAR, K_G},
      {K_h, K_APOSTROPHE, K_EXCLAIM, K_H},
      {K_j, K_PERIOD, K_AT, K_J},
      {K_k, K_MINUS, K_SLASH, K_K},
      {K_l, K_ASTERISK, K_BACKSLASH, K_L},
      {K_NULL}
    },
    // ZXC (mobile)
    {
      {K_TOGGLE, K_TOGGLE, K_TOGGLE, K_TOGGLE},
      {K_z, K_UNDERSCORE, K_CARET, K_Z},
      {K_x, K_PLUS, K_LBRACE, K_X},
      {K_c, K_COLON, K_RBRACE, K_C},
      {K_v, K_LBRACKET, K_PIPE, K_V},
      {K_b, K_RBRACKET, K_PERCENT, K_B},
      {K_n, K_LESS, K_BACKTICK, K_N},
      {K_m, K_GREATER, K_TILDE, K_M},
      {K_BACKSPACE, K_BACKSPACE, K_BACKSPACE, K_BACKSPACE},
      {K_NULL}
    },
    // FUNCs, SPACE (mobile)
    {
      {K_LINE_UP, K_PAGE_UP, K_LINE_UP, K_PAGE_UP},
      {K_LINE_DOWN, K_PAGE_DOWN, K_LINE_DOWN, K_PAGE_DOWN},
      {K_LPAREN, K_SLASH, K_COMMA, K_LBRACKET},
      {K_SPACE, K_SPACE, K_SPACE, K_SPACE},
      {K_RPAREN, K_HASH, K_EQUALS, K_RBRACKET},
      {K_TAG, K_TAG, K_TAG, K_TAG},
      {K_ENTER, K_ENTER, K_ENTER, K_ENTER},
      {K_NULL},
      {K_NULL},
      {K_NULL},
    }
  };

  // mobile
  struct Impl : public KeypadLayout {
    KeypadLayoutStyle getKeypadLayoutStyle() const override {
      return kNarrow;
    }
    
    RawKey getRawKey(int row, int col) const override {
      return MobileKeypadLayout::KEYS[row][col];
    }

    int getMaxCols() const override {
      return MAX_COLS;
    }

    int getMaxRows() const override {
      return MAX_ROWS;
    }

    int getRowLength(int row) const override {
      return ROW_LENGTHS[row];
    };

    int getSpaceCols() const override {
      return SPACE_COLS;
    }

    bool isCentered(int row) const override {
      return row == QWERTY_ROW || row == ASDF_ROW;
    }
  };
};

// layout for tablet devices
namespace TabletKeypadLayout {
  constexpr int ROW_LENGTHS[] = {7, 10, 9, 10, 9};
  constexpr int MAX_ROWS = 5;
  constexpr int MAX_COLS = 10;
  constexpr int QWERTY_ROW = 1;
  constexpr int ASDF_ROW = 2;
  constexpr int SPACE_COLS = 2;
  constexpr RawKey KEYS[MAX_ROWS][MAX_COLS] = {
    // Toolbar (tablet)
    {
      {K_CUT, K_CUT, K_CUT, K_CUT},
      {K_COPY, K_COPY, K_COPY, K_COPY},
      {K_PASTE, K_PASTE, K_PASTE, K_PASTE},
      {K_SEARCH, K_SEARCH, K_SEARCH, K_SEARCH},
      {K_SAVE, K_SAVE, K_SAVE, K_SAVE},
      {K_RUN, K_RUN, K_RUN, K_RUN},
      {K_HELP, K_HELP, K_HELP, K_HELP},
      {K_NULL},
      {K_NULL},
      {K_NULL}
    },
    // QWERTY (tablet)
    {
      {K_q, K_1, K_1, K_Q},
      {K_w, K_2, K_2, K_W},
      {K_e, K_3, K_3, K_E},
      {K_r, K_4, K_4, K_R},
      {K_t, K_5, K_5, K_T},
      {K_y, K_6, K_6, K_Y},
      {K_u, K_7, K_7, K_U},
      {K_i, K_8, K_8, K_I},
      {K_o, K_9, K_9, K_O},
      {K_p, K_0, K_0, K_P}
    },
    // ASDF (tablet)
    {
      {K_a, K_COMMA, K_HASH, K_A},
      {K_s, K_EQUALS, K_SEMICOLON, K_S},
      {K_d, K_LPAREN, K_QUESTION, K_D},
      {K_f, K_RPAREN, K_AMPERSAND, K_F},
      {K_g, K_QUOTE, K_DOLLAR, K_G},
      {K_h, K_APOSTROPHE, K_EXCLAIM, K_H},
      {K_j, K_PERIOD, K_AT, K_J},
      {K_k, K_MINUS, K_SLASH, K_K},
      {K_l, K_ASTERISK, K_BACKSLASH, K_L},
      {K_NULL}
    },
    // ZXC (tablet)
    {
      {K_TOGGLE, K_TOGGLE, K_TOGGLE, K_TOGGLE},
      {K_UPPER, K_UPPER, K_UPPER, K_UPPER},
      {K_z, K_UNDERSCORE, K_CARET, K_Z},
      {K_x, K_PLUS, K_LBRACE, K_X},
      {K_c, K_COLON, K_RBRACE, K_C},
      {K_v, K_LBRACKET, K_PIPE, K_V},
      {K_b, K_RBRACKET, K_PERCENT, K_B},
      {K_n, K_LESS, K_BACKTICK, K_N},
      {K_m, K_GREATER, K_TILDE, K_M},
      {K_BACKSPACE, K_BACKSPACE, K_BACKSPACE, K_BACKSPACE},
    },
    // FUNCs, SPACE (tablet)
    {
      {K_LINE_UP, K_PAGE_UP, K_LINE_UP, K_PAGE_UP},
      {K_LINE_DOWN, K_PAGE_DOWN, K_LINE_DOWN, K_PAGE_DOWN},
      {K_LPAREN, K_SLASH, K_COMMA, K_LBRACKET},
      {K_SPACE, K_SPACE, K_SPACE, K_SPACE},
      {K_RPAREN, K_HASH, K_EQUALS, K_RBRACKET},
      {K_LEFT, K_LEFT, K_LEFT, K_LEFT},
      {K_RIGHT, K_RIGHT, K_RIGHT, K_RIGHT},
      {K_ENTER, K_ENTER, K_ENTER, K_ENTER},
      {K_NULL},
    }
  };

  // tablet
  struct Impl : public KeypadLayout {
    KeypadLayoutStyle getKeypadLayoutStyle() const override {
      return kWide;
    }

    RawKey getRawKey(int row, int col) const override {
      return TabletKeypadLayout::KEYS[row][col];
    }

    int getMaxCols() const override {
      return ROW_LENGTHS[QWERTY_ROW];
    }

    int getMaxRows() const override {
      return MAX_ROWS;
    }

    int getRowLength(int row) const override {
      return ROW_LENGTHS[row];
    };

    int getSpaceCols() const override {
      return SPACE_COLS;
    }

    bool isCentered(int row) const override {
      return row == QWERTY_ROW || row == ASDF_ROW;
    }
  };
};

bool isPrintable(KeyCode key) {
  return key >= K_SPACE && key <= K_TILDE;
}

bool isArrow(KeyCode key) {
  bool result;
  switch (key) {
  case K_LEFT:
  case K_LINE_DOWN:
  case K_LINE_UP:
  case K_PAGE_DOWN:
  case K_PAGE_UP:
  case K_RIGHT:
    result = true;
    break;
  default:
    result = false;
  }
  return result;
}

bool isRightMargin(KeyCode key) {
  return key == K_ENTER || key == K_HELP || key == K_BACKSPACE;
}

KeypadLayout::~KeypadLayout() = default;

//
// KeypadImage
//
KeypadImage::KeypadImage() : ImageCodec() {
}

void KeypadImage::draw(int x, int y, int w, int h) const {
  MAPoint2d dstPoint;
  MARect srcRect;
  // buttons become narrow with larger font sizes
  int width = MIN(w, _width);
  dstPoint.x = x + (w - width) / 2;
  dstPoint.y = y + (h - _height) / 2;
  if (dstPoint.x < 0) {
    dstPoint.x = x;
  }
  if (dstPoint.y < 0) {
    dstPoint.y = y;
  }
  srcRect.left = 0;
  srcRect.top = 0;
  srcRect.width = width;
  srcRect.height = _height;
  maDrawRGB(&dstPoint, _pixels, &srcRect, 0, _width);
}

//
// KeypadDrawContext
//
KeypadDrawContext::KeypadDrawContext(int charWidth, int charHeight) :
  _charWidth(charWidth),
  _charHeight(charHeight),
  _imageSize(IMAGE_SIZE),
  _punctuation(false),
  _keySet(kLower) {
  if (!_cutImage.decode(img_cut, img_cut_len) ||
      !_copyImage.decode(img_copy, img_copy_len) ||
      !_pasteImage.decode(img_clipboard_paste, img_clipboard_paste_len) ||
      !_saveImage.decode(img_save, img_save_len) ||
      !_runImage.decode(img_bug_play, img_bug_play_len) ||
      !_helpImage.decode(img_book_info_2, img_book_info_2_len) ||
      !_backImage.decode(img_backspace, img_backspace_len) ||
      !_enterImage.decode(img_arrow_enter, img_arrow_enter_len) ||
      !_searchImage.decode(img_search, img_search_len) ||
      !_lineUpImage.decode(img_arrow_up, img_arrow_up_len) ||
      !_pageUpImage.decode(img_arrow_upload, img_arrow_upload_len) ||
      !_lineDownImage.decode(img_arrow_down, img_arrow_down_len) ||
      !_pageDownImage.decode(img_arrow_download, img_arrow_download_len) ||
      !_tagImage.decode(img_tag, img_tag_len) ||
      !_toggleImage.decode(img_layers, img_layers_len) ||
      !_leftImage.decode(img_arrow_left, img_arrow_left_len) ||
      !_rightImage.decode(img_arrow_right, img_arrow_right_len)) {
    deviceLog("%s", _cutImage.getLastError());
  }
}

const KeypadImage *KeypadDrawContext::getImage(const RawKey &key) const {
  const KeypadImage *result;
  switch (getKey(key)) {
  case K_CUT: result = &_cutImage; break;
  case K_COPY: result = &_copyImage; break;
  case K_PASTE: result = &_pasteImage; break;
  case K_SAVE: result = &_saveImage; break;
  case K_RUN: result = &_runImage; break;
  case K_HELP: result = &_helpImage; break;
  case K_BACKSPACE: result = &_backImage;break;
  case K_ENTER: result = &_enterImage; break;
  case K_SEARCH: result = &_searchImage; break;
  case K_TOGGLE: result = &_toggleImage; break;
  case K_LINE_UP: result = &_lineUpImage; break;
  case K_PAGE_UP: result = &_pageUpImage; break;
  case K_LINE_DOWN: result = &_lineDownImage; break;
  case K_PAGE_DOWN: result = &_pageDownImage; break;
  case K_TAG: result = &_tagImage; break;
  case K_LEFT: result = &_leftImage; break;
  case K_RIGHT: result = &_rightImage; break;
  default: result = nullptr; break;
  }
  return result;
}

KeyCode KeypadDrawContext::getKey(RawKey key) const {
  KeyCode keyCode;
  switch (_keySet) {
  case kLower: keyCode = key._lower; break;
  case kUpper: keyCode = key._upper; break;
  case kNumber: keyCode = key._number; break;
  case kSymbol: keyCode = key._symbol; break;
  case kSize: keyCode = K_NULL; break;
  default: keyCode = K_NULL; break;
  }
  return keyCode;
}

void KeypadDrawContext::layoutHeight(int padding) {
  const int baseSize = padding * 2 + _charHeight;
  const int imageSize = static_cast<int>(baseSize * IMAGE_SIZE_FACTOR);
  if (imageSize < _imageSize - 2 || imageSize > _imageSize + 2) {
    const int tagImageSize = static_cast<int>(baseSize * TAG_IMAGE_SIZE_FACTOR);
    const int pageImageSize = static_cast<int>(baseSize * PAGE_IMAGE_SIZE_FACTOR);
    _cutImage.resize(imageSize, imageSize);
    _copyImage.resize(imageSize, imageSize);
    _pasteImage.resize(imageSize, imageSize);
    _saveImage.resize(imageSize, imageSize);
    _runImage.resize(imageSize, imageSize);
    _helpImage.resize(imageSize, imageSize);
    _backImage.resize(imageSize, imageSize);
    _enterImage.resize(imageSize, imageSize);
    _searchImage.resize(imageSize, imageSize);
    _toggleImage.resize(imageSize, imageSize);
    _lineUpImage.resize(pageImageSize, pageImageSize);
    _lineDownImage.resize(pageImageSize, pageImageSize);
    _pageDownImage.resize(pageImageSize, pageImageSize);
    _pageUpImage.resize(pageImageSize, pageImageSize);
    _leftImage.resize(pageImageSize, pageImageSize);
    _rightImage.resize(pageImageSize, pageImageSize);
    _tagImage.resize(tagImageSize, tagImageSize);
    _imageSize = imageSize;
  }
}

void KeypadDrawContext::onClick(RawKey key) {
  switch (getKey(key)) {
  case K_ENTER:
  case K_RPAREN:
  case K_RBRACKET:
  case K_RBRACE:
    _keySet = kLower;
    _punctuation = false;
    break;
  default:
    _punctuation = (_keySet == kSymbol || _keySet == kNumber);
    break;
  }
}

void KeypadDrawContext::toggle() {
  if (_punctuation) {
    _keySet = kLower;
    _punctuation = false;
  } else {
    _keySet = static_cast<Keyset>((_keySet + 1) % kSize);
  }
}

void KeypadDrawContext::upper() {
  _keySet = _keySet == kLower ? kUpper : kLower;
}

//
// Key
//
Key::Key(const RawKey &k) :
  _key(k) {
  _printable = isPrintable(k._lower);
}

int Key::color(const KeypadTheme *theme) const {
  int result;
  if (_printable) {
    result = theme->_text;
  } else {
    result = theme->_funcKeyHighlight;
  }
  return result;
}

void Key::draw(const KeypadTheme *theme, const KeypadDrawContext *context, bool pressed) const {
  int rc = 5;
  int pad = 2;
  int pad_rc = pad + rc;
  int rx = _x + _w - pad; // right x
  int by = _y + _h - pad; // bottom y
  int lt = _x + pad_rc;   // left x (after corner)
  int vt = _y + pad_rc;   // top y (after corner)
  int rt = rx - rc;       // right x (before corner)
  int bt = by - rc;       // bottom y (before corner)
  int xcL = _x + pad_rc;  // x center for left arcs
  int xcR = rx - rc;      // x center for right arcs
  int ycT = _y + pad_rc;  // y center for top arcs
  int ycB = by - rc;      // y center for bottom arcs

  char keyChar = context->getKey(_key);
  bool printable = _printable && keyChar != K_TAG;

  // Set background color
  if (printable) {
    maSetColor(theme->_key);
    maFillRect(_x, _y, _w, _h);
  } else {
    maSetColor(theme->_funcKeyBg);
    maFillRect(_x + 1, _y + 1, _w - 2, _h - 2);
  }

  if (printable || pressed) {
    if (printable && pressed) {
      maSetColor(theme->_outline);
      maFillRect(_x + 4, _y + 4, _w - 7, _h - 7);
    }

    maSetColor(printable ? theme->_keyHighlight : theme->_funcKeyHighlight);

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

  if (printable) {
    char key[] = {keyChar, '\0'};
    int xOffset = (_w - context->_charWidth) / 2;
    int yOffset = (_h - context->_charHeight) / 2;
    int textX = _x + xOffset;
    int textY = _y + yOffset;
    maSetColor(color(theme));
    maDrawText(textX, textY, key, 1);
  } else if (keyChar == K_UPPER) {
    int xOffset = (_w - (context->_charWidth * 3)) / 2;
    int yOffset = (_h - context->_charHeight) / 2;
    int textX = _x + xOffset;
    int textY = _y + yOffset;
    maSetColor(color(theme));
    if (context->_keySet == kLower) {
      char key[] = {'A', 'B', 'C', '\0'};
      maDrawText(textX, textY, key, 3);
    } else {
      char key[] = {'a', 'b', 'c', '\0'};
      maDrawText(textX, textY, key, 3);
    }
  } else {
    auto *image = context->getImage(_key);
    if (image) {
      image->draw(_x, _y, _w, _h);
    }
  }
}

bool Key::inside(int x, int y) const {
  return (x >= _x &&
          x <= _xEnd &&
          y >= _y &&
          y <= _yEnd);
}

void Key::onClick(KeypadDrawContext *context) const {
  auto *event = new MAEvent();
  event->type = EVENT_TYPE_KEY_PRESSED;
  event->nativeKey = 0;
  switch (context->getKey(_key)) {
  case K_BACKSPACE:
    event->key = SB_KEY_BACKSPACE;
    break;
  case K_ENTER:
    event->key = SB_KEY_ENTER;
    context->onClick(_key);
    break;
  case K_SPACE:
    event->key = SB_KEY_SPACE;
    break;
  case K_SEARCH:
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
  case K_LINE_UP:
    event->key = SB_KEY_UP;
    break;
  case K_PAGE_UP:
    event->key = SB_KEY_PGUP;
    break;
  case K_LINE_DOWN:
    event->key = SB_KEY_DOWN;
    break;
  case K_PAGE_DOWN:
    event->key = SB_KEY_PGDN;
    break;
  case K_TAG:
    event->key = SB_KEY_CTRL('t');
    break;
  case K_LEFT:
    event->key = SB_KEY_LEFT;
    break;
  case K_RIGHT:
    event->key = SB_KEY_RIGHT;
    break;
  default:
    event->key = (unsigned char)context->getKey(_key);
    context->onClick(_key);
    break;
  }
  maPushEvent(event);
}

//
// Keypad
//
Keypad::Keypad(int screenWidth, int charWidth, int charHeight, bool toolbar)
  : _posX(0),
    _posY(0),
    _width(screenWidth),
    _height(0),
    _padding(static_cast<int>(charHeight * PADDING_FACTOR)),
    _toolbar(toolbar),
    _pressed(nullptr),
    _theme(&MODERN_DARK_THEME),
    _context(charWidth, charHeight) {
  selectLayout();
}

void Keypad::generateKeys() {
  _keys.clear();

  const int rows = _toolbar ? 1 : _layout->getMaxRows();
  for (int row = 0; row < rows; ++row) {
    int cols = _layout->getRowLength(row);
    for (int col = 0; col < cols; col++) {
      const RawKey &k = _layout->getRawKey(row, col);
      if (k._lower != K_NULL) {
        _keys.add(new Key(k));
      }
    }
  }
}

void Keypad::selectLayout() {
  if (_width >= WIDE_LAYOUT) {
    if (_layout == nullptr || _layout->getKeypadLayoutStyle() != kWide) {
      _layout = std::make_unique<TabletKeypadLayout::Impl>();
      generateKeys();
    }
  } else if (_layout == nullptr || _layout->getKeypadLayoutStyle() != kNarrow) {
    _layout = std::make_unique<MobileKeypadLayout::Impl>();
    generateKeys();
  }
}

void Keypad::layout(int x, int y, int w, int h) {
  _posX = x;
  _posY = y;
  _width = w;
  _height = h;

  if (!_toolbar) {
    selectLayout();
  }

  const int width = _width - _padding;
  const int keyW = width / _layout->getMaxCols();
  const int keyH = _context._charHeight + _padding * 2;
  const int xStart = _posX + ((w - _width) / 2);
  const int rows = _toolbar ? 1 : _layout->getMaxRows();
  int yPos = _posY;
  int index = 0;

  for (int row = 0; row < rows; ++row) {
    const int cols = _layout->getRowLength(row);
    int xPos = xStart;
    if (_layout->isCentered(row)) {
      const int rowWidth = keyW * cols;
      xPos += (_width - rowWidth) / 2;
    }
    for (int col = 0; col < cols; col++) {
      if (index >= (int)_keys.size()) {
        break;
      }
      Key *key = _keys[index++];
      int keyWidth = keyW;
      if (isRightMargin(key->_key._lower)) {
        keyWidth = _width - xPos;
      } else if (row == 0) {
        keyWidth = _width / cols;
      } else if (key->_key._lower == K_UPPER) {
        keyWidth = static_cast<int>(keyWidth * .8);
      } else if (isArrow(key->_key._lower)) {
        keyWidth = static_cast<int>(keyWidth * 1.2);
      } else if (!key->_printable && key->_key._lower != K_TAG) {
        const int numKeys = 2;
        keyWidth = (_width - ((cols - numKeys) * keyW)) / numKeys;
      } else if (key->_key._lower == K_SPACE) {
        keyWidth = (_layout->getSpaceCols() * keyW);
      }
      key->_x = xPos;
      key->_y = yPos;
      key->_w = keyWidth;
      key->_h = keyH;
      key->_xEnd = key->_x + key->_w;
      key->_yEnd = key->_y + key->_h;
      xPos += keyWidth;
    }
    yPos += keyH;
  }
}

int Keypad::layoutHeight(int screenHeight) {
  int charHeight = _context._charHeight;
  int maxHeight = static_cast<int>(screenHeight * MAX_HEIGHT_FACTOR);
  int padding = static_cast<int>(charHeight * PADDING_FACTOR);
  int rows = _toolbar ? 1 : _layout->getMaxRows();
  int height = rows * ((padding * 2) + charHeight);
  if (height > maxHeight) {
    // h = r(ch + 2p) -> p = (h - r * ch) / (r * 2)
    padding = ((maxHeight - (rows * charHeight)) / (rows * 2));
    height = rows * ((padding * 2) + charHeight);
  }
  _padding = padding;
  _context.layoutHeight(_padding);
  return height;
}

void Keypad::clicked(int x, int y, bool pressed) {
  Key *down = _pressed;
  _pressed = nullptr;
  for (const auto key : _keys) {
    if (key->inside(x, y)) {
      if (pressed) {
        _pressed = key;
      } else if (key->_key._lower == K_TOGGLE) {
        _context.toggle();
      } else if (key->_key._lower == K_UPPER) {
        _context.upper();
      } else if (key == down) {
        key->onClick(&_context);
      }
      break;
    }
  }
}

void Keypad::draw() const {
  maSetColor(_theme->_bg);
  maFillRect(_posX, _posY, _width, _height);
  for (const auto &key : _keys) {
    key->draw(_theme, &_context, key == _pressed);
  }
}

//
// KeypadInput
//
KeypadInput::KeypadInput(int screenWidth, bool floatTop, bool toolbar, int charWidth, int charHeight) :
  FormInput(0, 0, 0, charHeight * 2),
  _floatTop(floatTop) {
  _keypad = new Keypad(screenWidth, charWidth, charHeight, toolbar);
}

KeypadInput::~KeypadInput() {
  delete _keypad;
  _keypad = nullptr;
}

void KeypadInput::clicked(int x, int y, bool pressed) {
  _keypad->clicked(x, y, pressed);
}

void KeypadInput::draw(int x, int y, int w, int h, int chw) {
  _keypad->draw();
}

void KeypadInput::layout(int x, int y, int w, int h) {
  _x = x;
  _y = _floatTop ? 0 : h;
  _width = w;
  _keypad->layout(_x, _y, _width, _height);
}

int KeypadInput::layoutHeight(int screenHeight) {
  _height = _keypad->layoutHeight(screenHeight);
  return _height;
}
