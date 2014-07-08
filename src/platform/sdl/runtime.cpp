// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"
#include "common/fs_socket_client.h"
#include "platform/common/maapi.h"
#include "platform/common/utils.h"
#include "platform/common/form_ui.h"
#include "platform/sdl/runtime.h"
#include "platform/sdl/keymap.h"
#include "platform/sdl/main_bas.h"

#define WAIT_INTERVAL 10
#define MAIN_BAS "__main_bas__"

Runtime *runtime;

Runtime::Runtime(SDL_Window *window) :
  System(),
  _window(window) {
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

void Runtime::construct(const char *font, const char *boldFont) {
  logEntered();
  _state = kClosingState;
  _graphics = new Graphics(_window);
  if (_graphics && _graphics->construct(font, boldFont)) {
    int w, h;
    SDL_GetWindowSize(_window, &w, &h);
    _output = new AnsiWidget(this, w, h);
    if (_output && _output->construct()) {
      _eventQueue = new Stack<MAEvent *>();
      if (_eventQueue) {
        _state = kActiveState;
      }
    }
  }
}

void Runtime::pushEvent(MAEvent *event) {
  _eventQueue->push(event);
}

MAEvent *Runtime::popEvent() {
  return _eventQueue->pop();
}

void Runtime::runShell(const char *startupBas) {
  logEntered();

  os_graphics = 1;
  opt_interactive = true;
  opt_usevmt = 0;
  opt_file_permitted = 1;
  opt_ide = IDE_NONE;
  opt_graphics = true;
  opt_pref_bpp = 0;
  opt_nosave = true;

  if (startupBas != NULL) {
    runOnce(startupBas);
  } else {
    runMain(MAIN_BAS);
  }

  _state = kDoneState;
  logLeaving();
}

char *Runtime::loadResource(const char *fileName) {
  logEntered();
  char *buffer = System::loadResource(fileName);
  if (buffer == NULL && strcmp(fileName, MAIN_BAS) == 0) {
    buffer = (char *)tmp_alloc(main_bas_len + 1);
    memcpy(buffer, main_bas, main_bas_len);
    buffer[main_bas_len] = '\0';
  }
  return buffer;
}

void Runtime::showAlert(const char *title, const char *message) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, _window);
}

void Runtime::runPath(const char *path) {
  buttonClicked(path);
  setExit(false);
}

void Runtime::handleKeyEvent(MAEvent &event) {
  if (isRunning()) {
    int key = event.key;
    for (int i = 0; keymap[i] != 0; i += 2) {
      if (keymap[i] == key) {
        if (keymap[i + 1] != -1) {
          dev_pushkey(keymap[i + 1]);
        }
        key = -1;
        break;
      }
    }
    // not found
    if (key > 0 && event.nativeKey) {
      dev_pushkey(event.nativeKey);
    }
  }
}

MAEvent *getMotionEvent(int type, SDL_Event *event) {
  MAEvent *result = new MAEvent();
  result->type = type;
  result->point.x = event->motion.x;
  result->point.y = event->motion.y;
  return result;
}

void Runtime::pollEvents(bool blocking) {
  if (runtime && runtime->isActive()) {
    if (blocking) {
      SDL_WaitEvent(NULL);
    }
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      MAEvent *maEvent = NULL;
      switch (ev.type) {
      case SDL_QUIT:
        runtime->setExit(true);
        break;
      case SDL_KEYDOWN:
        if ((ev.key.keysym.sym == SDLK_c && (ev.key.keysym.mod & KMOD_CTRL))) {
          runtime->setExit(true);
        } else {
          maEvent = new MAEvent();
          maEvent->type = EVENT_TYPE_KEY_PRESSED;
          maEvent->nativeKey = (ev.key.keysym.scancode & 0xFF);
          maEvent->key = ev.key.keysym.sym;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_PRESSED, &ev);
        break;
      case SDL_MOUSEMOTION:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_DRAGGED, &ev);
        break;
      case SDL_MOUSEBUTTONUP:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_RELEASED, &ev);
        break;
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        maEvent = new MAEvent();
        maEvent->point.x = ev.window.data1;
        maEvent->point.y = ev.window.data2;
        maEvent->type = EVENT_TYPE_SCREEN_CHANGED;
        break;
      }
      if (maEvent != NULL) {
        runtime->pushEvent(maEvent);
      }
    }
  }
}

MAEvent Runtime::processEvents(int waitFlag) {
  switch (waitFlag) {
  case 1:
    // wait for an event
    _output->flush(true);
    pollEvents(true);
    break;
  case 2:
    // pause
    maWait(WAIT_INTERVAL);
    break;
  default:
    pollEvents(false);
    checkLoadError();
  }

  MAEvent event;
  if (hasEvent()) {
    MAEvent *nextEvent = popEvent();
    event = *nextEvent;
    delete nextEvent;
  } else {
    event.type = 0;
  }

  switch (event.type) {
  case EVENT_TYPE_SCREEN_CHANGED:
    _graphics->resize();
    resize();
    break;
  case EVENT_TYPE_KEY_PRESSED:
    handleKeyEvent(event);
    break;
  default:
    handleEvent(event);
    break;
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

void Runtime::onResize(int width, int height) {
  logEntered();
  if (_graphics != NULL) {
    int w = _graphics->getWidth();
    int h = _graphics->getHeight();
    if (w != width || h != height) {
      trace("Resized from %d %d to %d %d", w, h, width, height);
      _graphics->setSize(width, height);
      MAEvent *maEvent = new MAEvent();
      maEvent->type = EVENT_TYPE_SCREEN_CHANGED;
      runtime->pushEvent(maEvent);
    }
  }
}

//
// form_ui implementation
//
AnsiWidget *form_ui::getOutput() {
  return runtime->_output;
}

void form_ui::optionsBox(StringList *items) {
  // TODO: see fltk
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
      usleep(WAIT_INTERVAL * 1000);
      slept += WAIT_INTERVAL;
      if (timeout > 0 && slept > timeout) {
        break;
      }
    }
  }
}

int maGetMilliSecondCount(void) {
  timespec t;
  t.tv_sec = t.tv_nsec = 0;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (int) (1000L * t.tv_sec + ((double) t.tv_nsec / 1e6));
}

int maShowVirtualKeyboard(void) {
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
  runtime->showAlert(title, message);
}

//
// sbasic implementation
//
int osd_devinit(void) {
  setsysvar_str(SYSVAR_OSNAME, "SDL");
  osd_clear_sound_queue();
  runtime->setRunning(true);
  return 1;
}

void osd_sound(int frq, int dur, int vol, int bgplay) {
}

void osd_clear_sound_queue() {
}

void osd_beep(void) {
  osd_sound(1000, 30, 100, 0);
  osd_sound(500, 30, 100, 0);
}

