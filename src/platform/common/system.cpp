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
#include "common/fs_socket_client.h"

#define SYSTEM_MENU   "\033[ OConsole|Show keypad|View source|Restart"
#define MENU_CONSOLE  0
#define MENU_KEYPAD   1
#define MENU_SOURCE   2
#define MENU_RESTART  3
#define MENU_ZOOM_UP  4
#define MENU_ZOOM_DN  5
#define FONT_SCALE_INTERVAL 10
#define FONT_MIN 20
#define FONT_MAX 200
#define EVENT_CHECK_EVERY 2000
#define EVENT_MAX_BURN_TIME 30

System *g_system;

System::System() :
  _output(NULL),
  _state(kInitState),
  _lastEventTime(0),
  _eventTicks(0),
  _touchX(-1),
  _touchY(-1),
  _touchCurX(-1),
  _touchCurY(-1),
  _initialFontSize(0),
  _fontScale(100),
  _overruns(0),
  _systemMenu(false),
  _systemScreen(false),
  _mainBas(false),
  _buttonPressed(false),
  _programSrc(NULL) {
  g_system = this;
}

System::~System() {
  delete [] _programSrc;
}

void System::buttonClicked(const char *url) {
  _loadPath.empty();
  _loadPath.append(url, strlen(url));
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
      if (_touchX != -1 && _touchY != -1) {
        result = 1;
      } else {
        osd_events(0);
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
        if (event.key == SB_KEY_ENTER) {
          _state = kRunState;
        } else {
          _output->edit(formWidget, event.key);
        }
      }
    }
  }

  // paint the widget result onto the backing screen
  if (dest[0]) {
    _output->setXY(x, y);
    _output->print(dest);
  }

  delete formWidget;
  return dest;
}

void System::handleMenu(int menuId) {
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
  case MENU_RESTART:
    if (isRunning()) {
      brun_break();
    }
    _state = kRestartState;
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
    // restart the shell
    _output->setFontSize(fontSize);
    if (isRunning()) {
      brun_break();
    }
    _state = kRestartState;
  }

  if (!isRunning()) {
    _output->flush(true);
  }
}

void System::handleEvent(MAEvent event) {
  switch (event.type) {
  case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
    if (_systemMenu) {
      handleMenu(event.optionsBoxButtonIndex);
    } else if (isRunning()) {
      if (!_output->optionSelected(event.optionsBoxButtonIndex)) {
        dev_pushkey(event.optionsBoxButtonIndex);
      }
    }
    break;
  case EVENT_TYPE_SCREEN_CHANGED:
    resize();
    break;
  case EVENT_TYPE_POINTER_PRESSED:
    _touchX = _touchCurX = event.point.x;
    _touchY = _touchCurY = event.point.y;
    dev_pushkey(SB_KEY_MK_PUSH);
    _buttonPressed = _output->pointerTouchEvent(event);
    break;
  case EVENT_TYPE_POINTER_DRAGGED:
    _touchCurX = event.point.x;
    _touchCurY = event.point.y;
    _output->pointerMoveEvent(event);
    break;
  case EVENT_TYPE_POINTER_RELEASED:
    _buttonPressed = false;
    _touchX = _touchY = _touchCurX = _touchCurY = -1;
    _output->pointerReleaseEvent(event);
    break;
  default:
    // no event
    _output->flush(false);
    break;
  }
}

char *System::loadResource(const char *fileName) {
  char *buffer = NULL;
  if (strstr(fileName, "://") != NULL) {
    int handle = 1;
    var_t *var_p = v_new();
    dev_file_t *f = dev_getfileptr(handle);
    _output->print("\033[ LLoading...");
    if (dev_fopen(handle, fileName, 0)) {
      http_read(f, var_p, 0);
      int len = var_p->v.p.size;
      buffer = (char *)tmp_alloc(len + 1);
      memcpy(buffer, var_p->v.p.ptr, len);
      buffer[len] = '\0';
    } else {
      systemPrint("\nfailed");
    }
    dev_fclose(handle);
    v_free(var_p);
    tmp_free(var_p);
  }
  return buffer;
}

char *System::readSource(const char *fileName) {
  char *buffer = loadResource(fileName);
  if (!buffer) {
    int h = open(fileName, O_BINARY | O_RDONLY, 0644);
    if (h != -1) {
      int len = lseek(h, 0, SEEK_END);
      lseek(h, 0, SEEK_SET);
      buffer = (char *)tmp_alloc(len + 1);
      len = read(h, buffer, len);
      buffer[len] = '\0';
      close(h);
    }
  }
  if (buffer != NULL) {
    delete [] _programSrc;
    int len = strlen(buffer);
    _programSrc = new char[len + 1];
    strncpy(_programSrc, buffer, len);
    _programSrc[len] = 0;
    systemPrint("Opened: %s %d bytes\n", fileName, len);
  }
  return buffer;
}

void System::resize() {
  MAExtent screenSize = maGetScrSize();
  logEntered();
  _output->resize(EXTENT_X(screenSize), EXTENT_Y(screenSize));
  os_graf_mx = _output->getWidth();
  os_graf_my = _output->getHeight();
  dev_pushkey(SB_PKEY_SIZE_CHG);
}

void System::runMain(const char *mainBasPath) {
  logEntered();

  // activePath provides the program name after termination
  String activePath = mainBasPath;
  _loadPath = mainBasPath;
  _mainBas = true;
  strcpy(opt_command, "welcome");
  sbasic_main(_loadPath);

  while (!isClosing()) {
    if (isRestart()) {
      _loadPath = activePath;
      _state = kActiveState;
    } else {
      if (_loadPath.length() > 0) {
        _mainBas = false;
        activePath = _loadPath;
        setPath(_loadPath);
      } else {
        _mainBas = true;
        _loadPath = mainBasPath;
        activePath = mainBasPath;
      }
    }
    opt_command[0] = '\0';
    bool success = sbasic_main(_loadPath);
    _output->flush(true);
    if (!isClosing() && _overruns) {
      systemPrint("\nOverruns: %d\n", _overruns);
    }
    if (!isBack() && !isClosing()) {
      // load the next network file without displaying the previous result
      bool networkFile = (_loadPath.indexOf("://", 1) != -1);
      if (!_mainBas && !networkFile) {
        // display an indication the program has completed
        showCompletion(success);
      }
      if (!success) {
        // highlight the error
        showError();
      }
      if (!_mainBas && !networkFile) {
        // press back to continue
        while (!isBack() && !isClosing() && !isRestart()) {
          getNextEvent();
        }
      }
    }
  }
}

void System::runOnce(const char *startupBas) {
  logEntered();

  _loadPath = startupBas;
  _mainBas = false;
  bool success = sbasic_main(_loadPath);
  showCompletion(success);
  // press back to continue
  while (!isBack() && !isClosing() && !isRestart()) {
    getNextEvent();
  }
}

void System::setBack() {
  if (_systemScreen) {
    // restore user screens00
    _output->print("\033[ SR");
    _systemScreen = false;
  } else {
    // quit app when shell is active
    setExit(_mainBas);
  }
}

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

void System::setPath(const char *filename) {
  if (strstr(filename, "://") == NULL) {
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
    _overruns = 0;
  } else if (!isClosing() && !isRestart() && !isBack()) {
    _state = kActiveState;
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

// detect when we have been called too frequently
void System::checkLoadError() {
  int now = maGetMilliSecondCount();
  _eventTicks++;
  if (now - _lastEventTime >= EVENT_CHECK_EVERY) {
    // next time inspection interval
    if (_eventTicks >= EVENT_MAX_BURN_TIME) {
      _overruns++;
    }
    _lastEventTime = now;
    _eventTicks = 0;
  }
}

void System::showMenu() {
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

void System::showSystemScreen(bool showSrc) {
  if (showSrc) {
    // screen command write screen 6 (\014=CLS)
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

void System::systemPrint(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof(buf) - 1, format, args);
  va_end(args);
  *p = '\0';

  deviceLog("%s", buf);

  if (isSystemScreen()) {
    _output->print(buf);
  } else {
    _output->print("\033[ SW7");
    _output->print(buf);
    _output->print("\033[ Sw");
  }
}

//
// common device implementation
//
void osd_cls(void) {
  logEntered();
  ui_reset();
  g_system->_output->clearScreen();
}

int osd_devrestore(void) {
  ui_reset();
  g_system->setRunning(false);
  return 0;
}

int osd_events(int wait_flag) {
  int result;
  if (g_system->isBreak()) {
    result = -2;
  } else {
    g_system->processEvents(wait_flag);
    result = g_system->isBreak() ? -2 : 0;
  }
  return result;
}

int osd_getpen(int mode) {
  return g_system->getPen(mode);
}

long osd_getpixel(int x, int y) {
  return g_system->_output->getPixel(x, y);
}

int osd_getx(void) {
  return g_system->_output->getX();
}

int osd_gety(void) {
  return g_system->_output->getY();
}

void osd_line(int x1, int y1, int x2, int y2) {
  g_system->_output->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (fill) {
    g_system->_output->drawRectFilled(x1, y1, x2, y2);
  } else {
    g_system->_output->drawRect(x1, y1, x2, y2);
  }
}

void osd_refresh(void) {
  if (!g_system->isClosing()) {
    g_system->_output->flush(true);
  }
}

void osd_setcolor(long color) {
  if (!g_system->isClosing()) {
    g_system->_output->setColor(color);
  }
}

void osd_setpenmode(int enable) {
  // touch mode is always active
}

void osd_setpixel(int x, int y) {
  g_system->_output->setPixel(x, y, dev_fgcolor);
}

void osd_settextcolor(long fg, long bg) {
  g_system->_output->setTextColor(fg, bg);
}

void osd_setxy(int x, int y) {
  g_system->_output->setXY(x, y);
}

int osd_textheight(const char *str) {
  return g_system->_output->textHeight();
}

int osd_textwidth(const char *str) {
  MAExtent textSize = maGetTextSize(str);
  return EXTENT_X(textSize);
}

void osd_write(const char *str) {
  if (!g_system->isClosing()) {
    g_system->_output->print(str);
  }
}

void lwrite(const char *str) {
  if (!g_system->isClosing()) {
    g_system->systemPrint(str);
  }
}

void dev_delay(dword ms) {
  maWait(ms);
}

char *dev_gets(char *dest, int maxSize) {
  return g_system->getText(dest, maxSize);
}

char *dev_read(const char *fileName) {
  return g_system->readSource(fileName);
}


