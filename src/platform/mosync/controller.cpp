// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
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

#include "platform/mosync/controller.h"
#include "platform/mosync/utils.h"

#define LONG_PRESS_TIME 4000
#define SYSTEM_MENU "\033[ OView Source|Show Console|Show Keypad\034"
#define MENU_SOURCE 0
#define MENU_LOG    1
#define MENU_KEYPAD 2

#define FILE_MGR_RES "filemgr.bas"
#define ERROR_BAS "print \"Failed to open program file\""

Controller::Controller() :
  Environment(),
  output(0),
  runMode(init_state),
  lastEventTime(0),
  eventsPerTick(0),
  penMode(PEN_OFF),
  penDownX(-1),
  penDownY(-1),
  penDownTime(0),
  systemMenu(false),
  systemScreen(false),
  programSrc(NULL) {
  logEntered();
}

bool Controller::construct() {
  MAExtent screenSize = maGetScrSize();
  output = new AnsiWidget(this, EXTENT_X(screenSize), EXTENT_Y(screenSize));
  output->construct();

  runMode = init_state;
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

  return true;
}

Controller::~Controller() {
  delete output;
  delete [] programSrc;
}

const char *Controller::getLoadPath() {
  return !loadPath.size() ? NULL : loadPath.c_str();
}

int Controller::getPen(int code) {
  int result = 0;
  if (!isExit()) {
    if (penMode == PEN_OFF) {
      processEvents(0, -1);
    }

    switch (code) {
    case 0:
      // UNTIL PEN(0) - wait until move click or move
      processEvents(1, -1);    // fallthru to re-test

    case 3:                    // returns true if the pen is down (and save curpos)
      processEvents(0, -1);
      if (penDownX != -1 && penDownY != -1) {
        result = 1;
      }
      break;

    case 1:                      // last pen-down x
      result = penDownX;
      break;

    case 2:                      // last pen-down y
      result = penDownY;
      break;

    case 4:                      // cur pen-down x
    case 10:
      processEvents(0, -1);
      result = penDownX;
      break;

    case 5:                      // cur pen-down y
    case 11:
      processEvents(0, -1);
      result = penDownY;
      break;
    }
  }
  return result;
}

// runtime system event processor
int Controller::handleEvents(int waitFlag) {
  if (!waitFlag) {
    // pause when we have been called too frequently
    int now = maGetMilliSecondCount();
    if (now - lastEventTime <= EVT_CHECK_EVERY) {
      eventsPerTick += (now - lastEventTime);
      if (eventsPerTick >= EVT_MAX_BURN_TIME) {
        eventsPerTick = 0;
        waitFlag = 2;
      }
    }
    lastEventTime = now;
  }

  switch (waitFlag) {
  case 1:
    // wait for an event
    processEvents(-1, -1);
    break;
  case 2:
    // pause
    processEvents(EVT_PAUSE_TIME, -1);
    break;
  default:
    // pump messages without pausing
    processEvents(0, -1);
    break;
  }

  output->flush(false);
  return isBreak() ? -2 : 0;
}

// process events while in modal state
void Controller::modalLoop() {
  runMode = modal_state;
  while (runMode == modal_state) {
    processEvents(-1, -1);
  }
}

// pause for the given number of milliseconds
void Controller::pause(int ms) {
  if (runMode == run_state) {
    int msWait = ms / 2;
    int msStart = maGetMilliSecondCount();
    runMode = modal_state;
    while (runMode == modal_state) {
      if (maGetMilliSecondCount() - msStart >= ms) {
        runMode = run_state;
        break;
      }
      processEvents(msWait, -1);
    }
  } else {
    MAEvent event;
    int msStart = maGetMilliSecondCount();
    while (maGetMilliSecondCount() - msStart < ms) {
      if (!maGetEvent(&event)) {
        maWait(10);
      }
    }
  }
}

// process events on the system event queue
MAEvent Controller::processEvents(int ms, int untilType) {
  MAEvent event;
  MAExtent screenSize;
  int loadPathSize = loadPath.size();

  // long press = menu
  if (penDownTime != 0) {
    int now = maGetMilliSecondCount();
    if ((now - penDownTime) > LONG_PRESS_TIME) {
      penDownTime = now;
      systemMenu = true;
      output->print(SYSTEM_MENU);
    }
  }

  while (!isBreak() && maGetEvent(&event)) {
    if (isModal()) {
      // process events for any active GUI
      fireEvent(event);
    }

    switch (event.type) {
    case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
      if (systemMenu) {
        systemMenu = false;
        switch (event.optionsBoxButtonIndex) {
        case MENU_SOURCE:
          showSystemScreen(true);
          break;
        case MENU_LOG:
          showSystemScreen(false);
          break;
        case MENU_KEYPAD:
          maShowVirtualKeyboard();
          break;
        }
        if (!isRunning()) {
          output->flush(true);
        }
      } else if (isRunning()) {
        dev_pushkey(event.optionsBoxButtonIndex);
      }
      break;
    case EVENT_TYPE_SCREEN_CHANGED:
      screenSize = maGetScrSize();
      output->resize(EXTENT_X(screenSize), EXTENT_Y(screenSize));
      os_graf_mx = output->getWidth();
      os_graf_my = output->getHeight();
      handleKey(SB_PKEY_SIZE_CHG);
      break;
    case EVENT_TYPE_POINTER_PRESSED:
      penDownTime = maGetMilliSecondCount();
      penDownX = event.point.x;
      penDownY = event.point.y;
      handleKey(SB_KEY_MK_PUSH);
      output->pointerTouchEvent(event);
      break;
    case EVENT_TYPE_POINTER_DRAGGED:
      output->pointerMoveEvent(event);
      break;
    case EVENT_TYPE_POINTER_RELEASED:
      penDownTime = 0;
      penDownX = penDownY = -1;
      handleKey(SB_KEY_MK_RELEASE);
      output->pointerReleaseEvent(event);
      break;
    case EVENT_TYPE_CLOSE:
      setExit(false);
      break;
    case EVENT_TYPE_KEY_PRESSED:
      handleKey(event.key);
      break;
    }
    if ((untilType != -1 && untilType == event.type) || 
        (loadPathSize != loadPath.size())) {
      // found target event or loadPath changed
      break;
    }
  }

  if (isRunning() || (loadPathSize == loadPath.size())) {
    // pump messages into the engine
    runIdleListeners();

    // avoid pausing when loadPath has changed
    if (ms != 0) {
      maWait(ms);
    }
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

  trace("readSource %s %d %s", fileName, endIndex, opt_command);
  
  if (networkFile) {
    buffer = readConnection(fileName);
  } else if (strncasecmp(MAIN_BAS_RES, fileName, endIndex) == 0) {
    // load as resource
    int len = maGetDataSize(MAIN_BAS);
    buffer = (char *)tmp_alloc(len + 1);
    maReadData(MAIN_BAS, buffer, 0, len);
    buffer[len] = '\0';
  } else if (strncasecmp(FILE_MGR_RES, fileName, endIndex) == 0) {
    // load as resource
    int len = maGetDataSize(FILEMGR_BAS);
    buffer = (char *)tmp_alloc(len + 1);
    maReadData(FILEMGR_BAS, buffer, 0, len);
    buffer[len] = '\0';
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

  delete [] programSrc;
  len = strlen(buffer);
  programSrc = new char[len + 1];
  strncpy(programSrc, buffer, len);
  programSrc[len] = 0;

  logPrint("Opened: %s %d bytes\n", fileName, len);
  return buffer;
}

// stop and running program
void Controller::setExit(bool back) {
  if (isRunning()) {
    ui_reset();
    brun_break();
  }
  runMode = back ? back_state : exit_state; 
}

// commence runtime state
void Controller::setRunning(bool running) {
  if (running) {
    dev_fgcolor = -DEFAULT_COLOR;
    dev_bgcolor = 0;
    os_graf_mx = output->getWidth();
    os_graf_my = output->getHeight();
    
    os_ver = 1;
    os_color = 1;
    os_color_depth = 16;
    setsysvar_str(SYSVAR_OSNAME, "MoSync");
    
    dev_clrkb();
    ui_reset();
    
    runMode = run_state;
    loadPath.clear();
    output->reset();
  } else {
    runMode = init_state;
  }
}

void Controller::showError() {
  runMode = init_state;
  loadPath.clear();
  showSystemScreen(false);
}

void Controller::showCompletion(bool success) {
  int w = output->getWidth() - 1;
  int h = output->getHeight() - 1;
  int c = output->getColor();
  output->setColor(success ? 1 : 4);
  output->drawRect(0, 0, w, h);
  output->setColor(c);
  output->flush(true);
}

void Controller::logPrint(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof(buf) - 1, format, args);
  va_end(args);
  *p = '\0';

  trace(buf);
  if (systemScreen) {
    output->print(buf);
  } else {
    output->print("\033[ SW3\034");
    output->print(buf);
    output->print("\033[ Sw\034");
  }
}

// handler for hyperlink click actions
void Controller::buttonClicked(const char *url) {
  loadPath.clear();
  int len = strlen(url);
  if (len == 1) {
    handleKey(url[0]);
  } else {
    loadPath.append(url, strlen(url));
  }
}

// pass the event into the mosync framework
void Controller::fireEvent(MAEvent &event) {
  switch (event.type) {
  case EVENT_TYPE_CLOSE:
    fireCloseEvent();
    break;
  case EVENT_TYPE_FOCUS_GAINED:
    fireFocusGainedEvent();
    break;
  case EVENT_TYPE_FOCUS_LOST:
    fireFocusLostEvent();
    break;
  case EVENT_TYPE_KEY_PRESSED:
    fireKeyPressEvent(event.key, event.nativeKey);
    break;
  case EVENT_TYPE_KEY_RELEASED:
    fireKeyReleaseEvent(event.key, event.nativeKey);
    break;
  case EVENT_TYPE_CHAR:
    fireCharEvent(event.character);
    break;
  case EVENT_TYPE_POINTER_PRESSED:
    if (event.touchId == 0) {
      firePointerPressEvent(event.point);
    }
    fireMultitouchPressEvent(event.point, event.touchId);
    break;
  case EVENT_TYPE_POINTER_DRAGGED:
    if (event.touchId == 0) {
      firePointerMoveEvent(event.point);
    }
    fireMultitouchMoveEvent(event.point, event.touchId);
    break;
  case EVENT_TYPE_POINTER_RELEASED:
    if (event.touchId == 0) {
      firePointerReleaseEvent(event.point);
    }
    fireMultitouchReleaseEvent(event.point, event.touchId);
    break;
  case EVENT_TYPE_CONN:
    fireConnEvent(event.conn);
    break;
  case EVENT_TYPE_BT:
    fireBluetoothEvent(event.state);
    break;
  case EVENT_TYPE_TEXTBOX:
    fireTextBoxListeners(event.textboxResult, event.textboxLength);
    break;
  case EVENT_TYPE_SENSOR:
    fireSensorListeners(event.sensor);
    break;
  default:
    fireCustomEventListeners(event);
    break;
  }
}

// pass the key into the smallbasic keyboard handler
void Controller::handleKey(int key) {
  switch (key) {
  case MAK_FIRE:
  case MAK_5:
    break;
  case MAK_SOFTRIGHT:
  case MAK_BACK:
    if (systemScreen) {
      // restore the runtime screen
      output->print("\033[ Sp\034");
      systemScreen = false;
    } else {
      setExit(true);
    }
    break;
  case MAK_MENU:
    systemMenu = true;
    output->print(SYSTEM_MENU);
    break;
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

  output->print("\nLoading...");

  MAHandle conn = maConnect(url);
  if (conn < 1) {
    logPrint("Failed connecting to %s\n", url);
  } else {
    runMode = conn_state;
    logPrint("Connecting to %s\n", url);
    bool connected = false;
    byte buffer[256];
    int length = 0;
    int size = 0;
    int now = maGetMilliSecondCount();
    MAEvent event;

    // pause until connected
    while (runMode == conn_state) {
      event = processEvents(0, EVENT_TYPE_CONN);
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
              runMode = init_state;
            }
          }
          break;
        case CONNOP_READ:
          // connRead completed
          if (event.conn.result > 0) {
            // ensure previous read has completed. the next
            // or final completion event will unblock this
            maWait(1000);
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
            runMode = init_state;
          }
          break;
        default:
          logPrint("Connection error\n");
          runMode = init_state;
        }
      }
    }
    logPrint("Loaded in %d msecs\n", maGetMilliSecondCount() - now);
  }

  output->print("done");
  maConnClose(conn);
  return result;
}

void Controller::showSystemScreen(bool showSrc) {
  if (!systemScreen) {
    // remember the current user screen
    output->print("\033[ SP\034");
  }

  systemScreen = true;

  if (showSrc) {
    // screen command write screen 2 (\014=CLS)
    output->print("\033[ SW2\034\014");
    if (programSrc) {
      output->print(programSrc);
    }
    // screen command display write screen
    output->print("\033[ Sd\034");
  } else {
    // screen command display screen 3
    output->print("\033[ SD3\034");
  }
}
