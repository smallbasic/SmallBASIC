// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/fs_socket_client.h"
#include "common/keymap.h"
#include "ui/system.h"
#include "ui/inputs.h"
#include "ui/textedit.h"

#define MENU_CONSOLE    0
#define MENU_SOURCE     1
#define MENU_BACK       2
#define MENU_RESTART    3
#define MENU_KEYPAD     4
#define MENU_ZOOM_UP    5
#define MENU_ZOOM_DN    6
#define MENU_CUT        7
#define MENU_COPY       8
#define MENU_PASTE      9
#define MENU_CTRL_MODE  10
#define MENU_LIVEMODE   11
#define MENU_AUDIO      12
#define MENU_SCREENSHOT 13
#define MENU_EDIT_SRC   14
#define MENU_SIZE       15

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
  _userScreenId(-1),
  _systemMenu(NULL),
  _mainBas(false),
  _buttonPressed(false),
  _liveMode(false),
  _srcRendered(false),
  _menuActive(false),
  _programSrc(NULL),
  _modifiedTime(0) {
  g_system = this;
}

System::~System() {
  delete [] _systemMenu;
  delete [] _programSrc;

  _systemMenu = NULL;
  _programSrc = NULL;
}

void System::checkModifiedTime() {
  if (_liveMode && _activeFile.length() > 0 &&
      _modifiedTime != getModifiedTime()) {
    setRestart();
  }
}

void System::editSource() {
  logEntered();

  int w = _output->getWidth();
  int h = _output->getHeight();
  int charWidth = _output->getCharWidth();
  int charHeight = _output->getCharHeight();
  int prevScreenId = _output->selectScreen(SOURCE_SCREEN);

  TextEditInput *widget = new TextEditInput(_programSrc, charWidth, charHeight, 0, 0, w, h);
  widget->updateUI(NULL, NULL);
  widget->setFocus();
  _srcRendered = false;
  _output->clearScreen();
  _output->addInput(widget);
  _output->redraw();
  maShowVirtualKeyboard();

  while (!isClosing()) {
    MAEvent event = getNextEvent();
    if (event.type == EVENT_TYPE_KEY_PRESSED) {
      dev_clrkb();
      int sw = _output->getScreenWidth();
      if (widget->edit(event.key, sw, charWidth)) {
        _output->redraw();
      }
    }
  }

  if (!isClosing()) {
    _output->removeInput(widget);
    _output->selectScreen(prevScreenId);
  }
  logLeaving();
}

bool System::execute(const char *bas) {
  _output->reset();

  // reset program controlled options
  opt_antialias = true;
  opt_show_page = false;
  opt_quiet = true;
  opt_pref_width = _output->getWidth();
  opt_pref_height = _output->getHeight();
  opt_base = 0;
  opt_uipos = 0;
  opt_usepcre = 0;

  _state = kRunState;
  setWindowTitle(bas);
  bool result = ::sbasic_main(bas);

  if (isRunning()) {
    _state = kActiveState;
  }

  opt_command[0] = '\0';
  _output->flush(true);
  return result;
}

int System::getPen(int code) {
  int result = 0;
  if (!isClosing()) {
    switch (code) {
    case 0:
      // UNTIL PEN(0) - wait until click or move
      processEvents(1);
      // fallthru

    case 3:   // returns true if the pen is down (and save curpos)
      if (_touchX != -1 && _touchY != -1) {
        result = 1;
      } else {
        processEvents(0);
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

    case 12:  // true if left button pressed
      return _buttonPressed;
    }
  }
  return result;
}

char *System::getText(char *dest, int maxSize) {
  int x = _output->getX();
  int y = _output->getY();
  int w = EXTENT_X(maGetTextSize("YNM"));
  int h = _output->textHeight();
  int charWidth = _output->getCharWidth();

  FormInput *widget = new FormLineInput(NULL, maxSize, true, x, y, w, h);
  widget->setFocus();
  _output->addInput(widget);
  _output->redraw();
  _state = kModalState;
  maShowVirtualKeyboard();

  while (isModal()) {
    MAEvent event = getNextEvent();
    if (event.type == EVENT_TYPE_KEY_PRESSED) {
      dev_clrkb();
      if (isModal()) {
        int sw = _output->getScreenWidth();
        switch (event.key) {
        case SB_KEY_ENTER:
          _state = kRunState;
          break;
        case SB_KEY_MENU:
          break;
        default:
          if (widget->edit(event.key, sw, charWidth)) {
            _output->redraw();
          }
        }
      }
    }
  }

  const char *result = widget->getText();
  if (result) {
    strcpy(dest, result);
  } else {
    dest[0] = '\0';
  }

  // paint the widget result onto the backing screen
  if (dest[0]) {
    _output->setXY(x, y);
    _output->print(dest);
  }

  _output->removeInput(widget);
  delete widget;
  return dest;
}

uint32_t System::getModifiedTime() {
  uint32_t result = 0;
  if (_activeFile.length() > 0) {
    struct stat st_file;
    if (!stat(_activeFile.c_str(), &st_file)) {
      result = st_file.st_mtime;
    }
  }
  return result;
}

void System::handleMenu(int menuId) {
  int fontSize = _output->getFontSize();
  int menuItem = _systemMenu[menuId];
  delete [] _systemMenu;
  _systemMenu = NULL;

  switch (menuItem) {
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
    setRestart();
    break;
  case MENU_BACK:
    setBack();
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
  case MENU_COPY:
  case MENU_CUT:
    if (get_focus_edit() != NULL) {
      char *text = get_focus_edit()->copy(menuItem == MENU_CUT);
      if (text) {
        setClipboardText(text);
        free(text);
        _output->redraw();
      }
    }
    break;
  case MENU_PASTE:
    if (get_focus_edit() != NULL) {
      char *text = getClipboardText();
      get_focus_edit()->paste(text);
      _output->redraw();
      free(text);
    }
    break;
  case MENU_CTRL_MODE:
    if (get_focus_edit() != NULL) {
      bool controlMode = get_focus_edit()->getControlMode();
      get_focus_edit()->setControlMode(!controlMode);
    }
    break;
  case MENU_LIVEMODE:
    _liveMode = !_liveMode;
    break;
  case MENU_AUDIO:
    opt_mute_audio = !opt_mute_audio;
    break;
  case MENU_SCREENSHOT:
    ::screen_dump();
    break;
  case MENU_EDIT_SRC:
    editSource();
    break;
  }

  if (fontSize != _output->getFontSize()) {
    // restart the shell
    _output->setFontSize(fontSize);
    setRestart();
  }

  if (!isRunning()) {
    _output->flush(true);
  }
}

void System::handleEvent(MAEvent &event) {
  bool hasHover;

  switch (event.type) {
  case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
    if (_systemMenu != NULL) {
      handleMenu(event.optionsBoxButtonIndex);
    } else if (isRunning()) {
      if (!form_ui::optionSelected(event.optionsBoxButtonIndex)) {
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
    if (_buttonPressed) {
      showCursor(true);
    }
    break;
  case EVENT_TYPE_POINTER_DRAGGED:
    _touchCurX = event.point.x;
    _touchCurY = event.point.y;
    hasHover = _output->hasHover();
    _output->pointerMoveEvent(event);
    if (hasHover != _output->hasHover()) {
      showCursor(!hasHover);
    }
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
  if (_liveMode) {
    checkModifiedTime();
  }
}

char *System::loadResource(const char *fileName) {
  char *buffer = NULL;
  if (strstr(fileName, "://") != NULL) {
    int handle = 1;
    var_t *var_p = v_new();
    dev_file_t *f = dev_getfileptr(handle);
    _output->setStatus("Loading...");
    _output->redraw();
    if (dev_fopen(handle, fileName, 0)) {
      http_read(f, var_p);
      int len = var_p->v.p.size;
      buffer = (char *)malloc(len + 1);
      memcpy(buffer, var_p->v.p.ptr, len);
      buffer[len] = '\0';
    } else {
      systemPrint("\nfailed");
    }
    _output->setStatus(NULL);
    dev_fclose(handle);
    v_free(var_p);
    free(var_p);
  }
  return buffer;
}

char *System::readSource(const char *fileName) {
  _activeFile.empty();
  char *buffer = loadResource(fileName);
  if (!buffer) {
    int h = open(fileName, O_BINARY | O_RDONLY, 0644);
    if (h != -1) {
      int len = lseek(h, 0, SEEK_END);
      lseek(h, 0, SEEK_SET);
      buffer = (char *)malloc(len + 1);
      len = read(h, buffer, len);
      buffer[len] = '\0';
      close(h);
      _activeFile = fileName;
      _modifiedTime = getModifiedTime();
    }
  }
  if (buffer != NULL) {
    delete [] _programSrc;
    int len = strlen(buffer);
    _programSrc = new char[len + 1];
    strncpy(_programSrc, buffer, len);
    _programSrc[len] = '\0';
    _srcRendered = false;
    systemPrint("Opened: %s %d bytes\n", fileName, len);
  }
  return buffer;
}

void System::resize() {
  MAExtent screenSize = maGetScrSize();
  logEntered();
  _output->resize(EXTENT_X(screenSize), EXTENT_Y(screenSize));
  if (isRunning()) {
    setDimensions();
    dev_pushkey(SB_PKEY_SIZE_CHG);
  }
}

void System::runMain(const char *mainBasPath) {
  logEntered();

  // activePath provides the program name after termination
  String activePath = mainBasPath;
  _loadPath = mainBasPath;
  _mainBas = true;
  strcpy(opt_command, "welcome");

  bool started = execute(_loadPath);
  if (!started) {
    maAlert("", gsb_last_errmsg, NULL, NULL, NULL);
    _state = kClosingState;
  }

  while (!isClosing() && started) {
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

    bool success = execute(_loadPath);
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
        if (_mainBas) {
          // unexpected error in main.bas
          maAlert("", gsb_last_errmsg, NULL, NULL, NULL);
          _state = kClosingState;
        }
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
  // startupBas must not be _loadPath.c_str()
  logEntered();
  _mainBas = false;

  bool restart = true;
  while (restart) {
    bool success = execute(startupBas);
    if (_state == kActiveState) {
      showCompletion(success);
    }
    // press back to continue
    while (!isBack() && !isClosing() && !isRestart()) {
      getNextEvent();
    }
    restart = isRestart();
  }
}

void System::setBack() {
  if (_userScreenId != -1) {
    // restore user screen
    _output->selectBackScreen(_userScreenId);
    _output->selectFrontScreen(_userScreenId);
    _userScreenId = -1;
  } else {
    // quit app when shell is active
    setExit(_mainBas);
  }
}

void System::setLoadBreak(const char *path) {
  _loadPath = path;
  _state = kBreakState;
  brun_break();
}

void System::setLoadPath(const char *path) {
  _loadPath = path;
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

void System::setDimensions() {
  os_graf_mx = _output->getWidth();
  os_graf_my = _output->getHeight();
  setsysvar_int(SYSVAR_XMAX, os_graf_mx - 1);
  setsysvar_int(SYSVAR_YMAX, os_graf_my - 1);
}

void System::setRunning(bool running) {
  if (running) {
    dev_fgcolor = -DEFAULT_FOREGROUND;
    dev_bgcolor = -DEFAULT_BACKGROUND;
    setDimensions();
    dev_clrkb();

    _output->setAutoflush(!opt_show_page);
    _loadPath.empty();
    _lastEventTime = maGetMilliSecondCount();
    _eventTicks = 0;
    _overruns = 0;
    _userScreenId = -1;
  } else if (!isClosing() && !isRestart() && !isBack()) {
    _state = kActiveState;
    _output->setAutoflush(true);
  }
}

void System::showCompletion(bool success) {
  if (success) {
    _output->setStatus("Done - press back [<-]");
  } else {
    printErrorLine();
    _output->setStatus("Error - see console");
    showSystemScreen(true);
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
  logEntered();

  if (!_menuActive) {
    _menuActive = true;
    char buffer[64];
    if (_systemMenu != NULL) {
      delete [] _systemMenu;
    }

    StringList *items = new StringList();
    _systemMenu = new int[MENU_SIZE];
    int index = 0;
    if (get_focus_edit() != NULL) {
      items->add(new String("Cut"));
      items->add(new String("Copy"));
      items->add(new String("Paste"));
      _systemMenu[index++] = MENU_CUT;
      _systemMenu[index++] = MENU_COPY;
      _systemMenu[index++] = MENU_PASTE;
#if defined(_SDL)
      items->add(new String("Back"));
      _systemMenu[index++] = MENU_BACK;
#else
      items->add(new String("Show keypad"));
      _systemMenu[index++] = MENU_KEYPAD;
      bool controlMode = get_focus_edit()->getControlMode();
      sprintf(buffer, "Control Mode [%s]", (controlMode ? "ON" : "OFF"));
      items->add(new String(buffer));
      _systemMenu[index++] = MENU_CTRL_MODE;
#endif
    } else {
      if (_overruns == 0) {
        items->add(new String("Console"));
        _systemMenu[index++] = MENU_CONSOLE;

        if (!_mainBas && _activeFile.length() > 0) {
          items->add(new String("Edit source"));
          _systemMenu[index++] = MENU_EDIT_SRC;
        } else {
          items->add(new String("View source"));
          _systemMenu[index++] = MENU_SOURCE;
        }
      }
#if defined(_SDL)
      items->add(new String("Back"));
      _systemMenu[index++] = MENU_BACK;
#endif
      items->add(new String("Restart"));
      _systemMenu[index++] = MENU_RESTART;
#if !defined(_SDL)
      items->add(new String("Show keypad"));
      _systemMenu[index++] = MENU_KEYPAD;
#endif
      if (_mainBas) {
        sprintf(buffer, "Font Size %d%%", _fontScale - FONT_SCALE_INTERVAL);
        items->add(new String(buffer));
        sprintf(buffer, "Font Size %d%%", _fontScale + FONT_SCALE_INTERVAL);
        items->add(new String(buffer));
        _systemMenu[index++] = MENU_ZOOM_UP;
        _systemMenu[index++] = MENU_ZOOM_DN;
      }

#if defined(_SDL)
      sprintf(buffer, "Live Update [%s]", (_liveMode ? "ON" : "OFF"));
      items->add(new String(buffer));
      _systemMenu[index++] = MENU_LIVEMODE;
#endif

      sprintf(buffer, "Audio [%s]", (opt_mute_audio ? "OFF" : "ON"));
      items->add(new String(buffer));
      _systemMenu[index++] = MENU_AUDIO;

      items->add(new String("Screenshot"));
      _systemMenu[index++] = MENU_SCREENSHOT;
    }
    optionsBox(items);
    delete items;
    _menuActive = false;
  }
}

void System::printErrorLine() {
  if (_programSrc) {
    int line = 1;
    char *errLine = _programSrc;
    char *ch = _programSrc;
    while (line < gsb_last_line) {
      while (*ch && *ch != '\n') {
        ch++;
      }
      if (*ch) {
        errLine = ++ch;
      }
      line++;
    }
    while (*ch && *ch != '\n') {
      ch++;
    }
    char end;
    if (*ch == '\n') {
      ch++;
      end = *ch;
      *ch = '\0';
    } else {
      end = *ch;
    }
    while (*errLine && (IS_WHITE(*errLine))) {
      errLine++;
    }

    int prevScreen = _output->selectBackScreen(CONSOLE_SCREEN);
    _output->print("\033[4mError line:\033[0m\n");
    _output->print(errLine);
    *ch = end;
    _output->selectBackScreen(prevScreen);
  }
}

void System::printSourceLine(char *text, int line, bool last) {
  char lineMargin[32];
  sprintf(lineMargin, "\033[7m%03d\033[0m ", line);
  _output->print(lineMargin);
  if (line == gsb_last_line && gsb_last_error) {
    _output->print("\033[7m");
    _output->print(text);
    if (last) {
      _output->print("\n");
    }
    _output->print("\033[27;31m  --^\n");
    _output->print(gsb_last_errmsg);
    _output->print("\033[0m\n");
  } else {
    _output->print(text);
  }
}

void System::printSource() {
  if (_programSrc && !_srcRendered) {
    _srcRendered = true;
    _output->clearScreen();
    int line = 1;
    char *ch = _programSrc;
    char *nextLine = _programSrc;
    int errorLine = gsb_last_error ? gsb_last_line : -1;
    int charHeight = _output->getCharHeight();
    int height = _output->getHeight();
    int pageLines = height / charHeight;

    while (*ch) {
      while (*ch && *ch != '\n') {
        ch++;
      }
      if (*ch == '\n') {
        ch++;
        char end = *ch;
        *ch = '\0';
        printSourceLine(nextLine, line, false);
        *ch = end;
        nextLine = ch;
      } else {
        printSourceLine(nextLine, line, true);
      }
      line++;

      if (errorLine != -1 && line == errorLine + pageLines) {
        // avoid scrolling past the error line
        if (*ch) {
          _output->print("... \n");
        }
        break;
      }
    }

    // scroll to the error line
    if (errorLine != -1) {
      int displayLines = _output->getY() / charHeight;
      if (line > displayLines) {
        // printed more than displayed due to scrolling
        errorLine -= (line - displayLines);
      }
      int yScroll = charHeight * (errorLine - (pageLines / 2));
      int maxScroll = ((displayLines * charHeight) - height);
      if (yScroll < 0) {
        yScroll = 0;
      }
      if (yScroll < maxScroll) {
        _output->setScroll(0, yScroll);
      }
    } else {
      _output->setScroll(0, 0);
    }
  }
}

void System::setExit(bool quit) {
  if (!isClosing()) {
    bool running = isRunning();
    _state = quit ? kClosingState : kBackState;
    if (running) {
      brun_break();
    }
  }
}

void System::setRestart() {
  if (isRunning()) {
    brun_break();
  }
  _state = kRestartState;
}

void System::showSystemScreen(bool showSrc) {
  int prevScreenId;
  if (showSrc) {
    prevScreenId = _output->selectBackScreen(SOURCE_SCREEN);
    printSource();
    _output->selectBackScreen(prevScreenId);
    _output->selectFrontScreen(SOURCE_SCREEN);
  } else {
    prevScreenId = _output->selectFrontScreen(CONSOLE_SCREEN);
  }
  if (_userScreenId == -1) {
    _userScreenId = prevScreenId;
  }
}

void System::systemPrint(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof(buf) - 1, format, args);
  va_end(args);
  *p = '\0';

  deviceLog("%s", buf);

  int prevScreen = _output->selectBackScreen(CONSOLE_SCREEN);
  _output->print(buf);
  _output->selectBackScreen(prevScreen);
}

//
// common device implementation
//
void osd_cls(void) {
  logEntered();
  g_system->getOutput()->clearScreen();
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
  return g_system->getOutput()->getPixel(x, y);
}

int osd_getx(void) {
  return g_system->getOutput()->getX();
}

int osd_gety(void) {
  return g_system->getOutput()->getY();
}

void osd_line(int x1, int y1, int x2, int y2) {
  g_system->getOutput()->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (fill) {
    g_system->getOutput()->drawRectFilled(x1, y1, x2, y2);
  } else {
    g_system->getOutput()->drawRect(x1, y1, x2, y2);
  }
}

void osd_refresh(void) {
  if (!g_system->isClosing()) {
    g_system->getOutput()->flush(true);
  }
}

void osd_setcolor(long color) {
  if (!g_system->isClosing()) {
    g_system->getOutput()->setColor(color);
  }
}

void osd_setpenmode(int enable) {
  // touch mode is always active
}

void osd_setpixel(int x, int y) {
  g_system->getOutput()->setPixel(x, y, dev_fgcolor);
}

void osd_settextcolor(long fg, long bg) {
  g_system->getOutput()->setTextColor(fg, bg);
}

void osd_setxy(int x, int y) {
  g_system->getOutput()->setXY(x, y);
}

int osd_textheight(const char *str) {
  return g_system->getOutput()->textHeight();
}

int osd_textwidth(const char *str) {
  MAExtent textSize = maGetTextSize(str);
  return EXTENT_X(textSize);
}

void osd_write(const char *str) {
  if (!g_system->isClosing()) {
    g_system->getOutput()->print(str);
  }
}

void lwrite(const char *str) {
  if (!(str[0] == '\n' && str[1] == '\0') && !g_system->isClosing()) {
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

int maGetMilliSecondCount(void) {
  return dev_get_millisecond_count();
}

