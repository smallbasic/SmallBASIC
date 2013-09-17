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

#include "platform/mosync/interface.h"

using namespace Tizen::Base::Runtime;

struct Runtime : 
  public IButtonListener,
  public Thread {

  Runtime();
  ~Runtime();

  result Construct(int w, int h);
  void flush(bool force);
  void reset();
  void buttonClicked(const char *action);
  //void pushEvent(Common::EventType type, const Point &currentPosition);

private:
  Object *Run();

  enum ExecState {
    init_state,
    run_state,
    restart_state,
    modal_state,
    break_state,
    conn_state,
    back_state,
    exit_state
  };

  ExecState _runMode;

  const char *getLoadPath();
  bool isExit() { return _runMode == exit_state; }
  bool isBack() { return _runMode == back_state; }
  bool isModal() { return _runMode == modal_state; }
  bool isBreak() { return _runMode == exit_state || _runMode == back_state; }
  bool isRunning() { return _runMode == run_state || _runMode == modal_state; }
  void setExit(bool quit);
  void setRunning(bool running = true);
  void showError();
  void showCompletion(bool success);

  Tizen::Base::Runtime::Mutex *_eventQueueLock;
  //Common::Queue<Common::Event> _eventQueue;
  strlib::String _loadPath;
  char *_programSrc;

};

#endif
