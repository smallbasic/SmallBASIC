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

  result Construct();
  void buttonClicked(const char *action);
  void pushEvent(MAEvent event);
  void setExit(bool quit);
  void setRunning();

  bool isActive() { return _state != kInitState && _state != kDoneState; }
  bool isBack() { return _state == kBackState; }
  bool isBreak() { return _state == kClosingState || _state == kBackState; }
  bool isClosing() { return _state == kClosingState; }
  bool isInitial() { return _state == kInitState; }
  bool isModal() { return _state == kModalState; }
  bool isRunning() { return _state == kRunState || _state == kModalState; }

private:
  Object *Run();

  enum { 
    kInitState,    // thread not active
    kActiveState,  // thread activated
    kRunState,     // program is running
    kRestartState, // running program should restart
    kModalState,   // retrieving user input inside program
    kBreakState,   // running program should abort
    kConnState,    // retrieving remote program source
    kBackState,    // back button detected 
    kClosingState, // thread is terminating
    kDoneState     // thread has terminated
  } _state;

  const char *getLoadPath();
  void showError();
  void showCompletion(bool success);

  Mutex *_eventQueueLock;
  Queue *_eventQueue;
  strlib::String _loadPath;
  bool _drainError;
  char *_programSrc;
  int _w, _h;
};

#endif
