// -*- c-file-style: "java" -*-
// $Id: MainWindow.h,v 1.1 2004-11-07 23:01:14 zeeb90au Exp $
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

#include "Fl_Ansi_Window.h"
#include "EditorWindow.h"

struct MainWindow : public Window {
    MainWindow(int w, int h);
    ~MainWindow();
    
    bool wasBreakEv(void)   { return isBreak; }
    bool isTurboMode(void)  { return isTurbo; }
    bool isMenuActive(void) { return menuActive; }

    void run(const char* file);
    void resetPen() {
        penX = 0;
        penY = 0;
        penDownX = 0;
        penDownY = 0;
        penState = 0;
        menuActive = 0;
    }

    int penX;
    int penY;
    int penDownX;
    int penDownY;
    int penState;
    bool menuActive;
    Fl_Ansi_Window *out;
    EditorWindow* editWnd;

    private:
    bool isTurbo;
    bool isBreak;
};

#endif
