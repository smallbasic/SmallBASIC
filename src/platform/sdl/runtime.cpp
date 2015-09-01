// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/fs_socket_client.h"
#include "common/keymap.h"
#include "common/inet.h"
#include "common/pproc.h"
#include "lib/maapi.h"
#include "ui/utils.h"
#include "platform/sdl/runtime.h"
#include "platform/sdl/syswm.h"
#include "platform/sdl/keymap.h"
#include "platform/sdl/main_bas.h"

#include <SDL_clipboard.h>
#include <SDL_audio.h>
#include <queue>
#include <cmath>

#define WAIT_INTERVAL 5
#define COND_WAIT_TIME 250
#define MAIN_BAS "__main_bas__"
#define AMPLITUDE 22000
#define FREQUENCY 44100
#define OPTIONS_BOX_WIDTH_EXTRA 4
#define OPTIONS_BOX_BG 0xd2d1d0
#define OPTIONS_BOX_FG 0x3e3f3e

Runtime *runtime;
SDL_mutex *g_lock = NULL;
SDL_cond *g_cond = NULL;
SDL_bool g_debugPause = SDL_FALSE;
SDL_bool g_debugBreak = SDL_FALSE;
SDL_bool g_debugError = SDL_FALSE;
strlib::List<int*> g_breakPoints;

struct SoundObject {
  double v;
  double freq;
  int samplesLeft;
};

std::queue<SoundObject> sounds;
void audio_callback(void *beeper, Uint8 *stream8, int length);
int debugThread(void *data);

MAEvent *getMotionEvent(int type, SDL_Event *event) {
  MAEvent *result = new MAEvent();
  result->type = type;
  result->point.x = event->motion.x;
  result->point.y = event->motion.y;
  return result;
}

Runtime::Runtime(SDL_Window *window) :
  System(),
  _menuX(2),
  _menuY(2),
  _graphics(NULL),
  _eventQueue(NULL),
  _window(window),
  _cursorHand(NULL),
  _cursorArrow(NULL),
  _cursorIBeam(NULL) {
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

  SDL_FreeCursor(_cursorHand);
  SDL_FreeCursor(_cursorArrow);
  SDL_FreeCursor(_cursorIBeam);
  _cursorHand = NULL;
  _cursorArrow = NULL;
  _cursorIBeam = NULL;
}

void Runtime::alert(const char *title, const char *message) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, _window);
}

int Runtime::ask(const char *title, const char *prompt, bool cancel) {
  SDL_MessageBoxButtonData buttons[cancel ? 3: 2];
  memset(&buttons[0], 0, sizeof(SDL_MessageBoxButtonData));
  memset(&buttons[1], 0, sizeof(SDL_MessageBoxButtonData));
  buttons[0].text = "Yes";
  buttons[0].buttonid = 0;
  buttons[1].text = "No";
  buttons[1].buttonid = 1;
  if (cancel) {
    memset(&buttons[2], 0, sizeof(SDL_MessageBoxButtonData));
    buttons[2].text = "Cancel";
    buttons[2].buttonid = 2;
  }

  SDL_MessageBoxData data;
  memset(&data, 0, sizeof(SDL_MessageBoxData));
  data.window = _window;
  data.title = title;
  data.message = prompt;
  data.flags = SDL_MESSAGEBOX_INFORMATION;
  data.numbuttons = cancel ? 3 : 2;
  data.buttons = buttons;

  int buttonId;
  SDL_ShowMessageBox(&data, &buttonId);
  return buttonId;
}

void Runtime::construct(const char *font, const char *boldFont) {
  logEntered();
  _state = kClosingState;
  _graphics = new Graphics(_window);

  _cursorHand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
  _cursorArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  _cursorIBeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

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

void Runtime::pushEvent(MAEvent *event) {
  _eventQueue->push(event);
}

MAEvent *Runtime::popEvent() {
  return _eventQueue->pop();
}

int Runtime::runShell(const char *startupBas, int fontScale, int debugPort) {
  logEntered();

  os_graphics = 1;
  os_color_depth = 16;
  opt_interactive = true;
  opt_usevmt = 0;
  opt_file_permitted = 1;
  opt_graphics = true;
  opt_pref_bpp = 0;
  opt_nosave = true;

  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(getStartupFontSize(_window));
  _initialFontSize = _output->getFontSize();
  if (fontScale != 100) {
    _fontScale = fontScale;
    int fontSize = (_initialFontSize * _fontScale / 100);
    _output->setFontSize(fontSize);
  }

  SDL_Init(SDL_INIT_AUDIO);
  SDL_AudioSpec desiredSpec;
  desiredSpec.freq = FREQUENCY;
  desiredSpec.format = AUDIO_S16SYS;
  desiredSpec.channels = 1;
  desiredSpec.samples = 2048;
  desiredSpec.callback = audio_callback;

  SDL_AudioSpec obtainedSpec;
  SDL_OpenAudio(&desiredSpec, &obtainedSpec);

  if (debugPort > 0) {
    appLog("Debug active on port %d\n", debugPort);
    g_lock = SDL_CreateMutex();
    g_cond = SDL_CreateCond();
    opt_trace_on = 1;
    g_debugBreak = SDL_TRUE;
    SDL_Thread *thread =
      SDL_CreateThread(debugThread, "DBg", (void *)(intptr_t)debugPort);
    SDL_DetachThread(thread);
  }

  if (startupBas != NULL) {
    String bas = startupBas;
    if (opt_ide == IDE_INTERNAL) {
      runEdit(bas.c_str());
    } else {
      runOnce(bas.c_str());
    }
    while (_state == kRestartState) {
      _state = kActiveState;
      if (_loadPath.length() != 0) {
        bas = _loadPath;
      }
      runOnce(bas.c_str());
    }
  } else {
    runMain(MAIN_BAS);
  }

  if (debugPort > 0) {
    SDL_DestroyCond(g_cond);
    SDL_DestroyMutex(g_lock);
  }

  SDL_CloseAudio();
  _state = kDoneState;
  logLeaving();
  return _fontScale;
}

char *Runtime::loadResource(const char *fileName) {
  logEntered();
  char *buffer = System::loadResource(fileName);
  if (buffer == NULL && strcmp(fileName, MAIN_BAS) == 0) {
    buffer = (char *)malloc(main_bas_len + 1);
    memcpy(buffer, main_bas, main_bas_len);
    buffer[main_bas_len] = '\0';
  }
  return buffer;
}

void Runtime::handleKeyEvent(MAEvent &event) {
  int lenMap = sizeof(keymap) / sizeof(keymap[0]);
  for (int i = 0; i < lenMap && event.key != -1; i++) {
    if (event.key == keymap[i][0]) {
      event.key = keymap[i][1];
      break;
    }
  }

  // handle keypad keys
  if (event.key != -1) {
    if (event.key == SDLK_NUMLOCKCLEAR) {
      event.key = -1;
    } else if (event.key == SDLK_KP_1) {
      event.key = event.nativeKey == KMOD_NUM ? '1' : SB_KEY_END;
    } else if (event.key == SDLK_KP_2) {
      event.key = event.nativeKey == KMOD_NUM ? '2' : SB_KEY_DN;
    } else if (event.key == SDLK_KP_3) {
      event.key = event.nativeKey == KMOD_NUM ? '3' : SB_KEY_PGDN;
    } else if (event.key == SDLK_KP_4) {
      event.key = event.nativeKey == KMOD_NUM ? '4' : SB_KEY_LEFT;
    } else if (event.key == SDLK_KP_5) {
      event.key = '5';
    } else if (event.key == SDLK_KP_6) {
      event.key = event.nativeKey == KMOD_NUM ? '6' : SB_KEY_RIGHT;
    } else if (event.key == SDLK_KP_7) {
      event.key = event.nativeKey == KMOD_NUM ? '7' : SB_KEY_HOME;
    } else if (event.key == SDLK_KP_8) {
      event.key = event.nativeKey == KMOD_NUM ? '8' : SB_KEY_UP;
    } else if (event.key == SDLK_KP_9) {
      event.key = event.nativeKey == KMOD_NUM ? '9' : SB_KEY_PGUP;
    }
  }

  // handle ALT/SHIFT/CTRL states
  if (event.key != -1) {
    if ((event.nativeKey & KMOD_CTRL) &&
        (event.nativeKey & KMOD_ALT)) {
      event.key = SB_KEY_CTRL_ALT(event.key);
    } else if (event.nativeKey & KMOD_CTRL) {
      event.key = SB_KEY_CTRL(event.key);
    } else if (event.nativeKey & KMOD_ALT) {
      event.key = SB_KEY_ALT(event.key);
    } else if (event.nativeKey & KMOD_SHIFT) {
      bool shifted = false;
      if (event.key >= SDLK_a && event.key <= SDLK_z) {
        event.key = 'A' + (event.key - SDLK_a);
        shifted = true;
      } else {
        lenMap = sizeof(shiftmap) / sizeof(shiftmap[0]);
        for (int i = 0; i < lenMap; i++) {
          if (shiftmap[i][0] == event.key) {
            event.key = shiftmap[i][1];
            shifted = true;
            break;
          }
        }
      }
      if (!shifted) {
        event.key = SB_KEY_SHIFT(event.key);
      }
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
      MAEvent *maEvent = NULL;
      switch (ev.type) {
      case SDL_QUIT:
        setExit(true);
        break;
      case SDL_KEYDOWN:
        if (!isEditing() && ev.key.keysym.sym == SDLK_c
            && (ev.key.keysym.mod & KMOD_CTRL)) {
          setExit(true);
        } else if (ev.key.keysym.sym == SDLK_m && (ev.key.keysym.mod & KMOD_CTRL)) {
          showMenu();
        } else if (ev.key.keysym.sym == SDLK_b && (ev.key.keysym.mod & KMOD_CTRL)) {
          setBack();
        } else if (ev.key.keysym.sym == SDLK_BACKSPACE &&
                   get_focus_edit() == NULL &&
                   ((ev.key.keysym.mod & KMOD_CTRL) || !isRunning())) {
          setBack();
        } else if (!isEditing() && ev.key.keysym.sym == SDLK_PAGEUP &&
                   (ev.key.keysym.mod & KMOD_CTRL)) {
          _output->scroll(true, true);
        } else if (!isEditing() && ev.key.keysym.sym == SDLK_PAGEDOWN &&
                   (ev.key.keysym.mod & KMOD_CTRL)) {
          _output->scroll(false, true);
        } else if (!isEditing() && ev.key.keysym.sym == SDLK_UP &&
                   (ev.key.keysym.mod & KMOD_CTRL)) {
          _output->scroll(true, false);
        } else if (!isEditing() && ev.key.keysym.sym == SDLK_DOWN &&
                   (ev.key.keysym.mod & KMOD_CTRL)) {
          _output->scroll(false, false);
        } else if (ev.key.keysym.sym == SDLK_p && (ev.key.keysym.mod & KMOD_CTRL)) {
          ::screen_dump();
        } else {
          maEvent = new MAEvent();
          maEvent->type = EVENT_TYPE_KEY_PRESSED;
          maEvent->key = ev.key.keysym.sym;
          maEvent->nativeKey = ev.key.keysym.mod;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (ev.button.button == SDL_BUTTON_RIGHT) {
          _menuX = ev.motion.x;
          _menuY = ev.motion.y;
          showMenu();
        } else if (ev.motion.x != 0 && ev.motion.y != 0) {
          // avoid phantom down message when launching in windows
          maEvent = getMotionEvent(EVENT_TYPE_POINTER_PRESSED, &ev);
        }
        break;
      case SDL_MOUSEMOTION:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_DRAGGED, &ev);
        break;
      case SDL_MOUSEBUTTONUP:
        SDL_SetCursor(_cursorArrow);
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_RELEASED, &ev);
        break;
      case SDL_WINDOWEVENT:
        switch (ev.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
          onResize(ev.window.data1, ev.window.data2);
          break;
        case SDL_WINDOWEVENT_EXPOSED:
          _graphics->redraw();
          break;
        case SDL_WINDOWEVENT_LEAVE:
          _output->removeHover();
          break;
        }
        break;
      case SDL_DROPFILE:
        setLoadPath(ev.drop.file);
        setExit(false);
        SDL_free(ev.drop.file);
        break;
      case SDL_MOUSEWHEEL:
        if (!_output->scroll(ev.wheel.y == 1, false)) {
          maEvent = new MAEvent();
          maEvent->type = EVENT_TYPE_KEY_PRESSED;
          maEvent->key = ev.wheel.y == 1 ? SDLK_UP : SDLK_DOWN;
          maEvent->nativeKey = 0;
        }
        break;
      }
      if (maEvent != NULL) {
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
    pollEvents(true);
    break;
  case 2:
    _output->flush(false);
    pause(WAIT_INTERVAL);
    break;
  default:
    pollEvents(false);
    checkLoadError();
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
  default:
    handleEvent(event);
    break;
  }
}

void Runtime::setWindowTitle(const char *title) {
  if (strcmp(title, MAIN_BAS) == 0) {
    SDL_SetWindowTitle(_window, "SmallBASIC");
  } else {
    const char *slash = strrchr(title, '/');
    if (slash == NULL) {
      slash = title;
    } else {
      slash++;
    }
    int len = strlen(slash) + 16;
    char *buffer = new char[len];
    sprintf(buffer, "%s - SmallBASIC", slash);
    SDL_SetWindowTitle(_window, buffer);
    delete [] buffer;
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
  if (_graphics != NULL) {
    int w = _graphics->getWidth();
    int h = _graphics->getHeight();
    if (w != width || h != height) {
      trace("Resized from %d %d to %d %d", w, h, width, height);
      _graphics->resize(width, height);
      resize();
    }
  }
}

void Runtime::optionsBox(StringList *items) {
  if (!_menuX) {
    _menuX = 2;
  }
  if (!_menuY) {
    _menuY = 2;
  }

  int width = 0;
  int charWidth = _output->getCharWidth();
  _output->registerScreen(MENU_SCREEN);
  List_each(String *, it, *items) {
    char *str = (char *)(* it)->c_str();
    int w = (strlen(str) * charWidth);
    if (w > width) {
      width = w;
    }
  }
  width += (charWidth * OPTIONS_BOX_WIDTH_EXTRA);

  int charHeight = _output->getCharHeight();
  int textHeight = charHeight + (charHeight / 2);
  int height = textHeight * items->size();
  if (_menuX + width >= _output->getWidth()) {
    _menuX = _output->getWidth() - width;
  }
  if (_menuY + height >= _output->getHeight()) {
    _menuY = _output->getHeight() - height;
  }

  int screenId = _output->insetMenuScreen(_menuX, _menuY, width, height);
  int y = 0;
  int index = 0;
  int selectedIndex = -1;
  int releaseCount = 0;

  List_each(String *, it, *items) {
    char *str = (char *)(* it)->c_str();
    FormInput *item = new MenuButton(index, selectedIndex, str, 0, y, width, textHeight);
    _output->addInput(item);
    item->setColor(OPTIONS_BOX_BG, OPTIONS_BOX_FG);
    index++;
    y += textHeight;
  }

  _output->redraw();
  while (selectedIndex == -1) {
    MAEvent ev = processEvents(true);
    if (ev.type == EVENT_TYPE_KEY_PRESSED &&
        ev.key == 27) {
      break;
    }
    if (ev.type == EVENT_TYPE_POINTER_RELEASED &&
        ++releaseCount == 2) {
      break;
    }
  }

  _output->removeInputs();
  _output->selectScreen(screenId);
  _menuX = 2;
  _menuY = 2;
  if (selectedIndex != -1) {
    if (_systemMenu == NULL && isRunning() &&
        !form_ui::optionSelected(selectedIndex)) {
      dev_clrkb();
      dev_pushkey(selectedIndex);
    } else {
      MAEvent *maEvent = new MAEvent();
      maEvent->type = EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED;
      maEvent->optionsBoxButtonIndex = selectedIndex;
      pushEvent(maEvent);
    }
  } else {
    delete [] _systemMenu;
    _systemMenu = NULL;
  }

  _output->redraw();
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
    result = NULL;
  }
  return result;
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
  runtime->pause(timeout);
}

//
// audio
//
void audio_callback(void *beeper, Uint8 *stream8, int length) {
  Sint16 *stream = (Sint16 *)stream8;
  int samples = length / 2;
  int i = 0;
  while (i < samples) {
    if (sounds.empty()) {
      while (i < samples) {
        stream[i] = 0;
        i++;
      }
      return;
    }

    SoundObject &sound = sounds.front();
    int samplesToDo = std::min(i + sound.samplesLeft, samples);
    sound.samplesLeft -= samplesToDo - i;

    while (i < samplesToDo) {
      stream[i] = AMPLITUDE * std::sin(sound.v * 2 * M_PI / FREQUENCY);
      sound.v += sound.freq;
      i++;
    }
    if (sound.samplesLeft == 0) {
      sounds.pop();
    }
  }
}

void do_beep(double freq, int duration) {
  SoundObject sound;
  sound.freq = freq;
  sound.samplesLeft = duration * FREQUENCY / 1000;
  sound.v = 0;

  SDL_LockAudio();
  sounds.push(sound);
  SDL_UnlockAudio();
}

void flush_queue() {
  int size;
  int last_size = 0;
  int unplayed = 0;

  do {
    SDL_Delay(20);
    SDL_LockAudio();
    size = sounds.size();
    if (size != last_size) {
      unplayed++;
    } else {
      last_size = size;
    }
    SDL_UnlockAudio();
  } while (size > 0 && unplayed < 50);
}

//
// sbasic implementation
//
int osd_devinit(void) {
  setsysvar_str(SYSVAR_OSNAME, "SDL");
  runtime->setRunning(true);
  osd_clear_sound_queue();
  return 1;
}

int osd_devrestore(void) {
  runtime->setRunning(false);
  return 0;
}

void osd_beep() {
  SDL_PauseAudio(0);
  do_beep(1000, 30);
  do_beep(500, 30);
  flush_queue();
}

void osd_sound(int frq, int ms, int vol, int bgplay) {
  SDL_PauseAudio(0);
  do_beep(frq, ms);
  if (!bgplay) {
    flush_queue();
  }
}

void osd_clear_sound_queue() {
  flush_queue();
}

//
// debugging
//
void signalTrace(SDL_bool debugBreak, SDL_bool debugError = SDL_FALSE) {
  SDL_LockMutex(g_lock);
  g_debugPause = SDL_FALSE;
  g_debugBreak = debugBreak;
  g_debugError = debugError;
  SDL_CondSignal(g_cond);
  SDL_UnlockMutex(g_lock);
}

int debugThread(void *data) {
  int port = ((intptr_t) data);
  socket_t socket = net_listen(port);
  char buf[OS_PATHNAME_SIZE + 1];

  if (socket == -1) {
    signalTrace(SDL_FALSE, SDL_TRUE);
    return -1;
  }

  net_print(socket, "SmallBASIC debugger\n");
  while (socket != -1) {
    int size = net_input(socket, buf, sizeof(buf), "\r\n");
    if (size > 0) {
      char cmd = buf[0];
      switch (cmd) {
      case 'n':
        // step over next line
        signalTrace(SDL_TRUE);
        break;
      case 's':
        // status
        net_printf(socket, "%d\n", prog_line);
        for (unsigned i = 0; i < prog_varcount; i++) {
          pv_writevar(tvar[i], PV_NET, socket);
          net_print(socket, "\n");
        }
        break;
      case 'c':
        // continue
        signalTrace(SDL_FALSE);
        break;
      case 'b':
        // set breakpoint
        SDL_LockMutex(g_lock);
        g_breakPoints.add(new int(atoi(buf + 2)));
        SDL_UnlockMutex(g_lock);
        break;
      case 'q':
        // quit
        signalTrace(SDL_FALSE, SDL_TRUE);
        g_breakPoints.removeAll();
        net_print(socket, "Bye\n");
        net_disconnect(socket);
        socket = -1;
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

  if (!g_debugError) {
    if (!g_debugBreak) {
      List_each(int *, it, g_breakPoints) {
        int breakPoint = *(*it);
        if (breakPoint == lineNo) {
          runtime->systemPrint("Break point hit at line: %d", lineNo);
          g_debugBreak = SDL_TRUE;
          break;
        }
      }
    }
    if (g_debugBreak) {
      g_debugPause = SDL_TRUE;
      while (g_debugPause) {
        SDL_CondWaitTimeout(g_cond, g_lock, COND_WAIT_TIME);
        runtime->processEvents(0);
        if (!runtime->isRunning()) {
          break;
        }
      }
    }
  } else {
    runtime->setExit(true);
  }
  SDL_UnlockMutex(g_lock);
}

