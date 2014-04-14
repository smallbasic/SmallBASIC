// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#if defined(WIN32)
#include <fltk/win32.h>
#include <wingdi.h>
extern HDC fl_bitmap_dc;
#else
#include <fltk/x.h>
#endif

int x_get_pixel(int x, int y) {
  int result = 0;

#if defined(WIN32)
  result = -int(::GetPixel(fl_bitmap_dc, x, y));
#else
  XImage *image = XGetImage(fltk::xdisplay, fltk::xwindow, x, y, 1, 1, AllPlanes, ZPixmap);
  if (image) {
    int color = XGetPixel(image, 0, 0);
    XDestroyImage(image);
    result = -color;
  }
#endif
  return result;
}

//--Debug support---------------------------------------------------------------

#if defined(WIN32)
#include <windows.h>
#endif
// see http://download.sysinternals.com/Files/DebugView.zip
// for the free DebugView program
// an alternative debug method is to use insight.exe which
// is included with cygwin.

extern "C" void trace(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof buf - 1, format, args);
  va_end(args);

  while (p > buf && isspace(p[-1])) {
    *--p = '\0';
  }

  *p++ = '\r';
  *p++ = '\n';
  *p = '\0';
#if defined(WIN32)
  OutputDebugString(buf);
#else
  fprintf(stderr, buf, 0);
#endif
}

