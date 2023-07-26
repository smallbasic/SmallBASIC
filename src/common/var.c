// This file is part of SmallBASIC
//
// SmallBASIC variable manager.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/sberr.h"

#define INT_STR_LEN 64
#define VAR_POOL_SIZE 8192

var_t var_pool[VAR_POOL_SIZE];
var_t *var_pool_head;

void v_init_pool() {
  for (uint32_t i = 0; i < VAR_POOL_SIZE; i++) {
    v_init(&var_pool[i]);
    var_pool[i].pooled = 1;
    if (i + 1 < VAR_POOL_SIZE) {
      var_pool[i].v.pool_next = &var_pool[i + 1];
    } else {
      var_pool[i].v.pool_next = NULL;
    }
  }
  var_pool_head = &var_pool[0];
}

/*
 * creates and returns a new variable
 */
var_t *v_new() {
  var_t *result = var_pool_head;
  if (result != NULL) {
    // remove an item from the free-list
    var_pool_head = result->v.pool_next;
  } else {
    // pool exhausted
    result = (var_t *)malloc(sizeof(var_t));
    result->pooled = 0;
  }
  v_init(result);
  return result;
}

void v_pool_free(var_t *var) {
  // insert back into the free list
  var->v.pool_next = var_pool_head;
  var_pool_head = var;
}

uint32_t v_get_capacity(uint32_t size) {
  return size + (size / 2) + 1;
}

// allocate capacity in the array container
void v_alloc_capacity(var_t *var, uint32_t size) {
  uint32_t capacity = v_get_capacity(size);
  v_capacity(var) = capacity;
  v_asize(var) = size;
  v_data(var) = (var_t *)malloc(sizeof(var_t) * capacity);
  if (!v_data(var)) {
    err_memory();
  } else {
    for (uint32_t i = 0; i < capacity; i++) {
      var_t *e = v_elem(var, i);
      e->pooled = 0;
      v_init(e);
    }
  }
}

// create an new empty array
void v_init_array(var_t *var) {
  v_capacity(var) = 0;
  v_asize(var) = 0;
  v_data(var) = NULL;
  v_maxdim(var) = 1;
  v_ubound(var, 0) = opt_base;
  v_lbound(var, 0) = opt_base;
}

// create an array of the given size
void v_new_array(var_t *var, uint32_t size) {
  var->type = V_ARRAY;
  v_alloc_capacity(var, size);
}

void v_set_array1_size(var_t *var, uint32_t size) {
  v_asize(var) = size;
  v_maxdim(var) = 1;
  v_ubound(var, 0) = v_lbound(var, 0) + (size - 1);
}

void v_copy_array(var_t *dest, const var_t *src) {
  dest->type = V_ARRAY;
  v_alloc_capacity(dest, v_asize(src));

  // copy dimensions
  v_maxdim(dest) = v_maxdim(src);
  for (int i = 0; i < v_maxdim(src); i++) {
    v_ubound(dest, i) = v_ubound(src, i);
    v_lbound(dest, i) = v_lbound(src, i);
  }

  // copy each element
  uint32_t v_size = v_asize(src);
  for (uint32_t i = 0; i < v_size; i++) {
    var_t *dest_vp = v_elem(dest, i);
    v_init(dest_vp);
    v_set(dest_vp, v_elem(src, i));
  }
}

void v_array_free(var_t *var) {
  uint32_t v_size = v_capacity(var);
  if (v_size && v_data(var)) {
    for (uint32_t i = 0; i < v_size; i++) {
      v_free(v_elem(var, i));
    }
    free(var->v.a.data);
  }
}

void v_init_str(var_t *var, int length) {
  var->type = V_STR;
  var->v.p.ptr = malloc(length + 1);
  var->v.p.ptr[0] = '\0';
  var->v.p.length = length + 1;
  var->v.p.owner = 1;
}

void v_move_str(var_t *var, char *str) {
  var->type = V_STR;
  var->v.p.ptr = str;
  var->v.p.length = strlen(str) + 1;
  var->v.p.owner = 1;
}

/*
 * returns true if the user's program must use this var as an empty var
 * this is usefull for arrays
 */
int v_isempty(var_t *var) {
  switch (var->type) {
  case V_INT:
    return (var->v.i == 0);
  case V_NUM:
    return (var->v.n == 0.0);
  case V_STR:
    return (v_strlen(var) == 0);
  case V_ARRAY:
    return (v_asize(var) == 0);
  case V_PTR:
    return (var->v.ap.p == 0);
  case V_MAP:
    return map_is_empty(var);
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
  case V_INT:
    ltostr(var->v.i, tmpsb);
    return strlen(tmpsb);
  case V_NUM:
    ftostr(var->v.n, tmpsb);
    return strlen(tmpsb);
  case V_STR:
    return v_strlen(var);
  case V_ARRAY:
    return v_asize(var);
  case V_PTR:
    ltostr(var->v.ap.p, tmpsb);
    return strlen(tmpsb);
  case V_MAP:
    return map_length(var);
  case V_REF:
    return v_length(var->v.ref);
  }
  return 0;
}

/*
 * resize an existing array
 */
void v_resize_array(var_t *v, uint32_t size) {
  if (v->type != V_ARRAY) {
    err_varisnotarray();
  } else if ((int)size < 0) {
    err_evargerr();
  } else if (size == v_asize(v)) {
    // already at target size
  } else if (size == 0) {
    v_free(v);
    v_init_array(v);
    v->type = V_ARRAY;
  } else if (size < v_asize(v)) {
    // resize down. free discarded elements
    uint32_t v_size = v_asize(v);
    for (uint32_t i = size; i < v_size; i++) {
      v_free(v_elem(v, i));
    }
    v_set_array1_size(v, size);
  } else if (size <= v_capacity(v)) {
    // use existing capacity
    v_set_array1_size(v, size);
  } else {
    // insufficient capacity
    uint32_t prev_size = v_asize(v);
    if (prev_size == 0) {
      v_alloc_capacity(v, size);
    } else if (prev_size < size) {
      // resize & copy
      uint32_t capacity = v_get_capacity(size);
      v_capacity(v) = capacity;
      v_data(v) = (var_t *)realloc(v_data(v), sizeof(var_t) * capacity);
      for (uint32_t i = prev_size; i < capacity; i++) {
        var_t *e = v_elem(v, i);
        e->pooled = 0;
        v_init(e);
      }
    }

    // init vars
    for (uint32_t i = prev_size; i < size; i++) {
      v_init(v_elem(v, i));
    }

    v_set_array1_size(v, size);
  }
}

/*
 * create RxC array
 */
void v_tomatrix(var_t *v, int r, int c) {
  v_free(v);
  v_new_array(v, r * c);
  v_maxdim(v) = 2;
  v_lbound(v, 0) = v_lbound(v, 1) = opt_base;
  v_ubound(v, 0) = opt_base + (r - 1);
  v_ubound(v, 1) = opt_base + (c - 1);
}

/*
 * create array
 */
void v_toarray1(var_t *v, uint32_t r) {
  v_free(v);
  v->type = V_ARRAY;
  if (r > 0) {
    v_new_array(v, r);
    v_maxdim(v) = 1;
    v_lbound(v, 0) = opt_base;
    v_ubound(v, 0) = opt_base + (r - 1);
  } else {
    v_init_array(v);
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
    return (ABS(v->v.n) > 1E-308);
  case V_STR:
    return (v->v.p.length != 0);
  case V_ARRAY:
    return (v_asize(v) != 0);
  case V_PTR:
    return (v->v.ap.p != 0);
  case V_MAP:
    return !map_is_empty(v);
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
    if (v_asize(a) < v_asize(b)) {
      return -1;
    } else if (v_asize(a) > v_asize(b)) {
      return 1;
    }
    // check every element
    for (uint32_t i = 0; i < v_asize(a); i++) {
      var_t *ea = v_elem(a, i);
      var_t *eb = v_elem(b, i);
      int ci = v_compare(ea, eb);
      if (ci != 0) {
        return ci;
      }
    }
    // equal
    return 0;
  }

  if (a->type == V_MAP && b->type == V_MAP) {
    return map_compare(a, b);
  }

  if (a->type == V_NIL || b->type == V_NIL) {
    // return equal (0) when both NONE
    return (a->type != b->type);
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
    v_init_str(result, length);
    strcpy(result->v.p.ptr, a->v.p.ptr);
    strcat(result->v.p.ptr, b->v.p.ptr);
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
      v_init_str(result, strlen(a->v.p.ptr) + INT_STR_LEN);
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
      v_init_str(result, strlen(b->v.p.ptr) + INT_STR_LEN);
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
  case V_NUM:
    dest->v.n = src->v.n;
    break;
  case V_STR:
    if (src->v.p.owner) {
      dest->v.p.length = v_strlen(src) + 1;
      dest->v.p.ptr = (char *)malloc(dest->v.p.length);
      dest->v.p.owner = 1;
      strcpy(dest->v.p.ptr, src->v.p.ptr);
    } else {
      dest->v.p.length = src->v.p.length;
      dest->v.p.ptr = src->v.p.ptr;
      dest->v.p.owner = 0;
    }
    break;
  case V_ARRAY:
    if (v_asize(src)) {
      v_copy_array(dest, src);
    } else {
      v_init_array(dest);
    }
    break;
  case V_PTR:
    dest->v.ap.p = src->v.ap.p;
    dest->v.ap.v = src->v.ap.v;
    break;
  case V_MAP:
    // reset type since not yet a map
    dest->type = 0;
    map_set(dest, (const var_p_t)src);
    break;
  case V_REF:
    dest->v.ref = src->v.ref;
    break;
  case V_FUNC:
    dest->v.fn.cb = src->v.fn.cb;
    dest->v.fn.id = src->v.fn.id;
    break;
  case V_NIL:
    dest->type = V_NIL;
    dest->const_flag = 1;
    break;
  }
}

/*
 * assign (dest = src)
 */
void v_move(var_t *dest, const var_t *src) {
  v_free(dest);
  dest->const_flag = 0;
  dest->type = src->type;

  switch (src->type) {
  case V_INT:
    dest->v.i = src->v.i;
    break;
  case V_NUM:
    dest->v.n = src->v.n;
    break;
  case V_STR:
    dest->v.p.ptr = src->v.p.ptr;
    dest->v.p.length = src->v.p.length;
    dest->v.p.owner = src->v.p.owner;
    break;
  case V_ARRAY:
    memcpy(&dest->v.a, &src->v.a, sizeof(src->v.a));
    v_maxdim(dest) = v_maxdim(src);
    break;
  case V_PTR:
    dest->v.ap.p = src->v.ap.p;
    dest->v.ap.v = src->v.ap.v;
    break;
  case V_MAP:
    dest->v.m.map = src->v.m.map;
    dest->v.m.count = src->v.m.count;
    dest->v.m.size = src->v.m.size;
    dest->v.m.id = src->v.m.id;
    dest->v.m.lib_id = src->v.m.lib_id;
    dest->v.m.cls_id = src->v.m.cls_id;
    break;
  case V_REF:
    dest->v.ref = src->v.ref;
    break;
  case V_FUNC:
    dest->v.fn.cb = src->v.fn.cb;
    dest->v.fn.id = src->v.fn.id;
    break;
  case V_NIL:
    dest->type = V_NIL;
    dest->const_flag = 1;
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
 * returns the sign of a variable
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
  v_init_str(v, strlen(src));
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
  case V_NIL:
    buffer = malloc(5);
    strcpy(buffer, SB_KW_NONE_STR);
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
    v_free(arg);
    v_init_str(arg, strlen(tmp));
    strcpy(arg->v.p.ptr, tmp);
    free(tmp);
  }
}

/*
 * set the value of 'var' to string
 */
void v_setstr(var_t *var, const char *str) {
  if (var->type != V_STR || strcmp(str, var->v.p.ptr) != 0) {
    v_free(var);
    v_init_str(var, strlen(str));
    strcpy(var->v.p.ptr, str);
  }
}

void v_setstrn(var_t *var, const char *str, int len) {
  if (var->type != V_STR || strncmp(str, var->v.p.ptr, len) != 0) {
    v_free(var);
    v_init_str(var, len);
    strlcpy(var->v.p.ptr, str, len + 1);
  }
}

/*
 * adds a string to current string value
 */
void v_strcat(var_t *var, const char *str) {
  if (var->type == V_INT || var->type == V_NUM) {
    v_tostr(var);
  }
  if (var->type == V_STR) {
    if (var->v.p.owner) {
      var->v.p.length = strlen(var->v.p.ptr) + strlen(str) + 1;
      var->v.p.ptr = realloc(var->v.p.ptr, var->v.p.length);
      strcat(var->v.p.ptr, str);
    } else {
      // mutate into owner string
      char *p = var->v.p.ptr;
      int len = var->v.p.length + strlen(str);
      v_init_str(var, len);
      strcpy(var->v.p.ptr, p);
      strcat(var->v.p.ptr, str);
    }
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
  v_init_str(r, 0);
  r->v.p.ptr[0] = '\0';
}

/*
 * convert's a user's string to variable
 *
 * its decides in what format to store the value
 * its used mostly by 'input' functions
 */
void v_input2var(const char *str, var_t *var) {
  v_free(var);

  if (!str || str[0] == '\0') {
    // no data
    v_setstr(var, str);
  } else {
    int type;
    var_int_t lv;
    var_num_t dv;

    char *buf = strdup(str);
    char *sb = strdup(str);
    char *np = get_numexpr(sb, buf, &type, &lv, &dv);

    if (type == 1 && *np == '\0') {
      v_setint(var, lv);
    } else if (type == 2 && *np == '\0') {
      v_setreal(var, dv);
    } else {
      v_setstr(var, str);
    }
    free(sb);
    free(buf);
  }
}

void v_create_func(var_p_t map, const char *name, method cb) {
  var_p_t v_func = map_add_var(map, name, 0);
  v_func->type = V_FUNC;
  v_func->v.fn.cb = cb;
  v_func->v.fn.id = 0;
}
