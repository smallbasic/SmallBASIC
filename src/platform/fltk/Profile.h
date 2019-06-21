// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef PROFILE_H
#define PROFILE_H

#include <FL/Fl_Rect.H>
#include "ui/strlib.h"
#include "ui/textedit.h"

using namespace strlib;

struct MainWindow;
struct EditorWidget;
class HelpWidget;

struct Profile {
  Profile();

  bool createBackups() const { return _createBackups; }
  void loadConfig(EditorWidget *editor);
  void loadEditTheme(int themeId);
  void restore(MainWindow *wnd);
  void restoreAppPosition(Fl_Window *wnd);
  void setAppPosition(Fl_Rect rect) { _appPosition = rect; }
  void setEditTheme(EditorWidget *editor);
  void setHelpTheme(HelpWidget *help, int themeId = -1);
  void setFont(Fl_Font font) { _font = font; }
  void setFontSize(int size) { _fontSize = size; }
  void save(MainWindow *wnd);
  void updateTheme();

private:
  Fl_Font _font;
  Fl_Rect _appPosition;
  EditTheme _theme;
  EditTheme _helpTheme;
  bool _loaded;
  int _createBackups;
  int _lineNumbers;
  int _fontSize;
  int _indentLevel;
  int _themeId;
  int _helpThemeId;

  int nextInteger(const char *s, int len, int &index);
  Fl_Rect restoreRect(Properties<String *> *profile, const char *key);
  void restoreStyles(Properties<String *> *profile);
  void restoreTabs(MainWindow *wnd, Properties<String *> *profile);
  void restoreValue(Properties<String *> *profile, const char *key, int *value);
  void restoreWindowPos(MainWindow *wnd, Fl_Rect &rc);
  void saveRect(FILE *fp, const char *key, Fl_Rect *wnd);
  void saveStyles(FILE *fp);
  void saveTabs(FILE *fp, MainWindow *wnd);
  void saveValue(FILE *fp, const char *key, const char *value);
  void saveValue(FILE *fp, const char *key, int value);
};

#endif

