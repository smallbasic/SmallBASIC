// This file is part of SmallBASIC
//
// lowlevel device (OS) I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "config.h"

#include "include/osd.h"
#include "common/device.h"
#include "common/smbas.h"
#include "common/messages.h"
#include "common/keymap.h"
#include "common/inet.h"

/**
 * initialize all drivers
 */
int dev_init(int mode, int flags) {
  dev_initfs();
  dev_fgcolor = 0;
  dev_bgcolor = (os_graphics) ? 15 : 0;
  osd_devinit();

  // init the keyboard map
  keymap_init();

  dev_viewport(0, 0, 0, 0);
  dev_window(0, 0, 0, 0);

  if (os_graphics) {
    // dev_fgcolor + dev_bgcolor can be overridden in osd_devinit()
    // otherwise left as default black text on white background
    osd_settextcolor(dev_fgcolor, dev_bgcolor);
    osd_setcolor(dev_fgcolor);
  } else {
    dev_fgcolor = 7;
    dev_bgcolor = 0;
  }

  return 1;
}

/**
 * restore device's mode
 */
int dev_restore() {
  if (os_graphics) {
    osd_refresh();
  }
  dev_closefs();
  if (os_graphics) {
    osd_devrestore();
  }
  net_close();
  return 1;
}

/**
 * CHECK FOR EVENTS
 *
 * wait == 0 check & return
 *    != 0 wait for an event
 *
 * returns 0 for no events in queue
 */
int dev_events(int wait_flag) {
  return osd_events(wait_flag);
}

/**
 * BEEP
 */
void dev_beep() {
  if (!opt_mute_audio) {
    osd_beep();
  }
}

/**
 * plays an OGG or MP3 file
 */
void dev_audio(const char *path) {
  if (!opt_mute_audio) {
    osd_audio(path);
  }
}

/**
 * plays a sound
 */
void dev_sound(int frq, int ms, int vol, int bgplay) {
  if (!opt_mute_audio) {
    osd_sound(frq, ms, vol, bgplay);
  }
}

/**
 * clear background sound queue
 */
void dev_clear_sound_queue() {
  osd_clear_sound_queue();
}

/**
 * printf
 */
void dev_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  unsigned size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (size) {
    char *buf = malloc(size + 1);
    buf[0] = '\0';
    va_start(args, format);
    vsnprintf(buf, size + 1, format, args);
    va_end(args);
    buf[size] = '\0';
    dev_print(buf);
    free(buf);
  }
}

/**
 * In the FLTK build, prints to the LOG window, in other builds
 * prints to the output device as per dev_printf
 */
void log_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  unsigned size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (size) {
    char *buf = malloc(size + 3);
    buf[0] = '\0';
    va_start(args, format);
    vsnprintf(buf, size + 1, format, args);
    va_end(args);

    buf[size] = '\0';
    int i = size - 1;
    while (i >= 0 && isspace(buf[i])) {
      buf[i--] = '\0';
    }
    strcat(buf, "\r\n");
#if defined(IMPL_LOG_WRITE)
    lwrite(buf);
#else
    dev_print(buf);
#endif
    free(buf);
  }
}

#if !defined(_SDL)
void dev_trace_line(int lineNo) {
  dev_printf("<%d>", lineNo);
}
#endif

#ifndef IMPL_LOG_WRITE
void lwrite(const char *buf) {
  fprintf(stderr, "%s\n", buf);
}
#endif

/**
 * Displays a fatal error message
 */
void panic(const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vfprintf(stderr, fmt, argp);
  dev_print("\nFatal error\n");
  va_end(argp);
  exit(EXIT_FAILURE);
}
