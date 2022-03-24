// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include <emscripten.h>
#include "include/osd.h"
#include "common/smbas.h"
#include "lib/maapi.h"
#include "ui/utils.h"
#include "ui/audio.h"
#include "ui/theme.h"
#include "platform/emcc/runtime.h"
#include "platform/emcc/main_bas.h"

#define MAIN_BAS "__main_bas__"
#define WAIT_INTERVAL 10

Runtime *runtime;

MAEvent *getMotionEvent(int type, const EmscriptenMouseEvent *event) {
  MAEvent *result = new MAEvent();
  result->type = type;
  result->point.x = event->clientX;
  result->point.y = event->clientY;
  return result;
}

MAEvent *getKeyPressedEvent(int keycode, int nativeKey) {
  MAEvent *result = new MAEvent();
  result->type = EVENT_TYPE_KEY_PRESSED;
  result->key = keycode;
  result->nativeKey = nativeKey;
  return result;
}

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  runtime->handleMouse(eventType, e);
  return 0;
}

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  runtime->handleKeyboard(eventType, e);
  return 0;
}

Runtime::Runtime() :
  System() {
  logEntered();
  runtime = this;

  emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, mouse_callback);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, mouse_callback);
  emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, mouse_callback);
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, key_callback);

  MAExtent screenSize = maGetScrSize();
  _output = new AnsiWidget(EXTENT_X(screenSize), EXTENT_Y(screenSize));
  _output->construct();
  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(11);
  _eventQueue = new Stack<MAEvent *>();
  _state = kActiveState;
  g_themeId = 0;
}

Runtime::~Runtime() {
  logEntered();
  emscripten_html5_remove_all_event_listeners();
  delete _output;
  delete _eventQueue;
  runtime = nullptr;
  _output = nullptr;
  _eventQueue = nullptr;
}

void Runtime::alert(const char *title, const char *message) {
  EM_ASM_({
      window.alert(UTF8ToString($0) + "\n" + UTF8ToString($1));
    }, title, message);
}

int Runtime::ask(const char *title, const char *prompt, bool cancel) {
  int result = 0;
  return result;
}

void Runtime::browseFile(const char *url) {
  EM_ASM_({
      window.open(UTF8ToString($0));
    }, url);
}

char *Runtime::getClipboardText() {
  return nullptr;
}

char *Runtime::loadResource(const char *fileName) {
  logEntered();
  char *buffer = System::loadResource(fileName);
  if (buffer == nullptr && strcmp(fileName, MAIN_BAS) == 0) {
    buffer = (char *)malloc(main_bas_len + 1);
    memcpy(buffer, main_bas, main_bas_len);
    buffer[main_bas_len] = '\0';
  }
  return buffer;
}

void Runtime::handleKeyboard(int eventType, const EmscriptenKeyboardEvent *e) {
  trace("eventType: %d [%s %s %s] [%d %d %d] %s%s%s%s ",
        eventType,
        e->key, e->code, e->charValue,
        e->charCode,      e->keyCode,      e->which,
        e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "");

}

void Runtime::handleMouse(int eventType, const EmscriptenMouseEvent *e) {
  switch (eventType) {
  case EMSCRIPTEN_EVENT_MOUSEDOWN:
    if (e->button == 2) {
      _menuX = e->clientX;
      _menuY = e->clientY;
      showMenu();
    } else {
      pushEvent(getMotionEvent(EVENT_TYPE_POINTER_PRESSED, e));
    }
    break;
  case EMSCRIPTEN_EVENT_MOUSEMOVE:
    pushEvent(getMotionEvent(EVENT_TYPE_POINTER_DRAGGED, e));
    break;
  case EMSCRIPTEN_EVENT_MOUSEUP:
    pushEvent(getMotionEvent(EVENT_TYPE_POINTER_RELEASED, e));
    break;
  }
}

void Runtime::pause(int timeout) {
  if (timeout == -1) {
    if (hasEvent()) {
      MAEvent *event = popEvent();
      handleEvent(*event);
      delete event;
    }
  } else {
    int slept = 0;
    while (1) {
      if (isBreak()) {
        break;
      } else if (hasEvent()) {
        MAEvent *event = popEvent();
        handleEvent(*event);
        delete event;
      }
      emscripten_sleep(WAIT_INTERVAL);
      slept += WAIT_INTERVAL;
      if (timeout > 0 && slept > timeout) {
        break;
      }
    }
  }

}

MAEvent Runtime::processEvents(int waitFlag) {
  switch (waitFlag) {
  case 1:
    // wait for an event
    _output->flush(true);
    while (!hasEvent()) {
      emscripten_sleep(WAIT_INTERVAL);
    }
    break;
  case 2:
    _output->flush(false);
    pause(WAIT_INTERVAL);
    break;
  default:
    emscripten_sleep(WAIT_INTERVAL);
  }

  MAEvent event;
  if (hasEvent()) {
    MAEvent *nextEvent = popEvent();
    handleEvent(*nextEvent);
    event = *nextEvent;
    delete nextEvent;
  } else {
    event.type = 0;
  }
  return event;
}

void Runtime::runShell() {
  logEntered();

  audio_open();
  runMain(MAIN_BAS);
  audio_close();
  _state = kDoneState;
  logLeaving();
}

void Runtime::setClipboardText(const char *text) {
}

void Runtime::showCursor(CursorType cursorType) {
  static CursorType _cursorType;
  if (_cursorType != cursorType) {
    _cursorType = cursorType;
    EM_ASM_({
        document.body.style.cursor = UTF8ToString($0);
      }, cursorType == kIBeam ? "text" : cursorType == kArrow ? "auto" : "pointer");
  }
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

void maPushEvent(MAEvent *maEvent) {
  runtime->pushEvent(maEvent);
}

void maWait(int timeout) {
  runtime->pause(timeout);
}

//
// System platform methods
//
void System::editSource(strlib::String loadPath, bool restoreOnExit) {
  // empty
}

bool System::getPen3() {
  return false;
}

void System::completeKeyword(int index) {
  // empty
}

//
// sbasic implementation
//
int osd_devinit(void) {
  runtime->setRunning(true);
  return 1;
}

int osd_devrestore(void) {
  runtime->setRunning(false);
  return 0;
}
