// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
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
#include "platform/android/jni/audio.h"

#include <android_native_app_glue.h>
#include <android/keycodes.h>
#include <android/sensor.h>

constexpr int MAX_SENSORS = 8;

struct Runtime : public System {
  explicit Runtime(android_app *app);
  ~Runtime() override;

  void addShortcut(const char *path) override { setString("addShortcut", path); }
  void alert(const char *title, const char *message) override;
  void alert(const char *title) { setString("showToast", title); }
  int ask(const char *title, const char *prompt, bool cancel) override;
  void browseFile(const char *url) override { setString("browseFile", url); }
  void clearSoundQueue();
  void construct();
  void debugStart(TextEditInput *edit, const char *file) {}
  void debugStep(TextEditInput *edit, TextEditHelpWidget *help, bool cont) {}
  void debugStop() {}
  void disableSensor();
  void enableCursor(bool enabled) override {}
  bool enableSensor(int sensorId);
  android_app *getApp() { return _app; }
  jlong getActivity() { return (jlong)_app->activity->clazz; }
  bool getBoolean(const char *methodName);
  String getString(const char *methodName);
  String getStringBytes(const char *methodName);
  int getInteger(const char *methodName);
  int getUnicodeChar(int keyCode, int metaState);
  void redraw() { _graphics->redraw(); }
  void handleKeyEvent(MAEvent &event);
  void pause(int timeout);
  MAEvent processEvents(int waitFlag) override;
  void processEvent(MAEvent &event);
  bool hasEvent() { return _eventQueue && !_eventQueue->empty(); }
  void playAudio(const char *path) { setString("playAudio", path); }
  void playTone(int frq, int dur, int vol, bool bgplay) { _audio.play(frq, dur, vol, bgplay); }
  void pollEvents(bool blocking);
  MAEvent *popEvent();
  void pushEvent(MAEvent *event);
  void setFloat(const char *methodName, float value);
  void setLocationData(var_t *retval);
  void setSensorData(var_t *retval);
  void setString(const char *methodName, const char *value);
  void speak(const char *text) { setString("speak", text); }
  void runShell();
  char *loadResource(const char *fileName) override;
  void optionsBox(StringList *items) override;
  void saveWindowRect() override {}
  void setWindowRect(int x, int y, int width, int height) override {};
  void setWindowTitle(const char *title) override {}
  void share(const char *path) override { setString("share", path); }
  void showCursor(CursorType cursorType) override {}
  void showKeypad(bool show);
  void onPaused(bool paused) { if (_graphics != NULL) _graphics->onPaused(paused); }
  void onResize(int w, int h);
  void onRunCompleted() override;
  void onUnicodeChar(int ch);
  void loadConfig();
  static void loadEnvConfig(Properties<String *> &settings, const char *key);
  bool loadSettings(Properties<String *> &settings);
  void saveConfig();
  void runPath(const char *path);
  void setClipboardText(const char *s) override { if (s) setString("setClipboardText", s); }
  char *getClipboardText() override;
  void setFocus(bool focus) { _hasFocus = focus; }
  int  getFontId();

private:
  bool _keypadActive;
  bool _hasFocus;
  Graphics *_graphics;
  android_app *_app;
  Stack<MAEvent *> *_eventQueue;
  pthread_mutex_t _mutex{};
  ALooper *_looper;
  ASensorManager *_sensorManager;
  const ASensor * _sensors[MAX_SENSORS]{};
  ASensorEventQueue *_sensorEventQueue;
  Audio _audio;
};

#endif
