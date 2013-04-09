// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef PROFILE_H
#define PROFILE_H

#include <fltk/Color.h>
#include <fltk/Font.h>
#include <fltk/Rectangle.h>

#include "platform/common/StringLib.h"

using namespace fltk;
using namespace strlib;

struct MainWindow;
struct EditorWidget;

struct Profile {
  Profile();

  void loadConfig(EditorWidget *editor);
  void restore(MainWindow *wnd);
  void restoreAppPosition(fltk::Rectangle *wnd);
  void save(MainWindow *wnd);

  Color _color;
  Font *_font;
  fltk::Rectangle _appPosition;

  int _fontSize;
  int _indentLevel;
  int _createBackups;
  int _lineNumbers;

private:
  bool _loaded;
  int nextInteger(const char *s, int len, int &index);
  fltk::Rectangle restoreRect(Properties *profile, const char *key);
  void restoreStyles(Properties *profile);
  void restoreTabs(MainWindow *wnd, Properties *profile);
  void restoreValue(Properties *profile, const char *key, int *value);
  void restoreWindowPos(MainWindow *wnd, fltk::Rectangle &rc);
  void saveRect(FILE *fp, const char *key, fltk::Rectangle *wnd);
  void saveStyles(FILE *fp);
  void saveTabs(FILE *fp, MainWindow *wnd);
  void saveValue(FILE *fp, const char *key, const char *value);
  void saveValue(FILE *fp, const char *key, int value);
};

#endif

