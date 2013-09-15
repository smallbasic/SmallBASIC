// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "platform/tizen/runtime.h"
#include "platform/common/maapi.h"
#include "platform/common/utils.h"
#include "platform/mosync/form_ui.h"

#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"

#define ACCESS_EXIST 0
#define ACCESS_WRITE 2
#define ACCESS_READ  4

// the runtime thread owns the ansiwidget
// ansiwidget uses maXXX apis which are handled in the main/UI thread

AnsiWidget *output;

Runtime::Runtime():
  _eventQueueLock(NULL) {
}

Runtime::~Runtime() {
  delete output;
	delete _eventQueueLock;
}

result Runtime::Construct(int w, int h) {
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
  _runMode = init_state;
  output = new AnsiWidget(this, w, h);
  output->construct();
  output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  //output->setFontSize(_defsize);

  _eventQueueLock = new Mutex();
  _eventQueueLock->Create();

  return E_SUCCESS;
}

Tizen::Base::Object *Runtime::Run() {
  bool mainBas = true;
  sbasic_main("main.bas?welcome");

  while (isExit()) {
    if (isBack()) {
      if (mainBas) {
        setExit(!set_parent_path());
      }
      if (!isExit()) {
        mainBas = true;
        opt_command[0] = '\0';
        sbasic_main("main.bas");
      }
    } else if (getLoadPath() != NULL) {
      mainBas = (strncmp(getLoadPath(), "main.bas", 8) == 0);
      if (!mainBas) {
        set_path(getLoadPath());
      }
      bool success = sbasic_main(getLoadPath());
      if (!isBack()) {
        if (!mainBas) {
          // display an indication the program has completed
          //showCompletion(success);
        }
        if (!success) {
          // highlight the error
          //showError();
        }
      }
    } else {
      //setRunning(false);
      //processEvents(-1, -1);
    }
  }

  //_eventQueueLock->Acquire();
  //_eventQueueLock->Release();

  return 0;
}

void Runtime::setExit(bool quit) {
}

void Runtime::showError() {
}

void Runtime::showCompletion(bool success) {
}

const char *Runtime::getLoadPath() {
  return !_loadPath.length() ? NULL : _loadPath.c_str();
}

void Runtime::flush(bool force) {
  output->flush(force);
}

void Runtime::reset() {
  output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  //output->setFontSize(_defsize);
  output->reset();
}

void Runtime::buttonClicked(const char *action) {
}

//int DisplayWidget::handle(int e) {
  //  MAEvent event;
  /*
  switch (e) {
  case SHOW:
    if (!_screen) {
      _screen = new CanvasWidget(_defsize);
      _screen->create(w(), h());
      drawTarget = _screen;
    }
    if (!_ansiWidget) {
      _ansiWidget = new AnsiWidget(this, w(), h());
      _ansiWidget->construct();
      _ansiWidget->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
      _ansiWidget->setFontSize(_defsize);
    }
    break;

  case FOCUS:
    return 1;

  case PUSH:
    event.point.x = fltk::event_x();
    event.point.y = fltk::event_y();
    mouseActive = _ansiWidget->pointerTouchEvent(event);
    return mouseActive;

  case DRAG:
  case MOVE:
    event.point.x = fltk::event_x();
    event.point.y = fltk::event_y();
    if (mouseActive && _ansiWidget->pointerMoveEvent(event)) {
      Widget::cursor(fltk::CURSOR_HAND);
      return 1;
    }
    break;

  case RELEASE:
    if (mouseActive) {
      mouseActive = false;
      Widget::cursor(fltk::CURSOR_DEFAULT);
      event.point.x = fltk::event_x();
      event.point.y = fltk::event_y();
      _ansiWidget->pointerReleaseEvent(event);
    }
    break;
  }

  return Widget::handle(e);
  */
//  return 0;
//}

//
// form_ui implementation
//
bool form_ui::isRunning() { 
  return isRunning(); 
}

bool form_ui::isBreak() { 
  return isBreak(); 
}

void form_ui::processEvents() { 
  //processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_EXIT_ANY); 
}

void form_ui::buttonClicked(const char *url) { 
  //buttonClicked(url); 
}

//
// Form UI
//
struct Listener : IButtonListener {
  void buttonClicked(const char *action) {
    _action = action;
  }
  String _action;
};

void form_ui::optionsBox(StringList *items) {
//  widget->_ansiWidget->print("\033[ S#6");
//  int y = 0;
//  Listener listener;
//  List_each(String *, it, *items) {
//    char *str = (char *)(* it)->c_str();
//    int w = 0;//fltk::getwidth(str) + 20;
//    IFormWidget *item = widget->_ansiWidget->createButton(str, 2, y, w, 22);
//    item->setListener(&listener);
//    y += 24;
//  }
//  while (form_ui::isRunning() && !listener._action.length()) {
//    form_ui::processEvents();
//  }
//  int index = 0;
//  List_each(String *, it, *items) {
//    char *str = (char *)(* it)->c_str();
//    if (strcmp(str, listener._action.c_str()) == 0) {
//      break;
//    } else {
//      index++;
//    }
//  }
//  widget->_ansiWidget->print("\033[ SE6");
//  widget->_ansiWidget->optionSelected(index);
  //widget->redraw();
}

AnsiWidget *form_ui::getOutput() {
  return output;
}

//
// runtime helpers
//

// set the current working directory to the given path
void set_path(const char *filename) {
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
bool set_parent_path() {
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

//
// sbasic implementation
//

void osd_sound(int frq, int dur, int vol, int bgplay) {

}

void osd_clear_sound_queue() {

}

void osd_beep(void) {
  output->beep();
}

void osd_cls(void) {
  logEntered();
  ui_reset();
  output->clearScreen();
}

int osd_devinit(void) {
  logEntered();
  //setRunning(true);
  return 1;
}

int osd_devrestore(void) {
  ui_reset();
  return 0;
}

int osd_events(int wait_flag) {
  //return handleEvents(wait_flag);
  return 0;
}

int osd_getpen(int mode) {
  //return getPen(mode);
  return 0;
}

long osd_getpixel(int x, int y) {
  return output->getPixel(x, y);
}

int osd_getx(void) {
  return output->getX();
}

int osd_gety(void) {
  return output->getY();
}

void osd_line(int x1, int y1, int x2, int y2) {
  output->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (fill) {
    output->drawRectFilled(x1, y1, x2, y2);
  } else {
    output->drawRect(x1, y1, x2, y2);
  }
}

void osd_refresh(void) {
  output->flush(true);
}

void osd_setcolor(long color) {
  output->setColor(color);
}

void osd_setpenmode(int enable) {
  // touch mode is always active
}

void osd_setpixel(int x, int y) {
  output->setPixel(x, y, dev_fgcolor);
}

void osd_settextcolor(long fg, long bg) {
  output->setTextColor(fg, bg);
}

void osd_setxy(int x, int y) {
  output->setXY(x, y);
}

int osd_textheight(const char *str) {
  return output->textHeight();
}

int osd_textwidth(const char *str) {
  return get_text_width((char*) str);
}

void osd_write(const char *str) {
  logEntered();
  output->print(str);
}

char *dev_read(const char *fileName) {
  //return readSource(fileName);
  return NULL;
}

void lwrite(const char *str) {
  //logPrint("%s", str);
}

void dev_image(int handle, int index,
               int x, int y, int sx, int sy, int w, int h) {
}

int dev_image_width(int handle, int index) {
  return 0;
}

int dev_image_height(int handle, int index) {
  return 0;
}

void dev_delay(dword ms) {
  //processEvents(ms, EVENT_TYPE_EXIT_ANY);
}

char *dev_gets(char *dest, int maxSize) {
  //return getText(dest, maxSize);
  return NULL;
}

extern "C" int dev_clock() {
  return 0;
}
