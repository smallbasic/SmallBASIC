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

#include "StringLib.h"

struct MainWindow;

struct Profile {
  Profile();

  void restore(MainWindow* wnd);
  void save(MainWindow* wnd);

  int indentLevel;
  int ttyRows;
  int logPrint;
  int scrollLock;
  int hideIde;
  int gotoLine;

  private:
  void restoreTabs(MainWindow* wnd, strlib::List* paths);
  void restoreValue(strlib::Properties* profile, const char* key, int* value);
  void saveTabs(MainWindow* wnd, FILE* fp);
  void saveValue(FILE* fp, const char* key, const char* value);
  void saveValue(FILE* fp, const char* key, int value);
};

#endif

// $Id$
