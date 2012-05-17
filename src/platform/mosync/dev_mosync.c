// This file is part of SmallBASIC
//
// Copyright(C) 2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "common/sys.h"
#include "common/osd.h"
#include "common/device.h"

#include <stdio.h>

int access(const char *path, int amode) {
  return 0;
}

int usleep(useconds_t useconds) {
  return 0;
}

void chmod(const char *s, int mode) {
}

void osd_sound(int frq, int dur, int vol, int bgplay) {

}

void osd_clear_sound_queue() {

}

void osd_beep(void) {

}

void osd_cls(void) {
}

int osd_devinit(void) {
  return 0;
}

int osd_devrestore(void) {
  return 0;
}

int osd_events(int wait_flag) {
  return 0;
}

int osd_getpen(int mode) {
  return 0;
}

long osd_getpixel(int x, int y) {
  return 0;
}

int osd_getx(void) {
  return 0;
}

int osd_gety(void) {
  return 0;
}

void osd_line(int x1, int y1, int x2, int y2) {
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
}

void osd_refresh(void) {
}

void osd_setcolor(long color) {
}

void osd_setpenmode(int enable) {
}

void osd_setpixel(int x, int y) {
}

void osd_settextcolor(long fg, long bg) {
}

void osd_setxy(int x, int y) {
}

int osd_textheight(const char *str) {
  return 0;
}

int osd_textwidth(const char *str) {
  return 0;
}

void osd_write(const char *str) {

}

int term_events() {
  return 0;
}
