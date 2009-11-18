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

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <fltk/ColorChooser.h>
#include <fltk/Item.h>
#include <fltk/ask.h>
#include <fltk/damage.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/TiledGroup.h>

#include "MainWindow.h"
#include "EditorWidget.h"
#include "FileWidget.h"
#include "kwp.h"

using namespace fltk;

// in MainWindow
extern String recentPath[];
extern Widget* recentMenu[];

// in dev_fltk.cpp
void getHomeDir(char *filename);

int completionIndex = 0;

static bool rename_active = false;

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

const char configFile[] = "config.txt";
const char fontConfigRead[] = "name=%[^;];size=%d\n";
const char fontConfigSave[] = "name=%s;size=%d\n";

#define TOOLBAR_HEIGHT 26 // height of toolbar widget

/**
 * return whether the character is a valid variable symbol
 */
bool isvar(int c) {
  return (isalnum(c) || c == '_');
}

EditorWidget* get_editor() {
  EditorWidget* result = wnd->getEditor();
  if (!result) {
    result = wnd->getEditor(true);
  }
  return result;
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

  CodeEditor *editor = (CodeEditor*) cbArg;
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

//--CodeEditor------------------------------------------------------------------

CodeEditor::CodeEditor(int x, int y, int w, int h) : TextEditor(x, y, w, h)
{
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
}

CodeEditor::~CodeEditor()
{
  // cleanup
  delete stylebuf;
}

// 'style_parse()' - Parse text and produce style data.
void CodeEditor::styleParse(const char *text, char *style, int length)
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

void CodeEditor::styleChanged() {
  textbuf->select(0, textbuf->length());
  textbuf->select(0, 0);
  redraw(DAMAGE_ALL);
}

void CodeEditor::draw()
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

unsigned CodeEditor::getIndent(char *spaces, int len, int pos)
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

void CodeEditor::handleTab()
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

void CodeEditor::showMatchingBrace()
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

void CodeEditor::showFindText(const char *find)
{
  // copy lowercase search string for high-lighting
  strcpy(search, find);
  int findLen = strlen(search);

  for (int i = 0; i < findLen; i++) {
    search[i] = tolower(search[i]);
  }

  style_update_cb(0, textbuf->length(), textbuf->length(), 0, 0, this);
}

int CodeEditor::handle(int e)
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

void CodeEditor::showRowCol()
{
  int row, col;

  if (!position_to_linecol(cursor_pos_, &row, &col)) {
    // pageup/pagedown
    layout();
    position_to_linecol(cursor_pos_, &row, &col);
  }
  get_editor()->setRowCol(row, col + 1);
}

void CodeEditor::gotoLine(int line)
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
  get_editor()->setRowCol(line, 1);
}

void CodeEditor::getSelStartRowCol(int *row, int *col)
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

void CodeEditor::getSelEndRowCol(int *row, int *col)
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
char* CodeEditor::getSelection(Rectangle* rc) {
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

void CodeEditor::getRowCol(int *row, int *col)
{
  position_to_linecol(cursor_pos_, row, col);
}

bool CodeEditor::findText(const char *find, bool forward, bool updatePos)
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

//--EditorWidget----------------------------------------------------------------

EditorWidget::EditorWidget(int x, int y, int w, int h) : Group(x, y, w, h)
{
  int tbHeight = TOOLBAR_HEIGHT;
  int stHeight = MNU_HEIGHT;

  filename[0] = 0;
  dirty = false;
  loading = false;
  modifiedTime = 0;
  box(NO_BOX);

  begin();

  int tileHeight = h - (tbHeight + stHeight + 8);
  int ttyHeight = h / 8;
  int editHeight = tileHeight - ttyHeight;

  TiledGroup* tile = new TiledGroup(0, 0, w, tileHeight);
  tile->begin();

  editor = new CodeEditor(0, 0, w, editHeight);
  editor->linenumber_width(40);
  editor->wrap_mode(true, 0);
  editor->selection_color(fltk::color(190, 189, 188));
  editor->textbuf->add_modify_callback(style_update_cb, editor);
  editor->textbuf->add_modify_callback(changed_cb, this);
  editor->box(NO_BOX);
  editor->take_focus();

  tty = new TtyWidget(0, editHeight, w, ttyHeight, 1000);
  tty->color(WHITE); // bg
  tty->labelcolor(BLACK); // fg
  tile->end();

  // create the editor toolbar
  w -= 4;
  Group *toolbar = new Group(2, tty->b() + 2, w, tbHeight);
  toolbar->begin();
  toolbar->box(FLAT_BOX);

  // widths become relative when the outer window is resized
  int cmd_bn_w = 80;
  int func_bn_w = w / 3;
  int spacing = 8;
  int find_bn_w = w - (cmd_bn_w + func_bn_w + spacing);

  // command selection
  commandOpt = cmd_find;
  commandChoice = new Choice(2, 2, cmd_bn_w, MNU_HEIGHT);
  commandChoice->labelfont(HELVETICA);
  commandChoice->begin();
  new Item("Find:", 0, command_opt_cb, (void*) cmd_find);
  new Item("Inc Find:", 0, command_opt_cb, (void*) cmd_find_inc);
  new Item("Replace:" ,0, command_opt_cb, (void*) cmd_replace);
  new Item("With:" ,0, command_opt_cb, (void*) cmd_replace_with);
  new Item("Goto:", 0, command_opt_cb, (void*) cmd_goto);
  commandChoice->end();

  // command control
  commandText = new Input(commandChoice->r() + 2, 2, find_bn_w, MNU_HEIGHT);
  commandText->align(ALIGN_LEFT | ALIGN_CLIP);
  commandText->callback(EditorWidget::command_cb, (void*) 1);
  commandText->when(WHEN_ENTER_KEY_ALWAYS);
  commandText->labelfont(HELVETICA);

  // sub-func jump droplist
  funcList = new Choice(commandText->r() + 4, 2, func_bn_w, MNU_HEIGHT);
  funcList->callback(func_list_cb, 0);
  funcList->labelfont(HELVETICA);
  funcList->begin();
  new Item();
  new Item(SCAN_LABEL);
  funcList->end();

  // close the tool-bar with a resizeable end-box
  toolbar->resizable(commandText);
  toolbar->end();

  // editor status bar
  Group *statusBar = new Group(2, toolbar->b() + 2, w, MNU_HEIGHT);
  statusBar->begin();
  statusBar->box(NO_BOX);
  fileStatus = new Widget(0, 0, w - 140, MNU_HEIGHT - 2);
  modStatus = new Widget(fileStatus->r() + 2, 0, 33, MNU_HEIGHT - 2);
  runStatus = new Widget(modStatus->r() + 2, 0, 33, MNU_HEIGHT - 2);
  rowStatus = new Widget(runStatus->r() + 2, 0, 33, MNU_HEIGHT - 2);
  colStatus = new Widget(rowStatus->r() + 2, 0, 33, MNU_HEIGHT - 2);

  for (int n = 0; n < statusBar->children(); n++) {
    Widget *w = statusBar->child(n);
    w->labelfont(HELVETICA);
    w->box(BORDER_BOX);
  }

  fileStatus->align(ALIGN_INSIDE_LEFT | ALIGN_CLIP);
  statusBar->resizable(fileStatus);
  statusBar->end();

  resizable(editor);
  end();

  setEditorColor(WHITE, true);
  loadConfig();
}

EditorWidget::~EditorWidget()
{
  delete editor;
}

//--Event handler methods-------------------------------------------------------

void EditorWidget::change_case(Widget* w, void* eventData)
{
  int start, end;
  TextBuffer *tb = editor->buffer();
  char *selection;

  if (tb->selected()) {
    selection = (char *)tb->selection_text();
    tb->selection_position(&start, &end);
  }
  else {
    int pos = editor->insert_position();
    start = tb->word_start(pos);
    end = tb->word_end(pos);
    selection = (char *)tb->text_range(start, end);
  }
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
        selection[i+1] = toupper(selection[i+1]);
      }
    }
  }

  if (selection[0]) {
    tb->replace_selection(selection);
    tb->select(start, end);
  }
  free((void *)selection);
}

void EditorWidget::command_opt(Widget* w, void* eventData)
{
  setCommand((CommandOpt) (int) eventData);
}

void EditorWidget::cut_text(Widget* w, void* eventData)
{
  TextEditor::kf_cut(0, editor);
}

void EditorWidget::do_delete(Widget* w, void* eventData)
{
  editor->textbuf->remove_selection();
}

void EditorWidget::expand_word(Widget* w, void* eventData)
{
  int start, end;
  const char *fullWord = 0;
  unsigned fullWordLen = 0;

  TextBuffer *textbuf = editor->buffer();
  const char *text = textbuf->text();

  if (textbuf->selected()) {
    // get word before selection
    int pos1, pos2;
    textbuf->selection_position(&pos1, &pos2);
    start = textbuf->word_start(pos1 - 1);
    end = pos1;
    // get word from before selection to end of selection
    fullWord = text + start;
    fullWordLen = pos2 - start - 1;
  }
  else {
    // nothing selected - get word to left of cursor position
    int pos = editor->insert_position();
    end = textbuf->word_end(pos);
    start = textbuf->word_start(end - 1);
    completionIndex = 0;
  }

  if (start >= end) {
    return;
  }

  const char *expandWord = text + start;
  unsigned expandWordLen = end - start;
  int wordPos = 0;

  // scan for expandWord from within the current text buffer
  if (completionIndex != -1 &&
      searchBackward(text, start - 1, expandWord, expandWordLen, &wordPos)) {

    int matchPos = -1;
    if (textbuf->selected() == 0) {
      matchPos = wordPos;
      completionIndex = 1;      // find next word on next call
    }
    else {
      // find the next word prior to the currently selected word
      int index = 1;
      while (wordPos > 0) {
        if (strncasecmp(text + wordPos, fullWord, fullWordLen) != 0 ||
            isalpha(text[wordPos + fullWordLen + 1])) {
          // isalpha - matches fullWord but word has more chars
          matchPos = wordPos;
          if (completionIndex == index) {
            completionIndex++;
            break;
          }
          // count index for non-matching fullWords only
          index++;
        }

        if (searchBackward(text, wordPos - 1, expandWord,
                           expandWordLen, &wordPos) == 0) {
          matchPos = -1;
          break;                // no more partial matches
        }
      }
      if (index == completionIndex) {
        // end of expansion sequence
        matchPos = -1;
      }
    }
    if (matchPos != -1) {
      char *word = textbuf->text_range(matchPos, textbuf->word_end(matchPos));
      if (textbuf->selected()) {
        textbuf->replace_selection(word + expandWordLen);
      }
      else {
        textbuf->insert(end, word + expandWordLen);
      }
      textbuf->select(end, end + strlen(word + expandWordLen));
      editor->insert_position(end + strlen(word + expandWordLen));
      free((void *)word);
      return;
    }
  }

  completionIndex = -1;         // no more buffer expansions

  strlib::List keywords;
  getKeywords(keywords);

  // find the next replacement
  int firstIndex = -1;
  int lastIndex = -1;
  int curIndex = -1;
  int numWords = keywords.length();
  for (int i = 0; i < numWords; i++) {
    const char *keyword = ((String *) keywords.get(i))->toString();
    if (strncasecmp(expandWord, keyword, expandWordLen) == 0) {
      if (firstIndex == -1) {
        firstIndex = i;
      }
      if (fullWordLen == 0) {
        if (expandWordLen == strlen(keyword)) {
          // nothing selected and word to left of cursor matches
          curIndex = i;
        }
      }
      else if (strncasecmp(fullWord, keyword, fullWordLen) == 0) {
        // selection+word to left of selection matches
        curIndex = i;
      }
      lastIndex = i;
    }
    else if (lastIndex != -1) {
      // moved beyond matching words
      break;
    }
  }

  if (lastIndex != -1) {
    if (lastIndex == curIndex || curIndex == -1) {
      lastIndex = firstIndex;   // wrap to first in subset
    }
    else {
      lastIndex = curIndex + 1;
    }

    const char *keyword = ((String *) keywords.get(lastIndex))->toString();
    // updated the segment of the replacement text
    // that completes the current selection
    if (textbuf->selected()) {
      textbuf->replace_selection(keyword + expandWordLen);
    }
    else {
      textbuf->insert(end, keyword + expandWordLen);
    }
    textbuf->select(end, end + strlen(keyword + expandWordLen));
  }
}

void EditorWidget::find(Widget* w, void* eventData)
{
  setCommand(cmd_find);
}

void EditorWidget::command(Widget* w, void* eventData)
{
  if (!readonly()) {
    bool found = false;
    bool forward = (int) eventData;
    bool updatePos = (commandOpt != cmd_find_inc);

    switch (commandOpt) {
    case cmd_find_inc:
    case cmd_find:
      found = editor->findText(commandText->value(), forward, updatePos);
      commandText->textcolor(found ? commandChoice->textcolor() : RED);
      commandText->redraw();
      break;

    case cmd_replace:
      commandBuffer.empty();
      commandBuffer.append(commandText->value());
      setCommand(cmd_replace_with);
      break;

    case cmd_replace_with:
      replace_next();
      break;

    case cmd_goto:
      gotoLine(atoi(commandText->value()));
      take_focus();
      break;
    }
  }
}

void EditorWidget::font_name(Widget* w, void* eventData)
{
  setFont(fltk::font(w->label(), 0));
  wnd->updateConfig(this);
}

void EditorWidget::func_list(Widget* w, void* eventData)
{
  if (funcList && funcList->item()) {
    const char *label = funcList->item()->label();
    if (label) {
      if (strcmp(label, SCAN_LABEL) == 0) {
        funcList->clear();
        funcList->begin();
        createFuncList();
        new Item(SCAN_LABEL);
        funcList->end();
      }
      else {
        gotoLine((int) funcList->item()->user_data());
        take_focus();
      }
    }
  }
}

void EditorWidget::goto_line(Widget* w, void* eventData)
{
  setCommand(cmd_goto);  
}

void EditorWidget::paste_text(Widget* w, void* eventData)
{
  TextEditor::kf_paste(0, editor);
}

/**
 * rename the currently selected variable
 */
void EditorWidget::rename_word(Widget* w, void* eventData) {
  if (rename_active) {
    rename_active = false;
  }
  else {
    Rectangle rc;
    char* selection = getSelection(&rc);
    if (selection) {
      showFindText(selection);
      begin();
      LineInput *in = new LineInput(rc.x(), rc.y(), rc.w() + 10, rc.h());
      end();
      in->text(selection);
      in->callback(rename_word_cb);
      in->textfont(COURIER);
      in->textsize(getFontSize());

      rename_active = true;
      while (rename_active && in->focused()) {
        fltk::wait();
      }

      showFindText("");
      replaceAll(selection, in->value(), true, true);
      remove(in);
      take_focus();
      delete in;
      free((void *)selection);
    }
  }  
}

void EditorWidget::replace_next(Widget* w, void* eventData)
{
  if (readonly()) {
    return;
  }

  const char *find = commandBuffer;
  const char *replace = commandText->value();

  TextBuffer *textbuf = editor->textbuf;
  int pos = editor->insert_position();
  int found = textbuf->search_forward(pos, find, &pos);

  if (found) {
    // found a match; update the position and replace text
    textbuf->select(pos, pos + strlen(find));
    textbuf->remove_selection();
    textbuf->insert(pos, replace);
    textbuf->select(pos, pos + strlen(replace));
    editor->insert_position(pos + strlen(replace));
    editor->show_insert_position();
  }
  else {
    setCommand(cmd_find);
    editor->take_focus();
  }
}

void EditorWidget::save_file(Widget* w, void* eventData)
{
  if (filename[0] == '\0') {
    // no filename - get one!
    wnd->save_file_as();
    return;
  }
  else {
    doSaveFile(filename);
  }
}

void EditorWidget::set_color(Widget* w, void* eventData)
{
  StyleField styleField = (StyleField) (int) eventData;
  if (styleField == st_background || styleField == st_background_def) {
    uchar r,g,b;
    split_color(editor->color(),r,g,b);
    if (color_chooser(w->label(), r,g,b)) {
      Color c = fltk::color(r,g,b);
      set_color_index(fltk::FREE_COLOR + styleField, c);
      setEditorColor(c, styleField == st_background_def);
      editor->styleChanged();
    }
  }
  else {
    setColor(w->label(), styleField);
  }
  wnd->updateConfig(this);
}

void EditorWidget::show_replace(Widget* w, void* eventData)
{
  const char* prime = editor->search;
  if (!prime || !prime[0]) {
    // use selected text when search not available
    prime = editor->textbuf->selection_text();
  }
  commandText->value(prime);
  setCommand(cmd_replace);
}

void EditorWidget::undo(Widget* w, void* eventData)
{
  TextEditor::kf_undo(0, editor);
}

//--Public methods--------------------------------------------------------------

int EditorWidget::handle(int e)
{
  switch (e) {
  case FOCUS:
    fltk::focus(editor);
    handleFileChange();
    return 1;
  case ENTER:
    if (rename_active) {
      // prevent drawing over the inplace editor child control
      return 0;
    }
  }

  return Group::handle(e);
}

bool EditorWidget::readonly()
{
  return ((CodeEditor *) editor)->readonly;
}

void EditorWidget::readonly(bool is_readonly)
{
  if (!is_readonly && access(filename, W_OK) != 0) {
    // cannot set writable since file is readonly
    is_readonly = true;
  }
  modStatus->label(is_readonly ? "RO" : "");
  modStatus->redraw();
  editor->cursor_style(is_readonly ? TextDisplay::DIM_CURSOR :
                       TextDisplay::NORMAL_CURSOR);
  ((CodeEditor *) editor)->readonly = is_readonly;
}

/**
 * copy selection text to the clipboard
 */
void EditorWidget::copyText() {
  if (!tty->copySelection()) {
    TextEditor::kf_copy(0, editor);
  }
}

bool EditorWidget::checkSave(bool discard)
{
  if (!dirty) {
    return true;  // continue next operation
  }

  const char *msg = "The current file has not been saved.\n"
                    "Would you like to save it now?";
  int r = discard ? choice(msg, "Save", "Discard", "Cancel") :
          choice(msg, "Save", "Cancel", 0);
  if (r == 0) {
    save_file();     // Save the file
    return !dirty;
  }
  return (discard && r == 1);
}

void EditorWidget::loadFile(const char *newfile)
{
  TextBuffer *textbuf = editor->textbuf;
  loading = true;
  int r = textbuf->loadfile(newfile);
  if (r) {
    // restore previous
    textbuf->loadfile(filename);
    alert("Error reading from file \'%s\':\n%s.", newfile, strerror(errno));
  }
  else {
    dirty = false;
    strcpy(filename, newfile);
  }

  loading = false;
  textbuf->call_modify_callbacks();
  editor->show_insert_position();
  modifiedTime = getModifiedTime();
  readonly(false);

  FileWidget::forwardSlash(filename);
  wnd->updateEditTabName(this);
  wnd->showEditTab(this);

  statusMsg(filename);
  fileChanged(true);
  setRowCol(1, 1);
}

void EditorWidget::doSaveFile(const char *newfile)
{
  if (!dirty && strcmp(newfile, filename) == 0) {
    // neither buffer or filename have changed
    return;
  }

  char basfile[PATH_MAX];
  TextBuffer *textbuf = editor->textbuf;
  
  strcpy(basfile, newfile);
  if (strchr(basfile, '.') == 0) {
    strcat(basfile, ".bas");
  }
  
  if (textbuf->savefile(basfile)) {
    alert("Error writing to file \'%s\':\n%s.", basfile, strerror(errno));
    return;
  }

  dirty = 0;
  strcpy(filename, basfile);
  modifiedTime = getModifiedTime();

  if (filename[0] == 0) {
    // naming a previously unnamed buffer
    wnd->addHistory(basfile);
  }
  wnd->updateEditTabName(this);
  wnd->showEditTab(this);

  textbuf->call_modify_callbacks();
  statusMsg(filename);
  fileChanged(true);
  editor->take_focus();
}

void EditorWidget::gotoLine(int line)
{
  ((CodeEditor *) editor)->gotoLine(line);
}

void EditorWidget::getRowCol(int *row, int *col)
{
  return ((CodeEditor *) editor)->getRowCol(row, col);
}

void EditorWidget::getSelStartRowCol(int *row, int *col)
{
  return ((CodeEditor *) editor)->getSelStartRowCol(row, col);
}

void EditorWidget::getSelEndRowCol(int *row, int *col)
{
  return ((CodeEditor *) editor)->getSelEndRowCol(row, col);
}

void EditorWidget::saveConfig() {
  FILE *fp = wnd->openConfig(configFile);
  if (fp) {
    char buffer[MAX_PATH];
    int err;
    uchar r,g,b;

    sprintf(buffer, fontConfigSave, getFontName(), getFontSize());
    err = fwrite(buffer, strlen(buffer), 1, fp);

    for (int i = 0; i <= st_background; i++) {
      split_color(i == st_background ? editor->color() : styletable[i].color, r,g,b);
      sprintf(buffer, "%02d=#%02x%02x%02x\n", i, r,g,b);
      err = fwrite(buffer, strlen(buffer), 1, fp);
    }
    
    fclose(fp);
  }
}

/**
 * Saves the selected text to the given file path
 */
void EditorWidget::saveSelection(const char* path) {
  int err;
  FILE *fp = fopen(path, "w");
  if (fp) {
    Rectangle rc;
    char* selection = getSelection(&rc);
    if (selection) {
      err = fwrite(selection, strlen(selection), 1, fp);
      free((void *)selection);
    }
    else {
      // save as an empty file
      fputc(0, fp);
    }
    fclose(fp);
  }
}

void EditorWidget::setFontSize(int size)
{
  int len = sizeof(styletable) / sizeof(styletable[0]);
  for (int i = 0; i < len; i++) {
    styletable[i].size = size;
  }
  editor->styleChanged();
}

int EditorWidget::getFontSize()
{
  return (int)styletable[0].size;
}

void EditorWidget::setIndentLevel(int level)
{
  ((CodeEditor *) editor)->indentLevel = level;
}

void EditorWidget::focusWidget() {
  switch (event_key()) {
  case 'i':
    setCommand(cmd_find_inc);
    break;

  case 'f':
    if (strlen(commandText->value()) > 0 && commandOpt == cmd_find) {
      // continue search - shift -> backward else forward
      command(0, (void*)((event_key_state(LeftShiftKey) ||
                          event_key_state(RightShiftKey)) ? 0 : 1));
    }
    setCommand(cmd_find);
    break;
  }
}

void EditorWidget::statusMsg(const char *msg)
{
  const char *filename = getFilename();
  fileStatus->copy_label(msg && msg[0] ? msg :
                         filename && filename[0] ? filename : UNTITLED_FILE);
  fileStatus->labelcolor(rowStatus->labelcolor());
  fileStatus->redraw();
}

void EditorWidget::updateConfig(EditorWidget* current) {
  setFont(font(current->getFontName()));
  setFontSize(current->getFontSize());
  setEditorColor(current->editor->color(), false);
}

void EditorWidget::setRowCol(int row, int col)
{
  char rowcol[20];
  sprintf(rowcol, "%d", row);
  rowStatus->copy_label(rowcol);
  rowStatus->redraw();
  sprintf(rowcol, "%d", col);
  colStatus->copy_label(rowcol);
  colStatus->redraw();
}

void EditorWidget::runMsg(RunMessage runMessage)
{
  const char* msg = 0;
  switch (runMessage) {
  case msg_err:
    fileStatus->labelcolor(RED);
    msg = "ERR";
    break;
  case msg_run:
    fileStatus->labelcolor(rowStatus->labelcolor());
    msg = "RUN";
    break;
  default:
    msg = "";
  }
  runStatus->copy_label(msg);
  runStatus->redraw();
}

void EditorWidget::fileChanged(bool loadfile)
{
  FILE *fp;

  funcList->clear();
  funcList->begin();
  if (loadfile) {
    // update the func/sub navigator
    createFuncList();
    funcList->redraw();

    const char *filename = getFilename();
    if (filename && filename[0]) {
      // update the last used file menu
      bool found = false;

      for (int i = 0; i < NUM_RECENT_ITEMS; i++) {
        if (strcmp(filename, recentPath[i].toString()) == 0) {
          found = true;
          break;
        }
      }

      if (found == false) {
        // shift items downwards
        for (int i = NUM_RECENT_ITEMS - 1; i > 0; i--) {
          recentMenu[i]->copy_label(recentMenu[i - 1]->label());
          recentPath[i].empty();
          recentPath[i].append(recentPath[i - 1]);
        }
        // create new item in first position
        char *c = strrchr(filename, '/');
        if (c == 0) {
          c = strrchr(filename, '\\');
        }
        recentPath[0].empty();
        recentPath[0].append(filename);
        recentMenu[0]->copy_label(c ? c + 1 : filename);
      }
    }
  }
  else {
    // empty the last edited file
    char path[MAX_PATH];
    getHomeDir(path);
    strcat(path, LASTEDIT_FILE);
    fp = fopen(path, "w");
    if (!fwrite("\n", 1, 1, fp)) {
      // write error
    }
    fclose(fp);
  }

  new Item(SCAN_LABEL);
  funcList->end();
}

void EditorWidget::restoreEdit()
{
  FILE *fp;
  char path[MAX_PATH];

  // continue editing the previous file
  getHomeDir(path);
  strcat(path, LASTEDIT_FILE);
  fp = fopen(path, "r");
  if (fp) {
    if (fgets(path, sizeof(path), fp)) {
      path[strlen(path) - 1] = 0; // trim new-line
    }
    fclose(fp);
    if (access(path, 0) == 0) {
      loadFile(path);
      return;
    }
  }

  // continue editing scratch buffer
  getHomeDir(path);
  strcat(path, UNTITLED_FILE);
  if (access(path, 0) == 0) {
    loadFile(path);
  }
}

//--Protected methods-----------------------------------------------------------

void EditorWidget::createFuncList()
{
  TextBuffer *textbuf = editor->textbuf;
  const char *text = textbuf->text();
  int len = textbuf->length();
  int curLine = 0;

  for (int i = 0; i < len; i++) {
    if (text[i] == '\n' || i == 0) {
      curLine++;
    }

    // skip ahead to the next line when comments found
    if (text[i] == '#' || text[i] == '\'' || 
        strncasecmp(text + i, "rem", 3) == 0) {
      while (i < len && text[i] != '\n') {
        i++;
      }
      curLine++;
    }

    // avoid seeing "gosub" etc
    int offs = ((strncasecmp(text + i, "\nsub ", 5) == 0 ||
                 strncasecmp(text + i, " sub ", 5) == 0) ? 4 :
                (strncasecmp(text + i, "\nfunc ", 6) == 0 ||
                 strncasecmp(text + i, " func ", 6) == 0) ? 5 : 0);
    if (offs != 0) {
      char *c = strchr(text + i + offs, '\n');
      if (c) {
        if (text[i] == '\n') {
          i++;    // skip initial newline
        }
        int itemLen = c - (text + i);
        String s(text + i, itemLen);
        Item *item = new Item();
        item->copy_label(s.toString());
        item->user_data((void*) curLine);
        i += itemLen;
        curLine++;
      }
    }
  }
}

void EditorWidget::doChange(int inserted, int deleted)
{
  if (loading) {
    return;  // do nothing while file load in progress
  }

  if (inserted || deleted) {
    dirty = 1;
  }

  setModified(dirty);
}

void EditorWidget::findFunc(const char *find)
{
  const char *text = editor->textbuf->text();
  int findLen = strlen(find);
  int len = editor->textbuf->length();
  int lineNo = 1;
  for (int i = 0; i < len; i++) {
    if (strncasecmp(text + i, find, findLen) == 0) {
      gotoLine(lineNo);
      break;
    }
    else if (text[i] == '\n') {
      lineNo++;
    }
  }
}

char* EditorWidget::getSelection(Rectangle* rc)
{
  return ((CodeEditor *) editor)->getSelection(rc);
}

const char* EditorWidget::getFontName()
{
  return styletable[0].font->name();
}

void EditorWidget::getKeywords(strlib::List& keywords) {
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

U32 EditorWidget::getModifiedTime() {
  struct stat st_file;
  U32 modified = 0;
  if (filename[0] && !stat(filename, &st_file)) {
    modified = st_file.st_mtime;
  }
  return modified;
}

void EditorWidget::handleFileChange() {
  // handle outside changes to the file
  if (filename[0] && modifiedTime != 0 &&
      modifiedTime != getModifiedTime()) {
    const char *msg = "File %s\nhas changed on disk.\n\n"
      "Do you want to reload the file?";
    if (ask(msg, filename)) {
      reloadFile();
    }
    else {
      modifiedTime = 0;
    }
  }
}

/**
 * load any stored font or color settings
 */
void EditorWidget::loadConfig() {
  FILE *fp = wnd->openConfig(configFile, "r");
  if (fp) {
    char buffer[MAX_PATH];
    int size = 0;
    int i = 0;

    if (fscanf(fp, fontConfigRead, buffer, &size) == 2) {
      setFont(font(buffer));
      setFontSize(size);
    }

    while (feof(fp) == 0 && fgets(buffer, sizeof(buffer), fp)) {
      buffer[strlen(buffer) - 1] = 0; // trim new-line
      Color c = fltk::color(buffer + 3); // skip nn=#xxxxxx
      if (c != NO_COLOR) {
        if (i == st_background) {
          setEditorColor(c, false);
          break; // found final StyleField element
        }
        else {
          styletable[i].color = c;
        }
      }
      i++;
    }

    fclose(fp);
  }
}

void EditorWidget::newFile()
{
  if (readonly()) {
    return;
  }

  if (!checkSave(true)) {
    return;
  }

  TextBuffer *textbuf = editor->textbuf;
  filename[0] = '\0';
  textbuf->select(0, textbuf->length());
  textbuf->remove_selection();
  dirty = 0;
  textbuf->call_modify_callbacks();
  statusMsg(0);
  fileChanged(false);
  modifiedTime = 0;
}

void EditorWidget::reloadFile() {
  char buffer[PATH_MAX];
  strcpy(buffer, filename);
  loadFile(buffer);
}

int EditorWidget::replaceAll(const char* find, const char* replace, 
                             bool restorePos, bool matchWord)
{
  int times = 0;

  if (strcmp(find, replace) != 0) {
    TextBuffer *textbuf = editor->textbuf;
    int prevPos = editor->insert_position();
    
    // loop through the whole string
    int pos = 0;
    editor->insert_position(pos);
    
    while (textbuf->search_forward(pos, find, &pos)) {
      // found a match; update the position and replace text
      if (!matchWord ||
          ((pos == 0 || !isvar(textbuf->character(pos - 1))) &&
            !isvar(textbuf->character(pos + strlen(find))))) {
        textbuf->select(pos, pos + strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);
      }

      // advance beyond replace string
      pos += strlen(replace);
      editor->insert_position(pos);
      times++;
    }
    
    if (restorePos) {
      editor->insert_position(prevPos);
    }
    editor->show_insert_position();
  }

  return times;
}

bool EditorWidget::searchBackward(const char *text, int startPos,
                                  const char *find, int findLen, int *foundPos)
{
  int matchIndex = findLen - 1;
  for (int i = startPos; i >= 0; i--) {
    bool equals = toupper(text[i]) == toupper(find[matchIndex]);
    if (equals == false && matchIndex < findLen - 1) {
      // partial match now fails - reset search at current index
      matchIndex = findLen - 1;
      equals = toupper(text[i]) == toupper(find[matchIndex]);
    }
    matchIndex = (equals ? matchIndex - 1 : findLen - 1);
    if (matchIndex == -1 && (i == 0 || isalpha(text[i - 1]) == 0)) {
      // char prior to word is non-alpha
      *foundPos = i;
      return true;
    }
  }
  return false;
}

void EditorWidget::setColor(const char* label, StyleField field) {
  uchar r,g,b;
  split_color(styletable[field].color,r,g,b);
  if (color_chooser(label, r,g,b)) {
    Color c = fltk::color(r,g,b);
    set_color_index(fltk::FREE_COLOR + field, c);
    styletable[field].color = c;
    editor->styleChanged();
  }
} 

void EditorWidget::setCommand(CommandOpt command) {
  commandOpt = command;
  commandChoice->value(command);
  commandText->textcolor(commandChoice->textcolor());
  commandText->redraw();
  commandText->take_focus();
  commandText->when(commandOpt == cmd_find_inc ? 
                    WHEN_CHANGED : WHEN_ENTER_KEY_ALWAYS);
}

/**
 * Sets the editor and editor toolbar color
 */
void EditorWidget::setEditorColor(Color c, bool defColor) {
  editor->color(c);

  Color bg = lerp(c, BLACK, .1f); // same offset as editor line numbers
  Color fg = contrast(c, bg);
  int i;
  Widget* child;

  // set the colours on the command text bar
  for (i = commandText->parent()->children(); i > 0; i--) {
    child = commandText->parent()->child(i - 1);
    child->color(bg);
    child->textcolor(fg);
    child->redraw();
  }
  // set the colours on the status bar
  for (i = fileStatus->parent()->children(); i > 0; i--) {
    child = fileStatus->parent()->child(i - 1);
    child->color(bg);
    child->labelcolor(fg);
    child->redraw();
  }
  if (defColor) {
    // contrast the default colours against the background
    for (i = 0; i < st_background; i++) {
      styletable[i].color = contrast(defaultColor[i], c);
    }
  }
}

void EditorWidget::setFont(Font* font)
{
  if (font) {
    int len = sizeof(styletable) / sizeof(styletable[0]);
    for (int i = 0; i < len; i++) {
      styletable[i].font = font;
    }
    editor->styleChanged();
  }
}

void EditorWidget::setModified(bool dirty)
{
  this->dirty = dirty;
  modStatus->label(dirty ? "MOD" : "");
  modStatus->redraw();
}

void EditorWidget::showFindText(const char *text) {
  editor->showFindText(text);
}

//--EndOfFile-------------------------------------------------------------------
