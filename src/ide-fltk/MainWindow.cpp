// -*- c-file-style: "java" -*-
// $Id: MainWindow.cpp,v 1.1 2004-11-07 23:01:14 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Group.h>
#include <fltk/TabGroup.h>
#include <fltk/MenuBar.h>
#include <fltk/ScrollGroup.h>
#include <fltk/Button.h>
#include <fltk/events.h>

#include "MainWindow.h"
#include "EditorWindow.h"

using namespace fltk;

MainWindow* wnd;

void quit_cb(Widget*, void* v) {
    exit(0);
}

void about_cb(Widget*, void* v) {
    exit(0);
}

void break_cb(Widget*, void* v) {
    exit(0);
}

void run_cb(Widget*, void* v) {
    MainWindow* wnd = (MainWindow*)v;
    if (wnd->out->visible()) {
        wnd->out->hide();
    } else {
        wnd->out->print("\nshowing...");
        wnd->out->show();
    }
}

MainWindow::MainWindow(int w, int h) : Window(w, h, "SmallBASIC") {
    int mnuHeight = 20;

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

    TabGroup* tabGroup = new TabGroup(0, mnuHeight, w, h-mnuHeight);
    tabGroup->begin();

    Group* editGroup = new Group(0, mnuHeight, w, h, "Editor");
    editGroup->begin();
    editWnd = new EditorWindow(2, 1, w-4, h-(2*mnuHeight)-4);
    editGroup->end();
    tabGroup->resizable(editGroup);
    
    Group* helpGroup = new Group(0, mnuHeight, w, h, "Help");
    helpGroup->hide();
    helpGroup->begin();
    // TODO: add help control
    helpGroup->end();

    Group* outputGroup = new Group(0, mnuHeight, w, h, "Output");
    outputGroup->hide();
    outputGroup->begin();
    out = new Fl_Ansi_Window(2, 1, w-4, h-(2*mnuHeight)-4);
    out->print("SmallBASIC");
    outputGroup->resizable(out);
    outputGroup->end();

    Group* textOutputGroup = new Group(0, mnuHeight, w, h, "Text Output");
    textOutputGroup->hide();
    textOutputGroup->begin();
    // TODO: add text output control
    textOutputGroup->end();

    tabGroup->end();
    resizable(tabGroup);
    end();
}

MainWindow::~MainWindow() {}

int main(int argc, char **argv) {
    wnd = new MainWindow(600, 400);
    wnd->show(argc, argv);
    return run();
}
