// This file is part of SmallBASIC
//
// SmallBasic Variable Manager.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/str.h"
#include "common/var.h"
#include "common/smbas.h"
#include "common/sberr.h"
#include "common/var_map.h"

#include <malloc.h>

#define INT_STR_LEN 64
#define VAR_POOL_SIZE 256

var_t var_pool[VAR_POOL_SIZE];
int next_pool_free = 0;

void v_init_pool() {
  int i;
  next_pool_free = 0;
  for (i = 0; i < VAR_POOL_SIZE; i++) {
    v_init(&var_pool[i]);
    var_pool[i].pooled = 1;
    var_pool[i].attached = 0;
  }
}

/*
 * creates and returns a new variable
 */
var_t *v_new() {
  var_t *result = NULL;
  int i;
  for (i = 0; i < VAR_POOL_SIZE; i++) {
    next_pool_free = (next_pool_free + 1) % VAR_POOL_SIZE;
    if (!var_pool[next_pool_free].attached) {
      result = &var_pool[next_pool_free];
      result->attached = 1;
      break;
    }
  }
  if (!result) {
    result = (var_t *)malloc(sizeof(var_t));
    v_init(result);
    result->pooled = 0;
  }
  return result;
}

void v_new_array(var_t *var, unsigned size) {
  var->type = V_ARRAY;
  var->v.a.size = size;
  var->v.a.data = (var_t *)malloc(sizeof(var_t) * size);
}

void v_detach_array(var_t *var) {
  free(var->v.a.data);
}

/*
 * returns true if the user's program must use this var as an empty var
 * this is usefull for arrays
 */
int v_isempty(var_t *var) {
  switch (var->type) {
  case V_STR:
    return (v_strlen(var) == 0);
  case V_INT:
    return (var->v.i == 0);
  case V_MAP:
    return map_is_empty(var);
  case V_PTR:
    return (var->v.ap.p == 0);
  case V_NUM:
    return (var->v.n == 0.0);
  case V_ARRAY:
    return (var->v.a.size == 0);
  case V_REF:
    return v_isempty(var->v.ref);
  }

  return 1;
}

int v_strlen(const var_t *v) {
  int result;
  if (v->type == V_STR) {
    result = v->v.p.length;
    if (result && v->v.p.ptr[result - 1] == '\0') {
      result--;
    }
  } else {
    result = 0;
  }
  return result;
}

/*
 * returns the length of the variable
 */
int v_length(var_t *var) {
  char tmpsb[INT_STR_LEN];

  switch (var->type) {
  case V_STR:
    return v_strlen(var);
  case V_MAP:
    return map_length(var);
  case V_PTR:
    ltostr(var->v.ap.p, tmpsb);
    return strlen(tmpsb);
  case V_INT:
    ltostr(var->v.i, tmpsb);
    return strlen(tmpsb);
  case V_NUM:
    ftostr(var->v.n, tmpsb);
    return strlen(tmpsb);
  case V_ARRAY:
    return var->v.a.size;
  case V_REF:
    return v_length(var->v.ref);
  }

  return 1;
}

/*
 * return array element pointer
 */
var_t *v_getelemptr(var_t *v, dword index) {
  if (v->type == V_ARRAY) {
    if (index < v->v.a.size) {
      return v_elem(v, index);
    } else {
      err_vararridx(index, v->v.a.size);
      return NULL;
    }
  }
  err_varisnotarray();
  return NULL;
}

/*
 * resize an existing array
 */
void v_resize_array(var_t *v, dword size) {
  var_t *prev;
  dword i;

  if (v->type == V_ARRAY) {
    if ((int)size < 0) {
      err_evargerr();
      return;
    }

    if (size == 0) {
      v_free(v);
      v->type = V_ARRAY;
      v->v.a.size = 0;
      v->v.a.data = NULL;
      v->v.a.ubound[0] = v->v.a.lbound[0] = opt_base;
      v->v.a.maxdim = 1;
    } else if (v->v.a.size > size) {
      // resize down

      // free vars
      for (i = size; i < v->v.a.size; i++) {
        var_t *elem = v_elem(v, i);
        v_free(elem);
      }

      // do not resize array
      prev = NULL;

      // array data
      v->v.a.size = size;
      v->v.a.ubound[0] = v->v.a.lbound[0] + (size - 1);
      v->v.a.maxdim = 1;

      if (prev) {
        free(prev);
      }
    } else if (v->v.a.size < size) {
      // resize up
      // if there is space do not resize
      int prev_size = v->v.a.size;
      if (prev_size == 0) {
        prev = v->v.a.data;
        v_new_array(v, size);
      } else {
        if (prev_size < size) {
          // resize & copy
          prev = v->v.a.data;
          v_new_array(v, size);
          if (prev_size > 0) {
            memcpy(v->v.a.data, prev, prev_size * sizeof(var_t));
          }
        } else {
          prev = NULL;
        }
      }

      // init vars
      for (i = prev_size; i < size; i++) {
        var_t *elem = v_elem(v, i);
        v_init(elem);
      }

      // array data
      v->v.a.size = size;
      v->v.a.ubound[0] = v->v.a.lbound[0] + (size - 1);
      v->v.a.maxdim = 1;

      if (prev) {
        free(prev);
      }
    }
  } else {
    err_varisnotarray();
  }
}

/*
 * create RxC array
 */
void v_tomatrix(var_t *v, int r, int c) {
  int i;

  v_free(v);
  v_new_array(v, r * c);
  for (i = 0; i < r * c; i++) {
    var_t *e = v_elem(v, i);
    v_init(e);
  }

  // array info
  v->v.a.lbound[0] = v->v.a.lbound[1] = opt_base;
  v->v.a.ubound[0] = opt_base + (r - 1);
  v->v.a.ubound[1] = opt_base + (c - 1);
  v->v.a.maxdim = 2;
}

/*
 * create array
 */
void v_toarray1(var_t *v, dword r) {
  dword i;

  v_free(v);
  v->type = V_ARRAY;

  if (r > 0) {
    v_new_array(v, r);
    for (i = 0; i < r; i++) {
      var_t *e = v_elem(v, i);
      v_init(e);
    }

    // array info
    v->v.a.maxdim = 1;
    v->v.a.lbound[0] = opt_base;
    v->v.a.ubound[0] = opt_base + (r - 1);
  } else {
    v->v.a.size = 0;
    v->v.a.data = NULL;
    v->v.a.lbound[0] = v->v.a.ubound[0] = opt_base;
    v->v.a.maxdim = 1;
  }
}

/*
 * returns true if the variable v is not empty (0 for nums)
 */
int v_is_nonzero(var_t *v) {
  switch (v->type) {
  case V_INT:
    return (v->v.i != 0);
  case V_NUM:
    // return (v->v.n != 0.0 && v->v.n != -0.0);
    return (ABS(v->v.n) > 1E-308);
  case V_STR:
    return (v->v.p.length != 0);
  case V_MAP:
    return !map_is_empty(v);
  case V_PTR:
    return (v->v.ap.p != 0);
  case V_ARRAY:
    return (v->v.a.size != 0);
  };
  return 0;
}

/*
 * compare the variable a with the variable b
 * returns
 *   -1      a < b
 *   +1      a > b
 *   0       a = b
 */
int v_compare(var_t *a, var_t *b) {
  var_num_t dt;
  var_int_t di;
  if (a == 0 || b == 0) {
    err_evsyntax();
    return 0;
  }

  if (a->type == V_INT && b->type == V_INT) {
    di = (a->v.i - b->v.i);
    return (di < 0 ? -1 : di > 0 ? 1 : 0);
  } else if ((a->type == V_INT || a->type == V_NUM) &&
             (b->type == V_INT || b->type == V_NUM)) {
    var_num_t left = (a->type == V_NUM) ? a->v.n : a->v.i;
    var_num_t right = (b->type == V_NUM) ? b->v.n : b->v.i;
    if (fabs(left - right) < EPSILON) {
      return 0;
    } else {
      return (left - right) < 0.0 ? -1 : 1;
    }
  }
  if ((a->type == V_STR) && (b->type == V_STR)) {
    return strcmp(a->v.p.ptr, b->v.p.ptr);
  }
  if ((a->type == V_STR) && (b->type == V_NUM)) {
    if (a->v.p.ptr[0] == '\0' || is_number(a->v.p.ptr)) {
      // compare nums
      dt = v_getval(a);
      return (dt < b->v.n) ? -1 : ((dt == b->v.n) ? 0 : 1);
    }
    return 1;
  }
  if ((a->type == V_NUM) && (b->type == V_STR)) {
    if (b->v.p.ptr[0] == '\0' || is_number(b->v.p.ptr)) {
      // compare nums
      dt = v_getval(b);
      return (dt < a->v.n) ? 1 : ((dt == a->v.n) ? 0 : -1);
    }
    return - 1;
  }
  if ((a->type == V_STR) && (b->type == V_INT)) {
    if (a->v.p.ptr[0] == '\0' || is_number(a->v.p.ptr)) {
      // compare nums
      di = v_igetval(a);
      return (di < b->v.i) ? -1 : ((di == b->v.i) ? 0 : 1);
    }
    return 1;
  }
  if ((a->type == V_INT) && (b->type == V_STR)) {
    if (b->v.p.ptr[0] == '\0' || is_number(b->v.p.ptr)) {
      // compare nums
      di = v_igetval(b);
      return (di < a->v.i) ? 1 : ((di == a->v.i) ? 0 : -1);
    }
    return - 1;
  }
  if ((a->type == V_ARRAY) && (b->type == V_ARRAY)) {
    // check size
    if (a->v.a.size != b->v.a.size) {
      if (a->v.a.size < b->v.a.size) {
        return -1;
      }
      return 1;
    }
    // check every element
    int i, ci;
    for (i = 0; i < a->v.a.size; i++) {
      var_t *ea = v_elem(a, i);
      var_t *eb = v_elem(b, i);
      if ((ci = v_compare(ea, eb)) != 0) {
        return ci;
      }
    }
    // equal
    return 0;
  }

  if (a->type == V_MAP && b->type == V_MAP) {
    return map_compare(a, b);
  }

  err_evtype();
  return 1;
}

/*
 * add two variables
 * result = a + b
 */
void v_add(var_t *result, var_t *a, var_t *b) {
  char tmpsb[INT_STR_LEN];

  if (a->type == V_STR && b->type == V_STR) {
    int length = strlen(a->v.p.ptr) + strlen(b->v.p.ptr);
    result->type = V_STR;
    result->v.p.ptr = malloc(length + 1);
    strcpy(result->v.p.ptr, a->v.p.ptr);
    strcat(result->v.p.ptr, b->v.p.ptr);
    result->v.p.ptr[length] = '\0';
    result->v.p.length = length + 1;
    return;
  } else if (a->type == V_INT && b->type == V_INT) {
    result->type = V_INT;
    result->v.i = a->v.i + b->v.i;
    return;
  } else if (a->type == V_NUM && b->type == V_NUM) {
    result->type = V_NUM;
    result->v.n = a->v.n + b->v.n;
    return;
  } else if (a->type == V_NUM && b->type == V_INT) {
    result->type = V_NUM;
    result->v.n = a->v.n + b->v.i;
    return;
  } else if (a->type == V_INT && b->type == V_NUM) {
    result->type = V_NUM;
    result->v.n = a->v.i + b->v.n;
    return;
  } else if (a->type == V_STR && (b->type == V_INT || b->type == V_NUM)) {
    if (is_number(a->v.p.ptr)) {
      result->type = V_NUM;
      if (b->type == V_INT) {
        result->v.n = b->v.i + v_getval(a);
      } else {
        result->v.n = b->v.n + v_getval(a);
      }
    } else {
      result->type = V_STR;
      result->v.p.ptr = (char *)malloc(strlen(a->v.p.ptr) + INT_STR_LEN);
      strcpy(result->v.p.ptr, a->v.p.ptr);
      if (b->type == V_INT) {
        ltostr(b->v.i, tmpsb);
      } else {
        ftostr(b->v.n, tmpsb);
      }
      strcat(result->v.p.ptr, tmpsb);
      result->v.p.length = strlen(result->v.p.ptr) + 1;
    }
  } else if ((a->type == V_INT || a->type == V_NUM) && b->type == V_STR) {
    if (is_number(b->v.p.ptr)) {
      result->type = V_NUM;
      if (a->type == V_INT) {
        result->v.n = a->v.i + v_getval(b);
      } else {
        result->v.n = a->v.n + v_getval(b);
      }
    } else {
      result->type = V_STR;
      result->v.p.ptr = (char *)malloc(strlen(b->v.p.ptr) + INT_STR_LEN);
      if (a->type == V_INT) {
        ltostr(a->v.i, tmpsb);
      } else {
        ftostr(a->v.n, tmpsb);
      }
      strcpy(result->v.p.ptr, tmpsb);
      strcat(result->v.p.ptr, b->v.p.ptr);
      result->v.p.length = strlen(result->v.p.ptr) + 1;
    }
  } else if (b->type == V_MAP) {
    char *map = map_to_str(b);
    v_set(result, a);
    v_strcat(result, map);
    free(map);
  }
}

/*
 * assign (dest = src)
 */
void v_set(var_t *dest, const var_t *src) {
  v_free(dest);
  dest->const_flag = 0;
  dest->type = src->type;

  switch (src->type) {
  case V_INT:
    dest->v.i = src->v.i;
    break;
  case V_STR:
    dest->v.p.length = v_strlen(src) + 1;
    dest->v.p.ptr = (char *)malloc(dest->v.p.length);
    strcpy(dest->v.p.ptr, src->v.p.ptr);
    break;
  case V_NUM:
    dest->v.n = src->v.n;
    break;
  case V_MAP:
    map_set(dest, (const var_p_t)src);
    break;
  case V_PTR:
    dest->v.ap.p = src->v.ap.p;
    dest->v.ap.v = src->v.ap.v;
    break;
  case V_REF:
    dest->v.ref = src->v.ref;
    break;
  case V_FUNC:
    dest->v.fn.cb = src->v.fn.cb;
    dest->v.fn.self = src->v.fn.self;
    break;
  case V_ARRAY:
    if (src->v.a.size) {
      memcpy(&dest->v.a, &src->v.a, sizeof(src->v.a));
      v_new_array(dest, src->v.a.size);

      // copy each element
      int i;
      for (i = 0; i < src->v.a.size; i++) {
        var_t *src_vp = v_elem(src, i);
        var_t *dest_vp = v_elem(dest, i);
        v_init(dest_vp);
        v_set(dest_vp, src_vp);
      }
    } else {
      dest->v.a.size = 0;
      dest->v.a.data = NULL;
      dest->v.a.ubound[0] = dest->v.a.lbound[0] = opt_base;
      dest->v.a.maxdim = 1;
    }
    break;
  }
}

/*
 * return a full copy of the 'source'
 */
var_t *v_clone(const var_t *source) {
  var_t *vnew = v_new();
  v_set(vnew, source);
  return vnew;
}

/*
 * add b to a
 */
void v_inc(var_t *a, var_t *b) {
  if (a->type == V_INT && b->type == V_INT) {
    a->v.i += b->v.i;
  } else if (a->type == V_NUM && b->type == V_NUM) {
    a->v.n += b->v.n;
  } else if (a->type == V_NUM && b->type == V_INT) {
    a->v.n += b->v.i;
  } else if (a->type == V_INT && b->type == V_NUM) {
    a->type = V_NUM;
    a->v.n = a->v.i + b->v.n;
  } else {
    err_varnotnum();
  }
}

/*
 */
int v_sign(var_t *x) {
  if (x->type == V_INT) {
    return (x->v.i < 0) ? -1 : ((x->v.i == 0) ? 0 : 1);
  } else if (x->type == V_NUM) {
    return (x->v.n < 0) ? -1 : ((x->v.n == 0) ? 0 : 1);
  }
  err_varnotnum();
  return 0;
}

/*
 * setup a string variable
 */
void v_createstr(var_t *v, const char *src) {
  int l = strlen(src) + 1;
  v->type = V_STR;
  v->v.p.ptr = malloc(l);
  v->v.p.length = l;
  strcpy(v->v.p.ptr, src);
}

/*
 * returns the string representation of the variable
 */
char *v_str(var_t *arg) {
  char *buffer;
  switch (arg->type) {
  case V_INT:
    buffer = malloc(INT_STR_LEN);
    ltostr(arg->v.i, buffer);
    break;
  case V_NUM:
    buffer = malloc(INT_STR_LEN);
    ftostr(arg->v.n, buffer);
    break;
  case V_STR:
    buffer = strdup(arg->v.p.ptr);
    break;
  case V_ARRAY:
  case V_MAP:
    buffer = map_to_str(arg);
    break;
  case V_FUNC:
  case V_PTR:
    buffer = malloc(5);
    strcpy(buffer, "func");
    break;
  default:
    buffer = malloc(1);
    buffer[0] = '\0';
    break;
  }
  return buffer;
}

/*
 * converts the variable to string-variable
 */
void v_tostr(var_t *arg) {
  if (arg->type != V_STR) {
    char *tmp = v_str(arg);
    int len = strlen(tmp) + 1;
    arg->type = V_STR;
    arg->v.p.ptr = malloc(len);
    arg->v.p.length = len;
    strcpy(arg->v.p.ptr, tmp);
    free(tmp);
  }
}

/*
 * set the value of 'var' to string
 */
void v_setstr(var_t *var, const char *string) {
  if (var->type != V_STR || strcmp(string, var->v.p.ptr) != 0) {
    v_free(var);
    var->type = V_STR;
    var->v.p.length = strlen(string) + 1;
    var->v.p.ptr = malloc(var->v.p.length);
    strcpy(var->v.p.ptr, string);
  }
}

void v_setstrn(var_t *var, const char *string, int len) {
  if (var->type != V_STR || strncmp(string, var->v.p.ptr, len) != 0) {
    v_free(var);
    var->type = V_STR;
    var->v.p.length = len + 1;
    var->v.p.ptr = malloc(var->v.p.length);
    strncpy(var->v.p.ptr, string, len);
    var->v.p.ptr[len] = '\0';
  }
}

/*
 * adds a string to current string value
 */
void v_strcat(var_t *var, const char *string) {
  if (var->type == V_INT || var->type == V_NUM) {
    v_tostr(var);
  }
  if (var->type == V_STR) {
    var->v.p.length = strlen(var->v.p.ptr) + strlen(string) + 1;
    var->v.p.ptr = realloc(var->v.p.ptr, var->v.p.length);
    strcat(var->v.p.ptr, string);
  } else {
    err_typemismatch();
  }
}

/*
 * set the value of 'var' to n
 */
void v_setreal(var_t *var, var_num_t n) {
  v_free(var);
  var->type = V_NUM;
  var->v.n = n;
}

/*
 * set the value of 'var' to i
 */
void v_setint(var_t *var, var_int_t i) {
  v_free(var);
  var->type = V_INT;
  var->v.i = i;
}

/*
 * return the string value of 'var'
 */
char *v_getstr(var_t *var) {
  if (var->type != V_STR) {
    v_tostr(var);
  }
  return var->v.p.ptr;
}

/*
 * set an empty string
 */
void v_zerostr(var_t *r) {
  v_free(r);
  r->type = V_STR;
  r->v.p.ptr = malloc(1);
  r->v.p.ptr[0] = '\0';
  r->v.p.length = 1;
}

/*
 * convert's a user's string to variable
 *
 * its decides in what format to store the value
 * its used mostly by 'input' functions
 */
void v_input2var(const char *str, var_t *var) {
  v_free(var);

  if (strlen(str) == 0) {
    // no data
    v_setstr(var, str);
  } else {
    char *np, *sb;
    char buf[INT_STR_LEN];
    int type;
    var_int_t lv;
    var_num_t dv;

    sb = strdup(str);
    np = get_numexpr(sb, buf, &type, &lv, &dv);

    if (type == 1 && *np == '\0') {
      v_setint(var, lv);
    } else if (type == 2 && *np == '\0') {
      v_setreal(var, dv);
    } else {
      v_setstr(var, str);
    }
    free(sb);
  }
}

