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

constexpr int ROW_LENGTHS[] = {7, 10, 9, 9, 8};
constexpr int MAX_ROWS = 5;
constexpr int MAX_COLS = 10;
constexpr int QWERTY_ROW = 1;
constexpr int ASDF_ROW = 2;
constexpr int SPACE_COLS = 3;
constexpr int IMAGE_SIZE = 30;
constexpr double PI = 3.14159;

// padding size based on character height
constexpr double PADDING_FACTOR = 1.1;

// maximum keyboard height as based on screen height
constexpr double MAX_HEIGHT_FACTOR = 0.48;

// image height based on available rectangle
constexpr double IMAGE_SIZE_FACTOR = 0.65;

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

constexpr RawKey KEYS[MAX_ROWS][MAX_COLS] = {
  // Toolbar
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
  // QWERTY
  {
    {K_q, K_Q, K_1, K_EXCLAIM},
    {K_w, K_W, K_2, K_AT},
    {K_e, K_E, K_3, K_HASH},
    {K_r, K_R, K_4, K_DOLLAR},
    {K_t, K_T, K_5, K_PERCENT},
    {K_y, K_Y, K_6, K_CARET},
    {K_u, K_U, K_7, K_AMPERSAND},
    {K_i, K_I, K_8, K_ASTERISK},
    {K_o, K_O, K_9, K_LPAREN},
    {K_p, K_P, K_0, K_RPAREN}
  },
  // ASDF
  {
    {K_a, K_A, K_BACKTICK, K_TILDE},
    {K_s, K_S, K_MINUS, K_UNDERSCORE},
    {K_d, K_D, K_EQUALS, K_PLUS},
    {K_f, K_F, K_LBRACKET, K_LBRACE},
    {K_g, K_G, K_RBRACKET, K_RBRACE},
    {K_h, K_H, K_BACKSLASH, K_PIPE},
    {K_j, K_J, K_SEMICOLON, K_COLON},
    {K_k, K_K, K_APOSTROPHE, K_QUOTE},
    {K_l, K_L, K_HASH, K_TAG},
    {K_NULL}
  },
  // ZXC
  {
    {K_TOGGLE, K_TOGGLE, K_TOGGLE, K_TOGGLE},
    {K_z, K_Z, K_LESS, K_COMMA},
    {K_x, K_X, K_GREATER, K_PERIOD},
    {K_c, K_C, K_QUESTION, K_SLASH},
    {K_v, K_V, K_PLUS, K_EXT2},
    {K_b, K_B, K_ASTERISK, K_EXT3},
    {K_n, K_N, K_LPAREN, K_EXT4},
    {K_m, K_M, K_RPAREN, K_EXT5},
    {K_BACKSPACE, K_BACKSPACE, K_BACKSPACE, K_BACKSPACE},
    {K_NULL}
  },
  // FUNCs, SPACE
  {
    {K_LINE_UP, K_PAGE_UP, K_LINE_UP, K_PAGE_UP},
    {K_LINE_DOWN, K_PAGE_DOWN, K_LINE_DOWN, K_PAGE_DOWN},
    {K_LPAREN, K_LBRACKET, K_LPAREN, K_LBRACKET},
    {K_SPACE, K_SPACE, K_SPACE, K_SPACE},
    {K_RPAREN, K_RBRACKET, K_RPAREN, K_RBRACKET},
    {K_ENTER, K_ENTER, K_ENTER, K_ENTER},
    {K_NULL},
    {K_NULL},
    {K_NULL},
    {K_NULL}
  }
};

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
      !_toggleImage.decode(img_layers, img_layers_len)) {
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
  }
  return keyCode;
}

void KeypadDrawContext::layoutHeight(int padding) {
  const int imageSize = static_cast<int>((padding * 2 + _charHeight) * IMAGE_SIZE_FACTOR);
  if (imageSize < _imageSize - 2 || imageSize > _imageSize + 2) {
    _cutImage.resize(imageSize, imageSize);
    _copyImage.resize(imageSize, imageSize);
    _pasteImage.resize(imageSize, imageSize);
    _saveImage.resize(imageSize, imageSize);
    _runImage.resize(imageSize, imageSize);
    _helpImage.resize(imageSize, imageSize);
    _backImage.resize(imageSize, imageSize);
    _enterImage.resize(imageSize, imageSize);
    _searchImage.resize(imageSize, imageSize);
    _lineUpImage.resize(imageSize, imageSize);
    _pageUpImage.resize(imageSize, imageSize);
    _lineDownImage.resize(imageSize, imageSize);
    _pageDownImage.resize(imageSize, imageSize);
    _toggleImage.resize(imageSize, imageSize);
    _tagImage.resize(imageSize, imageSize);
    _imageSize = imageSize;
  }
}

void KeypadDrawContext::toggle() {
  _keySet = static_cast<Keyset>((_keySet + 1) % kSize);
}

//
// Key
//
Key::Key(const RawKey &k) :
  _key(k) {
  _printable = isPrintable(k._lower) || isExtended(k._lower);
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

char Key::getKey(const KeypadDrawContext *context) const {
  char result;
  KeyCode keyCode = context->getKey(_key);
  switch (keyCode) {
  case K_EXT1: result = (char)164; break;
  case K_EXT2: result = (char)172; break;
  case K_EXT3: result = (char)182; break;
  case K_EXT4: result = (char)222; break;
  case K_EXT5: result = (char)223; break;
  default:
    result = keyCode;
    break;
  }
  return result;
}

void Key::draw(const KeypadTheme *theme, const KeypadDrawContext *context, bool pressed) const {
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

  char keyChar = getKey(context);
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

void Key::onClick(const KeypadDrawContext *context) const {
  auto *event = new MAEvent();
  event->type = EVENT_TYPE_KEY_PRESSED;
  event->nativeKey = 0;
  switch (context->getKey(_key)) {
  case K_BACKSPACE:
    event->key = SB_KEY_BACKSPACE;
    break;
  case K_ENTER:
    event->key = SB_KEY_ENTER;
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
  default:
    event->key = (unsigned char)getKey(context);
    break;
  }
  maPushEvent(event);
}

//
// Keypad
//
Keypad::Keypad(int charWidth, int charHeight, bool toolbar)
  : _posX(0),
    _posY(0),
    _width(0),
    _height(0),
    _padding(static_cast<int>(charHeight * PADDING_FACTOR)),
    _theme(&MODERN_DARK_THEME),
    _context(charWidth, charHeight),
    _toolbar(toolbar),
    _pressed(nullptr) {
  generateKeys();
}

void Keypad::generateKeys() {
  _keys.clear();

  const int rows = _toolbar ? 1 : MAX_ROWS;
  for (int row = 0; row < rows; ++row) {
    int cols = ROW_LENGTHS[row];
    for (int col = 0; col < cols; col++) {
      const RawKey &k = KEYS[row][col];
      if (k._lower != K_NULL) {
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

  const int width = _width - _padding;
  const int keyW = width / ROW_LENGTHS[QWERTY_ROW];
  const int keyH = _context._charHeight + _padding * 2;
  const int xStart = _posX + ((w - _width) / 2);
  const int rows = _toolbar ? 1 : MAX_ROWS;
  int yPos = _posY;
  int index = 0;

  for (int row = 0; row < rows; ++row) {
    const int cols = ROW_LENGTHS[row];
    int xPos = xStart;
    if (row == QWERTY_ROW || row == ASDF_ROW) {
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
      } else if (isArrow(key->_key._lower)) {
        keyWidth = static_cast<int>(keyWidth * 1.2);
      } else if (!key->_printable) {
        const int numKeys = 2;
        keyWidth = (_width - ((cols - numKeys) * keyW)) / numKeys;
      } else if (key->_key._lower == K_SPACE) {
        keyWidth = (SPACE_COLS * keyW);
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
  int rows = _toolbar ? 1 : MAX_ROWS;
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
KeypadInput::KeypadInput(bool floatTop, bool toolbar, int charWidth, int charHeight) :
  FormInput(0, 0, 0, charHeight * 2),
  _floatTop(floatTop) {
  _keypad = new Keypad(charWidth, charHeight, toolbar);
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
