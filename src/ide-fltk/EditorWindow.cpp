// -*- c-file-style: "java" -*-
// $Id: EditorWindow.cpp,v 1.5 2004-11-11 22:31:33 zeeb90au Exp $
//
// Based on test/editor.cxx - A simple text editor program for the Fast 
// Light Tool Kit (FLTK). This program is described in Chapter 4 of the FLTK 
// Programmer's Guide.
// Copyright 1998-2003 by Bill Spitzak and others.
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
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

#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/file_chooser.h>
#include <fltk/Flags.h>
#include <fltk/FileChooser.h>
#include <fltk/MenuBar.h>
#include <fltk/Input.h>
#include <fltk/Button.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextBuffer.h>
#include <fltk/TextEditor.h>

#include "EditorWindow.h"
#include "MainWindow.h"
#include "kwp.cxx"

using namespace fltk;

TextDisplay::StyleTableEntry styletable[] = { // Style table
    { BLACK,           COURIER,        14 }, // A - Plain
    { color(0,0,128),  COURIER_ITALIC, 14 }, // B - Line comments
    { color(0,0,64),   COURIER_ITALIC, 14 }, // C - Block comments
    { color(0,64,128), COURIER,        14 }, // D - Strings
    { color(128,0,0),  COURIER,        14 }, // E - Directives
    { color(64,0,0),   COURIER_BOLD,   14 }, // F - Types
    { color(64,64,0),  COURIER_BOLD,   14 }, // G - Keywords
    { color(64,0,64),  COURIER_BOLD,   14 }  // H - procedures
};

TextBuffer *stylebuf = 0;
TextBuffer *textbuf = 0;
char filename[256];
int dirty = 0;
int loading = 0;

// 'compare_keywords()' - Compare two keywords
int compare_keywords(const void *a, const void *b) {
    return (strcmp(*((const char **)a), *((const char **)b)));
}

// 'style_parse()' - Parse text and produce style data.
void style_parse(const char *text, char *style, int length) {
    char current;
    int  col;
    int  last;
    char  buf[255];
    char *bufptr;
    const char *temp;

    for (current = *style, col = 0, last = 0; length > 0; length--, text ++) {
        if (current == 'B') { 
            current = 'A';
        }

        if (current == 'A') {
            // check for directives, comments, strings, and keywords
            if (col == 0 && *text == '#') {
                // set style to directive
                current = 'E';
            } else if (strncmp(text, "//", 2) == 0) {
                current = 'B';
                for (; length > 0 && *text != '\n'; length--, text ++) {
                    *style++ = 'B';
                }
                if (length == 0) {
                    break;
                }
            } else if (strncmp(text, "/*", 2) == 0) {
                current = 'C';
            } else if (strncmp(text, "\\\"", 2) == 0) {
                // quoted quote
                *style++ = current;
                *style++ = current;
                text ++;
                length--;
                col += 2;
                continue;
            } else if (*text == '\"') {
                current = 'D';
            } else if (!last && islower(*text)) {
                // might be a keyword
                for (temp = text, bufptr = buf;
                     islower(*temp) && bufptr < (buf + sizeof(buf) - 1);
                     *bufptr++ = *temp++);

                if (!islower(*temp)) {
                    *bufptr = '\0';
                    bufptr = buf;

                    if (bsearch(&bufptr, code_functions,
                                sizeof(code_functions) / sizeof(code_functions[0]),
                                sizeof(code_functions[0]), compare_keywords)) {
                        while (text < temp) {
                            *style++ = 'F';
                            text++;
                            length--;
                            col++;
                        }
                        text--;
                        length++;
                        last = 1;
                        continue;
                    } else if (bsearch(&bufptr, code_keywords,
                                       sizeof(code_keywords) / sizeof(code_keywords[0]),
                                       sizeof(code_keywords[0]), compare_keywords)) {
                        while (text < temp) {
                            *style++ = 'G';
                            text++;
                            length--;
                            col++;
                        }

                        text--;
                        length++;
                        last = 1;
                        continue;
                    } else if (bsearch(&bufptr, code_procedures,
                                       sizeof(code_procedures) / sizeof(code_procedures[0]),
                                       sizeof(code_procedures[0]), compare_keywords)) {
                        while (text < temp) {
                            *style++ = 'H';
                            text++;
                            length--;
                            col++;
                        }

                        text--;
                        length++;
                        last = 1;
                        continue;
                    }
                }
            }
        } else if (current == 'C' && strncmp(text, "*/", 2) == 0) {
            // close a C comment
            *style++ = current;
            *style++ = current;
            text++;
            length--;
            current = 'A';
            col += 2;
            continue;
        } else if (current == 'D') {
            // continuing in string
            if (strncmp(text, "\\\"", 2) == 0) {
                // quoted end quote
                *style++ = current;
                *style++ = current;
                text++;
                length--;
                col += 2;
                continue;
            } else if (*text == '\"') {
                // End quote
                *style++ = current;
                col++;
                current = 'A';
                continue;
            }
        }

        // copy style info
        if (current == 'A' && (*text == '{' || *text == '}')) {
            *style++ = 'G';
        } else {
            *style++ = current;
        }
        col++;
        last = isalnum(*text) || *text == '.';

        if (*text == '\n') {
            // reset column and possibly reset the style
            col = 0;
            if (current == 'B' || current == 'E') {
                current = 'A';
            }
        }
    }
}

// 'style_init()' - Initialize the style buffer
void style_init(void) {
    char *style = new char[textbuf->length() + 1];
    const char *text = textbuf->text();

    memset(style, 'A', textbuf->length());
    style[textbuf->length()] = '\0';

    if (!stylebuf) {
        stylebuf = new TextBuffer(textbuf->length());
    }

    style_parse(text, style, textbuf->length());
    stylebuf->text(style);
    delete[] style;
}

// 'style_unfinished_cb()' - Update unfinished styles.
void style_unfinished_cb() {}

char* get_style_range(int start, int end) {
    const char* s = stylebuf->text_range(start, end);
    char *style = new char[strlen(s) + 1];
    strcpy(style, s);
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
    char *style;             // Text data

    // if this is just a selection change, just unselect the style buffer
    if (nInserted == 0 && nDeleted == 0) {
        stylebuf->unselect();
        return;
    }

    // track changes in the text buffer
    if (nInserted > 0) { 
        // insert characters into the style buffer
        char *stylex = new char[nInserted + 1];
        memset(stylex, 'A', nInserted);
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

    //  printf("start = %d, end = %d, text = \"%s\", style = \"%s\"...\n",
    //         start, end, text, style);

    style_parse(text, style, end - start);

    //  printf("new style = \"%s\"...\n", style);

    stylebuf->replace(start, end, style);
    ((TextEditor *)cbArg)->redisplay_range(start, end);

    if (last != style[end - start - 1]) {
        // the last character on the line changed styles, 
        // so reparse the remainder of the buffer
        delete[] style;

        end   = textbuf->length();
        text  = textbuf->text_range(start, end);
        style = get_style_range(start, end);

        style_parse(text, style, end - start);
        stylebuf->replace(start, end, style);
        ((TextEditor *)cbArg)->redisplay_range(start, end);
    }

    delete[] style;
}

int check_save(bool discard) {
    if (!dirty) {
        return 1;
    }

    int r = choice("The current file has not been saved.\n"
                   "Would you like to save it now?",
                   "Cancel", "Save", discard? "Discard": 0);

    if (r == 1) {
        save_cb(); // Save the file
        return !dirty;
    }

    return (r == 2) ? 1 : 0;
}

void load_file(char *newfile, int ipos) {
    loading = 1;
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
    loading = 0;
    textbuf->call_modify_callbacks();
}

void save_file(char *newfile) {
    if (textbuf->savefile(newfile)) {
        alert("Error writing to file \'%s\':\n%s.", newfile, strerror(errno));
    } else {
        strcpy(filename, newfile);
    }
    dirty = 0;
    textbuf->call_modify_callbacks();
}

void copy_cb(Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    TextEditor::kf_copy(0, e->editor);
}

void cut_cb(Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    TextEditor::kf_cut(0, e->editor);
}

void delete_cb(Widget*, void*) {
    textbuf->remove_selection();
}

void find_cb(Widget* w, void* v) {
    EditorWindow* e = (EditorWindow*)v;

    const char *val = input("Search String:", e->search);
    if (val != NULL) {
        // user entered a string - go find it!
        strcpy(e->search, val);
        find2_cb(w, v);
    }
}

void find2_cb(Widget* w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    if (e->search[0] == '\0') {
        // search string is blank; get a new one
        find_cb(w, v);
        return;
    }

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, e->search, &pos);
    if (found) {
        // Found a match; select and update the position
        textbuf->select(pos, pos+strlen(e->search));
        e->editor->insert_position(pos+strlen(e->search));
        e->editor->show_insert_position();
    } else {
        alert("No occurrences of \'%s\' found!", e->search);
    }
}

void changed_cb(int, int nInserted, int nDeleted,int, const char*, void* v) {
    if ((nInserted || nDeleted) && !loading) {
        dirty = 1;
    }

    set_title();

    if (loading) {
        EditorWindow *w = (EditorWindow *)v;
        w->editor->show_insert_position();
    }
}

void new_cb(Widget*, void*) {
    if (!check_save(true)) {
        return;
    }

    filename[0] = '\0';
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
    dirty = 0;
    textbuf->call_modify_callbacks();
}

void open_cb(Widget*, void*) {
    if (!check_save(true)) {
        return;
    }

    char *newfile = file_chooser("Open File?", "*.bas", filename);
    if (newfile != NULL) {
        load_file(newfile, -1);
    }
}

void insert_cb(Widget*, void *v) {
    char *newfile = file_chooser("Insert File?", "*.bas", filename);
    EditorWindow *w = (EditorWindow *)v;
    if (newfile != NULL) {
        load_file(newfile, w->editor->insert_position());
    }
}

void paste_cb(Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    TextEditor::kf_paste(0, e->editor);
}

void close_cb(Widget*, void* v) {
    Window* w = (Window*)v;
    if (!check_save(true)) {
        return;
    }

    w->hide();
    textbuf->remove_modify_callback(changed_cb, w);
    delete w;
    exit(0);
}

// void quit_cb(Widget*, void*) {
//     if (changed && !check_save())
//         return;

//     exit(0);
// }

void replace_cb(Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replaceDlg->show();
}

void replace2_cb(Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char *find = e->replaceFind->value();
    const char *replace = e->replaceWith->value();

    if (find[0] == '\0') {
        // search string is blank; get a new one
        e->replaceDlg->show();
        return;
    }

    e->replaceDlg->hide();

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, find, &pos);

    if (found) {
        // found a match; update the position and replace text
        textbuf->select(pos, pos+strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);
        textbuf->select(pos, pos+strlen(replace));
        e->editor->insert_position(pos+strlen(replace));
        e->editor->show_insert_position();
    } else {
        alert("No occurrences of \'%s\' found!", find);
    }
}

void replall_cb(Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char *find = e->replaceFind->value();
    const char *replace = e->replaceWith->value();

    find = e->replaceFind->value();
    if (find[0] == '\0') {
        // search string is blank; get a new one
        e->replaceDlg->show();
        return;
    }

    e->replaceDlg->hide();
    e->editor->insert_position(0);
    int times = 0;

    // loop through the whole string
    for (int found = 1; found;) {
        int pos = e->editor->insert_position();
        found = textbuf->search_forward(pos, find, &pos);

        if (found) {
            // found a match; update the position and replace text
            textbuf->select(pos, pos+strlen(find));
            textbuf->remove_selection();
            textbuf->insert(pos, replace);
            e->editor->insert_position(pos+strlen(replace));
            e->editor->show_insert_position();
            times++;
        }
    }

    if (times) {
        message("Replaced %d occurrences.", times);
    } else {
        alert("No occurrences of \'%s\' found!", find);
    }
}

void replcan_cb(Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replaceDlg->hide();
}

void save_cb() {
    if (filename[0] == '\0') {
        // no filename - get one!
        saveas_cb();
        return;
    } else {
        save_file(filename);
    }
}

void saveas_cb() {
    char* newfile = file_chooser("Save File As?", "*.bas", filename);
    if (newfile != NULL) {
        save_file(newfile);
    }
}

EditorWindow::EditorWindow(int x, int y, int w, int h) : 
    Group(x, y, w, h) {

    replaceDlg = new Window(300, 105, "Replace");
    replaceDlg->begin();
    replaceFind = new Input(80, 10, 210, 25, "Find:");
    replaceFind->align(ALIGN_LEFT);
    replaceWith = new Input(80, 40, 210, 25, "Replace:");
    replaceWith->align(ALIGN_LEFT);
    replaceAll = new Button(10, 70, 90, 25, "Replace All");
    replaceAll->callback((Callback *)replall_cb, this);
    replaceNext = new ReturnButton(105, 70, 120, 25, "Replace Next");
    replaceNext->callback((Callback *)replace2_cb, this);
    replaceCancel = new Button(230, 70, 60, 25, "Cancel");
    replaceCancel->callback((Callback *)replcan_cb, this);
    replaceDlg->end();
    replaceDlg->set_non_modal();

    search[0] = 0;
    filename[0] = 0; 
    mainWnd = 0;
    dirty = 0;
    loading = 0;
    textbuf = new TextBuffer;
    style_init();

    begin();
    editor = new TextEditor(0, 0, w, h);
    editor->buffer(textbuf);
    editor->highlight_data(stylebuf, styletable,
                           sizeof(styletable) / sizeof(styletable[0]),
                           'A', style_unfinished_cb, 0);
    editor->textfont(COURIER);
    end();
    resizable(editor);

    textbuf->add_modify_callback(style_update, editor);
    textbuf->add_modify_callback(changed_cb, this);
}

EditorWindow::~EditorWindow() {
    delete replaceDlg;
}

bool EditorWindow::is_dirty() {
    return dirty;
}
