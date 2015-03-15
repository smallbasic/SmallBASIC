// This file is part of SmallBASIC
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "config.h"
#include "common/device.h"
#include "common/osd.h"

#ifndef IMPL_DEV_CIRCLE

#include <math.h>

#if !defined(M_PI)
#define M_PI  3.14159265358979323846
#endif

void set_4pixel(int x, int y, int xc, int yc) {
  dev_setpixel(xc + x, yc + y);
  dev_setpixel(xc - x, yc + y);
  dev_setpixel(xc + x, yc - y);
  dev_setpixel(xc - x, yc - y);
}

void set_4line(int x, int y, int xc, int yc) {
  dev_line(xc - x, yc + y, xc + x, yc + y);
  dev_line(xc - x, yc - y, xc + x, yc - y);
}

/*
 */
void simple_ellipse(int xc, int yc, int rx, int ry, int fill) {
  int x = 0;
  int y = ry;

  double a = rx; /* use 32-bit precision */
  double b = ry;
  double Asquared = a * a; /* initialize values outside */
  double TwoAsquared = 2 * Asquared; /* of loops */
  double Bsquared = b * b;
  double TwoBsquared = 2 * Bsquared;
  double d;
  double dx, dy;

  d = Bsquared - Asquared * b + Asquared / 4L;
  dx = 0;
  dy = TwoAsquared * b;

  while (dx < dy) {
    if (fill) {
      set_4line(x, y, xc, yc);
    } else {
      set_4pixel(x, y, xc, yc);
    }

    if (d > 0L) {
      --y;
      dy -= TwoAsquared;
      d -= dy;
    }

    ++x;
    dx += TwoBsquared;
    d += Bsquared + dx;
  }

  d += (3L * (Asquared - Bsquared) / 2L - (dx + dy)) / 2L;

  while (y >= 0) {
    if (fill) {
      set_4line(x, y, xc, yc);
    } else {
      set_4pixel(x, y, xc, yc);
    }
    if (d < 0L) {
      ++x;
      dx += TwoBsquared;
      d += dx;
    }

    --y;
    dy -= TwoAsquared;
    d += Asquared - dy;
  }
}

/*
 */
void dev_ellipse(int xc, int yc, int xr, int yr, double aspect, int fill) {
  simple_ellipse(xc, yc, xr, yr * aspect, fill);
}

/*
 */
void dev_arc(int xc, int yc, double r, double start, double end, double aspect) {
  double th, ph, xs, ys, xe, ye, x, y;
  int i;

  if (r < 1) {
    r = 1;
  }
  while (end < start) {
    end += M_PI * 2.0;
  }
  th = (end - start) / r;
  xs = xc + r * cos(start);
  ys = yc + r * aspect * sin(start);
  xe = xc + r * cos(end);
  ye = yc + r * aspect * sin(end);
  x = xs;
  y = ys;
  for (i = 1; i < r; i++) {
    ph = start + i * th;
    xs = xc + r * cos(ph);
    ys = yc + r * aspect * sin(ph);
    dev_line(x, y, xs, ys);
    x = xs;
    y = ys;
  }
  dev_line(x, y, xe, ye);
}

#endif
