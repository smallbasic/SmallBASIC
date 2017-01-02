// This file is part of SmallBASIC
//
// lowlevel device (OS) I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#define DEVICE_MODULE

#include "common/device.h"
#include "common/messages.h"
#include "common/osd.h"
#include "common/sberr.h"
#include "common/blib.h"

#define W2X(x) (((((x) - dev_Wx1) * dev_Vdx) / dev_Wdx) + dev_Vx1)
#define W2Y(y) (((((y) - dev_Wy1) * dev_Vdy) / dev_Wdy) + dev_Vy1)
#define X2W(x) (((((x) - dev_Vx1) * dev_Wdx) / dev_Vdx) + dev_Wx1)
#define Y2W(y) (((((y) - dev_Vy1) * dev_Wdy) / dev_Vdy) + dev_Wy1)
#define W2D2(x,y) { (x) = W2X((x)); (y) = W2Y((y)); }
#define W2D4(x1,y1,x2,y2) { W2D2((x1),(y1)); W2D2((x2),(y2)); }
#define CLIPENCODE(x,y,c) { c = (x < dev_Vx1); \
                            c |= ((y < dev_Vy1) << 1); \
                            c |= ((x > dev_Vx2) << 2); \
                            c |= ((y > dev_Vy2) << 3); }
#define CLIPIN(c) ((c & 0xF) == 0)

uint32_t os_ver = 0x40000;
uint32_t os_color_depth = 16;
byte os_graphics = 0; // CONSOLE
int os_graf_mx = 80;
int os_graf_my = 25;

// graphics - viewport
int32_t dev_Vx1;
int32_t dev_Vy1;
int32_t dev_Vx2;
int32_t dev_Vy2;
int32_t dev_Vdx;
int32_t dev_Vdy;

// graphics - window world coordinates
int32_t dev_Wx1;
int32_t dev_Wy1;
int32_t dev_Wx2;
int32_t dev_Wy2;
int32_t dev_Wdx;
int32_t dev_Wdy;
long dev_fgcolor = 0;
long dev_bgcolor = 15;

/**
 * Returns data from pointing-device
 * (see PEN(x), osd_getpen(x))
 */
int dev_getpen(int code) {
  int result = 0;
  if (os_graphics) {
    result = osd_getpen(code);
    switch (code) {
    case 1:
    case 4:
    case 10:
      result = X2W(result);
      break;

    case 2:
    case 5:
    case 11:
      result = Y2W(result);
      break;
    }
  }
  return result;
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
 * returns the x position of cursor (in pixels)
 */
int dev_getx() {
  return osd_getx();
}

/**
 * returns the y position of cursor (in pixels)
 */
int dev_gety() {
  return osd_gety();
}

/**
 * sets the position of cursor
 * x,y are in pixels
 */
void dev_setxy(int x, int y, int transform) {
  if (x < 0 || x > os_graf_mx) {
    return;
  }
  if (y < 0 || y > os_graf_my) {
    return;
  }

  if (transform) {
    x = W2X(x);
    y = W2Y(y);
  }

  osd_setxy(x, y);
}

/**
 * sets the currect foreground & background color
 * the background color is used only for texts
 */
void dev_settextcolor(long fg, long bg) {
  if (bg == -1) {
    bg = dev_bgcolor;
  }
  
  if ((fg <= 15) && (bg <= 15) && (fg >= 0) && (bg >= 0)) { // VGA
    if (bg != -1) {
      dev_bgcolor = bg;
    }
    osd_settextcolor(dev_fgcolor = fg, dev_bgcolor);
  } else {
    osd_settextcolor((dev_fgcolor = fg), (dev_bgcolor = bg));
  }
}

/**
 * prints a string
 */
void dev_print(const char *str) {
  osd_write(str);
}

/**
 * clears the screen
 */
void dev_cls() {
  graph_reset();
  osd_cls();
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
  if (color <= 15 && color >= 0) {
    osd_setcolor(dev_fgcolor = color);
  } else if (color < 0) {
    osd_setcolor((dev_fgcolor = color));
  }
}

/**
 * draw a pixel
 */
void dev_setpixel(int x, int y) {
  x = W2X(x);
  y = W2Y(y);
  if (x >= dev_Vx1 && x <= dev_Vx2) {
    if (y >= dev_Vy1 && y <= dev_Vy2) {
      osd_setpixel(x, y);
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
      return osd_getpixel(x, y);
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
    } else if ((c1 & c2 & 0x1) || (c1 & c2 & 0x2) || (c1 & c2 & 0x4) || (c1 & c2 & 0x8)) {
      done = 1;                 // visible = false
    } else {
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
        } else {
          *y1 = dev_Vy2;
        }
      } else {
        if (c1 & 0x1) {
          *y1 += (*y2 - *y1) * (dev_Vx1 - *x1) / (*x2 - *x1);
          *x1 = dev_Vx1;
        } else if (c1 & 0x4) {
          *y1 += (*y2 - *y1) * (dev_Vx2 - *x1) / (*x2 - *x1);
          *x1 = dev_Vx2;
        } else if (c1 & 0x2) {
          *x1 += (*x2 - *x1) * (dev_Vy1 - *y1) / (*y2 - *y1);
          *y1 = dev_Vy1;
        } else if (c1 & 0x8) {
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
    osd_line(x1, y1, x2, y2);
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
    osd_rect(x1, y1, x2, y2, fill);
  } else {
    /*
     *      partial inside
     *      TODO: something fast
     */
    int y;

    if (fill) {
      for (y = py1; y <= py2; y++) {
        dev_line(px1, y, px2, y);
      }
    } else {
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
  } else {
    if ((x1 < 0) || (x2 < 0) || (y1 < 0) || (y2 < 0) || (x1 >= os_graf_mx) || (x2 >= os_graf_mx)
        || (y1 >= os_graf_my) || (y2 >= os_graf_my)) {
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
  } else {
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
