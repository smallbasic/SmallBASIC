// -*- c-file-style: "java" -*-
// $Id: MainWindow.cpp,v 1.5 2004-11-11 22:31:33 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "sys.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <fltk/run.h>
#include <fltk/error.h>
#include <fltk/ask.H>
#include <fltk/Window.h>
#include <fltk/Group.h>
#include <fltk/TabGroup.h>
#include <fltk/MenuBar.h>
#include <fltk/ScrollGroup.h>
#include <fltk/Button.h>
#include <fltk/events.h>
#include <fltk/string.h>
#include <fltk/damage.h>

#include "MainWindow.h"
#include "EditorWindow.h"
#include "sbapp.h"

using namespace fltk;

MainWindow* wnd;

enum ExecState {
    init_state, edit_state, run_state, break_state, quit_state
} runMode = edit_state;

extern char filename[]; // in EditorWindow

void quit_cb(Widget*, void* v) {
    if (runMode == edit_state) {
        if (check_save(true)) {
            exit(0);
        }
    } else {
        // close after executor returns
        // TODO: confirm close running program
        runMode = quit_state;
    }
}

void about_cb(Widget*, void* v) {
    message("SmallBASIC 0.9.5.3");
}

void break_cb(Widget*, void* v) {
    runMode = break_state;
}

void run_cb(Widget*, void*) {
    if (check_save(false) && filename[0]) {
        wnd->tabGroup->selected_child(wnd->outputGroup);
        wnd->out->clearScreen();
        runMode = run_state;
        sbasic_main(filename);
        if (runMode == quit_state) {
            exit(0);
        }
        runMode = edit_state;
    }
}

void set_title() {
    char title[256];

    title[0] = 0;
    if (filename[0]) {
        char *slash = strrchr(filename, '/');
        if (slash != NULL) {
            strcpy(title, slash + 1);
        } else {
            strcpy(title, filename);
        }
    }
    wnd->updateStatusBar(title);
}

int arg_cb(int argc, char **argv, int &i) {
    if (i+1 >= argc) {
        return false;
    }

    switch (argv[i][1]) {
    case 'e':
        strcpy(filename, argv[i+1]);
        runMode = edit_state;
        i+=2;
        return 1;
    case 'r':
        strcpy(filename, argv[i+1]);
        runMode = run_state;
        i+=2;
        return 1;
    }
    return 0;
}

#if defined(__CYGWIN__)
// see http://www.sysinternals.com/ntw2k/utilities.shtml
// for the free DebugView program
#include <windows.h>
void trace(const char *format, ...) {
    char	buf[4096], *p = buf;
    va_list	args;
    
    va_start(args, format);
    p += vsnprintf(p, sizeof buf - 1, format, args);
    va_end(args);
    
    while (p > buf && isspace(p[-1])) {
        *--p = '\0';
    }
    
    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';
    OutputDebugString(buf);
}
#endif

int main(int argc, char **argv) {
    filename[0] = 0;
    int i=0;
    if (args(argc, argv, i, arg_cb) < argc) {
        fatal("Options are:\n -r[un] file.bas\n -e[dit] file.bas\n%s", help);
    }

    wnd = new MainWindow(600, 400);
    wnd->show(argc, argv);

    if (runMode == run_state) {
        // run the application now

    }

    return run();
}

struct EditGroup : public Group {
    EditGroup(int x, int y, int w, int h, const char * s) : 
        Group(x, y, w, h, s) {}
    EditorWindow* editWnd;
    int handle(int event) {
        // TextDisplay::layout() does nothing when not visible
        if (event == SHOW) {
            editWnd->layout();
        }
        return Group::handle(event);
    }
};

MainWindow::MainWindow(int w, int h) : Window(w, h, "SmallBASIC") {
    int mnuHeight = 20;
    int statusHeight = mnuHeight;
    int groupHeight = h-mnuHeight-statusHeight;
    int tabHeight = mnuHeight;

    isTurbo = 0;
	opt_graphics = 1;
	opt_quite = 1;
    opt_nosave = 1;
	opt_ide = 1;
	opt_command[0] = '\0';
	opt_pref_width = 0;
    opt_pref_height = 0;
    opt_pref_bpp = 0;
    
    begin();
    MenuBar* m = new MenuBar(0, 0, w, mnuHeight);
    m->add("&File/&New File",       0,        (Callback*)new_cb);
    m->add("&File/&Open File...",   CTRL+'o', (Callback*)open_cb);
    m->add("&File/_&Insert File...",CTRL+'i', (Callback*)insert_cb);
    m->add("&File/&Save File",      CTRL+'s', (Callback*)save_cb);
    m->add("&File/Save File &As...",CTRL+SHIFT+'S', (Callback*)saveas_cb);
    m->add("&File/E&xit",           CTRL+'q', (Callback*)quit_cb);
    m->add("&Edit/Cu&t",            CTRL+'x', (Callback*)cut_cb);
    m->add("&Edit/&Copy",           CTRL+'c', (Callback*)copy_cb);
    m->add("&Edit/&Paste",          CTRL+'v', (Callback*)paste_cb);
    m->add("&Edit/_&Delete",        0,        (Callback*)delete_cb);
    m->add("&Edit/&Settings",       0,        (Callback*)delete_cb);
    m->add("&Program/&Run",         CTRL+'r', (Callback*)run_cb);
    m->add("&Program/&Break",       CTRL+'b', (Callback*)break_cb);
    m->add("&Search/&Find...",      CTRL+'f', (Callback*)find_cb);
    m->add("&Search/Find A&gain",   CTRL+'g', (Callback*)find2_cb);
    m->add("&Search/&Replace...",   0,        (Callback*)replace_cb);
    m->add("&Search/Replace &Again",CTRL+'t', (Callback*)replace2_cb);
    m->add("&About...",             CTRL+'f', (Callback*)about_cb);

    //todo fixme
    callback(quit_cb);

    tabGroup = new TabGroup(0, mnuHeight, w, groupHeight);
    tabGroup->begin();

    EditGroup* eg = new EditGroup(0, 0, w, groupHeight-tabHeight, "Editor");
    editGroup = eg;
    editGroup->begin();
    editWnd = new EditorWindow(2, 2, w-4, groupHeight-tabHeight-4);
    eg->editWnd = editWnd;
    m->user_data(editWnd); // the EditorWindow is callback user data

    if (filename[0] != 0) {
        load_file(filename, 0);
    }
    editGroup->resizable(editWnd);
    editGroup->end();
    tabGroup->resizable(editGroup);

    helpGroup = new Group(0, 0, w, groupHeight-tabHeight, "Help");
    helpGroup->hide();
    helpGroup->begin();
    // TODO: add help control
    helpGroup->end();

    outputGroup = new Group(0, 0, w, groupHeight-tabHeight, "Output");
    outputGroup->begin();
    out = new Fl_Ansi_Window(2, 2, w-4, groupHeight-tabHeight-4);
    outputGroup->resizable(out);
    outputGroup->end();

    textOutputGroup = new Group(0, 0, w, groupHeight-tabHeight, "Text Output");
    textOutputGroup->hide();
    textOutputGroup->begin();
    // TODO: add text output control
    textOutputGroup->end();

    tabGroup->end();
    resizable(tabGroup);

    editWnd->mainWnd = this;
    statusBar = new Group(0, h-mnuHeight, w, mnuHeight);
    statusBar->align(ALIGN_INSIDE_LEFT);
    statusBar->labelfont(COURIER);
    updateStatusBar(0);
    end();
}

MainWindow::~MainWindow() {}

void MainWindow::updateStatusBar(const char* title) {
    char statusText[256];
    sprintf(statusText, 
            "--%s-- SmallBASIC: %s ----L%d--C%d----", 
            editWnd->is_dirty()? "**":"--",
            title&&title[0] ? title : "Untitled", 0,0);
    statusBar->copy_label(statusText);
    redraw();
}

bool MainWindow::wasBreakEv(void) {
    return (runMode == break_state || runMode == quit_state);
}

void MainWindow::resetPen() {
    penX = 0;
    penY = 0;
    penDownX = 0;
    penDownY = 0;
    penState = 0;
}
