// $Id: EditorWidget.h 622 2008-07-14 13:08:59Z zeeb90au $
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

struct FileWidget : public HelpWidget {
  FileWidget(int x, int y, int w, int h);
  ~FileWidget();

  // returns the final slash char
  static char* forwardSlash(char *filename);
  void fileOpen(EditorWidget* saveEditorAs);
  void anchorClick();
  int handle(int e);

  private:
  void displayPath();
  char path[PATH_MAX+1];
  EditorWidget* saveEditorAs;
};

#endif
