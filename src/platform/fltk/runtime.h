// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FLTK_RUNTIME_H
#define FLTK_RUNTIME_H

#include "ui/ansiwidget.h"
#include "ui/system.h"

struct Runtime : public System {
  Runtime(int w, int h, int fontSize);
  virtual ~Runtime();

  void addShortcut(const char *) {}
  void alert(const char *title, const char *message);
  int ask(const char *title, const char *prompt, bool cancel=true);
  void browseFile(const char *url);
  char *getClipboardText();
  int  getFontSize() { return _output->getFontSize(); }
  void enableCursor(bool enabled);
  int handle(int event);
  void optionsBox(StringList *items);
  MAEvent processEvents(int waitFlag);
  bool run(const char *bas) { return execute(bas); }
  void runSamples();
  void resize(int w, int h);
  void setClipboardText(const char *text);
  void setFontSize(int size);
  void setLoadBreak(const char *url) {}
  void setLoadPath(const char *url) {}
  void setWindowSize(int width, int height);
  void setWindowTitle(const char *title);
  void share(const char *path) {}
  void showCursor(CursorType cursorType);
};

#endif
