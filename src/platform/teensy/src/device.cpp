// This file is part of SmallBASIC
//
// Copyright(C) 2001-2018 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "include/osd.h"
#include "common/device.h"
#include "common/smbas.h"

//
// close driver
//
int osd_devrestore() {
  return 1;
}

//
// set foreground and background color
// a value of -1 means not change that color
//
void osd_settextcolor(long fg, long bg) {
}

//
// enable or disable PEN/MOUSE driver
//
void osd_setpenmode(int enable) {
}

//
// return pen/mouse info ('code' is the rq, see doc)
//
int osd_getpen(int code) {
  return 0;
}

//
// clear screen
//
void osd_cls() {
}

//
// returns the current x position (text-mode cursor)
//
int osd_getx() {
  return 0;
}

//
// returns the current y position (text-mode cursor)
//
int osd_gety() {
  return 0;
}

//
// set's text-mode (or graphics) cursor position
//
void osd_setxy(int x, int y) {
}

//
// Basic output - print sans control codes
//
void osd_write(const char *str) {

}

//
// events loop (called from main, every 50ms)
//
int osd_events(int wait_flag) {
  int result = 0;
  return result;
}

//
// sets foreground color
//
void osd_setcolor(long color) {
}

//
// draw a line
//
void osd_line(int x1, int y1, int x2, int y2) {
}

//
// draw an ellipse
//
void osd_ellipse(int xc, int yc, int xr, int yr, int fill) {
}

//
// draw an arc
//
void osd_arc(int xc, int yc, double r, double as, double ae, double aspect) {
}

//
// draw a pixel
//
void osd_setpixel(int x, int y) {
}

//
// returns pixel's color
//
long osd_getpixel(int x, int y) {
  long result;
  result = 0;
  return result;
}

//
// draw rectangle (parallelogram)
//
void osd_rect(int x1, int y1, int x2, int y2, int fill) {
}

//
// refresh/flush the screen/stdout
//
void osd_refresh() {
}

//
// just a beep
//
void osd_beep() {
}

//
// play a sound
//
void osd_sound(int frq, int ms, int vol, int bgplay) {
}

//
// clears sound-queue (stop background sound)
//
void osd_clear_sound_queue() {
}

//
// play the given audio file
//
void osd_audio(const char *path) {
}

//
// text-width in pixels
//
int osd_textwidth(const char *str) {
  int result;
  result = strlen(str);
  return result;
}

//
// text-height in pixels
//
int osd_textheight(const char *str) {
  int result;
  result = 1;
  return result;
}

//
// delay while pumping events
//
void dev_delay(uint32_t timeout) {
}

//
// unused
//
void dev_log_stack(const char *keyword, int type, int line) {}
void v_create_form(var_p_t var) {}
void v_create_window(var_p_t var) {}
void dev_show_page() {}
