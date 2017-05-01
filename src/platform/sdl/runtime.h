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
#include <SDL.h>

#include "lib/maapi.h"
#include "ui/ansiwidget.h"
#include "ui/system.h"
#include "platform/sdl/display.h"

struct Runtime : public System {
  Runtime(SDL_Window *window);
  virtual ~Runtime();

  void addShortcut(const char *) {}
  void alert(const char *title, const char *message);
  int ask(const char *title, const char *prompt, bool cancel);
  void browseFile(const char *url);
  void construct(const char *font, const char *boldFont);
  void debugStart(TextEditInput *edit, const char *file);
  void debugStep(TextEditInput *edit, TextEditHelpWidget *help, bool cont);
  void debugStop();
  void enableCursor(bool enabled);
  void redraw() { _graphics->redraw(); }
  void handleKeyEvent(MAEvent &event);
  bool hasEvent() { return _eventQueue && _eventQueue->size() > 0; }
  void pause(int timeout);
  void pollEvents(bool blocking);
  MAEvent *popEvent();
  MAEvent processEvents(int waitFlag);
  void processEvent(MAEvent &event);
  void pushEvent(MAEvent *event);
  void setWindowTitle(const char *title);
  void share(const char *path) {}
  void showCursor(CursorType cursorType);
  int runShell(const char *startupBas, int fontScale, int debugPort);
  char *loadResource(const char *fileName);
  void optionsBox(StringList *items);
  void onResize(int w, int h);
  void setClipboardText(const char *text);
  char *getClipboardText();

private:
  int _menuX, _menuY;
  Graphics *_graphics;
  Stack<MAEvent *> *_eventQueue;
  SDL_Window *_window;
  SDL_Cursor *_cursorHand;
  SDL_Cursor *_cursorArrow;
  SDL_Cursor *_cursorIBeam;
};

#endif
