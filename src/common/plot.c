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
