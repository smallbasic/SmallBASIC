// $Id$
// This file is part of SmallBASIC
//
// Non-graphics driver, redirects everything to terminal
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2001-02-13 Nicholas Christopoulos

#include "device.h"
#include "osd.h"
#include "str.h"
#include "dev_term.h"

//      initialize driver
int osd_devinit()
{
  os_color = 0;                 // color support = false
  os_color_depth = 1;           // bits per pixel = monochrome
  os_graf_mx = 80;              // screen width in "pixels" (characters =
  // os_graf_mx / textwidth("Q"))
  os_graf_my = 25;              // screen height in "pixels" (characters =
  // os_graf_my / textheight("Q"))

  setsysvar_str(SYSVAR_OSNAME, "ANY/NULL");
  osd_cls();

  return 1;
}

//      close driver
int osd_devrestore()
{
  return 1;
}

// set foreground and background color
// a value of -1 means not change that color
void osd_settextcolor(long fg, long bg)
{
}

//      enable or disable PEN/MOUSE driver
void osd_setpenmode(int enable)
{
//      mouse_show_cursor(enable);
}

//      return pen/mouse info ('code' is the rq, see doc)
int osd_getpen(int code)
{
  return 0;
}

//      clear screen
void osd_cls()
{
}

//      returns the current x position (text-mode cursor)
int osd_getx()
{
  return 0;
}

//      returns the current y position (text-mode cursor)
int osd_gety()
{
  return 0;
}

//      set's text-mode (or graphics) cursor position
void osd_setxy(int x, int y)
{
}

/**
 * Basic output - print sans control codes
 */
void osd_write(const char *str)
{
  int len = strlen(str);
  if (len) {
    int i, index = 0, escape = 0;
    char* buffer = (char*)malloc(len);
    for (i = 0; i < len; i++) {
      if (i + 1 < len && str[i] == '\033' && str[i + 1] == '[') {
        escape = 1;
      }
      else if (escape && str[i] == 'm') {
        escape = 0;
      }
      else if (!escape) {
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

// events loop (called from main, every 50ms)
int osd_events(int wait_flag)
{
  int evc = 0;

  evc = term_events();          // keyboard events
  return evc;
}

///////////////////////////////////////////////////////////////

// sets foreground color
void osd_setcolor(long color)
{
}

// draw a line
void osd_line(int x1, int y1, int x2, int y2)
{
}

// draw a pixel
void osd_setpixel(int x, int y)
{
}

// returns pixel's color 
long osd_getpixel(int x, int y)
{
  return 0;
}

// draw rectangle (parallelogram)
void osd_rect(int x1, int y1, int x2, int y2, int fill)
{
}

//      automagically called by main every 50ms, to refresh/flush the screen/stdout
void osd_refresh()
{
}

// just a beep
void osd_beep()
{
  printf("\a");
}

//      play a sound
//              frq is the freq, 
//              ms is the duration in milliseconds, 
//              vol is the volume (0-100)
//              bgplay is a flag (zero,non-zero) for play now or in background (add to queue)
void osd_sound(int frq, int ms, int vol, int bgplay)
{
}

//      clears sound-queue (stop background sound)
void osd_clear_sound_queue()
{
}

//      text-width in pixels
int osd_textwidth(const char *str)
{
  return strlen(str);
}

//      text-height in pixels
int osd_textheight(const char *str)
{
  // TODO: count \n
  return 1;
}
