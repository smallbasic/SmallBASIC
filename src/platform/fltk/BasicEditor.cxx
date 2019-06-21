// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>
#include <stdint.h>
#include <FL/Fl_Rect.H>
#include "platform/fltk/BasicEditor.h"
#include "platform/fltk/kwp.h"
#include "platform/fltk/Profile.h"

using namespace strlib;

Fl_Color defaultColor[] = {
  FL_BLACK,                            // A - Plain
  fl_rgb_color(0, 128, 0),             // B - Comments
  fl_rgb_color(0, 0, 192),             // C - Strings
  fl_rgb_color(192, 0, 0),             // D - code_keywords
  fl_rgb_color(128, 128, 0),           // E - code_functions
  fl_rgb_color(0, 128, 128),           // F - code_procedures
  fl_rgb_color(128, 0, 128),           // G - Find matches
  fl_rgb_color(0, 128, 0),             // H - Italic Comments '
  fl_rgb_color(0, 128, 128),           // I - Numbers
  fl_rgb_color(128, 128, 64),          // J - Operators
};

Fl_Text_Display::Style_Table_Entry styletable[] = {
  { defaultColor[0], FL_COURIER, 12},        // A - Plain
  { defaultColor[1], FL_COURIER, 12},        // B - Comments
  { defaultColor[2], FL_COURIER, 12},        // C - Strings
  { defaultColor[3], FL_COURIER, 12},        // D - code_keywords
  { defaultColor[4], FL_COURIER, 12},        // E - code_functions
  { defaultColor[5], FL_COURIER, 12},        // F - code_procedures
  { defaultColor[6], FL_COURIER, 12},        // G - Find matches
  { defaultColor[7], FL_COURIER_ITALIC, 12}, // H - Italic Comments
  { defaultColor[8], FL_COURIER, 12},        // I - Numbers
  { defaultColor[9], FL_COURIER, 12},        // J - Operators
  { FL_WHITE,        FL_COURIER, 12},        // K - Background
};

#define PLAIN      'A'
#define COMMENTS   'B'
#define STRINGS    'C'
#define KEYWORDS   'D'
#define FUNCTIONS  'E'
#define PROCEDURES 'F'
#define FINDMATCH  'G'
#define ITCOMMENTS 'H'
#define DIGITS     'I'
#define OPERATORS  'J'

/**
 * return whether the character is a valid variable symbol
 */
bool isvar(int c) {
  return (isalnum(c) || c == '_');
}

/**
 * Compare two keywords
 */
int compare_keywords(const void *a, const void *b) {
  return (strcasecmp(*((const char **)a), *((const char **)b)));
}

/**
 * Update unfinished styles.
 */
void style_unfinished_cb(int, void *) {
}

/**
 * Update the style buffer
 */
void style_update_cb(int pos,   // I - Position of update
                     int nInserted,     // I - Number of inserted chars
                     int nDeleted,      // I - Number of deleted chars
                     int /* nRestyled */ ,      // I - Number of restyled chars
                     const char * /* deletedText */ ,   // I - Text that was deleted
                     void *cbArg) {     // I - Callback data
  BasicEditor *editor = (BasicEditor *) cbArg;
  Fl_Text_Buffer *stylebuf = editor->_stylebuf;
  Fl_Text_Buffer *textbuf = editor->_textbuf;

  // if this is just a selection change, just unselect the style buffer
  if (nInserted == 0 && nDeleted == 0) {
    stylebuf->unselect();
    return;
  }
  // track changes in the text buffer
  if (nInserted > 0) {
    // insert characters into the style buffer
    char *stylex = new char[nInserted + 1];
    memset(stylex, PLAIN, nInserted);
    stylex[nInserted] = '\0';
    stylebuf->replace(pos, pos + nDeleted, stylex);
    delete[]stylex;
  } else {
    // just delete characters in the style buffer
    stylebuf->remove(pos, pos + nDeleted);
  }

  // Select the area that was just updated to avoid unnecessary callbacks
  stylebuf->select(pos, pos + nInserted - nDeleted);

  // re-parse the changed region; we do this by parsing from the
  // beginning of the line of the changed region to the end of
  // the line of the changed region Then we check the last
  // style character and keep updating if we have a multi-line
  // comment character
  if (nInserted > 0) {
    int start = textbuf->line_start(pos);
    int end = textbuf->line_end(pos + nInserted);
    char *text_range = textbuf->text_range(start, end);
    char *style_range = stylebuf->text_range(start, end);
    int last = style_range[end - start - 1];

    editor->styleParse(text_range, style_range, end - start);
    stylebuf->replace(start, end, style_range);
    editor->redisplay_range(start, end);

    if (last != style_range[end - start - 1]) {
      // the last character on the line changed styles,
      // so reparse the remainder of the buffer
      free(text_range);
      free(style_range);
      end = textbuf->length();
      text_range = textbuf->text_range(start, end);
      style_range = stylebuf->text_range(start, end);
      editor->styleParse(text_range, style_range, end - start);
      stylebuf->replace(start, end, style_range);
      editor->redisplay_range(start, end);
    }
    free(text_range);
    free(style_range);
  }
}

//--BasicEditor------------------------------------------------------------------

BasicEditor::BasicEditor(int x, int y, int w, int h, StatusBar *status) :
  Fl_Text_Editor(x, y, w, h),
  _readonly(false),
  _status(status) {

  const char *s = getenv("INDENT_LEVEL");
  _indentLevel = (s && s[0] ? atoi(s) : 2);
  _matchingBrace = -1;
  _textbuf = new Fl_Text_Buffer();
  _stylebuf = new Fl_Text_Buffer();
  _search[0] = 0;
  highlight_data(_stylebuf, styletable,
                 sizeof(styletable) / sizeof(styletable[0]),
                 PLAIN, style_unfinished_cb, 0);
  _textbuf->add_modify_callback(style_update_cb, this);
  buffer(_textbuf);
}

BasicEditor::~BasicEditor() {
  delete _stylebuf;
}

/**
 * Parse text and produce style data.
 */
void BasicEditor::styleParse(const char *text, char *style, int length) {
  char current = PLAIN;
  int last = 0;                 // prev char was alpha-num
  char buf[1024];
  char *bufptr;
  const char *temp;
  int searchLen = strlen(_search);

  for (; length > 0; length--, text++) {
    if (current == PLAIN) {
      // check for directives, comments, strings, and keywords
      if ((*text == '#' && (*(text - 1) == 0 || *(text - 1) == 10)) ||
          (strncasecmp(text, "rem", 3) == 0 && text[3] == ' ') || *text == '\'') {
        // basic comment
        current = COMMENTS;
        for (; length > 0 && *text != '\n'; length--, text++) {
          if (*text == ';') {
            current = ITCOMMENTS;
          }
          *style++ = current;
        }
        if (length == 0) {
          break;
        }
      } else if (strncmp(text, "\\\"", 2) == 0) {
        // quoted quote
        *style++ = current;
        *style++ = current;
        text++;
        length--;
        continue;
      } else if (*text == '\"') {
        current = STRINGS;
      } else if (!last) {
        // begin keyword/number search at non-alnum boundary

        // test for digit sequence
        if (isdigit(*text)) {
          *style++ = DIGITS;
          if (*text == '0' && *(text + 1) == 'x') {
            // hex number
            *style++ = DIGITS;
            text++;
            length--;
          }
          while (*text && (*(text + 1) == '.' || isdigit(*(text + 1)))) {
            *style++ = DIGITS;
            text++;
            length--;
          }
          continue;
        }
        // test for a keyword
        temp = text;
        bufptr = buf;
        while (*temp != 0 && *temp != ' ' &&
               *temp != '\n' && *temp != '\r' && *temp != '"' &&
               *temp != '(' && *temp != ')' && *temp != '=' && bufptr < (buf + sizeof(buf) - 1)) {
          *bufptr++ = tolower(*temp++);
        }

        *bufptr = '\0';
        bufptr = buf;

        if (searchLen > 0) {
          const char *sfind = strstr(bufptr, _search);
          // find text match
          if (sfind != 0) {
            int offset = sfind - bufptr;
            style += offset;
            text += offset;
            length -= offset;
            for (int i = 0; i < searchLen && text < temp; i++) {
              *style++ = FINDMATCH;
              text++;
              length--;
            }
            text--;
            length++;
            last = 1;
            continue;
          }
        }

        if (bsearch(&bufptr, code_keywords, code_keywords_len, sizeof(code_keywords[0]), compare_keywords)) {
          while (text < temp) {
            *style++ = KEYWORDS;
            text++;
            length--;
          }
          text--;
          length++;
          last = 1;
          continue;
        } else if (bsearch(&bufptr, code_functions, code_functions_len,
                           sizeof(code_functions[0]), compare_keywords)) {
          while (text < temp) {
            *style++ = FUNCTIONS;
            text++;
            length--;
          }
          text--;
          length++;
          last = 1;
          continue;
        } else if (bsearch(&bufptr, code_procedures, code_procedures_len,
                           sizeof(code_procedures[0]), compare_keywords)) {
          while (text < temp) {
            *style++ = PROCEDURES;
            text++;
            length--;
          }
          text--;
          length++;
          last = 1;
          continue;
        }
      }
    } else if (current == STRINGS) {
      // continuing in string
      if (strncmp(text, "\\\"", 2) == 0) {
        // quoted end quote
        *style++ = current;
        *style++ = current;
        text++;
        length--;
        continue;
      } else if (*text == '\"') {
        // End quote
        *style++ = current;
        current = PLAIN;
        continue;
      }
    }
    // copy style info
    *style++ = current;
    last = isvar(*text) || *text == '.';

    if (*text == '\n') {
      current = PLAIN;          // basic lines do not continue
    }
  }
}

/**
 * handler for the style change event
 */
void BasicEditor::styleChanged() {
  _textbuf->select(0, _textbuf->length());
  _textbuf->select(0, 0);
  damage(FL_DAMAGE_ALL);
}

/**
 * display the editor buffer
 */
void BasicEditor::draw() {
  Fl_Text_Editor::draw();
  if (_matchingBrace != -1) {
    // highlight the matching brace
    int X, Y;
    int cursor = cursor_style();
    cursor_style(BLOCK_CURSOR);
    if (position_to_xy(_matchingBrace, &X, &Y)) {
      draw_cursor(X, Y);
    }
    cursor_style(cursor);
  }
}

/**
 * returns the indent position level
 */
unsigned BasicEditor::getIndent(char *spaces, int len, int pos) {
  // count the indent level and find the start of text
  char *buf = buffer()->line_text(pos);
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

/**
 * handler for the TAB character
 */
void BasicEditor::handleTab() {
  char spaces[250];
  int indent;

  // get the desired indent based on the previous line
  int lineStart = buffer()->line_start(insert_position());
  int prevLineStart = buffer()->line_start(lineStart - 1);

  if (prevLineStart && prevLineStart + 1 == lineStart) {
    // allows for a single blank line between statments
    prevLineStart = buffer()->line_start(prevLineStart - 1);
  }
  // note - spaces not used in this context
  indent = prevLineStart == 0 ? 0 : getIndent(spaces, sizeof(spaces), prevLineStart);

  // get the current lines indent
  char *buf = buffer()->line_text(lineStart);
  int curIndent = 0;
  while (buf && buf[curIndent] == ' ') {
    curIndent++;
  }

  // adjust indent for closure statements
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
    buffer()->insert(lineStart, spaces);
    if (insert_position() - lineStart < indent) {
      // jump cursor to start of text
      insert_position(lineStart + indent);
    } else {
      // move cursor along with text movement, staying on same line
      int maxpos = buffer()->line_end(lineStart);
      if (insert_position() + len <= maxpos) {
        insert_position(insert_position() + len);
      }
    }
  } else if (curIndent > indent) {
    // remove excess spaces
    buffer()->remove(lineStart, lineStart + (curIndent - indent));
  } else {
    // already have ideal indent - soft-tab to indent
    insert_position(lineStart + indent);
  }
  free((void *)buf);
}

/**
 * sets the current display font
 */
void BasicEditor::setFont(Fl_Font font) {
  if (font) {
    int len = sizeof(styletable) / sizeof(styletable[0]);
    for (int i = 0; i < len; i++) {
      styletable[i].font = font;
    }
    styleChanged();
  }
}

/**
 * sets the current font size
 */
void BasicEditor::setFontSize(int size) {
  int len = sizeof(styletable) / sizeof(styletable[0]);
  for (int i = 0; i < len; i++) {
    styletable[i].size = size;
  }
  styleChanged();
}

/**
 * display the matching brace
 */
void BasicEditor::showMatchingBrace() {
  char cursorChar = buffer()->char_at(insert_position() - 1);
  char cursorMatch = 0;
  int pair = -1;
  int iter = -1;
  int pos = insert_position() - 2;

  switch (cursorChar) {
  case ']':
    cursorMatch = '[';
    break;
  case ')':
    cursorMatch = '(';
    break;
  case '(':
    cursorMatch = ')';
    pos = insert_position();
    iter = 1;
    break;
  case '[':
    cursorMatch = ']';
    iter = 1;
    pos = insert_position();
    break;
  }
  if (cursorMatch != -0) {
    // scan for matching opening on the same line
    int level = 1;
    int len = buffer()->length();
    int gap = 0;
    while (pos > 0 && pos < len) {
      char nextChar = buffer()->char_at(pos);
      if (nextChar == 0 || nextChar == '\n') {
        break;
      }
      if (nextChar == cursorChar) {
        level++;                // nested char
      } else if (nextChar == cursorMatch) {
        level--;
        if (level == 0) {
          // found matching char at pos
          if (gap > 1) {
            pair = pos;
          }
          break;
        }
      }
      pos += iter;
      gap++;
    }
  }

  if (_matchingBrace != -1) {
    int lineStart = buffer()->line_start(_matchingBrace);
    int lineEnd = buffer()->line_end(_matchingBrace);
    redisplay_range(lineStart, lineEnd);
    _matchingBrace = -1;
  }
  if (pair != -1) {
    redisplay_range(pair, pair);
    _matchingBrace = pair;
  }
}

/**
 * highlight the given search text
 */
void BasicEditor::showFindText(const char *find) {
  // copy lowercase search string for high-lighting
  strcpy(_search, find);
  int findLen = strlen(_search);

  for (int i = 0; i < findLen; i++) {
    _search[i] = tolower(_search[i]);
  }

  style_update_cb(0, _textbuf->length(), _textbuf->length(), 0, 0, this);
}

/**
 * FLTK event handler
 */
int BasicEditor::handle(int e) {
  int cursor_pos = insert_position();
  bool navigateKey = false;

  switch (Fl::event_key()) {
  case FL_Home:
  case FL_Left:
  case FL_Up:
  case FL_Right:
  case FL_Down:
  case FL_Page_Up:
  case FL_Page_Down:
  case FL_End:
    navigateKey = true;
  }

  if (_readonly && ((e == FL_KEYBOARD && !navigateKey) || e == FL_PASTE)) {
    // prevent buffer modification when in readonly state
    return 0;
  }

  if (e == FL_KEYBOARD && Fl::event_key() == FL_Tab) {
    if (Fl::event_state(FL_CTRL)) {
      // pass ctrl+key to parent
      return 0;
    }
    handleTab();
    return 1;                   // skip default handler
  }
  // call default handler then process keys
  int rtn = Fl_Text_Editor::handle(e);
  switch (e) {
  case FL_KEYBOARD:
    if (Fl::event_key() == FL_Enter) {
      char spaces[250];
      int indent = getIndent(spaces, sizeof(spaces), cursor_pos);
      if (indent) {
        buffer()->insert(insert_position(), spaces);
        insert_position(insert_position() + indent);
        damage(FL_DAMAGE_ALL);
      }
    }
    // fallthru to show row-col
  case FL_RELEASE:
    showMatchingBrace();
    showRowCol();
    break;
  }

  return rtn;
}

/**
 * displays the current row and col position
 */
void BasicEditor::showRowCol() {
  int row = -1;
  int col = 0;

  if (!position_to_linecol(insert_position(), &row, &col)) {
    // This is a workaround for a bug in the FLTK TextDisplay widget
    // where linewrapping causes a mis-calculation of line offsets which
    // sometimes prevents the display of the last few lines of text.
    insert_position(0);
    scroll(0, 0);
    insert_position(buffer()->length());
    scroll(count_lines(0, buffer()->length(), 1), 0);
    position_to_linecol(insert_position(), &row, &col);
  }

  _status->setRowCol(row, col + 1);
}

/**
 * sets the cursor to the given line number
 */
void BasicEditor::gotoLine(int line) {
  int numLines = buffer()->count_lines(0, buffer()->length());
  if (line < 1) {
    line = 1;
  } else if (line > numLines) {
    line = numLines;
  }
  int pos = buffer()->skip_lines(0, line - 1);  // find pos at line-1
  insert_position(buffer()->line_start(pos));   // insert at column 0
  show_insert_position();
  _status->setRowCol(line, 1);
  scroll(line, hor_offset());
}

/**
 * returns where text selection starts
 */
void BasicEditor::getSelStartRowCol(int *row, int *col) {
  int start = buffer()->primary_selection()->start();
  int end = buffer()->primary_selection()->end();
  if (start == end) {
    *row = -1;
    *col = -1;
  } else {
    position_to_linecol(start, row, col);
  }
}

/**
 * returns where text selection ends
 */
void BasicEditor::getSelEndRowCol(int *row, int *col) {
  int start = buffer()->primary_selection()->start();
  int end = buffer()->primary_selection()->end();
  if (start == end) {
    *row = -1;
    *col = -1;
  } else {
    position_to_linecol(end, row, col);
  }
}

/**
 * return the selected text and its coordinate rectangle
 */
char *BasicEditor::getSelection(Fl_Rect *rc) {
  char *result = 0;
  if (!_readonly) {
    int x1, y1, x2, y2, start, end;

    if (_textbuf->selected()) {
      _textbuf->selection_position(&start, &end);
    } else {
      int pos = insert_position();
      if (isvar(_textbuf->char_at(pos))) {
        start = _textbuf->word_start(pos);
        end = _textbuf->word_end(pos);
      } else {
        start = end = 0;
      }
    }

    if (start != end) {
      position_to_xy(start, &x1, &y1);
      position_to_xy(end, &x2, &y2);

      rc->x(x1);
      rc->y(y1);
      rc->w(x2 - x1);
      rc->h(maxSize());
      result = _textbuf->text_range(start, end);
    }
  }
  return result;
}

/**
 * returns the current font size
 */
int BasicEditor::getFontSize() {
  return (int)styletable[0].size;
}

/**
 * returns the current font face name
 */
Fl_Font BasicEditor::getFont() {
  return styletable[0].font;
}

/**
 * returns the BASIC keyword list
 */
void BasicEditor::getKeywords(strlib::List<String *> &keywords) {
  for (int i = 0; i < code_keywords_len; i++) {
    keywords.add(new String(code_keywords[i]));
  }

  for (int i = 0; i < code_functions_len; i++) {
    keywords.add(new String(code_functions[i]));
  }

  for (int i = 0; i < code_procedures_len; i++) {
    keywords.add(new String(code_procedures[i]));
  }
}

/**
 * returns the row and col position for the current cursor position
 */
void BasicEditor::getRowCol(int *row, int *col) {
  position_to_linecol(insert_position(), row, col);
}

/**
 * find text within the editor buffer
 */
bool BasicEditor::findText(const char *find, bool forward, bool updatePos) {
  showFindText(find);

  bool found = false;
  if (find != 0 && find[0] != 0) {
    int pos = insert_position();
    found = forward ? _textbuf->search_forward(pos, _search, &pos) :
            _textbuf->search_backward(pos - strlen(find), _search, &pos);
    if (found && updatePos) {
      _textbuf->select(pos, pos + strlen(_search));
      insert_position(pos + strlen(_search));
      show_insert_position();
    }
  }
  return found;
}

