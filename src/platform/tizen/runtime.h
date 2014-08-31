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

#include "ui/maapi.h"
#include "ui/interface.h"
#include "ui/ansiwidget.h"
#include "ui/system.h"

using namespace Tizen::App;
using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Ui;

struct RuntimeThread : 
  public System,
  public Thread {

  RuntimeThread();
  ~RuntimeThread();

  result Construct(String &resourcePath, int w, int h);
  void handleKey(MAEvent &event);
  bool hasEvent();
  MAEvent popEvent();
  void pushEvent(MAEvent event);
  MAEvent processEvents(int waitFlag);
  void setExit(bool quit);

private:
  Object *Run();
  MAEvent getNextEvent();

  String _appRootPath;
  Mutex *_eventQueueLock;
  Queue *_eventQueue;
};

#endif
