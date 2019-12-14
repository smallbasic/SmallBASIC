// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef Fl_HELP_VIEW
#define Fl_HELP_VIEW

#include "platform/fltk/HelpWidget.h"

const char *getBriefHelp(const char *selection);

struct HelpView : public HelpWidget {
  HelpView(Fl_Widget *rect, int fontSize);
  ~HelpView();

  void about();
  void anchorClick();
  void helpIndex();
  bool loadHelp(const char *path);
  void showContextHelp(const char *selection);

private:
  void showHelp(const char *node);

  int _openKeyword;
  int _openPackage;
};

#endif
