// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "platform/common/maapi.h"
#include "platform/common/utils.h"
#include "platform/mosync/form_ui.h"
#include "platform/tizen/controller.h"

Controller *controller;

int SBasicMain() {
  controller = new Controller();
  controller->construct();
 
  bool mainBas = true;
  sbasic_main("main.bas?welcome");

  while (!controller->isExit()) {
    if (controller->isBack()) {
      if (mainBas) {
        controller->setExit(!set_parent_path());
      }
      if (!controller->isExit()) {
        mainBas = true;
        opt_command[0] = '\0';
        sbasic_main("main.bas");
      }
    } else if (controller->getLoadPath() != NULL) {
      mainBas = (strncmp(controller->getLoadPath(), "main.bas", 8) == 0);
      if (!mainBas) {
        set_path(controller->getLoadPath());
      }
      bool success = sbasic_main(controller->getLoadPath());
      if (!controller->isBack()) {
        if (!mainBas) {
          // display an indication the program has completed
          controller->showCompletion(success);
        }
        if (!success) {
          // highlight the error
          controller->showError();
        }
      }
    } else {
      controller->setRunning(false);
      controller->processEvents(-1, -1);
    }
  }

  delete controller;
  return 0;
}

#define ACCESS_EXIST 0
#define ACCESS_WRITE 2
#define ACCESS_READ  4

bool form_ui::isRunning() { 
  return controller->isRunning(); 
}

bool form_ui::isBreak() { 
  return controller->isBreak(); 
}

void form_ui::processEvents() { 
  controller->processEvents(EVENT_WAIT_INFINITE, EVENT_TYPE_EXIT_ANY); 
}

void form_ui::buttonClicked(const char *url) { 
  controller->buttonClicked(url); 
}

AnsiWidget *form_ui::getOutput() {
  return controller->_output;
}

// workaround for android/mosync bug where outer spaces are not counted
int get_text_width(char *s) {
  int result = 0;
  if (s && s[0]) {
    int e = strlen(s) - 1;
    char c1 = s[0];
    char c2 = s[e];
    if (c1 == ' ') {
      s[0] = '_';
    }
    if (c2 == ' ') {
      s[e] = '_';
    }
    result = EXTENT_X(maGetTextSize(s));
    s[0] = c1;
    s[e] = c2;
  }
  return result;
}

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

void osd_sound(int frq, int dur, int vol, int bgplay) {

}

void osd_clear_sound_queue() {

}

void osd_beep(void) {
  controller->_output->beep();
}

void osd_cls(void) {
  logEntered();
  ui_reset();
  controller->_output->clearScreen();
}

int osd_devinit(void) {
  logEntered();
  controller->setRunning(true);
  return 1;
}

int osd_devrestore(void) {
  ui_reset();
  return 0;
}

int osd_events(int wait_flag) {
  return controller->handleEvents(wait_flag);
}

int osd_getpen(int mode) {
  return controller->getPen(mode);
}

long osd_getpixel(int x, int y) {
  return controller->_output->getPixel(x, y);
}

int osd_getx(void) {
  return controller->_output->getX();
}

int osd_gety(void) {
  return controller->_output->getY();
}

void osd_line(int x1, int y1, int x2, int y2) {
  controller->_output->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (fill) {
    controller->_output->drawRectFilled(x1, y1, x2, y2);
  } else {
    controller->_output->drawRect(x1, y1, x2, y2);
  }
}

void osd_refresh(void) {
  controller->_output->flush(true);
}

void osd_setcolor(long color) {
  controller->_output->setColor(color);
}

void osd_setpenmode(int enable) {
  // touch mode is always active
}

void osd_setpixel(int x, int y) {
  controller->_output->setPixel(x, y, dev_fgcolor);
}

void osd_settextcolor(long fg, long bg) {
  controller->_output->setTextColor(fg, bg);
}

void osd_setxy(int x, int y) {
  controller->_output->setXY(x, y);
}

int osd_textheight(const char *str) {
  return controller->_output->textHeight();
}

int osd_textwidth(const char *str) {
  return get_text_width((char*) str);
}

void osd_write(const char *str) {
  logEntered();
  controller->_output->print(str);
}

char *dev_read(const char *fileName) {
  return controller->readSource(fileName);
}

void lwrite(const char *str) {
  controller->logPrint("%s", str);
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
  controller->processEvents(ms, EVENT_TYPE_EXIT_ANY);
}

char *dev_gets(char *dest, int maxSize) {
  return controller->getText(dest, maxSize);
}
