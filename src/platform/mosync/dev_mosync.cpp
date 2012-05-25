// This file is part of SmallBASIC
//
// Copyright(C) 2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <MAUI/Screen.h>
#include <MAUI/Layout.h>
#include <MAUI/EditBox.h>

#include "platform/mosync/controller.h"
#include "platform/mosync/utils.h"

using namespace MAUI;

extern Controller *controller;
clock_t lastEventTime;
dword eventsPerTick;
int penMode;

#define EVT_MAX_BURN_TIME (CLOCKS_PER_SEC / 4)
#define EVT_PAUSE_TIME 5
#define EVT_CHECK_EVERY ((50 * CLOCKS_PER_SEC) / 1000)
#define PEN_OFF   0             // pen mode disabled
#define PEN_ON    2             // pen mode active

// process the event queue
char *dev_read(const char *fileName) {
  //  char buffer[maGetDataSize(TEST_BAS) + 1];
  //  maReadData(TEST_BAS, &buffer, 0, maGetDataSize(TEST_BAS));
  //  buffer[maGetDataSize(TEST_BAS)] = '\0';
  // FOR TESTING:
  char *buf = (char *) tmp_alloc(100);
  strcpy(buf, "? \"Hello world !\ninput n\nprint n\"");
  return buf;
}

void osd_sound(int frq, int dur, int vol, int bgplay) {

}

void osd_clear_sound_queue() {

}

void osd_beep(void) {

}

void osd_cls(void) {
  controller->output->clearScreen();
}

int osd_devinit(void) {
  os_graf_mx = controller->output->getWidth();
  os_graf_my = controller->output->getHeight();

  os_ver = 1;
  os_color = 1;
  os_color_depth = 16;
  setsysvar_str(SYSVAR_OSNAME, "MoSync");

  osd_cls();
  dev_clrkb();
  ui_reset();
  controller->runMode = Controller::run_state;
  return 1;
}

int osd_devrestore(void) {
  ui_reset();
}

int osd_events(int wait_flag) {
  if (!wait_flag) {
    // pause when we have been called too frequently
    clock_t now = clock();
    if (now - lastEventTime <= EVT_CHECK_EVERY) {
      eventsPerTick += (now - lastEventTime);
      if (eventsPerTick >= EVT_MAX_BURN_TIME) {
        eventsPerTick = 0;
        wait_flag = 2;
      }
    }
    lastEventTime = now;
  }

  switch (wait_flag) {
  case 1:
    // wait for an event
    controller->processEvents(-1);
    break;
  case 2:
    // pause
    maWait(EVT_PAUSE_TIME);
    break;
  default:
    // pump messages without pausing
    maWait(1);
    break;
  }

  return controller->isExit() ? -2 : 0;
}

int osd_getpen(int mode) {
  return 0;
}

long osd_getpixel(int x, int y) {
  return controller->output->getPixel(x, y);
}

int osd_getx(void) {
  return controller->output->getX();
}

int osd_gety(void) {
  return controller->output->getY();
}

void osd_line(int x1, int y1, int x2, int y2) {
  controller->output->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (fill) {
    controller->output->drawRectFilled(x1, y1, x2, y2);
  } else {
    controller->output->drawRect(x1, y1, x2, y2);
  }
}

void osd_refresh(void) {
  controller->output->refresh();
}

void osd_setcolor(long color) {
  controller->output->setColor(color);
}

void osd_setpenmode(int enable) {
  penMode = (enable ? PEN_ON : PEN_OFF);
}

void osd_setpixel(int x, int y) {
  controller->output->setPixel(x, y, dev_fgcolor);
}

void osd_settextcolor(long fg, long bg) {
  controller->output->setTextColor(fg, bg);
}

void osd_setxy(int x, int y) {
  controller->output->setXY(x, y);
}

int osd_textheight(const char *str) {
  return EXTENT_Y(maGetTextSize(str));
}

int osd_textwidth(const char *str) {
  return EXTENT_X(maGetTextSize(str));
}

void osd_write(const char *str) {
  controller->output->print(str);
}

C_LINKAGE_BEGIN

int access(const char *path, int amode) {
  return 0;
}

void chmod(const char *s, int mode) {
}

void dev_delay(dword ms) {
  controller->pause(ms);
}

char *dev_gets(char *dest, int maxSize) {
  if (controller->isRun()) {
    wchar_t *buffer = new wchar_t[maxSize + 1];
    if (maTextBox(L"INPUT:", L"", buffer, maxSize, 0) >=0) {
      for (int i = 0; i < maxSize; i++) {
        dest[i] = buffer[i];
      }
      dest[maxSize] = 0;
    } else {
      dest[0] = 0;
    }
    delete [] buffer;
  }
  return dest;
}

C_LINKAGE_END
