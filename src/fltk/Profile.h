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

  void restore(MainWindow* wnd);
  void save(MainWindow* wnd);
  void loadConfig(EditorWidget* editor);

  fltk::Color color;
  fltk::Font* font;
  int fontSize;
  int indentLevel;
  int logPrint;
  int scrollLock;
  int hideIde;
  int gotoLine;

  private:
  void restoreStyles(strlib::Properties* profile);
  void restoreTabs(MainWindow* wnd, strlib::Properties* profile);
  void restoreValue(strlib::Properties* profile, const char* key, int* value);
  void saveStyles(FILE *fp);
  void saveTabs(MainWindow* wnd, FILE* fp);
  void saveValue(FILE* fp, const char* key, const char* value);
  void saveValue(FILE* fp, const char* key, int value);
};

#endif

// $Id$
