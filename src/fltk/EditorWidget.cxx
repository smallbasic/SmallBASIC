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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include <fltk/Button.h>
#include <fltk/Choice.h>
#include <fltk/Flags.h>
#include <fltk/Input.h>
#include <fltk/Item.h>
#include <fltk/MenuBar.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextBuffer.h>
#include <fltk/TextEditor.h>
#include <fltk/ask.h>
#include <fltk/damage.h>
#include <fltk/events.h>
#include <fltk/file_chooser.h>

#include "MainWindow.h"
#include "EditorWidget.h"
#include "kwp.h"

using namespace fltk;

// in MainWindow
extern char path[MAX_PATH];
extern String recentPath[];
extern Widget* recentMenu[];

// in dev_fltk.cpp
void getHomeDir(char *filename);

TextDisplay::StyleTableEntry styletable[] = { // Style table
  { BLACK, COURIER, 12},                  // A - Plain
  { color(0, 128, 0), COURIER, 12},       // B - Comments
  { color(0, 0, 192), COURIER, 12},       // C - Strings
  { color(192, 0, 0), COURIER, 12},       // D - code_keywords
  { color(128, 128, 0), COURIER, 12},     // E - code_functions
  { color(0, 128, 128), COURIER, 12},     // F - code_procedures
  { color(128, 0, 128), COURIER, 12},     // G - Find matches
  { color(0, 128, 0), COURIER_ITALIC, 12},// H - Italic Comments ';
  { color(0, 128, 128), COURIER, 12},     // I - Numbers
  { color(128, 128, 64), COURIER, 12},    // J - Operators
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

EditorWidget* get_editor() {
  return wnd->getEditor();
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

CodeEditor::CodeEditor(int x, int y, int w, int h) : TextEditor(x, y, w, h) {
  readonly = false;
  const char *s = getenv("INDENT_LEVEL");
  indentLevel = (s && s[0] ? atoi(s) : 2);
  matchingBrace = -1;

  textbuf = new TextBuffer();
  buffer(textbuf);
  stylebuf = new TextBuffer();
  search[0] = 0;
  highlight_data(stylebuf, styletable,
                 sizeof(styletable) / sizeof(styletable[0]),
                 PLAIN, style_unfinished_cb, 0);
}

CodeEditor::~CodeEditor() {
  // cleanup buffers
  delete textbuf;
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
    last = isalnum(*text) || *text == '.' || *text == '_';

    if (*text == '\n') {
      current = PLAIN;          // basic lines do not continue
    }
  }
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

#if defined(WIN32)
  // in windows these message are sent here instead of EditorWidget
  switch (e) {  
  case DND_ENTER:
  case DND_DRAG:
  case DND_RELEASE:
  case DND_LEAVE:
    return 1;
  case PASTE:
    parent()->handle(e);
    return 1;
  }
#endif

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
  ((EditorWidget*) parent())->setRowCol(row, col + 1);
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
  ((EditorWidget*) parent())->setRowCol(line, 1);
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

void CodeEditor::getRowCol(int *row, int *col)
{
  position_to_linecol(cursor_pos_, row, col);
}

bool CodeEditor::findText(const char *find, bool forward)
{
  // copy lowercase search string for high-lighting
  strcpy(search, find);
  int findLen = strlen(search);

  for (int i = 0; i < findLen; i++) {
    search[i] = tolower(search[i]);
  }

  style_update_cb(0, textbuf->length(), textbuf->length(), 0, 0, this);
  if (find == 0 || find[0] == 0) {
    return 0;
  }

  int pos = insert_position();
  bool found = forward ? textbuf->search_forward(pos, search, &pos) :
               textbuf->search_backward(pos - strlen(find), search, &pos);
  if (found) {
    textbuf->select(pos, pos + strlen(search));
    insert_position(pos + strlen(search));
    show_insert_position();
  }
  return found;
}

//--EditorWidget----------------------------------------------------------------

EditorWidget::EditorWidget(int x, int y, int w, int h) : Group(x, y, w, h)
{
  int tbHeight = 26; // toolbar height
  int stHeight = MNU_HEIGHT;

  filename[0] = 0;
  dirty = false;
  loading = false;
  modifiedTime = 0;
  box(NO_BOX);

  begin();
  editor = new CodeEditor(0, 0, w, h - (tbHeight + stHeight + 8));
  editor->linenumber_width(40);
  editor->wrap_mode(true, 0);
  editor->selection_color(fltk::color(190, 189, 188));
  editor->color(WHITE);
  editor->textbuf->add_modify_callback(style_update_cb, editor);
  editor->textbuf->add_modify_callback(changed_cb, this);
  editor->box(NO_BOX);
  editor->take_focus();

  // create the editor toolbar
  w -= 4;
  Group *toolbar = new Group(2, editor->b() + 2, w, tbHeight);
  toolbar->begin();
  toolbar->box(FLAT_BOX);

  // find control
  findTextInput = new Input(38, 2, 120, MNU_HEIGHT, "Find:");
  findTextInput->align(ALIGN_LEFT | ALIGN_CLIP);
  Button *prevBn = new Button(160, 4, 18, MNU_HEIGHT - 4, "@-98>;");
  Button *nextBn = new Button(180, 4, 18, MNU_HEIGHT - 4, "@-92>;");
  prevBn->callback(EditorWidget::find_cb, (void *)0);
  nextBn->callback(EditorWidget::find_cb, (void *)1);
  findTextInput->callback(EditorWidget::find_cb, (void *)2);
  findTextInput->when(WHEN_ENTER_KEY_ALWAYS);
  findTextInput->labelfont(HELVETICA);

  // goto-line control
  gotoLineInput = new Input(238, 2, 40, MNU_HEIGHT, "Goto:");
  gotoLineInput->align(ALIGN_LEFT | ALIGN_CLIP);
  Button *gotoBn = new Button(280, 4, 18, MNU_HEIGHT - 4, "@-92>;");
  gotoBn->callback(EditorWidget::goto_line_cb, gotoLineInput);
  gotoLineInput->callback(EditorWidget::goto_line_cb, gotoLineInput);
  gotoLineInput->when(WHEN_ENTER_KEY_ALWAYS);
  gotoLineInput->labelfont(HELVETICA);

  // sub-func jump droplist
  funcList = new Choice(309, 2, 168, MNU_HEIGHT);
  funcList->callback(func_list_cb, 0);
  funcList->labelfont(COURIER);
  funcList->begin();
  new Item();
  new Item(SCAN_LABEL);
  funcList->end();

  // close the tool-bar with a resizeable end-box
  Group *boxEnd = new Group(1000, 4, 0, 0);
  toolbar->resizable(boxEnd);
  toolbar->end();

  // editor status bar
  Group *statusBar = new Group(2, toolbar->b() + 2, w, MNU_HEIGHT);
  statusBar->begin();
  statusBar->box(NO_BOX);
  fileStatus = new Widget(0, 0, w - 137, MNU_HEIGHT - 2);
  modStatus = new Widget(w - 136, 0, 33, MNU_HEIGHT - 2);
  runStatus = new Widget(w - 102, 0, 33, MNU_HEIGHT - 2);
  rowStatus = new Widget(w - 68, 0, 33, MNU_HEIGHT - 2);
  colStatus = new Widget(w - 34, 0, 33, MNU_HEIGHT - 2);

  for (int n = 0; n < statusBar->children(); n++) {
    Widget *w = statusBar->child(n);
    w->labelfont(HELVETICA);
    w->box(BORDER_BOX);
    w->color(color());
  }

  fileStatus->align(ALIGN_INSIDE_LEFT | ALIGN_CLIP);
  statusBar->resizable(fileStatus);
  statusBar->end();

  resizable(editor);
  end();
}

EditorWidget::~EditorWidget()
{
  editor->textbuf->remove_modify_callback(style_update_cb, editor);
  editor->textbuf->remove_modify_callback(changed_cb, this);
}

int EditorWidget::handle(int e)
{
  char buffer[PATH_MAX];

  switch (e) {
  case FOCUS:
    fltk::focus(editor);
    handleFileChange();
    return 1;
  case DND_ENTER:
  case DND_DRAG:
  case DND_RELEASE:
  case DND_LEAVE:
    return 1;
  case PASTE:
    strncpy(buffer, fltk::event_text(), fltk::event_length());
    buffer[fltk::event_length()] = 0;
    loadFile(buffer);
    return 1;
  }

  return Group::handle(e);
}

bool EditorWidget::readonly()
{
  return ((CodeEditor *) editor)->readonly;
}

void EditorWidget::readonly(bool is_readonly)
{
  editor->cursor_style(is_readonly ? TextDisplay::DIM_CURSOR :
                       TextDisplay::NORMAL_CURSOR);
  ((CodeEditor *) editor)->readonly = is_readonly;
}

void EditorWidget::doChange(int inserted, int deleted)
{
  if (loading) {
    return;                     // do nothing while file load in progress
  }

  if (inserted || deleted) {
    dirty = 1;
  }

  setModified(dirty);
}

bool EditorWidget::checkSave(bool discard)
{
  if (!dirty) {
    return true;                // continue next operation
  }

  const char *msg = "The current file has not been saved.\n"
                    "Would you like to save it now?";
  int r = discard ? choice(msg, "Save", "Discard", "Cancel") : 
          choice(msg, "Save", "Cancel", 0);
  if (r == 0) {
    saveFile();                 // Save the file
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
  statusMsg(filename);
  fileChanged(true);
  textbuf->call_modify_callbacks();
  editor->show_insert_position();
  setRowCol(1, 1);
  modifiedTime = getModifiedTime();
}

void EditorWidget::reloadFile() {
  char buffer[PATH_MAX];
  strcpy(buffer, filename);
  loadFile(buffer);
}

void EditorWidget::doSaveFile(const char *newfile)
{
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

void EditorWidget::showFindReplace(void* eventData)
{
  wnd->replaceFind->value(editor->search);
  wnd->replaceDlg->show();
}

void EditorWidget::replaceNext(void* eventData)
{
  if (readonly()) {
    return;
  }

  const char *find = wnd->replaceFind->value();
  const char *replace = wnd->replaceWith->value();

  if (find[0] == '\0') {
    // search string is blank; get a new one
    wnd->replaceDlg->show();
    return;
  }

  wnd->replaceDlg->hide();

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
    alert("No occurrences of \'%s\' found!", find);
  }
}

void EditorWidget::cancelReplace(void* eventData)
{
  wnd->replaceDlg->hide();
}

void EditorWidget::doDelete(void* eventData)
{
  editor->textbuf->remove_selection();
}

void EditorWidget::find(void* eventData)
{
  bool found = editor->findText(findTextInput->value(), (int)eventData);
  findTextInput->textcolor(found ? BLACK : RED);
  findTextInput->redraw();
  if (2 == (int)eventData) {
    take_focus();
  }
}

void EditorWidget::func_list(void* eventData)
{
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
      findFunc(label);
      take_focus();
    }
  }
}

void EditorWidget::goto_line(void* eventData)
{
  gotoLine(atoi(gotoLineInput->value()));
  take_focus();
}

void EditorWidget::newFile(void* eventData)
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

void EditorWidget::replaceAll(void* eventData)
{
  if (readonly()) {
    return;
  }

  const char *find = wnd->replaceFind->value();
  const char *replace = wnd->replaceWith->value();

  find = wnd->replaceFind->value();
  if (find[0] == '\0') {
    // search string is blank; get a new one
    wnd->replaceDlg->show();
    return;
  }

  wnd->replaceDlg->hide();
  editor->insert_position(0);
  int times = 0;

  // loop through the whole string
  for (int found = 1; found;) {
    int pos = editor->insert_position();
    TextBuffer *textbuf = editor->textbuf;

    found = textbuf->search_forward(pos, find, &pos);

    if (found) {
      // found a match; update the position and replace text
      textbuf->select(pos, pos + strlen(find));
      textbuf->remove_selection();
      textbuf->insert(pos, replace);
      editor->insert_position(pos + strlen(replace));
      editor->show_insert_position();
      times++;
    }
  }

  if (times) {
    message("Replaced %d occurrences.", times);
  }
  else {
    alert("No occurrences of \'%s\' found!", find);
  }
}

void EditorWidget::saveFile(void* eventData)
{
  if (filename[0] == '\0') {
    // no filename - get one!
    saveFileAs();
    return;
  }
  else {
    doSaveFile(filename);
  }
}

void EditorWidget::saveFileAs(void* eventData)
{
  const char *msg = "%s\n\nFile already exists.\nDo you want to replace it?";
  const char *newfile = file_chooser("Save File As?", "*.bas", filename);
  if (newfile != NULL) {
    if (access(newfile, 0) == 0 && ask(msg, newfile) == 0) {
      return;
    }
    doSaveFile(newfile);
  }
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

void EditorWidget::setFontSize(int size)
{
  int len = sizeof(styletable) / sizeof(styletable[0]);
  TextBuffer *textbuf = editor->textbuf;

  for (int i = 0; i < len; i++) {
    styletable[i].size = size;
  }
  textbuf->select(0, textbuf->length());
  textbuf->select(0, 0);
  editor->redraw(DAMAGE_ALL);
}

int EditorWidget::getFontSize()
{
  return (int)styletable[0].size;
}

void EditorWidget::createFuncList()
{
  TextBuffer *textbuf = editor->textbuf;
  const char *text = textbuf->text();
  int len = textbuf->length();

  for (int i = 0; i < len; i++) {
    // avoid seeing "gosub" etc
    int offs = ((strncasecmp(text + i, "\nsub ", 5) == 0 ||
                 strncasecmp(text + i, " sub ", 5) == 0) ? 4 :
                (strncasecmp(text + i, "\nfunc ", 6) == 0 ||
                 strncasecmp(text + i, " func ", 6) == 0) ? 5 : 0);
    if (offs != 0) {
      char *c = strchr(text + i + offs, '\n');
      if (c) {
        if (text[i] == '\n') {
          i++;                  // skip initial newline
        }
        int itemLen = c - (text + i);
        String s(text + i, itemLen);
        Item *item = new Item();
        item->copy_label(s.toString());
        i += itemLen;
      }
    }
  }
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

void EditorWidget::setIndentLevel(int level)
{
  ((CodeEditor *) editor)->indentLevel = level;
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

void EditorWidget::focusWidget() {
  switch (event_key()) {
  case 'f':
    findTextInput->take_focus();
    break;
  case 'g':
    gotoLineInput->take_focus();
    break;
  case 'h':
    funcList->take_focus();
    break;
  }
}

void EditorWidget::setModified(bool dirty)
{
  modStatus->label(dirty ? "MOD" : "");
  modStatus->redraw();
}

void EditorWidget::statusMsg(const char *msg)
{
  const char *filename = getFilename();
  fileStatus->copy_label(msg && msg[0] ? msg :
                         filename && filename[0] ? filename : UNTITLED_FILE);
  fileStatus->labelcolor(rowStatus->labelcolor());
  fileStatus->redraw();
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
    fileStatus->labelcolor(BLACK);
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
    getHomeDir(path);
    strcat(path, LASTEDIT_FILE);
    fp = fopen(path, "w");
    fwrite("\n", 1, 1, fp);
    fclose(fp);
  }

  new Item(SCAN_LABEL);
  funcList->end();
}

void EditorWidget::restoreEdit()
{
  FILE *fp;

  // continue editing the previous file
  getHomeDir(path);
  strcat(path, LASTEDIT_FILE);
  fp = fopen(path, "r");
  if (fp) {
    fgets(path, sizeof(path), fp);
    fclose(fp);
    path[strlen(path) - 1] = 0; // trim new-line
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

//--EndOfFile-------------------------------------------------------------------
