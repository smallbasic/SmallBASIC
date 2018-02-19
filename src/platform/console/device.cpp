// This file is part of SmallBASIC
//
// Copyright(C) 2001-2018 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "common/device.h"
#include "common/extlib.h"
#include "common/osd.h"
#include "common/str.h"

typedef void (*settextcolor_fn)(long fg, long bg);
typedef void (*setpenmode_fn)(int enable);
typedef int  (*getpen_fn)(int code);
typedef void (*cls_fn)();
typedef int  (*getx_fn)();
typedef int  (*gety_fn)();
typedef void (*setxy_fn)(int x, int y);
typedef void (*write_fn)(const char *str);
typedef int  (*events_fn)(int);
typedef void (*setcolor_fn)(long color);
typedef void (*line_fn)(int x1, int y1, int x2, int y2);
typedef void (*ellipse_fn)(int xc, int yc, int xr, int yr, int fill);
typedef void (*arc_fn)(int xc, int yc, double r, double as, double ae, double aspect);
typedef void (*setpixel_fn)(int x, int y);
typedef long (*getpixel_fn)(int x, int y);
typedef void (*rect_fn)(int x1, int y1, int x2, int y2, int fill);
typedef void (*refresh_fn)();
typedef void (*beep_fn)();
typedef void (*sound_fn)(int frq, int ms, int vol, int bgplay);
typedef void (*clear_sound_queue_fn)();
typedef void (*audio_fn)(const char *path);
typedef int  (*textwidth_fn)(const char *str);
typedef int  (*textheight_fn)(const char *str);

static settextcolor_fn p_settextcolor;
static setpenmode_fn p_setpenmode;
static getpen_fn p_getpen;
static cls_fn p_cls;
static getx_fn p_getx;
static gety_fn p_gety;
static setxy_fn p_setxy;
static write_fn p_write;
static events_fn p_events;
static setcolor_fn p_setcolor;
static line_fn p_line;
static ellipse_fn p_ellipse;
static arc_fn p_arc;
static setpixel_fn p_setpixel;
static getpixel_fn p_getpixel;
static rect_fn p_rect;
static refresh_fn p_refresh;
static beep_fn p_beep;
static sound_fn p_sound;
static clear_sound_queue_fn p_clear_sound_queue;
static audio_fn p_audio;
static textwidth_fn p_textwidth;
static textheight_fn p_textheight;

void default_write(const char *str) {
  int len = strlen(str);
  if (len) {
    int i, index = 0, escape = 0;
    char *buffer = (char *)malloc(len + 1);
    for (i = 0; i < len; i++) {
      if (i + 1 < len && str[i] == '\033' && str[i + 1] == '[') {
        escape = 1;
      } else if (escape && str[i] == 'm') {
        escape = 0;
      } else if (!escape) {
        buffer[index++] = str[i];
      }
    }
    if (index) {
      buffer[index] = 0;
      printf("%s", buffer);
    }
    free(buffer);
  }
}

// initialize driver
int osd_devinit() {
  p_settextcolor = (settextcolor_fn)slib_get_func("sblib_settextcolor");
  p_setpenmode = (setpenmode_fn)slib_get_func("sblib_setpenmode");
  p_getpen = (getpen_fn)slib_get_func("sblib_getpen");
  p_cls = (cls_fn)slib_get_func("sblib_cls");
  p_getx = (getx_fn)slib_get_func("sblib_getx");
  p_gety = (gety_fn)slib_get_func("sblib_gety");
  p_setxy = (setxy_fn)slib_get_func("sblib_setxy");
  p_write = (write_fn)slib_get_func("sblib_write");
  p_events = (events_fn)slib_get_func("sblib_events");
  p_setcolor = (setcolor_fn)slib_get_func("sblib_setcolor");
  p_line = (line_fn)slib_get_func("sblib_line");
  p_ellipse = (ellipse_fn)slib_get_func("sblib_ellipse");
  p_arc = (arc_fn)slib_get_func("sblib_arc");
  p_setpixel = (setpixel_fn)slib_get_func("sblib_setpixel");
  p_getpixel = (getpixel_fn)slib_get_func("sblib_getpixel");
  p_rect = (rect_fn)slib_get_func("sblib_rect");
  p_refresh = (refresh_fn)slib_get_func("sblib_refresh");
  p_beep = (beep_fn)slib_get_func("sblib_beep");
  p_sound = (sound_fn)slib_get_func("sblib_sound");
  p_clear_sound_queue = (clear_sound_queue_fn)slib_get_func("sblib_p_clear_sound_queue");
  p_audio = (audio_fn)slib_get_func("sblib_audio");
  p_textwidth = (textwidth_fn)slib_get_func("sblib_textwidth");
  p_textheight = (textheight_fn)slib_get_func("sblib_textheight");

  if (p_write == NULL) {
    p_write = default_write;
  }

  os_color_depth = 1;
  os_graf_mx = 80;
  os_graf_my = 25;
  osd_cls();
  return 1;
}

// close driver
int osd_devrestore() {
  return 1;
}

// set foreground and background color
// a value of -1 means not change that color
void osd_settextcolor(long fg, long bg) {
  if (p_settextcolor) {
    p_settextcolor(fg, bg);
  }
}

// enable or disable PEN/MOUSE driver
void osd_setpenmode(int enable) {
  if (p_setpenmode) {
    p_setpenmode(enable);
  }
}

// return pen/mouse info ('code' is the rq, see doc)
int osd_getpen(int code) {
  int result;
  if (p_getpen) {
    result = p_getpen(code);
  } else {
    result = 0;
  }
  return result;
}

// clear screen
void osd_cls() {
  if (p_cls) {
    p_cls();
  }
}

// returns the current x position (text-mode cursor)
int osd_getx() {
  int result;
  if (p_getx) {
    result = p_getx();
  } else {
    result = 0;
  }
  return result;
}

// returns the current y position (text-mode cursor)
int osd_gety() {
  int result;
  if (p_gety) {
    result = p_gety();
  } else {
    result = 0;
  }
  return result;
}

// set's text-mode (or graphics) cursor position
void osd_setxy(int x, int y) {
  if (p_setxy) {
    p_setxy(x, y);
  }
}

// Basic output - print sans control codes
void osd_write(const char *str) {
  p_write(str);
}

// events loop (called from main, every 50ms)
int osd_events(int wait_flag) {
  int result = 0;
  if (p_events) {
    result = p_events(wait_flag);
  }
  return result;
}

// sets foreground color
void osd_setcolor(long color) {
  if (p_setcolor) {
    p_setcolor(color);
  }
}

// draw a line
void osd_line(int x1, int y1, int x2, int y2) {
  if (p_line) {
    p_line(x1, y1, x2, y2);
  }
}

// draw an ellipse
void osd_ellipse(int xc, int yc, int xr, int yr, int fill) {
  if (p_ellipse) {
    p_ellipse(xc, yc, xr, yr, fill);
  }
}

// draw an arc
void osd_arc(int xc, int yc, double r, double as, double ae, double aspect) {
  if (p_arc) {
    p_arc(xc, yc, r, as, ae, aspect);
  }
}

// draw a pixel
void osd_setpixel(int x, int y) {
  if (p_setpixel) {
    p_setpixel(x, y);
  }
}

// returns pixel's color
long osd_getpixel(int x, int y) {
  long result;
  if (p_getpixel) {
    result = p_getpixel(x, y);
  } else {
    result = 0;
  }
  return result;
}

// draw rectangle (parallelogram)
void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (p_rect) {
    p_rect(x1, y1, x2, y2, fill);
  }
}

// refresh/flush the screen/stdout
void osd_refresh() {
  if (p_refresh) {
    p_refresh();
  }
}

// just a beep
void osd_beep() {
  if (p_beep) {
    p_beep();
  } else {
    printf("\a");
  }
}

// play a sound
void osd_sound(int frq, int ms, int vol, int bgplay) {
  if (p_sound) {
    p_sound(frq, ms, vol, bgplay);
  }
}

// clears sound-queue (stop background sound)
void osd_clear_sound_queue() {
  if (p_clear_sound_queue) {
    p_clear_sound_queue();
  }
}

// play the given audio file
void osd_audio(const char *path) {
  if (p_audio) {
    p_audio(path);
  }
}

// text-width in pixels
int osd_textwidth(const char *str) {
  int result;
  if (p_textwidth) {
    result = p_textwidth(str);
  } else {
    result = strlen(str);
  }
  return result;
}

// text-height in pixels
int osd_textheight(const char *str) {
  int result;
  if (p_textheight) {
    result = p_textheight(str);
  } else {
    result = 1;
  }
  return result;
}

// unused
void dev_log_stack(const char *keyword, int type, int line) {}
