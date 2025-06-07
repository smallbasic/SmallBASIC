// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "keypad.h"
#include "lib/maapi.h"
#include "common/device.h"

constexpr char cutIcon[] = {'\346', '\0'};
constexpr char copyIcon[] = {'\251', '\0'};
constexpr char pasteIcon[] = {'\273', '\0'};
constexpr char saveIcon[] = {'\247', '\0'};
constexpr char runIcon[] = {'\245', '\0'};
constexpr char helpIcon[] = {'\277', '\0'};
constexpr char shiftKey[] = "Shift";
constexpr char backKey[] = "Back";
constexpr char spaceKey[] = "         ";
constexpr char enterKey[] = "Enter";
constexpr char toggleKey[] = "?123";
constexpr int maxRows = 5;
constexpr int maxCols = 10;
constexpr int defaultPadding = 16;
constexpr double PI = 3.14159;

KeypadTheme retroTheme = {
  ._bg = 0x1e1e1e,          // Dark gray background
  ._key = 0x2d2d2d,         // Darker key face
  ._keyHighlight = 0x3d3d3d,// Key highlight: medium-dark gray
  ._text = 0xffffff,        // White text
  ._outline = 0x5e5e5e,     // Medium gray outlines
  ._funcKeyBg = 0x4b0082,   // Dark purple (Indigo)
  ._funcKeyHighlight = 0x000dad, // Vivid deep blue
  ._funcText = 0xffffff,    // White function key text
};

KeypadTheme modernDarkTheme = {
  ._bg = 0x121212,          // Very dark gray background
  ._key = 0x1f1f1f,         // Slightly lighter dark gray keys
  ._keyHighlight = 0x2c2c2c,// Highlighted key: a bit lighter
  ._text = 0xe0e0e0,        // Soft light gray text (was wrongly blue)
  ._outline = 0x3a3a3a,     // Subtle gray outlines
  ._funcKeyBg = 0x2962ff,   // Vibrant Material blue
  ._funcKeyHighlight = 0x448aff, // Lighter vibrant blue
  ._funcText = 0xffffff,    // White function key text
};

KeypadTheme modernLightTheme = {
  ._bg = 0xfafafa,          // Very light gray (off-white)
  ._key = 0xffffff,         // White key background
  ._keyHighlight = 0xe0e0e0,// Light gray for pressed keys
  ._text = 0x212121,        // Dark gray text
  ._outline = 0xcccccc,     // Soft gray outlines (corrected from teal-ish 0x00cccc)
  ._funcKeyBg = 0x1976d2,   // Material Design blue
  ._funcKeyHighlight = 0x63a4ff, // Lighter blue highlight
  ._funcText = 0xffffff,    // White text
};

constexpr RawKey letters[][maxCols] = {
  {{cutIcon, cutIcon}, {copyIcon, copyIcon}, {pasteIcon, pasteIcon}, {saveIcon, saveIcon}, {runIcon, runIcon}, {helpIcon, helpIcon}},
  {{"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"}, {"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"}, {"o", "O"}, {"p", "P"}},
  {{"a", "A"}, {"s", "S"}, {"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"}, {"j", "J"}, {"k", "K"}, {"l", "L"}},
  {{shiftKey, shiftKey}, {"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"}, {"b", "B"}, {"n", "N"}, {"m", "M"}, {backKey, backKey}},
  {{toggleKey, toggleKey}, {spaceKey, spaceKey}, {enterKey, enterKey}}
};

constexpr RawKey numbers[][maxCols] = {
  {{cutIcon, cutIcon}, {copyIcon, copyIcon}, {pasteIcon, pasteIcon}, {saveIcon, saveIcon}, {runIcon, runIcon}, {helpIcon, helpIcon}},
  {{"1", "!"}, {"2", "@"}, {"3", "#"}, {"4", "$"}, {"5", "%"}, {"6", "^"}, {"7", "&"}, {"8", "*"}, {"9", "("}, {"0", ")"}},
  {{"-", "_"}, {"/", "\\"}, {":", ";"}, {"(", ")"}, {"$", "€"}, {"&", "|"}, {"@", "~"}, {"\"", "'"}, {backKey, backKey}},
  {{toggleKey, toggleKey}, {spaceKey, spaceKey}, {enterKey, enterKey}}
};

constexpr RawKey symbols[][maxCols] = {
  {{cutIcon, cutIcon}, {copyIcon, copyIcon}, {pasteIcon, pasteIcon}, {saveIcon, saveIcon}, {runIcon, runIcon}, {helpIcon, helpIcon}},
  {{"[", "{"}, {"]", "}"}, {"{", "{"}, {"}", "}"}, {"#", "#"}, {"%", "%"}, {"^", "^"}, {"*", "*"}, {"+", "+"}, {"=", "="}},
  {{"_", "_"}, {"\\", "\\"}, {"|", "|"}, {"~", "~"}, {"<", "<"}, {">", ">"}, {"`", "`"}, {backKey, backKey}},
  {{toggleKey, toggleKey}, {spaceKey, spaceKey}, {enterKey, enterKey}}
};

constexpr int rowLengths[][5] = {
  {6, 10, 9, 9, 3}, // letters
  {6, 10, 9, 3, 0}, // numbers
  {6, 10, 8, 3, 0}, // symbols
};

constexpr int rowCharLengths[][5] = {
  {6, 10, 9, 16, 14},// letters
  {6, 10, 9, 14, 0}, // numbers
  {6, 10, 8, 14, 0}, // symbols
};

Key::Key(const RawKey &k) :
  _label(k._normal),
  _altLabel(k._shifted) {
  _labelLength = _label.length();
  _pressed = false;
  _special = (uint8_t )k._normal[0] > 128 || _labelLength > 1;
  _number = k._normal[0] >= '0' && k._normal[0] <= '9';
}

int Key::color(KeypadTheme *theme, bool shiftActive) const {
  int result;
  if (_pressed || (_label.equals(shiftKey) && shiftActive)) {
    result = _special ? theme->_funcKeyHighlight : theme->_keyHighlight;
  } else if (_special) {
    result = theme->_funcKeyHighlight;
  } else if (_number) {
    result = theme->_funcKeyHighlight;
  } else {
    result = theme->_text;
  }
  return result;
}

void Key::drawButton(KeypadTheme *theme) const {
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
  maSetColor(_special ? theme->_funcKeyBg : theme->_key);
  maFillRect(_x, _y, _w, _h);

  maSetColor(_special ? theme->_funcKeyHighlight : theme->_keyHighlight);

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

bool Key::inside(int x, int y) const {
  return (x >= _x &&
          x <= _x + _w &&
          y >= _y &&
          y <= _y + _h);
}

//
// Keypad
//
Keypad::Keypad(int charWidth, int charHeight)
  : _posX(0),
    _posY(0),
    _width(0),
    _height(0),
    _charWidth(charWidth),
    _charHeight(charHeight),
    _shiftActive(false),
    _capsLockActive(false),
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
  case LayoutNumbers:
    activeLayout = numbers;
    break;
  default:
    activeLayout = symbols;
    break;
  }

  for (int row = 0; row < maxRows; ++row) {
    int cols = rowLengths[_currentLayout][row];
    for (int col = 0; col < cols; col++) {
      const RawKey &k = activeLayout[row][col];
      if (k._normal != nullptr) {
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

  // start with optimum padding, then reduce to fit width
  int padding = defaultPadding;
  int width = maxCols * (_charWidth + (padding * 2));

  while (width > w && padding > 0) {
    padding--;
    width = maxCols * (_charWidth + (padding * 2));
  }

  int keyW = _charWidth + (padding * 2);
  int keyH = _charHeight + (padding * 2);
  int xStart = _posX + ((w - width) / 2);
  int yPos = _posY;
  int index = 0;

  for (int row = 0; row < maxRows; ++row) {
    int cols = rowLengths[_currentLayout][row];
    int chars = rowCharLengths[_currentLayout][row];
    int xPos = xStart;
    if (cols < maxCols) {
      // center narrow row
      int rowWidth = (chars * _charWidth) + (cols * padding * 2);
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
      int length = key->_labelLength;
      int keyWidth = keyW;
      if (length > 1) {
        keyWidth = (length * _charWidth) + (padding * 2);
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
      if (key->_label.equals(shiftKey)) {
        toggleShift();
      } else if (key->_label.equals(toggleKey)) {
        _currentLayout = static_cast<KeypadLayout>((_currentLayout + 1) % 3);
        generateKeys();
        layout(_posX, _posY, _width, _height);
        break;
      } else {
        bool useShift = _shiftActive ^ _capsLockActive;
        String label = useShift ? key->_altLabel : key->_label;
        fprintf(stderr, "pressed %s\n", label.c_str());

        if (_shiftActive && !key->_special) {
          _shiftActive = false;
        }
        // TODO: Output the key's character/command
      }
      break;
    }
  }
}

void Keypad::toggleShift() {
  _shiftActive = !_shiftActive;
}

void Keypad::draw() {
  maSetColor(_theme->_bg);
  maFillRect(_posX, _posY, _width, _height);
  for (const auto &key : _keys) {
    key->drawButton(_theme);

    String label;
    if (key->_special) {
      label = key->_label;
    } else {
      bool useShift = _shiftActive ^ _capsLockActive;
      label = useShift ? key->_altLabel : key->_label;
    }

    int labelLength = key->_labelLength;
    int xOffset = (key->_w - (_charWidth * labelLength)) / 2;
    int yOffset = (key->_h - _charHeight) / 2;
    int textX = key->_x + xOffset;
    int textY = key->_y + yOffset;
    maSetColor(key->color(_theme, _shiftActive));
    maDrawText(textX, textY, label.c_str(), labelLength);
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
