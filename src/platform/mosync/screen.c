// $Id var_hash.c 783 2010-03-21 122127Z zeeb90au $
// This file is part of SmallBASIC
//
// Unsupported functionality due to platform limitations
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010 Chris Warren-Smith. [http//tinyurl.com/ja2ss]

#include "sys.h"
#include "osd.h"
#include "device.h"

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
