//
// FileWidget
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FILE_WIDGET_H
#define FILE_WIDGET_H

#include "HelpWidget.h"
#include "EditorWidget.h"
#include <limits.h>

struct FileWidget:public HelpWidget {
  FileWidget(int x, int y, int w, int h);
  ~FileWidget();

  static const char *forwardSlash(char *filename);
  static const char *splitPath(const char *filename, char *path);
  static const char *trimEOL(char *buffer);

  void anchorClick();
  void fileOpen(EditorWidget *saveEditorAs);
  void openPath(const char *newPath);

private:
  void changeDir(const char *target);
  void displayPath();
  void enterPath();
  int handle(int e);
  void saveAs();

  char path[PATH_MAX + 1];
  EditorWidget *saveEditorAs;
};

#endif
