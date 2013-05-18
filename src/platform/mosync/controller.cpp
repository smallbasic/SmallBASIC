// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <ma.h>
#include <MAUI/Engine.h>
#include <MAUI/Font.h>

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "MAHeaders.h"
#include <mavsprintf.h>

#include "platform/mosync/controller.h"
#include "platform/mosync/utils.h"

#define SYSTEM_MENU   "\033[ OConsole|Show Keypad|View Source"
#define MENU_CONSOLE  0
#define MENU_KEYPAD   1
#define MENU_SOURCE   2
#define MENU_ZOOM_UP  3
#define MENU_ZOOM_DN  4
#define ERROR_BAS "print \"Failed to open program file\""
#define FONT_SCALE_INTERVAL 10
#define FONT_MIN 20
#define FONT_MAX 200
#define STORE_VERSION 1

Controller::Controller() :
  Environment(),
  _output(NULL),
  _runMode(init_state),
  _lastEventTime(0),
  _eventTicks(0),
  _touchX(-1),
  _touchY(-1),
  _touchCurX(-1),
  _touchCurY(-1),
  _initialFontSize(0),
  _fontScale(100),
  _mainBas(false),
  _systemMenu(false),
  _systemScreen(false),
  _drainError(false),
  _programSrc(NULL) {
  logEntered();
}

bool Controller::construct() {
  MAExtent screenSize = maGetScrSize();
  _output = new AnsiWidget(this, EXTENT_X(screenSize), EXTENT_Y(screenSize));
  _output->construct();
  _initialFontSize = _output->getFontSize();

  _runMode = init_state;
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

  // restore the selected font scale and path
  MAHandle data = maCreatePlaceholder();
  MAHandle store = maOpenStore(PACKAGE, 0);

  if (store != STERR_NONEXISTENT) {
    if (maReadStore(store, data) == RES_OK) {
      int offset = 0;
      int storeVersion;
      int pathLength;
      char path[FILENAME_MAX + 1];
      
      maReadData(data, &storeVersion, offset, sizeof(int));
      offset += sizeof(int);

      if (storeVersion == STORE_VERSION) {
        maReadData(data, &_fontScale, offset, sizeof(int));
        offset += sizeof(int);        

        if (_fontScale != 100) {
          int fontSize = (_initialFontSize * _fontScale / 100);
          _output->setFontSize(fontSize);
        }
        
        maReadData(data, &pathLength, offset, sizeof(int));
        maReadData(data, &path, offset+ sizeof(int), pathLength);
        if (pathLength > 1) {
          chdir(path);
        }
      }
    }
    maCloseStore(store, 0);
  }
  maDestroyPlaceholder(data);

  return true;
}

Controller::~Controller() {
  // remember the selected font scale and path
  char path[FILENAME_MAX + 1];
  getcwd(path, FILENAME_MAX);
  int pathLength = strlen(path) + 1;
  int dataLength = (sizeof(int) * 3) + pathLength;
  MAHandle data = maCreatePlaceholder();

  if (maCreateData(data, dataLength) == RES_OK) {
    int storeVersion = STORE_VERSION;
    int offset = 0;

    // write the version number
    maWriteData(data, &storeVersion, offset, sizeof(int));
    offset += sizeof(int);

    // write the fontScale
    maWriteData(data, &_fontScale, offset, sizeof(int));
    offset += sizeof(int);
    
    // write the current path
    maWriteData(data, &pathLength, offset, sizeof(int));
    maWriteData(data, path, offset + sizeof(int), pathLength);

    MAHandle store = maOpenStore(PACKAGE, MAS_CREATE_IF_NECESSARY);
    maWriteStore(store, data);
    maCloseStore(store, 0);
  }
  maDestroyPlaceholder(data);

  delete _output;
  delete [] _programSrc;
}

const char *Controller::getLoadPath() {
  return !_loadPath.length() ? NULL : _loadPath.c_str();
}

int Controller::getPen(int code) {
  int result = 0;
  if (!isExit()) {
    switch (code) {
    case 0:
      // UNTIL PEN(0) - wait until click or move
      processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_POINTER_PRESSED);
      // fallthru

    case 3:   // returns true if the pen is down (and save curpos)
      if (_touchX != -1 && _touchY != -1) {
        result = 1;
      } else {
        processEvents(EVENT_WAIT_NONE);
      }
      break;

    case 1:   // last pen-down x
      result = _touchX;
      break;

    case 2:   // last pen-down y
      result = _touchY;
      break;

    case 4:   // cur pen-down x
    case 10:
      result = _touchCurX;
      break;

    case 5:   // cur pen-down y
    case 11:
      result = _touchCurY;
      break;
    }
  }
  return result;
}

char *Controller::getText(char *dest, int maxSize) {
  int x = _output->getX();
  int y = _output->getY();
  int w = EXTENT_X(maGetTextSize("YNM"));
  int h = _output->textHeight();

  dest[0] = '\0';
  _runMode = modal_state;
  IFormWidget *formWidget = _output->createLineInput(dest, maxSize, x, y, w, h);
  _output->redraw();
  maShowVirtualKeyboard();

  while (isModal()) {
    MAEvent event = processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_KEY_PRESSED);
    if (event.type == EVENT_TYPE_KEY_PRESSED) {
      dev_clrkb();
      if (isModal()) {
        if (event.key == 10) {
          _runMode = run_state;
        } else {
          _output->edit(formWidget, event.key);
        }
      }
    }
  }

  // paint the widget result onto the backing screen
  if (dest[0]) {
    _output->print(dest);
  }

  delete formWidget;
  return dest;
}

// runtime system event processor
int Controller::handleEvents(int waitFlag) {
  if (!waitFlag) {
    // detect when we have been called too frequently
    int now = maGetMilliSecondCount();
    _eventTicks++;
    if (now - _lastEventTime >= EVENT_CHECK_EVERY) {
      // next time inspection interval
      if (_eventTicks >= EVENT_MAX_BURN_TIME) {
        _output->print("\033[ LBattery drain");
        _drainError = true;
      } else if (_drainError) {
        _output->print("\033[ L");
        _drainError = false;
      }
      _lastEventTime = now;
      _eventTicks = 0;
    }
  }

  switch (waitFlag) {
  case 1:
    // wait for an event
    processEvents(EVENT_WAIT_INFINITE);
    break;
  case 2:
    // pause
    processEvents(EVENT_PAUSE_TIME);
    break;
  default:
    // pump messages without pausing
    processEvents(EVENT_WAIT_NONE);
    break;
  }

  _output->flush(false);
  return isBreak() ? -2 : 0;
}

// handle the system menu
void Controller::handleMenu(int menuId) {
  int fontSize = _output->getFontSize();
  _systemMenu = false;

  switch (menuId) {
  case MENU_SOURCE:
    showSystemScreen(true);
    break;
  case MENU_CONSOLE:
    showSystemScreen(false);
    break;
  case MENU_KEYPAD:
    maShowVirtualKeyboard();
    break;
  case MENU_ZOOM_UP:
    if (_fontScale > FONT_MIN) {
      _fontScale -= FONT_SCALE_INTERVAL;
      fontSize = (_initialFontSize * _fontScale / 100);
    }
    break;
  case MENU_ZOOM_DN:
    if (_fontScale < FONT_MAX) {
      _fontScale += FONT_SCALE_INTERVAL;
      fontSize = (_initialFontSize * _fontScale / 100);
    }
    break;
  }

  if (fontSize != _output->getFontSize()) {
    _output->setFontSize(fontSize);
    // restart the shell
    buttonClicked("main.bas");
    brun_break();
    _runMode = break_state;
  }
  
  if (!isRunning()) {
    _output->flush(true);
  }
}

// process events on the system event queue
MAEvent Controller::processEvents(int ms, int untilType) {
  MAEvent event;
  MAExtent screenSize;
  int loadPathSize = _loadPath.length();

  if (ms < 0 && untilType != -1) {
    // flush the display before pausing for target event
    if (isRunning()) {
      _output->flush(true);
    }
    maWait(ms);
    ms = EVENT_WAIT_NONE;
  }

  while (!isBreak() && maGetEvent(&event)) {
    switch (event.type) {
    case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
      if (_systemMenu) {
        handleMenu(event.optionsBoxButtonIndex);
        ms = EVENT_WAIT_NONE;
      } else if (isRunning()) {
        if (!_output->optionSelected(event.optionsBoxButtonIndex)) {
          dev_pushkey(event.optionsBoxButtonIndex);
        }
      }
      break;
    case EVENT_TYPE_SCREEN_CHANGED:
      screenSize = maGetScrSize();
      _output->resize(EXTENT_X(screenSize), EXTENT_Y(screenSize));
      os_graf_mx = _output->getWidth();
      os_graf_my = _output->getHeight();
      handleKey(SB_PKEY_SIZE_CHG);
      break;
    case EVENT_TYPE_POINTER_PRESSED:
      _touchX = _touchCurX = event.point.x;
      _touchY = _touchCurY = event.point.y;
      handleKey(SB_KEY_MK_PUSH);
      _output->pointerTouchEvent(event);
      break;
    case EVENT_TYPE_POINTER_DRAGGED:
      _touchCurX = event.point.x;
      _touchCurY = event.point.y;
      _output->pointerMoveEvent(event);
      break;
    case EVENT_TYPE_POINTER_RELEASED:
      _touchX = _touchY = _touchCurX = _touchCurY = -1;
      handleKey(SB_KEY_MK_RELEASE);
      _output->pointerReleaseEvent(event);
      break;
    case EVENT_TYPE_CLOSE:
      setExit(true);
      break;
    case EVENT_TYPE_KEY_PRESSED:
      handleKey(event.key);
      break;
    }
    if (untilType == EVENT_TYPE_EXIT_ANY ||
        untilType == event.type ||
        loadPathSize != _loadPath.length()) {
      // skip next maWait() - found target event or loadPath changed
      ms = EVENT_WAIT_NONE;
      break;
    }
  }

  if (ms != EVENT_WAIT_NONE) {
    maWait(ms);
  }
  return event;
}

char *Controller::readSource(const char *fileName) {
  char *buffer = NULL;
  bool networkFile = strstr(fileName, "://");
  const char *delim = strchr(fileName, '?');
  int len = strlen(fileName);
  int endIndex = delim ? (delim - fileName) : len;
  if (delim && !networkFile) {
    strcpy(opt_command, delim + 1);
  }
  
  _mainBas = false;
  trace("readSource %s %d %s", fileName, endIndex, opt_command);

  if (networkFile) {
    buffer = readConnection(fileName);
  } else if (strncasecmp("main.bas", fileName, endIndex) == 0) {
    // load as resource
    int len = maGetDataSize(MAIN_BAS);
    buffer = (char *)tmp_alloc(len + 1);
    maReadData(MAIN_BAS, buffer, 0, len);
    buffer[len] = '\0';
    _mainBas = true;
  } else {
    // load from file system
    MAHandle handle = maFileOpen(fileName, MA_ACCESS_READ);
    if (maFileExists(handle)) {
      int len = maFileSize(handle);
      buffer = (char *)tmp_alloc(len + 1);
      maFileRead(handle, buffer, len);
      buffer[len] = '\0';
    }
    maFileClose(handle);
  }

  if (buffer == NULL) {
    buffer = (char *)tmp_alloc(strlen(ERROR_BAS) + 1);
    strcpy(buffer, ERROR_BAS);
  }

  delete [] _programSrc;
  len = strlen(buffer);
  _programSrc = new char[len + 1];
  strncpy(_programSrc, buffer, len);
  _programSrc[len] = 0;

  logPrint("Opened: %s %d bytes\n", fileName, len);
  return buffer;
}

// stop and running program
void Controller::setExit(bool quit) {
  if (isRunning()) {
    brun_break();
  }
  _runMode = quit ? exit_state : back_state;
}

// commence runtime state
void Controller::setRunning(bool running) {
  if (running) {
    dev_fgcolor = -DEFAULT_FOREGROUND;
    dev_bgcolor = -DEFAULT_BACKGROUND;
    os_graf_mx = _output->getWidth();
    os_graf_my = _output->getHeight();

    os_ver = 1;
    os_color = 1;
    os_color_depth = 16;
    setsysvar_str(SYSVAR_OSNAME, "MoSync");

    dev_clrkb();
    ui_reset();

    _runMode = run_state;
    _loadPath.empty();
    _output->reset();

    _lastEventTime = maGetMilliSecondCount();
    _eventTicks = 0;
    _drainError = false;
  } else {
    _runMode = init_state;
  }
}

void Controller::showError() {
  _runMode = init_state;
  _loadPath.empty();
  showSystemScreen(false);
}

void Controller::showCompletion(bool success) {
  if (success) {
    _output->print("\033[ LDone - press back [<-]");
  } else {
    _output->print("\033[ LError - see console");
  }
  _output->flush(true);
}

void Controller::showMenu() {
  _systemMenu = true;
  if (_mainBas) {
    char buffer[128];
    sprintf(buffer, "%s|Zoom %d%%|Zoom %d%%", SYSTEM_MENU, 
            _fontScale - FONT_SCALE_INTERVAL,
            _fontScale + FONT_SCALE_INTERVAL);
    _output->print(buffer);
  } else {
    _output->print(SYSTEM_MENU);
  }
}

void Controller::logPrint(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof(buf) - 1, format, args);
  va_end(args);
  *p = '\0';

  lprintfln(buf);

  if (_systemScreen) {
    _output->print(buf);
  } else {
    _output->print("\033[ SW7");
    _output->print(buf);
    _output->print("\033[ Sw");
  }
}

// handler for hyperlink click actions
void Controller::buttonClicked(const char *url) {
  _loadPath.empty();
  _loadPath.append(url, strlen(url));
}

// pass the key into the smallbasic keyboard handler
void Controller::handleKey(int key) {
  switch (key) {
  case MAK_FIRE:
  case MAK_5:
    return;
  case MAK_SOFTRIGHT:
  case MAK_BACK:
    if (_systemScreen) {
      // restore user screens
      _output->print("\033[ SR");
      _systemScreen = false;
    } else {
      // quit app when shell is active
      setExit(_mainBas);
    }
    return;
  case MAK_MENU:
    showMenu();
    return;
  }

  if (isRunning()) {
    switch (key) {
    case MAK_TAB:
      dev_pushkey(SB_KEY_TAB);
      break;
    case MAK_HOME:
      dev_pushkey(SB_KEY_KP_HOME);
      break;
    case MAK_END:
      dev_pushkey(SB_KEY_END);
      break;
    case MAK_INSERT:
      dev_pushkey(SB_KEY_INSERT);
      break;
    case MAK_KP_MULTIPLY:
      dev_pushkey(SB_KEY_KP_MUL);
      break;
    case MAK_KP_PLUS:
      dev_pushkey(SB_KEY_KP_PLUS);
      break;
    case MAK_KP_MINUS:
      dev_pushkey(SB_KEY_KP_MINUS);
      break;
    case MAK_SLASH:
      dev_pushkey(SB_KEY_KP_DIV);
      break;
    case MAK_PAGEUP:
      dev_pushkey(SB_KEY_PGUP);
      break;
    case MAK_PAGEDOWN:
      dev_pushkey(SB_KEY_PGDN);
      break;
    case MAK_UP:
      dev_pushkey(SB_KEY_UP);
      break;
    case MAK_DOWN:
      dev_pushkey(SB_KEY_DN);
      break;
    case MAK_LEFT:
      dev_pushkey(SB_KEY_LEFT);
      break;
    case MAK_RIGHT:
      dev_pushkey(SB_KEY_RIGHT);
      break;
    case MAK_CLEAR:
    case MAK_BACKSPACE:
    case MAK_DELETE:
      dev_pushkey(SB_KEY_BACKSPACE);
      break;
    default:
      dev_pushkey(key);
      break;
    }
  }
}

// returns the contents of the given url
char *Controller::readConnection(const char *url) {
  char *result = NULL;
  logEntered();
  _output->print("\033[ LLoading...");

  MAHandle conn = maConnect(url);
  if (conn < 1) {
    logPrint("Failed connecting to %s\n", url);
  } else {
    _runMode = conn_state;
    logPrint("Connecting to %s\n", url);

    bool connected = false;
    byte buffer[1024];
    int length = 0;
    int size = 0;
    int now = maGetMilliSecondCount();
    MAEvent event;

    // pause until connected
    while (_runMode == conn_state) {
      event = processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_CONN);
      if (event.type == EVENT_TYPE_CONN) {
        switch (event.conn.opType) {
        case CONNOP_CONNECT:
          // connection established
          if (!connected) {
            connected = (event.conn.result > 0);
            if (connected) {
              memset(buffer, 0, sizeof(buffer));
              maConnRead(conn, buffer, sizeof(buffer));
            } else {
              logPrint("Connection error\n");
              _runMode = init_state;
            }
          }
          break;
        case CONNOP_READ:
          // connRead completed
          if (event.conn.result > 0) {
            size = event.conn.result;
            if (result == NULL) {
              result = (char *)tmp_alloc(size + 1);
              memcpy(result, buffer, size);
              length = size;
            } else {
              result = (char *)tmp_realloc(result, length + size + 1);
              memcpy(result + length, buffer, size);
              length += size;
            }
            result[length] = 0;
            memset(buffer, 0, sizeof(buffer));
            maConnRead(conn, buffer, sizeof(buffer));
          } else {
            // no more data
            _runMode = init_state;
          }
          break;
        default:
          logPrint("Connection error\n");
          _runMode = init_state;
        }
      }
    }
    logPrint("Loaded in %d msecs\n", maGetMilliSecondCount() - now);
  }

  maConnClose(conn);
  return result;
}

void Controller::showSystemScreen(bool showSrc) {
  if (showSrc) {
    // screen command write screen 2 (\014=CLS)
    _output->print("\033[ SW6\014");
    if (_programSrc) {
      _output->print(_programSrc);
    }
    // restore write screen, display screen 6 (source)
    _output->print("\033[ Sw; SD6");
  } else {
    // screen command display screen 7 (console)
    _output->print("\033[ SD7");
  }
  _systemScreen = true;
}
