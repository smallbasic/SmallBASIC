// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <android/native_window.h>

#include "platform/android/jni/runtime.h"
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
#define DEFAULT_FONT_SIZE 30
#define MAIN_BAS "__main_bas__"

Runtime *runtime;

int32_t handleInput(android_app *app, AInputEvent *event) {
  int32_t result = 0;
  if (runtime->isActive()) {
    MAEvent *maEvent = NULL;
    switch (AInputEvent_getType(event)) {
    case AINPUT_EVENT_TYPE_MOTION:
      switch (AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK) {
      case AMOTION_EVENT_ACTION_DOWN:
        maEvent = new MAEvent();
        maEvent->type = EVENT_TYPE_POINTER_PRESSED;
        maEvent->point.x = AMotionEvent_getX(event, 0);
        maEvent->point.y = AMotionEvent_getY(event, 0);
        break;
      case AMOTION_EVENT_ACTION_UP:
        maEvent = new MAEvent();
        maEvent->type = EVENT_TYPE_POINTER_RELEASED;
        maEvent->point.x = AMotionEvent_getX(event, 0);
        maEvent->point.y = AMotionEvent_getY(event, 0);
        break;
      case AMOTION_EVENT_ACTION_MOVE:
        maEvent = new MAEvent();
        maEvent->type = EVENT_TYPE_POINTER_DRAGGED;
        maEvent->point.x = AMotionEvent_getX(event, 0);
        maEvent->point.y = AMotionEvent_getY(event, 0);
        break;
      }
      break;
    case AINPUT_EVENT_TYPE_KEY:
      maEvent = new MAEvent();
      maEvent->type = EVENT_TYPE_KEY_PRESSED;
      maEvent->nativeKey = AKeyEvent_getKeyCode(event);
      maEvent->key = AKeyEvent_getKeyCode(event);
      break;
    }
    if (maEvent != NULL) {
      result = 1;
      runtime->pushEvent(maEvent);
    }
  }
  return result;
}

void handleCommand(android_app *app, int32_t cmd) {
  trace("handleCommand = %d", cmd);
  switch (cmd) {
  case APP_CMD_INIT_WINDOW:
    // thread is ready to start
    runtime->construct();
    break;
  case APP_CMD_TERM_WINDOW:
    if (runtime) {
      // thread is ending
      runtime->setExit(true);
    }
    break;
  case APP_CMD_LOST_FOCUS:
    break;
  }
}

Runtime::Runtime(android_app *app) : 
  System(),
  _app(app) {
  _app->userData = NULL;
  _app->onAppCmd = handleCommand;
  _app->onInputEvent = handleInput;
  runtime = this;
}

Runtime::~Runtime() {
  logEntered();
  delete _output;
  delete _eventQueue;
  delete _graphics;
  runtime = NULL;
  _output = NULL;
  _eventQueue = NULL;
  _graphics = NULL;
}

void Runtime::buttonClicked(const char *url) {
  _loadPath.empty();
  _loadPath.append(url, strlen(url));
}

void Runtime::construct() {
  logEntered();
  _state = kClosingState;
  _graphics = new Graphics(_app);
  if (_graphics && _graphics->construct()) {
    int w = ANativeWindow_getWidth(_app->window);
    int h = ANativeWindow_getHeight(_app->window);
    _output = new AnsiWidget(this, w, h);
    if (_output && _output->construct()) {
      _eventQueue = new Stack<MAEvent *>();
      if (_eventQueue) {
        _state = kActiveState;
      }
    }
  }
}

char *Runtime::loadResource(const char *fileName) {
  char *buffer = System::loadResource(fileName);
  if (buffer == NULL && strcmp(fileName, MAIN_BAS) == 0) {
    AAssetManager *assetManager = _app->activity->assetManager;
    AAsset *mainBasFile = AAssetManager_open(assetManager, "main.bas", AASSET_MODE_BUFFER);
    off_t len = AAsset_getLength(mainBasFile);
    buffer = (char *)tmp_alloc(len + 1);
    if (AAsset_read(mainBasFile, buffer, len) < 0) {
      trace("failed to read main.bas");
    }
    buffer[len] = '\0';
    trace("loaded main.bas [%ld] bytes", len);
    AAsset_close(mainBasFile);
  }
  return buffer;
}

void Runtime::runShell() {
  logEntered();

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

  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(DEFAULT_FONT_SIZE);
  _initialFontSize = _output->getFontSize();

  trace("internalDataPath=%s", _app->activity->internalDataPath);
  runMain(MAIN_BAS);

  _state = kDoneState;
  logLeaving();
}

void Runtime::handleKeyEvent(MAEvent &event) {
  trace("key = %d", event.nativeKey);
  switch (event.nativeKey) {
  case AKEYCODE_BACK:
    setBack();
    break;
  case AKEYCODE_MENU:
    showMenu();
    break;
  default:
    if (isRunning()) {
      
    }
  }
}

void Runtime::optionsBox(StringList *items) {
  logEntered();
}

void Runtime::pollEvents(bool blocking) {
  int events;
  android_poll_source *source;
  ALooper_pollAll(blocking ? -1 : 0, NULL, &events, (void **)&source);
  if (source != NULL) {
    source->process(_app, source);
  }
  if (_app->destroyRequested != 0) {
    trace("Thread destroy requested");
    setExit(true);
  }
}

MAEvent Runtime::processEvents(bool waitFlag) {
  if (!waitFlag) {
    showLoadError();
  } else {
    _output->flush(true);
  }
  pollEvents(waitFlag);

  MAEvent event;
  if (hasEvent()) {
    MAEvent *nextEvent = popEvent();
    event = *nextEvent;
    delete nextEvent;
  } else {
    event.type = 0;
  }

  if (event.type == EVENT_TYPE_KEY_PRESSED) {
    handleKeyEvent(event);
  } else {
    handleEvent(event);
  }
  return event;
}

void Runtime::setExit(bool quit) {
  if (!isClosing()) {
    _state = quit ? kClosingState : kBackState;
    if (isRunning()) {
      brun_break();
    }
  }
}

//
// form_ui implementation
//
void form_ui::buttonClicked(const char *url) {
  runtime->buttonClicked(url);
}

AnsiWidget *form_ui::getOutput() {
  return runtime->_output;
}

void form_ui::optionsBox(StringList *items) {
  runtime->optionsBox(items);
}

//
// ma event handling
//
int maGetEvent(MAEvent *event) {
  int result;
  if (runtime->hasEvent()) {
    MAEvent *nextEvent = runtime->popEvent();
    event->point = nextEvent->point;
    event->type = nextEvent->type;
    delete nextEvent;
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

void maWait(int timeout) {
  if (timeout == -1) {
    runtime->pollEvents(true);
  } else {
    int slept = 0;
    while (1) {
      runtime->pollEvents(false);
      if (runtime->hasEvent()
          || runtime->isBack()
          || runtime->isClosing()) {
        break;
      }
      usleep(WAIT_INTERVAL);
      slept += WAIT_INTERVAL;
      if (timeout > 0 && slept > timeout) {
        break;
      }
    }
  }
}

int maGetMilliSecondCount(void) {
  struct timespec t;
  t.tv_sec = t.tv_nsec = 0;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (int)(((int64_t)t.tv_sec) * 1000000000LL + t.tv_nsec)/1000000;
}

int maShowVirtualKeyboard(void) {
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
}

//
// sbasic implementation
//
int osd_devinit(void) {
  setsysvar_str(SYSVAR_OSNAME, "Android");
  runtime->setRunning(true);
  return 1;
}

void osd_sound(int frq, int dur, int vol, int bgplay) {
}

void osd_clear_sound_queue() {
}

void osd_beep(void) {
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


