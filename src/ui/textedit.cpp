// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <stdlib.h>
#include <string.h>

#include "ui/textedit.h"
#include "ui/inputs.h"
#include "ui/utils.h"
#include "ui/strlib.h"
#include "ui/kwp.h"

void safe_memmove(void *dest, const void *src, size_t n) {
  if (n > 0 && dest != NULL && src != NULL) {
    memmove(dest, src, n);
  }
}

#define STB_TEXTEDIT_IS_SPACE(ch) IS_WHITE(ch)
#define STB_TEXTEDIT_IS_PUNCT(ch) (ch != '_' && ch != '$' && ispunct(ch))
#define IS_VAR_CHAR(ch) (ch == '_' || ch == '$' || isalpha(ch) || isdigit(ch))
#define STB_TEXTEDIT_memmove safe_memmove
#define STB_TEXTEDIT_IMPLEMENTATION
#include "lib/stb_textedit.h"

#define GROW_SIZE 128
#define LINE_BUFFER_SIZE 200
#define INDENT_LEVEL 2
#define HELP_WIDTH 22
#define NUM_THEMES 5
#define TWISTY1_OPEN  "> "
#define TWISTY1_CLOSE "< "
#define TWISTY2_OPEN  "  > "
#define TWISTY2_CLOSE "  < "
#define TWISTY1_LEN 2
#define TWISTY2_LEN 4
#define HELP_BG 0x73c990
#define HELP_FG 0x20242a
#define DOUBLE_CLICK_MS 200

#if defined(_Win32)
#include <shlwapi.h>
#define strcasestr StrStrI
#endif

extern "C" dword dev_get_millisecond_count();

unsigned g_themeId = 0;
int g_lineMarker[MAX_MARKERS] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

const int theme1[] = {
  0xc8cedb, 0xa7aebc, 0x484f5f, 0xa7aebc, 0xa7aebc, 0x00bb00,
  0x272b33, 0x3d4350, 0x2b3039, 0x3875ed, 0x373b88, 0x2b313a,
  0x0083f8, 0xff9d00, 0x31ccac, 0xc679dd, 0x0083f8
};

const int theme2[] = {
  0xcccccc, 0x000077, 0x333333, 0x333333, 0x0000aa, 0x008888,
  0x010101, 0xeeeeee, 0x010101, 0xffff00, 0x00ff00, 0x010101,
  0x00ffff, 0xff00ff, 0xffffff, 0x00ffff, 0x00aaff
};

const int theme3[] = {
  0xc8cedb, 0xd7decc, 0x484f5f, 0xa7aebc, 0xa7aebc, 0x00bb00,
  0x001b33, 0x0088ff, 0x000d1a, 0x0051b1, 0x373b88, 0x022444,
  0x0083f8, 0xff9d00, 0x31ccac, 0xc679dd, 0x0083f8
};

const int theme4[] = {
  0x4f4a44, 0x222228, 0x77839b, 0x484f5f, 0xa7aebc, 0x5f9e59,
  0xcdc0b0, 0xe1e1e1, 0xefeff0, 0x1f51eb, 0x000000, 0xcbb8a2,
  0x4c9f9a, 0xaf5fd6, 0x0000ff, 0xc679dd, 0x0083f8, 0
};

int g_user_theme[] = {
  0xc8cedb, 0xa7aebc, 0x484f5f, 0xa7aebc, 0xa7aebc, 0x00bb00,
  0x2e3436, 0x888a85, 0x000000, 0x4d483b, 0x000000, 0x2b313a,
  0x0083f8, 0xff9d00, 0x31ccac, 0xc679dd, 0x0083f8
};

const int* themes[] = {
  theme1, theme2, theme3, theme4, g_user_theme
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
  "A-g goto line\n"
  "A-n trim line-endings\n"
  "A-t select theme\n"
  "A-. break mode\n"
  "A-<n> recent file\n"
  "SHIFT-<arrow> select\n"
  "TAB indent line\n"
  "F1,A-h keyword help\n"
  "F2 online help\n"
  "F3,F4 export\n"
  "F5 debug\n"
  "F9, C-r run\n"
  "F10, set command$\n";

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

//
// EditTheme
//
EditTheme::EditTheme() {
  if (g_themeId >= (sizeof(themes) / sizeof(themes[0]))) {
    g_themeId = 0;
  }
  selectTheme(themes[g_themeId]);
}

EditTheme::EditTheme(int fg, int bg) :
  _color(fg),
  _background(bg),
  _selection_color(bg),
  _selection_background(fg),
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

//
// EditBuffer
//
EditBuffer::EditBuffer(TextEditInput *in, const char *text) :
  _buffer(NULL),
  _len(0),
  _size(0),
  _in(in) {
  if (text != NULL && text[0]) {
    _len = strlen(text);
    _size = _len + 1;
    _buffer = (char *)malloc(_size);
    memcpy(_buffer, text, _len);
    _buffer[_len] = '\0';
  }
}

EditBuffer::~EditBuffer() {
  clear();
}

void EditBuffer::clear() {
  free(_buffer);
  _buffer = NULL;
  _len = _size = 0;
}

int EditBuffer::deleteChars(int pos, int num) {
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

int EditBuffer::insertChars(int pos, const char *text, int num) {
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
  return 1;
}

char *EditBuffer::textRange(int start, int end) {
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
  _theme(NULL),
  _charWidth(chW),
  _charHeight(chH),
  _marginWidth(0),
  _scroll(0),
  _cursorRow(0),
  _cursorLine(0),
  _indentLevel(INDENT_LEVEL),
  _matchingBrace(-1),
  _ptY(-1),
  _pressTick(0),
  _dirty(false) {
  stb_textedit_initialize_state(&_state, false);
}

TextEditInput::~TextEditInput() {
  delete _theme;
  _theme = NULL;
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
  const char *help = NULL;
  char *selection = getWordBeforeCursor();
  if (selection != NULL) {
    int len = strlen(selection);
    int count = 0;
    for (int i = 0; i < keyword_help_len; i++) {
      if (strncasecmp(selection, keyword_help[i].keyword, len) == 0 &&
          count++ == index) {
        if (IS_WHITE(_buf._buffer[_state.cursor]) || _buf._buffer[_state.cursor] == '\0') {
          completeWord(keyword_help[i].keyword);
        }
        help = keyword_help[i].signature;
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
          if (_marginWidth > 0) {
            drawText(x + _marginWidth, y + baseY, _buf._buffer + i, numChars, syntax);
          } else {
            maSetColor(_theme->_color);
            maDrawText(x + _marginWidth, y + baseY, _buf._buffer + i, numChars);
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
          if (is_comment(_buf._buffer, j)) {
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

void TextEditInput::drawText(int x, int y, const char *str,
                             int length, SyntaxState &state) {
  int i = 0;
  int offs = 0;
  SyntaxState nextState = state;

  while (offs < length && i < length) {
    int count = 0;
    int next = 0;
    nextState = state;

    // find the end of the current segment
    while (i < length) {
      if (state == kComment || is_comment(str, i)) {
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
    if (_cursorRow - _scroll > pageRows || _cursorRow < _scroll) {
      // scroll for cursor outside of current frame
      updateScroll();
    }
  }
  findMatchingBrace();
  return true;
}

bool TextEditInput::find(const char *word, bool next) {
  bool result = false;
  if (_buf._buffer != NULL && word != NULL) {
    const char *found = strcasestr(_buf._buffer + _state.cursor, word);
    if (next && found != NULL) {
      // skip to next word
      found = strcasestr(found + strlen(word), word);
    }
    if (found == NULL) {
      // start over
      found = strcasestr(_buf._buffer, word);
    }
    if (found != NULL) {
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

char *TextEditInput::getTextSelection() {
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
  } else {
    result = _buf.textRange(0, _buf._len);
  }
  return result;
}

int *TextEditInput::getMarkers() {
  return g_lineMarker;
}

void TextEditInput::gotoLine(const char *buffer) {
  if (_buf._buffer != NULL && buffer != NULL) {
    setCursorRow(atoi(buffer) - 1);
  }
}

void TextEditInput::reload(const char *text) {
  _scroll = 0;
  _cursorRow = 0;
  _buf.clear();
  if (text != NULL) {
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
  if (x < _marginWidth) {
    _ptY = -1;
  } else if (pressed) {
    int tick = dev_get_millisecond_count();
    if (_pressTick && tick - _pressTick < DOUBLE_CLICK_MS) {
      _state.select_start = wordStart();
      _state.select_end = wordEnd();
    } else  {
      stb_textedit_click(&_buf, &_state, x - _marginWidth, y + (_scroll * _charHeight));
    }
    _pressTick = tick;
  }
}

void TextEditInput::updateField(var_p_t form) {
  var_p_t field = getField(form);
  if (field != NULL) {
    var_p_t value = map_get(field, FORM_INPUT_VALUE);
    v_setstrn(value, _buf._buffer, _buf._len);
  }
}

bool TextEditInput::updateUI(var_p_t form, var_p_t field) {
  bool updated = (form && field) ? FormInput::updateUI(form, field) : false;
  if (!_theme) {
    if (_fg == DEFAULT_FOREGROUND && _bg == DEFAULT_BACKGROUND) {
      _theme = new EditTheme();
    } else {
      _theme = new EditTheme(_fg, _bg);
    }
    updated = true;
  }
  return updated;
}

bool TextEditInput::selected(MAPoint2d pt, int scrollX, int scrollY, bool &redraw) {
  bool focus = hasFocus();
  if (focus) {
    if (pt.x < _marginWidth) {
      dragPage(pt.y, redraw);
    } else {
      stb_textedit_drag(&_buf, &_state, pt.x - _marginWidth,
                        pt.y + scrollY + (_scroll * _charHeight));
      redraw = true;
    }
  }
  return focus;
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
    result = NULL;
  }
  return result;
}

void TextEditInput::paste(const char *text) {
  if (text != NULL) {
    stb_textedit_paste(&_buf, &_state, text, strlen(text));
    _cursorRow = getCursorRow();
    updateScroll();
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

int TextEditInput::charWidth(int k, int i) const {
  int result = 0;
  if (k + i < _buf._len && _buf._buffer[k + i] != '\n') {
    result = _charWidth;
  }
  return result;
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
    char rowBuffer[places + 1];
    int offs = (_marginWidth - (_charWidth * places)) / 2;

    sprintf(rowBuffer, "%d", row);
    maDrawText(x + offs, y, rowBuffer, places);
  }
}

void TextEditInput::editDeleteLine() {
  int start = _state.cursor;
  int end = linePos(_state.cursor, true, true);
  if (end > start) {
    stb_textedit_delete(&_buf, &_state, start, end - start);
    _state.cursor = start;
  } else if (start == end) {
    stb_textedit_delete(&_buf, &_state, start, 1);
  }
}

void TextEditInput::editEnter() {
  stb_textedit_key(&_buf, &_state, STB_TEXTEDIT_NEWLINE);
  char spaces[LINE_BUFFER_SIZE];
  int start = lineStart(_state.cursor);
  int prevLineStart = lineStart(start - 1);

  if (prevLineStart || _cursorLine == 1) {
    int indent = getIndent(spaces, sizeof(spaces), prevLineStart);
    if (indent) {
      _buf.insertChars(_state.cursor, spaces, indent);
      stb_text_makeundo_insert(&_state, _state.cursor, indent);
      _state.cursor += indent;
    }
  }
}

void TextEditInput::editTab() {
  char spaces[LINE_BUFFER_SIZE];
  int indent;

  // get the desired indent based on the previous line
  int start = lineStart(_state.cursor);
  int prevLineStart = lineStart(start - 1);

  if (prevLineStart && prevLineStart + 1 == start) {
    // allows for a single blank line between statements
    prevLineStart = lineStart(prevLineStart - 1);
  }
  // note - spaces not used in this context
  indent = (prevLineStart || _cursorLine == 2) ? getIndent(spaces, sizeof(spaces), prevLineStart) : 0;

  // get the current lines indent
  char *buf = lineText(start);
  int curIndent = 0;
  while (buf && (buf[curIndent] == ' ' || buf[curIndent] == '\t')) {
    curIndent++;
  }

  // adjust indent for statement terminators
  if (indent >= _indentLevel && endStatement(buf + curIndent)) {
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
  unsigned len = selection != NULL ? strlen(selection) : 0;
  if (len > 0) {
    for (int i = 0; i < keyword_help_len && count < max; i++) {
      if (strncasecmp(selection, keyword_help[i].keyword, len) == 0) {
        list->add(keyword_help[i].keyword);
        count++;
      }
    }
  }
  free(selection);
  return count;
}

int TextEditInput::getCursorRow() const {
  StbTexteditRow r;
  int len = _buf._len;
  int row = 0;

  for (int i = 0; i < len;) {
    layout(&r, i);
    if (_state.cursor == i + r.num_chars &&
        _buf._buffer[i + r.num_chars - 1] == STB_TEXTEDIT_NEWLINE) {
      // at end of line
      row++;
      break;
    } else if (_state.cursor >= i && _state.cursor < i + r.num_chars) {
      // within line
      break;
    }
    i += r.num_chars;
    if (i < len) {
      row++;
    }
  }
  return row + 1;
}

uint32_t TextEditInput::getHash(const char *str, int offs, int &count) {
  uint32_t result = 0;
  if ((offs == 0 || IS_WHITE(str[offs - 1]) || ispunct(str[offs - 1]))
       && !IS_WHITE(str[offs]) && str[offs] != '\0') {
    for (count = 0; count < keyword_max_len; count++) {
      char ch = str[offs + count];
      if (!isalpha(ch) && ch != '_') {
        // non keyword character
        while (isalnum(ch) || ch == '.' || ch == '_') {
          // skip any program variable characters
          count++;
          ch = str[offs + count];
        }
        break;
      }
      result += tolower(str[offs + count]);
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
  while (buf && (buf[i] == ' ' || buf[i] == '\t') && i < len) {
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
        if (is_comment(buf, j)) {
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

int TextEditInput::getLineChars(StbTexteditRow *row, int pos) {
  int numChars = row->num_chars;
  if (numChars > 0 && _buf._buffer[pos + row->num_chars - 1] == '\r') {
    numChars--;
  }
  if (numChars > 0 && _buf._buffer[pos + row->num_chars - 1] == '\n') {
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
  const char *result = NULL;
  int len = selection != NULL ? strlen(selection) : 0;
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
    result = NULL;
  }
  return result;
}

bool TextEditInput::replaceNext(const char *buffer) {
  bool changed = false;
  if (_state.select_start != _state.select_end &&
      _buf._buffer != NULL && buffer != NULL) {
    int start, end;
    char *selection = getSelection(&start, &end);
    stb_textedit_paste(&_buf, &_state, buffer, strlen(buffer));
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

void TextEditInput::lineNavigate(bool arrowDown) {
  if (arrowDown) {
    // starting from the cursor position (relative to the screen),
    // count the number of rows to the bottom of the document.
    int rowCount = _cursorLine - _scroll;
    for (int i = _state.cursor; i < _buf._len; i++) {
      if (_buf._buffer[i] == '\n') {
        rowCount++;
      }
    }
    int pageRows = (_height / _charHeight) - 1;
    if (rowCount >= pageRows) {
      // rows exist below end of page to pull up
      for (int i = _state.cursor; i < _buf._len; i++) {
        if (_buf._buffer[i] == '\n' && i + 1 < _buf._len) {
          _state.cursor = i + 1;
          _scroll += 1;
          break;
        }
      }
    }
  } else if (_scroll > 0) {
    int newLines = 0;
    int i = _state.cursor - 1;
    while (i > 0) {
      if (_buf._buffer[i] == '\n' && ++newLines == 2) {
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

  for (; i < len && row != nextRow; i += r.num_chars, row++) {
    layout(&r, i);
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
  updateScroll();
}

void TextEditInput::removeTrailingSpaces() {
  int row = getCursorRow();
  _buf.removeTrailingSpaces(&_state);
  setCursorRow(row - 1);
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
  while (IS_VAR_CHAR(_buf._buffer[i]) && i < _buf._len) {
    i++;
  }
  return i;
}

int TextEditInput::wordStart() {
  int cursor = _state.cursor == 0 ? 0 : _state.cursor - 1;
  return (_buf._buffer[cursor] == '\n' ? _state.cursor :
          is_word_boundary(&_buf, _state.cursor) ? _state.cursor :
          stb_textedit_move_to_word_previous(&_buf, &_state));
}

//
// TextEditHelpWidget
//
TextEditHelpWidget::TextEditHelpWidget(TextEditInput *editor, int chW, int chH, bool overlay) :
  TextEditInput(NULL, chW, chH, editor->_x, editor->_y, editor->_width, editor->_height),
  _mode(kNone),
  _editor(editor),
  _openPackage(NULL),
  _openKeyword(-1) {
  _theme = new EditTheme(HELP_BG, HELP_FG);
  hide();
  if (overlay) {
    _x = editor->_width - (chW * HELP_WIDTH);
    _width = chW * HELP_WIDTH;
  }
}

TextEditHelpWidget::~TextEditHelpWidget() {
  _outline.emptyList();
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
  case kSearchReplace:
    result = TextEditInput::edit(key, screenWidth, charWidth);
    if (key == SB_KEY_ENTER) {
      _buf.clear();
      _mode = kReplace;
    } else {
      _editor->find(_buf._buffer, false);
    }
    break;
  case kReplace:
    if (key == SB_KEY_ENTER) {
      if (!_editor->replaceNext(_buf._buffer)) {
        _mode = kReplaceDone;
      }
    } else {
      result = TextEditInput::edit(key, screenWidth, charWidth);
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
      if (_mode == kOutline && _outline.size()) {
        int cursor = (intptr_t)_outline[_cursorRow - 1];
        _editor->setCursor(cursor);
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
      if (_mode == kHelpKeyword && _openKeyword != -1 && key < 0) {
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
    stb_textedit_click(&_buf, &_state, 0, y + (_scroll * _charHeight));
    if (_mode == kHelpKeyword && x - _x <= _charWidth * 3) {
      toggleKeyword();
    }
  }
}

void TextEditHelpWidget::createCompletionHelp() {
  reset(kCompletion);

  char *selection = _editor->getWordBeforeCursor();
  int len = selection != NULL ? strlen(selection) : 0;
  if (len > 0) {
    StringList words;
    for (int i = 0; i < keyword_help_len; i++) {
      if (strncasecmp(selection, keyword_help[i].keyword, len) == 0) {
        words.add(keyword_help[i].keyword);
        _buf.append(keyword_help[i].keyword);
        _buf.append("\n", 1);
      }
    }
    const char *text = _editor->getText();
    const char *found = strcasestr(text, selection);
    while (found != NULL) {
      const char *end = found;
      const char pre = found > text ? *(found - 1) : ' ';
      while (IS_VAR_CHAR(*end) && *end != '\0') {
        end++;
      }
      if (end - found > len && (IS_WHITE(pre) || pre == '.')) {
        String next;
        next.append(found, end - found);
        if (!words.exists(next)) {
          words.add(next);
          _buf.append(found, end - found);
          _buf.append("\n", 1);
        }
      }
      found = strcasestr(end, selection);
    }
  } else {
    const char *package = NULL;
    for (int i = 0; i < keyword_help_len; i++) {
      if (package == NULL || strcasecmp(package, keyword_help[i].package) != 0) {
        // next package
        package = keyword_help[i].package;
        _buf.append("[");
        _buf.append(package);
        _buf.append("]\n");
      }
      _buf.append(keyword_help[i].keyword);
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

  bool keywordFound = false;
  if (keyword != NULL) {
    for (int i = 0; i < keyword_help_len && !keywordFound; i++) {
      if (strcasecmp(keyword, keyword_help[i].keyword) == 0) {
        _buf.append(TWISTY2_OPEN, TWISTY2_LEN);
        _buf.append(keyword_help[i].keyword);
        _openPackage = keyword_help[i].package;
        keywordFound = true;
        toggleKeyword();
        break;
      }
    }
    free(keyword);
  }

  if (!keywordFound) {
    const char *package = NULL;
    for (int i = 0; i < keyword_help_len; i++) {
      if (package == NULL || strcasecmp(package, keyword_help[i].package) != 0) {
        package = keyword_help[i].package;
        _buf.append(TWISTY1_OPEN, TWISTY1_LEN);
        _buf.append(package);
        _buf.append("\n", 1);
      }
    }
  }
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
  if (_mode == kSearch) {
    _editor->find(_buf._buffer, true);
  } else {
    reset(replace ? kSearchReplace : kSearch);
  }
}

void TextEditHelpWidget::paste(const char *text) {
  switch (_mode) {
  case kSearch:
  case kSearchReplace:
  case kReplace:
  case kLineEdit:
    TextEditInput::paste(text);
    break;
  default:
    break;
  }
}

void TextEditHelpWidget::reset(HelpMode mode) {
  stb_textedit_clear_state(&_state, mode == kSearch);
  _outline.emptyList();
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
  char *line = lineText(_state.cursor);
  bool open1 = strncmp(line, TWISTY1_OPEN, TWISTY1_LEN) == 0;
  bool open2 = strncmp(line, TWISTY2_OPEN, TWISTY2_LEN) == 0;
  bool close1 = strncmp(line, TWISTY1_CLOSE, TWISTY1_LEN) == 0;
  bool close2 = strncmp(line, TWISTY2_CLOSE, TWISTY2_LEN) == 0;
  if (open1 || open2 || close1 || close2) {
    const char *nextLine = line + TWISTY1_LEN;
    const char *package = (open2 || close2) && _openPackage != NULL ? _openPackage : nextLine;
    const char *nextPackage = NULL;
    int pageRows = _height / _charHeight;
    int open1Count = 0;
    int open2Count = 0;
    _buf.clear();
    _matchingBrace = -1;
    _openKeyword = -1;
    _state.select_start = _state.select_end = 0;

    for (int i = 0; i < keyword_help_len; i++) {
      if (nextPackage == NULL || strcasecmp(nextPackage, keyword_help[i].package) != 0) {
        nextPackage = keyword_help[i].package;
        if (strcasecmp(package, nextPackage) == 0) {
          // selected item
          if (open1 || close1) {
            _state.cursor = _buf._len;
            _cursorRow = open1Count;
          }

          _buf.append(open1 || open2 || close2 ? TWISTY1_CLOSE : TWISTY1_OPEN, TWISTY1_LEN);
          _buf.append(nextPackage);
          _buf.append("\n", 1);

          if (open1) {
            _openPackage = nextPackage;
            open1Count++;
          } else if (open2) {
            nextLine = line + TWISTY2_LEN;
            open2Count++;
          }
          if (open1 || open2 || close2) {
            while (i < keyword_help_len &&
                   strcasecmp(nextPackage, keyword_help[i].package) == 0) {
              open2Count++;
              if (open2 && strcasecmp(nextLine, keyword_help[i].keyword) == 0) {
                _openKeyword = i;
                _state.cursor = _buf._len;
                _cursorRow = open1Count + open2Count;
                _buf.append(TWISTY2_CLOSE, TWISTY2_LEN);
                _buf.append(keyword_help[i].keyword);
                _buf.append("\n\n", 2);
                _buf.append(keyword_help[i].signature);
                _buf.append("\n\n", 2);
                _buf.append(keyword_help[i].help);
                _buf.append("\n\n", 2);
              } else {
                _buf.append(TWISTY2_OPEN, TWISTY2_LEN);
                _buf.append(keyword_help[i].keyword);
                _buf.append("\n", 1);
              }
              i++;
            }
          }
        } else {
          // next package item (level 1)
          _buf.append(TWISTY1_OPEN, TWISTY1_LEN);
          _buf.append(nextPackage);
          _buf.append("\n", 1);
          open1Count++;
        }
      }
    }
    if (_cursorRow + 4 < pageRows) {
      _scroll = 0;
    } else {
      _scroll = _cursorRow - (pageRows / 4);
    }
  }
  free(line);
}
