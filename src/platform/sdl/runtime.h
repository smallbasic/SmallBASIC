// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef RUNTIME_H
#define RUNTIME_H

#include "config.h"

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>

#include "lib/maapi.h"
#include "ui/ansiwidget.h"
#include "ui/system.h"
#include "platform/sdl/display.h"

struct Runtime : public System {
  explicit Runtime(SDL_Window *window);
  ~Runtime() override;

  void addShortcut(const char *) override {}
  void alert(const char *title, const char *message) override;
  int ask(const char *title, const char *prompt, bool cancel) override;
  void browseFile(const char *url) override;
  void construct(const char *font, const char *boldFont);
  bool debugActive();
  bool debugOpen(const char *file);
  void debugStart(TextEditInput *edit, const char *file);
  void debugStep(TextEditInput *edit, TextEditHelpWidget *help, bool cont);
  void debugStop();
  void enableCursor(bool enabled) override;
  void exportRun(const char *path);
  void redraw() { _graphics->redraw(); }
  bool toggleFullscreen();
  void handleKeyEvent(MAEvent &event);
  bool hasEvent() { return _eventQueue && _eventQueue->size() > 0; }
  void pause(int timeout);
  void pollEvents(bool blocking);
  MAEvent *popEvent() { return _eventQueue->pop(); }
  MAEvent processEvents(int waitFlag) override;
  void processEvent(MAEvent &event);
  void pushEvent(MAEvent *event) { _eventQueue->push(event); }
  void saveWindowRect() override;
  void setWindowRect(int x, int y, int width, int height) override;
  void setWindowTitle(const char *title) override;
  void share(const char *path) override {}
  void showCursor(CursorType cursorType) override;
  int runShell(const char *startupBas, bool once, int fontScale, int debugPort);
  char *loadResource(const char *fileName) override;
  void logStack(int line, bool subOrFunc);
  void onResize(int w, int h);
  void onRunCompleted() override;
  void setClipboardText(const char *text) override;
  char *getClipboardText() override;
  void setWindowRect(SDL_Rect &rect);
  SDL_Rect getWindowRect();

private:
  bool _fullscreen;
  SDL_Rect _windowRect;
  SDL_Rect _saveRect;
  Graphics *_graphics;
  Stack<MAEvent *> *_eventQueue;
  SDL_Window *_window;
  SDL_Cursor *_cursorHand;
  SDL_Cursor *_cursorArrow;
  SDL_Cursor *_cursorIBeam;
};

#endif
