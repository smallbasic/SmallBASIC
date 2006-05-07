// -*- c-file-style: "java" -*-
// $Id: EditorWindow.cpp,v 1.57 2006-05-07 05:18:28 zeeb90au Exp $
//
// Based on test/editor.cxx - A simple text editor program for the Fast 
// Light Tool Kit (FLTK). This program is described in Chapter 4 of the FLTK 
// Programmer's Guide.
// Copyright 1998-2003 by Bill Spitzak and others.
//
// Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
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

#include <fltk/Button.h>
#include <fltk/Choice.h>
#include <fltk/FileChooser.h>
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

#include "EditorWindow.h"
#include "MainWindow.h"
#include "kwp.cxx"

#if defined(WIN32) 
#include <fltk/win32.h>
#define restoreFocus() ::SetFocus(xid(Window::first())); redraw()
#else
#define restoreFocus() take_focus()
#endif

using namespace fltk;

TextDisplay::StyleTableEntry styletable[] = { // Style table
    { BLACK,            COURIER, 12 }, // A - Plain
    { color(0,128,0),   COURIER, 12 }, // B - Comments
    { color(0,0,192),   COURIER, 12 }, // C - Strings
    { color(192,0,0),   COURIER, 12 }, // D - code_keywords
    { color(128,128,0), COURIER, 12 }, // E - code_functions
    { color(0,128,128), COURIER, 12 }, // F - code_procedures
    { color(128,0,128), COURIER, 12 }, // G - Find matches
    { color(0,128,0),   COURIER_ITALIC, 12 }, // H - Italic Comments ';
    { color(0,128,128), COURIER, 12 }, // I - Numbers
    { color(128,128,64),COURIER, 12 }, // J - Operators
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

TextBuffer *stylebuf = 0;
TextBuffer *textbuf = 0;
char search[256];

const int numCodeKeywords = sizeof(code_keywords) / sizeof(code_keywords[0]);
const int numCodeFunctions = sizeof(code_functions) / sizeof(code_functions[0]);
const int numCodeProcedures = sizeof(code_procedures) / sizeof(code_procedures[0]);

// 'compare_keywords()' - Compare two keywords
int compare_keywords(const void *a, const void *b) {
    return (strcasecmp(*((const char **)a), *((const char **)b)));
}

// 'style_parse()' - Parse text and produce style data.
void style_parse(const char *text, char *style, int length) {
    char current = PLAIN;
    int  last = 0; // prev char was alpha-num
    char  buf[255];
    char *bufptr;
    const char *temp;
    int searchLen = strlen(search);

    for (;length > 0; length--, text++) {
        if (current == PLAIN) {
            // check for directives, comments, strings, and keywords
            if ((*text == '#' && (*(text-1) == 0 || *(text-1) == 10)) ||
                (strncasecmp(text, "rem", 3) == 0 && text[3] == ' ') ||
                *text == '\'') {
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
                    if (*text == '0' && *(text+1) == 'x') {
                        // hex number
                        *style++ = DIGITS;
                        text++;
                        length--;
                    }
                    while (*text && (*(text+1)=='.' || isdigit(*(text+1)))) {
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
                    const char* sfind = strstr(bufptr, search);
                    // find text match
                    if (sfind != 0) {
                        int offset = sfind-bufptr;
                        style += offset;
                        text += offset;
                        length -= offset;
                        for (int i=0; i<searchLen && text < temp; i++) {
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
                } else if (bsearch(&bufptr, code_functions, numCodeFunctions,
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
                } else if (bsearch(&bufptr, code_procedures, numCodeProcedures,
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
        last = isalnum(*text) || *text == '.' || *text == '_';

        if (*text == '\n') {
            current = PLAIN; // basic lines do not continue
        }
    }
}

// 'style_init()' - Initialize the style buffer
void style_init(void) {
    char *style = new char[textbuf->length() + 1];
    const char *text = textbuf->text();

    memset(style, PLAIN, textbuf->length());
    style[textbuf->length()] = '\0';

    if (!stylebuf) {
        stylebuf = new TextBuffer(textbuf->length());
    }

    style_parse(text, style, textbuf->length());
    stylebuf->text(style);

    free((void*)text);
    delete[] style;
}

// 'style_unfinished_cb()' - Update unfinished styles.
void style_unfinished_cb(int, void *) {}

char* get_style_range(int start, int end) {
    const char* s = stylebuf->text_range(start, end);
    char *style = new char[strlen(s) + 1];
    strcpy(style, s);
    free((void*)s);
    return style;
}

// 'style_update()' - Update the style buffer
void style_update(int pos,        // I - Position of update
                  int nInserted,  // I - Number of inserted chars
                  int nDeleted,   // I - Number of deleted chars
                  int /*nRestyled*/,  // I - Number of restyled chars
                  const char * /*deletedText*/,// I - Text that was deleted
                  void *cbArg) {   // I - Callback data

    int start;              // Start of text
    int end;                // End of text
    char  last;             // Last style on line
    const char *text;       // Text data
    char *style;            // Text data

    TextEditor* editor = (TextEditor*)cbArg;

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
    } else {
        // just delete characters in the style buffer
        stylebuf->remove(pos, pos + nDeleted);
    }

    // Select the area that was just updated to avoid unnecessary callbacks
    stylebuf->select(pos, pos + nInserted - nDeleted);

    // re-parse the changed region; we do this by parsing from the
    // beginning of the line of the changed region to the end of
    // the line of the changed region  Then we check the last
    // style character and keep updating if we have a multi-line
    // comment character
    start = textbuf->line_start(pos);
    end   = textbuf->line_end(pos + nInserted);
    text  = textbuf->text_range(start, end);
    style = get_style_range(start, end);
    last  = style[end - start - 1];

    style_parse(text, style, end - start);
    stylebuf->replace(start, end, style);
    editor->redisplay_range(start, end);

    if (last != style[end - start - 1]) {
        // the last character on the line changed styles, 
        // so reparse the remainder of the buffer
        delete[] style;
        free((void*)text);

        end   = textbuf->length();
        text  = textbuf->text_range(start, end);
        style = get_style_range(start, end);
        style_parse(text, style, end - start);
        stylebuf->replace(start, end, style);
        editor->redisplay_range(start, end);
    }

    free((void*)text);
    delete[] style;
}

//--CodeEditor------------------------------------------------------------------

struct CodeEditor : public TextEditor {
    CodeEditor(int x, int y, int w, int h) : TextEditor(x, y, w, h) {
        readonly = false;
        const char* s = getenv("INDENT_LEVEL");
        indentLevel = (s && s[0] ? atoi(s) : 2);
        matchingBrace = -1;
    }

    int handle(int e);
    void gotoLine(int line);
    void getSelStartRowCol(int *row, int *col);
    void getSelEndRowCol(int *row, int *col);
    unsigned getIndent(char* indent, int len, int pos);
    void handleTab();
    void showRowCol();
    void getRowCol(int *row, int *col);
    void showMatchingBrace();
    void draw();

    bool readonly;
    int indentLevel;
    int matchingBrace;
};

void CodeEditor::draw() {
    TextEditor::draw();
    if (matchingBrace != -1) {
        // highlight the matching brace
        int X,Y;
        int cursor = cursor_style_;
        cursor_style_ = BLOCK_CURSOR;
        if (position_to_xy(matchingBrace, &X, &Y)) {
            draw_cursor(X, Y);
        }
        cursor_style_ = cursor;
    }
}

unsigned CodeEditor::getIndent(char* spaces, int len, int pos) {
    // count the indent level and find the start of text
    const char *buf = buffer()->line_text(pos);
    int i = 0;
    while (buf && buf[i] == ' ' && i < len) {
        spaces[i] = buf[i];
        i++;
    }
    
    if (strncasecmp(buf+i, "while ", 6) == 0 ||
        strncasecmp(buf+i, "if ", 3) == 0 ||
        strncasecmp(buf+i, "elseif ", 7) == 0 ||
        strncasecmp(buf+i, "elif ", 5) == 0 ||
        strncasecmp(buf+i, "else", 4) == 0 ||
        strncasecmp(buf+i, "repeat", 6) == 0 ||
        strncasecmp(buf+i, "for ", 4) == 0 ||
        strncasecmp(buf+i, "select ", 7) == 0 ||
        strncasecmp(buf+i, "case ", 5) == 0 ||
        strncasecmp(buf+i, "sub ", 4) == 0 ||
        strncasecmp(buf+i, "func ", 5) == 0) {

        // handle if-then-blah on same line
        if (strncasecmp(buf+i, "if ", 3) == 0) {
            // find the end of line index
            int j=i+4;
            while (buf[j] != 0 && buf[j] != '\n') {
                // line also 'ends' at start of comments
                if (strncasecmp(buf+j, "rem", 3) == 0 ||
                    buf[j] == '\'') {
                    break;
                }
                j++;
            }
            // right trim trailing spaces
            while (buf[j-1] == ' ' && j > i) {
                j--;
            }
            if (strncasecmp(buf+j-4, "then", 4) != 0) {
                // 'then' is not final text on line
                spaces[i] = 0;
                return i;
            }
        }

        // indent new line
        for (int j=0; j<indentLevel; j++, i++) {
            spaces[i] = ' ';
        }
    }
    spaces[i] = 0;
    free((void*)buf);
    return i;
}

void CodeEditor::handleTab() {
    char spaces[250];
    int indent;

    // get the desired indent based on the previous line
    int lineStart = buffer()->line_start(cursor_pos_);
    int prevLineStart = buffer()->line_start(lineStart-1);

    if (prevLineStart && prevLineStart+1 == lineStart) {
        // allows for a single blank line between statments
        prevLineStart = buffer()->line_start(prevLineStart-1);
    }

    // note - spaces not used in this context
    indent = prevLineStart == 0 ? 0 : 
        getIndent(spaces, sizeof(spaces), prevLineStart);
    
    // get the current lines indent
    const char *buf = buffer()->line_text(lineStart);
    int curIndent = 0;
    while (buf && buf[curIndent] == ' ') {
        curIndent++;
    }
    
    // adjust indent for closure statements
    if (strncasecmp(buf+curIndent, "wend", 4) == 0 ||
        strncasecmp(buf+curIndent, "fi", 2) == 0 ||
        strncasecmp(buf+curIndent, "endif", 5) == 0 ||
        strncasecmp(buf+curIndent, "elseif ", 7) == 0 ||
        strncasecmp(buf+curIndent, "elif ", 5) == 0 ||
        strncasecmp(buf+curIndent, "else", 4) == 0 ||
        strncasecmp(buf+curIndent, "next", 4) == 0 ||
        strncasecmp(buf+curIndent, "case", 4) == 0 ||
        strncasecmp(buf+curIndent, "end", 3) == 0  ||
        strncasecmp(buf+curIndent, "until ", 6) == 0) {
        if (indent >= indentLevel) {
            indent -= indentLevel;
        }
    }
    if (curIndent < indent) {
        // insert additional spaces
        int len = indent-curIndent;
        if (len > (int)sizeof(spaces)-1) {
            len = (int)sizeof(spaces)-1;
        }
        memset(spaces, ' ', len);
        spaces[len] = 0;
        buffer()->insert(lineStart, spaces);
        if (cursor_pos_-lineStart < indent) {
            // jump cursor to start of text
            cursor_pos_ = lineStart + indent;
        } else {
            // move cursor along with text movement, staying on same line
            int maxpos = buffer()->line_end(lineStart);
            if (cursor_pos_ + len <= maxpos) {
                cursor_pos_ += len;
            }
        }
    } else if (curIndent > indent) {
        // remove excess spaces
        buffer()->remove(lineStart, lineStart+(curIndent-indent));
    } else {
        // already have ideal indent - soft-tab to indent
        insert_position(lineStart + indent);
    }
    free((void*)buf);
}

void CodeEditor::showMatchingBrace() {
    char cursorChar = buffer()->character(cursor_pos_-1);
    char cursorMatch=0;
    int pair = -1;
    int iter = -1;
    int pos = cursor_pos_-2;

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
                level++; // nested char
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
    
int CodeEditor::handle(int e) {
    int cursorPos = cursor_pos_;
    char spaces[250];
    int indent;

    if (readonly && (e == KEY || e == PASTE)) {
        return 0;
    }

    if (e == KEY && event_key() == TabKey) {
        if (event_key_state(LeftCtrlKey) ||
            event_key_state(RightCtrlKey)) {
            // pass ctrl+key to parent
            return 0;
        }
        handleTab();
        return 1; // skip default handler
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

void CodeEditor::showRowCol() {
    int row, col;

    if (!position_to_linecol(cursor_pos_, &row, &col)) {
        // pageup/pagedown
        layout();
        position_to_linecol(cursor_pos_, &row, &col);
    }
    setRowCol(row, col+1);
}
    
void CodeEditor::gotoLine(int line) {
    int numLines = buffer()->count_lines(0, buffer()->length());
    if (line < 1) {
        line = 1;
    } else if (line > numLines) {
        line = numLines;
    }
    int pos = buffer()->skip_lines(0, line-1); // find pos at line-1
    insert_position(buffer()->line_start(pos)); // insert at column 0
    show_insert_position();
    setRowCol(line, 1);
}

void CodeEditor::getSelStartRowCol(int *row, int *col) {
    int start = buffer()->primary_selection()->start();
    int end = buffer()->primary_selection()->end();
    if (start == end) {
        *row = -1;
        *col = -1;
    } else {
        position_to_linecol(start, row, col);
    }
}

void CodeEditor::getSelEndRowCol(int *row, int *col) {
    int start = buffer()->primary_selection()->start();
    int end = buffer()->primary_selection()->end();
    if (start == end) {
        *row = -1;
        *col = -1;
    } else {
        position_to_linecol(end, row, col);
    }
}

void CodeEditor::getRowCol(int *row, int *col) {
    position_to_linecol(cursor_pos_, row, col);
}

//--EditorWindow----------------------------------------------------------------

EditorWindow::EditorWindow(int x, int y, int w, int h) : 
    Group(x, y, w, h) {

    replaceDlg = new Window(300, 105, "Replace");
    replaceDlg->begin();
    replaceFind = new Input(80, 10, 210, 25, "Find:");
    replaceFind->align(ALIGN_LEFT);
    replaceWith = new Input(80, 40, 210, 25, "Replace:");
    replaceWith->align(ALIGN_LEFT);
    Button* replaceAll = new Button(10, 70, 90, 25, "Replace All");
    replaceAll->callback((Callback *)replall_cb, this);
    ReturnButton* replaceNext = 
        new ReturnButton(105, 70, 120, 25, "Replace Next");
    replaceNext->callback((Callback *)replace2_cb, this);
    Button* replaceCancel = new Button(230, 70, 60, 25, "Cancel");
    replaceCancel->callback((Callback *)replcan_cb, this);
    replaceDlg->end();
    replaceDlg->set_non_modal();

    search[0] = 0;
    filename[0] = 0; 
    dirty = false;
    loading = false;
    textbuf = new TextBuffer();
    style_init();

    begin();
    editor = new CodeEditor(0, 0, w, h);
    editor->buffer(textbuf);
    editor->highlight_data(stylebuf, styletable,
                           sizeof(styletable) / sizeof(styletable[0]),
                           PLAIN, style_unfinished_cb, 0);
    //editor->wrap_mode(true, 80);
    editor->selection_color(fltk::color(190,189,188));
    editor->color(WHITE);
    end();
    resizable(editor);

    textbuf->add_modify_callback(style_update, editor);
    textbuf->add_modify_callback(changed_cb, this);
}

EditorWindow::~EditorWindow() {
    delete replaceDlg;
    textbuf->remove_modify_callback(style_update, editor);
    textbuf->remove_modify_callback(changed_cb, this);
}

int EditorWindow::handle(int e) {
    if (e == FOCUS) {
        fltk::focus(editor);
        return 1;
    }
    return Group::handle(e);
}

bool EditorWindow::readonly() {
    return ((CodeEditor*)editor)->readonly;
}

void EditorWindow::readonly(bool is_readonly) {
    editor->cursor_style(is_readonly ? TextDisplay::DIM_CURSOR : 
                         TextDisplay::NORMAL_CURSOR);
    ((CodeEditor*)editor)->readonly = is_readonly;
}

void EditorWindow::doChange(int inserted, int deleted) {
    if (loading) {
        return; // do nothing while file load in progress
    }

    if (inserted || deleted) {
        dirty = 1;
    }

    setModified(dirty);
}

bool EditorWindow::checkSave(bool discard) {
    if (!dirty) {
        return true; // continue next operation
    }

    const char* msg = "The current file has not been saved.\n"
        "Would you like to save it now?";
    int r = discard ? 
        choice(msg, "Save", "Discard", "Cancel") :
        choice(msg, "Save", "Cancel", 0);

    if (r == 0) {
        saveFile(); // Save the file
        return !dirty;
    }
    return (discard && r==1);
}

void EditorWindow::loadFile(const char *newfile, int ipos, bool updateUI) {
    loading = true;
    int insert = (ipos != -1);
    dirty = insert;
    if (!insert) {
        strcpy(filename, "");
    }

    int r;
    if (!insert) {
        r = textbuf->loadfile(newfile);
    } else {
        r = textbuf->insertfile(newfile, ipos);
    }

    if (r) {
        alert("Error reading from file \'%s\':\n%s.", newfile, strerror(errno));
    } else {
        if (!insert) {
            strcpy(filename, newfile);
        }
    }

    loading = false;
    if (updateUI) {
        statusMsg(filename);
        addHistory(filename);
        fileChanged(true);
    }

    textbuf->call_modify_callbacks();
    editor->show_insert_position();
    setRowCol(1, 1);
}

void EditorWindow::doSaveFile(const char *newfile, bool updateUI) {
    char basfile[PATH_MAX];
    strcpy(basfile, newfile);
    if (strchr(basfile, '.') == 0) {
        strcat(basfile, ".bas");
    }

    if (textbuf->savefile(basfile)) {
        alert("Error writing to file \'%s\':\n%s.", basfile, strerror(errno));
        return;
    }

    if (updateUI) {
        dirty = 0;
        if (filename[0] == 0) {
            // naming a previously unnamed buffer
            addHistory(basfile);
        }
        strcpy(filename, basfile);
        textbuf->call_modify_callbacks();
        statusMsg(basfile);
        fileChanged(true);
    }
}

void EditorWindow::showFindReplace() {
    replaceFind->value(search);
    replaceDlg->show();
}

bool EditorWindow::findText(const char* find, bool forward) {
    // copy lowercase search string for high-lighting
    strcpy(search, find);
    int findLen = strlen(search);
    for (int i=0; i<findLen; i++) {
        search[i] = tolower(search[i]);
    }

    style_update(0, textbuf->length(), textbuf->length(), 0,0, editor);
    if (find == 0 || find[0] == 0) {
        return 0;
    }

    int pos = editor->insert_position();
    bool found = forward ? textbuf->search_forward(pos, search, &pos) :
        textbuf->search_backward(pos-strlen(find), search, &pos);
    if (found) {
        textbuf->select(pos, pos+strlen(search));
        editor->insert_position(pos+strlen(search));
        editor->show_insert_position();
    }
    return found;
}

void EditorWindow::newFile() {
    if (readonly()) {
        return;
    }

    if (!checkSave(true)) {
        return;
    }

    filename[0] = '\0';
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
    dirty = 0;
    textbuf->call_modify_callbacks();
    statusMsg(0);
    fileChanged(false);
}

void EditorWindow::openFile() {
    if (readonly()) {
        return;
    }

    if (!checkSave(true)) {
        return;
    }

    const char *newfile = file_chooser("Open File", "*.bas", filename);
    if (newfile != NULL) {
        loadFile(newfile, -1, true);
    }
    restoreFocus();
}

void EditorWindow::insertFile() {
    if (readonly()) {
        return;
    }

    const char *newfile = file_chooser("Insert File?", "*.bas", filename);
    if (newfile != NULL) {
        loadFile(newfile, editor->insert_position(), true);
    }
    restoreFocus();
}

void EditorWindow::replaceNext() {
    if (readonly()) {
        return;
    }

    const char *find = replaceFind->value();
    const char *replace = replaceWith->value();

    if (find[0] == '\0') {
        // search string is blank; get a new one
        replaceDlg->show();
        return;
    }

    replaceDlg->hide();

    int pos = editor->insert_position();
    int found = textbuf->search_forward(pos, find, &pos);

    if (found) {
        // found a match; update the position and replace text
        textbuf->select(pos, pos+strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);
        textbuf->select(pos, pos+strlen(replace));
        editor->insert_position(pos+strlen(replace));
        editor->show_insert_position();
    } else {
        alert("No occurrences of \'%s\' found!", find);
    }
}

void EditorWindow::replaceAll() {
    if (readonly()) {
        return;
    }

    const char *find = replaceFind->value();
    const char *replace = replaceWith->value();

    find = replaceFind->value();
    if (find[0] == '\0') {
        // search string is blank; get a new one
        replaceDlg->show();
        return;
    }

    replaceDlg->hide();
    editor->insert_position(0);
    int times = 0;

    // loop through the whole string
    for (int found = 1; found;) {
        int pos = editor->insert_position();
        found = textbuf->search_forward(pos, find, &pos);

        if (found) {
            // found a match; update the position and replace text
            textbuf->select(pos, pos+strlen(find));
            textbuf->remove_selection();
            textbuf->insert(pos, replace);
            editor->insert_position(pos+strlen(replace));
            editor->show_insert_position();
            times++;
        }
    }

    if (times) {
        message("Replaced %d occurrences.", times);
    } else {
        alert("No occurrences of \'%s\' found!", find);
    }
    restoreFocus();
}

void EditorWindow::cancelReplace() {
    replaceDlg->hide();
}

void EditorWindow::saveFile() {
    if (filename[0] == '\0') {
        // no filename - get one!
        saveFileAs();
        return;
    } else {
        doSaveFile(filename, true);
        restoreFocus();
    }
}

void EditorWindow::saveFileAs() {
    const char* msg = 
        "%s\n\nFile already exists.\nDo you want to replace it?";
    const char* newfile = file_chooser("Save File As?", "*.bas", filename);
    if (newfile != NULL) {
        if (access(newfile, 0) == 0 && ask(msg, newfile) == 0) {
            return;
        }
        doSaveFile(newfile, true);
    }
    restoreFocus();
}

void EditorWindow::doDelete() {
    textbuf->remove_selection();
}

void EditorWindow::gotoLine(int line) {
    ((CodeEditor*)editor)->gotoLine(line);
}

void EditorWindow::getRowCol(int* row, int* col) {
    return ((CodeEditor*)editor)->getRowCol(row, col);
}

void EditorWindow::getSelStartRowCol(int *row, int *col) {
    return ((CodeEditor*)editor)->getSelStartRowCol(row, col);
}

void EditorWindow::getSelEndRowCol(int *row, int *col) {
    return ((CodeEditor*)editor)->getSelEndRowCol(row, col);
}

void EditorWindow::setFontSize(int size) {
    int len = sizeof(styletable) / sizeof(styletable[0]);
    for (int i=0; i<len; i++) {
        styletable[i].size = size;
    }
    textbuf->select(0, textbuf->length());
    textbuf->select(0, 0);
    editor->redraw(DAMAGE_ALL);
}

int EditorWindow::getFontSize() {
    return (int)styletable[0].size;
}

void EditorWindow::createFuncList() {
    const char *text = textbuf->text();
    int len = textbuf->length();
    for (int i=0; i<len; i++) {
        // avoid seeing "gosub" etc
        int offs = ((strncasecmp(text+i, "\nsub ", 5) == 0 ||
                     strncasecmp(text+i, " sub ", 5) == 0) ? 4 :
                    (strncasecmp(text+i, "\nfunc ", 6) == 0 || 
                     strncasecmp(text+i, " func ", 6) == 0) ? 5 : 0);
        if (offs != 0) {
            char* c = strchr(text+i+offs, '\n');
            if (c) {
                if (text[i] == '\n') {
                    i++; // skip initial newline
                }
                int itemLen = c-(text+i);
                String s(text+i, itemLen);
                Item* item = new Item();
                item->copy_label(s.toString());
                i += itemLen;
            }
        }
    }
    free((void*)text);
}

void EditorWindow::findFunc(const char* find) {
    const char *text = textbuf->text();
    int findLen = strlen(find);
    int len = textbuf->length();
    int lineNo = 1;
    for (int i=0; i<len; i++) {
        if (strncasecmp(text+i, find, findLen) == 0) {
            gotoLine(lineNo);
            break;
        } else if (text[i] == '\n') {
            lineNo++;
        }
    }
    free((void*)text);
}

void EditorWindow::setIndentLevel(int level) {
    ((CodeEditor*)editor)->indentLevel = level;
}

//--EndOfFile-------------------------------------------------------------------
