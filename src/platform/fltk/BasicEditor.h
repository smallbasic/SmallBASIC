//
// Based on test/editor.cxx - A simple text editor program for the Fast
// Light Tool Kit (FLTK). This program is described in Chapter 4 of the FLTK
// Programmer's Guide.
// Copyright 1998-2003 by Bill Spitzak and others.
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef BASIC_EDITOR_H
#define BASIC_EDITOR_H

#include <fltk/TextEditor.h>
#include "ui/strlib.h"

using namespace fltk;

bool isvar(int c);

struct StatusBar {
  virtual ~StatusBar() {}
  virtual void setRowCol(int row, int col) = 0;
};

struct BasicEditor : public TextEditor {
  BasicEditor(int x, int y, int w, int h, StatusBar *status);
  ~BasicEditor();

  bool findText(const char *find, bool forward, bool updatePos);
  int handle(int e);
  unsigned getIndent(char *indent, int len, int pos);
  void draw();
  int getFontSize();
  const char *getFontName();
  void getKeywords(strlib::List<strlib::String *> &keywords);
  void getRowCol(int *row, int *col);
  void getSelEndRowCol(int *row, int *col);
  void getSelStartRowCol(int *row, int *col);
  char *getSelection(Rectangle *rc);
  void gotoLine(int line);
  void handleTab();
  void setFont(Font *font);
  void setFontSize(int size);
  void showFindText(const char *text);
  void showMatchingBrace();
  void showRowCol();
  void styleChanged();
  void styleParse(const char *text, char *style, int length);

  bool readonly;
  int indentLevel;
  int matchingBrace;

  fltk::TextBuffer *stylebuf;
  fltk::TextBuffer *textbuf;
  char search[256];
  StatusBar *status;
};

#endif
