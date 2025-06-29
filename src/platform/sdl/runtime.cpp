// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "include/osd.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/device.h"
#include "common/keymap.h"
#include "common/inet.h"
#include "common/pproc.h"
#include "lib/maapi.h"
#include "ui/utils.h"
#include "ui/audio.h"
#include "platform/sdl/runtime.h"
#include "platform/sdl/syswm.h"
#include "platform/sdl/keymap.h"
#include "platform/sdl/main_bas.h"

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_messagebox.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>
#include <math.h>
#include <wchar.h>

#define WAIT_INTERVAL 5
#define COND_WAIT_TIME 250
#define PAUSE_DEBUG_LAUNCH 750
#define PAUSE_DEBUG_STEP 50
#define MAIN_BAS "__main_bas__"
#define EVENT_TYPE_RESTART 101

Runtime *runtime;
SDL_Mutex *g_lock = nullptr;
SDL_Condition *g_cond = nullptr;
bool g_debugPause = false;
bool g_debugBreak = false;
bool g_debugTrace = true;
int g_debugLine = 0;
strlib::List<int*> g_breakPoints;
socket_t g_debugee = -1;
extern int g_debugPort;

int debugThread(void *data);

MAEvent *getMotionEvent(int type, SDL_Event *event) {
  MAEvent *result = new MAEvent();
  result->type = type;
  result->point.x = event->motion.x;
  result->point.y = event->motion.y;
  return result;
}

MAEvent *getKeyPressedEvent(int keycode, int nativeKey) {
  MAEvent *result = new MAEvent();
  result->type = EVENT_TYPE_KEY_PRESSED;
  result->key = keycode;
  result->nativeKey = nativeKey;
  return result;
}

Runtime::Runtime(SDL_Window *window) :
  System(),
  _fullscreen(false),
  _graphics(nullptr),
  _eventQueue(nullptr),
  _window(window),
  _cursorHand(nullptr),
  _cursorArrow(nullptr),
  _cursorIBeam(nullptr) {
  runtime = this;
}

Runtime::~Runtime() {
  logEntered();
  delete _output;
  delete _eventQueue;
  delete _graphics;
  runtime = nullptr;
  _output = nullptr;
  _eventQueue = nullptr;
  _graphics = nullptr;

  SDL_DestroyCursor(_cursorHand);
  SDL_DestroyCursor(_cursorArrow);
  SDL_DestroyCursor(_cursorIBeam);
  _cursorHand = nullptr;
  _cursorArrow = nullptr;
  _cursorIBeam = nullptr;
}

void Runtime::alert(const char *title, const char *message) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, _window);
}

int Runtime::ask(const char *title, const char *prompt, bool cancel) {
  SDL_MessageBoxButtonData buttons[cancel ? 3: 2];
  memset(&buttons[0], 0, sizeof(SDL_MessageBoxButtonData));
  memset(&buttons[1], 0, sizeof(SDL_MessageBoxButtonData));
  buttons[0].text = "Yes";
  buttons[0].buttonID = 0;
  buttons[1].text = "No";
  buttons[1].buttonID = 1;
  if (cancel) {
    memset(&buttons[2], 0, sizeof(SDL_MessageBoxButtonData));
    buttons[2].text = "Cancel";
    buttons[2].buttonID = 2;
  }

  SDL_MessageBoxData data;
  memset(&data, 0, sizeof(SDL_MessageBoxData));
  data.window = _window;
  data.title = title;
  data.message = prompt;
  data.flags = SDL_MESSAGEBOX_INFORMATION;
  data.numbuttons = cancel ? 3 : 2;
  data.buttons = buttons;

  int buttonID;
  SDL_ShowMessageBox(&data, &buttonID);
  return buttonID;
}

void Runtime::browseFile(const char *url) {
  ::browseFile(_window, url);
}

void Runtime::construct(const char *font, const char *boldFont) {
  logEntered();
  _state = kClosingState;
  _graphics = new Graphics(_window);

  _cursorHand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
  _cursorArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
  _cursorIBeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);

  if (_graphics && _graphics->construct(font, boldFont)) {
    int w, h;
    SDL_GetWindowSize(_window, &w, &h);
    _output = new AnsiWidget(w, h);
    if (_output && _output->construct()) {
      _eventQueue = new Stack<MAEvent *>();
      if (_eventQueue) {
        _state = kActiveState;
      }
    }
  } else {
    alert("Unable to start", "Font resource not loaded");
    fprintf(stderr, "failed to load: [%s] [%s]\n", font, boldFont);
    exit(1);
  }
}

bool Runtime::debugActive() {
  bool open;
  if (g_debugee != -1) {
    // potentially open
    char buf[OS_PATHNAME_SIZE + 1];
    net_print(g_debugee, "l\n");
    open = net_input(g_debugee, buf, sizeof(buf), "\n") > 0;
  } else {
    // not open
    open = false;
  }
  return open;
}

bool Runtime::debugOpen(const char *file) {
  bool open = debugActive();
  if (!open) {
    launchDebug(file);
    pause(PAUSE_DEBUG_LAUNCH);
    SDL_RaiseWindow(_window);
  }
  return open;
}

void Runtime::debugStart(TextEditInput *editWidget, const char *file) {
  bool open = debugOpen(file);
  if (!open) {
    g_debugee = net_connect("localhost", g_debugPort);
    if (g_debugee != -1) {
      net_print(g_debugee, "l\n");
      char buf[OS_PATHNAME_SIZE + 1];
      int size = net_input(g_debugee, buf, sizeof(buf), "\n");
      if (size > 0) {
        int *marker = editWidget->getMarkers();
        for (int i = 0; i < MAX_MARKERS; i++) {
          if (marker[i] != -1) {
            net_printf(g_debugee, "b %d\n", marker[i]);
          }
        }
        editWidget->gotoLine(buf);
        appLog("Debug session ready");
      }
    } else {
      appLog("Failed to attach to debug window");
    }
  } else {
    debugStop();
  }
}

void Runtime::debugStep(TextEditInput *edit, TextEditHelpWidget *help, bool cont) {
  if (g_debugee != -1) {
    char buf[OS_PATHNAME_SIZE + 1];
    net_print(g_debugee, cont ? "c\n" : "n\n");
    pause(PAUSE_DEBUG_STEP);
    net_print(g_debugee, "l\n");
    int size = net_input(g_debugee, buf, sizeof(buf), "\n");
    if (size > 0) {
      edit->gotoLine(buf);
      net_print(g_debugee, "v\n");
      help->reload(nullptr);
      bool endChar = false;
      do {
        size = net_read(g_debugee, buf, sizeof(buf));
        if (!size) {
          break;
        }
        if (buf[size - 1] == '\1') {
          endChar = true;
          size--;
        }
        help->append(buf, size);
        help->append("\n", 1);
      } while (!endChar);
    }
  }
}

void Runtime::debugStop() {
  if (g_debugee != -1) {
    net_print(g_debugee, "q\n");
    net_disconnect(g_debugee);
    g_debugee = -1;
    appLog("Closed debug session");
  }
}

void Runtime::enableCursor(bool enabled) {
  if (enabled) {
    SDL_ShowCursor();
  } else {
    SDL_HideCursor();
  }
}

void Runtime::exportRun(const char *file) {
  launchExec(file);
  SDL_RaiseWindow(_window);
}

bool Runtime::toggleFullscreen() {
  if (!_fullscreen) {
    setWindowRect(_windowRect);
  }
  _fullscreen = !_fullscreen;
  SDL_SetWindowFullscreen(_window, _fullscreen);
  return _fullscreen;
}

int Runtime::runShell(const char *startupBas, bool runWait, int fontScale, int debugPort) {
  logEntered();

  os_graphics = 1;
  opt_file_permitted = 1;
  opt_graphics = true;
  opt_nosave = true;

  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(getStartupFontSize(_window));
  _initialFontSize = _output->getFontSize();
  if (fontScale != 100) {
    _fontScale = fontScale;
    int fontSize = (_initialFontSize * _fontScale / 100);
    _output->setFontSize(fontSize);
  }

  audio_open();
  net_init();

  if (debugPort > 0) {
    appLog("Debug active on port %d\n", debugPort);
    g_lock = SDL_CreateMutex();
    g_cond = SDL_CreateCondition();
    opt_trace_on = 1;
    g_debugBreak = true;
    SDL_Thread *thread =
      SDL_CreateThread(debugThread, "DBg", (void *)(intptr_t)debugPort);
    SDL_DetachThread(thread);
  }

  if (startupBas != nullptr) {
    String bas = startupBas;
    if (opt_ide == IDE_INTERNAL) {
      runEdit(bas.c_str());
    } else if (opt_ide == IDE_EXTERNAL) {
      runLive(bas.c_str());
    } else {
      runOnce(bas.c_str(), runWait);
    }
    while (_state == kRestartState) {
      _state = kActiveState;
      if (!_loadPath.empty()) {
        bas = _loadPath;
      }
      runOnce(bas.c_str(), runWait);
    }
  } else {
    runMain(MAIN_BAS);
  }

  if (debugPort > 0) {
    SDL_DestroyCondition(g_cond);
    SDL_DestroyMutex(g_lock);
  }

  debugStop();
  net_close();
  audio_close();
  _state = kDoneState;
  logLeaving();
  return _fontScale;
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

void Runtime::handleKeyEvent(MAEvent &event) {
  // handle keypad keys
  if (event.key != -1) {
    if (event.key == SDLK_NUMLOCKCLEAR) {
      event.key = -1;
    } else if (event.key == SDLK_KP_1) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '1' : SB_KEY_END;
    } else if (event.key == SDLK_KP_2) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '2' : SB_KEY_DN;
    } else if (event.key == SDLK_KP_3) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '3' : SB_KEY_PGDN;
    } else if (event.key == SDLK_KP_4) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '4' : SB_KEY_LEFT;
    } else if (event.key == SDLK_KP_5) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '5' : SB_KEY_CTRL(SB_KEY_HOME);
    } else if (event.key == SDLK_KP_6) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '6' : SB_KEY_RIGHT;
    } else if (event.key == SDLK_KP_7) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '7' : SB_KEY_HOME;
    } else if (event.key == SDLK_KP_8) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '8' : SB_KEY_UP;
    } else if (event.key == SDLK_KP_9) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '9' : SB_KEY_PGUP;
    } else if (event.key == SDLK_KP_0) {
      event.key = event.nativeKey == SDL_KMOD_NUM ? '0' : SB_KEY_INSERT;
    }
  }

  // handle ALT/SHIFT/CTRL states
  if (event.key != -1) {
    if ((event.nativeKey & SDL_KMOD_ALT) &&
        (event.key == SDLK_LALT || event.key == SDLK_RALT)) {
      // ignore ALT press without modifier key
      event.key = -1;
    } else if ((event.nativeKey & SDL_KMOD_CTRL) &&
        (event.nativeKey & SDL_KMOD_ALT)) {
      event.key = SB_KEY_CTRL_ALT(event.key);
    } else if ((event.nativeKey & SDL_KMOD_CTRL) &&
               (event.nativeKey & SDL_KMOD_SHIFT)) {
      event.key = SB_KEY_SHIFT_CTRL(event.key);
    } else if ((event.nativeKey & SDL_KMOD_ALT) &&
               (event.nativeKey & SDL_KMOD_SHIFT)) {
      event.key = SB_KEY_ALT_SHIFT(event.key);
    } else if (event.nativeKey & SDL_KMOD_CTRL) {
      event.key = SB_KEY_CTRL(event.key);
    } else if (event.nativeKey & SDL_KMOD_ALT) {
      event.key = SB_KEY_ALT(event.key);
    } else if (event.nativeKey & SDL_KMOD_SHIFT) {
      event.key = SB_KEY_SHIFT(event.key);
    }
  }

  // push to runtime queue
  if (event.key != -1 && isRunning()) {
    dev_pushkey(event.key);
  }
}

void Runtime::pause(int timeout) {
  if (timeout == -1) {
    pollEvents(true);
    if (hasEvent()) {
      MAEvent *event = popEvent();
      processEvent(*event);
      delete event;
    }
  } else {
    int slept = 0;
    while (1) {
      pollEvents(false);
      if (isBreak()) {
        break;
      } else if (hasEvent()) {
        MAEvent *event = popEvent();
        processEvent(*event);
        delete event;
      }
      usleep(WAIT_INTERVAL * 1000);
      slept += WAIT_INTERVAL;
      if (timeout > 0 && slept > timeout) {
        break;
      }
    }
  }
}

void Runtime::pollEvents(bool blocking) {
  if (isActive() && !isRestart()) {
    SDL_Event ev;
    if (blocking ? SDL_WaitEvent(&ev) : SDL_PollEvent(&ev)) {
      MAEvent *maEvent = nullptr;
      SDL_Keymod mod;
      switch (ev.type) {
      case SDL_EVENT_TEXT_INPUT:
        // pre-transformed/composted text
        mod = SDL_GetModState();
        if (!mod || (mod & (SDL_KMOD_SHIFT | SDL_KMOD_CAPS | SDL_KMOD_NUM))) {
          // ALT + CTRL keys handled in SDL_EVENT_KEY_DOWN
          if (ev.text.text[0] < 0) {
            wchar_t keycode;
            mbstowcs(&keycode, ev.text.text, 1);
            pushEvent(getKeyPressedEvent((int)keycode, 0));
          } else {
            for (int i = 0; ev.text.text[i] != 0; i++) {
              pushEvent(getKeyPressedEvent(ev.text.text[i], 0));
            }
          }
        }
        break;
      case SDL_EVENT_QUIT:
        setExit(true);
        break;
      case SDL_EVENT_KEY_DOWN:
        if (!isEditing() && ev.key.key == SDLK_C
            && (ev.key.mod & SDL_KMOD_CTRL)) {
          setExit(true);
        } else if (ev.key.key == SDLK_M && (ev.key.mod & SDL_KMOD_CTRL)) {
          showMenu();
        } else if (ev.key.key == SDLK_B && (ev.key.mod & SDL_KMOD_CTRL)) {
          setBack();
        } else if (ev.key.key == SDLK_BACKSPACE &&
                   get_focus_edit() == nullptr &&
                   ((ev.key.mod & SDL_KMOD_CTRL) || !isRunning())) {
          setBack();
        } else if (!isEditing() && ev.key.key == SDLK_PAGEUP &&
                   (ev.key.mod & SDL_KMOD_CTRL)) {
          _output->scroll(true, true);
        } else if (!isEditing() && ev.key.key == SDLK_PAGEDOWN &&
                   (ev.key.mod & SDL_KMOD_CTRL)) {
          _output->scroll(false, true);
        } else if (!isEditing() && ev.key.key == SDLK_UP &&
                   (ev.key.mod & SDL_KMOD_CTRL)) {
          _output->scroll(true, false);
        } else if (!isEditing() && ev.key.key == SDLK_DOWN &&
                   (ev.key.mod & SDL_KMOD_CTRL)) {
          _output->scroll(false, false);
        } else if (ev.key.key == SDLK_P && (ev.key.mod & SDL_KMOD_CTRL)) {
          ::screen_dump();
        } else if (ev.key.key == SDLK_F11) {
          if (toggleFullscreen()) {
            _output->setStatus("Press F11 to exit full screen.");
          }
        } else {
          int lenMap = sizeof(keymap) / sizeof(keymap[0]);
          if (ev.key.key == SDLK_KP_PERIOD) {
            if (ev.key.mod == SDL_KMOD_NUM) {
              // '.' character sent as SDL_EVENT_TEXT_INPUT
            } else {
              pushEvent(getKeyPressedEvent(SB_KEY_DELETE, 0));
            }
            lenMap = 0;
          }
          for (int i = 0; i < lenMap; i++) {
            if (ev.key.key == keymap[i][0]) {
              pushEvent(getKeyPressedEvent(keymap[i][1], ev.key.mod));
              break;
            }
          }
          if (maEvent == nullptr &&
              // Non-numeric-keypad, Control and Alt keys
              (((ev.key.key >= SDLK_KP_1 && ev.key.key <= SDLK_KP_0) &&
                ev.key.mod != SDL_KMOD_NUM) ||
               ((ev.key.mod & SDL_KMOD_CTRL) &&
                ev.key.key != SDLK_LSHIFT &&
                ev.key.key != SDLK_RSHIFT &&
                ev.key.key != SDLK_LCTRL &&
                ev.key.key != SDLK_RCTRL) ||
               (ev.key.mod & SDL_KMOD_ALT))) {
            maEvent = getKeyPressedEvent(ev.key.key, ev.key.mod);
          }
        }
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (ev.button.button == SDL_BUTTON_RIGHT) {
          _menuX = ev.motion.x;
          _menuY = ev.motion.y;
          showMenu();
        } else if (ev.motion.x != 0 && ev.motion.y != 0) {
          // avoid phantom down message when launching in windows
          maEvent = getMotionEvent(EVENT_TYPE_POINTER_PRESSED, &ev);
        }
        break;
      case SDL_EVENT_MOUSE_MOTION:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_DRAGGED, &ev);
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_RELEASED, &ev);
        break;
      case SDL_EVENT_WINDOW_FOCUS_GAINED:
        break;
      case SDL_EVENT_WINDOW_RESIZED:
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        onResize(ev.window.data1, ev.window.data2);
        break;
      case SDL_EVENT_WINDOW_EXPOSED:
      case SDL_EVENT_WINDOW_SHOWN:
        _graphics->redraw();
        break;
      case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        _output->removeHover();
        break;
      case SDL_EVENT_DROP_FILE:
        setLoadPath((const char *)ev.drop.data);
        setExit(false);
        break;
      case SDL_EVENT_MOUSE_WHEEL:
        if (!_output->scroll(ev.wheel.y == 1, false)) {
          maEvent = new MAEvent();
          maEvent->type = EVENT_TYPE_KEY_PRESSED;
          maEvent->key = ev.wheel.y == 1 ? SB_KEY_UP : SB_KEY_DN;
          maEvent->nativeKey = SDL_KMOD_CTRL;
        }
        break;
      }
      if (maEvent != nullptr) {
        pushEvent(maEvent);
      }
    }
  }
}

MAEvent Runtime::processEvents(int waitFlag) {
  switch (waitFlag) {
  case 1:
    // wait for an event
    _output->flush(true);
    if (!hasEvent()) {
      pollEvents(true);
    }
    break;
  case 2:
    _output->flush(false);
    pause(WAIT_INTERVAL);
    break;
  default:
    pollEvents(false);
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
  case EVENT_TYPE_KEY_PRESSED:
    handleKeyEvent(event);
    break;
  case EVENT_TYPE_RESTART:
    setRestart();
    break;
  default:
    handleEvent(event);
    break;
  }
}

void Runtime::setWindowRect(int x, int y, int width, int height) {
  logEntered();
  if (width > 0 && height > 0) {
    SDL_SetWindowSize(_window, width, height);
    pause(10);
  }
  if (x > 0 && y > 0) {
    SDL_SetWindowPosition(_window, x, y);
  }
}

void Runtime::setWindowTitle(const char *title) {
  if (strcmp(title, MAIN_BAS) == 0) {
    SDL_SetWindowTitle(_window, "SmallBASIC");
  } else {
    const char *slash = strrchr(title, '/');
    if (slash == nullptr) {
      slash = title;
    } else {
      slash++;
    }
    int len = strlen(slash) + 16;
    char *buffer = new char[len];
    if (buffer) {
      sprintf(buffer, "%s - SmallBASIC", slash);
      SDL_SetWindowTitle(_window, buffer);
      delete [] buffer;
    }
  }
}

void Runtime::showCursor(CursorType cursorType) {
  switch (cursorType) {
  case kHand:
    SDL_SetCursor(_cursorHand);
    break;
  case kArrow:
    SDL_SetCursor(_cursorArrow);
    break;
  case kIBeam:
    SDL_SetCursor(_cursorIBeam);
    break;
  }
}

void Runtime::onResize(int width, int height) {
  logEntered();
  if (_graphics != nullptr) {
    int w = _graphics->getWidth();
    int h = _graphics->getHeight();
    if (w != width || h != height) {
      trace("Resized from %d %d to %d %d", w, h, width, height);
      _graphics->resize(width, height);
      resize();
    }
  }
}

SDL_Rect Runtime::getWindowRect() {
  SDL_Rect result;
  if (_fullscreen) {
    result = _windowRect;
  } else {
    setWindowRect(result);
#if defined(__linux__)
    int top, left,bottom, right;
    SDL_GetWindowBordersSize(_window, &top, &left, &bottom, &right);
    // subtract the X11 border
    result.y -= top;
#endif
  }
  return result;
}

void Runtime::setWindowRect(SDL_Rect &rc) {
  SDL_GetWindowPosition(_window, &rc.x, &rc.y);
  SDL_GetWindowSize(_window, &rc.w, &rc.h);
}

void Runtime::setClipboardText(const char *text) {
  SDL_SetClipboardText(text);
}

char *Runtime::getClipboardText() {
  char *result;
  char *text = SDL_GetClipboardText();
  if (text && text[0]) {
    result = strdup(text);
    SDL_free(text);
  } else {
    result = nullptr;
  }
  return result;
}

void Runtime::onRunCompleted() {
  SDL_SetWindowPosition(_window, _saveRect.x, _saveRect.y);
  SDL_SetWindowSize(_window, _saveRect.w, _saveRect.h);
  setWindowRect(_saveRect.x, _saveRect.y, _saveRect.w, _saveRect.h);
}

void Runtime::saveWindowRect() {
  setWindowRect(_saveRect);
}

//
// System platform methods
//
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-in-bool-context"
bool System::getPen3() {
  SDL_PumpEvents();
  float x, y;
  bool result = SDL_BUTTON_MASK(SDL_BUTTON_LEFT) && SDL_GetMouseState(&x, &y);
  _touchCurX = x;
  _touchCurY = y;
  return result;

}
#pragma GCC diagnostic pop

void System::completeKeyword(int index) const {
  if (get_focus_edit() && isEditing()) {
    const char *help = get_focus_edit()->completeKeyword(index);
    if (help) {
      runtime->getOutput()->setStatus(help);
      runtime->getOutput()->redraw();
    }
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
// debugging
//
void signalTrace(bool debugBreak) {
  SDL_LockMutex(g_lock);
  g_debugPause = false;
  g_debugBreak = debugBreak;
  SDL_SignalCondition(g_cond);
  SDL_UnlockMutex(g_lock);
}

void dumpStack(socket_t socket) {
  net_print(socket, "\nStack:\n");
  for (int i = prog_stack_count - 1; i > -1; i--) {
    stknode_t node = prog_stack[i];
    switch (node.type) {
    case kwFUNC:
      net_print(socket, " FUNC\n");
      break;
    case kwPROC:
      net_print(socket, " SUB\n");
      break;
    }
  }
}

void dumpVariables(socket_t socket) {
  net_print(socket, "Variables:\n");

  bool localScope = false;
  int count = 0;
  for (int i = prog_stack_count - 1; !localScope && i > -1; i--) {
    stknode_t node = prog_stack[i];
    switch (node.type) {
    case kwTYPE_CRVAR:
      // local variable
      net_printf(socket, "[%d] ", count++);
      pv_writevar(tvar[node.x.vdvar.vid], PV_NET, socket);
      net_print(socket, "\n");
      break;
    case kwFUNC:
    case kwPROC:
      localScope = true;
      break;
    }
  }
  if (!localScope) {
    for (unsigned i = SYSVAR_COUNT; i < prog_varcount; i++) {
      if (!v_isempty(tvar[i])) {
        net_printf(socket, "[%d] ", count++);
        if (tvar[i]->const_flag) {
          net_print(socket, "const:");
        }
        pv_writevar(tvar[i], PV_NET, socket);
        net_print(socket, "\n");
      }
    }
  }
}

void restart() {
  SDL_LockMutex(g_lock);
  g_debugPause = false;
  g_debugBreak = false;
  g_debugTrace = false;
  g_breakPoints.removeAll();

  MAEvent *event = new MAEvent();
  event->type = EVENT_TYPE_RESTART;
  runtime->pushEvent(event);
  if (runtime->isThreadActive()) {
    // break out of waitForBack()
    SDL_Event event;
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = SDLK_BACKSPACE;
    SDL_PushEvent(&event);
  }

  SDL_SignalCondition(g_cond);
  SDL_UnlockMutex(g_lock);
}

int debugThread(void *data) {
  int port = ((intptr_t) data);
  socket_t socket = net_listen(port);
  char buf[OS_PATHNAME_SIZE + 1];

  if (socket == -1) {
    signalTrace(false);
    exit(1);
    return -1;
  }

  while (socket != -1) {
    int size = net_input(socket, buf, sizeof(buf), "\r\n");
    if (size > 0) {
      char cmd = buf[0];
      switch (cmd) {
      case 'n':
        // step over next line
        signalTrace(true);
        break;
      case 'c':
        // continue
        signalTrace(false);
        break;
      case 'l':
        // current line number
        SDL_LockMutex(g_lock);
        net_printf(socket, "%d\n", g_debugLine);
        SDL_UnlockMutex(g_lock);
        break;
      case 'v':
        // variables
        SDL_LockMutex(g_lock);
        if (!runtime->isRunning()) {
          net_printf(socket, "\n");
        } else {
          dumpVariables(socket);
          dumpStack(socket);
          net_print(socket, "\1");
        }
        SDL_UnlockMutex(g_lock);
        break;
      case 'b':
        // set breakpoint
        SDL_LockMutex(g_lock);
        g_breakPoints.add(new int(atoi(buf + 2)));
        SDL_UnlockMutex(g_lock);
        break;
      case 'q':
        // quit
        g_breakPoints.removeAll();
        net_disconnect(socket);
        socket = -1;
        runtime->setExit(true);
        break;
      case 'h':
        net_print(socket, "SmallBASIC debugger\n");
        break;
      case 'x':
        restart();
        break;
      default:
        // unknown command
        net_printf(socket, "Unknown command '%s'\n", buf);
        break;
      };
    }
  }
  return 0;
}

extern "C" void dev_trace_line(int lineNo) {
  SDL_LockMutex(g_lock);
  g_debugLine = lineNo;

  if (!g_debugBreak) {
    List_each(int *, it, g_breakPoints) {
      int breakPoint = *(*it);
      if (breakPoint == lineNo) {
        runtime->systemPrint("Break point hit at line: %d", lineNo);
        g_debugBreak = true;
        break;
      }
    }
  }
  if (g_debugBreak) {
    runtime->getOutput()->redraw();
    g_debugPause = true;
    while (g_debugPause) {
      // wait for g_debugPause condition to be signalled via signalTrace()
      SDL_WaitConditionTimeout(g_cond, g_lock, COND_WAIT_TIME);
      runtime->processEvents(0);
      if (!runtime->isRunning()) {
        break;
      }
    }
  } else if (!g_breakPoints.size() && g_debugTrace) {
    runtime->systemPrint("Trace line: %d", lineNo);
  }
  SDL_UnlockMutex(g_lock);
}
