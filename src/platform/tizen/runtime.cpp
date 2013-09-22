// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <FApp.h>
#include "platform/tizen/runtime.h"
#include "platform/common/maapi.h"
#include "platform/common/utils.h"
#include "platform/mosync/form_ui.h"
#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"

#define WAIT_INTERVAL 10
#define EVENT_CHECK_EVERY 2000
#define EVENT_MAX_BURN_TIME 30
#define EVENT_PAUSE_TIME 400

using namespace Tizen::App;
using namespace Tizen::Base::Utility;

// The runtime thread owns the ansiwidget which uses
// ma apis. The ma apis are handled in the main thread
AnsiWidget *output;
RuntimeThread *thread;
bool systemScreen;

//
// converts a Tizen (wchar) String into a StringLib (char) string
//
String fromString(const Tizen::Base::String &in) {
  Tizen::Base::ByteBuffer *buf = StringUtil::StringToUtf8N(in);
	String result((const char*)buf->GetPointer());
	delete buf;
	return result;
}

RuntimeThread::RuntimeThread(int w, int h) :
  _state(kInitState),
  _eventQueueLock(NULL),
  _eventQueue(NULL),
  _lastEventTime(0),
  _eventTicks(0),
  _drainError(false),
  _programSrc(NULL),
  _w(w),
  _h(h) {
  thread = this;
}

RuntimeThread::~RuntimeThread() {
  delete _eventQueueLock;
  delete _eventQueue;
  _eventQueueLock = NULL;
  _eventQueue = NULL;
}

void RuntimeThread::buttonClicked(const char *action) {
  logEntered();
}

result RuntimeThread::Construct() {
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
  return r;
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
  RuntimeEvent *event = new RuntimeEvent();
  event->maEvent = maEvent;
  _eventQueue->Enqueue(event);
  _eventQueueLock->Release();
}

int RuntimeThread::processEvents(int waitFlag) {
  if (thread->isBreak()) {
    return -2;
  }

  if (!waitFlag) {
    // detect when we have been called too frequently
    int now = maGetMilliSecondCount();
    _eventTicks++;
    if (now - _lastEventTime >= EVENT_CHECK_EVERY) {
      // next time inspection interval
      if (_eventTicks >= EVENT_MAX_BURN_TIME) {
        output->print("\033[ LBattery drain");
        _drainError = true;
      } else if (_drainError) {
        output->print("\033[ L");
        _drainError = false;
      }
      _lastEventTime = now;
      _eventTicks = 0;
    }
  }

  // pull events from the queue, invoke ansiwidget event handlers
  switch (waitFlag) {
  case 1:
    // wait for an event
    output->flush(true);
    maWait(0);
    break;
  case 2:
    // pause
    maWait(EVENT_PAUSE_TIME);
    break;
  default:
    // pump messages without pausing
    break;
  }

  if (hasEvent()) {
    MAEvent event = thread->popEvent();
    switch (event.type) {
    case EVENT_TYPE_POINTER_PRESSED:
      output->pointerTouchEvent(event);
      break;

    case EVENT_TYPE_POINTER_DRAGGED:
      output->pointerMoveEvent(event);
      break;

    case EVENT_TYPE_POINTER_RELEASED:
      output->pointerReleaseEvent(event);
      break;
    }
  }
  output->flush(false);
  return 0;
}

void RuntimeThread::setExit(bool quit) {
  if (_state != kDoneState) {
    _eventQueueLock->Acquire();
    if (isRunning()) {
      brun_break();
    }
    if (!isClosing()) {
      _state = quit ? kClosingState : kBackState;
    }
    _eventQueueLock->Release();
  }
}

void RuntimeThread::setRunning() {
  _state = kRunState;
  _loadPath.empty();
  _lastEventTime = maGetMilliSecondCount();
  _eventTicks = 0;
  _drainError = false;
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

  output = new AnsiWidget(this, _w, _h);
  output->construct();
  output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);

	String resourcePath = fromString(App::GetInstance()->GetAppResourcePath());
  String mainBasPath = resourcePath + "main.bas";

  strcpy(opt_command, "welcome");
  sbasic_main(mainBasPath);
  bool mainBas = true;
  while (!isClosing()) {
    if (isBack() || getLoadPath() == NULL) {
      if (mainBas) {
        setExit(!set_parent_path());
      }
      if (!isClosing()) {
        mainBas = true;
        opt_command[0] = '\0';
        sbasic_main(mainBasPath);
      }
    } else {
      mainBas = (strncmp(getLoadPath(), "main.bas", 8) == 0);
      if (!mainBas) {
        set_path(getLoadPath());
      }
      bool success = sbasic_main(getLoadPath());
      if (!isBack()) {
        if (!mainBas) {
          // display an indication the program has completed
          showCompletion(success);
        }
        if (!success) {
          // highlight the error
          showError();
        }
      }
    }
  }

  delete output;
  _state = kDoneState;
  logLeaving();
  App::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, NULL);
  return 0;
}

void RuntimeThread::showCompletion(bool success) {
  if (success) {
    output->print("\033[ LDone - press back [<-]");
  } else {
    output->print("\033[ LError - see console");
  }
  output->flush(true);
}

void RuntimeThread::showError() {
  _state = kActiveState;
  _loadPath.empty();
  showSystemScreen(false);
}

void RuntimeThread::showSystemScreen(bool showSrc) {
  if (showSrc) {
    // screen command write screen 2 (\014=CLS)
    output->print("\033[ SW6\014");
    if (_programSrc) {
      output->print(_programSrc);
    }
    // restore write screen, display screen 6 (source)
    output->print("\033[ Sw; SD6");
  } else {
    // screen command display screen 7 (console)
    output->print("\033[ SD7");
  }
  systemScreen = true;
}

const char *RuntimeThread::getLoadPath() {
  return !_loadPath.length() ? NULL : _loadPath.c_str();
}

//
// form_ui implementation
//
bool form_ui::isRunning() {
  return thread->isRunning();
}

bool form_ui::isBreak() {
  return thread->isBreak();
}

void form_ui::processEvents() {
  thread->processEvents(0);
}

void form_ui::buttonClicked(const char *url) {
  //buttonClicked(url);
}

struct Listener : IButtonListener {
  void buttonClicked(const char *action) {
    _action = action;
  }
  String _action;
};

void form_ui::optionsBox(StringList *items) {
  output->print("\033[ S#6");
  int y = 0;
  Listener listener;
  List_each(String *, it, *items) {
    char *str = (char *)(* it)->c_str();
    int w = osd_textwidth(str) + 20;
    IFormWidget *item = output->createButton(str, 2, y, w, 22);
    item->setListener(&listener);
    y += 24;
  }
  while (thread->isRunning() && !listener._action.length()) {
    osd_events(0);
  }
  int index = 0;
  List_each(String *, it, *items) {
    char *str = (char *)(* it)->c_str();
    if (strcmp(str, listener._action.c_str()) == 0) {
      break;
    } else {
      index++;
    }
  }
  output->print("\033[ SE6");
  output->optionSelected(index);
  maUpdateScreen();
}

AnsiWidget *form_ui::getOutput() {
  return output;
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
    if (thread->hasEvent()) {
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

void osd_cls(void) {
  logEntered();
  ui_reset();
  output->clearScreen();
}

int osd_devinit(void) {
  logEntered();
  dev_fgcolor = -DEFAULT_FOREGROUND;
  dev_bgcolor = -DEFAULT_BACKGROUND;
  os_graf_mx = output->getWidth();
  os_graf_my = output->getHeight();

  os_ver = 1;
  os_color = 1;
  os_color_depth = 16;
  setsysvar_str(SYSVAR_OSNAME, "Tizen");

  dev_clrkb();
  ui_reset();

  output->reset();
  thread->setRunning();

  return 1;
}

int osd_devrestore(void) {
  ui_reset();
  return 0;
}

int osd_events(int wait_flag) {
  return thread->processEvents(wait_flag);
}

int osd_getpen(int mode) {
  //return getPen(mode);
  return 0;
}

long osd_getpixel(int x, int y) {
  return output->getPixel(x, y);
}

int osd_getx(void) {
  return output->getX();
}

int osd_gety(void) {
  return output->getY();
}

void osd_line(int x1, int y1, int x2, int y2) {
  output->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (fill) {
    output->drawRectFilled(x1, y1, x2, y2);
  } else {
    output->drawRect(x1, y1, x2, y2);
  }
}

void osd_refresh(void) {
  output->flush(true);
}

void osd_setcolor(long color) {
  output->setColor(color);
}

void osd_setpenmode(int enable) {
  // touch mode is always active
}

void osd_setpixel(int x, int y) {
  output->setPixel(x, y, dev_fgcolor);
}

void osd_settextcolor(long fg, long bg) {
  output->setTextColor(fg, bg);
}

void osd_setxy(int x, int y) {
  output->setXY(x, y);
}

int osd_textheight(const char *str) {
  return output->textHeight();
}

int osd_textwidth(const char *str) {
  MAExtent textSize = maGetTextSize(str);
  return EXTENT_X(textSize);
}

void osd_write(const char *str) {
  logEntered();
  output->print(str);
}

void lwrite(const char *str) {
  AppLog(str);

  if (systemScreen) {
    output->print(str);
  } else {
    output->print("\033[ SW7");
    output->print(str);
    output->print("\033[ Sw");
  }
}

void dev_image(int handle, int index,
               int x, int y, int sx, int sy, int w, int h) {
}

int dev_image_width(int handle, int index) {
  return 0;
}

int dev_image_height(int handle, int index) {
  return 0;
}

void dev_delay(dword ms) {
  maWait(ms);
}

char *dev_gets(char *dest, int maxSize) {
  //return getText(dest, maxSize);
  return NULL;
}
