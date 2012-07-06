// This file is part of SmallBASIC
//
// strings
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/str.h"
#include "common/panic.h"
#include "common/fmt.h"

/**
 * removes spaces and returns a new string
 */
char *trimdup(const char *str) {
  char *buf;
  char *p;

  buf = tmp_alloc(strlen(str) + 1);
  strcpy(buf, str);

  if (*str == '\0') {
    return buf;
  }

  p = (char *) str;
  while (is_wspace(*p)) {
    p++;
  }
  strcpy(buf, p);

  if (*p != '\0') {
    p = buf;
    while (*p) {
      p++;
    }
    p--;
    while (p > buf && is_wspace(*p)) {
      p--;
    }
    p++;
    *p = '\0';
  }
  return buf;
}

/**
 * removes spaces
 */
void str_alltrim(char *str) {
  char *buf;

  buf = trimdup(str);
  strcpy(str, buf);
  tmp_free(buf);
}

/**
 * caseless string compare
 */
int strcaseless(const char *s1, const char *s2) {
  const char *p1 = s1;
  const char *p2 = s2;
  int ch1, ch2;

  while (*p1) {
    if (*p1 == '\0' && *p2 != '\0') {
      return -1;
    }
    if (*p2 == '\0' && *p1 != '\0') {
      return 1;
    }
    ch1 = to_upper(*p1);
    ch2 = to_upper(*p2);
    if (ch1 < ch2) {
      return -1;
    }
    if (ch1 > ch2) {
      return 1;
    }
    p1++;
    p2++;
  }
  return 0;
}

/**
 * caseless string compare
 */
int strcaselessn(const char *s1, const char *s2, int len) {
  const char *p1 = s1;
  const char *p2 = s2;
  int ch1, ch2, count;

  count = 0;
  while (*p1) {
    if (count == len) {
      return 0;
    }
    if (*p1 == '\0' && *p2 != '\0') {
      return -1;
    }
    if (*p2 == '\0' && *p1 != '\0') {
      return 1;
    }
    ch1 = to_upper(*p1);
    ch2 = to_upper(*p2);
    if (ch1 < ch2) {
      return -1;
    }
    if (ch1 > ch2) {
      return 1;
    }
    p1++;
    p2++;
    count++;
  }
  return 0;
}

/**
 * strstr with ignore case
 */
char *stristr(const char *s1, const char *s2) {
  char *p;
  int l2;

  p = (char *) s1;
  l2 = strlen(s2);
  while (*p) {
    if (strcaselessn(p, s2, l2) == 0)
      return p;
    p++;
  }
  return NULL;
}

/**
 *
 */
char *transdup(const char *src, const char *what, const char *with) {
  char *p = (char *) src;
  char *dest, *d;
  int lwhat, lwith, size, len;

  lwhat = strlen(what);
  lwith = strlen(with);

  size = 256;
  dest = tmp_alloc(size);
  d = dest;
  *d = '\0';

  while (*p) {
    if (strncmp(p, what, lwhat) == 0) {
      if ((d - dest) + lwith >= size - 1) {
        len = d - dest;
        size += 256;
        dest = tmp_realloc(dest, size);
        d = dest + len;
      }

      memcpy(d, with, lwith);
      d += lwith;
      p += (lwhat - 1);
    } else {
      if ((d - dest) + 1 >= size - 1) {
        len = d - dest;
        size += 256;
        dest = tmp_realloc(dest, size);
        d = dest + len;
      }

      *d = *p;
      d++;
    }
    p++;
  }

  *d = '\0';
  return dest;
}

/*
 *  strstr with support for quotes
 */
char *q_strstr(const char *s1, const char *s2, const char *pairs) {
  char *p, *z;
  int l2;
  int wait_q, open_q, level_q;

  p = (char *) s1;
  l2 = strlen(s2);
  wait_q = open_q = level_q = 0;

  while (*p) {

    if (*p == wait_q) {         // i am waiting that. level down
      level_q--;
      if (level_q <= 0) {       // level = 0
        level_q = 0;
        wait_q = 0;
      }
    } else if ((z = strchr(pairs, *p)) != NULL) { // character is a
      // delimiter;
      // level up
      open_q = ((z - pairs) + 1) % 2; // true, if its a 'begin'
      // delimiter

      if (wait_q && open_q) {
        if (*(z + 1) == wait_q) // open_q of our pair?
          level_q++;            // increase level
      } else if (wait_q)
        ;         // do nothing, I am waitting something
      // else
      else {                    // its a new section
        if (open_q) {
          level_q++;            // level = 1
          wait_q = *(z + 1);    // what to wait for
        }
      }
    } else if (wait_q == 0) {     // it is a regular character
      if (strncmp(p, s2, l2) == 0)
        return p;
    }
    // next
    p++;
  }

  return NULL;
}

/**
 *
 */
int is_alpha(int ch) {
  if (ch == 0)
    return 0;
  if ((ch > 64 && ch < 91) || (ch > 96 && ch < 123))
    return -1;
  // return
  // (strchr("_���������������������������������������������������������", ch)
  // != NULL); // Greek
  return (ch & 0x80);           // +foreign
}

/**
 * returns true if the ch is alphanumeric
 */
int is_alnum(int ch) {
  if (ch == 0) {
    return 0;
  }
  return (is_alpha(ch) || is_digit(ch));
}

/**
 * returns true if the ch is an 'empty' character
 */
int is_space(int ch) {
  return (ch == ' ' || ch == '\t' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\v') ? -1 : 0;
}

/**
 * returns true if 'text' contains digits only
 */
int is_all_digits(const char *text) {
  const char *p = text;

  if (p == NULL) {
    return 0;
  }
  if (*p == '\0') {
    return 0;
  }
  while (*p) {
    if (!is_digit(*p)) {
      return 0;
    }
    p++;
  }
  return 1;
}

/**
 * returns true if the text is keyword
 */
int is_keyword(const char *name) {
  char *p = (char *) name;

  if (p == NULL) {
    return 0;
  }
  if (is_alpha(name[0])) {
    while (is_alnum(*p) || (*p == '_')) {
      p++;
    }
    return (*p == '\0');
  }
  return 0;
}

/**
 * converts the 'str' string to uppercase
 */
char *strupper(char *str) {
  char *p = str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    *p = to_upper(*p);
    p++;
  }
  return str;
}

/**
 * converts the 'str' string to lowercase
 */
char *strlower(char *str) {
  char *p = str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    *p = to_lower(*p);
    p++;
  }
  return str;
}

/**
 *
 */
char *get_keyword(char *text, char *dest) {
  char *p = (char *) text;
  char *d = dest;

  if (p == NULL) {
    *dest = '\0';
    return 0;
  }

  while (is_space(*p)) {
    p++;
  }
  while (is_alnum(*p) || (*p == '_')) {
    *d = to_upper(*p);
    d++;
    p++;
  }

  // Code to kill the $
  // if ( *p == '$' ) 
  // p ++;

  if (*p == '$') {
    *d++ = *p++;
  }
  *d = '\0';
  while (is_space(*p)) {
    p++;
  }
  // special case, something wrong, jump to next char
  if (p == text) {
    *dest = *p;
    *(dest + 1) = '\0';
    p++;
  }

  return p;
}

/**
 * Returns the number of a string (constant numeric expression)
 *
 * type  <=0 = error
 *         1 = int32
 *         2 = double
 *
 * Warning: octals are different from C (QB compatibility: 009 = 9)
 */
char *get_numexpr(char *text, char *dest, int *type, var_int_t *lv, var_num_t *dv) {
  char *p = (char *) text;
  char *d = dest;
  char *epos = NULL, *epos_on_text = NULL;
  byte base = 10;
  byte dpc = 0, stupid_e_fmt = 0, eop = '+';
  int sign = 1;
  var_num_t power = 1.0;
  var_num_t num;

  *type = 0;
  *lv = 0;
  *dv = 0.0;

  if (p == NULL) {
    *dest = '\0';
    return NULL;
  }
  // spaces
  while (is_space(*p)) {
    p++;
  }
  // sign
  if ((*p == '-' || *p == '+') && strchr("0123456789.", *(p + 1)) &&
  *(p + 1) != '\0'){if (*p == '-') {
    sign = -1;
  }
  p++;                        // don't copy it
}
  // 
  // resolve the base (hex, octal and binary)
  // 
  if ((*p == '&') || (*p == '0' && (*(p + 1) != '\0' && strchr("HXBO", to_upper(*(p + 1))) != NULL))) {

    p++;
    switch (*p) {
    case 'H':
    case 'h':
    case 'X':
    case 'x':
      base = 16;
      break;
    case 'O':
    case 'o':
      base = 8;
      break;
    case 'B':
    case 'b':
      base = 2;
      break;
    default:
      *type = -1;
      return p;                 // Unknown base
    }

    p++;
  }
  // 
  // copy parts of number
  // 
  if (base == 16) {
    // copy hex
    while (is_hexdigit(*p)) {
      *d = to_upper(*p);
      d++;
      p++;
    }
  } else if (base != 10) {
    // copy octal | bin
    while (is_digit(*p))
      *d++ = *p++;
  } else if (is_digit(*p) || *p == '.') {
    // copy number (first part)
    while (is_digit(*p) || *p == '.') {
      if (*p == '.') {
        dpc++;
        if (dpc > 1) {
          *type = -2;           // DP ERROR
          break;
        }
      }

      *d++ = *p++;
    }

    // check second part
    if ((*p == 'E' || *p == 'e') && (*type == 0)) {
      epos = d;
      epos_on_text = p;
      *d++ = *p++;              // E

      if (*p == '+' || *p == '-' || is_digit(*p) || *p == '.') {
        dpc = 0;

        // copy second part (power)
        if (*p == '+' || *p == '-') {
          *d++ = *p++;
          if (strchr("+-*/\\^", *p) != 0) { // stupid E format 
            // (1E--9 ||
            // 1E++9)
            stupid_e_fmt = 1;
            eop = *p;
            *d++ = *p++;
            if (*p == '+' || *p == '-')
              *d++ = *p++;
          }
        }
        // power
        while (is_digit(*p) || *p == '.') {
          if (*p == '.') {
            dpc++;
            if (dpc > 1) {
              *type = -4;       // DP ERROR (second part)
              break;
            }
          }
          *d++ = *p++;
        }                       // after E

      }                         // 
      else {
        *type = -3;             // E+- ERROR
      }
    }
  } else
    *type = -9;                 // NOT A NUMBER

  *d = '\0';

  // 
  // finaly, calculate the number
  // 
  if (*type == 0) {
    switch (base) {
    case 10:
      if (dpc || (epos != NULL) || (strlen(dest) > 8)) {
        *type = 2;              // double
        if (epos) {
          if (stupid_e_fmt) {
            int r_type = 1;

            *epos = '\0';
            num = sb_strtof(dest) * ((double) sign);
            *epos = 'E';        // restore E

            /*
             * if ( *p == 'E' || *p == 'e' ) { long r_lv; double r_dv;
             * 
             * p = get_numexpr(epos_on_text+3, dest, &r_type, &r_lv, &r_dv);
             * 
             * switch ( r_type ) { case 1: power = r_lv; break; case 2: power = 
             * r_dv; break; default: // error *type = r_type; } } else 
             */
            power = sb_strtof(epos + 3);

            if (r_type > 0) {
              switch (eop) {
              case '+':
                *dv = num + power;
                break;
              case '-':
                *dv = num - power;
                break;
              case '*':
                *dv = num * power;
                break;
              case '/':
                if (ABS(power) != 0.0)
                  *dv = num / power;
                else
                  *dv = 0;
                // else if(comp) sc_raise() else rt_raise
                break;
              case '\\':
                if ((long) power != 0) {
                  *type = 1;
                  *lv = num / (long) power;
                } else {
                  *type = 1;
                  *lv = 0;
                }
                // else if(comp) sc_raise() else rt_raise
                break;
              case '^':
                *dv = pow(num, power);
                break;
              }
            }
          } else {
            *epos = '\0';
            power = pow(10, sb_strtof(epos + 1));
            *dv = sb_strtof(dest) * ((double) sign) * power;
            *epos = 'E';
          }
        } else {
          *dv = sb_strtof(dest) * ((double) sign);
        }
      } else {
        // dpc = 0 && epos = 0
        *type = 1;              // int32
        *lv = xstrtol(dest) * sign;
      }
      break;
    case 16:
      *type = 1;                // int32
      *lv = hextol(dest);
      break;
    case 8:
      *type = 1;                // int32
      *lv = octtol(dest);
      break;
    case 2:
      *type = 1;                // int32
      *lv = bintol(dest);
      break;
    }
  }
  // 
  if (is_alpha(*p))
    *type = -9;                 // ITS NOT A NUMBER
  while (is_space(*p))
    p++;
  return p;
}

/**
 *
 */
var_num_t numexpr_sb_strtof(char *source) {
  char buf[256], *np;
  int type;
  var_int_t lv;
  var_num_t dv;

  np = get_numexpr(source, buf, &type, &lv, &dv);

  if (type == 1 && *np == '\0') {
    return (var_num_t) lv;
  } else if (type == 2 && *np == '\0') {
    return dv;
  }
  return 0.0;
}

/**
 *
 */
var_int_t numexpr_strtol(char *source) {
  char buf[256], *np;
  int type;
  var_int_t lv;
  var_num_t dv;

  np = get_numexpr(source, buf, &type, &lv, &dv);

  if (type == 1 && *np == '\0') {
    return lv;
  } else if (type == 2 && *np == '\0') {
    return (var_int_t) dv;
  }
  return 0;
}

/**
 * convertion: binary to decimal
 */
long bintol(const char *str) {
  long r = 0;
  char *p = (char *) str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    if (*p == 48 || *p == 49)   // 01
      r = (r << 1) + ((*p) - 48);
    p++;
  }
  return r;
}

/**
 * convertion: octal to decimal
 */
long octtol(const char *str) {
  long r = 0;
  char *p = (char *) str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    if (*p >= 48 && *p <= 55)   // 01234567
      r = (r << 3) + ((*p) - 48);
    p++;
  }
  return r;
}

/**
 * convertion: hexadecimal to decimal
 */
long hextol(const char *str) {
  long r = 0;
  char *p = (char *) str;

  if (p == NULL) {
    return 0;
  }
  while (*p) {
    if (is_digit(*p))           // 0123456789
      r = (r << 4) + ((*p) - 48);
    else if (*p >= 65 && *p <= 70)  // ABCDEF
      r = (r << 4) + ((*p) - 55);
    else if (*p >= 97 && *p <= 102) // abcdef
      r = (r << 4) + ((*p) - 87);
    p++;
  }
  return r;
}

/**
 * string to double
 */
var_num_t sb_strtof(const char *str) {
  char *p = (char *) str;
  var_num_t r = 0.0;
  var_num_t d = 10.0;
  var_num_t sign = 1;
  int decp = 0;

  if (p == NULL) {
    return 0;
  }
  if (*p == '-') {
    sign = -1;
    p++;
  } else if (*p == '+')
    p++;

  while (*p) {
    if (is_digit(*p)) {
      if (!decp)
        r = (r * 10) + ((*p) - 48);
      else {
        r += (((*p) - 48) * 1 / d);
        d *= 10;
      }
    } else if (*p == '.')
      decp = 1;
    else if (*p == ' ')
      break;
    else {
      r = 0;
      break;
    }

    p++;
  }

  return r * sign;
}

/**
 *
 */
long xstrtol(const char *str) {
  if (str == NULL) {
    return 0;
  }
  return atoi(str);
}

/**
 *
 */
int is_number(const char *str) {
  char *p = (char *) str;
  int dpc = 0, cnt = 0;

  if (str == NULL) {
    return 0;
  }
  if (*p == '+' || *p == '-')
    p++;

  while (*p) {
    if (strchr("0123456789.", *p) == NULL
      )
      return 0;
    else
      cnt++;
    if (*p == '.') {
      dpc++;
      if (dpc > 1)
        return 0;
    }
    p++;
  }

  if (cnt)
    return 1;
  return 0;
}

/**
 * double to string
 */
char *ftostr(var_num_t num, char *dest) {
  bestfta(num, dest);
  return dest;
}

/**
 *
 */
char *ltostr(var_int_t num, char *dest) {
  if (dest == NULL) {
    panic("l2s(..,null)");
  }
  sprintf(dest, VAR_INT_FMT, num);
  return dest;
}

/**
 * newdir must ends with dirsep
 * new_ext must starts with .
 */
char *chgfilename(char *dest, char *source, char *newdir, char *prefix, char *new_ext, char *suffix) {
  char *plast_dir;
  char *plast_point;

  dest[0] = '\0';
  plast_dir = strrchr(source, OS_DIRSEP);
  if (!plast_dir) {
    plast_dir = source;
  } else {
    plast_dir++;
  }
  plast_point = strrchr(source, '.');

  if (newdir) {
    strcat(dest, newdir);
  }
  if (prefix) {
    strcat(dest, prefix);
  }
  if (new_ext) {
    *plast_point = '\0';
  }
  strcat(dest, plast_dir);

  if (suffix) {
    strcat(dest, suffix);
  }
  if (new_ext) {
    strcat(dest, new_ext);
    *plast_point = '.';
  }
  return dest;
}

/**
 *
 */
char *xbasename(char *dest, const char *source) {
  char *p;

  p = strrchr(source, OS_DIRSEP);
  if (!p)
    p = (char *) source;
  else
    p++;

  strcpy(dest, p);
  return dest;
}

/**
 *
 */
int is_wspace(int c) {
  return (c != 0 && strchr(" \t\n\r\v\f", c));
}

/**
 * squeeze (& strdup)
 */
char *sqzdup(const char *source) {
  char *rp, *p, *d;
  int lc = 0;

  rp = tmp_alloc(strlen(source) + 1);
  p = (char *) source;
  d = rp;

  while (*p != '\0' && is_wspace(*p))
    p++;

  while (*p) {
    if (is_wspace(*p)) {
      if (!lc) {
        lc = 1;
        if (p > source) {
          if (is_alpha(*(p - 1)) || is_digit(*(p - 1)))
            *d++ = ' ';
          else {
            char *nc;

            nc = p;
            while (*nc != '\0' && is_wspace(*nc))
              nc++;
            if (is_alpha(*nc) || is_digit(*nc)
              )
              *d++ = ' ';
          }
        }
      }
    } else
      (lc = 0, *d++ = *p);

    p++;
  }

  *d = '\0';
  if (d > rp) {
    if (is_wspace(*(d - 1)))
      *(d - 1) = '\0';
  }

  return rp;
}

/**
 * enclose, returns a newly created string
 */
char *encldup(const char *source, const char *pairs) {
  char *rp;
  int l;

  l = strlen(source);
  rp = tmp_alloc(l + 3);
  memcpy(rp + 1, source, l);
  *(rp) = pairs[0];
  if (pairs[1])
    *(rp + l + 1) = pairs[1];
  else
    *(rp + l + 1) = pairs[0];
  *(rp + l + 2) = '\0';

  return rp;
}

/**
 * disclose, returns a newly created string
 */
char *discldup(const char *source, const char *pairs, const char *ignpairs) {
  char *rp, *np, *r, *p, *z;
  int wait_p = 0, open_p = 0, level_p = 0;
  int wait_q = 0, open_q = 0, level_q = 0;
  int record = 0;

  rp = tmp_strdup(source);
  r = rp;

  p = (char *) source;
  while (*p) {

    // ignore pairs
    if (*p == wait_q) {         // ignore pair - level down
      level_q--;
      if (level_q <= 0) {
        level_q = 0;
        wait_q = 0;
      }
    } else if ((z = strchr(ignpairs, *p)) != NULL) {
      open_q = ((z - ignpairs) + 1) % 2;

      if (wait_q && open_q) {
        if (*(z + 1) == wait_q) // open_q of our pair?
          level_q++;
      } else if (wait_q)
        ;         // do nothing, I am waitting something
      // else
      else {                    // new pair
        if (open_q) {
          level_q++;
          wait_q = *(z + 1);
        }
      }
    }
    // primary pairs
    else if (*p == wait_p && wait_q == 0) { // primary pair - level
      // down
      level_p--;
      if (level_p <= 0) {
        // store and exit
        record = 0;
        break;
      }
    } else if ((z = strchr(pairs, *p)) != NULL && wait_q == 0) {
      open_p = ((z - pairs) + 1) % 2;

      if (wait_p && open_p) {
        if (*(z + 1) == wait_p) // open_q of our pair?
          level_p++;
      } else if (wait_p)
        ;         // do nothing, I am waitting something
      // else
      else {                    // new pair
        if (open_p) {
          level_p++;
          wait_p = *(z + 1);
          record = 1;
        }
      }
    }
    // next
    if (record == 1)            // ignore the first
      record++;
    else if (record == 2)
      *r++ = *p;

    p++;
  }

  *r = '\0';
  np = tmp_strdup(rp);          // actually, resize down
  tmp_free(rp);

  return np;
}

/**
 * C-Style control codes
 */
char *cstrdup(const char *source) {
  char *buf, *p, *d;

  buf = tmp_alloc(strlen(source) + 1);
  p = (char *) source;
  d = buf;
  while (*p) {
    if (*p == '\\') {
      p++;
      switch (*p) {
      case 'e':
        *d++ = '\033';
        break;
      case 'v':
        *d++ = '\v';
        break;
      case 't':
        *d++ = '\t';
        break;
      case 'r':
        *d++ = '\r';
        break;
      case 'n':
        *d++ = '\n';
        break;
      case 'b':
        *d++ = '\b';
        break;
      case '\'':
        *d++ = '\'';
        break;
      case '\"':
        *d++ = '\"';
        break;
      case 'a':
        *d++ = '\a';
        break;
      case 'f':
        *d++ = '\f';
        break;
      case '\\':
        *d++ = '\\';
        break;
      case 'x':                // hex
        if (is_hexdigit(*(p + 1)) && is_hexdigit(*(p + 2))) {
          int c = 0;

          p++;
          if (is_digit(*p))
            c |= ((*p - '0') << 4);
          else
            c |= (((to_upper(*p) - 'A') + 10) << 4);
          p++;
          if (is_digit(*p))
            c |= *p - '0';
          else
            c |= (to_upper(*p) - 'A') + 10;

          *d++ = c;
        } else
          *d++ = '\0';
        break;
      case '0':                // oct
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
        if (is_octdigit(*(p + 1)) && is_octdigit(*(p + 2))) {
          int c = 0;

          c |= ((*p - '0') << 6);
          p++;
          c |= ((*p - '0') << 3);
          p++;
          c |= (*p - '0');

          *d++ = c;
        } else
          *d++ = '\0';
        break;

      default:
        *d++ = *p;
      }

      p++;
    } else
      *d++ = *p++;
  }

  *d = '\0';
  return buf;
}

/**
 * basic-string to c-string convertion
 */
char *bstrdup(const char *source) {
  char *buf, *p, *d;

  buf = tmp_alloc(strlen(source) * 4 + 1);
  p = (char *) source;
  d = buf;
  while (*p) {
    if (*p < 32 && *p >= 0) {
      switch (*p) {
      case '\033':
        *d++ = '\\';
        *d++ = 'e';
        break;
      case '\v':
        *d++ = '\\';
        *d++ = 'v';
        break;
      case '\t':
        *d++ = '\\';
        *d++ = 't';
        break;
      case '\r':
        *d++ = '\\';
        *d++ = 'r';
        break;
      case '\n':
        *d++ = '\\';
        *d++ = 'n';
        break;
      case '\b':
        *d++ = '\\';
        *d++ = 'b';
        break;
      case '\'':
        *d++ = '\\';
        *d++ = '\'';
        break;
      case '\"':
        *d++ = '\\';
        *d++ = '\"';
        break;
      case '\a':
        *d++ = '\\';
        *d++ = 'a';
        break;
      case '\f':
        *d++ = '\\';
        *d++ = 'f';
        break;
      case '\\':
        *d++ = '\\';
        *d++ = '\\';
        break;
      default:
        *d++ = '\\';
        *d++ = 'x';
        *d++ = to_hexdigit((*p & 0xF0) >> 4);
        *d++ = to_hexdigit(*p & 0xF);
      }

      p++;
    } else
      *d++ = *p++;
  }

  *d = '\0';
  return buf;
}

/**
 *
 */
const char *baseof(const char *source, int delim) {
  const char *p;

  p = strrchr(source, delim);
  if (p) {
    return p + 1;
  }
  return source;
}

/**
 *
 */
char char_table_replace(const char *what_table, int ch, const char *replace_table) {
  const char *p;

  p = strchr(what_table, ch);
  if (!p) {
    return ch;
  }
  return *(replace_table + (p - what_table));
}
