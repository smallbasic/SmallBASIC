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

#define ACCESS_EXIST 0
#define ACCESS_WRITE 2
#define ACCESS_READ  4
#define PAUSE_TIME 500

// The runtime thread owns the ansiwidget which uses
// ma apis. The ma apis are handled in the main thread
AnsiWidget *output;
RuntimeThread *thread;

RuntimeThread::RuntimeThread(int w, int h) :
  _state(kInitState),
  _eventQueueLock(NULL),
  _eventQueue(NULL),
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

  sbasic_main("main.bas?welcome");
  bool mainBas = true;
  while (!isClosing()) {
    if (isBack()) {
      if (mainBas) {
        setExit(!set_parent_path());
      }
      if (!isClosing()) {
        mainBas = true;
        opt_command[0] = '\0';
        sbasic_main("main.bas");
      }
    } else if (getLoadPath() != NULL) {
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
    } else {
      output->flush(false);
      Sleep(PAUSE_TIME);
      // nothing to run
    }
  }

  delete output;
  _state = kDoneState;
  logLeaving();
  Tizen::App::App::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, NULL);
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

void RuntimeThread::showError() {
}

void RuntimeThread::showCompletion(bool success) {
}

const char *RuntimeThread::getLoadPath() {
  return !_loadPath.length() ? NULL : _loadPath.c_str();
}

void RuntimeThread::pushEvent(MAEvent maEvent) {
  _eventQueueLock->Acquire();
  RuntimeEvent *event = new RuntimeEvent();
  event->maEvent = maEvent;
  _eventQueue->Enqueue(event);
  _eventQueueLock->Release();
}

void RuntimeThread::setRunning() {
  _state = kRunState;
  _loadPath.empty();
  _drainError = false;
}

//
// form_ui implementation
//
bool form_ui::isRunning() {
  return isRunning();
}

bool form_ui::isBreak() {
  return isBreak();
}

void form_ui::processEvents() {
  //processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_EXIT_ANY);
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
  //  widget->_ansiWidget->print("\033[ S#6");
  //  int y = 0;
  //  Listener listener;
  //  List_each(String *, it, *items) {
  //    char *str = (char *)(* it)->c_str();
  //    int w = 0;//fltk::getwidth(str) + 20;
  //    IFormWidget *item = widget->_ansiWidget->createButton(str, 2, y, w, 22);
  //    item->setListener(&listener);
  //    y += 24;
  //  }
  //  while (form_ui::isRunning() && !listener._action.length()) {
  //    form_ui::processEvents();
  //  }
  //  int index = 0;
  //  List_each(String *, it, *items) {
  //    char *str = (char *)(* it)->c_str();
  //    if (strcmp(str, listener._action.c_str()) == 0) {
  //      break;
  //    } else {
  //      index++;
  //    }
  //  }
  //  widget->_ansiWidget->print("\033[ SE6");
  //  widget->_ansiWidget->optionSelected(index);
  //widget->redraw();
}

AnsiWidget *form_ui::getOutput() {
  return output;
}

//
// event handling
//

int maGetEvent(MAEvent *event) {
  int result = 0;
  /*
    if (check()) {
    switch (fltk::event()) {
    case PUSH:
    event->type = EVENT_TYPE_POINTER_PRESSED;
    result = 1;
    break;
    case DRAG:
    event->type = EVENT_TYPE_POINTER_DRAGGED;
    result = 1;
    break;
    case RELEASE:
    event->type = EVENT_TYPE_POINTER_RELEASED;
    result = 1;
    break;
    }
    }
  */
  return result;
}

void maWait(int timeout) {
  //fltk::wait(timeout);
}

//
// sbasic implementation
//

void osd_sound(int frq, int dur, int vol, int bgplay) {

}

void osd_clear_sound_queue() {

}

void osd_beep(void) {
  output->beep();
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
  //return handleEvents(wait_flag);
  /*
    switch (wait_flag) {
    case 1:
    // wait for an event
    wnd->_out->flush(true);
    fltk::wait();
    break;
    case 2:
    // pause
    fltk::wait(EVT_PAUSE_TIME);
    break;
    default:
    // pump messages without pausing
    fltk::check();
    }

    if (wnd->isBreakExec()) {
    clearOutput();
    return -2;
    }

    wnd->_out->flush(false);

    case PUSH:
    event.point.x = fltk::event_x();
    event.point.y = fltk::event_y();
    mouseActive = _ansiWidget->pointerTouchEvent(event);
    return mouseActive;

    case DRAG:
    case MOVE:
    event.point.x = fltk::event_x();
    event.point.y = fltk::event_y();
    if (mouseActive && _ansiWidget->pointerMoveEvent(event)) {
    Widget::cursor(fltk::CURSOR_HAND);
    return 1;
    }
    break;

    case RELEASE:
    if (mouseActive) {
    mouseActive = false;
    Widget::cursor(fltk::CURSOR_DEFAULT);
    event.point.x = fltk::event_x();
    event.point.y = fltk::event_y();
    _ansiWidget->pointerReleaseEvent(event);
    }
    break;
    }
  */
  return 0;
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

char *dev_read(const char *fileName) {
  //return readSource(fileName);
  // TODO: does fopen work in tizen?
  return NULL;
}

void lwrite(const char *str) {
  AppLog(str);
  //output->print("\033[ SW7");
  output->print(str);
  //output->print("\033[ Sw");
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
  //processEvents(ms, EVENT_TYPE_EXIT_ANY);
}

char *dev_gets(char *dest, int maxSize) {
  //return getText(dest, maxSize);
  return NULL;
}

extern "C" int dev_clock() {
  return 0;
}
