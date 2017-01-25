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
