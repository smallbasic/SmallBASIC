// -*- c-file-style: "java" -*-
// $Id: EditorWindow.h,v 1.2 2004-11-08 22:22:51 zeeb90au Exp $
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

#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <fltk/DoubleBufferWindow.h>
#include <fltk/Window.h>
#include <fltk/Widget.h>
#include <fltk/Input.h>
#include <fltk/ReturnButton.h>
#include <fltk/Button.h>
#include <fltk/TextEditor.h>

using namespace fltk;

class EditorWindow : public DoubleBufferWindow {
    public:
    EditorWindow(int x, int y, int w, int h);
    ~EditorWindow();

    Window          *replaceDlg;
    Input           *replaceFind;
    Input           *replaceWith;
    Button          *replaceAll;
    ReturnButton    *replaceNext;
    Button          *replaceCancel;
    TextEditor      *editor;
    char            search[256];
};

// editor callback functions
void save_cb();
void saveas_cb();
void find2_cb(Widget*, void*);
void new_cb(Widget*, void*);
void open_cb(Widget*, void*);
void insert_cb(Widget*, void *v);
void save_cb();
void cut_cb(Widget*, void* v);
void paste_cb(Widget*, void* v);
void copy_cb(Widget*, void* v);
void delete_cb(Widget*, void*);
void find_cb(Widget* w, void* v);
void find2_cb(Widget* w, void* v);
void replace_cb(Widget*, void* v);
void replace2_cb(Widget*, void* v);
void load_file(char *newfile, int ipos);

#endif
