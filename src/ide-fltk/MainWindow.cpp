// -*- c-file-style: "java" -*-
// $Id: MainWindow.cpp,v 1.3 2004-11-09 22:06:18 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "sys.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

#include "MainWindow.h"
#include "EditorWindow.h"
#include "sbapp.h"

using namespace fltk;

MainWindow* wnd;

enum ExecMode {
    run_mode, edit_mode, init_mode
} runMode = init_mode;

char fileName[256];

void quit_cb(Widget*, void* v) {
    exit(0);
}

void about_cb(Widget*, void* v) {
    message("SmallBASIC 0.9.4.5");
}

void break_cb(Widget*, void* v) {
    MainWindow* wnd = (MainWindow*)v;
    wnd->isBreak = true;
}

void run_cb(Widget*, void* v) {
    MainWindow* wnd = (MainWindow*)v;
    wnd->tabGroup->selected_child(wnd->outputGroup);
    sbasic_main(wnd->editWnd->get_filename());
}

MainWindow::MainWindow(int w, int h) : Window(w, h, "SmallBASIC") {
    int mnuHeight = 20;
    int statusHeight = mnuHeight;
    int groupHeight = h-mnuHeight-statusHeight;
    int tabHeight = mnuHeight;

    menuActive = 0;
    isTurbo = 0;
    isBreak = 0;
	opt_graphics = 0;
	opt_quite = 0;
	opt_ide = 0;
	opt_command[0] = '\0';
	opt_pref_width = 0;
    opt_pref_height = 0;
    opt_pref_bpp = 0;

    begin();
    MenuBar* m = new MenuBar(0, 0, w, mnuHeight);
    m->user_data(this); // make this be passed to all menu callbacks
    m->add("&File/&New File",       0,        (Callback*)new_cb);
    m->add("&File/&Open File...",   CTRL+'o', (Callback*)open_cb);
    m->add("&File/_&Insert File...",CTRL+'i', (Callback*)insert_cb);
    m->add("&File/&Save File",      CTRL+'s', (Callback*)save_cb);
    m->add("&File/Save File &As...",CTRL+SHIFT+'S', (Callback*)saveas_cb);
    m->add("&File/E&xit",           CTRL+'q', (Callback*)quit_cb);
    m->add("&Edit/Cu&t",            CTRL+'x', (Callback*)cut_cb);
    m->add("&Edit/&Copy",           CTRL+'c', (Callback*)copy_cb);
    m->add("&Edit/&Paste",          CTRL+'v', (Callback*)paste_cb);
    m->add("&Edit/&Delete",         0,        (Callback*)delete_cb);
    m->add("&Edit/_&Settings",      0,        (Callback*)delete_cb);
    m->add("&Program/&Run",         CTRL+'z', (Callback*)run_cb);
    m->add("&Program/&Break",       CTRL+'z', (Callback*)break_cb);
    m->add("&Edit/Cu&t",            CTRL+'x', (Callback*)cut_cb);
    m->add("&Edit/&Copy",           CTRL+'c', (Callback*)copy_cb);
    m->add("&Edit/&Paste",          CTRL+'v', (Callback*)paste_cb);
    m->add("&Edit/&Delete",         0,        (Callback*)delete_cb);
    m->add("&Search/&Find...",      CTRL+'f', (Callback*)find_cb);
    m->add("&Search/Find A&gain",   CTRL+'g', (Callback*)find2_cb);
    m->add("&Search/&Replace...",   CTRL+'r', (Callback*)replace_cb);
    m->add("&Search/Replace &Again",CTRL+'t', (Callback*)replace2_cb);
    m->add("&About...",             CTRL+'f', (Callback*)about_cb);

    tabGroup = new TabGroup(0, mnuHeight, w, groupHeight);
    tabGroup->begin();

    editGroup = new Group(0, 0, w, groupHeight-tabHeight, "Editor");
    editGroup->begin();
    editWnd = new EditorWindow(2, 2, w-4, groupHeight-tabHeight-4);
    if (fileName[0] != 0) {
        load_file(fileName, 0);
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
    editWnd->statusBar = new Group(0, h-mnuHeight, w, mnuHeight);
    end();
}

MainWindow::~MainWindow() {}

int arg_cb(int argc, char **argv, int &i) {
    if (i+1 >= argc) {
        return false;
    }

    switch (argv[i][1]) {
    case 'e':
        strcpy(fileName, argv[i+1]);
        runMode = edit_mode;
        i+=2;
        return 1;
    case 'r':
        strcpy(fileName, argv[i+1]);
        runMode = run_mode;
        i+=2;
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    fileName[0] = 0;
    int i=0;
    if (args(argc, argv, i, arg_cb) < argc) {
        fatal("Options are:\n -r[un] file.bas\n -e[dit] file.bas\n%s", help);
    }

    wnd = new MainWindow(600, 400);
    wnd->show(argc, argv);
    return run();
}
