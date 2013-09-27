// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include "platform/common/StringLib.h"
#include "platform/common/ansiwidget.h"

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
  bool isRunning() { return _state == kRunState || _state == kModalState; }
  bool isSystemScreen() { return _systemScreen; }
  void setBack();
  void setRunning(bool running);
  void systemPrint(const char *msg);

  AnsiWidget *_output;

protected:
  virtual void setExit(bool quit) = 0;
  virtual MAEvent getNextEvent() = 0;

  void handleMenu(int menuId);
  void runMain(const char *mainBasPath);
  void setPath(const char *filename);
  bool setParentPath();
  const char *getLoadPath();
  void showCompletion(bool success);
  void showError();
  void showSystemScreen(bool showSrc);
  void showMenu();

  enum { 
    kInitState = 0,// thread not active
    kActiveState,  // thread activated
    kRunState,     // program is running
    kRestartState, // running program should restart
    kModalState,   // retrieving user input inside program
    kConnState,    // retrieving remote program source
    kBreakState,   // running program should abort
    kBackState,    // back button detected 
    kClosingState, // thread is terminating
    kDoneState     // thread has terminated
  } _state;


  strlib::String _loadPath;
  int _lastEventTime;
  int _eventTicks;
  int _initialFontSize;
  int _fontScale;
  bool _drainError;
  bool _systemMenu;
  bool _systemScreen;
  bool _mainBas;
  char *_programSrc;
};

#endif
