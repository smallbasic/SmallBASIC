// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <cstdlib>
//#include <cstring>

#include "ui/textedit.h"
#include "ui/inputs.h"
#include "ui/utils.h"
#include "ui/strlib.h"
#include "ui/kwp.h"
#include "ui/keypad.h"

void safe_memmove(void *dest, const void *src, size_t n) {
  if (n > 0 && dest != nullptr && src != nullptr) {
    memmove(dest, src, n);
  }
}

#define STB_TEXTEDIT_IS_SPACE(ch) IS_WHITE(ch)
#define STB_TEXTEDIT_IS_PUNCT(ch) ((ch) != '_' && (ch) != '$' && ispunct(ch))
#define IS_VAR_CHAR(ch) ((ch) == '_' || (ch) == '$' || isalpha(ch) || isdigit(ch))
#define STB_TEXTEDIT_memmove safe_memmove
#define STB_TEXTEDIT_IMPLEMENTATION

int is_word_border(EditBuffer *_str, int _idx) {
  return _idx > 0 ? ((STB_TEXTEDIT_IS_SPACE(STB_TEXTEDIT_GETCHAR(_str,_idx-1)) ||
                      STB_TEXTEDIT_IS_PUNCT(STB_TEXTEDIT_GETCHAR(_str,_idx-1))) &&
                     !STB_TEXTEDIT_IS_SPACE(STB_TEXTEDIT_GETCHAR(_str, _idx))) : 1;
}

int textedit_move_to_word_previous(EditBuffer *str, int c) {
  --c; // always move at least one character
  while (c >= 0 && !is_word_border(str, c)) {
    --c;
  }
  if (c < 0) {
    c = 0;
  }
  return c;
}

int textedit_move_to_word_next(EditBuffer *str, int c) {
  const int len = str->_len;
  ++c; // always move at least one character
  while (c < len && !is_word_border(str, c)) {
    ++c;
  }
  if (c > len) {
    c = len;
  }
  return c;
}

#define STB_TEXTEDIT_MOVEWORDLEFT textedit_move_to_word_previous
#define STB_TEXTEDIT_MOVEWORDRIGHT textedit_move_to_word_next

#pragma GCC diagnostic ignored "-Wunused-function"
#include "lib/stb/stb_textedit.h"
#pragma GCC diagnostic pop

#define GROW_SIZE 128
#define LINE_BUFFER_SIZE 200
#define INDENT_LEVEL 2
#define HELP_WIDTH 22
#define LEVEL1_CLOSE "[v] "
#define LEVEL1_OPEN  "[>] "
#define LEVEL2_OPEN  "   +- "
#define LEVEL2_CLOSE "  [*] "
#define LEVEL1_LEN 4
#define LEVEL1_OFFSET 6
#define LEVEL2_LEN 6
#define LEVEL2_CLOSE_LEN 6
#define HELP_BG 0x20242a
#define HELP_FG 0x73c990
#define DOUBLE_CLICK_MS 200
#define SIDE_BAR_WIDTH 30

#if defined(_Win32)
#include <shlwapi.h>
#define strcasestr StrStrI
#endif

unsigned g_themeId = 0;
int g_lineMarker[MAX_MARKERS] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

const char *themeName() {
  switch (g_themeId) {
  case 0: return "Dark";
  case 1: return "Light";
  case 2: return "Shian";
  case 3: return "ATOM 1";
  case 4: return "ATOM 2";
  case 5: return "R157";
  default: return "";
  }
}

// see: http://ethanschoonover.com/solarized#features
#define sol_base03  0x002b36
#define sol_base02  0x073642
#define sol_base01  0x586e75
#define sol_base00  0x657b83
#define sol_base0   0x839496
#define sol_base1   0x93a1a1
#define sol_base2   0xeee8d5
#define sol_base3   0xfdf6e3
#define sol_yellow  0xb58900
#define sol_orange  0xcb4b16
#define sol_red     0xdc322f
#define sol_magenta 0xd33682
#define sol_violet  0x6c71c4
#define sol_blue    0x268bd2
#define sol_cyan    0x2aa198
#define sol_green   0x859900

// 0 - color
// 1 - selection_color
// 2 - number_color
// 3 - number_selection_color
// 4 - cursor_color
// 5 - syntax_comments
// 6 - background
// 7 - selection_background
// 8 - number_selection_background
// 9 - cursor_background
// 10 - match_background
// 11 - row_cursor
// 12 - syntax_text
// 13 - syntax_command
// 14 - syntax_statement
// 15 - syntax_digit
// 16 - row_marker

const int solarized_dark[] = {
  sol_base0, sol_base02, sol_base01, sol_base02, 0xa7aebc, sol_base01,
  sol_base03, sol_base1, sol_base0, 0x3875ed, 0x373b88, sol_base02,
  sol_green, sol_yellow, sol_blue, sol_cyan, 0x0083f8
};

const int solarized_light[] = {
  sol_base00, sol_base02, sol_base1, sol_base03, 0xa7aebc, sol_base1,
  sol_base3, sol_base1, sol_base0, 0x3875ed, 0x373b88, sol_base2,
  sol_green, sol_violet, sol_yellow, sol_blue, 0x0083f8
};

const int shian[] = {
  0xcccccc, 0x000077, 0x333333, 0x333333, 0x0000aa, 0x008888,
  0x010101, 0xeeeeee, 0x010101, 0xffff00, 0x00ff00, 0x010101,
  0x00ffff, 0xff00ff, 0xffffff, 0x00ffff, 0x00aaff
};

const int atom1[] = {
  0xc8cedb, 0xa7aebc, 0x484f5f, 0xa7aebc, 0xa7aebc, 0x00bb00,
  0x272b33, 0x3d4350, 0x2b3039, 0x3875ed, 0x373b88, 0x2b313a,
  0x0083f8, 0xff9d00, 0x31ccac, 0xc679dd, 0x0083f8
};

const int atom2[] = {
  0xc8cedb, 0xd7decc, 0x484f5f, 0xa7aebc, 0xa7aebc, 0x00bb00,
  0x001b33, 0x0088ff, 0x000d1a, 0x0051b1, 0x373b88, 0x022444,
  0x0083f8, 0xff9d00, 0x31ccac, 0xc679dd, 0x0083f8
};

const int r157[] = {
  0x80dfff, 0xa7aebc, 0xffffff, 0xa7aebc, 0xa7aebc, 0xd0d6e1,
  0x2e3436, 0x888a85, 0x000000, 0x4d483b, 0x000000, 0x576375,
  0xffffff, 0xffc466, 0xffcce0, 0xffff66, 0x0083f8
};

int g_user_theme[] = {
  0xc8cedb, 0xa7aebc, 0x484f5f, 0xa7aebc, 0xa7aebc, 0x00bb00,
  0x2e3436, 0x888a85, 0x000000, 0x4d483b, 0x000000, 0x2b313a,
  0x0083f8, 0xff9d00, 0x31ccac, 0xc679dd, 0x0083f8
};

const int* themes[] = {
  solarized_dark, solarized_light, shian, atom1, atom2, r157, g_user_theme
};

const char *helpText =
  "C-a select-all\n"
  "C-b back, exit\n"
  "C-k delete line\n"
  "C-d delete char\n"
  "C-s save\n"
  "C-x cut\n"
  "C-c copy\n"
  "C-v paste\n"
  "C-z undo\n"
  "C-y redo\n"
  "C-f find, find-next\n"
  "C-n find, replace\n"
  "C-t toggle marker\n"
  "C-g goto marker\n"
  "C-l outline\n"
  "C-o show output\n"
  "C-SPC auto-complete\n"
  "C-home top\n"
  "C-end bottom\n"
  "C-5,6,7 macro\n"
  "A-c change case\n"
  "A-d kill word\n"
  "A-g goto line\n"
  "A-n trim line-endings\n"
  "A-t select theme\n"
  "A-w select word\n"
  "A-. return mode\n"
  "A-<n> recent file\n"
  "A-= count chars\n"
  "SHIFT-<arrow> select\n"
  "TAB indent line\n"
  "F1,A-h keyword help\n"
  "F2 online help\n"
  "F3,F4 export\n"
  "F5,F6,F7 debug\n"
  "F8 live edit\n"
  "F9, C-r run\n"
  "F10 set command$\n"
  "F11 full screen\n";

inline bool match(const char *str, const char *pattern , int len) {
  int i, j;
  for (i = 0, j = 0; i < len; i++, j += 2) {
    if (str[i] != pattern[j] && str[i] != pattern[j + 1]) {
      break;
    }
  }
  return i == len;
}

inline bool is_comment(const char *str, int offs) {
  return (str[offs] == '\'' || (str[offs] == '#' && !isdigit(str[offs + 1]))
          || match(str + offs, "RrEeMm  ", 3));
}

int compareIntegers(const void *p1, const void *p2) {
  int i1 = *((int *)p1);
  int i2 = *((int *)p2);
  return i1 < i2 ? -1 : i1 == i2 ? 0 : 1;
}

const char *find_str(bool allUpper, const char *haystack, const char *needle) {
  return allUpper ? strstr(haystack, needle) : strcasestr(haystack, needle);
}

int shade(int c, float weight) {
  uint8_t r = ((uint8_t)(c >> 16));
  uint8_t g = ((uint8_t)(c >> 8));
  uint8_t b = ((uint8_t)(c));
  r = (r * weight);
  g = (g * weight);
  b = (b * weight);
  r = r < 255 ? r : 255;
  g = g < 255 ? g : 255;
  b = b < 255 ? b : 255;
  return (r << 16) + (g << 8) + (b);
}

//
// TODO: move to keypad.cpp
//
Keypad *g_keypad = nullptr;

//
// EditTheme
//
EditTheme::EditTheme() :_plainText(false) {
  if (g_themeId >= (sizeof(themes) / sizeof(themes[0]))) {
    g_themeId = 0;
  }
  selectTheme(themes[g_themeId]);
}

EditTheme::EditTheme(int fg, int bg) :
  _plainText(true),
  _color(fg),
  _background(bg),
  _selection_color(bg),
  _selection_background(fg),
  _number_color(fg),
  _number_selection_color(fg),
  _number_selection_background(bg),
  _cursor_color(bg),
  _cursor_background(fg),
  _match_background(fg),
  _row_cursor(bg),
  _syntax_comments(bg),
  _syntax_text(fg),
  _syntax_command(fg),
  _syntax_statement(fg),
  _syntax_digit(fg),
  _row_marker(fg) {
}

void EditTheme::setId(const unsigned themeId) {
  if (themeId >= (sizeof(themes) / sizeof(themes[0]))) {
    selectTheme(themes[0]);
  } else {
    selectTheme(themes[themeId]);
  }
}

void EditTheme::selectTheme(const int theme[]) {
  _color = theme[0];
  _selection_color = theme[1];
  _number_color = theme[2];
  _number_selection_color = theme[3];
  _cursor_color = theme[4];
  _syntax_comments = theme[5];
  _background = theme[6];
  _selection_background = theme[7];
  _number_selection_background = theme[8];
  _cursor_background = theme[9];
  _match_background = theme[10];
  _row_cursor = theme[11];
  _syntax_text = theme[12];
  _syntax_command = theme[13];
  _syntax_statement = theme[14];
  _syntax_digit = theme[15];
  _row_marker = theme[16];
}

void EditTheme::contrast(EditTheme *other) {
  int fg = shade(other->_color, .65);
  int bg = shade(other->_background, .65);
  _color = fg;
  _background = bg;
  _selection_color = bg;
  _selection_background = shade(bg, .65);
  _number_color = fg;
  _number_selection_color = fg;
  _number_selection_background = bg;
  _cursor_color = bg;
  _cursor_background = fg;
  _match_background = fg;
  _row_cursor = bg;
  _syntax_comments = bg;
  _syntax_text = fg;
  _syntax_command = fg;
  _syntax_statement = fg;
  _syntax_digit = fg;
  _row_marker = fg;
}

//
// EditBuffer
//
EditBuffer::EditBuffer(TextEditInput *in, const char *text) :
  _buffer(nullptr),
  _len(0),
  _size(0),
  _lines(-1),
  _in(in) {
  if (text != nullptr && text[0]) {
    _len = strlen(text);
    _size = _len + 1;
    _buffer = (char *)malloc(_size);
    memcpy(_buffer, text, _len);
    _buffer[_len] = '\0';
    convertTabs();
  }
}

EditBuffer::~EditBuffer() {
  clear();
}

void EditBuffer::convertTabs() {
  for (int i = 0; i < _len; i++) {
    if (_buffer[i] == '\t') {
      _buffer[i] = ' ';
    }
  }
}

void EditBuffer::clear() {
  free(_buffer);
  _buffer = nullptr;
  _len = _size = 0;
  _lines = -1;
}

int EditBuffer::countNewlines(const char *text, int num) {
  int result = 0;
  for (int i = 0; i < num; i++) {
    if (text[i] == '\n') {
      result++;
    }
  }
  return result;
}

int EditBuffer::deleteChars(int pos, int num) {
  if (num > 1) {
    _lines -= countNewlines(_buffer + pos, num);
  } else if (_buffer[pos] == '\n') {
    _lines--;
  }

  if (_len - (pos + num) > 0) {
    memmove(&_buffer[pos], &_buffer[pos + num], _len - (pos + num));
  }
  // otherwise no more characters to pull back over the hole
  _len -= num;
  if (_len < 0) {
    _len = 0;
  }
  _buffer[_len] = '\0';
  _in->setDirty(true);
  return 1;
}

char EditBuffer::getChar(int pos) const {
  char result;
  if (_buffer != nullptr && pos >= 0 && pos < _len) {
    result = _buffer[pos];
  } else {
    result = '\0';
  }
  if (result == '\r') {
    result = '\n';
  }
  return result;
}

int EditBuffer::insertChars(int pos, const char *text, int num) {
  if (num == 1 && *text < 0) {
    return 0;
  }
  int required = _len + num + 1;
  if (required >= _size) {
    _size += (required + GROW_SIZE);
    _buffer = (char *)realloc(_buffer, _size);
  }
  if (_len - pos > 0) {
    memmove(&_buffer[pos + num], &_buffer[pos], _len - pos);
  }
  memcpy(&_buffer[pos], text, num);
  _len += num;
  _buffer[_len] = '\0';
  _in->setDirty(true);
  convertTabs();
  if (num > 1) {
    _lines += countNewlines(text, num);
  } else if (text[0] == '\n') {
    _lines++;
  }
  return 1;
}

int EditBuffer::lineCount() {
  if (_lines < 0) {
    _lines = 1 + countNewlines(_buffer, _len);
  }
  return _lines;
}

char *EditBuffer::textRange(int start, int end) const {
  char *result;
  int len;
  if (start < 0 || start > _len || end <= start) {
    len = 0;
    result = (char*)malloc(len + 1);
  } else {
    if (end > _len) {
      end = _len;
    }
    len = end - start;
    result = (char*)malloc(len + 1);
    memcpy(result, &_buffer[start], len);
  }
  result[len] = '\0';
  return result;
}

void EditBuffer::removeTrailingSpaces(STB_TexteditState *state) {
  int lineEnd = _len - 1;
  int lastChar = lineEnd;
  bool atEnd = true;

  for (int i = _len - 1; i >= 0; i--) {
    if (_buffer[i] == '\n' || i == 0) {
      // boundary - set new lineEnd
      if (atEnd && lastChar < lineEnd) {
        stb_textedit_delete(this, state, lastChar + 1, lineEnd - lastChar);
      }
      lineEnd = lastChar = i - 1;
      atEnd = true;
    } else if (atEnd) {
      if (_buffer[i] == '\r' ||
          _buffer[i] == '\t' ||
          _buffer[i] == ' ') {
        // whitespace
        lastChar--;
      } else {
        // no more whitespace
        if (lastChar < lineEnd) {
          stb_textedit_delete(this, state, lastChar + 1, lineEnd - lastChar);
        }
        atEnd = false;
      }
    }
  }
}

//
// TextEditInput
//
TextEditInput::TextEditInput(const char *text, int chW, int chH,
                             int x, int y, int w, int h) :
  FormEditInput(x, y, w, h),
  _buf(this, text),
  _theme(nullptr),
  _charWidth(chW),
  _charHeight(chH),
  _marginWidth(0),
  _scroll(0),
  _cursorCol(0),
  _cursorRow(0),
  _cursorLine(0),
  _indentLevel(INDENT_LEVEL),
  _matchingBrace(-1),
  _ptY(-1),
  _pressTick(0),
  _xmargin(0),
  _ymargin(0),
  _bottom(false),
  _dirty(false),
  _comment(true),
  _showKeypad(false) {
  stb_textedit_initialize_state(&_state, false);
  _resizable = true;
}

TextEditInput::~TextEditInput() {
  delete _theme;
  _theme = nullptr;

  delete g_keypad;
  g_keypad = nullptr;
}

void TextEditInput::completeWord(const char *word) {
  if (_state.select_start == _state.select_end) {
    int start = wordStart();
    int end = _state.cursor;
    int len = end - start;
    int insertLen = strlen(word) - len;
    int index = end == 0 ? 0 : end - 1;
    bool lastUpper = isupper(_buf._buffer[index]);

    paste(word + len);
    for (int i = 0; i < insertLen; i++) {
      char c = _buf._buffer[i + end];
      _buf._buffer[i + end] = lastUpper ? toupper(c) : tolower(c);
    }
  }
}

const char *TextEditInput::completeKeyword(int index) {
  const char *help = nullptr;
  char *selection = getWordBeforeCursor();
  if (selection != nullptr) {
    int len = strlen(selection);
    int count = 0;
    for (auto & i : keyword_help) {
      if (strncasecmp(selection, i.keyword, len) == 0 &&
          count++ == index) {
        if (IS_WHITE(_buf._buffer[_state.cursor]) || _buf._buffer[_state.cursor] == '\0') {
          completeWord(i.keyword);
        }
        help = i.signature;
        break;
      }
    }
    free(selection);
  }
  return help;
}

void TextEditInput::draw(int x, int y, int w, int h, int chw) {
  SyntaxState syntax = kReset;
  StbTexteditRow r;
  int len = _buf._len;
  int i = 0;
  int baseY = 0;
  int cursorX = x;
  int cursorY = y;
  int cursorMatchX = x;
  int cursorMatchY = y;
  int row = 0;
  int line = 0;
  int selectStart = MIN(_state.select_start, _state.select_end);
  int selectEnd = MAX(_state.select_start, _state.select_end);

  maSetColor(_theme->_background);
  maFillRect(x, y, _width, _height);
  maSetColor(_theme->_color);

  while (i < len) {
    layout(&r, i);
    if (baseY + r.ymax > _height) {
      break;
    }

    if (i == 0 ||
        _buf._buffer[i - 1] == '\r' ||
        _buf._buffer[i - 1] == '\n') {
      syntax = kReset;
      line++;
    }

    if (row++ >= _scroll) {
      if (_matchingBrace != -1 && _matchingBrace >= i &&
          _matchingBrace < i + r.num_chars) {
        cursorMatchX = x + ((_matchingBrace - i) * chw);
        cursorMatchY = y + baseY;
      }

      if ((_state.cursor >= i && _state.cursor < i + r.num_chars) ||
          (i + r.num_chars == _buf._len && _state.cursor == _buf._len)) {
        // set cursor position
        if (_state.cursor == i + r.num_chars &&
            _buf._buffer[i + r.num_chars - 1] == STB_TEXTEDIT_NEWLINE) {
          // place cursor on newline
          cursorX = x;
          cursorY = y + baseY + _charHeight;
        } else {
          cursorX = x + ((_state.cursor - i) * chw);
          cursorY = y + baseY;
        }
        // the logical line, will be < _cursorRow when there are wrapped lines
        _cursorLine = line;

        if (_marginWidth > 0 && selectStart == selectEnd) {
          maSetColor(_theme->_row_cursor);
          maFillRect(x + _marginWidth, cursorY, _width, _charHeight);
          maSetColor(_theme->_color);
        }
      }

      int numChars = getLineChars(&r, i);
      if (selectStart != selectEnd && i + numChars > selectStart && i < selectEnd) {
        if (numChars) {
          // draw selected text
          int begin = selectStart - i;
          int baseX = _marginWidth;
          if (begin > 0) {
            // initial non-selected chars
            maSetColor(_theme->_color);
            maDrawText(x + baseX, y + baseY, _buf._buffer + i, begin);
            baseX += begin * _charWidth;
          } else if (begin < 0) {
            // started on previous row
            selectStart = i;
            begin = 0;
          }

          int count = selectEnd - selectStart;
          if (count > numChars - begin) {
            // fill to end of row
            count = numChars - begin;
            numChars = 0;
          }

          maSetColor(_theme->_selection_background);
          maFillRect(x + baseX, y + baseY, count * _charWidth, _charHeight);
          maSetColor(_theme->_selection_color);
          maDrawText(x + baseX, y + baseY, _buf._buffer + i + begin, count);

          int end = numChars - (begin + count);
          if (end) {
            // trailing non-selected chars
            baseX += count * _charWidth;
            maSetColor(_theme->_color);
            maDrawText(x + baseX, y + baseY, _buf._buffer + i + begin + count, end);
          }
        } else {
          // draw empty row selection
          maSetColor(_theme->_selection_background);
          maFillRect(x + _marginWidth, y + baseY, _charWidth / 2, _charHeight);
        }
        drawLineNumber(x, y + baseY, line, true);
      } else {
        drawLineNumber(x, y + baseY, line, false);
        if (numChars) {
          if (_theme->_plainText) {
            maSetColor(_theme->_color);
            maDrawText(x + _marginWidth, y + baseY, _buf._buffer + i, numChars);
          } else {
            drawText(x + _marginWidth, y + baseY, _buf._buffer + i, numChars, syntax);
          }
        }
      }
      baseY += _charHeight;
    } else if (row <= _scroll && syntax == kReset) {
      int end = i + r.num_chars - 1;
      if (_buf._buffer[end] != '\r' &&
          _buf._buffer[end] != '\n') {
        // scrolled line continues to next line
        for (int j = i; j < end; j++) {
          if (_comment && is_comment(_buf._buffer, j)) {
            syntax = kComment;
            break;
          } else if (_buf._buffer[j] == '\"') {
            syntax = (syntax == kText) ? kReset : kText;
          }
        }
      }
    }
    i += r.num_chars;
  }

  _bottom = i >= _buf._len;
  drawLineNumber(x, y + baseY, line + 1, false);

  // draw cursor
  maSetColor(_theme->_cursor_background);
  maFillRect(cursorX + _marginWidth, cursorY, chw, _charHeight);
  if (_state.cursor < _buf._len) {
    maSetColor(_theme->_cursor_color);
    if (_buf._buffer[_state.cursor] != '\r' &&
        _buf._buffer[_state.cursor] != '\n') {
      maDrawText(cursorX + _marginWidth, cursorY, _buf._buffer + _state.cursor, 1);
    }
  }
  if (_matchingBrace != -1) {
    maSetColor(_theme->_match_background);
    maFillRect(cursorMatchX + _marginWidth, cursorMatchY, chw, _charHeight);
    if (_matchingBrace < _buf._len) {
      maSetColor(_theme->_cursor_color);
      maDrawText(cursorMatchX + _marginWidth, cursorMatchY, _buf._buffer + _matchingBrace, 1);
    }
  }

  if (_showKeypad && g_keypad != nullptr) {
    g_keypad->draw();
  }
}

void TextEditInput::dragPage(int y, bool &redraw) {
  int size = abs(y - _ptY);
  int minSize = _charHeight / 4;
  if (_ptY == -1) {
    _ptY = y;
  } else if (size > minSize) {
    lineNavigate(y < _ptY);
    redraw = true;
    _ptY = y;
  }
}

void TextEditInput::drawText(int x, int y, const char *str, int length, SyntaxState &state) {
  int i = 0;
  int offs = 0;
  SyntaxState nextState = state;

  while (offs < length && i < length) {
    int count = 0;
    int next = 0;
    nextState = state;

    // find the end of the current segment
    while (i < length) {
      if (state == kComment || (_comment && is_comment(str, i))) {
        next = length - i;
        nextState = kComment;
        break;
      } else if (state == kText || str[i] == '\"') {
        next = 1;
        while (i + next < length && str[i + next] != '\"') {
          if (i + next + 1 < length &&
              str[i + next] == '\\' && str[i + next + 1] == '\"') {
            next++;
          }
          next++;
        }
        if (str[i + next] == '\"') {
          next++;
        }
        nextState = kText;
        break;
      } else if (state == kReset && isdigit(str[i]) &&
                 (i == 0 || !isalnum(str[i - 1]))) {
        next = 1;
        while (i + next < length && isdigit(str[i + next])) {
          next++;
        }
        if (!isalnum(str[i + next])) {
          if (i > 0 && str[i - 1] == '.') {
            i--;
            count--;
            next++;
          }
          nextState = kDigit;
          break;
        } else {
          i += next;
          count += next;
          next = 0;
        }
      } else if (state == kReset) {
        int size = 0;
        uint32_t hash = getHash(str, i, size);
        if (hash > 0) {
          if (matchCommand(hash)) {
            nextState = kCommand;
            next = size;
            break;
          } else if (matchStatement(hash)) {
            nextState = kStatement;
            next = size;
            break;
          } else if (size > 0) {
            i += size - 1;
            count += size - 1;
          }
        }
      }
      i++;
      count++;
    }

    // draw the current segment
    if (count > 0) {
      setColor(state);
      maDrawText(x, y, str + offs, count);
      offs += count;
      x += (count * _charWidth);
    }

    // draw the next segment
    if (next > 0) {
      setColor(nextState);
      maDrawText(x, y, str + offs, next);
      state = kReset;
      offs += next;
      x += (next * _charWidth);
      i += next;
    }
  }

  char cend = str[length];
  if (cend == '\r' || cend == '\n') {
    state = kReset;
  } else {
    state = nextState;
  }
}

bool TextEditInput::edit(int key, int screenWidth, int charWidth) {
  switch (key) {
  case SB_KEY_CTRL('a'):
    selectAll();
    break;
  case SB_KEY_ALT('c'):
    changeCase();
    break;
  case SB_KEY_ALT('d'):
    killWord();
    break;
  case SB_KEY_ALT('w'):
    selectWord();
    break;
  case SB_KEY_CTRL('d'):
    stb_textedit_key(&_buf, &_state, STB_TEXTEDIT_K_DELETE);
    break;
  case SB_KEY_CTRL('k'):
    editDeleteLine();
    break;
  case SB_KEY_ALT('n'):
    removeTrailingSpaces();
    break;
  case SB_KEY_ALT('t'):
    cycleTheme();
    break;
  case SB_KEY_TAB:
    editTab();
    break;
  case SB_KEY_CTRL('t'):
    toggleMarker();
    break;
  case SB_KEY_CTRL('g'):
    gotoNextMarker();
    break;
  case SB_KEY_SHIFT(SB_KEY_PGUP):
  case SB_KEY_PGUP:
    pageNavigate(false, key == (int)SB_KEY_SHIFT(SB_KEY_PGUP));
    return true;
  case SB_KEY_SHIFT(SB_KEY_PGDN):
  case SB_KEY_PGDN:
    pageNavigate(true, key == (int)SB_KEY_SHIFT(SB_KEY_PGDN));
    return true;
  case SB_KEY_CTRL(SB_KEY_UP):
    lineNavigate(false);
    return true;
  case SB_KEY_CTRL(SB_KEY_DN):
    lineNavigate(true);
    return true;
  case SB_KEY_ENTER:
    editEnter();
    break;
  case SB_KEY_SHIFT_CTRL(SB_KEY_LEFT):
    selectNavigate(true);
    break;
  case SB_KEY_SHIFT_CTRL(SB_KEY_RIGHT):
    selectNavigate(false);
    break;
  case SB_KEY_ALT(SB_KEY_LEFT):
  case SB_KEY_ALT(SB_KEY_RIGHT):
  case SB_KEY_ALT(SB_KEY_UP):
  case SB_KEY_ALT(SB_KEY_DOWN):
  case SB_KEY_ALT(SB_KEY_ESCAPE):
    // TODO: block move text selections
    return false;
    break;
  case -1:
    return false;
    break;
  default:
    stb_textedit_key(&_buf, &_state, key);
    break;
  }

  _cursorRow = getCursorRow();
  if (key == STB_TEXTEDIT_K_UP ||
      key == (int)SB_KEY_SHIFT(STB_TEXTEDIT_K_UP)) {
    if (_cursorRow == _scroll) {
      updateScroll();
    }
  } else {
    int pageRows = _height / _charHeight;
    if (_cursorRow - _scroll >= pageRows || _cursorRow < _scroll) {
      // scroll for cursor outside of current frame
      updateScroll();
    }
  }
  findMatchingBrace();
  return true;
}

bool TextEditInput::find(const char *word, bool next) {
  bool result = false;
  bool allUpper = true;
  int len = word == nullptr ? 0 : strlen(word);
  for (int i = 0; i < len; i++) {
    if (islower(word[i])) {
      allUpper = false;
      break;
    }
  }

  if (_buf._buffer != nullptr && word != nullptr) {
    const char *found = find_str(allUpper, _buf._buffer + _state.cursor, word);
    if (next && found != nullptr) {
      // skip to next word
      found = find_str(allUpper, found + strlen(word), word);
    }
    if (found == nullptr) {
      // start over
      found = find_str(allUpper, _buf._buffer, word);
    }
    if (found != nullptr) {
      result = true;
      _state.cursor = found - _buf._buffer;
      _state.select_start = _state.cursor;
      _state.select_end = _state.cursor + strlen(word);
      _cursorRow = getCursorRow();
      updateScroll();
    }
  }
  return result;
}

void TextEditInput::getSelectionCounts(int *lines, int *chars) {
  *lines = 1;
  *chars = 0;
  if (_state.select_start != _state.select_end) {
    int start = MIN(_state.select_start, _state.select_end);
    int end = MAX(_state.select_start, _state.select_end);
    int len = _buf._len;
    StbTexteditRow r;

    *chars = (end - start);
    for (int i = start; i < end && i < len; i += r.num_chars) {
      layout(&r, i);
      if (i + r.num_chars < end) {
        // found another row before selection end
        *lines += 1;
        *chars -= 1;
      } else if (i + r.num_chars == end) {
        // cursor at start of next line
        *chars -= 1;
      }
    }
  }
}

int TextEditInput::getSelectionRow() {
  int result;
  if (_state.select_start != _state.select_end) {
    int pos = MIN(_state.select_start, _state.select_end);
    int len = _buf._len;
    result = 0;
    StbTexteditRow r;
    for (int i = 0; i < len; i += r.num_chars) {
      layout(&r, i);
      if (pos >= i && pos < i + r.num_chars) {
        break;
      }
      result++;
    }
  } else {
    result = 0;
  }
  return result;
}

char *TextEditInput::getTextSelection(bool selectAll) {
  char *result;
  if (_state.select_start != _state.select_end) {
    int start, end;
    if (_state.select_start > _state.select_end) {
      end = _state.select_start;
      start = _state.select_end;
    } else {
      start = _state.select_start;
      end = _state.select_end;
    }
    result = _buf.textRange(start, end);
  } else if (selectAll) {
    result = _buf.textRange(0, _buf._len);
  } else {
    result = nullptr;
  }
  return result;
}

int *TextEditInput::getMarkers() {
  return g_lineMarker;
}

void TextEditInput::gotoLine(const char *buffer) {
  if (_buf._buffer != nullptr && buffer != nullptr) {
    setCursorRow(atoi(buffer) - 1);
  }
}

void TextEditInput::reload(const char *text) {
  _scroll = 0;
  _cursorRow = 0;
  _buf.clear();
  if (text != nullptr) {
    _buf.insertChars(0, text, strlen(text));
  }
  stb_textedit_initialize_state(&_state, false);
}

bool TextEditInput::save(const char *filePath) {
  bool result = true;
  FILE *fp = fopen(filePath, "wb");
  if (fp) {
    fwrite(_buf._buffer, sizeof(char), _buf._len, fp);
    fclose(fp);
    _dirty = false;
  } else {
    result = false;
  }
  return result;
}

void TextEditInput::selectAll() {
  _state.cursor = _state.select_start = 0;
  _state.select_end = _buf._len;
}

void TextEditInput::setCursor(int cursor) {
  _state.cursor = lineStart(cursor);
  _cursorRow = getCursorRow();
  _matchingBrace = -1;
  updateScroll();
}

void TextEditInput::setCursorPos(int pos) {
  _state.cursor = pos;
  _cursorRow = getCursorRow();
  _matchingBrace = -1;
  updateScroll();
}

void TextEditInput::setCursorRow(int row) {
  int len = _buf._len;
  StbTexteditRow r;
  for (int i = 0, nextRow = 0; i < len; i += r.num_chars, nextRow++) {
    layout(&r, i);
    if (row == nextRow) {
      _state.cursor = i;
      break;
    }
  }
  _cursorRow = row;
  _matchingBrace = -1;
  updateScroll();
}

void TextEditInput::clicked(int x, int y, bool pressed) {
  FormEditInput::clicked(x, y, pressed);
  if (x < _marginWidth) {
    _ptY = -1;
  } else if (pressed) {
    int tick = maGetMilliSecondCount();
    if (_pressTick && tick - _pressTick < DOUBLE_CLICK_MS) {
      _state.select_start = wordStart();
      _state.select_end = wordEnd();
    } else  {
      stb_textedit_click(&_buf, &_state, (x - _x) - _marginWidth, (y - _y) + (_scroll * _charHeight));
    }
    _pressTick = tick;
  }
}

void TextEditInput::updateField(var_p_t form) {
  var_p_t field = getField(form);
  if (field != nullptr) {
    var_p_t value = map_get(field, FORM_INPUT_VALUE);
    if (value != nullptr) {
      v_setstrn(value, _buf._buffer, _buf._len);
    }
  }
}

bool TextEditInput::updateUI(var_p_t form, var_p_t field) {
  bool updated = (form && field) ? FormInput::updateUI(form, field) : false;
  if (!_theme) {
    _theme = new EditTheme();
    updated = true;
  }
  return updated;
}

bool TextEditInput::selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) {
  bool result = hasFocus() && FormEditInput::selected(pt, scrollX, scrollY, redraw);
  if (result) {
    if (pt.x < _marginWidth) {
      dragPage(pt.y, redraw);
    } else {
      stb_textedit_drag(&_buf, &_state, (pt.x - _x) - _marginWidth,
                        (pt.y - _y) + scrollY + (_scroll * _charHeight));
      redraw = true;
    }
  }
  return result;
}

void TextEditInput::selectNavigate(bool up) {
  int start = _state.select_start == _state.select_end ? _state.cursor : _state.select_start;
  _state.select_start = _state.select_end = _state.cursor;
  stb_textedit_key(&_buf, &_state, up ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_WORDRIGHT);
  _state.select_start = start;
  _state.select_end = _state.cursor;
}

char *TextEditInput::copy(bool cut) {
  int selectStart = MIN(_state.select_start, _state.select_end);
  int selectEnd = MAX(_state.select_start, _state.select_end);
  char *result;
  if (selectEnd > selectStart) {
    result = _buf.textRange(selectStart, selectEnd);
    if (cut) {
      stb_textedit_cut(&_buf, &_state);
    }
    _state.select_start = _state.select_end;
  } else {
    result = nullptr;
  }
  return result;
}

void TextEditInput::paste(const char *text) {
  if (text != nullptr) {
    int lines = _buf._lines;
    stb_textedit_paste(&_buf, &_state, text, strlen(text));
    if (lines != _buf._lines) {
      _cursorRow = getCursorRow();
      updateScroll();
    }
  }
}

void TextEditInput::layout(StbTexteditRow *row, int start) const {
  int i = start;
  int len = _buf._len;
  int x1 = 0;
  int x2 = _width - _charWidth - _marginWidth;
  int numChars = 0;

  // advance to newline or rectangle edge
  while (i < len
         && x1 < x2
         && _buf._buffer[i] != '\r'
         && _buf._buffer[i] != '\n') {
    x1 += _charWidth;
    numChars++;
    i++;
  }

  row->num_chars = numChars;
  row->x1 = x1;

  if (_buf._buffer[i] == '\r') {
    // advance over DOS newline
    row->num_chars++;
    i++;
  }
  if (_buf._buffer[i] == '\n') {
    // advance over newline
    row->num_chars++;
  }
  row->x0 = 0.0f;
  row->ymin = 0.0f;
  row->ymax = row->baseline_y_delta = _charHeight;
}

void TextEditInput::layout(int w, int h) {
  if (_resizable) {
    if (_showKeypad && g_keypad != nullptr) {
      int keypadHeight = g_keypad->outerHeight(_charHeight);
      _width = w - (_x + _xmargin);
      _height = h - (_y + _ymargin + keypadHeight);
      g_keypad->layout(0, _height, w, keypadHeight);
    } else {
      _width = w - (_x + _xmargin);
      _height = h - (_y + _ymargin);
    }
  }
}

int TextEditInput::charWidth(int k, int i) const {
  int result = 0;
  if (k + i < _buf._len && _buf._buffer[k + i] != '\n') {
    result = _charWidth;
  }
  return result;
}

void TextEditInput::calcMargin() {
  MAExtent screenSize = maGetScrSize();
  _xmargin = EXTENT_X(screenSize) - (_x + _width);
  _ymargin = EXTENT_Y(screenSize) - (_y + _height);
}

void TextEditInput::changeCase() {
  int start, end;
  char *selection = getSelection(&start, &end);
  int len = strlen(selection);
  enum { up, down, mixed } curcase = isupper(selection[0]) ? up : down;

  for (int i = 1; i < len; i++) {
    if (isalpha(selection[i])) {
      bool isup = isupper(selection[i]);
      if ((curcase == up && isup == false) || (curcase == down && isup)) {
        curcase = mixed;
        break;
      }
    }
  }

  // transform pattern: Foo -> FOO, FOO -> foo, foo -> Foo
  for (int i = 0; i < len; i++) {
    selection[i] = curcase == mixed ? toupper(selection[i]) : tolower(selection[i]);
  }
  if (curcase == down) {
    selection[0] = toupper(selection[0]);
    // upcase chars following non-alpha chars
    for (int i = 1; i < len; i++) {
      if (isalpha(selection[i]) == false && i + 1 < len) {
        selection[i + 1] = toupper(selection[i + 1]);
      }
    }
  }
  if (selection[0]) {
    _state.select_start = start;
    _state.select_end = end;
    stb_textedit_paste(&_buf, &_state, selection, strlen(selection));
  }
  free(selection);
}

void TextEditInput::cycleTheme() {
  g_themeId = (g_themeId + 1) % NUM_THEMES;
  _theme->selectTheme(themes[g_themeId]);
}

void TextEditInput::drawLineNumber(int x, int y, int row, bool selected) {
  if (_marginWidth > 0) {
    bool markerRow = false;
    for (int i = 0; i < MAX_MARKERS && !markerRow; i++) {
      if (row == g_lineMarker[i]) {
        markerRow = true;
      }
    }
    if (markerRow) {
      maSetColor(_theme->_row_marker);
    } else if (selected) {
      maSetColor(_theme->_number_selection_background);
      maFillRect(x, y, _marginWidth, _charHeight);
      maSetColor(_theme->_number_selection_color);
    } else {
      maSetColor(_theme->_number_color);
    }
    int places = 0;
    for (int n = row; n > 0; n /= 10) {
      places++;
    }
    char rowBuffer[14];
    int offs = (_marginWidth - (_charWidth * places)) / 2;
    sprintf(rowBuffer, "%d", row);
    maDrawText(x + offs, y, rowBuffer, places);
  }
}

void TextEditInput::editDeleteLine() {
  int start = _state.cursor;
  int end = linePos(_state.cursor, true, true);
  if (end > start) {
    // delete the entire line when the cursor is at the home position
    stb_textedit_delete(&_buf, &_state, start, end - start + (_cursorCol == 0 ? 1 : 0));
    _state.cursor = start;
  } else if (start == end) {
    stb_textedit_delete(&_buf, &_state, start, 1);
  }
}

void TextEditInput::editEnter() {
  stb_textedit_key(&_buf, &_state, STB_TEXTEDIT_NEWLINE);
  int start = lineStart(_state.cursor);
  int prevLineStart = lineStart(start - 1);

  if (prevLineStart || _cursorLine == 1) {
    char spaces[LINE_BUFFER_SIZE];
    int indent = getIndent(spaces, LINE_BUFFER_SIZE, prevLineStart);

    // check whether the previous line was a comment
    char *buf = lineText(prevLineStart);
    int length = strlen(buf);
    int pos = 0;
    while (buf && (buf[pos] == ' ' || buf[pos] == '\t')) {
      pos++;
    }
    if (length > 2 && (buf[pos] == '#' || buf[pos] == '\'') && indent + 2 < LINE_BUFFER_SIZE) {
      spaces[indent] = buf[pos];
      spaces[++indent] = ' ';
      spaces[++indent] = '\0';
    } else if (length > 4 && strncasecmp(buf + pos, "rem", 3) == 0) {
      indent = strlcat(spaces, "rem ", LINE_BUFFER_SIZE);
    }
    free(buf);

    if (indent) {
      _buf.insertChars(_state.cursor, spaces, indent);
      stb_text_makeundo_insert(&_state, _state.cursor, indent);
      _state.cursor += indent;
    }
  }
}

void TextEditInput::editTab() {
  char spaces[LINE_BUFFER_SIZE];

  // get the desired indent based on the previous line
  int start = lineStart(_state.cursor);
  int prevLineStart = lineStart(start - 1);

  if (prevLineStart && prevLineStart + 1 == start) {
    // allows for a single blank line between statements
    prevLineStart = lineStart(prevLineStart - 1);
  }
  // note - spaces not used in this context
  int indent = (prevLineStart || _cursorLine == 2) ? getIndent(spaces, sizeof(spaces), prevLineStart) : 0;

  // get the current lines indent
  char *buf = lineText(start);
  int curIndent = 0;
  while (buf && (buf[curIndent] == ' ' || buf[curIndent] == '\t')) {
    curIndent++;
  }

  // adjust indent for statement terminators
  if (indent >= _indentLevel && buf && endStatement(buf + curIndent)) {
    indent -= _indentLevel;
  }
  if (curIndent < indent) {
    // insert additional spaces
    int len = indent - curIndent;
    if (len > (int)sizeof(spaces) - 1) {
      len = (int)sizeof(spaces) - 1;
    }
    memset(spaces, ' ', len);
    spaces[len] = 0;
    _buf.insertChars(start, spaces, len);
    stb_text_makeundo_insert(&_state, start, len);

    if (_state.cursor - start < indent) {
      // jump cursor to start of text
      _state.cursor = start + indent;
    } else {
      // move cursor along with text movement, staying on same line
      int maxpos = lineEnd(start);
      if (_state.cursor + len <= maxpos) {
        _state.cursor += len;
      }
    }
  } else if (curIndent > indent) {
    // remove excess spaces
    stb_textedit_delete(&_buf, &_state, start, curIndent - indent);
    _state.cursor = start + indent;
  } else if (start + indent > 0) {
    // already have ideal indent - soft-tab to indent
    _state.cursor = start + indent;
  }
  free(buf);
}

bool TextEditInput::endStatement(const char *buf) {
  static const struct Holder {
    const char *symbol;
    int len;
  } term[] = {
    {"wend", 4},
    {"fi", 2},
    {"endif", 5},
    {"elseif ", 7},
    {"elif ", 5},
    {"else", 4},
    {"next", 4},
    {"case", 4},
    {"end", 3},
    {"until ", 6}
  };
  static const int len = sizeof(term) / sizeof(Holder);
  bool result = false;
  for (int i = 0; i < len && !result; i++) {
    if (strncasecmp(buf, term[i].symbol, term[i].len) == 0) {
      char c = buf[term[i].len];
      if (c == '\0' || IS_WHITE(c)) {
        result = true;
      }
    }
  }
  return result;
}

void TextEditInput::findMatchingBrace() {
  char cursorChar = _state.cursor < _buf._len ? _buf._buffer[_state.cursor] : '\0';
  char cursorMatch = '\0';
  int pair = -1;
  int iter = -1;
  int pos;

  switch (cursorChar) {
  case ']':
    cursorMatch = '[';
    pos = _state.cursor - 1;
    break;
  case ')':
    cursorMatch = '(';
    pos = _state.cursor - 1;
    break;
  case '(':
    cursorMatch = ')';
    pos = _state.cursor + 1;
    iter = 1;
    break;
  case '[':
    cursorMatch = ']';
    iter = 1;
    pos = _state.cursor + 1;
    break;
  }
  if (cursorMatch != '\0') {
    // scan for matching opening on the same line
    int level = 1;
    int len = _buf._len;
    int gap = 0;
    while (pos > 0 && pos < len) {
      char nextChar = _buf._buffer[pos];
      if (nextChar == '\0' || nextChar == '\n') {
        break;
      }
      if (nextChar == cursorChar) {
        // nested char
        level++;
      } else if (nextChar == cursorMatch) {
        level--;
        if (level == 0) {
          // found matching char at pos
          if (gap > 0) {
            pair = pos;
          }
          break;
        }
      }
      pos += iter;
      gap++;
    }
  }
  _matchingBrace = pair;
}

int TextEditInput::getCompletions(StringList *list, int max) {
  int count = 0;
  char *selection = getWordBeforeCursor();
  unsigned len = selection != nullptr ? strlen(selection) : 0;
  if (len > 0) {
    for (int i = 0; i < keyword_help_len && count < max; i++) {
      if (strncasecmp(selection, keyword_help[i].keyword, len) == 0) {
        auto *s = new String();
        s->append(" ");
        s->append(keyword_help[i].keyword);
        list->add(s);
        count++;
      }
    }
  }
  free(selection);
  return count;
}

int TextEditInput::getCursorRow() {
  StbTexteditRow r;
  int len = _buf._len;
  int row = 0;
  int i;

  for (i = 0; i < len;) {
    layout(&r, i);
    if (_state.cursor == i + r.num_chars &&
        _buf._buffer[i + r.num_chars - 1] == STB_TEXTEDIT_NEWLINE) {
      // at end of line
      row++;
      _cursorCol = 0;
      break;
    } else if (_state.cursor >= i && _state.cursor < i + r.num_chars) {
      // within line
      _cursorCol = _state.cursor - i;
      break;
    }
    i += r.num_chars;
    if (i < len) {
      row++;
    }
  }
  return row;
}

uint32_t TextEditInput::getHash(const char *str, int offs, int &count) {
  uint32_t result = 0;
  if ((offs == 0 || IS_WHITE(str[offs - 1]) || ispunct(str[offs - 1]))
      && !IS_WHITE(str[offs]) && str[offs] != '\0') {
    for (count = 0; count < keyword_max_len; count++) {
      char ch = str[offs + count];
      if (ch == '.') {
        // could be SELF keyword
        break;
      } else if (!isalpha(ch) && ch != '_') {
        // non keyword character
        break;
      }
      result += tolower(ch);
      result += (result << 4);
      result ^= (result >> 2);
    }
  }
  return result;
}

int TextEditInput::getIndent(char *spaces, int len, int pos) {
  // count the indent level and find the start of text
  char *buf = lineText(pos);
  int i = 0;
  while (i < len && (buf[i] == ' ' || buf[i] == '\t')) {
    spaces[i] = buf[i];
    i++;
  }

  if (strncasecmp(buf + i, "while ", 6) == 0 ||
      strncasecmp(buf + i, "if ", 3) == 0 ||
      strncasecmp(buf + i, "elseif ", 7) == 0 ||
      strncasecmp(buf + i, "elif ", 5) == 0 ||
      strncasecmp(buf + i, "else", 4) == 0 ||
      strncasecmp(buf + i, "repeat", 6) == 0 ||
      strncasecmp(buf + i, "for ", 4) == 0 ||
      strncasecmp(buf + i, "select ", 7) == 0 ||
      strncasecmp(buf + i, "case ", 5) == 0 ||
      strncasecmp(buf + i, "sub ", 4) == 0 ||
      strncasecmp(buf + i, "func ", 5) == 0) {

    // handle if-then-blah on same line
    if (strncasecmp(buf + i, "if ", 3) == 0) {
      // find the end of line index
      int j = i + 4;
      while (buf[j] != 0 && buf[j] != '\n') {
        // line also 'ends' at start of comments
        if (_comment && is_comment(buf, j)) {
          break;
        }
        j++;
      }
      // right trim trailing spaces
      while ((buf[j - 1] == ' ' || buf[j - 1] == '\t') && j > i) {
        j--;
      }
      if (strncasecmp(buf + j - 4, "then", 4) != 0) {
        // 'then' is not final text on line
        spaces[i] = 0;
        free(buf);
        return i;
      }
    }
    // indent new line
    for (int j = 0; j < _indentLevel; j++, i++) {
      spaces[i] = ' ';
    }
  }
  spaces[i] = 0;
  free(buf);
  return i;
}

int TextEditInput::getLineChars(StbTexteditRow *row, int pos) const {
  int numChars = row->num_chars;
  if (numChars > 0 && _buf._buffer[pos + numChars - 1] == '\n') {
    numChars--;
  }
  if (numChars > 0 && _buf._buffer[pos + numChars - 1] == '\r') {
    numChars--;
  }
  return numChars;
}

char *TextEditInput::getSelection(int *start, int *end) {
  char *result;

  if (_state.select_start != _state.select_end) {
    result = _buf.textRange(_state.select_start,  _state.select_end);
    *start = _state.select_start;
    *end = _state.select_end;
  } else {
    *start = wordStart();
    *end = wordEnd();
    result = _buf.textRange(*start, *end);
  }
  return result;
}

const char *TextEditInput::getNodeId() {
  char *selection = getWordBeforeCursor();
  const char *result = nullptr;
  size_t len = selection != nullptr ? strlen(selection) : 0;
  if (len > 0) {
    for (int i = 0; i < keyword_help_len && !result; i++) {
      if (strcasecmp(selection, keyword_help[i].keyword) == 0) {
        result = keyword_help[i].nodeId;
      }
    }
  }
  free(selection);
  return result;
}

char *TextEditInput::getWordBeforeCursor() {
  char *result;
  if (_state.select_start == _state.select_end && _buf._len > 0) {
    int start, end;
    result = getSelection(&start, &end);
  } else {
    result = nullptr;
  }
  return result;
}

bool TextEditInput::replaceNext(const char *buffer, bool skip) {
  bool changed = false;
  if (_state.select_start != _state.select_end &&
      _buf._buffer != nullptr && buffer != nullptr) {
    int start, end;
    char *selection = getSelection(&start, &end);
    if (!skip) {
      stb_textedit_paste(&_buf, &_state, buffer, strlen(buffer));
    } else {
      _state.cursor++;
    }
    changed = find(selection, false);
    free(selection);
  }
  return changed;
}

void TextEditInput::gotoNextMarker() {
  int next = 0;
  int first = -1;
  for (int i = 0; i < MAX_MARKERS; i++) {
    if (g_lineMarker[i] != -1) {
      if (first == -1) {
        first = i;
      }
      if (g_lineMarker[i] == _cursorLine) {
        next = i + 1 == MAX_MARKERS ? first : i + 1;
        break;
      }
    }
  }
  if (first != -1) {
    if (g_lineMarker[next] == -1) {
      next = first;
    }
    if (g_lineMarker[next] != -1) {
      setCursorRow(g_lineMarker[next] - 1);
    }
  }
}

void TextEditInput::killWord() {
  int start = _state.cursor;
  int end = wordEnd();
  if (start == end) {
    int word = textedit_move_to_word_next(&_buf, _state.cursor);
    end = textedit_move_to_word_next(&_buf, word) - 1;
    int bound = lineEnd(start);
    if (end > bound && bound != start) {
      // clip to line end when there are characters prior to the line end
      end = bound;
    }
  }
  if (end > start) {
    stb_textedit_delete(&_buf, &_state, start, end - start);
  }
}

void TextEditInput::lineNavigate(bool arrowDown) {
  if (arrowDown) {
    if (!_bottom) {
      StbTexteditRow r;
      layout(&r, _state.cursor);
      _state.cursor += r.num_chars;
      _scroll += 1;
    }
  } else if (_scroll > 0) {
    int newLines = 0;
    int i = _state.cursor - 1;
    while (i > 0) {
      if (_buf._buffer[i] == '\n' && ++newLines == 2) {
        // scan to before the previous line, then add 1
        break;
      }
      i--;
    }
    _state.cursor = i == 0 ? 0 : i + 1;
    _scroll -= 1;
  }
}

char *TextEditInput::lineText(int pos) {
  StbTexteditRow r;
  int len = _buf._len;
  int start = 0;
  int end = 0;
  for (int i = 0; i < len; i += r.num_chars) {
    layout(&r, i);
    if (pos >= i && pos < i + r.num_chars) {
      start = i;
      end = i + getLineChars(&r, i);
      break;
    }
  }
  return _buf.textRange(start, end);
}

int TextEditInput::linePos(int pos, bool end, bool excludeBreak) {
  StbTexteditRow r;
  int len = _buf._len;
  int start = 0;
  for (int i = 0; i < len; i += r.num_chars) {
    layout(&r, i);
    if (pos >= i && pos < i + r.num_chars) {
      if (end) {
        if (excludeBreak) {
          start = i + getLineChars(&r, i);
        } else {
          start = i + r.num_chars;
        }
      } else {
        start = i;
      }
      break;
    }
  }
  return start;
}

bool TextEditInput::matchCommand(uint32_t hash) {
  bool result = false;
  for (int i = 0; i < keyword_hash_command_len && !result; i++) {
    if (keyword_hash_command[i] == hash) {
      result = true;
    }
  }
  return result;
}

bool TextEditInput::matchStatement(uint32_t hash) {
  bool result = false;
  for (int i = 0; i < keyword_hash_statement_len && !result; i++) {
    if (keyword_hash_statement[i] == hash) {
      result = true;
    }
  }
  return result;
}

void TextEditInput::pageNavigate(bool pageDown, bool shift) {
  int pageRows = (_height / _charHeight) + 1;
  int nextRow = _cursorRow + (pageDown ? pageRows : -pageRows);
  if (nextRow < 0) {
    nextRow = 0;
  }

  StbTexteditRow r;
  int len = _buf._len;
  int row = 0;
  int i = 0;
  int count = 0;

  for (; i < len && row != nextRow; i += r.num_chars, row++) {
    layout(&r, i);
    count += r.num_chars;
  }

  if (count == _buf._len) {
    // at end
    row--;
    i -= r.num_chars;
  }

  if (shift) {
    if (_state.select_start == _state.select_end) {
      _state.select_start = _state.cursor;
    }
    _state.select_end = i;
  } else {
    _state.select_start = _state.select_end;
  }

  _state.cursor = i;
  _cursorRow = row;
  _cursorCol = 0;
  updateScroll();
}

void TextEditInput::removeTrailingSpaces() {
  int row = getCursorRow();
  _buf.removeTrailingSpaces(&_state);
  setCursorRow(row - 1);
}

void TextEditInput::selectWord() {
  if (_state.select_start != _state.select_end) {
    // advance to next word
    _state.cursor = textedit_move_to_word_next(&_buf, _state.cursor);
    _state.select_start = _state.select_end = -1;
  }
  _state.select_start = wordStart();
  _state.select_end = _state.cursor = wordEnd();

  if (_state.select_start == _state.select_end) {
    // move to next word
    _state.cursor = textedit_move_to_word_next(&_buf, _state.cursor);
  }
}

void TextEditInput::setColor(SyntaxState &state) {
  switch (state) {
  case kComment:
    maSetColor(_theme->_syntax_comments);
    break;
  case kText:
    maSetColor(_theme->_syntax_text);
    break;
  case kCommand:
    maSetColor(_theme->_syntax_command);
    break;
  case kStatement:
    maSetColor(_theme->_syntax_statement);
    break;
  case kDigit:
    maSetColor(_theme->_syntax_digit);
    break;
  case kReset:
    maSetColor(_theme->_color);
    break;
  }
}

void TextEditInput::toggleMarker() {
  bool found = false;
  for (int i = 0; i < MAX_MARKERS && !found; i++) {
    if (_cursorLine == g_lineMarker[i]) {
      g_lineMarker[i] = -1;
      found = true;
    }
  }
  if (!found) {
    for (int i = 0; i < MAX_MARKERS && !found; i++) {
      if (g_lineMarker[i] == -1) {
        g_lineMarker[i] = _cursorLine;
        found = true;
        break;
      }
    }
  }
  if (!found) {
    g_lineMarker[0] = _cursorLine;
  }
  qsort(g_lineMarker, MAX_MARKERS, sizeof(int), compareIntegers);
}

void TextEditInput::updateScroll() {
  int pageRows = _height / _charHeight;
  if (_cursorRow + 1 < pageRows) {
    _scroll = 0;
  } else if (_cursorRow >= _scroll + pageRows || _cursorRow <= _scroll) {
    // cursor outside current view
    _scroll = _cursorRow - (pageRows / 2);
  }
}

int TextEditInput::wordEnd() {
  int i = _state.cursor;
  while (i >= 0 && i < _buf._len && IS_VAR_CHAR(_buf._buffer[i])) {
    i++;
  }
  return i;
}

int TextEditInput::wordStart() {
  int cursor = _state.cursor == 0 ? 0 : _state.cursor - 1;
  return ((cursor >= 0 && cursor < _buf._len && _buf._buffer[cursor] == '\n') ? _state.cursor :
          is_word_border(&_buf, _state.cursor) ? _state.cursor :
          textedit_move_to_word_previous(&_buf, _state.cursor));
}

int TextEditInput::showKeypad() {
  if (!g_keypad) {
    g_keypad = new Keypad(_charWidth, _charHeight);
  }
  _showKeypad = true;
  layout(_width, _height);
  return g_keypad->outerHeight(_charHeight);
}

//
// KeywordIterator
//
template<typename KeywordIteratorFunc>
void keywordIterator(KeywordIteratorFunc &&callback) {
  const char *package = keyword_help[0].package;
  int packageIndex = 0;

  for (auto i = 0; i < keyword_help_len; i++) {
    if (strcasecmp(package, keyword_help[i].package) != 0) {
      // start of next package
      package = keyword_help[i].package;
      packageIndex++;
      if (!callback(i, packageIndex, true)) {
        break;
      }
    }
    else if (!callback(i, packageIndex, i == 0)) {
      break;
    }
  }
}

//
// TextEditHelpWidget
//
TextEditHelpWidget::TextEditHelpWidget(TextEditInput *editor, int chW, int chH, bool overlay) :
  TextEditInput(nullptr, chW, chH, editor->_x, editor->_y, editor->_width, editor->_height),
  _mode(kNone),
  _editor(editor),
  _keywordIndex(-1),
  _packageIndex(0),
  _packageOpen(false),
  _layout(kPopup) {
  _theme = new EditTheme(HELP_FG, HELP_BG);
  _comment = false;
  hide();
  if (overlay) {
    _x = editor->_width - (chW * HELP_WIDTH);
    _width = chW * HELP_WIDTH;
  }
}

TextEditHelpWidget::~TextEditHelpWidget() {
  _outline.clear();
}

bool TextEditHelpWidget::closeOnEnter() const {
  return (_mode != kSearch && _mode != kHelpKeyword);
}

bool TextEditHelpWidget::edit(int key, int screenWidth, int charWidth) {
  bool result = false;
  switch (_mode) {
  case kSearch:
    result = TextEditInput::edit(key, screenWidth, charWidth);
    _editor->find(_buf._buffer, key == SB_KEY_ENTER);
    break;
  case kEnterReplace:
    result = TextEditInput::edit(key, screenWidth, charWidth);
    if (key == SB_KEY_ENTER) {
      _buf.clear();
      _mode = kEnterReplaceWith;
    } else {
      _editor->find(_buf._buffer, false);
    }
    break;
  case kEnterReplaceWith:
    if (key == SB_KEY_ENTER) {
      _mode = kReplace;
    } else {
      result = TextEditInput::edit(key, screenWidth, charWidth);
    }
    break;
  case kReplace:
    switch (key) {
    case SB_KEY_ENTER:
      if (!_editor->replaceNext(_buf._buffer, false)) {
        _mode = kReplaceDone;
      }
      break;
    case ' ':
      if (!_editor->replaceNext(_buf._buffer, true)) {
        // skip to next
        _mode = kReplaceDone;
      }
      break;
    }
    break;
  case kGotoLine:
    result = TextEditInput::edit(key, screenWidth, charWidth);
    if (key == SB_KEY_ENTER) {
      _editor->gotoLine(_buf._buffer);
    }
    break;
  case kLineEdit:
    if (key != SB_KEY_ENTER) {
      result = TextEditInput::edit(key, screenWidth, charWidth);
    }
    break;
  case kMessage:
    //  readonly mode
    break;
  default:
    switch (key) {
    case STB_TEXTEDIT_K_LEFT:
    case STB_TEXTEDIT_K_RIGHT:
    case STB_TEXTEDIT_K_UP:
    case STB_TEXTEDIT_K_DOWN:
    case STB_TEXTEDIT_K_PGUP:
    case STB_TEXTEDIT_K_PGDOWN:
    case STB_TEXTEDIT_K_LINESTART:
    case STB_TEXTEDIT_K_LINEEND:
    case STB_TEXTEDIT_K_TEXTSTART:
    case STB_TEXTEDIT_K_TEXTEND:
    case STB_TEXTEDIT_K_WORDLEFT:
    case STB_TEXTEDIT_K_WORDRIGHT:
      result = TextEditInput::edit(key, screenWidth, charWidth);
      if (_mode == kOutline && _cursorRow < _outline.size()) {
        int cursor = (intptr_t)_outline[_cursorRow];
        _editor->setCursor(cursor);
      } else if (_mode == kStacktrace && _cursorRow < _outline.size()) {
        int cursorRow = (intptr_t)_outline[_cursorRow];
        _editor->setCursorRow(cursorRow - 1);
      }
      break;
    case SB_KEY_ENTER:
      switch (_mode) {
      case kCompletion:
        completeWord(_state.cursor);
        break;
      case kHelpKeyword:
        toggleKeyword();
        break;
      default:
        break;
      }
      result = true;
      break;
    default:
      if (_mode == kHelpKeyword && _keywordIndex != -1 && key < 0) {
        result = TextEditInput::edit(key, screenWidth, charWidth);
      }
      break;
    }
  }
  return result;
}

void TextEditHelpWidget::completeLine(int pos) {
  int end = pos;
  while (end < _buf._len && _buf._buffer[end] != '\n') {
    end++;
  }
  char *text = _buf.textRange(pos, end);
  if (text[0] != '\0' && text[0] != '[') {
    _editor->completeWord(text);
  }
  free(text);
}

void TextEditHelpWidget::completeWord(int pos) {
  char *text = lineText(pos);
  if (text[0] != '\0' && text[0] != '[') {
    _editor->completeWord(text);
  }
  free(text);
}

void TextEditHelpWidget::clicked(int x, int y, bool pressed) {
  _ptY = -1;
  if (pressed) {
    stb_textedit_click(&_buf, &_state, 0, (y - _y) + (_scroll * _charHeight));
    if (_mode == kHelpKeyword) {
      toggleKeyword();
    }
  }
}

void TextEditHelpWidget::createCompletionHelp() {
  reset(kCompletion);

  char *selection = _editor->getWordBeforeCursor();
  int len = selection != nullptr ? strlen(selection) : 0;
  if (len > 0) {
    StringList words;
    for (auto & i : keyword_help) {
      if (strncasecmp(selection, i.keyword, len) == 0) {
        words.add(i.keyword);
        _buf.append(i.keyword);
        _buf.append("\n", 1);
      }
    }
    const char *text = _editor->getText();
    const char *found = strcasestr(text, selection);
    while (found != nullptr) {
      const char *end = found;
      const char pre = found > text ? *(found - 1) : ' ';
      while (IS_VAR_CHAR(*end) && *end != '\0') {
        end++;
      }
      if (end - found > len && (IS_WHITE(pre) || pre == '.')) {
        String next;
        next.append(found, end - found);
        if (!words.contains(next)) {
          words.add(next);
          _buf.append(found, end - found);
          _buf.append("\n", 1);
        }
      }
      found = strcasestr(end, selection);
    }
  } else {
    const char *package = nullptr;
    for (auto & i : keyword_help) {
      if (package == nullptr || strcasecmp(package, i.package) != 0) {
        // next package
        package = i.package;
        _buf.append("[");
        _buf.append(package);
        _buf.append("]\n");
      }
      _buf.append(i.keyword);
      _buf.append("\n", 1);
    }
  }
  free(selection);
}

void TextEditHelpWidget::createGotoLine() {
  reset(kGotoLine);
}

void TextEditHelpWidget::createHelp() {
  reset(kHelp);
  _buf.append(helpText, strlen(helpText));
}

void TextEditHelpWidget::createLineEdit(const char *value) {
  reset(kLineEdit);
  if (value && value[0]) {
    _buf.append(value, strlen(value));
  }
}

void TextEditHelpWidget::createKeywordIndex() {
  char *keyword = _editor->getWordBeforeCursor();
  reset(kHelpKeyword);

  if (keyword != nullptr) {
    keywordIterator([=](int index, int packageIndex, bool) {
      bool result = true;
      if (strcasecmp(keyword, keyword_help[index].keyword) == 0) {
        // found keyword at cursor
        _packageIndex = packageIndex;
        _keywordIndex = index;
        _packageOpen = false;
        result = false;
      }
      return result;
    });
    free(keyword);
  }

  buildKeywordIndex();
}

void TextEditHelpWidget::createOutline() {
  const char *text = _editor->getText();
  int len = _editor->getTextLength();
  const char *keywords[] = {
    "sub ", "func ", "def ", "label ", "const ", "local ", "dim "
  };
  int keywords_length = sizeof(keywords) / sizeof(keywords[0]);
  int keywords_len[keywords_length];
  for (int j = 0; j < keywords_length; j++) {
    keywords_len[j] = strlen(keywords[j]);
  }

  reset(kOutline);

  int cursorPos = _editor->getCursorPos();

  for (int i = 0; i < len; i++) {
    // skip to the newline start
    while (i < len && i != 0 && text[i] != '\n') {
      i++;
    }

    // skip any successive newlines
    while (i < len && text[i] == '\n') {
      i++;
    }

    // skip any leading whitespace
    while (i < len && (text[i] == ' ' || text[i] == '\t')) {
      i++;
    }

    int iNext = i;

    for (int j = 0; j < keywords_length; j++) {
      if (!strncasecmp(text + i, keywords[j], keywords_len[j])) {
        i += keywords_len[j];
        int iBegin = i;
        while (i < len && text[i] != '=' && text[i] != '\r' && text[i] != '\n') {
          i++;
        }
        if (i > iBegin) {
          int numChars = i - iBegin;
          int padding = j > 1 ? 4 : 2;
          if (numChars > HELP_WIDTH - padding) {
            numChars = HELP_WIDTH - padding;
          }
          if (numChars > 0) {
            if (j > 1) {
              _buf.append(" .", 2);
            }
            _buf.append(text + iBegin, numChars);
            _buf.append("\n", 1);
            _outline.add((int *)(intptr_t)i);
          }
        }
        break;
      }
    }

    if (iNext < i && cursorPos < i && !_state.cursor) {
      _state.cursor = _buf._len - 1;
    }

    if (text[i] == '\n') {
      // avoid eating the entire next line
      i--;
    }
  }

  _state.cursor = lineStart(_state.cursor);
  _cursorRow = getCursorRow();
  updateScroll();
}

void TextEditHelpWidget::createSearch(bool replace) {
  if (_mode != kSearch) {
    reset(replace ? kEnterReplace : kSearch);
  }

  char *text = _editor->getTextSelection(false);
  if (text != nullptr) {
    // prime search from selected text
    _buf.clear();
    _buf.insertChars(0, text, strlen(text));
    free(text);

    // ensure the selected word is first match
    _editor->setCursorPos(_editor->getSelectionStart());
  }
}

void TextEditHelpWidget::createStackTrace(const char *error, int line, StackTrace &trace) {
  reset(kStacktrace);

  _outline.add((int *)(intptr_t)line);
  _buf.append("Error:\n");

  List_each(StackTraceNode *, it, trace) {
    StackTraceNode *node = (*it);
    _outline.add((int *)(intptr_t)node->_line);
    _buf.append(" ", 1);
    _buf.append(node->_keyword);
    _buf.append("\n", 1);
  }

  _buf.append("\n", 1);
  _buf.append(error);
  _buf.append("\n", 1);
  _outline.add((int *)(intptr_t)line);
  _outline.add((int *)(intptr_t)line);
}

void TextEditHelpWidget::paste(const char *text) {
  switch (_mode) {
  case kSearch:
  case kEnterReplace:
  case kEnterReplaceWith:
  case kLineEdit:
    TextEditInput::paste(text);
    break;
  default:
    break;
  }
}

void TextEditHelpWidget::reset(HelpMode mode) {
  stb_textedit_clear_state(&_state, mode == kSearch);
  _outline.clear();
  _mode = mode;
  _buf.clear();
  _scroll = 0;
  _matchingBrace = -1;
}

bool TextEditHelpWidget::selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) {
  bool result = hasFocus();
  if (result) {
    dragPage(pt.y, redraw);
  }
  return result;
}

void TextEditHelpWidget::toggleKeyword() {
  auto *line = lineText(_state.cursor);
  auto level1 = (strstr(line, LEVEL1_OPEN) != nullptr ||
                 strstr(line, LEVEL1_CLOSE) != nullptr);
  auto level2Open = (strstr(line, LEVEL2_OPEN) != nullptr);
  auto level2Close = (strstr(line, LEVEL2_CLOSE) != nullptr);

  int keywordIndex = _keywordIndex;
  int packageIndex = _packageIndex;
  bool packageOpen = _packageOpen;

  keywordIterator([=](int index, int packageIndex, bool nextPackage) {
    bool result = true;
    if (nextPackage) {
      const char *package = keyword_help[index].package;
      if (level1 && strcasecmp(line + LEVEL1_OFFSET, package) == 0) {
        _packageOpen = !_packageOpen;
        _packageIndex = packageIndex;
        _keywordIndex = -1;
        result = false;
      }
    }
    if (level2Open && strcasecmp(line + LEVEL2_LEN, keyword_help[index].keyword) == 0) {
      _keywordIndex = index;
      _packageOpen = false;
      result = false;
    }
    else if (level2Close && strcasecmp(line + LEVEL2_CLOSE_LEN, keyword_help[index].keyword) == 0) {
      _keywordIndex = -1;
      _packageOpen = true;
      result = false;
    }
    return result;
  });

  free(line);

  if (keywordIndex != _keywordIndex ||
      packageIndex != _packageIndex ||
      packageOpen != _packageOpen) {
    buildKeywordIndex();
  }
}

void TextEditHelpWidget::buildKeywordIndex() {
  _buf.clear();
  _buf.append("SmallBASIC language reference\n\n");
  _buf.append("+ Select a category\n|\n");

  keywordIterator([=](int index, int packageIndex, bool nextPackage) {
    if (nextPackage) {
      const char *package = keyword_help[index].package;
      if (packageIndex < _packageIndex) {
        _buf.append("| ", 2);
      } else if (packageIndex == _packageIndex) {
        _buf.append("|+", 2);
        _state.cursor = _buf._len + 1;
      } else {
        _buf.append("  ", 2);
      }
      if (_packageOpen && _packageIndex == packageIndex && _keywordIndex == -1) {
        _buf.append(LEVEL1_CLOSE, LEVEL1_LEN);
      } else {
        _buf.append(LEVEL1_OPEN, LEVEL1_LEN);
      }
      _buf.append(package);
      _buf.append("\n", 1);

      if (_packageOpen && _packageIndex == packageIndex) {
        _buf.append("   |\n", 5);
      }
    }

    // next keyword
    if (_packageOpen && _packageIndex == packageIndex) {
      _buf.append(LEVEL2_OPEN, LEVEL2_LEN);
      _buf.append(keyword_help[index].keyword);
      _buf.append("\n", 1);
    }

    return true;
  });

  if (_keywordIndex != -1) {
    _buf.append("\n", 1);
    _state.cursor = _buf._len + 3;
    _buf.append(LEVEL2_CLOSE, LEVEL2_CLOSE_LEN);
    _buf.append(keyword_help[_keywordIndex].keyword);
    _buf.append("\n\n", 2);
    _buf.append(keyword_help[_keywordIndex].signature);
    _buf.append("\n\n", 2);
    _buf.append(keyword_help[_keywordIndex].help);
    _buf.append("\n\n", 2);
  }

  _cursorRow = getCursorRow();
}

void TextEditHelpWidget::showPopup(int cols, int rows) {
  if (cols < 0) {
    _width = _editor->_width - (_charWidth * -cols);
  } else {
    _width = _charWidth * cols;
  }
  if (rows < 0) {
    _height = _editor->_height - (_charHeight * -rows);
  } else {
    _height = _charHeight * rows;
  }
  if (_width > _editor->_width) {
    _width = _editor->_width;
  }
  if (_height > _editor->_height) {
    _height = _editor->_height;
  }
  _x = (_editor->_width - _width) / 2;
  if (rows == 1) {
    _layout = kLine;
    _y = _editor->_height - (_charHeight * 2.5);
  } else {
    _layout = kPopup;
    _y = (_editor->_height - _height) / 2;
  }
  _theme->contrast(_editor->getTheme());
  calcMargin();
  show();
}

void TextEditHelpWidget::showSidebar() {
  int border = _charWidth * 2;
  _width = _charWidth * SIDE_BAR_WIDTH;
  _height = _editor->_height - (border * 2);
  _x = _editor->_width - (_width + border);
  _y = border;
  _theme->contrast(_editor->getTheme());
  _layout = kSidebar;
  calcMargin();
  show();
}

void TextEditHelpWidget::draw(int x, int y, int w, int h, int chw) {
  TextEditInput::draw(x, y, w, h, chw);
  int shadowW = _charWidth / 3;
  int shadowH = _charWidth / 3;

  maSetColor(_theme->_selection_background);
  maFillRect(x + _width, y + shadowH, shadowW, _height);
  maFillRect(x + shadowW, y + _height, _width, shadowH);
}

void TextEditHelpWidget::layout(int w, int h) {
  if (_resizable) {
    int border;
    switch (_layout) {
    case kLine:
      _x = (w - _width) / 2;
      _y = h - (_charHeight * 2.5);
      break;
    case kSidebar:
      border = _charWidth * 2;
      _height = h - (border * 2);
      _x = w - (_width + border);
      break;
    case kPopup:
      _width = w - (_x + _xmargin);
      _height = h - (_y + _ymargin);
    }
  }
}
