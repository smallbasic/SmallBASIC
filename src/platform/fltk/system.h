// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FLTK_SYSTEM_H
#define FLTK_SYSTEM_H

struct System {
  bool isBreak();
  bool isRunning();
  void setLoadPath(const char *url);
  void setLoadBreak(const char *url);
  MAEvent processEvents(bool wait);
  void optionsBox(StringList *items);
  void systemPrint(const char *message, ...);
  AnsiWidget *getOutput();
};

#endif
