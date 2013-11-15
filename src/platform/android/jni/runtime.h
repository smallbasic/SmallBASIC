// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef RUNTIME_H
#define RUNTIME_H

#include "config.h"
#include "platform/common/maapi.h"
#include "platform/common/interface.h"
#include "platform/common/ansiwidget.h"
#include "platform/common/system.h"
#include "platform/android/jni/display.h"

#include <android_native_app_glue.h>

struct Runtime :
  public System,
  public IButtonListener {

  Runtime(android_app *app);
  ~Runtime();

  bool construct();
  void buttonClicked(const char *action);
  void redraw() { _display->redraw(); }
  void handleKey(MAEvent &event);
  MAEvent getNextEvent();
  MAEvent processEvents(bool waitFlag);
  void setExit(bool quit);
  void runShell();

private:
  Viewable *_display;
  android_app *_app;
};

#endif
