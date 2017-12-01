// This file is part of SmallBASIC
//
// SmallBASIC RTL - GRAPHICS
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/pproc.h"
#include "common/messages.h"

// graphics - relative coordinates
int gra_x;
int gra_y;

void graph_reset() {
  gra_x = gra_y = 0;
}

//
//  VIEW [x1,y1,x2,y2[,c[,b]]]
//
void cmd_view() {
  ipt_t p1, p2;
  int32_t prev_color = dev_fgcolor;
  int32_t color = dev_bgcolor;
  int32_t bcolor = -1;

  if (code_peek() != kwTYPE_EOC && code_peek() != kwTYPE_LINE) {
    p1 = par_getipt();
    if (prog_error)
      return;
    par_getcomma();
    if (prog_error)
      return;
    p2 = par_getipt();
    if (prog_error)
      return;
    if (code_peek() == kwTYPE_SEP) {
      par_getcomma();
      if (prog_error)
        return;
      color = par_getint();
      if (prog_error)
        return;
      if (code_peek() == kwTYPE_SEP) {
        par_getcomma();
        if (prog_error)
          return;
        bcolor = par_getint();
        if (prog_error)
          return;
      }
    }

    dev_setcolor(color);
    dev_rect(p1.x, p1.y, p2.x, p2.y, 1);
    if (bcolor != -1) {
      dev_setcolor(bcolor);
      dev_rect(p1.x - 1, p1.y - 1, p2.x + 1, p2.y + 1, 0);
    }
    dev_setcolor(prev_color);
    dev_viewport(p1.x, p1.y, p2.x, p2.y);
  } else {
    dev_viewport(0, 0, 0, 0);
  }
}

//
//  WINDOW [x1,y1,x2,y2]
//
void cmd_window() {
  ipt_t p1, p2;

  if (code_peek() != kwTYPE_EOC && code_peek() != kwTYPE_LINE) {
    p1 = par_getipt();
    if (prog_error)
      return;
    par_getcomma();
    if (prog_error)
      return;
    p2 = par_getipt();
    if (prog_error)
      return;
    dev_window(p1.x, p2.y, p2.x, p1.y); // QB compatible
    // dev_window(p1.x, p1.y, p2.x, p2.y); // SB default (logical one)
  } else {
    dev_window(0, 0, 0, 0);
  }
}

//
//  PSET [STEP] x, y [, color | COLOR color]
//
void cmd_pset() {
  long color = dev_fgcolor;
  int32_t step1 = 0;
  ipt_t pt;

  /*
   * [STEP] x, y
   */
  if (code_peek() == kwSTEP) {
    code_skipnext();
    step1 = 1;
  }

  pt = par_getipt();
  if (step1)
    (pt.x += gra_x, pt.y += gra_y);

  if (code_peek() == kwCOLOR) {
    code_skipnext();
    color = par_getint();
    if (prog_error)
      return;
  }
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error)
      return;
    color = par_getint();
    if (prog_error)
      return;
  }

  gra_x = pt.x;
  gra_y = pt.y;

  if (color != dev_fgcolor) {
    int prev_color = dev_fgcolor;

    dev_setcolor(color);
    dev_setpixel(pt.x, pt.y);
    dev_setcolor(prev_color);
  } else
    dev_setpixel(pt.x, pt.y);
}

//
//  LINE [STEP] x, y [{,|STEP} x2, y2] [, color | COLOR color]
//
void cmd_line() {
  ipt_t p1, p2;
  byte step = 0;
  int32_t color = dev_fgcolor;

  /*
   * [STEP] x, y
   */
  if (code_peek() == kwSTEP) {
    code_skipnext();
    step = 1;
  }

  p1 = par_getipt();
  if (step) {
    (p1.x += gra_x, p1.y += gra_y, step = 0);
  }
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
  }
  /*
   * x, y [,] ---> [STEP] ?
   */
  if (code_peek() == kwSTEP) {
    code_skipnext();
    step = 1;
  }

  if (code_peek() != kwCOLOR && code_peek() != kwTYPE_EOC && code_peek() != kwTYPE_LINE) {
    /*
     * [STEP] x, y [[,]STEP] ---> x2, y2
     */
    p2 = par_getipt();
    if (step)
      (p2.x += p1.x, p2.y += p1.y);
  } else {
    p2 = p1;
    p1.x = gra_x;
    p1.y = gra_y;
  }

  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error)
      return;
    color = par_getint();
    if (prog_error)
      return;
  }
  if (code_peek() == kwCOLOR) {
    code_skipnext();
    color = par_getint();
    if (prog_error)
      return;
  }

  /*
   * draw
   */
  gra_x = p2.x;
  gra_y = p2.y;

  if (color != dev_fgcolor) {
    int32_t prev_color = dev_fgcolor;

    dev_setcolor(color);
    dev_line(p1.x, p1.y, p2.x, p2.y);
    dev_setcolor(prev_color);
  } else
    dev_line(p1.x, p1.y, p2.x, p2.y);
}

//
//  RECT [STEP] x, y [{,|STEP} x2, y2] [COLOR color] [FILLED]
//
void cmd_rect() {
  ipt_t p1, p2;
  int32_t color = dev_fgcolor;
  byte fill = 0, step = 0;

  /*
   * [STEP] x, y
   */
  if (code_peek() == kwSTEP) {
    code_skipnext();
    step = 1;
  }

  p1 = par_getipt();
  if (step)
    (p1.x += gra_x, p1.y += gra_y, step = 0);

  if (code_peek() == kwTYPE_SEP)
    par_getcomma();

  /*
   * x, y [,] ---> [STEP] ?
   */
  if (code_peek() == kwSTEP) {
    code_skipnext();
    step = 1;
  }

  if (code_peek() != kwCOLOR && code_peek() != kwFILLED && code_peek() != kwTYPE_EOC
      && code_peek() != kwTYPE_LINE) {
    /*
     * [STEP] x, y [[,]STEP] ---> x2, y2
     */
    p2 = par_getipt();
    if (step)
      (p2.x += p1.x, p2.y += p1.y);
  } else {
    p2 = p1;
    p1.x = gra_x;
    p1.y = gra_y;
  }

  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error)
      return;
    color = par_getint();
    if (prog_error)
      return;
  }
  if (code_peek() == kwCOLOR) {
    code_skipnext();
    color = par_getint();
    if (prog_error)
      return;
  }
  if (code_peek() == kwFILLED) {
    code_skipnext();
    fill = 1;
  }

  /*
   * draw
   */
  gra_x = p2.x;
  gra_y = p2.y;

  if (color != dev_fgcolor) {
    int prev_color = dev_fgcolor;

    dev_setcolor(color);
    dev_rect(p1.x, p1.y, p2.x, p2.y, fill);
    dev_setcolor(prev_color);
  } else
    dev_rect(p1.x, p1.y, p2.x, p2.y, fill);
}

//
//  DRAWPOLY v() [, xorg, yorg [, scale[, color]] [COLOR color] [FILLED]
//
//  0,2,4... x
//  1,3,5... y
//
void cmd_drawpoly() {
  int i, count;
  var_num_t xorg = 0, yorg = 0;
  int32_t prev_color = dev_fgcolor;
  int32_t color = dev_fgcolor;
  byte filled = 0, scalef = 0;
  var_num_t scale = 1.0;
  ipt_t *poly = NULL;

  // array
  count = par_getipoly(&poly);
  if (prog_error) {
    free(poly);
    return;
  }
  if (count == 0) {
    free(poly);
    return;
  }
  // x,y origin
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (!prog_error) {
      xorg = par_getreal();
      if (!prog_error) {
        par_getcomma();
        if (!prog_error)
          yorg = par_getreal();
      }
    }

    if (prog_error) {
      free(poly);
      return;
    }

    // scale factor
    if (code_peek() == kwTYPE_SEP) {
      par_getcomma();
      if (!prog_error) {
        scale = par_getnum();
        if (!prog_error) {
          scalef++;
          if (code_peek() == kwTYPE_SEP) {
            par_getcomma();
            if (!prog_error)
              color = par_getint();
          }
        }
      }
    }

    if (prog_error) {
      free(poly);
      return;
    }
  }

  // color
  if (code_peek() == kwCOLOR) {
    code_skipnext();
    color = par_getint();
    if (prog_error) {
      free(poly);
      return;
    }
  }

  // filled
  if (code_peek() == kwFILLED) {
    code_skipnext();
    filled++;
  }

  // scale it and move it
  if (scalef || xorg != 0 || yorg != 0) {
    if (scalef) {
      for (i = 0; i < count; i++) {
        poly[i].x = xorg + poly[i].x * scale;
        poly[i].y = yorg + poly[i].y * scale;
      }
    } else {
      for (i = 0; i < count; i++) {
        poly[i].x = xorg + poly[i].x;
        poly[i].y = yorg + poly[i].y;
      }
    }
  }

  // ready
  if (color != dev_fgcolor)
    dev_setcolor(color);

  if (!filled) {
    for (i = 1; i < count; i++)
      dev_line(poly[i - 1].x, poly[i - 1].y, poly[i].x, poly[i].y);
  } else
    dev_pfill(poly, count);

  // cleanup
  free(poly);

  if (color != prev_color)
    dev_setcolor(prev_color);
}

//
//  CIRCLE [STEP] x, y, r [, aspect[, color]] [COLOR color] [FILLED]
//
void cmd_circle() {
  int32_t color = dev_fgcolor;
  byte fill = 0, step = 0;
  ipt_t pt;
  int r;
  var_num_t aspect = 1.0;
  byte code;

  /*
   * [STEP] x, y
   */
  code = code_peek();
  if (code == kwSTEP) {
    code_skipnext();
    step = 1;
  }

  // xc,yc
  pt = par_getipt();
  if (prog_error)
    return;
  // r
  par_getcomma();
  if (prog_error)
    return;
  r = par_getint();
  if (prog_error)
    return;
  if (step)
    (pt.x += gra_x, pt.y += gra_y);

  // aspect
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error)
      return;
    aspect = par_getnum();
    if (prog_error)
      return;

    if (code_peek() == kwTYPE_SEP) {
      par_getcomma();
      if (prog_error)
        return;
      color = par_getint();
      if (prog_error)
        return;
    }
  }

  // COLOR
  if (code_peek() == kwCOLOR) {
    code_skipnext();
    color = par_getint();
    if (prog_error)
      return;
  }

  // FILLED
  if (code_peek() == kwFILLED) {
    code_skipnext();
    fill = 1;
  }

  if (color != dev_fgcolor) {
    int32_t prev_color = dev_fgcolor;

    dev_setcolor(color);
    dev_ellipse(pt.x, pt.y, r, r, aspect, fill);
    dev_setcolor(prev_color);
  } else
    dev_ellipse(pt.x, pt.y, r, r, aspect, fill);
}

//
//  ARC [STEP] x, y, r, start, end [, aspect[, color]] [COLOR color]
//
void cmd_arc() {
  int32_t color = dev_fgcolor;
  byte step = 0;
  int r;
  var_num_t as, ae, aspect = 1.0;
  byte code;
  ipt_t pt;

  /*
   * [STEP] x, y
   */
  code = code_peek();
  if (code == kwSTEP) {
    code_skipnext();
    step = 1;
  }

  // xc,yc
  pt = par_getipt();
  if (prog_error)
    return;

  // r
  par_getcomma();
  if (prog_error)
    return;
  r = par_getint();
  if (prog_error)
    return;

  // a.st.
  par_getcomma();
  if (prog_error)
    return;
  as = par_getnum();
  if (prog_error)
    return;

  // a.end
  par_getcomma();
  if (prog_error)
    return;
  ae = par_getnum();
  if (prog_error)
    return;

  if (step)
    (pt.x += gra_x, pt.y += gra_y);

  // aspect
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error)
      return;
    aspect = par_getnum();
    if (prog_error)
      return;

    if (code_peek() == kwTYPE_SEP) {
      par_getcomma();
      if (prog_error)
        return;
      color = par_getint();
      if (prog_error)
        return;
    }
  }

  if (code_peek() == kwCOLOR) {
    code_skipnext();
    color = par_getint();
    if (prog_error)
      return;
  }

  if (color != dev_fgcolor) {
    int32_t prev_color = dev_fgcolor;

    dev_setcolor(color);
    dev_arc(pt.x, pt.y, r, as, ae, aspect);
    dev_setcolor(prev_color);
  } else {
    dev_arc(pt.x, pt.y, r, as, ae, aspect);
  }
}

//
//  PAINT [STEP] x, y [, fillcolor [, bordercolor]]
//
void cmd_paint() {
  int32_t prev_color = dev_fgcolor;
  int32_t color = dev_fgcolor;
  byte step = 0;
  int32_t fc = dev_fgcolor, bc = -1;
  byte code;
  ipt_t pt;

  /*
   * [STEP] x, y
   */
  code = code_peek();
  if (code == kwSTEP) {
    code_skipnext();
    step = 1;
  }

  // xc,yc
  pt = par_getipt();
  if (prog_error) {
    return;
  }
  if (step) {
    (pt.x += gra_x, pt.y += gra_y);
  }
  // fillcolor
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error) {
      return;
    }
    fc = par_getint();
    if (prog_error) {
      return;
    }
  }

  // bordercolor
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error) {
      return;
    }
    bc = par_getint();
    if (prog_error) {
      return;
    }
  }

  dev_setcolor(color);
  dev_ffill(pt.x, pt.y, fc, bc);
  dev_setcolor(prev_color);
}

//
char *draw_getval(const char *src, int *c) {
  char *p = (char *) src;
  char *dst, buf[64];

  dst = buf;
  p++;
  *c = 0;
  if (*p == '-') {
    *dst = '-';
    dst++;
    p++;
  }

  while (is_digit(*p)) {
    *dst++ = *p++;
  }
  *dst = '\0';

  *c = xstrtol(buf);
  return p;
}

//
//  DRAW "commands"
//
void cmd_draw() {
  register int draw = 1, update = 1;
  int x, y, r;
  int32_t prev_color = dev_fgcolor;
  char *p;
  var_t var;

  par_getstr(&var);
  if (prog_error) {
    return;
  }
  p = (char *) var.v.p.ptr;
  while (*p) {

    // 'N' command must affect only the next drawing command.
    update = 1;
    // Haraszti -- 'B' command must affect only the next drawing command.
    draw = 1;

    // commands prefix
    while (strchr("BbNn", *p)) {
      if (*p == 'B' || *p == 'b') { // do not draw
        draw = 0;
        p++;
      } else {
        draw = 1;
      }
      if (*p == 'N' || *p == 'n') { // do not update the position
        update = 0;
        p++;
      } else {
        update = 1;
      }
    }

    // commands
    switch (*p) {
    case 'U':
    case 'u':                  // up
      p = draw_getval(p, &y);
      if (draw) {
        dev_line(gra_x, gra_y, gra_x, gra_y - y);
      }
      if (update) {
        gra_y -= y;
      }
      continue;
    case 'D':
    case 'd':                  // down
      p = draw_getval(p, &y);
      if (draw) {
        dev_line(gra_x, gra_y, gra_x, gra_y + y);
      }
      if (update) {
        gra_y += y;
      }
      continue;
    case 'L':
    case 'l':                  // left
      p = draw_getval(p, &x);
      if (draw) {
        dev_line(gra_x, gra_y, gra_x - x, gra_y);
      }
      if (update) {
        gra_x -= x;
      }
      continue;
    case 'R':
    case 'r':                  // right
      p = draw_getval(p, &x);
      if (draw) {
        dev_line(gra_x, gra_y, gra_x + x, gra_y);
      }
      if (update) {
        gra_x += x;
      }
      continue;
    case 'E':
    case 'e':                  // up & right
      p = draw_getval(p, &x);
      if (draw) {
        dev_line(gra_x, gra_y, gra_x + x, gra_y - x);
      }
      if (update) {
        gra_x += x;
        gra_y -= x;
      }
      continue;
    case 'F':
    case 'f':                  // down & right
      p = draw_getval(p, &x);
      if (draw) {
        dev_line(gra_x, gra_y, gra_x + x, gra_y + x);
      }
      if (update) {
        gra_x += x;
        gra_y += x;
      }
      continue;
    case 'G':
    case 'g':                  // down & left
      p = draw_getval(p, &x);
      if (draw) {
        dev_line(gra_x, gra_y, gra_x - x, gra_y + x);
      }
      if (update) {
        gra_x -= x;
        gra_y += x;
      }
      continue;
    case 'H':
    case 'h':                  // up & left
      p = draw_getval(p, &x);
      if (draw) {
        dev_line(gra_x, gra_y, gra_x - x, gra_y - x);
      }
      if (update) {
        gra_x -= x;
        gra_y -= x;
      }
      continue;
    case 'M':
    case 'm':                  // move to x, y
      if (*(p + 1) == '-') {    // relative
        r = -1;
        p++;
      }
      if (*(p + 1) == '+') {    // relative
        r = 1;
        p++;
      } else {
        r = 0;                  // absolute
      }
      p = draw_getval(p, &x);
      if (*p != ',') {
        rt_raise(ERR_DRAW_SEP);
        v_free(&var);
        return;
      } else {
        // Haraszti -- next pointer forward is an error because draw_getval
        // contain p++ too!!!
        // p ++;
      }
      p = draw_getval(p, &y);

      if (r) {
        if (draw)
          dev_line(gra_x, gra_y, gra_x + x * r, gra_y + y * r);
        if (update) {
          gra_x += x * r;
          gra_y += x * r;
        }
      } else {
        if (draw) {
          dev_line(gra_x, gra_y, x, y);
        }
        if (update) {
          gra_x = x;
          gra_y = y;
        }
      }
      continue;
    case 'C':
    case 'c':                  // color
      p = draw_getval(p, &x);
      dev_setcolor(x);
      continue;
      // Haraszti -- next case filter out the spaces or tabs and semicolons
      // (GWBASIC compatibility)
    case ' ':
    case '\t':
    case ';':
      p++;
      continue;
    default:
      rt_raise(ERR_DRAW_CMD, *p);
      v_free(&var);
      return;
    }
    p++;
  }

  dev_setcolor(prev_color);
  v_free(&var);
}

//
//  CHART chart-type, v() [, mark-type [, x1, y1, x2, y2]]
//
//  chart-type
//      1 = line-chart
//      2 = bar-chart
//
//  mark-type (bit-mask)
//      0 = none
//      1 = labels
//      2 = ruler
//
void cmd_chart_fstr(var_num_t v, char *buf) {
  if (fabsl(v) >= 10E+9) {
    ftostr(v / 1E+9, buf);
    if (buf[3] == '.') {
      buf[3] = '\0';
    } else {
      buf[4] = '\0';
    }
    strcat(buf, "G");
  } else if (fabsl(v) >= 10E+6) {
    ftostr(v / 1E+6, buf);
    if (buf[3] == '.') {
      buf[3] = '\0';
    } else {
      buf[4] = '\0';
    }
    strcat(buf, "M");
  } else if (fabsl(v) >= 10E+3) {
    ftostr(v / 1E+3, buf);
    if (buf[3] == '.') {
      buf[3] = '\0';
    } else {
      buf[4] = '\0';
    }
    strcat(buf, "K");
  } else {
    ftostr(v, buf);
    buf[5] = '\0';
  }
}

/*
 *   draw a chart
 *
 *   x1,y1-x2,y2 = the area to draw
 *   vals = the values
 *   count = the number of the values
 *   xvals, xcount = for ruler the xvalues (use NULL for default)
 *   chart = chart type (1=line chart, 0=bar chart, 5=points)
 *   marks = marks type (2 & ruler, 1 & marks)
 */
void chart_draw(int x1, int y1, int x2, int y2, var_num_t *vals, int count, var_num_t *xvals, int xcount,
                int chart, int marks) {
  int *pts;
  int rx1, dx, dy, i;
  var_num_t vmin, vmax, lx, ly;
  char buf[32];
  int32_t color = 0;

  rx1 = x1;

  // ready
  dev_settextcolor(0, 15);
  pts = (int *) malloc(sizeof(int) * count * 2);

  if (marks & 0x2) {            // ruler
    x1 += dev_textwidth("00000") + 1;
    y2 -= (dev_textheight("0") + 1);
  }

  if (marks & 0x1) {
    if (chart == 1) {           // line
      x1 += 2;
      x2 -= 2;
      y1 += 2;
      y2 -= 2;
    }
  }

  dx = (x2 - x1);
  dy = (y2 - y1);

  // limits
  vmin = vmax = vals[0];
  for (i = 1; i < count; i++) {
    if (vmin > vals[i]) {
      vmin = vals[i];
    }
    if (vmax < vals[i]) {
      vmax = vals[i];
    }
  }

  if (chart == 1) {
    // line-chart
    lx = ((var_num_t) dx) / (var_num_t) (count - 1);
  } else {
    lx = ((var_num_t) dx) / (var_num_t) count;
  }
  ly = ((var_num_t) dy) / (vmax - vmin);

  // calc points
  for (i = 0; i < count; i++) {
    pts[i * 2] = x1 + i * lx;
    pts[i * 2 + 1] = y1 + (dy - ((vals[i] - vmin) * ly));
  }

  // draw ruler
  if (marks & 0x2) {
    int fh, fw, n, y, x;
    var_num_t v;

    // vertical
    fh = dev_textheight("0");
    n = dy / (fh * 1.5);

    if ((n - 1) > 0) {
      for (i = 0; i <= n; i++) {
        if (i == 0) {
          v = vmin;
        }
        else if (i == n) {
          v = vmax;
        } else {
          v = vmin + (((var_num_t) i + 1) * ((vmax - vmin) / (var_num_t) (n + 1)));
        }
        cmd_chart_fstr(v, buf);

        y = y1 + (dy - ((v - vmin) * ly));

        if (i != 0) {
          dev_setxy(rx1 + 1, y + 1, 0);
        } else {
          dev_setxy(rx1 + 1, y - fh, 0);
        }
        dev_print(buf);
        dev_line(x1 - 4, y, x1, y);
      }
    }

    // horizontal
    fw = dev_textwidth("000");
    n = -1;
    if (count <= 24) {
      if (count * (fw * 1.34) < dx) {
        n = count;
      }
    }

    if (n == -1) {
      n = dx / (fw * 1.5);
    }
    if ((n - 1) > 0) {
      for (i = 0; i < n; i++) {
        if (i == 0) {
          v = 0;
        } else {
          v = i * ((var_num_t) count / (var_num_t) n);
        }
        if (xvals) {
          // I have xvals
          var_num_t x, dx;
          var_num_t xmin, xmax;

          xmin = xvals[0];
          xmax = xvals[xcount - 1];
          dx = xmax - xmin;
          if (i == 0) {
            x = xmin;
          } else if (i == n) {
            x = xmax;
          } else {
            x = xmin + ((dx / n) * i);
          }
          ftostr(x, buf);
        } else {
          // i don't have xvals
          ftostr(i + 1, buf);
        }

        //
        buf[3] = '\0';
        fw = dev_textwidth(buf);

        x = x1 + v * lx;

        if (chart == 1 || chart == 5) {
          dev_setxy(x - fw, y2 + 1, 0);
        } else {
          if (x + fw + 1 < x2) {
            dev_setxy(x + 1, y2 + 1, 0);
          }
        }

        dev_print(buf);
        dev_line(x, y2, x, y2 + 4);
      }
    }

    dev_line(x1, y1, x1, y2);
    dev_line(x1, y2, x2, y2);

    x1++;
    y2--;
    dx = (x2 - x1);
    dy = (y2 - y1);
  }

  // draw
  switch (chart) {

  case 1:                      // line chart
  case 5:                      // points
    if (chart == 5) {
      for (i = 0; i < count; i++) {
        dev_setpixel(pts[i * 2], pts[i * 2 + 1]);
      }
    } else {
      for (i = 1; i < count; i++) {
        dev_line(pts[(i - 1) * 2], pts[(i - 1) * 2 + 1], pts[i * 2], pts[i * 2 + 1]);
      }
    }

    // draw marks
    if (marks & 0x1) {
      for (i = 0; i < count; i++) {
        int mx, my;
        int fh, fw;

        cmd_chart_fstr(vals[i], buf);

        fw = dev_textwidth(buf);
        fh = dev_textheight(buf);

        mx = pts[i * 2] - fw / 2;
        my = pts[i * 2 + 1];

        if (my > (y1 + (y2 - y1) / 2)) {
          my -= fh;
        }
        if (mx <= x1) {
          mx = x1 + 1;
        }
        if (mx + fw >= x2) {
          mx = x2 - fw;
        }
        dev_setxy(mx, my, 0);
        dev_print(buf);
        dev_rect(pts[i * 2] - 2, pts[i * 2 + 1] - 2, pts[i * 2] + 2, pts[i * 2 + 1] + 2, 1);
      }
    }
    break;

  case 2:                      // bar chart
    // draw rect
    color = 0;
    for (i = 1; i < count; i++) {
      if (os_color_depth > 2) {
        dev_setcolor(color);
        color++;
        if (color >= 15) {
          color = 0;
        }
      }
      dev_rect(pts[(i - 1) * 2], pts[(i - 1) * 2 + 1], pts[i * 2] - 2, y2, 1);
    }

    if (os_color_depth > 2) {
      dev_setcolor(color);
    }
    dev_rect(pts[(count - 1) * 2], pts[(count - 1) * 2 + 1], pts[(count - 1) * 2] + lx - 1, y2, 1);

    // draw marks
    if (marks & 0x1) {
      color = 0;
      for (i = 0; i < count; i++) {
        int mx, my;
        int fh, fw;

        cmd_chart_fstr(vals[i], buf);

        fw = dev_textwidth(buf);
        fh = dev_textheight(buf);

        mx = pts[i * 2] + lx / 2 - fw / 2;
        my = pts[i * 2 + 1];

        if (os_color_depth > 2) {
          if (my - fh >= y1) {
            dev_settextcolor(0, 15);
          } else {
            if (color >= 7 && color != 8) {
              dev_settextcolor(0, color);
            } else {
              dev_settextcolor(15, color);
            }
          }

          color++;
          if (color >= 15) {
            color = 0;
          }
        }

        if (my - fh >= y1) {
          my -= fh;
        }
        if (mx <= x1) {
          mx = x1 + 1;
        }
        if (mx + fw >= x2) {
          mx = x2 - fw;
        }
        dev_setxy(mx, my, 0);
        dev_print(buf);
      }
    }
    break;
  };

  //
  free(pts);
}

//
//  CHART
//
void cmd_chart() {
  var_t *var_p, *elem_p;
  int i, count;
  var_num_t *vals;
  int32_t prev_fgcolor = dev_fgcolor;
  int32_t prev_bgcolor = dev_bgcolor;

  // parameters
  int chart;
  int marks = 0;
  int x1 = 0, y1 = 0, x2 = os_graf_mx, y2 = os_graf_my;

  // chart type
  chart = par_getint();
  if (prog_error)
    return;

  par_getcomma();
  if (prog_error)
    return;

  // array
  var_p = par_getvarray();
  if (prog_error)
    return;
  count = var_p->v.a.size;

  // optional labels-flag
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error)
      return;
    marks = par_getint();
    if (prog_error)
      return;

    // optional x1,y1,x2,y2
    if (code_peek() == kwTYPE_SEP) {
      par_getcomma();
      if (prog_error)
        return;
      x1 = par_getint();
      if (prog_error)
        return;
      par_getcomma();
      if (prog_error)
        return;
      y1 = par_getint();
      if (prog_error)
        return;
      par_getcomma();
      if (prog_error)
        return;
      x2 = par_getint();
      if (prog_error)
        return;
      par_getcomma();
      if (prog_error)
        return;
      y2 = par_getint();
      if (prog_error)
        return;
    }
  }

  // get array's values
  vals = (var_num_t *) malloc(sizeof(var_num_t) * count);
  for (i = 0; i < count; i++) {
    elem_p = v_getelemptr(var_p, i);
    if (prog_error) {
      free(vals);
      return;
    }

    switch (elem_p->type) {
    case V_INT:
      vals[i] = elem_p->v.i;
      break;
    case V_NUM:
      vals[i] = elem_p->v.n;
      break;
    case V_STR:
      vals[i] = v_getreal(elem_p);
      break;
    default:
      err_typemismatch();
      free(vals);
      return;
    }
  }

  //
  chart_draw(x1, y1, x2, y2, vals, count, NULL, 0, chart, marks);

  //
  free(vals);
  dev_settextcolor(prev_fgcolor, prev_bgcolor);
}

var_t *par_getm3() {
  // array
  var_t *vp = par_getvarray();
  if (prog_error) {
    return NULL;
  }
  if (vp == NULL || vp->type != V_ARRAY || vp->v.a.size != 9) {
    err_typemismatch();
    return NULL;
  }
  return vp;
}

void m3combine(var_t *m, var_num_t nm[3][3]) {
  var_num_t om[3][3];
  int i, j;
  var_t *e;

  // copy m to om
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      e = v_elem(m, (i * 3 + j));
      if (e->type == V_NUM) {
        om[i][j] = e->v.n;
      } else if (e->type == V_INT) {
        om[i][j] = e->v.i;
      } else {
        om[i][j] = v_getval(e);
      }
    }
  }

  // combine
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      e = v_elem(m, (i * 3 + j));
      if (e->type != V_NUM) {
        v_free(e);
      }
      e->type = V_NUM;
      e->v.n = nm[i][0] * om[0][j] + nm[i][1] * om[1][j] + nm[i][2] * om[2][j];
    }
  }

}

//
void m3ident(var_num_t m[3][3]) {
  int i, j;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      m[i][j] = (i == j) ? 1.0 : 0.0;
    }
  }
}

//
//  M3IDENT BYREF m3x3
//
void cmd_m3ident() {
  var_t *m, *e;
  int i, j;

  m = par_getm3();
  if (prog_error) {
    return;
  }
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      e = v_elem(m, (i * 3 + j));
      v_init(e);
      e->type = V_NUM;
      e->v.n = (i == j) ? 1.0 : 0.0;
    }
  }
}

//
//  M3ROTATE BYREF m3x3, angle[, x, y]
//
void cmd_m3rotate() {
  var_t *m;
  var_num_t angle, x = 0, y = 0, c, s;
  var_num_t matrix[3][3];

  m = par_getm3();
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }
  angle = par_getnum();
  if (prog_error) {
    return;
  }
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error)
      return;
    x = par_getnum();
    if (prog_error)
      return;
    par_getcomma();
    if (prog_error)
      return;
    y = par_getnum();
    if (prog_error) {
      return;
    }
  }

  c = cos(angle);
  s = sin(angle);

  m3ident(matrix);
  matrix[0][0] = c;
  matrix[0][1] = s;
  matrix[1][0] = -s;
  matrix[1][1] = c;
  matrix[2][0] = (1.0 - c) * x + (s * y);
  matrix[2][1] = (1.0 - c) * y - (s * x);
  m3combine(m, matrix);
}

//
//  M3SCALE BYREF m3x3, x, y, fx, fy
//
void cmd_m3scale() {
  var_t *m;
  var_num_t x, y, fx, fy;
  var_num_t matrix[3][3];

  m = par_getm3();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  x = par_getnum();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  y = par_getnum();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  fx = par_getnum();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  fy = par_getnum();
  if (prog_error)
    return;

  m3ident(matrix);
  matrix[0][0] = fx;
  matrix[1][1] = fy;
  matrix[2][0] = (1.0 - fx) * x;
  matrix[2][1] = (1.0 - fy) * y;
  m3combine(m, matrix);
}

//
//  M3TRANS BYREF m3x3, x, y
//
void cmd_m3translate() {
  var_t *m;
  var_num_t x, y;
  var_num_t matrix[3][3];

  m = par_getm3();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  x = par_getnum();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  y = par_getnum();
  if (prog_error)
    return;

  m3ident(matrix);
  matrix[2][0] = x;
  matrix[2][1] = y;
  m3combine(m, matrix);
}

//
//  M3APPLY m3x3, BYREF poly
//
void cmd_m3apply() {
  var_t *m, *p, *e;
  var_num_t om[3][3], x, y;
  int i, j, count;

  m = par_getm3();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  p = par_getvarray();
  if (prog_error)
    return;
  count = p->v.a.size;

  // copy m to om
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      e = v_elem(m, i * 3 + j);
      om[i][j] = v_getreal(e);
    }
  }

  // apply
  e = v_elem(p, 0);
  if (e->type != V_ARRAY) {
    int o;

    count = (p->v.a.size >> 1);
    for (i = 0; i < count; i++) {
      o = i << 1;
      x = v_getreal(v_elem(p, o));
      y = v_getreal(v_elem(p, o + 1));
      v_setreal(v_elem(p, o), x * om[0][0] + y * om[1][0] + om[2][0]);
      v_setreal(v_elem(p, o + 1), x * om[0][1] + y * om[1][1] + om[2][1]);
    }
  } else {
    for (i = 0; i < count; i++) {
      e = v_elem(p, i);

      if (e->type != V_ARRAY)
        err_parsepoly(i, 10);
      else if ((e->v.a.size % 2) != 0)
        err_parsepoly(i, 11);

      if (prog_error)
        break;

      x = v_getreal(v_elem(e, 0));
      y = v_getreal(v_elem(e, 1));
      v_setreal(v_elem(e, 0), x * om[0][0] + y * om[1][0] + om[2][0]);
      v_setreal(v_elem(e, 1), x * om[0][1] + y * om[1][1] + om[2][1]);
    }
  }
}

//
// INTERSECT a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y, BYREF type, BYREF r.x, BYREF r.y
//
void cmd_intersect() {
  var_num_t a, b, c, s;
  pt_t A, B, C, D, R;
  var_t *type, *vrx, *vry = NULL;
  byte style = 0;

  // parameters
  A = par_getpt();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  B = par_getpt();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  C = par_getpt();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;
  D = par_getpt();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;

  type = par_getvar_ptr();
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error)
    return;

  vrx = par_getvar_ptr();
  if (prog_error)
    return;
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error)
      return;
    vry = par_getvar_ptr();
    if (prog_error)
      return;
  } else
    style = 1;

  // initialize vars
  v_free(type);
  R.x = R.y = 0;

  //
  a = (B.y - A.y) * (D.x - C.x) - (B.x - A.x) * (D.y - C.y);
  b = (A.x - C.x) * (D.y - C.y) - (A.y - C.y) * (D.x - C.x);
  c = (A.x - C.x) * (B.y - A.y) - (A.y - C.y) * (B.x - A.x);

  if (a == 0.0)
    type->v.i = (b == 0.0) ? 3 : 2;
  else {
    if (a > 0.0) {
      if ((b >= 0.0 && b <= a) && (c >= 0.0 && c <= a)) {
        type->v.i = 1;
      } else {
        type->v.i = 0;
      }
    } else {
      if ((b <= 0.0 && b >= a) && (c <= 0.0 && c >= a)) {
        type->v.i = 1;
      } else {
        type->v.i = 0;
      }
    }
  }

  //
  if (type->v.i == 1 || type->v.i == 0) {

    if (b == a || c == a || c == 0.0)
      if (type->v.i == 1)
        type->v.i = 4;          // Special case

    s = b / a;

    if (C.x == D.x)
      R.x = C.x;
    else
      R.x = A.x + ((B.x - A.x) * s);

    if (C.y == D.y)
      R.y = C.y;
    else
      R.y = A.y + ((B.y - A.y) * s);
  }

  //
  if (style == 1) {
    v_toarray1(vrx, 2);
    v_setreal(v_elem(vrx, 0), R.x);
    v_setreal(v_elem(vrx, 1), R.y);
  } else {
    v_setreal(vrx, R.x);
    v_setreal(vry, R.y);
  }
}

//
//  POLYEXT poly, BYREF xmin, BYREF ymin, BYREF xmax, BYREF ymax
//
void cmd_polyext() {
  var_t *xmin, *ymin, *xmax, *ymax;
  int count, i;
  pt_t *poly = NULL;

  count = par_getpoly(&poly);
  if (prog_error)
    return;

  par_massget("PPPP", &xmin, &ymin, &xmax, &ymax);
  if (prog_error) {
    free(poly);
    return;
  }

  // initialize vars
  v_free(xmin);
  xmin->type = V_NUM;
  v_free(xmax);
  xmax->type = V_NUM;
  v_free(ymin);
  ymin->type = V_NUM;
  v_free(ymax);
  ymax->type = V_NUM;

  if (count == 0) {
    xmin->v.n = ymin->v.n = xmax->v.n = ymax->v.n = 0.0;
    free(poly);
    return;
  }

  xmin->v.n = xmax->v.n = poly[0].x;
  ymin->v.n = ymax->v.n = poly[0].y;
  for (i = 1; i < count; i++) {
    if (poly[i].x > xmax->v.n)
      xmax->v.n = poly[i].x;
    if (poly[i].x < xmin->v.n)
      xmin->v.n = poly[i].x;

    if (poly[i].y > ymax->v.n)
      ymax->v.n = poly[i].y;
    if (poly[i].y < ymin->v.n)
      ymin->v.n = poly[i].y;
  }

  free(poly);
}
