// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "platform/common/system.h"
#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"

System::System() :
  _output(NULL),
  _state(kInitState),
  _lastEventTime(0),
  _eventTicks(0),
  _drainError(false),
  _systemScreen(false),
  _mainBas(false),
  _programSrc(NULL) {
}

System::~System() {
}

// set the current working directory to the given path
void System::setPath(const char *filename) {
  const char *slash = strrchr(filename, '/');
  if (!slash) {
    slash = strrchr(filename, '\\');
  }
  if (slash) {
    int len = slash - filename;
    if (len > 0) {
      char path[1024];
      strncpy(path, filename, len);
      path[len] = 0;
      chdir(path);
    }
  }
}

// change the current working directory to the parent level folder
bool System::setParentPath() {
  bool result = true;
  char path[FILENAME_MAX + 1];
  getcwd(path, FILENAME_MAX);
  if (!path[0] || strcmp(path, "/") == 0) {
    result = false;
  } else {
    int len = strlen(path);
    if (path[len - 1] == '/') {
      // eg /sdcard/bas/
      path[len - 1] = '\0';
    }
    const char *slash = strrchr(path, '/');
    len = slash - path;
    if (!len) {
      strcpy(path, "/");
    } else {
      path[len] = 0;
    }
    chdir(path);
  }
  return result;
}

int System::getPen(int code) {
  int result = 0;
  MAEvent event;
  if (!isClosing()) {
    switch (code) {
    case 0:
      // UNTIL PEN(0) - wait until click or move
      event = getNextEvent();
      // fallthru

    case 3:   // returns true if the pen is down (and save curpos)
//      if (_touchX != -1 && _touchY != -1) {
//        result = 1;
//      } else {
//        processEvents(EVENT_WAIT_NONE);
//      }
      break;

    case 1:   // last pen-down x
//      result = _touchX;
      break;

    case 2:   // last pen-down y
//      result = _touchY;
      break;

    case 4:   // cur pen-down x
    case 10:
//      result = _touchCurX;
      break;

    case 5:   // cur pen-down y
    case 11:
//      result = _touchCurY;
      break;
    }
  }
  return result;
}

char *System::getText(char *dest, int maxSize) {
  int x = _output->getX();
  int y = _output->getY();
  int w = EXTENT_X(maGetTextSize("YNM"));
  int h = _output->textHeight();

  dest[0] = '\0';
  _state = kModalState;
  IFormWidget *formWidget = _output->createLineInput(dest, maxSize, x, y, w, h);
  _output->redraw();
  maShowVirtualKeyboard();

  while (isModal()) {
    MAEvent event = getNextEvent();
    if (event.type == EVENT_TYPE_KEY_PRESSED) {
      dev_clrkb();
      if (isModal()) {
        if (event.key == 10) {
          _state = kRunState;
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

void System::setBack() {
  if (_systemScreen) {
    // restore user screens
    _output->print("\033[ SR");
    _systemScreen = false;
  } else {
    // quit app when shell is active
    setExit(_mainBas);
  }
}

void System::setRunning(bool running) {
  if (running) {
    dev_fgcolor = -DEFAULT_FOREGROUND;
    dev_bgcolor = -DEFAULT_BACKGROUND;
    os_graf_mx = _output->getWidth();
    os_graf_my = _output->getHeight();
    
    os_ver = 1;
    os_color = 1;
    os_color_depth = 16;
    
    dev_clrkb();
    ui_reset();
    
    _output->reset();
    _state = kRunState;
    _loadPath.empty();
    _lastEventTime = maGetMilliSecondCount();
    _eventTicks = 0;
    _drainError = false;
  } else if (!isClosing()) {
    _state = kActiveState;
  }
}

void System::systemPrint(const char *str) {
  if (isSystemScreen()) {
    _output->print(str);
  } else {
    _output->print("\033[ SW7");
    _output->print(str);
    _output->print("\033[ Sw");
  }
}

void System::runMain(const char *mainBasPath) {
  logEntered();

  _mainBas = true;
  strcpy(opt_command, "welcome");
  sbasic_main(mainBasPath);

  while (!isClosing()) {
    if (isBack()) {
      if (_mainBas) {
        setExit(!setParentPath());
      }
      if (!isClosing()) {
        _mainBas = true;
        opt_command[0] = '\0';
        sbasic_main(mainBasPath);
      }
    } else if (getLoadPath() != NULL) {
      _mainBas = (strncmp(getLoadPath(), "main.bas", 8) == 0);
      if (!_mainBas) {
        setPath(getLoadPath());
      }
      bool success = sbasic_main(getLoadPath());
      if (!isBack()) {
        if (!_mainBas) {
          // display an indication the program has completed
          showCompletion(success);
        }
        if (!success) {
          // highlight the error
          showError();
        }
      }
    } else {
      getNextEvent();
    }
  }
}

void System::showCompletion(bool success) {
  if (success) {
    _output->print("\033[ LDone - press back [<-]");
  } else {
    _output->print("\033[ LError - see console");
  }
  _output->flush(true);
}

void System::showError() {
  _state = kActiveState;
  _loadPath.empty();
  showSystemScreen(false);
}

void System::showSystemScreen(bool showSrc) {
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

const char *System::getLoadPath() {
  return !_loadPath.length() ? NULL : _loadPath.c_str();
}

