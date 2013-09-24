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
#include "platform/common/interface.h"
#include "platform/common/ansiwidget.h"
#include "platform/common/system.h"

using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;

struct RuntimeThread : 
  public System,
  public IButtonListener,
  public Thread {

  RuntimeThread(int w, int h);
  ~RuntimeThread();

  void buttonClicked(const char *action);
  result Construct(String &resourcePath);
  bool hasEvent();
  MAEvent popEvent();
  void pushEvent(MAEvent event);
  int processEvents(bool waitFlag);
  void setExit(bool quit);

private:
  Object *Run();
  MAEvent getNextEvent();

  String _mainBasPath;
  Mutex *_eventQueueLock;
  Queue *_eventQueue;
  int _w, _h;
};

#endif
