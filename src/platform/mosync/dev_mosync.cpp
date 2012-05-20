// This file is part of SmallBASIC
//
// Copyright(C) 2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "platform/mosync/ansiwidget.h"

#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"

extern AnsiWidget *output;

clock_t lastEventTime;
dword eventsPerTick;
int penMode;

#define EVT_MAX_BURN_TIME (CLOCKS_PER_SEC / 4)
#define EVT_PAUSE_TIME 5
#define EVT_CHECK_EVERY ((50 * CLOCKS_PER_SEC) / 1000)
#define PEN_OFF   0             // pen mode disabled
#define PEN_ON    2             // pen mode active

// process the event queue
bool processEvents() {
  bool result = true;
  MAEvent event;
  MAExtent screenSize;

  while (maGetEvent(&event)) {
    switch (event.type) {
    case EVENT_TYPE_KEY_PRESSED:
      switch(event.key) {
      case MAK_FIRE:
      case MAK_5:
        break;
      case MAK_SOFTRIGHT:
      case MAK_0:
      case MAK_BACK:
        result = false;
        break;
      }
      break;

    case EVENT_TYPE_SCREEN_CHANGED:
      screenSize = maGetScrSize();
      output->resize(EXTENT_X(screenSize), EXTENT_Y(screenSize));
      os_graf_mx = output->getWidth();
      os_graf_my = output->getHeight();
      break;
    
    case EVENT_TYPE_POINTER_PRESSED:
      break;

    case EVENT_TYPE_CLOSE:
      result = false;
      break;

    case EVENT_TYPE_FOCUS_LOST:
      break;

    case EVENT_TYPE_FOCUS_GAINED:
      break;

    default:
      break;
    }
  }

  if (!result) {
    ui_reset();
    brun_break();
  }

  return result;
}

extern "C" int access(const char *path, int amode) {
  return 0;
}

extern "C" void chmod(const char *s, int mode) {
}

char *dev_read(const char *fileName) {
  //  char buffer[maGetDataSize(TEST_BAS) + 1];
  //  maReadData(TEST_BAS, &buffer, 0, maGetDataSize(TEST_BAS));
  //  buffer[maGetDataSize(TEST_BAS)] = '\0';

  return 0;
}

int usleep(useconds_t useconds) {
  return 0;
}

void osd_sound(int frq, int dur, int vol, int bgplay) {

}

void osd_clear_sound_queue() {

}

void osd_beep(void) {

}

void osd_cls(void) {
  output->clearScreen();
}

int osd_devinit(void) {
  os_graf_mx = output->getWidth();
  os_graf_my = output->getHeight();

  os_ver = 1;
  os_color = 1;
  os_color_depth = 16;
  setsysvar_str(SYSVAR_OSNAME, "MoSync");

  osd_cls();
  dev_clrkb();
  ui_reset();
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
    maWait(-1);
    processEvents();
    break;
  case 2:
    // pause
    maWait(EVT_PAUSE_TIME);
    break;
  default:
    // pump messages without pausing
    break;
  }
  return 0;
}

int osd_getpen(int mode) {
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
  output->refresh();
}

void osd_setcolor(long color) {
  output->setColor(color);
}

void osd_setpenmode(int enable) {
  penMode = (enable ? PEN_ON : PEN_OFF);
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
  return EXTENT_Y(maGetTextSize(str));
}

int osd_textwidth(const char *str) {
  return EXTENT_X(maGetTextSize(str));
}

void osd_write(const char *str) {
  output->print(str);
}
