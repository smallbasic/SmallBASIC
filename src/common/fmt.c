// This file is part of SmallBASIC
//
// formating numbers and strings
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/str.h"
#include "common/panic.h"
#include "common/fmt.h"
#include "common/device.h"
#include "common/pproc.h"
#include "common/messages.h"
#include "common/blib_math.h"

// limits for use with 64bit integer algorithm
#define FMT_xMIN        1e-8          // lowest limit to use the exp. format
#define FMT_xMAX        1e+9          // highest limit to use the exp. format
#define FMT_RND         9             // rounding on x digits
#define FMT_xRND        1e+9          // 1 * 10 ^ FMT_RND
#define FMT_xRND2       1e+8          // 1 * 10 ^ (FMT_RND-1)

// PRINT USING; format-list
#define MAX_FMT_N       128

void bestfta_p(var_num_t x, char *dest, var_num_t minx, var_num_t maxx);
void fmt_nmap(int dir, char *dest, char *fmt, char *src);
void fmt_omap(char *dest, const char *fmt);
int fmt_cdig(char *fmt);
char *fmt_getnumfmt(char *dest, char *source);
char *fmt_getstrfmt(char *dest, char *source);
void fmt_addfmt(const char *fmt, int type);
void fmt_printL(int output, int handle);

typedef struct {
  char *fmt;                    // the format or a string
  int type;                     // 0 = string, 1 = numeric format, 2 = string
// format
} fmt_node_t;

static fmt_node_t fmt_stack[MAX_FMT_N]; // the list
static int fmt_count;           // number of elements in the list
static int fmt_cur;             // next format element to be used

/*
 * tables of powers :)
 */
static double nfta_eplus[] = { 1e+8, 1e+16, 1e+24, 1e+32, 1e+40, 1e+48, 1e+56, 1e+64,  // 8
    1e+72, 1e+80, 1e+88, 1e+96, 1e+104, 1e+112, 1e+120, 1e+128, // 16
    1e+136, 1e+144, 1e+152, 1e+160, 1e+168, 1e+176, 1e+184, 1e+192, // 24
    1e+200, 1e+208, 1e+216, 1e+224, 1e+232, 1e+240, 1e+248, 1e+256, // 32
    1e+264, 1e+272, 1e+280, 1e+288, 1e+296, 1e+304  // 38
    };

static double nfta_eminus[] = { 1e-8, 1e-16, 1e-24, 1e-32, 1e-40, 1e-48, 1e-56, 1e-64,  // 8
    1e-72, 1e-80, 1e-88, 1e-96, 1e-104, 1e-112, 1e-120, 1e-128, // 16
    1e-136, 1e-144, 1e-152, 1e-160, 1e-168, 1e-176, 1e-184, 1e-192, // 24
    1e-200, 1e-208, 1e-216, 1e-224, 1e-232, 1e-240, 1e-248, 1e-256, // 32
    1e-264, 1e-272, 1e-280, 1e-288, 1e-296, 1e-304  // 38
    };

/*
 * Part of floating point to string (by using integers) algorithm
 * where x any number 2^31 > x >= 0 
 */
void fptoa(var_num_t x, char *dest) {
  dest[0] = '\0';
  sprintf(dest, VAR_INT_NUM_FMT, x);
}

/*
 * remove rightest zeroes from the number
 */
var_num_t rmzeros(var_num_t n) {
  var_int_t i = fint(n);
  while (i > 0 && i % 10 == 0) {
    i /= 10;
  }
  return i;
}

/*
 * best float to string (lib)
 *
 * This is the real float-to-string routine.
 * It used by the routines:
 *   bestfta(double x, char *dest)
 *   expfta(double x, char *dest)
 */
void bestfta_p(var_num_t x, char *dest, var_num_t minx, var_num_t maxx) {
  var_num_t ipart, fpart, fdif;
  var_int_t power = 0;
  int sign, i;
  char *d = dest;
  char buf[64];

  memset(buf, 0, sizeof(buf));

  if (fabsl(x) == 0.0) {
    strcpy(dest, "0");
    return;
  }

  // find sign
  sign = sgn(x);
  if (sign < 0) {
    *d++ = '-';
  }
  x = fabsl(x);

  if (x >= 1E308) {
    *d = '\0';
    strcat(d, WORD_INF);
    return;
  } else if (x <= 1E-307) {
    *d = '\0';
    strcat(d, "0");
    return;
  }

  // find power
  if (x < minx) {
    for (i = 37; i >= 0; i--) {
      if (x < nfta_eminus[i]) {
        x *= nfta_eplus[i];
        power = -((i + 1) * 8);
      } else {
        break;
      }
    }

    while (x < 1.0 && power > -307) {
      x *= 10.0;
      power--;
    }
  } else if (x > maxx) {
    for (i = 37; i >= 0; i--) {
      if (x > nfta_eplus[i]) {
        x /= nfta_eplus[i];
        power = ((i + 1) * 8);
      } else {
        break;
      }
    }

    while (x >= 10.0 && power < 308) {
      x /= 10.0;
      power++;
    }
  }

  // format left part
  ipart = fabsl(fint(x));
  fpart = fround(frac(x), FMT_RND) * FMT_xRND;
  if (fpart >= FMT_xRND) {      // rounding bug
    ipart = ipart + 1.0;
    if (ipart >= maxx) {
      ipart = ipart / 10.0;
      power++;
    }
    fpart = 0.0;
  }

  fptoa(ipart, buf);
  strcpy(d, buf);
  d += strlen(buf);

  if (fpart > 0.0) {
    // format right part
    *d++ = '.';

    fdif = frac(x) * FMT_xRND;
    if (fdif < fpart) {
      // rounded value has greater precision
      fdif = fpart;
    }

    while (fdif < FMT_xRND2) {
      fdif *= 10;
      *d++ = '0';
    }

    fptoa(rmzeros(fpart), buf);
    strcpy(d, buf);
    d += strlen(buf);
  }

  if (power) {
    // add the power
    *d++ = 'E';
    if (power > 0) {
      *d++ = '+';
    }
    fptoa(power, buf);
    strcpy(d, buf);
    d += strlen(buf);
  }

  // finish
  *d = '\0';
}

/*
 * best float to string (user)
 */
void bestfta(var_num_t x, char *dest) {
  bestfta_p(x, dest, FMT_xMIN, FMT_xMAX);
}

/*
 * float to string (user, E mode)
 */
void expfta(var_num_t x, char *dest) {
  bestfta_p(x, dest, 1.0, 1.0);
  if (strchr(dest, 'E') == NULL) {
    strcat(dest, "E+0");
  }
}

/*
 * format: map number to format
 *
 * dir = direction, 1 = left to right, -1 right to left
 */
void fmt_nmap(int dir, char *dest, char *fmt, char *src) {
  char *p, *d, *s;

  *dest = '\0';
  if (dir > 0) {
    // 
    // left to right
    // 
    p = fmt;
    d = dest;
    s = src;
    while (*p) {
      switch (*p) {
      case '#':
      case '^':
        if (*s) {
          *d++ = *s++;
        }
        break;
      case '0':
        if (*s) {
          *d++ = *s++;
        } else {
          *d++ = '0';
        }
        break;
      default:
        *d++ = *p;
      }

      p++;
    }

    *d = '\0';
  } else {
    // 
    // right to left
    // 
    p = fmt + (strlen(fmt) - 1);
    d = dest + (strlen(fmt) - 1);
    *(d + 1) = '\0';
    s = src + (strlen(src) - 1);
    while (p >= fmt) {
      switch (*p) {
      case '#':
      case '^':
        if (s >= src) {
          *d-- = *s--;
        } else {
          *d-- = ' ';
        }
        break;
      case '0':
        if (s >= src) {
          *d-- = *s--;
        } else {
          *d-- = '0';
        }
        break;
      default:
        if (*p == ',') {
          if (s >= src) {
            if (*s == '-') {
              *d-- = *s--;
            } else {
              *d-- = *p;
            }
          } else {
            *d-- = ' ';
          }
        } else {
          *d-- = *p;
        }
      }

      p--;
    }
  }
}

/*
 * format: map number-overflow to format
 */
void fmt_omap(char *dest, const char *fmt) {
  char *p = (char *) fmt;
  char *d = dest;

  while (*p) {
    switch (*p) {
    case '#':
    case '0':
    case '^':
      *d++ = '*';
      break;
    default:
      *d++ = *p;
    }

    p++;
  }
  *d = '\0';
}

/*
 * format: count digits
 */
int fmt_cdig(char *fmt) {
  char *p = fmt;
  int count = 0;

  while (*p) {
    switch (*p) {
    case '#':
    case '0':
    case '^':
      count++;
      break;
    }

    p++;
  }

  return count;
}

/*
 * format: format a number
 *
 * symbols:
 *   # = digit or space
 *   0 = digit or zero
 *   ^ = exponential digit/format
 *   . = decimal point
 *   , = thousands
 *   - = minus for negative
 *   + = sign of number
 */
void format_num(char *dest, const char *fmt_cnst, var_num_t x) {
  char *p, *fmt;
  char left[64], right[64];
  char lbuf[64], rbuf[64];
  int dp = 0, lc = 0, sign = 0;
  int rsz, lsz;

  // backup of format
  fmt = malloc(strlen(fmt_cnst) + 1);
  strcpy(fmt, fmt_cnst);

  // check sign
  if (strchr(fmt, '-') || strchr(fmt, '+')) {
    sign = 1;
    if (x < 0.0) {
      sign = -1;
      x = -x;
    }
  }

  if (strchr(fmt_cnst, '^')) {
    // 
    // E format
    // 

    lc = fmt_cdig(fmt);
    if (lc < 4) {
      fmt_omap(dest, fmt);
      free(fmt);
      return;
    }

    // convert
    expfta(x, dest);

    // format
    p = strchr(dest, 'E');
    if (p) {
      *p = '\0';
      strcpy(left, dest);
      strcpy(right, p + 1);
      lsz = strlen(left);
      rsz = strlen(right) + 1;

      if (lc < rsz + 1) {
        fmt_omap(dest, fmt);
        free(fmt);
        return;
      }

      if (lc < lsz + rsz + 1)
        left[lc - rsz] = '\0';

      strcpy(lbuf, left);
      strcat(lbuf, "E");
      strcat(lbuf, right);
      fmt_nmap(-1, dest, fmt, lbuf);
    } else {
      strcpy(left, dest);
      fmt_nmap(-1, dest, fmt, left);
    }
  } else {
    // 
    // normal format
    // 

    // rounding
    p = strchr(fmt, '.');
    if (p) {
      x = fround(x, fmt_cdig(p + 1));
    } else {
      x = fround(x, 0);
    }

    // convert
    bestfta(x, dest);
    if (strchr(dest, 'E')) {
      fmt_omap(dest, fmt);
      free(fmt);
      return;
    }

    // left & right parts
    left[0] = right[0] = '\0';
    p = strchr(dest, '.');
    if (p) {
      *p = '\0';
      strcpy(right, p + 1);
    }
    strcpy(left, dest);

    // map format
    rbuf[0] = lbuf[0] = '\0';
    p = strchr(fmt, '.');
    if (p) {
      dp = 1;
      *p = '\0';
      fmt_nmap(1, rbuf, p + 1, right);
    }

    lc = fmt_cdig(fmt);
    if (lc < strlen(left)) {
      fmt_omap(dest, fmt_cnst);
      free(fmt);
      return;
    }
    fmt_nmap(-1, lbuf, fmt, left);

    strcpy(dest, lbuf);
    if (dp) {
      strcat(dest, ".");
      strcat(dest, rbuf);
    }
  }

  // sign in format
  if (sign) {                   // 24/6 Snoopy42 modifications
    char *e;

    e = strchr(dest, 'E');
    if (e) {                    // special treatment for E format 
      p = strchr(dest, '+');
      if (p && p < e) {          // the sign bust be before the E 
        *p = (sign > 0) ? '+' : '-';
      }
      p = strchr(dest, '-');
      if (p && p < e) {
        *p = (sign > 0) ? ' ' : '-';
      }
    } else {                      // no E format 
      p = strchr(dest, '+');
      if (p) {
        *p = (sign > 0) ? '+' : '-';
      }
      p = strchr(dest, '-');
      if (p) {
        *p = (sign > 0) ? ' ' : '-';
      }
    }
  }

  // cleanup
  free(fmt);
}

/*
 * format: format a string
 *
 * symbols:
 *   &       the whole string
 *   !       the first char
 *   \\      segment 
 */
void format_str(char *dest, const char *fmt_cnst, const char *str) {
  char *p, *d, *ss = NULL;
  int ps = 0, pe = 0, l, srclen;
  int count, lc;

  if (strchr(fmt_cnst, '&')) {
    strcpy(dest, str);
    return;
  }
  if (strchr(fmt_cnst, '!')) {
    dest[0] = str[0];
    dest[1] = '\0';
    return;
  }

  // segment
  l = strlen(fmt_cnst);
  srclen = strlen(str);
  p = (char *) fmt_cnst;
  lc = 0;
  count = 0;
  while (*p) {
    if (*p == '\\' && lc != '_') {
      if (count == 0) {
        ss = p;
        ps = (int) (p - fmt_cnst);
        count++;
      } else {
        pe = p - fmt_cnst;
        count++;
        break;
      }
    } else if (count) {
      count++;
    }
    lc = *p;
    p++;
  }

  memset(dest, ' ', l - 1);
  dest[l] = '\0';
  d = dest;
  if (ps) {
    memcpy(d, fmt_cnst, ps);
    d += ps;
  }

  /*
   *      convert
   */
  if (ss) {
    int i, j;

    for (i = j = 0; i < count; i++) {
      switch (ss[i]) {
      case '\\':
      case ' ':
        if (j < srclen) {
          d[i] = str[j];
          j++;
        } else
          d[i] = ' ';
        break;
      default:
        d[i] = ss[i];
      }
    }
  }

  // 
  d += count;
  *d = '\0';
  if (*(fmt_cnst + pe + 1) != '\0') {
    strcat(dest, fmt_cnst + pe + 1);
  }
}

/*
 * get numeric format
 */
char *fmt_getnumfmt(char *dest, char *source) {
  int dp = 0, sign = 0, exitf = 0;
  char *p = source;
  char *d = dest;

  while (*p) {
    switch (*p) {
    case '^':
    case '#':
    case '0':
    case ',':
      *d++ = *p;
      break;
    case '-':
    case '+':
      sign++;
      if (sign > 1)
        exitf = 1;
      else
        *d++ = *p;
      break;
    case '.':
      dp++;
      if (dp > 1) {
        exitf = 1;
      } else {
        *d++ = *p;
      }
      break;
    default:
      exitf = 1;
    }

    if (exitf) {
      break;
    }
    p++;
  }

  *d = '\0';
  return p;
}

/*
 * get string format
 */
char *fmt_getstrfmt(char *dest, char *source) {
  char *p = source;
  char *d = dest;

  if (source[0] == '&' || source[0] == '!') {
    *d++ = *source;
    *d++ = '\0';
    return p + 1;
  }

  while (*p) {
    *d++ = *p++;
    if (*p == '\\') {
      *d++ = *p++;
      break;
    }
  }

  *d = '\0';
  return p;
}

/*
 * add format node
 */
void fmt_addfmt(const char *fmt, int type) {
  fmt_node_t *node;

  node = &fmt_stack[fmt_count];
  fmt_count++;
  if (fmt_count >= MAX_FMT_N) {
    panic("Maximum format-node reached");
  }
  node->fmt = malloc(strlen(fmt) + 1);
  strcpy(node->fmt, fmt);
  node->type = type;
}

/*
 * cleanup format-list
 */
void free_format() {
  int i;
  fmt_node_t *node;

  for (i = 0; i < fmt_count; i++) {
    node = &fmt_stack[i];
    free(node->fmt);
  }

  fmt_count = fmt_cur = 0;
}

/*
 * The final format - create the format-list 
 * (that list it will be used later by fmt_printN and fmt_printS)
 *
 * '_' the next character is not belongs to format (simple string)
 */
void build_format(const char *fmt_cnst) {
  char *fmt;
  char *p;
  int nc;
  char buf[1024], *b;

  free_format();

  // backup of format
  fmt = malloc(strlen(fmt_cnst) + 1);
  strcpy(fmt, fmt_cnst);

  p = fmt;
  b = buf;
  nc = 0;
  while (*p) {
    switch (*p) {
    case '_':
      // store prev. buf
      *b = '\0';
      if (strlen(buf)) {
        fmt_addfmt(buf, 0);
      }
      // store the new
      buf[0] = *(p + 1);
      buf[1] = '\0';
      fmt_addfmt(buf, 0);
      b = buf;
      p++;
      break;
    case '-':
    case '+':
    case '^':
    case '0':
    case '#':
      // store prev. buf
      *b = '\0';
      if (strlen(buf)) {
        fmt_addfmt(buf, 0);
      }
      // get num-fmt
      p = fmt_getnumfmt(buf, p);
      fmt_addfmt(buf, 1);
      b = buf;
      nc = 1;
      break;
    case '&':
    case '!':
    case '\\':
      // store prev. buf
      *b = '\0';
      if (strlen(buf)) {
        fmt_addfmt(buf, 0);
      }
      // get str-fmt
      p = fmt_getstrfmt(buf, p);
      fmt_addfmt(buf, 2);
      b = buf;
      nc = 1;
      break;
    default:
      *b++ = *p;
    }

    if (*p) {
      if (nc) {                  // do not advance
        nc = 0;
      } else {
        p++;
      }
    }
  }

  // store prev. buf
  *b = '\0';
  if (strlen(buf)) {
    fmt_addfmt(buf, 0);
  }
  // cleanup
  free(fmt);
}

/*
 * print simple strings (parts of format)
 */
void fmt_printL(int output, int handle) {
  fmt_node_t *node;

  if (fmt_count == 0) {
    return;
  } else {
    do {
      node = &fmt_stack[fmt_cur];
      if (node->type == 0) {
        pv_write(node->fmt, output, handle);
        fmt_cur++;
        if (fmt_cur >= fmt_count) {
          fmt_cur = 0;
        }
      }
    } while (node->type == 0 && fmt_cur != 0);
  }
}

/*
 * print formated number
 */
void fmt_printN(var_num_t x, int output, int handle) {
  fmt_node_t *node;
  char buf[64];

  if (fmt_count == 0) {
    rt_raise(ERR_FORMAT_INVALID_FORMAT);
  } else {
    fmt_printL(output, handle);
    node = &fmt_stack[fmt_cur];
    fmt_cur++;
    if (fmt_cur >= fmt_count)
      fmt_cur = 0;
    if (node->type == 1) {
      format_num(buf, node->fmt, x);
      pv_write(buf, output, handle);
      if (fmt_cur != 0) {
        fmt_printL(output, handle);
      }
    } else {
      rt_raise(ERR_FORMAT_INVALID_FORMAT);
    }
  }
}

/*
 * print formated string
 */
void fmt_printS(const char *str, int output, int handle) {
  fmt_node_t *node;
  char buf[1024];

  if (fmt_count == 0) {
    rt_raise(ERR_FORMAT_INVALID_FORMAT);
  } else {
    fmt_printL(output, handle);
    node = &fmt_stack[fmt_cur];
    fmt_cur++;
    if (fmt_cur >= fmt_count) {
      fmt_cur = 0;
    }
    if (node->type == 2) {
      format_str(buf, node->fmt, str);
      pv_write(buf, output, handle);
      if (fmt_cur != 0) {
        fmt_printL(output, handle);
      }
    } else {
      rt_raise(ERR_FORMAT_INVALID_FORMAT);
    }
  }
}

