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

#define STB_TEXTEDIT_IS_SPACE(ch) IS_WHITE(ch)
#define STB_TEXTEDIT_IMPLEMENTATION
#include "lib/stb_textedit.h"

#define GROW_SIZE 128
#define LINE_BUFFER_SIZE 200
#define INDENT_LEVEL 2
#define THEME_FOREGROUND 0xa7aebc
#define THEME_BACKGROUND 0x272b33
#define THEME_SELECTION_BACKGROUND 0x3d4350
#define THEME_NUMBER_SELECTION_BACKGROUND 0x2b3039
#define THEME_NUMBER_SELECTION 0xabb2c0
#define THEME_NUMBER 0x484f5f
#define THEME_CURSOR 0xa7aebc
#define THEME_CURSOR_BACKGROUND 0x3875ed
#define THEME_MATCH_BACKGROUND 0x373b88
#define HELP_WIDTH 22

const char *helpText =
  "C-a select-all\n"
  "C-b back\n"
  "C-k delete line\n"
  "C-d delete char\n"
  "C-r run\n"
  "C-s save\n"
  "C-x cut\n"
  "C-c copy\n"
  "C-v paste\n"
  "C-z undo\n"
  "C-y redo\n"
  "C-o outline\n"
  "C-j keywords\n"
  "A-a home\n"
  "A-d end\n"
  "A-c change case\n"
  "A-s start\n"
  "A-r end\n"
  "F9  run\n"
  "ESC close help";

//
// EditTheme
//
EditTheme::EditTheme() :
  _color(THEME_FOREGROUND),
  _background(THEME_BACKGROUND),
  _selection_color(THEME_FOREGROUND),
  _selection_background(THEME_SELECTION_BACKGROUND),
  _number_color(THEME_NUMBER),
  _number_selection_color(THEME_FOREGROUND),
  _number_selection_background(THEME_NUMBER_SELECTION_BACKGROUND),
  _cursor_color(THEME_CURSOR),
  _cursor_background(THEME_CURSOR_BACKGROUND),
  _match_background(THEME_MATCH_BACKGROUND) {
}

EditTheme::EditTheme(int fg, int bg) :
  _color(fg),
  _background(bg),
  _selection_color(bg),
  _selection_background(fg),
  _cursor_color(bg),
  _cursor_background(fg),
  _match_background(fg) {
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
  delete _buffer;
  _buffer = NULL;
  _len = _size = 0;
}

int EditBuffer::deleteChars(int pos, int num) {
  memmove(&_buffer[pos], &_buffer[pos+num], _len - (pos + num));
  _len -= num;
  _in->setDirty(true);
  return 1;
}

int EditBuffer::insertChars(int pos, const char *text, int num) {
  int required = _len + num + 1;
  if (required >= _size) {
    _size += (required + GROW_SIZE);
    _buffer = (char *)realloc(_buffer, _size);
  }
  memmove(&_buffer[pos + num], &_buffer[pos], _len - pos);
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

void EditBuffer::replaceChars(const char *replace, int start, int end) {
  for (int i = start, j = 0; i < end && i < _len && replace[j] != '\0'; i++, j++) {
    _buffer[i] = replace[j];
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
  _indentLevel(INDENT_LEVEL),
  _matchingBrace(-1),
  _dirty(false) {
  stb_textedit_initialize_state(&_state, false);
}

void TextEditInput::draw(int x, int y, int w, int h, int chw) {
  StbTexteditRow r;
  int len = _buf._len;
  int i = 0;
  int baseY = 0;
  int cursorX = x;
  int cursorY = y;
  int cursorMatchX = x;
  int cursorMatchY = y;
  int row = 0;
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

    if (row++ >= _scroll) {
      int numChars = getLineChars(&r, i);
      if (selectStart != selectEnd && i + numChars > selectStart && i < selectEnd) {
        if (numChars) {
          // draw selected text
          int begin = selectStart - i;
          int baseX = _marginWidth;
          if (begin > 0) {
            // initial non-selected chars
            maDrawText(x, y + baseY, _buf._buffer + i, begin);
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
          maSetColor(_theme->_color);

          int end = numChars - (begin + count);
          if (end) {
            // trailing non-selected chars
            baseX += count * _charWidth;
            maDrawText(x + baseX, y + baseY, _buf._buffer + i + begin + count, end);
          }
        } else {
          // draw empty row selection
          maSetColor(_theme->_selection_background);
          maFillRect(x + _marginWidth, y + baseY, _charWidth / 2, _charHeight);
          maSetColor(_theme->_color);
        }
      } else if (numChars) {
        maDrawText(x + _marginWidth, y + baseY, _buf._buffer + i, numChars);
      }
      if (_matchingBrace != -1 && _matchingBrace >= i && _matchingBrace < i + r.num_chars) {
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
      }
      baseY += _charHeight;
    }
    i += r.num_chars;
  }

  // draw cursor
  maSetColor(_theme->_cursor_background);
  maFillRect(cursorX + _marginWidth, cursorY, chw, _charHeight);
  if (_state.cursor < _buf._len) {
    maSetColor(_theme->_cursor_color);
    maDrawText(cursorX + _marginWidth, cursorY, _buf._buffer + _state.cursor, 1);
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

bool TextEditInput::edit(int key, int screenWidth, int charWidth) {
  switch (key) {
  case SB_KEY_CTRL('a'):
    _state.cursor = 0;
    _state.select_start = 0;
    _state.select_end = _buf._len;
    break;
  case SB_KEY_ALT('a'):
    stb_textedit_key(&_buf, &_state, STB_TEXTEDIT_K_LINESTART);
    break;
  case SB_KEY_ALT('c'):
    changeCase();
    break;
  case SB_KEY_CTRL('d'):
    stb_textedit_key(&_buf, &_state, STB_TEXTEDIT_K_DELETE);
    break;
  case SB_KEY_ALT('e'):
    stb_textedit_key(&_buf, &_state, STB_TEXTEDIT_K_LINEEND);
    break;
  case SB_KEY_CTRL('k'):
    editDeleteLine();
    break;
  case SB_KEY_TAB:
    editTab();
    break;
  case SB_KEY_SHIFT(SB_KEY_PGUP):
  case SB_KEY_PGUP:
    editNavigate(false, key == (int)SB_KEY_SHIFT(SB_KEY_PGUP));
    return true;
  case SB_KEY_SHIFT(SB_KEY_PGDN):
  case SB_KEY_PGDN:
    editNavigate(true, key == (int)SB_KEY_SHIFT(SB_KEY_PGDN));
    return true;
  case SB_KEY_ENTER:
    editEnter();
    break;
  case -1:
    return false;
    break;
  default:
    stb_textedit_key(&_buf, &_state, key);
    break;
  }
  _cursorRow = getCursorRow();
  updateScroll();
  findMatchingBrace();
  return true;
}

bool TextEditInput::save(const char *filePath) {
  bool result = true;
  FILE *fp = fopen(filePath, "w");
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
  _state.select_start = 0;
  _state.select_end = _buf._len;
}

void TextEditInput::setCursor(int cursor) {
  _state.cursor = lineStart(cursor);
  _cursorRow = getCursorRow();
  _matchingBrace = -1;
  updateScroll();
}

void TextEditInput::clicked(int x, int y, bool pressed) {
  if (pressed) {
    stb_textedit_click(&_buf, &_state, x, y);
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
  stb_textedit_drag(&_buf, &_state, pt.x, pt.y);
  redraw = true;
  return 1;
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

void TextEditInput::paste(char *text) {
  stb_textedit_paste(&_buf, &_state, text, strlen(text));
}

void TextEditInput::layout(StbTexteditRow *row, int start) const {
  int i = start;
  int len = _buf._len;
  int x2 = _width - _charWidth - _marginWidth;
  row->x1 = 0;
  row->num_chars = 0;

  // advance to newline or rectangle edge
  while (i < len
         && (int)row->x1 < x2
         && _buf._buffer[i] != '\r'
         && _buf._buffer[i] != '\n') {
    row->x1 += _charWidth;
    row->num_chars++;
    i++;
  }

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
    _buf.replaceChars(selection, start, end);
    _state.select_start = start;
    _state.select_end = end;
  }
  free((void *)selection);
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
  if (prevLineStart) {
    int indent = getIndent(spaces, sizeof(spaces), prevLineStart);
    if (indent) {
      _buf.insertChars(_state.cursor, spaces, indent);
      stb_text_makeundo_insert(&_state, _state.cursor, indent);
      _state.cursor += indent;
    }
  }
}

void TextEditInput::editNavigate(bool pageDown, bool shift) {
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
  indent = prevLineStart == 0 ? 0 : getIndent(spaces, sizeof(spaces), prevLineStart);

  // get the current lines indent
  char *buf = lineText(start);
  int curIndent = 0;
  while (buf && buf[curIndent] == ' ') {
    curIndent++;
  }

  // adjust indent for statement terminators
  if (strncasecmp(buf + curIndent, "wend", 4) == 0 ||
      strncasecmp(buf + curIndent, "fi", 2) == 0 ||
      strncasecmp(buf + curIndent, "endif", 5) == 0 ||
      strncasecmp(buf + curIndent, "elseif ", 7) == 0 ||
      strncasecmp(buf + curIndent, "elif ", 5) == 0 ||
      strncasecmp(buf + curIndent, "else", 4) == 0 ||
      strncasecmp(buf + curIndent, "next", 4) == 0 ||
      strncasecmp(buf + curIndent, "case", 4) == 0 ||
      strncasecmp(buf + curIndent, "end", 3) == 0 ||
      strncasecmp(buf + curIndent, "until ", 6) == 0) {
    if (indent >= _indentLevel) {
      indent -= _indentLevel;
    }
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
  } else {
    // already have ideal indent - soft-tab to indent
    _state.cursor = start + indent;
  }
  free((void *)buf);
}

void TextEditInput::findMatchingBrace() {
  char cursorChar = _buf._buffer[_state.cursor];
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

int TextEditInput::getIndent(char *spaces, int len, int pos) {
  // count the indent level and find the start of text
  char *buf = lineText(pos);
  int i = 0;
  while (buf && buf[i] == ' ' && i < len) {
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
        if (strncasecmp(buf + j, "rem", 3) == 0 || buf[j] == '\'') {
          break;
        }
        j++;
      }
      // right trim trailing spaces
      while (buf[j - 1] == ' ' && j > i) {
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
  free((void *)buf);
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
    *start = is_word_boundary(&_buf, _state.cursor) ? _state.cursor :
             stb_textedit_move_to_word_previous(&_buf, &_state);
    int i = _state.cursor;
    while (!IS_WHITE(_buf._buffer[i]) && i < _buf._len) {
      i++;
    }
    *end = i;
    result = _buf.textRange(*start, *end);
  }
  return result;
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

void TextEditInput::updateScroll() {
  int pageRows = _height / _charHeight;
  if (_cursorRow + 1 < pageRows) {
    _scroll = 0;
  } else if (_cursorRow > _scroll + pageRows || _cursorRow <= _scroll) {
    // cursor outside current view
    _scroll = _cursorRow - (pageRows / 2);
  }
}

//
// TextEditHelpWidget
//
TextEditHelpWidget::TextEditHelpWidget(TextEditInput *editor, int chW, int chH) :
  TextEditInput(NULL, chW, chH, editor->_width - (chW * HELP_WIDTH), editor->_y,
                chW * HELP_WIDTH, editor->_height),
  _editor(editor) {
  _theme = new EditTheme(_fg, _bg);
  hide();
}

TextEditHelpWidget::~TextEditHelpWidget() {
  _outline.emptyList();
}

bool TextEditHelpWidget::edit(int key, int screenWidth, int charWidth) {
  bool result;

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
    if (_outline.size()) {
      int cursor = (intptr_t)_outline[_cursorRow - 1];
      _editor->setCursor(cursor);
    }
    break;
  default:
    result = false;
    break;
  }
  return result;
}

void TextEditHelpWidget::createHelp() {
  reset();
  _buf.append(helpText, strlen(helpText));
}

void TextEditHelpWidget::createKeywordHelp() {
  reset();
  _buf.append("[Keywords]\n");
  for (int i = 0; i < code_keywords_length; i++) {
    _buf.append(code_keywords[i]);
    _buf.append("\n", 1);
  }
  _buf.append("\n[Procedures]\n");
  for (int i = 0; i < code_procedures_length; i++) {
    _buf.append(code_procedures[i]);
    _buf.append("\n", 1);
  }
  _buf.append("\n[Functions]\n");
  for (int i = 0; i < code_functions_length; i++) {
    _buf.append(code_functions[i]);
    _buf.append("\n", 1);
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

  reset();

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

    for (int j = 0; j < keywords_length; j++) {
      if (!strncasecmp(text + i, keywords[j], keywords_len[j])) {
        i += keywords_len[j];
        int i_begin = i;
        while (i < len && text[i] != '=' && text[i] != '\r' && text[i] != '\n') {
          i++;
        }
        if (i > i_begin) {
          int numChars = i - i_begin;
          int padding = j > 1 ? 4 : 2;
          if (numChars > HELP_WIDTH - padding) {
            numChars = HELP_WIDTH - padding;
          }
          if (numChars > 0) {
            if (j > 1) {
              _buf.append(" .", 2);
            }
            _buf.append(text + i_begin, numChars);
            _buf.append("\n", 1);
            _outline.add((int *)(intptr_t)i);
          }
        }
        break;
      }
    }
    if (text[i] == '\n') {
      // avoid eating the entire next line
      i--;
    }
  }
}

void TextEditHelpWidget::reset() {
  stb_textedit_clear_state(&_state, false);
  _outline.emptyList();
  _buf.clear();
  _scroll = 0;
  _matchingBrace = -1;
}
