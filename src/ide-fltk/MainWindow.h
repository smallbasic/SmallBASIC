// -*- c-file-style: "java" -*-
// $Id: MainWindow.h,v 1.4 2004-11-11 22:31:33 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2003 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <fltk/Window.h>
#include <fltk/TabGroup.h>

#include "Fl_Ansi_Window.h"
#include "EditorWindow.h"

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }

void trace(const char *format, ...);

struct MainWindow : public Window {
    MainWindow(int w, int h);
    ~MainWindow();

    bool isTurboMode(void)  { return isTurbo; }
    bool wasBreakEv(void);
    void updateStatusBar(const char* title);
    void resetPen();

    int penX;
    int penY;
    int penDownX;
    int penDownY;
    int penState;
    bool isTurbo;

    // screen parts
    Fl_Ansi_Window *out;
    EditorWindow* editWnd;
    Group* outputGroup;
    Group* helpGroup;
    Group* editGroup;
    Group* textOutputGroup;
    Group* statusBar;
    TabGroup* tabGroup;
};

#endif
