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
#include <limits.h>

struct FileWidget;
extern FileWidget* fileWidget;

#ifdef CALLBACK_METHOD
#undef CALLBACK_METHOD
#endif

#define CALLBACK_METHOD(FN)                     \
  void FN(void *v=0);                           \
  static void FN ## _cb(Widget* w, void *v) {   \
    fileWidget->FN(v);                          \
  }

struct FileWidget : public HelpWidget {
  FileWidget(int x, int y, int w, int h);
  ~FileWidget();

  void displayPath();
  CALLBACK_METHOD(anchorClick);

  private:
  char path[PATH_MAX+1];
};

#endif
