// $Id$
// This file is part of SmallBASIC
//
// lowlevel device (OS) I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#define DEVICE_MODULE

#include "sys.h"
#include "str.h"
#include "var.h"
#include "device.h"
#include "osd.h"
#include "smbas.h"
#include "sberr.h"
#include "messages.h"

#define W2X(x)  ( ( (((x)-dev_Wx1)*dev_Vdx)/dev_Wdx ) + dev_Vx1 )
#define W2Y(y)  ( ( (((y)-dev_Wy1)*dev_Vdy)/dev_Wdy ) + dev_Vy1 )
#define W2D2(x,y) { (x) = W2X((x)); (y) = W2Y((y)); }
#define W2D4(x1,y1,x2,y2) { W2D2((x1),(y1)); W2D2((x2),(y2)); }
#define CLIPENCODE(x,y,c) { c = (x < dev_Vx1); c |= ((y < dev_Vy1) << 1); c |= ((x > dev_Vx2) << 2); c |= ((y > dev_Vy2) << 3); }
#define CLIPIN(c)       ((c & 0xF) == 0)

#if defined(_FRANKLIN_EBM)
// /
// / EBM
// /
byte os_graphics = 1;
int os_graf_mx = 200;
int os_graf_my = 240;
dword os_ver = 0x20000;
byte os_color = 1;
dword os_color_depth = 16;
#elif defined(_PalmOS)
// /
// / PalmOS
// /
dword os_ver = 0x30100;
dword os_color_depth = 1;
byte os_color = 0;
#if defined(SONY_CLIE)
byte use_sony_clie = 0;
UInt16 sony_refHR;
int os_graf_mx = 320;
int os_graf_my = 320;
#else
int os_graf_mx = 160;
int os_graf_my = 160;
#endif
byte os_graphics = 1;
#elif defined(_VTOS)
// /
// / VTOS
// /
dword os_ver = OS_VER;
dword os_color_depth = 1;
byte os_color = 0;
byte os_graphics = 1;
int os_graf_mx = 160;
int os_graf_my = 160;
#else
// /
// / Unix/DOS/Windows
// /
dword os_ver = 0x40000;
byte os_color = 1;
dword os_color_depth = 16;
byte os_graphics = 0; // CONSOLE
int os_graf_mx = 80;
int os_graf_my = 25;
#endif

// cache
word os_cclabs1 = 256;
word os_ccpass2 = 256;

// graphics - viewport
int32 dev_Vx1;
int32 dev_Vy1;
int32 dev_Vx2;
int32 dev_Vy2;

int32 dev_Vdx;
int32 dev_Vdy;

// graphics - window world coordinates
int32 dev_Wx1;
int32 dev_Wy1;
int32 dev_Wx2;
int32 dev_Wy2;
int32 dev_Wdx;
int32 dev_Wdy;
long dev_fgcolor = 0;
long dev_bgcolor = 15;

/**
 * initialize all drivers
 */
int dev_init(int mode, int flags) {
#if defined(_UnixOS)
  int verfd;
  //      struct utsname uts;
#endif

#if defined(DRV_SOUND)
  drvsound_ok = drvsound_init();
#endif
#if defined(DRV_MOUSE)
  if (os_graphics)
    drvmouse_ok = drvmouse_init();
#endif
  dev_initfs();
  dev_fgcolor = 0;
#if defined(_PalmOS)
  dev_bgcolor = 15;
#else
  dev_bgcolor = (os_graphics) ? 15 : 0;
#endif

#if USE_TERM_IO
  os_graphics = mode;
  term_init();                  // by default
  if (mode) {
#if defined(_UnixOS)
    char buf[256];

    if (term_israw()) {
      setsysvar_str(SYSVAR_OSNAME, "Unix/RAW");
    }
    else {
      if (getenv("TERM")) {
        strcpy(buf, "Unix/Terminal:");
        strcat(buf, getenv("TERM"));
        setsysvar_str(SYSVAR_OSNAME, buf);
      }
      else {
        setsysvar_str(SYSVAR_OSNAME, "Unix/Stream");
      }
    }
#endif

    if (osd_devinit() == 0)
#if defined(_WinBCB)
      panic("osd_devinit() failed");
#else
    exit(1);
#endif
  }
#else
  osd_devinit();
#endif

  dev_viewport(0, 0, 0, 0);
  dev_window(0, 0, 0, 0);

  if (os_graphics) {
    // dev_fgcolor + dev_bgcolor can be overridden in osd_devinit()
    // otherwise left as default black text on white background
    osd_settextcolor(dev_fgcolor, dev_bgcolor);
    osd_setcolor(dev_fgcolor);
  }
  else {
    dev_fgcolor = 7;
    dev_bgcolor = 0;
#if USE_TERM_IO
    //              term_settextcolor(dev_fgcolor, dev_bgcolor);
#endif
  }

#if defined(_UnixOS)
  /*
    if  ( uname(&uts) == 0 )  {
    // will use the POSIX's uname()
    strcpy(tmp, "Unix/");
    strcat(tmp, uts.machine);
    strcat(tmp, "/");
    strcat(tmp, uts.sysname);
    setsysvar_str(SYSVAR_OSNAME, tmp);
    }
    else  {
  */
  // will try to read /proc/version

  verfd = open("/proc/version", O_RDONLY);
  if (verfd != -1) {
    char *p;
    char verstr[256];
    char tmp[300];
    int bytes;

    memset(verstr, 0, 256);
    bytes = read(verfd, verstr, 255);
    verstr[(bytes < 256) ? bytes : 255] = '\0';
    p = strchr(verstr, '\n');
    if (p) {
      *p = '\0';
    }
    close(verfd);

    // store name to system constant
    strcpy(tmp, "Unix/");
    strcat(tmp, verstr);
    setsysvar_str(SYSVAR_OSNAME, tmp);

    // build OSVER
    if ((p = strstr(verstr, "ersion")) != NULL) {
      long vi = 0;
      int dg = 0;

      p += 6;
      while (*p == ' ' || *p == '\t') {
        p++;
      }
      while (*p) {
        if (is_digit(*p)) {
          vi = (vi << 4) + (*p - '0');
          dg++;
        }
        else if (*p == '.') {
          switch (dg) {
          case 0:
            vi = vi << 8;
            break;
          case 1:
            vi = vi << 4;
            break;
          };

          dg = 0;
        }
        else {
          break;
        }

        p++;
      }                         // while (*p)

      os_ver = vi;
    }                           // if ver
  }                             // verfd

  setsysvar_int(SYSVAR_OSVER, os_ver);
#elif defined(_DOS)
  os_ver = ((_osmajor << 16) | (_osminor)) << 8;
  setsysvar_int(SYSVAR_OSVER, os_ver);
#elif defined(_WinBCB)
  if (flags == 0) {
    setsysvar_int(SYSVAR_OSVER, os_ver);
  }
#else
  setsysvar_int(SYSVAR_OSVER, os_ver);
#endif

#if defined(_WinBCB)
  if (flags == 0) {
#endif
    setsysvar_int(SYSVAR_XMAX, os_graf_mx - 1);
    setsysvar_int(SYSVAR_YMAX, os_graf_my - 1);
    if (os_graphics) {
      setsysvar_int(SYSVAR_BPP, os_color_depth);
    }
    else {
      setsysvar_int(SYSVAR_BPP, 4);
    }
#if defined(_WinBCB)
  }
#endif

#if USE_TERM_IO && !defined(__MINGW32__)
  signal(SIGINT, termination_handler);
  signal(SIGQUIT, termination_handler);
#endif
  return 1;
}

/**
 * restore device's mode
 */
int dev_restore() {
  if (os_graphics) {
    osd_refresh();
  }

#if defined(DRV_SOUND)
  if (drvsound_ok) {
    drvsound_close();
  }
#endif
#if defined(DRV_MOUSE)
  if (os_graphics) {
    if (drvmouse_ok) {
      drvmouse_close();
    }
  }
#endif
  dev_closefs();
  if (os_graphics) {
    osd_devrestore();
  }
#if USE_TERM_IO
  term_restore();               // by default
#endif

#if USE_TERM_IO && !defined(__MINGW32__)
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
#endif

  net_close();

  return 1;
}

/**
 * enable/disable default pointing device (pen or mouse)
 */
void dev_setpenmode(int enable) {
  if (os_graphics) {
    osd_setpenmode(enable);
  }
}

/**
 * clear to eol
 */
void dev_clreol() {
  dev_print("\033[K");          // ANSI
}

/**
 * returns the x position of cursor (in pixels)
 */
int dev_getx() {
#if USE_TERM_IO
  if (os_graphics) {
    return osd_getx();
  }
  return term_getx();
#else
  return osd_getx();
#endif
}

/**
 * returns the y position of cursor (in pixels)
 */
int dev_gety() {
#if USE_TERM_IO
  if (os_graphics) {
    return osd_gety();
  }
  return term_gety();
#else
  return osd_gety();
#endif
}

/**
 * sets the position of cursor
 * x,y are in pixels
 */
void dev_setxy(int x, int y) {
  if (x < 0 || x > os_graf_mx) {
    return;
  }
  if (y < 0 || y > os_graf_my) {
    return;
  }
#if USE_TERM_IO
  if (os_graphics) {
    osd_setxy(x, y);
  }
  else {
    term_setxy(x, y);
  }
#else
  osd_setxy(x, y);
#endif
}

/**
 * sets the currect foreground & background color
 * the background color is used only for texts
 */
void dev_settextcolor(long fg, long bg) {
#if USE_TERM_IO
  if (os_graphics) {
#endif
    if (bg == -1) {
      bg = dev_bgcolor;
    }

    if ((fg <= 15) && (bg <= 15) && (fg >= 0) && (bg >= 0)) { // VGA
      if (bg != -1) {
        dev_bgcolor = bg;
      }
      osd_settextcolor(dev_fgcolor = fg, dev_bgcolor);
    }
    else {
      osd_settextcolor((dev_fgcolor = fg), (dev_bgcolor = bg));
    }

#if USE_TERM_IO
  }
  else {
    term_settextcolor(fg, bg);
  }
#endif
}

/**
 * prints a string
 */
void dev_print(const char *str) {
#if USE_TERM_IO
  if (os_graphics) {
    osd_write(str);
  }
  else {
    term_print(str);
  }
#else
  osd_write(str);
#endif
}

/**
 * clears the screen
 */
void dev_cls() {
#if USE_TERM_IO
  if (os_graphics) {
    osd_cls();
  }
  else {
    term_cls();
  }
#else
  osd_cls();
#endif
}

/**
 * returns the width of 'str' in pixels
 */
int dev_textwidth(const char *str) {
  if (os_graphics) {
    return osd_textwidth(str);
  }
  return strlen(str);           // console
}

/**
 * returns the height of 'str' in pixels
 */
int dev_textheight(const char *str) {
  if (os_graphics) {
    return osd_textheight(str);
  }
  return 1;                     // console
}

/**
 * changes the current foreground color
 */
void dev_setcolor(long color) {
#if USE_TERM_IO
  if (os_graphics) {
#endif
    if (color <= 15 && color >= 0) {
      osd_setcolor(dev_fgcolor = color);
    }
    else if (color < 0) {
      osd_setcolor((dev_fgcolor = color));
    }
#if USE_TERM_IO
  }
  else {
    if (color <= 15 && color >= 0) {
      term_settextcolor(color, -1);
    }
  }
#endif
}

/**
 * draw a pixel
 */
void dev_setpixel(int x, int y) {
  x = W2X(x);
  y = W2Y(y);
  if (x >= dev_Vx1 && x <= dev_Vx2) {
    if (y >= dev_Vy1 && y <= dev_Vy2) {
#if USE_TERM_IO
      if (os_graphics) {
        osd_setpixel(x, y);
      }
      else {
        term_drawpoint(x, y);
      }
#else
      osd_setpixel(x, y);
#endif
    }
  }
}

/**
 * returns the value of a pixel
 */
long dev_getpixel(int x, int y) {
  x = W2X(x);
  y = W2Y(y);
  if (x >= dev_Vx1 && x <= dev_Vx2) {
    if (y >= dev_Vy1 && y <= dev_Vy2) {
#if USE_TERM_IO
      if (os_graphics) {
        return osd_getpixel(x, y);
      }
      else {
        return term_getpoint(x, y);
      }
#else
      return osd_getpixel(x, y);
#endif
    }
  }
  return 0;
}

/**
 * Cohen-Sutherland clipping
 */
void dev_clipline(int *x1, int *y1, int *x2, int *y2, int *visible) {
  int done, in1, in2, sw;
  int c1, c2;

  *visible = done = 0;
  do {
    CLIPENCODE(*x1, *y1, c1);
    CLIPENCODE(*x2, *y2, c2);
    in1 = CLIPIN(c1);
    in2 = CLIPIN(c2);
    if (in1 && in2) {
      *visible = done = 1;
    }
    else if ((c1 & c2 & 0x1) || (c1 & c2 & 0x2) || (c1 & c2 & 0x4) || (c1 & c2 & 0x8)) {
      done = 1;                 // visible = false
    }
    else {
      // at least one point is outside
      if (in1) {
        // swap
        sw = *x1;
        *x1 = *x2;
        *x2 = sw;
        sw = *y1;
        *y1 = *y2;
        *y2 = sw;
        sw = c1;
        c1 = c2;
        c2 = sw;
      }

      if (*x1 == *x2) {
        if (c1 & 0x2) {
          *y1 = dev_Vy1;
        }
        else {
          *y1 = dev_Vy2;
        }
      }
      else {
        if (c1 & 0x1) {
          *y1 += (*y2 - *y1) * (dev_Vx1 - *x1) / (*x2 - *x1);
          *x1 = dev_Vx1;
        }
        else if (c1 & 0x4) {
          *y1 += (*y2 - *y1) * (dev_Vx2 - *x1) / (*x2 - *x1);
          *x1 = dev_Vx2;
        }
        else if (c1 & 0x2) {
          *x1 += (*x2 - *x1) * (dev_Vy1 - *y1) / (*y2 - *y1);
          *y1 = dev_Vy1;
        }
        else if (c1 & 0x8) {
          *x1 += (*x2 - *x1) * (dev_Vy2 - *y1) / (*y2 - *y1);
          *y1 = dev_Vy2;
        }
      }
    }
  } while (!done);
}

/**
 * draw line
 */
void dev_line(int x1, int y1, int x2, int y2) {
  int visible;

  W2D4(x1, y1, x2, y2);

  // clip_line
  dev_clipline(&x1, &y1, &x2, &y2, &visible);
  if (visible) {
#if USE_TERM_IO
    if (os_graphics) {
      osd_line(x1, y1, x2, y2);
    }
    else {
      term_drawline(x1, y1, x2, y2);
    }
#else
    osd_line(x1, y1, x2, y2);
#endif
  }
}

/**
 * draw rectangle (filled or not)
 */
void dev_rect(int x1, int y1, int x2, int y2, int fill) {
  int px1, py1, px2, py2;
  int c1, c2, in1, in2;

  px1 = x1;
  py1 = y1;
  px2 = x2;
  py2 = y2;

  W2D4(x1, y1, x2, y2);

  if (x1 == x2) {
    dev_line(px1, py1, px2, py2);
    return;
  }
  if (y1 == y2) {
    dev_line(px1, py1, px2, py2);
    return;
  }

  /*
   *      check inside
   */
  CLIPENCODE(x1, y1, c1);
  CLIPENCODE(x2, y2, c2);
  in1 = CLIPIN(c1);
  in2 = CLIPIN(c2);
  if (in1 && in2) {
    /*
     *      its inside
     */

#if USE_TERM_IO
    if (os_graphics) {
      osd_rect(x1, y1, x2, y2, fill);
    }
    else {
      term_drawrect(x1, y1, x2, y2, fill);
    }
#else
    osd_rect(x1, y1, x2, y2, fill);
#endif
  }
  else {
    /*
     *      partial inside
     *      TODO: something fast
     */
    int y;

    if (fill) {
      for (y = py1; y <= py2; y++) {
        dev_line(px1, y, px2, y);
      }
    }
    else {
      dev_line(px1, py1, px1, py2);
      dev_line(px1, py2, px2, py2);
      dev_line(px2, py2, px2, py1);
      dev_line(px2, py1, px1, py1);
    }
  }
}

/**
 * set viewport
 */
void dev_viewport(int x1, int y1, int x2, int y2) {
  if (x1 == x2 || y1 == y2) {
    // reset
    dev_Vx1 = 0;
    dev_Vy1 = 0;
    dev_Vx2 = os_graf_mx - 1;
    dev_Vy2 = os_graf_my - 1;

    dev_Vdx = os_graf_mx - 1;
    dev_Vdy = os_graf_my - 1;
  }
  else {
    if ((x1 < 0) || (x2 < 0) ||
        (y1 < 0) || (y2 < 0) ||
        (x1 >= os_graf_mx) || (x2 >= os_graf_mx) ||
        (y1 >= os_graf_my) || (y2 >= os_graf_my)) {
      rt_raise(ERR_VP_POS);
    }

    dev_Vx1 = x1;
    dev_Vy1 = y1;
    dev_Vx2 = x2;
    dev_Vy2 = y2;

    dev_Vdx = ABS(x2 - x1);
    dev_Vdy = ABS(y2 - y1);

    if (dev_Vdx == 0 || dev_Vdy == 0) {
      rt_raise(ERR_VP_ZERO);
    }
  }

  // reset window
  dev_Wx1 = dev_Vx1;
  dev_Wy1 = dev_Vy1;
  dev_Wx2 = dev_Vx2;
  dev_Wy2 = dev_Vy2;

  dev_Wdx = dev_Vdx;
  dev_Wdy = dev_Vdy;
}

/**
 * set window
 */
void dev_window(int x1, int y1, int x2, int y2) {
  if (x1 == x2 || y1 == y2) {
    // reset
    dev_Wx1 = dev_Vx1;
    dev_Wy1 = dev_Vy1;
    dev_Wx2 = dev_Vx2;
    dev_Wy2 = dev_Vy2;

    dev_Wdx = dev_Vdx;
    dev_Wdy = dev_Vdy;
  }
  else {
    dev_Wx1 = x1;
    dev_Wy1 = y1;
    dev_Wx2 = x2;
    dev_Wy2 = y2;

    dev_Wdx = x2 - x1;
    dev_Wdy = y2 - y1;

    if (dev_Wdx == 0 || dev_Wdy == 0) {
      rt_raise(ERR_WIN_ZERO);
    }
  }
}
