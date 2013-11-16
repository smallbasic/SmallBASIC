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
#define DEFAULT_FONT_SIZE 32
Runtime *runtime;

static int32_t handle_input(struct android_app *app, AInputEvent *event) {
  Runtime *runtime = (Runtime *)app->userData;
  int32_t result;

  trace("Key event: action=%d keyCode=%d metaState=0x%x",
        AKeyEvent_getAction(event),
        AKeyEvent_getKeyCode(event),
        AKeyEvent_getMetaState(event));

  switch (AInputEvent_getType(event)) {
  case AINPUT_EVENT_TYPE_MOTION:
    //engine->animating = 1;
    result = 1;
    break;
  case AINPUT_EVENT_TYPE_KEY:
    result = 0;
    break;
  }
  return result;
}

static void handle_cmd(struct android_app *app, int32_t cmd) {
  Runtime *runtime = (Runtime *)app->userData;
  switch (cmd) {
  case APP_CMD_INIT_WINDOW:
    runtime->redraw();
    break;
  case APP_CMD_TERM_WINDOW:
    //engine_term_display(engine);
    break;
  case APP_CMD_LOST_FOCUS:
    //engine->animating = 0;
    //engine_draw_frame(engine);
    break;
  }
}

Runtime::Runtime(android_app *app) : 
  _app(app) {
  runtime = this;
}

Runtime::~Runtime() {
  delete _output;
  delete _eventQueue;
  runtime = NULL;
}

void Runtime::buttonClicked(const char *url) {
  _loadPath.empty();
  _loadPath.append(url, strlen(url));
}

bool Runtime::construct() {
  bool result = true;
  
  int w = ANativeWindow_getWidth(_app->window);
  int h = ANativeWindow_getHeight(_app->window);
  _output = new AnsiWidget(this, w, h);
  if (_output) {
    _app->userData = this;
    _app->onAppCmd = handle_cmd;
    _app->onInputEvent = handle_input;
    _eventQueue = new Stack<MAEvent *>();
  } else {
    result = false;
  }
  return result;
}

void Runtime::runShell() {
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

  _output->construct();
  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(DEFAULT_FONT_SIZE);
  _initialFontSize = _output->getFontSize();

  String basePath = _app->activity->internalDataPath;
  String mainBasPath = basePath + "res/main.bas";
  setPath(basePath + "res/samples/");
  runMain(mainBasPath);

  delete _output;
  _state = kDoneState;
  logLeaving();
}

void Runtime::handleKey(MAEvent &event) {
}

MAEvent Runtime::processEvents(bool waitFlag) {
  MAEvent event;

  int events;
  android_poll_source *source;
  while (ALooper_pollAll(waitFlag ? -1 : 0, NULL, 
                         &events, (void **)&source) >= 0) {
    // process this event.
    if (source != NULL) {
      source->process(_app, source);
    }
    
    // check if we are exiting.
    if (_app->destroyRequested != 0) {
      trace("Engine thread destroy requested!");
      //engine_term_display(&engine);
      //return;
    }
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
  int slept = 0;
  while (1) {
    if (runtime->hasEvent()
        || runtime->isBack()
        || runtime->isClosing()) {
      break;
    }
    //thread->Sleep(WAIT_INTERVAL);
    slept += WAIT_INTERVAL;
    if (timeout > 0 && slept > timeout) {
      break;
    }
  }
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


