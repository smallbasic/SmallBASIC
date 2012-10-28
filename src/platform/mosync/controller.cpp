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

#define SYSTEM_MENU "\033[ OView Source|Show Console|Show Keypad"
#define MENU_SOURCE 0
#define MENU_LOG    1
#define MENU_KEYPAD 2
#define ERROR_BAS "print \"Failed to open program file\""

Controller::Controller() :
  Environment(),
  output(NULL),
  runMode(init_state),
  lastEventTime(0),
  eventTicks(0),
  touchX(-1),
  touchY(-1),
  touchCurX(-1),
  touchCurY(-1),
  systemMenu(false),
  systemScreen(false),
  drainError(false),
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
    switch (code) {
    case 0:
      // UNTIL PEN(0) - wait until click or move
      processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_POINTER_PRESSED);
      // fallthru

    case 3:   // returns true if the pen is down (and save curpos)
      if (touchX != -1 && touchY != -1) {
        result = 1;
      } else {
        processEvents(EVENT_WAIT_NONE);
      }
      break;

    case 1:   // last pen-down x
      result = touchX;
      break;

    case 2:   // last pen-down y
      result = touchY;
      break;

    case 4:   // cur pen-down x
    case 10:
      result = touchCurX;
      break;

    case 5:   // cur pen-down y
    case 11:
      result = touchCurY;
      break;
    }
  }
  return result;
}

char *Controller::getText(char *dest, int maxSize) {
  int x = output->getX();
  int y = output->getY();
  int w = EXTENT_X(maGetTextSize("sample text"));
  int h = output->textHeight();

  dest[0] = '\0';
  runMode = modal_state;
  IFormWidget *formWidget = output->createLineInput(dest, maxSize, x, y, w, h);
  output->redraw();
  maShowVirtualKeyboard();

  while (isModal()) {
    MAEvent event = processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_KEY_PRESSED);
    if (event.type == EVENT_TYPE_KEY_PRESSED) {
      dev_clrkb();
      if (isModal()) {
        if (event.key == 10) {
          runMode = run_state;
        } else {
          output->edit(formWidget, event.key);
        }
      }
    }
  }

  delete formWidget;
  return dest;
}

// runtime system event processor
int Controller::handleEvents(int waitFlag) {
  if (!waitFlag) {
    // detect when we have been called too frequently
    int now = maGetMilliSecondCount();
    eventTicks++;
    if (now - lastEventTime >= EVENT_CHECK_EVERY) {
      // next time inspection interval
      if (eventTicks >= EVENT_MAX_BURN_TIME) {
        output->print("\033[ LBattery drain");
        drainError = true;
      } else if (drainError) {
        output->print("\033[ L");
        drainError = false;
      }
      lastEventTime = now;
      eventTicks = 0;
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

  output->flush(false);
  return isBreak() ? -2 : 0;
}

// process events on the system event queue
MAEvent Controller::processEvents(int ms, int untilType) {
  MAEvent event;
  MAExtent screenSize;
  int loadPathSize = loadPath.size();

  if (ms < 0 && untilType != -1) {
    // flush the display before pausing for target event
    if (isRunning()) {
      output->flush(true);
    }
    maWait(ms);
    ms = EVENT_WAIT_NONE;
  }

  while (!isBreak() && maGetEvent(&event)) {
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
        if (!output->optionSelected(event.optionsBoxButtonIndex)) {
          dev_pushkey(event.optionsBoxButtonIndex);
        }
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
      touchX = touchCurX = event.point.x;
      touchY = touchCurY = event.point.y;
      handleKey(SB_KEY_MK_PUSH);
      output->pointerTouchEvent(event);
      break;
    case EVENT_TYPE_POINTER_DRAGGED:
      touchCurX = event.point.x;
      touchCurY = event.point.y;
      output->pointerMoveEvent(event);
      break;
    case EVENT_TYPE_POINTER_RELEASED:
      touchX = touchY = touchCurX = touchCurY = -1;
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
    if (untilType == EVENT_TYPE_EXIT_ANY ||
        untilType == event.type ||
        loadPathSize != loadPath.size()) {
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

  trace("readSource %s %d %s", fileName, endIndex, opt_command);

  if (networkFile) {
    buffer = readConnection(fileName);
  } else if (strncasecmp("main.bas", fileName, endIndex) == 0) {
    // load as resource
    int len = maGetDataSize(MAIN_BAS);
    buffer = (char *)tmp_alloc(len + 1);
    maReadData(MAIN_BAS, buffer, 0, len);
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

    lastEventTime = maGetMilliSecondCount();
    eventTicks = 0;
    drainError = false;
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
  if (success) {
    output->print("\033[ LDone - press back [<-]");
  } else {
    output->print("\033[ LError - see console");
  }
  output->flush(true);
}

void Controller::logPrint(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof(buf) - 1, format, args);
  va_end(args);
  *p = '\0';

  lprintfln(buf);

  if (systemScreen) {
    output->print(buf);
  } else {
    output->print("\033[ SW7");
    output->print(buf);
    output->print("\033[ Sw");
  }
}

// handler for hyperlink click actions
void Controller::buttonClicked(const char *url) {
  loadPath.clear();
  loadPath.append(url, strlen(url));
}

// pass the key into the smallbasic keyboard handler
void Controller::handleKey(int key) {
  switch (key) {
  case MAK_FIRE:
  case MAK_5:
    return;
  case MAK_SOFTRIGHT:
  case MAK_BACK:
    if (systemScreen) {
      // restore user screens
      output->print("\033[ SR");
      systemScreen = false;
    } else {
      setExit(true);
    }
    return;
  case MAK_MENU:
    systemMenu = true;
    output->print(SYSTEM_MENU);
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
  output->print("\033[ LLoading...");

  MAHandle conn = maConnect(url);
  if (conn < 1) {
    logPrint("Failed connecting to %s\n", url);
  } else {
    runMode = conn_state;
    logPrint("Connecting to %s\n", url);

    bool connected = false;
    byte buffer[1024];
    int length = 0;
    int size = 0;
    int now = maGetMilliSecondCount();
    MAEvent event;

    // pause until connected
    while (runMode == conn_state) {
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
              runMode = init_state;
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

  maConnClose(conn);
  return result;
}

void Controller::showSystemScreen(bool showSrc) {
  if (showSrc) {
    // screen command write screen 2 (\014=CLS)
    output->print("\033[ SW6\014");
    if (programSrc) {
      output->print(programSrc);
    }
    // restore write screen, display screen 6 (source)
    output->print("\033[ Sw; SD6");
  } else {
    // screen command display screen 7 (console)
    output->print("\033[ SD7");
  }
  systemScreen = true;
}
