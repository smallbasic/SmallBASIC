// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FILE_WIDGET_H
#define FILE_WIDGET_H

#include <limits.h>
#include "platform/fltk/HelpWidget.h"
#include "platform/fltk/EditorWidget.h"

struct FileWidget : public HelpWidget {
  FileWidget(Fl_Widget *rect, int fontSize);
  ~FileWidget();

  static const char *forwardSlash(char *filename);
  static const char *splitPath(const char *filename, char *path);
  static const char *trimEOL(char *buffer);

  void anchorClick();
  void fileOpen(EditorWidget *saveEditorAs);
  void openPath(const char *newPath, StringList *recentPaths);

private:
  void changeDir(const char *target);
  void setDir(const char *target);
  void displayPath();
  void enterPath();
  int handle(int e);
  void saveAs();

  char _path[PATH_MAX + 1];
  EditorWidget *_saveEditorAs;
  StringList *_recentPaths;
};

#endif
