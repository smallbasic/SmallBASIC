// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "common/sbapp.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/fs_socket_client.h"
#include "common/keymap.h"
#include "ui/system.h"
#include "ui/inputs.h"

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
#define MENU_EDITMODE   11
#define MENU_AUDIO      12
#define MENU_SCREENSHOT 13
#define MENU_UNDO       14
#define MENU_REDO       15
#define MENU_SAVE       16
#define MENU_RUN        17
#define MENU_DEBUG      18
#define MENU_OUTPUT     19
#define MENU_HELP       20
#define MENU_SHORTCUT   21
#define MENU_SIZE       22
#define MENU_COMPETION_0  (MENU_SIZE + 1)
#define MENU_COMPETION_1  (MENU_SIZE + 2)
#define MENU_COMPETION_2  (MENU_SIZE + 3)
#define MENU_COMPETION_3  (MENU_SIZE + 4)
#define MAX_COMPLETIONS 4
#define MAX_CACHE 8

#define FONT_SCALE_INTERVAL 10
#define FONT_MIN 20
#define FONT_MAX 200

System *g_system;

void Cache::add(const char *key, const char *value) {
  if (_size == _count) {
    // overwrite at next index position
    _head[_index]->empty();
    _head[_index]->append(key);
    _head[_index + 1]->empty();
    _head[_index + 1]->append(value);
    _index = (_index + 2) % _size;
  } else {
    Properties::put(key, value);
  }
}

System::System() :
  _output(NULL),
  _state(kInitState),
  _cache(MAX_CACHE),
  _touchX(-1),
  _touchY(-1),
  _touchCurX(-1),
  _touchCurY(-1),
  _initialFontSize(0),
  _fontScale(100),
  _userScreenId(-1),
  _systemMenu(NULL),
  _mainBas(false),
  _buttonPressed(false),
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
  if (opt_ide == IDE_EXTERNAL && _activeFile.length() > 0 &&
      _modifiedTime != getModifiedTime()) {
    setRestart();
  }
}

bool System::execute(const char *bas) {
  _output->reset();
  reset_image_cache();

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
  showCursor(kArrow);
  int result = ::sbasic_main(bas);
  if (isRunning()) {
    _state = kActiveState;
  }

  opt_command[0] = '\0';
  _output->resetFont();
  _output->flush(true);
  return result;
}

bool System::fileExists(strlib::String &path) {
  bool result = false;
  if (path.indexOf("://", 1) != -1) {
    result = true;
  } else if (path.length() > 0) {
    struct stat st_file;
    result = stat(path.c_str(), &st_file) == 0;
  }
  return result;
}

int System::getPen(int code) {
  _output->flush(true);
  int result = 0;
  if (!isClosing()) {
    switch (code) {
    case 0:
      // UNTIL PEN(0) - wait until click or move
      processEvents(1);
      // fallthru

    case 3:   // returns true if the pen is down (and save curpos)
      result = getPen3();
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
  widget->setFocus(true);
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

void System::handleMenu(MAEvent &event) {
  int menuId = event.optionsBoxButtonIndex;
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
  case MENU_EDITMODE:
#if defined(_SDL)
    opt_ide = (opt_ide == IDE_NONE ? IDE_INTERNAL :
               opt_ide == IDE_INTERNAL ? IDE_EXTERNAL : IDE_NONE);
#else
    opt_ide = (opt_ide == IDE_NONE ? IDE_INTERNAL : IDE_NONE);
#endif
    break;
  case MENU_AUDIO:
    opt_mute_audio = !opt_mute_audio;
    break;
  case MENU_SCREENSHOT:
    ::screen_dump();
    break;
  case MENU_UNDO:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_CTRL('z');
    break;
  case MENU_REDO:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_CTRL('y');
    break;
  case MENU_SAVE:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_CTRL('s');
    break;
  case MENU_RUN:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_F(9);
    break;
  case MENU_DEBUG:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_F(5);
    break;
  case MENU_OUTPUT:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_CTRL('o');
    break;
  case MENU_HELP:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_F(1);
    break;
  case MENU_SHORTCUT:
    if (_activeFile.length() > 0) {
      addShortcut(_activeFile.c_str());
    }
    break;
  case MENU_COMPETION_0:
    completeKeyword(0);
    break;
  case MENU_COMPETION_1:
    completeKeyword(1);
    break;
  case MENU_COMPETION_2:
    completeKeyword(2);
    break;
  case MENU_COMPETION_3:
    completeKeyword(3);
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
      handleMenu(event);
    } else if (isRunning()) {
      if (!form_ui::optionSelected(event.optionsBoxButtonIndex)) {
        dev_clrkb();
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
    showCursor(get_focus_edit() != NULL ? kIBeam : kHand);
    break;
  case EVENT_TYPE_POINTER_DRAGGED:
    _touchCurX = event.point.x;
    _touchCurY = event.point.y;
    hasHover = _output->hasHover();
    _output->pointerMoveEvent(event);
    if (hasHover != _output->hasHover()) {
      showCursor(hasHover ? kArrow : kHand);
    } else if (_output->hasMenu()) {
      showCursor(kArrow);
    }
    break;
  case EVENT_TYPE_POINTER_RELEASED:
    _buttonPressed = false;
    _touchX = _touchY = _touchCurX = _touchCurY = -1;
    _output->pointerReleaseEvent(event);
    showCursor(get_focus_edit() != NULL ? kIBeam : kArrow);
    break;
  default:
    // no event
    _output->flush(false);
    break;
  }
  if (opt_ide == IDE_EXTERNAL) {
    checkModifiedTime();
  }
}

char *System::loadResource(const char *fileName) {
  char *buffer = NULL;
  if (strstr(fileName, "://") != NULL) {
    String *cached = _cache.get(fileName);
    if (cached != NULL) {
      int len = cached->length();
      buffer = (char *)malloc(len + 1);
      memcpy(buffer, cached->c_str(), len);
    } else {
      int handle = 1;
      var_t *var_p = v_new();
      dev_file_t *f = dev_getfileptr(handle);
      _output->setStatus("Loading...");
      _output->redraw();
      if (dev_fopen(handle, fileName, 0)) {
        if (http_read(f, var_p) == 0) {
          systemPrint("\nfailed to read %s\n", fileName);
        } else {
          int len = var_p->v.p.size;
          buffer = (char *)malloc(len + 1);
          memcpy(buffer, var_p->v.p.ptr, len);
          buffer[len] = '\0';
          _cache.add(fileName, buffer);
        }
      } else {
        systemPrint("\nfailed to open %s\n", fileName);
      }
      _output->setStatus(NULL);
      dev_fclose(handle);
      v_free(var_p);
      free(var_p);
    }
  }
  return buffer;
}

bool System::loadSource(const char *fileName) {
  // loads _programSrc
  char *source = readSource(fileName);
  if (source != NULL) {
    free(source);
    return true;
  }
  return false;
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

void System::runEdit(const char *startupBas) {
  logEntered();
  _mainBas = false;
  String loadPath = startupBas;

  while (true) {
    if (loadSource(startupBas)) {
      editSource(loadPath);
      if (isBack() || isClosing()) {
        break;
      } else {
        do {
          execute(startupBas);
        } while (isRestart());
      }
    } else {
      FILE *fp = fopen(startupBas, "w");
      if (fp) {
        fprintf(fp, "rem Welcome to SmallBASIC\n");
        fclose(fp);
      } else {
        alert("Error", "Failed to load file");
        break;
      }
    }
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
    alert("Error", gsb_last_errmsg);
    _state = kClosingState;
  }

  while (!isClosing() && started) {
    if (isRestart()) {
      _loadPath = activePath;
      _state = kActiveState;
    } else {
      if (fileExists(_loadPath)) {
        _mainBas = false;
        activePath = _loadPath;
        setupPath();
      } else {
        _mainBas = true;
        _loadPath = mainBasPath;
        activePath = mainBasPath;
      }
    }

    if (!_mainBas && opt_ide == IDE_INTERNAL && !isRestart() &&
        _loadPath.indexOf("://", 1) == -1 && loadSource(_loadPath)) {
      editSource(_loadPath);
      if (isBack()) {
        _loadPath.empty();
        _state = kActiveState;
        continue;
      } else if (isClosing()) {
        break;
      }
    }

    bool success = execute(_loadPath);
    bool networkFile = (_loadPath.indexOf("://", 1) != -1);
    if (!isBack() && !isClosing() &&
        (opt_ide != IDE_INTERNAL || success || networkFile)) {
      // when editing, only pause here when successful, otherwise the editor shows
      // the error. load the next network file without displaying the previous result
      if (!_mainBas && !networkFile) {
        // display an indication the program has completed
        showCompletion(success);
      }
      if (!success) {
        if (_mainBas) {
          // unexpected error in main.bas
          alert("", gsb_last_errmsg);
          _state = kClosingState;
        } else {
          // don't reload
          _loadPath.empty();
          _state = kActiveState;
        }
      }
      if (!_mainBas && !networkFile) {
        waitForBack();
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
    waitForBack();
    restart = isRestart();
  }
}

void System::saveFile(TextEditInput *edit, strlib::String &path) {
  if (!edit->save(path)) {
    alert("", "Failed to save file");
  } else {
    _modifiedTime = getModifiedTime();
  }
}

void System::setBack() {
  if (_userScreenId != -1) {
    // return (back) from user screen, (view source)
    _output->selectBackScreen(_userScreenId);
    _output->selectFrontScreen(_userScreenId);
    _userScreenId = -1;
  } else {
    // quit app when shell is active
    setExit(_mainBas);

    // follow history when available and not exiting
    if (!_mainBas) {
      // remove the current item
      strlib::String *old = _history.pop();
      if (old) {
        delete old;
      }
      if (_history.peek() != NULL) {
        _loadPath.empty();
        _loadPath.append(_history.peek());
      }
    }
  }
}

void System::setLoadBreak(const char *path) {
  _loadPath = path;
  _history.push(new strlib::String(path));
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

void System::setupPath() {
  const char *filename = _loadPath;
  if (strstr(filename, "://") == NULL) {
    const char *slash = strrchr(filename, '/');
    if (!slash) {
      slash = strrchr(filename, '\\');
    }
    if (slash) {
      int len = slash - filename;
      if (len > 0) {
        // change to the loadPath directory
        char path[FILENAME_MAX + 1];
        strncpy(path, filename, len);
        path[len] = 0;
        chdir(path);
        struct stat st_file;
        if (stat(_loadPath.c_str(), &st_file) < 0) {
          // reset relative path back to full path
          getcwd(path, FILENAME_MAX);
          strcat(path, filename + len);
          _loadPath = path;
        }
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
    if (_mainBas || opt_ide != IDE_INTERNAL ||
        _loadPath.indexOf("://", 1) != -1) {
      _loadPath.empty();
    }
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

void System::showMenu() {
  logEntered();

  if (!_menuActive) {
    _menuActive = true;
    char buffer[64];
    if (_systemMenu != NULL) {
      delete [] _systemMenu;
    }

    StringList *items = new StringList();
    int completions = 0;

    if (get_focus_edit() && isEditing()) {
      completions = get_focus_edit()->getCompletions(items, MAX_COMPLETIONS);
    }

    _systemMenu = new int[MENU_SIZE + completions];

    int index = 0;
    if (get_focus_edit() != NULL) {
      if (isEditing()) {
        items->add(new String("Undo"));
        items->add(new String("Redo"));
        items->add(new String("Cut"));
        items->add(new String("Copy"));
        items->add(new String("Paste"));
        items->add(new String("Save"));
        items->add(new String("Run"));
#if defined(_SDL)
        items->add(new String("Debug"));
        items->add(new String("Show Output"));
#endif
        items->add(new String("Help"));
        for (int i = 0; i < completions; i++) {
          _systemMenu[index++] = MENU_COMPETION_0 + i;
        }
        _systemMenu[index++] = MENU_UNDO;
        _systemMenu[index++] = MENU_REDO;
        _systemMenu[index++] = MENU_CUT;
        _systemMenu[index++] = MENU_COPY;
        _systemMenu[index++] = MENU_PASTE;
        _systemMenu[index++] = MENU_SAVE;
        _systemMenu[index++] = MENU_RUN;
#if defined(_SDL)
        _systemMenu[index++] = MENU_DEBUG;
        _systemMenu[index++] = MENU_OUTPUT;
#endif
        _systemMenu[index++] = MENU_HELP;
      } else if (isRunning()) {
        items->add(new String("Cut"));
        items->add(new String("Copy"));
        items->add(new String("Paste"));
        _systemMenu[index++] = MENU_CUT;
        _systemMenu[index++] = MENU_COPY;
        _systemMenu[index++] = MENU_PASTE;
      }
#if defined(_SDL)
      items->add(new String("Back"));
      _systemMenu[index++] = MENU_BACK;
#else
      items->add(new String("Show keypad"));
      _systemMenu[index++] = MENU_KEYPAD;
      if (!isEditing()) {
        bool controlMode = get_focus_edit()->getControlMode();
        sprintf(buffer, "Control Mode [%s]", (controlMode ? "ON" : "OFF"));
        items->add(new String(buffer));
        _systemMenu[index++] = MENU_CTRL_MODE;
      }
#endif
    } else {
      items->add(new String("Console"));
      items->add(new String("View source"));
      _systemMenu[index++] = MENU_CONSOLE;
      _systemMenu[index++] = MENU_SOURCE;
      if (!isEditing()) {
        items->add(new String("Restart"));
        _systemMenu[index++] = MENU_RESTART;
      }
#if !defined(_SDL)
      items->add(new String("Show keypad"));
      _systemMenu[index++] = MENU_KEYPAD;
#endif
      items->add(new String("Screenshot"));
      _systemMenu[index++] = MENU_SCREENSHOT;
      if (_mainBas) {
        sprintf(buffer, "Font Size %d%%", _fontScale - FONT_SCALE_INTERVAL);
        items->add(new String(buffer));
        sprintf(buffer, "Font Size %d%%", _fontScale + FONT_SCALE_INTERVAL);
        items->add(new String(buffer));
        _systemMenu[index++] = MENU_ZOOM_UP;
        _systemMenu[index++] = MENU_ZOOM_DN;
        sprintf(buffer, "Editor [%s]", (opt_ide == IDE_NONE ? "OFF" :
                                        opt_ide == IDE_INTERNAL ? "ON" : "Live Mode"));
        items->add(new String(buffer));
        _systemMenu[index++] = MENU_EDITMODE;
      }
#if !defined(_SDL)
      if (!_mainBas && _activeFile.length() > 0) {
        items->add(new String("Desktop Shortcut"));
        _systemMenu[index++] = MENU_SHORTCUT;
      }
#endif
      sprintf(buffer, "Audio [%s]", (opt_mute_audio ? "OFF" : "ON"));
      items->add(new String(buffer));
      _systemMenu[index++] = MENU_AUDIO;
#if defined(_SDL)
      items->add(new String("Back"));
      _systemMenu[index++] = MENU_BACK;
#endif
    }
    optionsBox(items);
    delete items;
    _menuActive = false;
  }
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

void System::waitForBack() {
  while (!isBack() && !isClosing() && !isRestart()) {
    MAEvent event = getNextEvent();
    if (event.type == EVENT_TYPE_KEY_PRESSED &&
        event.key == SB_KEY_BACKSPACE) {
      break;
    }
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

void System::systemPrint(const char *format, ...) {
  va_list args;

  va_start(args, format);
  unsigned size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (size) {
    char *buf = (char *)malloc(size + 1);
    va_start(args, format);
    vsnprintf(buf, size + 1, format, args);
    va_end(args);
    buf[size] = '\0';

    deviceLog("%s", buf);

    int prevScreen = _output->selectBackScreen(CONSOLE_SCREEN);
    _output->print(buf);
    _output->selectBackScreen(prevScreen);
    free(buf);
  }
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
  g_system->getOutput()->redraw();

  MARect rc;
  int data[1];
  rc.left = x;
  rc.top = y;
  rc.width = 1;
  rc.height = 1;
  maGetImageData(HANDLE_SCREEN, &data, &rc, 1);
  int result = -(data[0] & 0x00FFFFFF);
  return result;
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
  g_system->getOutput()->flush(true);
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

void create_func(var_p_t form, const char *name, method cb) {
  var_p_t v_func = map_add_var(form, name, 0);
  v_func->type = V_FUNC;
  v_func->v.fn.self = form;
  v_func->v.fn.cb = cb;
}
