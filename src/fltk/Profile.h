// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2010 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef PROFILE_H
#define PROFILE_H

#include <fltk/Color.h>
#include <fltk/Font.h>

#include "StringLib.h"

struct MainWindow;
struct EditorWidget;

struct Profile {
  Profile();

  void loadConfig(EditorWidget* editor);
  void restore(MainWindow* wnd);
  void save(MainWindow* wnd);

  fltk::Color color;
  fltk::Font* font;
  int fontSize;
  int indentLevel;
  int createBackups;

  private:
  bool loaded;
  int nextInteger(const char* s, int len, int& index);
  void restoreStyles(strlib::Properties* profile);
  void restoreTabs(MainWindow* wnd, strlib::Properties* profile);
  void restoreValue(strlib::Properties* profile, const char* key, int* value);
  void restoreWindowPos(MainWindow* wnd, strlib::Properties* profile);
  void saveStyles(FILE *fp);
  void saveTabs(FILE* fp, MainWindow* wnd);
  void saveValue(FILE* fp, const char* key, const char* value);
  void saveValue(FILE* fp, const char* key, int value);
  void saveWindowPos(FILE* fp, MainWindow* wnd);
};

#endif

// $Id$
