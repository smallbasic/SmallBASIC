// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include "ui/system.h"
#include <emscripten/html5.h>

struct Runtime : public System {
  Runtime();
  virtual ~Runtime();

  void addShortcut(const char *) {}
  void alert(const char *title, const char *message);
  int  ask(const char *title, const char *prompt, bool cancel=true);
  void browseFile(const char *url);
  char *getClipboardText();
  int  getFontSize() { return _output->getFontSize(); }
  void editSource(String loadPath, bool restoreOnExit) override;
  void externalExecute(const char *bas) const override {}
  void enableCursor(bool enabled) {}
  int  handle(int event);
  char *loadResource(const char *fileName);
  void openFolder() override {}
  void onRunCompleted() {}
  void saveWindowRect() {}
  bool handleKeyboard(int eventType, const EmscriptenKeyboardEvent *e);
  bool handleMouse(int eventType, const EmscriptenMouseEvent *e);
  bool hasEvent() { return _eventQueue && _eventQueue->size() > 0; }
  void pause(int timeout);
  void pushEvent(MAEvent *event) { _eventQueue->push(event); }
  MAEvent *popEvent() { return _eventQueue->pop(); }
  MAEvent processEvents(int waitFlag);
  void processEvent(MAEvent &event);
  bool run(const char *bas) { return execute(bas); }
  void runShell();
  void setClipboardText(const char *text);
  void setFontSize(int size);
  void setLoadBreak(const char *url) {}
  void setLoadPath(const char *url) {}
  void setWindowRect(int x, int y, int width, int height) {}
  void setWindowTitle(const char *title) {}
  void share(const char *path) {}
  void showCursor(CursorType cursorType);

private:
  Stack<MAEvent *> *_eventQueue;
};
