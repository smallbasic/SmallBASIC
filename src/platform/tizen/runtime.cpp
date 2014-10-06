// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <FApp.h>
#include <FUi.h>
#include "platform/tizen/runtime.h"
#include "ui/maapi.h"
#include "ui/utils.h"
#include "ui/form.h"
#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"
#include "common/fs_socket_client.h"

#define WAIT_INTERVAL 10
#define DEFAULT_FONT_SIZE 32

struct RuntimeEvent : 
  public Tizen::Base::Object {
  MAEvent maEvent;
};

// The runtime thread owns the ansiwidget which uses
// ma apis. The ma apis are handled in the main thread
RuntimeThread *thread;

RuntimeThread::RuntimeThread() :
  System(),
  _eventQueueLock(NULL),
  _eventQueue(NULL) {
  thread = this;
}

RuntimeThread::~RuntimeThread() {
  delete _eventQueueLock;
  delete _eventQueue;
  _eventQueueLock = NULL;
  _eventQueue = NULL;
}

result RuntimeThread::Construct(String &appRootPath, int w, int h) {
  logEntered();
  result r = Thread::Construct();
  if (!IsFailed(r)) {
    _eventQueueLock = new Mutex();
    r = _eventQueueLock != NULL ? _eventQueueLock->Create() : E_OUT_OF_MEMORY;
  }
  if (!IsFailed(r)) {
    _eventQueue = new Queue();
    r = _eventQueue != NULL ? _eventQueue->Construct() : E_OUT_OF_MEMORY;
  }
  if (!IsFailed(r)) {
    _appRootPath = appRootPath;
    _output = new AnsiWidget(this, w, h);
    if (!_output) {
      r = E_OUT_OF_MEMORY;
    }
  }
  return r;
}

void RuntimeThread::handleKey(MAEvent &event) {
  bool pushed = false;
  switch ((KeyCode) event.nativeKey) {
  case KEY_TAB:
    event.key = SB_KEY_TAB;
    break;
  case KEY_HOME:
    event.key = SB_KEY_KP_HOME;
    break;
  case KEY_MOVE_END:
    event.key = SB_KEY_END;
    break;
  case KEY_INSERT:
    event.key = SB_KEY_INSERT;
    break;
  case KEY_NUMPAD_MULTIPLY:
    dev_pushkey(SB_KEY_KP_MUL);
    pushed = true;
    break;
  case KEY_NUMPAD_ADD:
    dev_pushkey(SB_KEY_KP_PLUS);
    pushed = true;
    break;
  case KEY_NUMPAD_SUBTRACT:
    dev_pushkey(SB_KEY_KP_MINUS);
    pushed = true;
    break;
  case KEY_SLASH:
    dev_pushkey(SB_KEY_KP_DIV);
    pushed = true;
    break;
  case KEY_PAGE_UP:
    event.key = SB_KEY_PGUP;
    break;
  case KEY_PAGE_DOWN:
    event.key = SB_KEY_PGDN;
    break;
  case KEY_UP:
    event.key = SB_KEY_UP;
    break;
  case KEY_DOWN:
    event.key = SB_KEY_DN;
    break;
  case KEY_LEFT:
    event.key = SB_KEY_LEFT;
    break;
  case KEY_RIGHT:
    event.key = SB_KEY_RIGHT;
    break;
  case KEY_CLEAR:
  case KEY_BACKSPACE:
  case KEY_DELETE:
    event.key = MAK_CLEAR;
    break;
  case KEY_ENTER:
    event.key = SB_KEY_ENTER;
    break;
  default:
    break;
  }
  if (!pushed) {
    dev_pushkey(event.key);
  }
}

bool RuntimeThread::hasEvent() {
  _eventQueueLock->Acquire();
  bool result = _eventQueue->GetCount() > 0; 
  _eventQueueLock->Release();
  return result;
}

MAEvent RuntimeThread::popEvent() {
  _eventQueueLock->Acquire();
  RuntimeEvent *event = (RuntimeEvent *)_eventQueue->Dequeue();
  _eventQueueLock->Release();

  MAEvent result = event->maEvent;
  delete event;
  return result;
}

void RuntimeThread::pushEvent(MAEvent maEvent) {
  _eventQueueLock->Acquire();
  bool addItem = true;
  if (maEvent.type == EVENT_TYPE_POINTER_DRAGGED &&
      _eventQueue->GetCount() > 0) {
    RuntimeEvent *previous = (RuntimeEvent *)_eventQueue->Peek();
    if (previous->maEvent.type == EVENT_TYPE_POINTER_DRAGGED) {
      addItem = false;
    }
  }
  if (addItem) {
    RuntimeEvent *event = new RuntimeEvent();
    event->maEvent = maEvent;
    _eventQueue->Enqueue(event);
  }
  _eventQueueLock->Release();
}

MAEvent RuntimeThread::processEvents(int waitFlag) {
  MAEvent event;

  if (!waitFlag) {
    checkLoadError();
  } else {
    // wait for an event
    _output->flush(true);
    maWait(-1);
  }

  _eventQueueLock->Acquire();
  if (_eventQueue->GetCount() > 0) {
    RuntimeEvent *runtimeEvent = (RuntimeEvent *)_eventQueue->Dequeue();
    event = runtimeEvent->maEvent;
    delete runtimeEvent;
  } else {
    event.type = 0;
  }
  _eventQueueLock->Release();

  switch (event.type) {
  case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
    if (_systemMenu) {
      handleMenu(event.optionsBoxButtonIndex);
    } else if (isRunning()) {
      if (!_output->optionSelected(event.optionsBoxButtonIndex)) {
        dev_pushkey(event.optionsBoxButtonIndex);
      }
    }
    break;
  case EVENT_TYPE_SCREEN_CHANGED:
    resize();
    break;
  case EVENT_TYPE_KEY_PRESSED:
    if (event.nativeKey == KEY_CONTEXT_MENU) {
      showMenu();
    } else if (isRunning()) {
      handleKey(event);
    }
    break;
  case EVENT_TYPE_POINTER_PRESSED:
    _touchX = _touchCurX = event.point.x;
    _touchY = _touchCurY = event.point.y;
    dev_pushkey(SB_KEY_MK_PUSH);
    _output->pointerTouchEvent(event);
    break;
  case EVENT_TYPE_POINTER_DRAGGED:
    _touchCurX = event.point.x;
    _touchCurY = event.point.y;
    _output->pointerMoveEvent(event);
    break;
  case EVENT_TYPE_POINTER_RELEASED:
    _touchX = _touchY = _touchCurX = _touchCurY = -1;
    _output->pointerReleaseEvent(event);
    break;
  default:
    // no event
    _output->flush(false);
    break;
  }
  return event;
}

void RuntimeThread::setExit(bool quit) {
  if (!isClosing()) {
    _eventQueueLock->Acquire();
    _state = quit ? kClosingState : kBackState;
    if (isRunning()) {
      brun_break();
    }
    _eventQueueLock->Release();
  }
}

Tizen::Base::Object *RuntimeThread::Run() {
  logEntered();

  _state = kActiveState;
  opt_ide = IDE_NONE;
  opt_graphics = true;
  opt_pref_bpp = 0;
  opt_nosave = true;
  opt_interactive = true;
  opt_verbose = false;
  opt_quiet = true;
  opt_command[0] = 0;
  opt_usevmt = 0;
  os_graphics = 1;
  opt_file_permitted = 1;

  _output->construct();
  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(DEFAULT_FONT_SIZE);
  _initialFontSize = _output->getFontSize();

  String mainBasPath = _appRootPath + "res/main.bas";
  setPath(_appRootPath + "res/samples/");
  runMain(mainBasPath);

  delete _output;
  _state = kDoneState;
  logLeaving();
  App::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, NULL);
  return 0;
}

//
// form_ui implementation
//
void form_ui::buttonClicked(const char *url) {
  thread->buttonClicked(url);
}

AnsiWidget *form_ui::getOutput() {
  return thread->_output;
}

//
// ma event handling
//
int maGetEvent(MAEvent *event) {
  int result;
  if (thread->hasEvent()) {
    MAEvent nextEvent = thread->popEvent();
    event->point = nextEvent.point;
    event->type = nextEvent.type;
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

void maWait(int timeout) {
  int slept = 0;
  while (1) {
    if (thread->hasEvent()
        || thread->isBack()
        || thread->isClosing()) {
      break;
    }
    thread->Sleep(WAIT_INTERVAL);
    slept += WAIT_INTERVAL;
    if (timeout > 0 && slept > timeout) {
      break;
    }
  }
}

//
// sbasic implementation
//
void osd_sound(int frq, int dur, int vol, int bgplay) {
}

void osd_clear_sound_queue() {
}

void osd_beep(void) {
}

int osd_devinit(void) {
  logEntered();
  thread->setRunning(true);
  setsysvar_str(SYSVAR_OSNAME, "Tizen");
  return 1;
}

