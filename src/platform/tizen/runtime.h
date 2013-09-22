// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef RUNTIME_H
#define RUNTIME_H

#include <FBase.h>

#include "platform/common/maapi.h"
#include "platform/mosync/interface.h"

using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;

struct RuntimeEvent : 
  public Tizen::Base::Object {
  MAEvent maEvent;
};

struct RuntimeThread : 
  public IButtonListener,
  public Thread {

  RuntimeThread(int w, int h);
  ~RuntimeThread();

  void buttonClicked(const char *action);
  result Construct();
  bool hasEvent();
  MAEvent popEvent();
  void pushEvent(MAEvent event);
  int processEvents(int waitFlag);
  void setExit(bool quit);
  void setRunning();

  bool isActive() { return _state != kInitState && _state != kDoneState; }
  bool isBack() { return _state == kBackState; }
  bool isBreak() { return _state >= kBreakState; }
  bool isClosing() { return _state == kClosingState; }
  bool isInitial() { return _state == kInitState; }
  bool isModal() { return _state == kModalState; }
  bool isRunning() { return _state == kRunState || _state == kModalState; }

private:
  Object *Run();

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

  const char *getLoadPath();
  void showCompletion(bool success);
  void showError();
  void showSystemScreen(bool showSrc);

  Mutex *_eventQueueLock;
  Queue *_eventQueue;
  strlib::String _loadPath;
  int _lastEventTime;
  int _eventTicks;
  bool _drainError;
  char *_programSrc;
  int _w, _h;
};

#endif
