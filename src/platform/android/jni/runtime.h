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

struct Runtime : public System {
  Runtime(android_app *app);
  virtual ~Runtime();

  void clearSoundQueue();
  void construct();
  bool getUntrusted();
  String getStartupBas();
  int getUnicodeChar(int keyCode, int metaState);
  void redraw() { _graphics->redraw(); }
  void handleKeyEvent(MAEvent &event);
  MAEvent processEvents(int waitFlag);
  bool hasEvent() { return _eventQueue && _eventQueue->size() > 0; }
  void playTone(int frq, int dur, int vol, bool bgplay);
  void pollEvents(bool blocking);
  MAEvent *popEvent();
  void pushEvent(MAEvent *event);
  void setExit(bool quit);
  void runShell();
  char *loadResource(const char *fileName);
  void optionsBox(StringList *items);
  void showKeypad(bool show);
  void showAlert(const char *title, const char *message);
  void onResize(int w, int h);
  void loadConfig();
  void loadEnvConfig(Properties &profile, const char *key);
  void saveConfig();
  void runPath(const char *path);

private:
  bool _keypadActive;
  Graphics *_graphics;
  android_app *_app;
  Stack<MAEvent *> *_eventQueue;
  pthread_mutex_t _mutex;
  ALooper *_looper;
};

#endif
