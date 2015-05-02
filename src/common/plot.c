// This file is part of SmallBASIC
//
// plot routines
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/var.h"
#include "common/pproc.h"
#include "common/device.h"
#include "common/blib.h"

void plot_array_exr(var_t *var_p, var_num_t *xmin, var_num_t *xmax);
void plot_draw_x_axis(var_num_t xmin, var_num_t xmax, int left, int right, int y);
void plot_draw_y_axis(var_num_t ymin, var_num_t ymax, int top, int bottom, int x);

/*
 * returns the min & max value of the array
 */
void plot_array_exr(var_t *var_p, var_num_t *xmin, var_num_t *xmax) {
  var_int_t count, i;
  var_t *elem_p;
  var_num_t x;

  count = v_asize(var_p);

  if (count <= 0) {
    *xmin = *xmax = 0.0;
  } else {
    // starting value
    elem_p = v_elem(var_p, 0);
    *xmin = *xmax = v_getreal(elem_p);

    // scan
    for (i = 0; i < count; i++) {
      elem_p = v_elem(var_p, i);
      x = v_getreal(elem_p);

      if (*xmin > x) {
        *xmin = x;
      }
      if (*xmax < x) {
        *xmax = x;
      }
    }
  }
}

/*
 * draw X axis
 */
void plot_draw_x_axis(var_num_t xmin, var_num_t xmax, int left, int right, int y) {
  var_num_t xrange, dx, x;
  var_int_t count, sxrange, smin_dx, smin_dy;
  var_int_t i, sx, fw;
  char buf[64];

  xrange = xmax - xmin;
  sxrange = right - left;

  smin_dx = dev_textwidth("000"); // minimum screen-dx
  smin_dy = dev_textheight("0") / 2;  // minimum screen-dy
  count = sxrange / smin_dx;    // values to draw
  dx = xrange / (var_num_t) count;  // dx for values

  // draw
  dev_line(left, y, right, y);
  for (i = 0, sx = left, x = xmin; i < count; i++, sx += smin_dx, x += dx) {
    // draw text
    ftostr(x, buf);
    buf[3] = '\0';
    fw = dev_textwidth(buf);
    dev_setxy(sx - fw, y + 1, 0);
    dev_print(buf);
    dev_line(sx, y, sx, y + smin_dy);
  }
}

/*
 * draw Y axis
 */
void plot_draw_y_axis(var_num_t ymin, var_num_t ymax, int top, int bottom, int x) {
  var_num_t yrange, dy, y;
  var_int_t count, syrange, smin_dy, smin_dx;
  var_int_t i, sy, fh;
  char buf[64];

  yrange = ymax - ymin;
  syrange = bottom - top;

  smin_dx = dev_textwidth("0"); // minimum screen-dx
  smin_dy = dev_textheight("0") * 1.5;  // minimum screen-dy
  count = syrange / smin_dy;    // values to draw
  dy = yrange / (var_num_t) count;  // dx for values

  // draw
  dev_line(x, top, x, bottom);
  for (i = 0, sy = top, y = ymax; i < count; i++, sy += smin_dy, y -= dy) {
    // draw text
    ftostr(y, buf);
    buf[3] = '\0';
    fh = dev_textheight(buf);
    dev_setxy(x - dev_textwidth(buf), sy - fh, 0);
    dev_print(buf);
    dev_line(x, sy, x - smin_dx, sy);
  }
}

/*
 * PLOT xset, yset
 * PLOT xset USE y_expr
 * PLOT USE xy_expr
 *
 * Optional: ISO[TROPIC]
 */
void cmd_plot2(void) {
  var_t *vx = NULL, *vy = NULL;

  par_massget("pp", &vx, &vy);
  if (!prog_error) {
    // is there a use keyword ?
    if (code_peek() == kwUSE) {
      code_skipnext();
      code_getaddr();
      code_getaddr();
    } else {
      rt_raise("PLOT: Missing USE keyword");
      return;
    }
  }
}

/*
 * PLOT3D xset, yset, zset
 * PLOT3D xset, yset USE z_expr
 * PLOT3D xset USE zy_expr
 * PLOT3D USE xzy_expr
 */

/*
 * PLOT4D [xset], [yset], [zset], [tset]
 */

//
// PLOT xmin, xmax [, count] USE ...
//
void cmd_plot() {
  var_num_t x, xmin = 0, xmax = 0, dx, xstep;
  var_num_t *yt, *xt;
  var_int_t count = 0, i, border;
  bcip_t use_ip, exit_ip;
  int prev_fgcolor = dev_fgcolor;
  int prev_bgcolor = dev_bgcolor;

  par_massget("FFi", &xmin, &xmax, &count);
  if (prog_error) {
    return;
  }

  // is there a use keyword ?
  if (code_peek() == kwUSE) {
    code_skipnext();
    use_ip = code_getaddr();
    exit_ip = code_getaddr();
  } else {
    rt_raise("PLOT: Missing USE keyword");
    return;
  }

  // .................
  border = dev_textwidth("00000");
  dx = ABS(xmax - xmin);
  if (count <= 0) {
    count = os_graf_mx - border;
  }
  xstep = dx / (var_num_t) count;

  yt = (var_num_t*) malloc(sizeof(var_num_t) * count);
  xt = (var_num_t*) malloc(sizeof(var_num_t) * count);

  // execute user's expression for each element
  // get y values
  if (use_ip != INVALID_ADDR) {
    var_t v;

    for (i = 0, x = xmin; i < count && !prog_error; i++, x += xstep) {
      v_init(&v);
      v_setreal(&v, x);
      exec_usefunc(&v, use_ip);
      xt[i] = x;
      yt[i] = v_getreal(&v);
      v_free(&v);
    }

    // jmp to correct location
    code_jump(exit_ip);
  }

  if (!prog_error) {
    // draw
    chart_draw(0, 0, os_graf_mx, os_graf_my, yt, count, xt, count, 5
    /* points */, 2 /* ruler */);
//      for ( i = 0; i < count; i ++ )  
//              dev_setpixel(i, (yt[i] - ymin) * ystep);
  }

  free(xt);
  free(yt);
  dev_settextcolor(prev_fgcolor, prev_bgcolor);
}
