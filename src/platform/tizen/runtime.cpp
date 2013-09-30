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
#include "platform/common/maapi.h"
#include "platform/common/utils.h"
#include "platform/common/form_ui.h"
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

RuntimeThread::RuntimeThread(int w, int h) :
  System(),
  _eventQueueLock(NULL),
  _eventQueue(NULL),
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

void RuntimeThread::buttonClicked(const char *url) {
  _loadPath.empty();
  _loadPath.append(url, strlen(url));
}

result RuntimeThread::Construct(String &appRootPath) {
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

MAEvent RuntimeThread::processEvents(bool waitFlag) {
  MAEvent event;

  if (!waitFlag) {
    showLoadError();
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
    dev_pushkey(SB_KEY_MK_RELEASE);
    _output->pointerReleaseEvent(event);
    break;
  default:
    // no event
    _output->flush(false);
    break;
  }
  return event;
}

char *RuntimeThread::readSource(const char *fileName) {
  char *buffer = NULL;
  bool networkFile = strstr(fileName, "://");

  if (networkFile) {
    int handle = 1;
    var_t *var_p = v_new();
    dev_file_t *f = dev_getfileptr(handle);
    systemPrint(fileName);
    if (dev_fopen(handle, fileName, 0)) {
      http_read(f, var_p, 0);
      int len = var_p->v.p.size;
      buffer = (char *)tmp_alloc(len + 1);
      memcpy(buffer, var_p->v.p.ptr, len);
      buffer[len] = '\0';
    }
    dev_fclose(handle);
    v_free(var_p);
    tmp_free(var_p);
  } else {
    int h = open(comp_file_name, O_BINARY | O_RDONLY, 0644);
    if (h != -1) {
      int len = lseek(h, 0, SEEK_END);
      lseek(h, 0, SEEK_SET);
      buffer = (char *)tmp_alloc(len + 1);
      read(h, buffer, len);
      buffer[len] = '\0';
      close(h);
    }
  }
  if (buffer != NULL) {
    delete [] _programSrc;
    int len = strlen(buffer);
    _programSrc = new char[len + 1];
    strncpy(_programSrc, buffer, len);
    _programSrc[len] = 0;
  }
  return buffer;
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

  _output = new AnsiWidget(this, _w, _h);
  _output->construct();
  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(DEFAULT_FONT_SIZE);
  _initialFontSize = _output->getFontSize();

  String mainBasPath = _appRootPath + "res/main.bas";
  setPath(_appRootPath);
  runMain(mainBasPath);

  delete _output;
  _state = kDoneState;
  logLeaving();
  App::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, NULL);
  return 0;
}

MAEvent RuntimeThread::getNextEvent() {
  return processEvents(true);
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
  if (!isBreak()) {
    thread->processEvents(true);
  }
}

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

void osd_cls(void) {
  logEntered();
  ui_reset();
  thread->_output->clearScreen();
}

int osd_devinit(void) {
  logEntered();
  thread->setRunning(true);
  setsysvar_str(SYSVAR_OSNAME, "Tizen");
  return 1;
}

int osd_devrestore(void) {
  ui_reset();
  thread->setRunning(false);
  return 0;
}

int osd_events(int wait_flag) {
  int result;
  if (thread->isBreak()) {
    result = -2;
  } else {
    thread->processEvents(wait_flag);
    result = 0;
  }
  return result;
}

int osd_getpen(int mode) {
  return thread->getPen(mode);
}

long osd_getpixel(int x, int y) {
  return thread->_output->getPixel(x, y);
}

int osd_getx(void) {
  return thread->_output->getX();
}

int osd_gety(void) {
  return thread->_output->getY();
}

void osd_line(int x1, int y1, int x2, int y2) {
  thread->_output->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (fill) {
    thread->_output->drawRectFilled(x1, y1, x2, y2);
  } else {
    thread->_output->drawRect(x1, y1, x2, y2);
  }
}

void osd_refresh(void) {
  if (!thread->isClosing()) {
    thread->_output->flush(true);
  }
}

void osd_setcolor(long color) {
  if (!thread->isClosing()) {
    thread->_output->setColor(color);
  }
}

void osd_setpenmode(int enable) {
  // touch mode is always active
}

void osd_setpixel(int x, int y) {
  thread->_output->setPixel(x, y, dev_fgcolor);
}

void osd_settextcolor(long fg, long bg) {
  thread->_output->setTextColor(fg, bg);
}

void osd_setxy(int x, int y) {
  thread->_output->setXY(x, y);
}

int osd_textheight(const char *str) {
  return thread->_output->textHeight();
}

int osd_textwidth(const char *str) {
  MAExtent textSize = maGetTextSize(str);
  return EXTENT_X(textSize);
}

void osd_write(const char *str) {
  if (!thread->isClosing()) {
    thread->_output->print(str);
  }
}

void lwrite(const char *str) {
  if (!thread->isClosing()) {
    thread->systemPrint(str);
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
  return thread->getText(dest, maxSize);
}

char *dev_read(const char *fileName) {
  return thread->readSource(fileName);
}
