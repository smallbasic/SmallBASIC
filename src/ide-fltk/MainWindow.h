// -*- c-file-style: "java" -*-
// $Id: MainWindow.h,v 1.3 2004-11-10 22:19:57 zeeb90au Exp $
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
    void updateStatusBar();
    void run(const char* file);
    void resetPen() {
        penX = 0;
        penY = 0;
        penDownX = 0;
        penDownY = 0;
        penState = 0;
    }

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
    char statusText[256];
};

#endif
