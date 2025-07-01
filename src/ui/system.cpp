// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <cerrno>

#include "include/osd.h"
#include "common/sbapp.h"
#include "common/device.h"
#include "common/fs_socket_client.h"
#include "common/keymap.h"
#include "ui/system.h"
#include "ui/inputs.h"
#include "ui/theme.h"

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
#define MENU_SELECT_ALL 10
#define MENU_CTRL_MODE  11
#define MENU_EDITMODE   12
#define MENU_AUDIO      13
#define MENU_SCREENSHOT 14
#define MENU_UNDO       15
#define MENU_REDO       16
#define MENU_SAVE       17
#define MENU_RUN        18
#define MENU_DEBUG      19
#define MENU_OUTPUT     20
#define MENU_HELP       21
#define MENU_SHORTCUT   22
#define MENU_SHARE      23
#define MENU_THEME      24
#define MENU_FIND       25
#define MENU_SIZE       26
#define MENU_COMPLETION_0  (MENU_SIZE + 1)
#define MENU_COMPLETION_1  (MENU_SIZE + 2)
#define MENU_COMPLETION_2  (MENU_SIZE + 3)
#define MENU_COMPLETION_3  (MENU_SIZE + 4)
#define MAX_COMPLETIONS 4
#define MAX_CACHE 8
#define CHANGE_WAIT_SLEEP 1000

#define FONT_SCALE_INTERVAL 10
#define FONT_MIN 80
#define FONT_MAX 200

#define OPTIONS_BOX_WIDTH_EXTRA 1
#define OPTIONS_BOX_BG 0xd2d1d0
#define OPTIONS_BOX_FG 0x3e3f3e

#if defined(_SDL) || defined(_EMCC)
#define MK_MENU(l, a) " " l " " a " "
#else
#define MK_MENU(l, a) " " l
#endif

#define MENU_STR_BACK    MK_MENU("Back",  "^b")
#define MENU_STR_COPY    MK_MENU("Copy",  "^c")
#define MENU_STR_CUT     MK_MENU("Cut",   "^x")
#define MENU_STR_PASTE   MK_MENU("Paste", "^v")
#define MENU_STR_REDO    MK_MENU("Redo",  "^y")
#define MENU_STR_RUN     MK_MENU("Run",   "^r")
#define MENU_STR_SAVE    MK_MENU("Save",  "^s")
#define MENU_STR_UNDO    MK_MENU("Undo",  "^z")
#define MENU_STR_SCREEN  MK_MENU("Screenshot", "^p")
#define MENU_STR_SELECT  MK_MENU("Select All", "^a")
#define MENU_STR_OFF     "OFF"
#define MENU_STR_ON      "ON"
#define MENU_STR_AUDIO   " Audio  [%s] "
#define MENU_STR_EDITOR  " Editor [%s] "
#define MENU_STR_THEME   " Theme  [%s] "
#define MENU_STR_CONSOLE " Console "
#define MENU_STR_CONTROL " Control Mode [%s] "
#define MENU_STR_DEBUG   " Debug "
#define MENU_STR_FONT    " Font Size %d%% "
#define MENU_STR_HELP    " Help "
#define MENU_STR_KEYPAD  " Show Keypad "
#define MENU_STR_OUTPUT  " Show Output "
#define MENU_STR_RESTART " Restart "
#define MENU_STR_SHARE   " Share "
#define MENU_STR_SHORT   " Desktop Shortcut "
#define MENU_STR_SOURCE  " View Source "
#define MENU_STR_FIND    " Find "

System *g_system;

void Cache::add(const char *key, const char *value) {
  if (_size == _count) {
    // overwrite at next index position
    _head[_index]->clear();
    _head[_index]->append(key);
    _head[_index + 1]->clear();
    _head[_index + 1]->append(value);
    _index = (_index + 2) % _size;
  } else {
    Properties::put(key, value);
  }
}

System::System() :
  _cache(MAX_CACHE),
  _output(nullptr),
  _editor(nullptr),
  _systemMenu(nullptr),
  _programSrc(nullptr),
  _state(kInitState),
  _touchX(-1),
  _touchY(-1),
  _touchCurX(-1),
  _touchCurY(-1),
  _initialFontSize(0),
  _fontScale(100),
  _userScreenId(-1),
  _menuX(0),
  _menuY(0),
  _modifiedTime(0),
  _mainBas(false),
  _buttonPressed(false),
  _srcRendered(false),
  _menuActive(false) {
  g_system = this;
}

System::~System() {
  delete [] _systemMenu;
  delete [] _programSrc;
  delete _editor;

  _systemMenu = nullptr;
  _programSrc = nullptr;
  _editor = nullptr;
}

bool System::execute(const char *bas) {
  _stackTrace.removeAll();
  _output->reset();
  reset_image_cache();

  // reset program controlled options
  opt_antialias = 1;
  opt_show_page = 0;

  opt_pref_width = _output->getWidth();
  opt_pref_height = _output->getHeight();
  opt_base = 0;
  opt_usepcre = 0;
  opt_autolocal = 0;

  _state = kRunState;
  setWindowTitle(bas);
  showCursor(kArrow);
  saveWindowRect();

  int result = ::sbasic_main(bas);
  if (isRunning()) {
    _state = kActiveState;
  }

  if (_editor == nullptr) {
    opt_command[0] = '\0';
  }

  if (!_mainBas) {
    onRunCompleted();
  }

  enableCursor(true);
  opt_file_permitted = 1;
  _output->selectScreen(USER_SCREEN1);
  _output->resetFont();
  _output->flush(true);
  _userScreenId = -1;
  return result != 0;
}

void System::formatOptions(StringList *items) {
  int maxLength = 0;
  bool hasControl = false;
  List_each(String *, it, *items) {
    String *str = * it;
    if (str->indexOf('^', 0) != -1) {
      hasControl = true;
    }
    int len = str->length();
    if (len > maxLength) {
      maxLength = len;
    }
  }
  if (hasControl) {
    List_each(String *, it, *items) {
      String *str = * it;
      if (str->indexOf('^', 0) != -1) {
        String command = str->leftOf('^');
        String control = str->rightOf('^');
        int len = maxLength - str->length();
        for (int i = 0; i < len; i++) {
          command.append(' ');
        }
        command.append("C-");
        command.append(control);
        str->clear();
        str->append(command);
      }
    }
  }
}

bool System::fileExists(strlib::String &path) {
  bool result = false;
  if (path.indexOf("://", 1) != -1) {
    result = true;
  } else if (!path.empty()) {
    struct stat st_file{};
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

  FormInput *widget = new FormLineInput(nullptr, nullptr, maxSize, true, x, y, w, h);
  widget->setFocus(true);

  int bg = _output->getBackgroundColor();
  int fg = _output->getColor();
  if (bg != DEFAULT_BACKGROUND || fg != DEFAULT_FOREGROUND) {
    widget->setColor(bg, fg);
  } else {
    widget->setColor(FOCUS_COLOR, DEFAULT_FOREGROUND);
  }
  _output->addInput(widget);
  _output->redraw();
  _state = kModalState;
  maShowVirtualKeyboard();
  showCursor(kIBeam);

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

  maHideVirtualKeyboard();
  showCursor(kArrow);
  _output->removeInput(widget);
  delete widget;
  return dest;
}

uint32_t System::getModifiedTime() const {
  uint32_t result = 0;
  if (!_activeFile.empty()) {
    struct stat st_file{};
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
  _systemMenu = nullptr;

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
    if (get_focus_edit() != nullptr) {
      char *text = get_focus_edit()->copy(menuItem == MENU_CUT);
      if (text) {
        setClipboardText(text);
        free(text);
        _output->redraw();
      }
    }
    break;
  case MENU_PASTE:
    if (get_focus_edit() != nullptr) {
      char *text = getClipboardText();
      get_focus_edit()->paste(text);
      _output->redraw();
      free(text);
    }
    break;
  case MENU_SELECT_ALL:
    if (get_focus_edit() != nullptr) {
      get_focus_edit()->selectAll();
      _output->redraw();
    }
    break;
  case MENU_CTRL_MODE:
    if (get_focus_edit() != nullptr) {
      bool controlMode = get_focus_edit()->getControlMode();
      get_focus_edit()->setControlMode(!controlMode);
    }
    break;
  case MENU_EDITMODE:
    opt_ide = (opt_ide == IDE_NONE ? IDE_INTERNAL : IDE_NONE);
    break;
  case MENU_THEME:
    g_themeId = (g_themeId + 1) % NUM_THEMES;
    setRestart();
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
  case MENU_FIND:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_CTRL('f');
    break;
  case MENU_HELP:
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = SB_KEY_F(1);
    break;
  case MENU_SHORTCUT:
    if (!_activeFile.empty()) {
      addShortcut(_activeFile.c_str());
    }
    break;
  case MENU_SHARE:
    if (!_activeFile.empty()) {
      share(_activeFile.c_str());
    }
    break;
  case MENU_COMPLETION_0:
    completeKeyword(0);
    break;
  case MENU_COMPLETION_1:
    completeKeyword(1);
    break;
  case MENU_COMPLETION_2:
    completeKeyword(2);
    break;
  case MENU_COMPLETION_3:
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
  switch (event.type) {
  case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
    if (_systemMenu != nullptr) {
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
    if (_output->overMenu(_touchX, _touchY)) {
      showMenu();
    } else {
      dev_pushkey(SB_KEY_MK_PUSH);
      _buttonPressed = _output->pointerTouchEvent(event);
      if (_buttonPressed) {
        showCursor(kHand);
      } else {
        showCursor(get_focus_edit() != nullptr ? kIBeam : kHand);
      }
    }
    break;
  case EVENT_TYPE_POINTER_DRAGGED:
    _touchCurX = event.point.x;
    _touchCurY = event.point.y;
    _output->pointerMoveEvent(event);
    if (_output->hasHover() ||
        _output->overMenu(_touchCurX, _touchCurY) ||
        (_touchX != -1 && _touchY != -1)) {
      showCursor(kHand);
    } else if (_output->hasMenu()) {
      showCursor(kArrow);
    } else if (get_focus_edit()) {
      showCursor(kIBeam);
    } else {
      showCursor(kArrow);
    }
    break;
  case EVENT_TYPE_POINTER_RELEASED:
    _touchCurX = event.point.x;
    _touchCurY = event.point.y;
    _touchX = -1;
    _touchY = -1;
    _output->pointerReleaseEvent(event);
    if (!_buttonPressed) {
      showCursor(get_focus_edit() != nullptr ? kIBeam : kArrow);
    }
    _buttonPressed = false;
    break;
  default:
    // no event
    _output->flush(false);
    break;
  }
}

char *System::loadResource(const char *fileName) {
  char *buffer = nullptr;
  if (strstr(fileName, "://") != nullptr) {
    String *cached = _cache.get(fileName);
    if (cached != nullptr) {
      int len = cached->length();
      buffer = (char *)malloc(len + 1);
      memcpy(buffer, cached->c_str(), len);
      buffer[len] = '\0';
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
          uint32_t len = var_p->v.p.length;
          buffer = (char *)malloc(len + 1);
          memcpy(buffer, var_p->v.p.ptr, len);
          buffer[len] = '\0';
          _cache.add(fileName, buffer);
        }
      } else {
        systemPrint("\nfailed to open %s\n", fileName);
      }
      _output->setStatus(nullptr);
      dev_fclose(handle);
      v_free(var_p);
      v_detach(var_p);
      opt_file_permitted = 0;
    }
  }
  if (buffer == nullptr) {
    // remove failed item from history
    strlib::String *old = _history.peek();
    if (old && old->equals(fileName)) {
      delete _history.pop();
    }
  }
  return buffer;
}

bool System::loadSource(const char *fileName) {
  // loads _programSrc
  char *source = readSource(fileName);
  if (source != nullptr) {
    free(source);
    return true;
  }
  return false;
}

void System::logStack(const char *keyword, int type, int line) {
#if defined(_SDL)
  if (_editor != nullptr) {
    if (type == kwPROC || type == kwFUNC) {
      _stackTrace.add(new StackTraceNode(keyword, type, line));
    }
  }
#endif
}

void System::optionsBox(StringList *items) {
  int backScreenId = _output->getScreenId(true);
  int frontScreenId = _output->getScreenId(false);
  int screenHeight = _output->getStatusHeight();
  _output->selectBackScreen(MENU_SCREEN);

  int width = 0;
  int charWidth = _output->getCharWidth();
  List_each(String *, it, *items) {
    char *str = (char *)(* it)->c_str();
    int w = (strlen(str) * charWidth);
    if (w > width) {
      width = w;
    }
  }
  width += (charWidth * OPTIONS_BOX_WIDTH_EXTRA);

  int charHeight = _output->getCharHeight();
  int textHeight = charHeight + (charHeight / 3);
  int height = textHeight * items->size();

  if (!_menuX) {
    _menuX = _output->getWidth() - (width + charWidth * 2);
  }
  if (!_menuY) {
    _menuY = screenHeight - height;
  }

  if (_menuX + width >= _output->getWidth()) {
    _menuX = _output->getWidth() - width;
  }
  if (_menuY + height >= screenHeight) {
    _menuY = screenHeight - height;
  }

  int y = 0;
  int index = 0;
  int selectedIndex = -1;
  int releaseCount = 0;

  _output->insetMenuScreen(_menuX, _menuY, width, height);

  List_each(String *, it, *items) {
    char *str = (char *)(* it)->c_str();
    FormInput *item = new MenuButton(index, selectedIndex, str, 0, y, width, textHeight);
    _output->addInput(item);
    item->setColor(OPTIONS_BOX_BG, OPTIONS_BOX_FG);
    index++;
    y += textHeight;
  }

  _output->redraw();
  showCursor(kArrow);
  while (selectedIndex == -1 && !isClosing()) {
    MAEvent ev = processEvents(true);
    if (ev.type == EVENT_TYPE_KEY_PRESSED) {
      if (ev.key == 27) {
        break;
      } else if (ev.key == 13) {
        selectedIndex = _output->getMenuIndex();
        break;
      } else if (ev.key == SB_KEY_UP) {
        _output->handleMenu(true);
      } else if (ev.key == SB_KEY_DOWN) {
        _output->handleMenu(false);
      }
    }
    if (ev.type == EVENT_TYPE_POINTER_RELEASED) {
      showCursor(kArrow);
      if (++releaseCount == 2) {
        break;
      }
    }
  }

  _output->removeInputs();
  _output->selectBackScreen(backScreenId);
  _output->selectFrontScreen(frontScreenId);
  _menuX = 0;
  _menuY = 0;
  if (selectedIndex != -1) {
    if (_systemMenu == nullptr && isRunning() &&
        !form_ui::optionSelected(selectedIndex)) {
      dev_clrkb();
      dev_pushkey(selectedIndex);
    } else {
      auto *maEvent = new MAEvent();
      maEvent->type = EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED;
      maEvent->optionsBoxButtonIndex = selectedIndex;
      maPushEvent(maEvent);
    }
  } else {
    delete [] _systemMenu;
    _systemMenu = nullptr;
  }

  _output->redraw();
}

char *System::readSource(const char *fileName) {
  _activeFile.clear();
  char *buffer;
  if (!_mainBas && _editor != nullptr && _loadPath.equals(fileName)) {
    buffer = _editor->getTextSelection(true);
  } else {
    buffer = loadResource(fileName);
    if (!buffer) {
      int h = open(fileName, O_BINARY | O_RDONLY);
      if (h != -1) {
        struct stat st{};
        if (fstat(h, &st) == 0) {
          size_t len = st.st_size;
          buffer = (char *)malloc(len + 1);
          len = read(h, buffer, len);
          buffer[len] = '\0';
          _modifiedTime = st.st_mtime;
          char fullPath[PATH_MAX + 1];
          char *path = realpath(fileName, fullPath);
          if (path != nullptr) {
            // set full path for getModifiedTime()
            _activeFile = fullPath;
          } else {
            _activeFile = fileName;
          }
        }
        close(h);
      }
    }
  }
  if (buffer != nullptr) {
    delete [] _programSrc;
    int len = strlen(buffer) + 1;
    _programSrc = new char[len];
    memcpy(_programSrc, buffer, len);
    _programSrc[len - 1] = '\0';
    _srcRendered = false;
    systemPrint("Opened: %s %d bytes\n", fileName, len);
  }
  return buffer;
}

void System::resize() const {
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
  _loadPath = startupBas;

  while (true) {
    if (loadSource(_loadPath)) {
      setupPath(_loadPath);
      editSource(_loadPath, false);
      if (isBack() || isClosing()) {
        break;
      } else {
        do {
          execute(_loadPath);
        } while (isRestart());
      }
    } else {
      FILE *fp = fopen(_loadPath, "w");
      if (fp) {
        fprintf(fp, "rem Welcome to SmallBASIC\n");
        fclose(fp);
      } else {
        alert("Failed to load file", strerror(errno));
        break;
      }
    }
  }
}

void System::runLive(const char *startupBas) {
  logEntered();
  _mainBas = false;

  while (!isBack() && !isClosing()) {
    bool success = execute(startupBas);
    if (isClosing()) {
      break;
    } else if (!success) {
      showSystemScreen(true);
      _output->selectBackScreen(CONSOLE_SCREEN);
      _output->flush(true);
      waitForChange(true);
    } else {
      _state = kActiveState;
      waitForChange(false);
    }
  }
}

void System::runMain(const char *mainBasPath) {
  logEntered();

  // activePath provides the program name after termination
  String activePath = mainBasPath;
  _loadPath = mainBasPath;
  _mainBas = true;
  strlcpy(opt_command, "welcome", sizeof(opt_command));

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
        if (!isEditReady()) {
          setupPath(_loadPath);
        }
      } else {
        _mainBas = true;
        _loadPath = mainBasPath;
        activePath = mainBasPath;
      }
    }

    if (!_mainBas && isEditReady() && loadSource(_loadPath)) {
      editSource(_loadPath, true);
      if (isBack()) {
        _loadPath.clear();
        _state = kActiveState;
        continue;
      } else if (isClosing()) {
        break;
      }
    }

    bool success = execute(_loadPath);
    bool networkFile = isNetworkLoad();
    if (!isBack() && !isClosing() &&
        (success || networkFile || !isEditEnabled())) {
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
          _loadPath.clear();
          _state = kActiveState;
        }
      }
      if (!_mainBas && !networkFile) {
        waitForBack();
      }
    }
  }
}

void System::runOnce(const char *startupBas, bool runWait) {
  // startupBas must not be _loadPath.c_str()
  logEntered();
  _mainBas = false;

  bool restart = true;
  while (restart) {
    bool success = execute(startupBas);
    if (_state == kActiveState) {
      showCompletion(success);
    }
    if (runWait) {
      waitForBack();
    }
    restart = isRestart();
  }
}

void System::saveFile(TextEditInput *edit, strlib::String &path) {
  if (!edit->save(path)) {
    systemPrint("\nfailed to save: %s. error: %s\n", path.c_str(), strerror(errno));
    alert(strerror(errno), "Failed to save file");
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
      delete old;
      if (_history.peek() != nullptr) {
        _loadPath.clear();
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
    size_t len = strlen(path);
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

void System::setupPath(String &loadPath) {
  const char *filename = loadPath;
  if (strstr(filename, "://") == nullptr) {
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
        struct stat st_file{};
        if (stat(loadPath.c_str(), &st_file) < 0) {
          // reset relative path back to full path
          getcwd(path, FILENAME_MAX);
          strlcat(path, filename + len, sizeof(path));
          loadPath = path;
        }
      }
    }
  }
}

void System::setDimensions() const {
  dev_resize(_output->getWidth(), _output->getHeight());
}

void System::setRunning(bool running) {
  if (running) {
    dev_fgcolor = -DEFAULT_FOREGROUND;
    dev_bgcolor = -DEFAULT_BACKGROUND;
    setDimensions();
    dev_clrkb();
    _output->setAutoflush(!opt_show_page);
    if (_mainBas || isNetworkLoad() || !isEditEnabled()) {
      _loadPath.clear();
    }
    _userScreenId = -1;
  } else {
    osd_clear_sound_queue();
    if (!isClosing() && !isRestart() && !isBack()) {
      _state = kActiveState;
      _output->setAutoflush(true);
    }
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
    delete [] _systemMenu;
    auto *items = new StringList();
    int completions = 0;

    if (get_focus_edit() && isEditing()) {
      completions = get_focus_edit()->getCompletions(items, MAX_COMPLETIONS);
    }

    _systemMenu = new int[MENU_SIZE + completions];

    int index = 0;
    if (get_focus_edit() != nullptr) {
      if (isEditing()) {
        items->add(new String(MENU_STR_UNDO));
        items->add(new String(MENU_STR_REDO));
        items->add(new String(MENU_STR_CUT));
        items->add(new String(MENU_STR_COPY));
        items->add(new String(MENU_STR_PASTE));
        items->add(new String(MENU_STR_SELECT));
        items->add(new String(MENU_STR_SAVE));
        items->add(new String(MENU_STR_RUN));
#if defined(_SDL)
        items->add(new String(MENU_STR_DEBUG));
        items->add(new String(MENU_STR_OUTPUT));
#elif defined(_ANDROID)
        items->add(new String(MENU_STR_FIND));
#endif
        items->add(new String(MENU_STR_HELP));
        for (int i = 0; i < completions; i++) {
          _systemMenu[index++] = MENU_COMPLETION_0 + i;
        }
        _systemMenu[index++] = MENU_UNDO;
        _systemMenu[index++] = MENU_REDO;
        _systemMenu[index++] = MENU_CUT;
        _systemMenu[index++] = MENU_COPY;
        _systemMenu[index++] = MENU_PASTE;
        _systemMenu[index++] = MENU_SELECT_ALL;
        _systemMenu[index++] = MENU_SAVE;
        _systemMenu[index++] = MENU_RUN;
#if defined(_SDL)
        _systemMenu[index++] = MENU_DEBUG;
        _systemMenu[index++] = MENU_OUTPUT;
#elif defined(_ANDROID)
        _systemMenu[index++] = MENU_FIND;
#endif
        _systemMenu[index++] = MENU_HELP;
      } else if (isRunning()) {
        items->add(new String(MENU_STR_CUT));
        items->add(new String(MENU_STR_COPY));
        items->add(new String(MENU_STR_PASTE));
        items->add(new String(MENU_STR_SELECT));
        _systemMenu[index++] = MENU_CUT;
        _systemMenu[index++] = MENU_COPY;
        _systemMenu[index++] = MENU_PASTE;
        _systemMenu[index++] = MENU_SELECT_ALL;
      }
#if defined(_SDL) || defined(_FLTK) || defined(_EMCC)
      items->add(new String(MENU_STR_BACK));
      _systemMenu[index++] = MENU_BACK;
#else
      if (!isEditing()) {
        items->add(new String(MENU_STR_KEYPAD));
        _systemMenu[index++] = MENU_KEYPAD;
        bool controlMode = get_focus_edit()->getControlMode();
        sprintf(buffer, MENU_STR_CONTROL, (controlMode ? MENU_STR_ON : MENU_STR_OFF));
        items->add(new String(buffer));
        _systemMenu[index++] = MENU_CTRL_MODE;
      }
#endif
    } else {
      items->add(new String(MENU_STR_CONSOLE));
      items->add(new String(MENU_STR_SOURCE));
      _systemMenu[index++] = MENU_CONSOLE;
      _systemMenu[index++] = MENU_SOURCE;
#if !defined(_FLTK)
      if (!isEditing()) {
        items->add(new String(MENU_STR_RESTART));
        _systemMenu[index++] = MENU_RESTART;
      }
#endif
#if !defined(_SDL) && !defined(_FLTK) && !defined(_EMCC)
      items->add(new String(MENU_STR_KEYPAD));
      _systemMenu[index++] = MENU_KEYPAD;
#endif
      if (_mainBas) {
#if !defined(_EMCC)
        sprintf(buffer, MENU_STR_FONT, _fontScale - FONT_SCALE_INTERVAL);
        items->add(new String(buffer));
        sprintf(buffer, MENU_STR_FONT, _fontScale + FONT_SCALE_INTERVAL);
        items->add(new String(buffer));
        _systemMenu[index++] = MENU_ZOOM_UP;
        _systemMenu[index++] = MENU_ZOOM_DN;
#endif
#if !defined(_ANDROID_LIBRARY)
        sprintf(buffer, MENU_STR_EDITOR, opt_ide == IDE_NONE ? MENU_STR_OFF : MENU_STR_ON);
        items->add(new String(buffer));
        _systemMenu[index++] = MENU_EDITMODE;
#endif
        sprintf(buffer, MENU_STR_THEME, themeName());
        items->add(new String(buffer));
        _systemMenu[index++] = MENU_THEME;
      }
      sprintf(buffer, MENU_STR_AUDIO, (opt_mute_audio ? MENU_STR_OFF : MENU_STR_ON));
      items->add(new String(buffer));
      _systemMenu[index++] = MENU_AUDIO;
#if !defined(_SDL) && !defined(_FLTK) && !defined(_EMCC)
      if (!_mainBas && !_activeFile.empty()) {
        items->add(new String(MENU_STR_SHORT));
        items->add(new String(MENU_STR_SHARE));
        _systemMenu[index++] = MENU_SHORTCUT;
        _systemMenu[index++] = MENU_SHARE;
      }
#endif
#if !defined(_EMCC)
      items->add(new String(MENU_STR_SCREEN));
      _systemMenu[index++] = MENU_SCREENSHOT;
#endif
#if defined(_SDL) || defined(_FLTK) || defined(_EMCC)
      items->add(new String(MENU_STR_BACK));
      _systemMenu[index++] = MENU_BACK;
#endif
    }

    formatOptions(items);
    optionsBox(items);
    delete items;
    _menuActive = false;
  }
}

void System::showSystemScreen(bool showSrc) {
  int prevScreenId;
  if (showSrc) {
    prevScreenId = _output->getScreenId(true);
    _output->selectBackScreen(SOURCE_SCREEN);
    printSource();
    _output->selectBackScreen(prevScreenId);
    _output->selectFrontScreen(SOURCE_SCREEN);
  } else {
    prevScreenId = _output->getScreenId(false);
    _output->selectFrontScreen(CONSOLE_SCREEN);
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

void System::waitForChange(bool error) {
  while (!isBack() && !isClosing()) {
    processEvents(0);
    if (error && _userScreenId == -1) {
      // back button presses while error displayed
      setExit(true);
      break;
    }
    if (_modifiedTime != getModifiedTime()) {
      break;
    }
    dev_delay(CHANGE_WAIT_SLEEP);
  }
}

void System::printErrorLine() const {
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

    int prevScreenId = _output->getScreenId(true);
    _output->selectBackScreen(CONSOLE_SCREEN);
    _output->print("\033[4mError line:\033[0m\n");
    _output->print(errLine);
    *ch = end;
    _output->selectBackScreen(prevScreenId);
  }
}

void System::printSourceLine(char *text, int line, bool last) const {
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
    _state = (quit && _editor == nullptr) ? kClosingState : kBackState;
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

void System::systemLog(const char *buf) const {
  deviceLog("%s", buf);
  int prevScreenId = _output->getScreenId(true);
  _output->selectBackScreen(CONSOLE_SCREEN);
  _output->print(buf);
  _output->selectBackScreen(prevScreenId);
}

void System::systemPrint(const char *format, ...) const {
  va_list args;

  va_start(args, format);
  unsigned size = format ? vsnprintf(nullptr, 0, format, args) : 0;
  va_end(args);

  if (size) {
    char *buf = (char *)malloc(size + 1);
    buf[0] = '\0';
    va_start(args, format);
    vsnprintf(buf, size + 1, format, args);
    va_end(args);
    buf[size] = '\0';
    systemLog(buf);
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

void osd_arc(int xc, int yc, double r, double start, double end, double aspect) {
  g_system->getOutput()->drawArc(xc, yc, r, start, end, aspect);
}

void osd_ellipse(int xc, int yc, int xr, int yr, int fill) {
  g_system->getOutput()->drawEllipse(xc, yc, xr, yr, fill);
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
  g_system->enableCursor(enable);
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

#if !defined(_FLTK)
void osd_write(const char *str) {
  if (!g_system->isClosing()) {
    g_system->getOutput()->print(str);
  }
}
#endif

void lwrite(const char *str) {
  if (!(str[0] == '\n' && str[1] == '\0') && !g_system->isClosing()) {
    g_system->systemLog(str);
  }
}

void dev_delay(uint32_t ms) {
  g_system->getOutput()->flush(true);
  maWait(ms);
}

char *dev_gets(char *dest, int maxSize) {
  return g_system->getText(dest, maxSize);
}

char *dev_read(const char *fileName) {
  return g_system->readSource(fileName);
}

void dev_log_stack(const char *keyword, int type, int line) {
  return g_system->logStack(keyword, type, line);
}

int maGetMilliSecondCount() {
  return dev_get_millisecond_count();
}
