// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef RUNTIME_H
#define RUNTIME_H

#include "config.h"
#include "lib/maapi.h"
#include "ui/inputs.h"
#include "ui/system.h"
#include "platform/android/jni/display.h"

#include <android_native_app_glue.h>

struct Runtime : public System {
  Runtime(android_app *app);
  virtual ~Runtime();

  void alert(const char *title, const char *message, bool cancel);
  int ask(const char *title, const char *prompt);
  void clearSoundQueue();
  void construct();
  bool getUntrusted();
  String getString(const char *methodName);
  int getInteger(const char *methodName);
  int getUnicodeChar(int keyCode, int metaState);
  void redraw() { _graphics->redraw(); }
  void handleKeyEvent(MAEvent &event);
  void pause(int timeout);
  MAEvent processEvents(int waitFlag);
  void processEvent(MAEvent &event);
  bool hasEvent() { return _eventQueue && _eventQueue->size() > 0; }
  void playTone(int frq, int dur, int vol, bool bgplay);
  void pollEvents(bool blocking);
  MAEvent *popEvent();
  void pushEvent(MAEvent *event);
  void setString(const char *methodName, const char *value);
  void runShell();
  char *loadResource(const char *fileName);
  void optionsBox(StringList *items);
  void setWindowTitle(const char *title) {}
  void showCursor(bool hand) {}
  void showKeypad(bool show);
  void onResize(int w, int h);
  void loadConfig();
  void loadEnvConfig(Properties &profile, const char *key);
  void saveConfig();
  void runPath(const char *path);
  void setClipboardText(const char *text);
  char *getClipboardText();

private:
  bool _keypadActive;
  Graphics *_graphics;
  android_app *_app;
  Stack<MAEvent *> *_eventQueue;
  pthread_mutex_t _mutex;
  ALooper *_looper;
};

#endif
