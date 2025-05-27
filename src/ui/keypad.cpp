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

constexpr char cutIcon[] = {'\367', '\0'};
constexpr char copyIcon[] = {'\251', '\0'};
constexpr char pasteIcon[] = {'\273', '\0'};
constexpr char saveIcon[] = {'\247', '\0'};
constexpr char runIcon[] = {'\245', '\0'};
constexpr char helpIcon[] = {'\277', '\0'};
constexpr char shiftKey[] = "Shift";
constexpr char backKey[] = "Back";
constexpr char spaceKey[] = "SPACE";
constexpr char enterKey[] = "Enter";
constexpr char toggleKey[] = "?123";

constexpr RawKey letters[][10] = {
  {{cutIcon, "Cut"}, {copyIcon, "Copy"}, {pasteIcon, "Paste"}, {saveIcon, "Save"}, {runIcon, "Run"}, {helpIcon, "Help"}},
  {{"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"}, {"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"}, {"o", "O"}, {"p", "P"}},
  {{"a", "A"}, {"s", "S"}, {"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"}, {"j", "J"}, {"k", "K"}, {"l", "L"}},
  {{shiftKey, shiftKey}, {"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"}, {"b", "B"}, {"n", "N"}, {"m", "M"}, {backKey, backKey}},
  {{toggleKey, toggleKey}, {spaceKey, spaceKey}, {enterKey, enterKey}}
};

constexpr RawKey numbers[][10] = {
  {{cutIcon, "Cut"}, {copyIcon, "Copy"}, {pasteIcon, "Paste"}, {saveIcon, "Save"}, {runIcon, "Run"}, {helpIcon, "Help"}},
  {{"1", "!"}, {"2", "@"}, {"3", "#"}, {"4", "$"}, {"5", "%"}, {"6", "^"}, {"7", "&"}, {"8", "*"}, {"9", "("}, {"0", ")"}},
  {{"-", "_"}, {"/", "\\"}, {":", ";"}, {"(", ")"}, {"$", "€"}, {"&", "|"}, {"@", "~"}, {"\"", "'"}, {backKey, backKey}},
  {{toggleKey, toggleKey}, {spaceKey, spaceKey}, {enterKey, enterKey}}
};

constexpr RawKey symbols[][10] = {
  {{cutIcon, "Cut"}, {copyIcon, "Copy"}, {pasteIcon, "Paste"}, {saveIcon, "Save"}, {runIcon, "Run"}, {helpIcon, "Help"}},
  {{"[", "{"}, {"]", "}"}, {"{", "{"}, {"}", "}"}, {"#", "#"}, {"%", "%"}, {"^", "^"}, {"*", "*"}, {"+", "+"}, {"=", "="}},
  {{"_", "_"}, {"\\", "\\"}, {"|", "|"}, {"~", "~"}, {"<", "<"}, {">", ">"}, {"`", "`"}, {backKey, backKey}},
  {{toggleKey, toggleKey}, {spaceKey, spaceKey}, {enterKey, enterKey}}
};

constexpr int rowLengths[][5] = {
  {6, 10, 9, 9, 3}, // letters
  {6, 10, 9, 3, 0}, // numbers
  {6, 10, 8, 3, 0}, // symbols
};

constexpr int maxRows = 5;
constexpr int maxCols = 10;
constexpr int numRows = 6;
constexpr int keyMargin = 4;

Key::Key(const RawKey &k) :
  _label(k._normal),
  _altLabel(k._shifted) {
  _pressed = false;
  _special = k._normal[0] > 128 || _label.length() > 1;
}

void Key::draw(EditTheme *theme, bool shiftActive, bool capsLockActive) {
  int rx = _w / 2;
  int ry = _h / 2;

  int color = theme->_number_color;
  if (_pressed || (_label.equals(shiftKey) && shiftActive)) {
    color = _special ? theme->_selection_background : theme->_number_selection_background;
  } else if (_special) {
    color = theme->_selection_color;
  }

  maSetColor(color);
  maEllipse(_x + rx, _y + ry, rx, ry, 1);
  maSetColor(theme->_row_marker);
  maEllipse(_x + rx, _y + ry, rx, ry, 0);
  maSetColor(theme->_syntax_text);

  String label;
  if (_special) {
    label = _label;
  } else {
    bool useShift = shiftActive ^ capsLockActive;
    label = useShift ? _altLabel : _label;
  }

  int textX = _x + _w / 4;
  int textY = _y + _h / 4;
  maDrawText(textX, textY, label.c_str(), label.length());
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
  return !_visible ? 0 : numRows * ((keyMargin * 4) + charHeight);
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
  int padding = 12;
  int width = maxCols * (_charWidth + (padding * 2));

  while (width > w && padding > 0) {
    padding--;
    width = maxCols * (_charWidth + (padding * 2));
  }

  int keyW = _charWidth + (padding * 2);
  int keyH = _charHeight + (padding * 2);
  int index = 0;
  int xStart = _posX + ((w - width) / 2);
  int yPos = _posY;

  for (int row = 0; row < maxRows; ++row) {
    int cols = rowLengths[_currentLayout][row];
    int xPos = xStart;
    if (cols < maxCols) {
      // 8/9th of width / 2
      int xOffs = (cols * width / maxCols) / 2;
      xPos += xOffs;
    }
    for (int col = 0; col < cols; col++) {
      if (index >= (int)_keys.size()) {
        break;
      }
      Key *key = _keys[index++];
      key->_x = xPos;
      key->_y = yPos;
      key->_w = keyW;
      key->_h = keyH;
      xPos += keyW;
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
    key->draw(_theme, _shiftActive, _capsLockActive);
  }
}

