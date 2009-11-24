// $Id$
//
// Based on test/editor.cxx - A simple text editor program for the Fast
// Light Tool Kit (FLTK). This program is described in Chapter 4 of the FLTK
// Programmer's Guide.
// Copyright 1998-2003 by Bill Spitzak and others.
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <fltk/damage.h>
#include <fltk/events.h>

#include "BasicEditor.h"
#include "kwp.h"

using namespace fltk;
using namespace strlib;

Color defaultColor[] = {
  BLACK,                 // A - Plain
  color(0, 128, 0),      // B - Comments
  color(0, 0, 192),      // C - Strings
  color(192, 0, 0),      // D - code_keywords
  color(128, 128, 0),    // E - code_functions
  color(0, 128, 128),    // F - code_procedures
  color(128, 0, 128),    // G - Find matches
  color(0, 128, 0),      // H - Italic Comments ';
  color(0, 128, 128),    // I - Numbers
  color(128, 128, 64),   // J - Operators
};

TextDisplay::StyleTableEntry styletable[] = { // Style table
  { defaultColor[0], COURIER, 12},     // A - Plain
  { defaultColor[1], COURIER, 12},     // B - Comments
  { defaultColor[2], COURIER, 12},     // C - Strings
  { defaultColor[3], COURIER, 12},     // D - code_keywords
  { defaultColor[4], COURIER, 12},     // E - code_functions
  { defaultColor[5], COURIER, 12},     // F - code_procedures
  { defaultColor[6], COURIER, 12},     // G - Find matches
  { defaultColor[7], COURIER_ITALIC, 12},// H - Italic Comments ';
  { defaultColor[8], COURIER, 12},     // I - Numbers
  { defaultColor[9], COURIER, 12},     // J - Operators
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

const int numCodeKeywords = sizeof(code_keywords) / sizeof(code_keywords[0]);
const int numCodeFunctions = sizeof(code_functions) / sizeof(code_functions[0]);
const int numCodeProcedures = sizeof(code_procedures) / sizeof(code_procedures[0]);

/**
 * return whether the character is a valid variable symbol
 */
bool isvar(int c) {
  return (isalnum(c) || c == '_');
}

// 'compare_keywords()' - Compare two keywords
int compare_keywords(const void *a, const void *b)
{
  return (strcasecmp(*((const char **)a), *((const char **)b)));
}

// 'style_unfinished_cb()' - Update unfinished styles.
void style_unfinished_cb(int, void *)
{
}

// 'style_update()' - Update the style buffer
void style_update_cb(int pos,      // I - Position of update
                     int nInserted,  // I - Number of inserted chars
                     int nDeleted, // I - Number of deleted chars
                     int /* nRestyled */ , // I - Number of restyled chars
                     const char * /* deletedText */ ,  // I - Text that was deleted
                     void *cbArg)
{                               // I - Callback data
  int start;              // Start of text
  int end;                // End of text
  char last;              // Last style on line
  char *text_range;       // Text data
  char *style_range;      // Text data

  BasicEditor *editor = (BasicEditor*) cbArg;
  TextBuffer *stylebuf = editor->stylebuf;
  TextBuffer *textbuf = editor->textbuf;

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
    delete[] stylex;
  }
  else {
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
  start = textbuf->line_start(pos);
  end = textbuf->line_end(pos + nInserted);
  text_range = textbuf->text_range(start, end);
  style_range = stylebuf->text_range(start, end);
  last = style_range[end - start - 1];

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

//--BasicEditor------------------------------------------------------------------

BasicEditor::BasicEditor(int x, int y, int w, int h, StatusBar* status) : 
  TextEditor(x, y, w, h), status(status) {
  readonly = false;
  const char *s = getenv("INDENT_LEVEL");
  indentLevel = (s && s[0] ? atoi(s) : 2);
  matchingBrace = -1;

  textbuf = buffer(); // reference only
  stylebuf = new TextBuffer();
  search[0] = 0;
  highlight_data(stylebuf, styletable,
                 sizeof(styletable) / sizeof(styletable[0]),
                 PLAIN, style_unfinished_cb, 0);

  textbuf->add_modify_callback(style_update_cb, this);
}

BasicEditor::~BasicEditor()
{
  // cleanup
  delete stylebuf;
}

/**
 * Parse text and produce style data.
 */
void BasicEditor::styleParse(const char *text, char *style, int length)
{
  char current = PLAIN;
  int last = 0;                 // prev char was alpha-num
  char buf[255];
  char *bufptr;
  const char *temp;
  int searchLen = strlen(search);

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
      }
      else if (strncmp(text, "\\\"", 2) == 0) {
        // quoted quote
        *style++ = current;
        *style++ = current;
        text++;
        length--;
        continue;
      }
      else if (*text == '\"') {
        current = STRINGS;
      }
      else if (!last) {
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
               *temp != '(' && *temp != ')' && *temp != '=' &&
               bufptr < (buf + sizeof(buf) - 1)) {
          *bufptr++ = tolower(*temp++);
        }

        *bufptr = '\0';
        bufptr = buf;

        if (searchLen > 0) {
          const char *sfind = strstr(bufptr, search);
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

        if (bsearch(&bufptr, code_keywords, numCodeKeywords,
                    sizeof(code_keywords[0]), compare_keywords)) {
          while (text < temp) {
            *style++ = KEYWORDS;
            text++;
            length--;
          }
          text--;
          length++;
          last = 1;
          continue;
        }
        else if (bsearch(&bufptr, code_functions, numCodeFunctions,
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
        }
        else if (bsearch(&bufptr, code_procedures, numCodeProcedures,
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
    }
    else if (current == STRINGS) {
      // continuing in string
      if (strncmp(text, "\\\"", 2) == 0) {
        // quoted end quote
        *style++ = current;
        *style++ = current;
        text++;
        length--;
        continue;
      }
      else if (*text == '\"') {
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
  textbuf->select(0, textbuf->length());
  textbuf->select(0, 0);
  redraw(DAMAGE_ALL);
}

/**
 * display the editor buffer
 */
void BasicEditor::draw()
{
  TextEditor::draw();
  if (matchingBrace != -1) {
    // highlight the matching brace
    int X, Y;
    int cursor = cursor_style_;
    cursor_style_ = BLOCK_CURSOR;
    if (position_to_xy(matchingBrace, &X, &Y)) {
      draw_cursor(X, Y);
    }
    cursor_style_ = cursor;
  }
}

/**
 * returns the indent position level
 */
unsigned BasicEditor::getIndent(char *spaces, int len, int pos)
{
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
    for (int j = 0; j < indentLevel; j++, i++) {
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
void BasicEditor::handleTab()
{
  char spaces[250];
  int indent;

  // get the desired indent based on the previous line
  int lineStart = buffer()->line_start(cursor_pos_);
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
    if (indent >= indentLevel) {
      indent -= indentLevel;
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
    if (cursor_pos_ - lineStart < indent) {
      // jump cursor to start of text
      cursor_pos_ = lineStart + indent;
    }
    else {
      // move cursor along with text movement, staying on same line
      int maxpos = buffer()->line_end(lineStart);
      if (cursor_pos_ + len <= maxpos) {
        cursor_pos_ += len;
      }
    }
  }
  else if (curIndent > indent) {
    // remove excess spaces
    buffer()->remove(lineStart, lineStart + (curIndent - indent));
  }
  else {
    // already have ideal indent - soft-tab to indent
    insert_position(lineStart + indent);
  }
  free((void *)buf);
}

/**
 * sets the current display font
 */
void BasicEditor::setFont(Font* font)
{
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
void BasicEditor::setFontSize(int size)
{
  int len = sizeof(styletable) / sizeof(styletable[0]);
  for (int i = 0; i < len; i++) {
    styletable[i].size = size;
  }
  styleChanged();
}

/**
 * display the matching brace
 */
void BasicEditor::showMatchingBrace()
{
  char cursorChar = buffer()->character(cursor_pos_ - 1);
  char cursorMatch = 0;
  int pair = -1;
  int iter = -1;
  int pos = cursor_pos_ - 2;

  switch (cursorChar) {
  case ']':
    cursorMatch = '[';
    break;
  case ')':
    cursorMatch = '(';
    break;
  case '(':
    cursorMatch = ')';
    pos = cursor_pos_;
    iter = 1;
    break;
  case '[':
    cursorMatch = ']';
    iter = 1;
    pos = cursor_pos_;
    break;
  }
  if (cursorMatch != -0) {
    // scan for matching opening on the same line
    int level = 1;
    int len = buffer()->length();
    int gap = 0;
    while (pos > 0 && pos < len) {
      char nextChar = buffer()->character(pos);
      if (nextChar == 0 || nextChar == '\n') {
        break;
      }
      if (nextChar == cursorChar) {
        level++;                // nested char
      }
      else if (nextChar == cursorMatch) {
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

  if (matchingBrace != -1) {
    int lineStart = buffer()->line_start(matchingBrace);
    int lineEnd = buffer()->line_end(matchingBrace);
    redisplay_range(lineStart, lineEnd);
    matchingBrace = -1;
  }
  if (pair != -1) {
    redisplay_range(pair, pair);
    matchingBrace = pair;
  }
}

/**
 * highlight the given search text
 */
void BasicEditor::showFindText(const char *find)
{
  // copy lowercase search string for high-lighting
  strcpy(search, find);
  int findLen = strlen(search);

  for (int i = 0; i < findLen; i++) {
    search[i] = tolower(search[i]);
  }

  style_update_cb(0, textbuf->length(), textbuf->length(), 0, 0, this);
}

/**
 * FLTK event handler
 */
int BasicEditor::handle(int e)
{
  int cursorPos = cursor_pos_;
  char spaces[250];
  int indent;

  if (readonly && (e == KEY || e == PASTE)) {
    return 0;
  }

  if (e == KEY && event_key() == TabKey) {
    if (event_key_state(LeftCtrlKey) || event_key_state(RightCtrlKey)) {
      // pass ctrl+key to parent
      return 0;
    }
    handleTab();
    return 1;                   // skip default handler
  }

  // call default handler then process keys
  int rtn = TextEditor::handle(e);
  switch (e) {
  case KEY:
    if (event_key() == ReturnKey) {
      indent = getIndent(spaces, sizeof(spaces), cursorPos);
      if (indent) {
        buffer()->insert(cursor_pos_, spaces);
        cursor_pos_ += indent;
        redraw(DAMAGE_ALL);
      }
    }

    // fallthru to show row-col
  case RELEASE:
    showMatchingBrace();
    showRowCol();
    break;
  }

  return rtn;
}

/**
 * displays the current row and col position
 */
void BasicEditor::showRowCol()
{
  int row, col;

  if (!position_to_linecol(cursor_pos_, &row, &col)) {
    // pageup/pagedown
    layout();
    position_to_linecol(cursor_pos_, &row, &col);
  }
  status->setRowCol(row, col + 1);
}

/**
 * sets the cursor to the given line number
 */
void BasicEditor::gotoLine(int line)
{
  int numLines = buffer()->count_lines(0, buffer()->length());
  if (line < 1) {
    line = 1;
  }
  else if (line > numLines) {
    line = numLines;
  }
  int pos = buffer()->skip_lines(0, line - 1);  // find pos at line-1
  insert_position(buffer()->line_start(pos)); // insert at column 0
  show_insert_position();
  status->setRowCol(line, 1);
}

/**
 * returns where text selection starts
 */
void BasicEditor::getSelStartRowCol(int *row, int *col)
{
  int start = buffer()->primary_selection()->start();
  int end = buffer()->primary_selection()->end();
  if (start == end) {
    *row = -1;
    *col = -1;
  }
  else {
    position_to_linecol(start, row, col);
  }
}

/**
 * returns where text selection ends
 */
void BasicEditor::getSelEndRowCol(int *row, int *col)
{
  int start = buffer()->primary_selection()->start();
  int end = buffer()->primary_selection()->end();
  if (start == end) {
    *row = -1;
    *col = -1;
  }
  else {
    position_to_linecol(end, row, col);
  }
}

/**
 * return the selected text and its coordinate rectangle
 */
char* BasicEditor::getSelection(Rectangle* rc) {
  char* result = 0;
  if (!readonly) {
    int x1, y1, x2, y2, start, end;

    if (textbuf->selected()) {
      textbuf->selection_position(&start, &end);
    }
    else {
      int pos = insert_position();
      if (isvar(textbuf->character(pos))) {
        start = textbuf->word_start(pos);
        end = textbuf->word_end(pos);
      }
      else {
        start = end = 0;
      }
    }

    if (start != end) {
      position_to_xy(start, &x1, &y1);
      position_to_xy(end, &x2, &y2);
    
      rc->x(x1);
      rc->y(y1);
      rc->w(x2 - x1);
      rc->h(maxsize_);
      result = textbuf->text_range(start, end);
    }
  }
  return result;
}

/**
 * returns the current font size
 */
int BasicEditor::getFontSize()
{
  return (int)styletable[0].size;
}

/**
 * returns the current font face name
 */
const char* BasicEditor::getFontName()
{
  return styletable[0].font->name();
}

/**
 * returns the BASIC keyword list
 */
void BasicEditor::getKeywords(strlib::List& keywords) {
  for (int i = 0; i < numCodeKeywords; i++) {
    keywords.add(new String(code_keywords[i]));
  }

  for (int i = 0; i < numCodeFunctions; i++) {
    keywords.add(new String(code_functions[i]));
  }

  for (int i = 0; i < numCodeProcedures; i++) {
    keywords.add(new String(code_procedures[i]));
  }
}

/**
 * returns the row and col position for the current cursor position
 */
void BasicEditor::getRowCol(int *row, int *col)
{
  position_to_linecol(cursor_pos_, row, col);
}

/**
 * find text within the editor buffer
 */
bool BasicEditor::findText(const char *find, bool forward, bool updatePos)
{
  showFindText(find);

  bool found = false;
  if (find != 0 && find[0] != 0) {
    int pos = insert_position();
    found = forward ? textbuf->search_forward(pos, search, &pos) :
            textbuf->search_backward(pos - strlen(find), search, &pos);
    if (found && updatePos) {
      textbuf->select(pos, pos + strlen(search));
      insert_position(pos + strlen(search));
      show_insert_position();
    }
  }
  return found;
}

//--EndOfFile-------------------------------------------------------------------
