// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include "config.h"
#include "ui/strlib.h"
#include "ui/ansiwidget.h"

#if defined(_FLTK)
  #include "platform/fltk/system.h"
#else

struct System {
  System();
  virtual ~System();

  int getPen(int code);
  char *getText(char *dest, int maxSize);
  bool isActive() { return _state != kInitState && _state != kDoneState; }
  bool isBack() { return _state == kBackState; }
  bool isBreak() { return _state >= kBreakState; }
  bool isClosing() { return _state >= kClosingState; }
  bool isInitial() { return _state == kInitState; }
  bool isModal() { return _state == kModalState; }
  bool isRestart() { return _state == kRestartState; }
  bool isRunning() { return _state == kRunState || _state == kModalState; }
  bool isSystemScreen() { return _userScreenId != -1; }
  char *readSource(const char *fileName);
  void setBack();
  void setExit(bool quit);
  void setLoadBreak(const char *path);
  void setLoadPath(const char *path);
  void setRunning(bool running);
  void systemPrint(const char *msg, ...);
  AnsiWidget *getOutput() { return _output; }

  virtual MAEvent processEvents(int waitFlag) = 0;
  virtual char *loadResource(const char *fileName);
  virtual void optionsBox(StringList *items) = 0;
  virtual void setWindowTitle(const char *title) = 0;
  virtual void showCursor(bool hand) = 0;
  virtual void setClipboardText(const char *text) = 0;
  virtual char *getClipboardText() = 0;

protected:
  void checkModifiedTime();
  void editSource();
  bool execute(const char *bas);
  MAEvent getNextEvent() { return processEvents(1); }
  uint32_t getModifiedTime();
  void handleEvent(MAEvent &event);
  void handleMenu(int menuId);
  void resize();
  void runMain(const char *mainBasPath);
  void runOnce(const char *startupBas);
  void setPath(const char *filename);
  bool setParentPath();
  void setDimensions();
  void showCompletion(bool success);
  void showError();
  void checkLoadError();
  void printErrorLine();
  void printSource();
  void printSourceLine(char *text, int line, bool last);
  void setRestart();
  void showSystemScreen(bool showSrc);
  void showMenu();
  AnsiWidget *_output;

  enum {
    kInitState = 0,// thread not active
    kActiveState,  // thread activated
    kRunState,     // program is running
    kModalState,   // retrieving user input inside program
    kConnState,    // retrieving remote program source
    kBreakState,   // running program should abort
    kRestartState, // running program should restart
    kBackState,    // back button detected
    kClosingState, // thread is terminating
    kDoneState     // thread has terminated
  } _state;

  strlib::String _loadPath;
  strlib::String _activeFile;
  int _lastEventTime;
  int _eventTicks;
  int _touchX;
  int _touchY;
  int _touchCurX;
  int _touchCurY;
  int _initialFontSize;
  int _fontScale;
  int _overruns;
  int _userScreenId;
  int *_systemMenu;
  bool _mainBas;
  bool _buttonPressed;
  bool _liveMode;
  bool _srcRendered;
  bool _menuActive;
  char *_programSrc;
  uint32_t _modifiedTime;
};

#endif
#endif
