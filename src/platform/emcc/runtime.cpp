// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include <emscripten.h>
#include <emscripten/key_codes.h>
#include "include/osd.h"
#include "common/inet.h"
#include "common/device.h"
#include "lib/maapi.h"
#include "ui/audio.h"
#include "ui/theme.h"
#include "ui/utils.h"
#include "platform/emcc/runtime.h"
#include "platform/emcc/main_bas.h"
#include "platform/emcc/keymap.h"

#define FONT_SIZE 14
#define MAIN_BAS "__main_bas__"
#define WAIT_INTERVAL 10
#define EVENT_TYPE_RESTART 101
#define EVENT_TYPE_SHOW_MENU 102
#define EVENT_TYPE_RESIZE 103
#define EVENT_TYPE_BACK 104
#define EVENT_TYPE_PGUP 105
#define EVENT_TYPE_PGDN 106
#define EVENT_TYPE_UP 107
#define EVENT_TYPE_DN 108

Runtime *runtime;
String clipboard;

int getFontSize() {
  return EM_ASM_INT({
      const parameters = new URLSearchParams(window.location.search);
      const result = parameters.get('fontSize') || $0;
      console.log(result);
      return result;
    }, FONT_SIZE);
}

MAEvent *getMotionEvent(int type, const EmscriptenMouseEvent *event) {
  MAEvent *result = new MAEvent();
  result->type = type;
  if (event) {
    result->point.x = event->clientX;
    result->point.y = event->clientY;
  }
  return result;
}

MAEvent *getKeyPressedEvent(int keycode, int nativeKey = 0) {
  MAEvent *result = new MAEvent();
  result->type = EVENT_TYPE_KEY_PRESSED;
  result->key = keycode;
  result->nativeKey = nativeKey;
  return result;
}

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
  return runtime->handleMouse(eventType, e);
}

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  return runtime->handleKeyboard(eventType, e);
}

EM_BOOL resize_callback(int eventType, const EmscriptenUiEvent *e, void *userData) {
  MAEvent *event = new MAEvent();
  event->type = EVENT_TYPE_RESIZE;
  event->point.x = e->documentBodyClientWidth;
  event->point.y = e->documentBodyClientHeight;
  runtime->pushEvent(event);
  return 0;
}

Runtime::Runtime() :
  System() {
  logEntered();
  runtime = this;

  // note: cannot call emscripten_sleep from inside the callbacks
  emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, mouse_callback);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, mouse_callback);
  emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, mouse_callback);
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, key_callback);
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, resize_callback);

  MAExtent screenSize = maGetScrSize();
  _output = new AnsiWidget(EXTENT_X(screenSize), EXTENT_Y(screenSize));
  _output->construct();
  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(::getFontSize());
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
  return EM_ASM_INT({
      return !window.confirm(UTF8ToString($0) + "\n" + UTF8ToString($1));
    }, title, prompt);
}

void Runtime::browseFile(const char *url) {
  EM_ASM_({
      window.open(UTF8ToString($0));
    }, url);
}

char *Runtime::getClipboardText() {
  return strdup(clipboard.c_str());
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

bool Runtime::handleKeyboard(int eventType, const EmscriptenKeyboardEvent *e) {
  int keyCode = e->keyCode;
  bool result = true;
  static bool capsLock = false;
  switch (e->keyCode) {
  case DOM_VK_SHIFT:
  case DOM_VK_CONTROL:
  case DOM_VK_ALT:
    // ignore press without modifier key
    result = false;
    break;

  case DOM_VK_F5:
  case DOM_VK_F11:
  case DOM_VK_F12:
    // ignore standard browser keystrokes
    result = false;
    break;

  case DOM_VK_CAPS_LOCK:
    capsLock = !capsLock;
    break;

  default:
    for (int i = 0; i < KEYMAP_LEN; i++) {
      if (keyCode == KEYMAP[i][0]) {
        keyCode = KEYMAP[i][1];
        break;
      }
    }
    if (e->ctrlKey && e->altKey) {
      pushEvent(getKeyPressedEvent(SB_KEY_CTRL_ALT(keyCode)));
    } else if (e->ctrlKey && e->shiftKey) {
      pushEvent(getKeyPressedEvent(SB_KEY_SHIFT_CTRL(keyCode)));
    } else if (e->altKey && e->shiftKey) {
      pushEvent(getKeyPressedEvent(SB_KEY_ALT_SHIFT(keyCode)));
    } else if (e->ctrlKey) {
      keyCode = to_lower(keyCode);
      switch (keyCode) {
      case 'b':
        pushEvent(getMotionEvent(EVENT_TYPE_BACK, nullptr));
        break;
      case 'm':
        pushEvent(getMotionEvent(EVENT_TYPE_SHOW_MENU, nullptr));
        break;
      case SB_KEY_PGUP:
        pushEvent(getMotionEvent(EVENT_TYPE_PGUP, nullptr));
        break;
      case SB_KEY_PGDN:
        pushEvent(getMotionEvent(EVENT_TYPE_PGDN, nullptr));
        break;
      case SB_KEY_UP:
        pushEvent(getMotionEvent(EVENT_TYPE_UP, nullptr));
        break;
      case SB_KEY_DN:
        pushEvent(getMotionEvent(EVENT_TYPE_PGDN, nullptr));
        break;
      default:
        pushEvent(getKeyPressedEvent(SB_KEY_CTRL(keyCode)));
      }
    } else if (e->altKey) {
      switch (keyCode) {
      case 'D':
        // browser keystroke
        result = false;
        break;
      default:
        pushEvent(getKeyPressedEvent(SB_KEY_ALT(keyCode)));
        break;
      }
    } else if (e->shiftKey) {
      bool shifted = false;
      for (int i = 0; i < SHIFT_KEYMAP_LEN; i++) {
        if (keyCode == SHIFT_KEYMAP[i][0]) {
          keyCode = SHIFT_KEYMAP[i][1];
          shifted = true;
          break;
        }
      }
      if (shifted) {
        pushEvent(getKeyPressedEvent(keyCode));
      } else {
        pushEvent(getKeyPressedEvent(SB_KEY_SHIFT(keyCode)));
      }
    } else {
      pushEvent(getKeyPressedEvent(capsLock ? keyCode : to_lower(keyCode)));
    }
    break;
  }
  return result;
}

bool Runtime::handleMouse(int eventType, const EmscriptenMouseEvent *e) {
  bool result = true;
  switch (eventType) {
  case EMSCRIPTEN_EVENT_MOUSEDOWN:
    if (e->button == 2) {
      pushEvent(getMotionEvent(EVENT_TYPE_SHOW_MENU, e));
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
  default:
    result = false;
    break;
  }
  return result;
}

void Runtime::pause(int timeout) {
  if (timeout == -1) {
    if (hasEvent()) {
      MAEvent *event = popEvent();
      processEvent(*event);
      delete event;
    }
  } else {
    int slept = 0;
    while (1) {
      if (isBreak()) {
        break;
      } else if (hasEvent()) {
        MAEvent *event = popEvent();
        processEvent(*event);
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
    processEvent(*nextEvent);
    event = *nextEvent;
    delete nextEvent;
  } else {
    event.type = 0;
  }
  return event;
}

void Runtime::processEvent(MAEvent &event) {
  switch (event.type) {
  case EVENT_TYPE_RESTART:
    setRestart();
    break;
  case EVENT_TYPE_SHOW_MENU:
    _menuX = event.point.x;
    _menuY = event.point.y;
    showMenu();
    break;
  case EVENT_TYPE_RESIZE:
    resize();
    break;
  case EVENT_TYPE_BACK:
    setBack();
    break;
  case EVENT_TYPE_PGUP:
    _output->scroll(true, true);
    break;
  case EVENT_TYPE_PGDN:
    _output->scroll(false, true);
    break;
  case EVENT_TYPE_UP:
    _output->scroll(true, false);
    break;
  case EVENT_TYPE_DN:
    _output->scroll(false, false);
    break;
  case EVENT_TYPE_KEY_PRESSED:
    handleEvent(event);
    if (event.key != -1 && isRunning()) {
      dev_pushkey(event.key);
    }
    break;

  default:
    handleEvent(event);
    break;
  }
}

void Runtime::runShell() {
  logEntered();
  audio_open();
  net_init();
  chdir("/basic");
  while (1) {
    runMain(MAIN_BAS);
  }
}

void Runtime::setClipboardText(const char *text) {
  EM_ASM_({
      navigator.clipboard.writeText(UTF8ToString($0));
    }, text);
  clipboard = text;
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

void System::editSource(strlib::String loadPath, bool restoreOnExit) {
  logEntered();

  strlib::String fileName;
  int i = loadPath.lastIndexOf('/', 0);
  if (i != -1) {
    fileName = loadPath.substring(i + 1);
  } else {
    fileName = loadPath;
  }

  strlib::String dirtyFile;
  dirtyFile.append(" * ");
  dirtyFile.append(fileName);
  strlib::String cleanFile;
  cleanFile.append(" - ");
  cleanFile.append(fileName);

  int w = _output->getWidth();
  int h = _output->getHeight();
  int charWidth = _output->getCharWidth();
  int charHeight = _output->getCharHeight();
  int prevScreenId = _output->selectScreen(SOURCE_SCREEN);
  TextEditInput *editWidget;
  if (_editor != nullptr) {
    editWidget = _editor;
    editWidget->_width = w;
    editWidget->_height = h;
  } else {
    editWidget = new TextEditInput(_programSrc, charWidth, charHeight, 0, 0, w, h);
  }
  auto *helpWidget = new TextEditHelpWidget(editWidget, charWidth, charHeight, false);
  auto *widget = editWidget;
  _modifiedTime = getModifiedTime();
  editWidget->updateUI(nullptr, nullptr);
  editWidget->setLineNumbers();
  editWidget->setFocus(true);

  _output->clearScreen();
  _output->addInput(editWidget);
  _output->addInput(helpWidget);

  if (gsb_last_line && isBreak()) {
    String msg = "Break at line: ";
    msg.append(gsb_last_line);
    alert("Error", msg);
  } else if (gsb_last_error && !isBack()) {
    // program stopped with an error
    editWidget->setCursorRow(gsb_last_line + editWidget->getSelectionRow() - 1);
    alert("Error", gsb_last_errmsg);
  }

  bool showStatus = !editWidget->getScroll();
  _srcRendered = false;
  _output->setStatus(showStatus ? cleanFile : "");
  _output->redraw();
  _state = kEditState;

  while (_state == kEditState) {
    MAEvent event = getNextEvent();
    switch (event.type) {
    case EVENT_TYPE_POINTER_PRESSED:
      if (!showStatus && widget == editWidget && event.point.x < editWidget->getMarginWidth()) {
        _output->setStatus(editWidget->isDirty() ? dirtyFile : cleanFile);
        _output->redraw();
        showStatus = true;
      }
      break;
    case EVENT_TYPE_POINTER_RELEASED:
      if (showStatus && event.point.x < editWidget->getMarginWidth() && editWidget->getScroll()) {
        _output->setStatus("");
        _output->redraw();
        showStatus = false;
      }
      break;
    case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
      if (editWidget->isDirty() && !editWidget->getScroll()) {
        _output->setStatus(dirtyFile);
        _output->redraw();
      }
      break;
    case EVENT_TYPE_KEY_PRESSED:
      if (_userScreenId == -1) {
        int sw = _output->getScreenWidth();
        bool redraw = true;
        bool dirty = editWidget->isDirty();
        char *text;

        switch (event.key) {
        case SB_KEY_F(2):
        case SB_KEY_F(3):
        case SB_KEY_F(4):
        case SB_KEY_F(5):
        case SB_KEY_F(6):
        case SB_KEY_F(7):
        case SB_KEY_F(8):
        case SB_KEY_F(10):
        case SB_KEY_F(11):
        case SB_KEY_F(12):
        case SB_KEY_MENU:
        case SB_KEY_BREAK:
          // unhandled keys
          redraw = false;
        break;
        case SB_KEY_ESCAPE:
          widget = editWidget;
          helpWidget->hide();
          helpWidget->cancelMode();
          editWidget->setFocus(true);
          break;
        case SB_KEY_F(1):
          widget = helpWidget;
          helpWidget->createKeywordIndex();
          helpWidget->showPopup(-4, -2);
          helpWidget->setFocus(true);
          showStatus = false;
          break;
        case SB_KEY_F(9):
        case SB_KEY_CTRL('r'):
          _state = kRunState;
          break;
        case SB_KEY_CTRL('s'):
          saveFile(editWidget, loadPath);
          break;
        case SB_KEY_CTRL('c'):
        case SB_KEY_CTRL('x'):
          text = widget->copy(event.key == (int)SB_KEY_CTRL('x'));
          if (text) {
            setClipboardText(text);
            free(text);
          }
          break;
        case SB_KEY_CTRL('v'):
          text = getClipboardText();
          widget->paste(text);
          free(text);
          break;
        case SB_KEY_CTRL('o'):
          _output->selectScreen(USER_SCREEN1);
          showCompletion(true);
          _output->redraw();
          _state = kActiveState;
          waitForBack();
          _output->selectScreen(SOURCE_SCREEN);
          _state = kEditState;
          break;
        default:
          redraw = widget->edit(event.key, sw, charWidth);
          break;
        }
        if (editWidget->isDirty() != dirty && !editWidget->getScroll()) {
          _output->setStatus(editWidget->isDirty() ? dirtyFile : cleanFile);
        }
        if (redraw) {
          _output->redraw();
        }
      }
    }

    if (isBack() && widget == helpWidget) {
      widget = editWidget;
      helpWidget->hide();
      editWidget->setFocus(true);
      _state = kEditState;
      _output->redraw();
    }

    if (widget->isDirty()) {
      int choice = -1;
      if (isClosing()) {
        choice = 0;
      } else if (isBack()) {
        const char *message = "The current file has not been saved.\n"
                              "Would you like to save it now?";
        choice = ask("Save changes?", message, isBack());
      }
      if (choice == 0) {
        widget->save(loadPath);
      } else if (choice == 2) {
        // cancel
        _state = kEditState;
      }
    }
  }

  if (_state == kRunState) {
    // allow the editor to be restored on return
    if (!_output->removeInput(editWidget)) {
      trace("Failed to remove editor input");
    }
    _editor = editWidget;
    _editor->setFocus(false);
  } else {
    _editor = nullptr;
  }

  // deletes editWidget unless it has been removed
  _output->removeInputs();
  if (!isClosing() && restoreOnExit) {
    _output->selectScreen(prevScreenId);
  }
  logLeaving();
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
bool System::getPen3() {
  bool result = false;
  if (_touchX != -1 && _touchY != -1) {
    result = true;
  } else {
    // get mouse
    processEvents(0);
    if (_touchX != -1 && _touchY != -1) {
      result = true;
    }
  }
  return result;
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

//
// not implemented
//
void System::completeKeyword(int index) {}
void maHideVirtualKeyboard(void) {}
void maShowVirtualKeyboard(void) {}
void maUpdateScreen(void) {}
