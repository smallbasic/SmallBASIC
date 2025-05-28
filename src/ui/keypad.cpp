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

constexpr RawKey letters[][10] = {
  {{cutIcon, cutIcon}, {copyIcon, copyIcon}, {pasteIcon, pasteIcon}, {saveIcon, saveIcon}, {runIcon, runIcon}, {helpIcon, helpIcon}},
  {{"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"}, {"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"}, {"o", "O"}, {"p", "P"}},
  {{"a", "A"}, {"s", "S"}, {"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"}, {"j", "J"}, {"k", "K"}, {"l", "L"}},
  {{shiftKey, shiftKey}, {"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"}, {"b", "B"}, {"n", "N"}, {"m", "M"}, {backKey, backKey}},
  {{toggleKey, toggleKey}, {spaceKey, spaceKey}, {enterKey, enterKey}}
};

constexpr RawKey numbers[][10] = {
  {{cutIcon, cutIcon}, {copyIcon, copyIcon}, {pasteIcon, pasteIcon}, {saveIcon, saveIcon}, {runIcon, runIcon}, {helpIcon, helpIcon}},
  {{"1", "!"}, {"2", "@"}, {"3", "#"}, {"4", "$"}, {"5", "%"}, {"6", "^"}, {"7", "&"}, {"8", "*"}, {"9", "("}, {"0", ")"}},
  {{"-", "_"}, {"/", "\\"}, {":", ";"}, {"(", ")"}, {"$", "€"}, {"&", "|"}, {"@", "~"}, {"\"", "'"}, {backKey, backKey}},
  {{toggleKey, toggleKey}, {spaceKey, spaceKey}, {enterKey, enterKey}}
};

constexpr RawKey symbols[][10] = {
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

constexpr int maxRows = 5;
constexpr int maxCols = 10;
constexpr int defaultPadding = 16;

Key::Key(const RawKey &k) :
  _label(k._normal),
  _altLabel(k._shifted) {
  _labelLength = _label.length();
  _pressed = false;
  _special = k._normal[0] > 128 || _labelLength > 1;
  _number = k._normal[0] >= '0' && k._normal[0] <= '9';
}

int Key::color(EditTheme *theme, bool shiftActive) {
  int result;
  if (_pressed || (_label.equals(shiftKey) && shiftActive)) {
    result = _special ? theme->_selection_background : theme->_number_selection_background;
  } else if (_special) {
    result = theme->_selection_color;
  } else if (_number) {
    result = theme->_number_color;
  } else {
    result = theme->_syntax_text;
  }
  return result;
}

void Key::drawButton(EditTheme *theme) {
  int rc = 1;
  int pad = 2;
  int rx = _x + _w - pad; // right x
  int by = _y + _h - pad; // bottom y
  int lt = _x + rc + pad; // left top
  int vt = _y + rc + pad; // vertical top
  int rt = rx - rc; // right top

  maSetColor(theme->_row_marker);
  maFillRect(_x, _y, _w, _h);
  maSetColor(theme->_cursor_color);
  maLine(lt, by, rt, by); // bottom
  maLine(rx, vt, rx, by); // right
}

bool Key::inside(int x, int y) const {
  return (x >= _x &&
          x <= _x + _w &&
          y >= _y &&
          y <= _y + _h);
}

Keypad::Keypad(int charWidth, int charHeight)
  : _posX(0),
    _posY(0),
    _width(0),
    _height(0),
    _charWidth(charWidth),
    _charHeight(charHeight),
    _shiftActive(false),
    _capsLockActive(false),
    _visible(true),
    _theme(nullptr),
    _currentLayout(LayoutLetters) {
  generateKeys();
}

int Keypad::outerHeight(int charHeight) const {
  return !_visible ? 0 : maxRows * ((defaultPadding * 2) + charHeight) + defaultPadding;
}

void Keypad::generateKeys() {
  _keys.clear();

  const RawKey (*activeLayout)[10] = nullptr;
  switch (_currentLayout) {
  case LayoutLetters:
    activeLayout = letters;
    break;
  case LayoutNumbers:
    activeLayout = numbers;
    break;
  case LayoutSymbols:
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
        if (_shiftActive && !key->_special) {
          _shiftActive = false;
        }
        // TODO: Output the key's character/command
      }
    }
  }
}

void Keypad::toggleShift() {
  _shiftActive = !_shiftActive;
}

void Keypad::draw() {
  maSetColor(_theme->_background);
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
    int textX = key->_x + xOffset;
    int textY = key->_y + key->_h / 4;
    maSetColor(key->color(_theme, _shiftActive));
    maDrawText(textX, textY, label.c_str(), labelLength);
  }
}

