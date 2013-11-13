// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "platform/android/jni/runtime.h"
#include "platform/common/maapi.h"
#include "platform/common/utils.h"
#include "platform/common/form_ui.h"
#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"
#include "common/fs_socket_client.h"

//
// form_ui implementation
//
bool form_ui::isRunning() {
  return false;
}

bool form_ui::isBreak() {
  return false;
}

void form_ui::processEvents() {
}

void form_ui::buttonClicked(const char *url) {
}

AnsiWidget *form_ui::getOutput() {
  return NULL;
}

//
// ma event handling
//
int maGetEvent(MAEvent *event) {
  int result;
  return result;
}

void maWait(int timeout) {
}

//
// sbasic implementation
//
void osd_sound(int frq, int dur, int vol, int bgplay) {
}

void osd_clear_sound_queue() {
}

void osd_beep(void) {
}

void osd_cls(void) {
}

int osd_devinit(void) {
  setsysvar_str(SYSVAR_OSNAME, "Android");
  return 1;
}

int osd_devrestore(void) {
  return 0;
}

int osd_events(int wait_flag) {
  int result;
  return result;
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
  // touch mode is always active
}

void osd_setpixel(int x, int y) {
}

void osd_settextcolor(long fg, long bg) {
}

void osd_setxy(int x, int y) {
}

int osd_textheight(const char *str) {
}

int osd_textwidth(const char *str) {
}

void osd_write(const char *str) {
}

void lwrite(const char *str) {
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
  maWait(ms);
}

char *dev_gets(char *dest, int maxSize) {
  return NULL;
}

char *dev_read(const char *fileName) {
  return NULL;
}
