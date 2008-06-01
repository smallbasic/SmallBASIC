// $Id$
// This file is part of SmallBASIC
//
// SmallBasic Variable Manager.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sys.h"
#include "str.h"
#include "var.h"
#include "smbas.h"
#include "sberr.h"
#include "var_uds.h"
#include "var_hash.h"

#define ARR_ALLOC       256

/*
 * initialize a variable
 */
void v_init(var_t * v)
{
  v->type = V_INT;
  v->const_flag = 0;
  v->v.i = 0;
}

/*
 * creates and returns a new variable
 */
var_t *v_new()
{
  var_t *ptr;

  ptr = (var_t *) tmp_alloc(sizeof(var_t));
  v_init(ptr);
  return ptr;
}

/*
 * release variable
 */
void v_free(var_t * v)
{
  int i;
  var_t *elem;

  switch (v->type) {
  case V_STR:
    if (v->v.p.ptr) {
      tmp_free(v->v.p.ptr);
    }
    v->v.p.ptr = NULL;
    v->v.p.size = 0;
    break;
  case V_ARRAY:
    if (v->v.a.size) {
      if (v->v.a.ptr) {
        for (i = 0; i < v->v.a.size; i++) {
          elem = (var_t *) (v->v.a.ptr + (sizeof(var_t) * i));
          v_free(elem);
        }

        tmp_free(v->v.a.ptr);
        v->v.a.ptr = NULL;
        v->v.a.size = 0;
      }
    }
    break;
  case V_UDS:
    uds_free(v);
    break;
  case V_HASH:
    hash_free(v);
  }

  v_init(v);
}

/*
 * returns true if the user's program must use this var as an empty var
 * this is usefull for arrays
 */
int v_isempty(var_t * var)
{
  switch (var->type) {
  case V_STR:
    return (strlen((char *)var->v.p.ptr) == 0);
  case V_INT:
    return (var->v.i == 0);
  case V_UDS:
    return uds_is_empty(var);
  case V_HASH:
    return hash_is_empty(var);
  case V_PTR:
    return (var->v.ap.p == 0);
  case V_NUM:
    return (var->v.n == 0.0);
  case V_ARRAY:
    return (var->v.a.size == 0);
  }

  return 1;
}

/*
 * returns the length of the variable
 */
int v_length(var_t * var)
{
  char tmpsb[64];

  switch (var->type) {
  case V_STR:
    return strlen((char *)var->v.p.ptr);
  case V_UDS:
    uds_to_str((const var_p_t)var->v.uds, tmpsb, 64);
    return strlen(tmpsb);
  case V_HASH:
    hash_to_str((const var_p_t)var->v.uds, tmpsb, 64);
    return strlen(tmpsb);
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
  }

  return 1;
}

/*
 * returns the floating-point value of the variable v
 */
double v_getval(var_t * v)
{
  if (v == NULL) {
    err_evsyntax();
  }
  else {
    switch (v->type) {
    case V_UDS:
      return uds_to_int(v);
    case V_HASH:
      return hash_to_int(v);
    case V_PTR:
      return v->v.ap.p;
    case V_INT:
      return v->v.i;
    case V_NUM:
      return v->v.n;
    case V_STR:
      return numexpr_sb_strtof((char *)v->v.p.ptr);
    default:
      err_varisarray();
    }
  }
  return 0;
}

/*
 * returns the integer value of the variable v
 */
long v_igetval(var_t * v)
{
  if (v == 0) {
    err_evsyntax();
  }
  else {
    switch (v->type) {
    case V_UDS:
      return uds_to_int(v);
    case V_HASH:
      return hash_to_int(v);
    case V_PTR:
      return v->v.ap.p;
    case V_INT:
      return v->v.i;
    case V_NUM:
      return v->v.n;
    case V_STR:
      return numexpr_strtol((char *)v->v.p.ptr);
    default:
      err_varisarray();
    }
  }
  return 0;
}

/*
 * return array element pointer
 */
#if defined(OS_ADDR16)
var_t *v_getelemptr(var_t * v, word index)
#else
var_t *v_getelemptr(var_t * v, dword index)
#endif
{
  if (v->type == V_ARRAY) {
    if (index < v->v.a.size)
      return (var_t *) (v->v.a.ptr + (index * sizeof(var_t)));
    else {
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
#if defined(OS_ADDR16)
void v_resize_array(var_t * v, word size)
#else
void v_resize_array(var_t * v, dword size)
#endif
{
  byte *prev;
  var_t *elem;
#if defined(OS_ADDR16)
  word i;
#else
  dword i;
#endif

  if (v->type == V_ARRAY) {
    if (size < 0) {
      err_evargerr();
      return;
    }

    if (size == 0) {
      v_free(v);
      v->type = V_ARRAY;
      v->v.a.size = 0;
      v->v.a.ptr = NULL;
      v->v.a.ubound[0] = v->v.a.lbound[0] = opt_base;
      v->v.a.maxdim = 1;
    }
    else if (v->v.a.size > size) {
      // resize down

      // free vars
      for (i = size; i < v->v.a.size; i++) {
        elem = (var_t *) (v->v.a.ptr + (sizeof(var_t) * i));
        v_free(elem);
      }

#if defined(OS_ADDR32)
      // do not resize array
      prev = NULL;
#else
      // resize & copy
      prev = v->v.a.ptr;
      v->v.a.ptr = tmp_alloc(size * sizeof(var_t));
      memcpy(v->v.a.ptr, prev, size * sizeof(var_t));
#endif
      // array data
      v->v.a.size = size;
      v->v.a.ubound[0] = v->v.a.lbound[0] + (size - 1);
      v->v.a.maxdim = 1;

      // 
      if (prev) {
        tmp_free(prev);
      }
    }
    else if (v->v.a.size < size) {
      // resize up

#if defined(OS_ADDR32)
      // if there is space do not resize 
      if (v->v.a.size == 0) {
        prev = v->v.a.ptr;
        v->v.a.ptr = tmp_alloc((size + ARR_ALLOC) * sizeof(var_t));
      }
      else {
        if (MemPtrSize(v->v.a.ptr) < size * sizeof(var_t)) {
          // resize & copy
          prev = v->v.a.ptr;
          v->v.a.ptr = tmp_alloc((size + ARR_ALLOC) * sizeof(var_t));
          if (v->v.a.size > 0)
            memcpy(v->v.a.ptr, prev, v->v.a.size * sizeof(var_t));
        }
        else
          prev = NULL;
      }
#else
      // resize & copy
      prev = v->v.a.ptr;
      v->v.a.ptr = tmp_alloc(size * sizeof(var_t));
      if (v->v.a.size > 0) {
        memcpy(v->v.a.ptr, prev, v->v.a.size * sizeof(var_t));
      }
#endif

      // init vars
      for (i = v->v.a.size; i < size; i++) {
        elem = (var_t *) (v->v.a.ptr + (sizeof(var_t) * i));
        v_init(elem);
      }

      // array data
      v->v.a.size = size;
      v->v.a.ubound[0] = v->v.a.lbound[0] + (size - 1);
      v->v.a.maxdim = 1;

      // 
      if (prev) {
        tmp_free(prev);
      }
    }
  }
  else {
    err_varisnotarray();
  }
}

/*
 * create RxC array
 */
void v_tomatrix(var_t * v, int r, int c)
{
  var_t *e;
  int i;

  v_free(v);
  v->type = V_ARRAY;

  // create data
  v->v.a.size = r * c;
  v->v.a.ptr = tmp_alloc(sizeof(var_t) * v->v.a.size);
  for (i = 0; i < r * c; i++) {
    e = (var_t *) (v->v.a.ptr + (sizeof(var_t) * i));
    v_init(e);
  }

  // array info
  v->v.a.lbound[0] = v->v.a.lbound[1] = opt_base;
  v->v.a.ubound[0] = opt_base + (r - 1);
  v->v.a.ubound[1] = opt_base + (c - 1);
  v->v.a.maxdim = 2;
}

/*
 * create RxC array
 */
var_t *v_new_matrix(int r, int c)
{
  var_t *v;

  v = v_new();
  v_tomatrix(v, r, c);

  return v;
}

/*
 * create array
 */
#if defined(OS_ADDR16)
void v_toarray1(var_t * v, word r)
#else
void v_toarray1(var_t * v, dword r)
#endif
{
  var_t *e;
#if defined(OS_ADDR16)
  word i;
#else
  dword i;
#endif

  v_free(v);
  v->type = V_ARRAY;

  if (r > 0) {
    // create data
    v->v.a.size = r;
#if defined(OS_ADDR32)
    v->v.a.ptr = tmp_alloc(sizeof(var_t) * (v->v.a.size + ARR_ALLOC));
#else
    v->v.a.ptr = tmp_alloc(sizeof(var_t) * v->v.a.size);
#endif
    for (i = 0; i < r; i++) {
      e = (var_t *) (v->v.a.ptr + (sizeof(var_t) * i));
      v_init(e);
    }

    // array info
    v->v.a.maxdim = 1;
    v->v.a.lbound[0] = opt_base;
    v->v.a.ubound[0] = opt_base + (r - 1);
  }
  else {
    v->v.a.size = 0;
    v->v.a.ptr = NULL;
    v->v.a.lbound[0] = v->v.a.ubound[0] = opt_base;
    v->v.a.maxdim = 1;
  }
}

/*
 * returns true if the variable v is not empty (0 for nums)
 */
int v_is_nonzero(var_t * v)
{
  switch (v->type) {
  case V_INT:
    return (v->v.i != 0);
  case V_NUM:
    // return (v->v.n != 0.0 && v->v.n != -0.0);
    return (ABS(v->v.n) > 1E-308);
  case V_STR:
    return (v->v.p.size != 0);
  case V_UDS:
    return !uds_is_empty(v);
  case V_HASH:
    return !hash_is_empty(v);
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
int v_compare(var_t * a, var_t * b)
{
  double dt;
  long di;
  int i, ci;
  var_t *ea, *eb;

  if (a == 0 || b == 0) {
    err_evsyntax();
    return 0;
  }

  if (a->type == V_INT && b->type == V_INT) {
    di = (a->v.i - b->v.i);
    if (di < 0) {               // ndc: 18/03/2001
      return -1;
    }
    if (di > 0) {
      return 1;
    }
    return 0;
  }
  else if ((a->type == V_INT || a->type == V_NUM) &&
           (b->type == V_INT || b->type == V_NUM)) {
    dt =
      (((a->type == V_INT) ? a->v.i : a->v.n) - ((b->type ==
                                                  V_INT) ? b->v.i : b->v.n));
    // printf("== %g - %g = %g ==", a->v.n, b->v.n, dt);
    return ZSGN(dt);
  }
  if ((a->type == V_STR) && (b->type == V_STR))
    return strcmp(a->v.p.ptr, b->v.p.ptr);
  if ((a->type == V_STR) && (b->type == V_NUM)) {
    if (a->v.p.ptr[0] == '\0' || is_number((char *)a->v.p.ptr)) { // compare 
      // nums
      dt = v_getval(a);
      return (dt < b->v.n) ? -1 : ((dt == b->v.n) ? 0 : 1);
    }
    return 1;
  }
  if ((a->type == V_NUM) && (b->type == V_STR)) {
    if (b->v.p.ptr[0] == '\0' || is_number((char *)b->v.p.ptr)) { // compare 
      // nums
      dt = v_getval(b);
      return (dt < a->v.n) ? 1 : ((dt == a->v.n) ? 0 : -1);
    }
    return -1;
  }
  if ((a->type == V_STR) && (b->type == V_INT)) {
    if (a->v.p.ptr[0] == '\0' || is_number((char *)a->v.p.ptr)) { // compare 
      // nums
      di = v_igetval(a);
      return (di < b->v.i) ? -1 : ((di == b->v.i) ? 0 : 1);
    }
    return 1;
  }
  if ((a->type == V_INT) && (b->type == V_STR)) {
    if (b->v.p.ptr[0] == '\0' || is_number((char *)b->v.p.ptr)) { // compare 
      // nums
      di = v_igetval(b);
      return (di < a->v.i) ? 1 : ((di == a->v.i) ? 0 : -1);
    }
    return -1;
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
    for (i = 0; i < a->v.a.size; i++) {
      ea = (var_t *) (a->v.a.ptr + sizeof(var_t) * i);
      eb = (var_t *) (b->v.a.ptr + sizeof(var_t) * i);
      if ((ci = v_compare(ea, eb)) != 0) {
        return ci;
      }
    }

    return 0;                   // equal
  }

  if (a->type == V_UDS && b->type == V_UDS) {
    return uds_compare(a, b);
  }

  if (a->type == V_HASH && b->type == V_HASH) {
    return hash_compare(a, b);
  }

  err_evtype();                 // ndc 01/08/2001
  return 1;
}

/*
 */
int v_addtype(var_t * a, var_t * b)
{
  if (a->type == V_STR) {
    return V_STR;
  }
  if (a->type == V_NUM || b->type == V_NUM) {
    return V_NUM;
  }
  if (a->type == V_INT || b->type == V_STR) {
    return V_NUM;
  }
  return V_INT;
}

/*
 * add two variables
 * result = a + b
 */
void v_add(var_t * result, var_t * a, var_t * b)
{
  char tmpsb[64];

  if (a->type == V_STR && b->type == V_STR) {
    result->type = V_STR;
    result->v.p.ptr =
      (byte *) tmp_alloc(strlen((char *)a->v.p.ptr) + strlen((char *)b->v.p.ptr) +
                         1);
    strcpy((char *)result->v.p.ptr, (char *)a->v.p.ptr);
    strcat((char *)result->v.p.ptr, (char *)b->v.p.ptr);
    result->v.p.size = strlen((char *)result->v.p.ptr) + 1;
    return;
  }
  else if (a->type == V_INT && b->type == V_INT) {
    result->type = V_INT;
    result->v.i = a->v.i + b->v.i;
    return;
  }
  else if (a->type == V_NUM && b->type == V_NUM) {
    result->type = V_NUM;
    result->v.n = a->v.n + b->v.n;
    return;
  }
  else if (a->type == V_NUM && b->type == V_INT) {
    result->type = V_NUM;
    result->v.n = a->v.n + b->v.i;
    return;
  }
  else if (a->type == V_INT && b->type == V_NUM) {
    result->type = V_NUM;
    result->v.n = a->v.i + b->v.n;
    return;
  }
  else if (a->type == V_STR && (b->type == V_INT || b->type == V_NUM)) {
    if (is_number((char *)a->v.p.ptr)) {
      result->type = V_NUM;
      if (b->type == V_INT) {
        result->v.n = b->v.i + v_getval(a);
      }
      else {
        result->v.n = b->v.n + v_getval(a);
      }
    }
    else {
      result->type = V_STR;
      result->v.p.ptr = (byte *) tmp_alloc(strlen((char *)a->v.p.ptr) + 64);
      strcpy((char *)result->v.p.ptr, (char *)a->v.p.ptr);
      if (b->type == V_INT) {
        ltostr(b->v.i, tmpsb);
      }
      else {
        ftostr(b->v.n, tmpsb);
      }
      strcat((char *)result->v.p.ptr, tmpsb);
      result->v.p.size = strlen((char *)result->v.p.ptr) + 1;
    }
  }
  else if ((a->type == V_INT || a->type == V_NUM) && b->type == V_STR) {
    if (is_number((char *)b->v.p.ptr)) {
      result->type = V_NUM;
      if (a->type == V_INT)
        result->v.n = a->v.i + v_getval(b);
      else
        result->v.n = a->v.n + v_getval(b);
    }
    else {
      result->type = V_STR;
      result->v.p.ptr = (byte *) tmp_alloc(strlen((char *)b->v.p.ptr) + 64);
      if (a->type == V_INT) {
        ltostr(a->v.i, tmpsb);
      }
      else {
        ftostr(a->v.n, tmpsb);
      }
      strcpy((char *)result->v.p.ptr, tmpsb);
      strcat((char *)result->v.p.ptr, (char *)b->v.p.ptr);
      result->v.p.size = strlen((char *)result->v.p.ptr) + 1;
    }
  }
}

/*
 * assign (dest = src)
 */
void v_set(var_t *dest, const var_t *src)
{
  int i;
  var_t *dest_vp, *src_vp;

  if (src->type == V_UDS) {
    uds_set(dest, (const var_p_t)src);
    return;
  }
  else if (dest->type == V_UDS) {
    // lvalue struct assigned to non-struct rvalue
    uds_clear(dest);
    return;
  }
  else if (src->type == V_HASH) {
    hash_set(dest, (const var_p_t)src);
    return;
  }
  else if (dest->type == V_HASH) {
    // lvalue struct assigned to non-struct rvalue
    hash_clear(dest);
    return;
  }

  v_free(dest);
  *dest = *src;
  dest->const_flag = 0;
  switch (src->type) {
  case V_STR:
    dest->v.p.ptr = (byte *) tmp_alloc(strlen((char *)src->v.p.ptr) + 1);
    strcpy((char *)dest->v.p.ptr, (char *)src->v.p.ptr);
    break;

  case V_ARRAY:
    if (src->v.a.size) {
      dest->v.a.ptr = tmp_alloc(src->v.a.size * sizeof(var_t));

      // copy each element
      for (i = 0; i < src->v.a.size; i++) {
        src_vp = (var_t *) (src->v.a.ptr + (sizeof(var_t) * i));
        dest_vp = (var_t *) (dest->v.a.ptr + (sizeof(var_t) * i));
        v_init(dest_vp);
        v_set(dest_vp, src_vp);
      }
    }
    else {
      dest->v.a.size = 0;
      dest->v.a.ptr = NULL;
      dest->v.a.ubound[0] = dest->v.a.lbound[0] = opt_base;
      dest->v.a.maxdim = 1;
    }
    break;

  case V_PTR:
    dest->v.ap = src->v.ap;
    dest->type = src->type;
    break;
  }
}

/*
 * return a full copy of the 'source'
 */
var_t *v_clone(const var_t * source)
{
  var_t *vnew;

  vnew = (var_t *) tmp_alloc(sizeof(var_t));
  v_init(vnew);
  v_set(vnew, source);
  return vnew;
}

/*
 * add b to a
 */
void v_inc(var_t * a, var_t * b)
{
  if (a->type == V_INT && b->type == V_INT) {
    a->v.i += b->v.i;
  }
  else if (a->type == V_NUM && b->type == V_NUM) {
    a->v.n += b->v.n;
  }
  else if (a->type == V_NUM && b->type == V_INT) {
    a->v.n += b->v.i;
  }
  else if (a->type == V_INT && b->type == V_NUM) {
    a->type = V_NUM;
    a->v.n = a->v.i + b->v.n;
  }
  else {
    err_varnotnum();
  }
}

/*
 */
int v_sign(var_t * x)
{
  if (x->type == V_INT) {
    return (x->v.i < 0) ? -1 : ((x->v.i == 0) ? 0 : 1);
  }
  else if (x->type == V_NUM) {
    return (x->v.n < 0) ? -1 : ((x->v.n == 0) ? 0 : 1);
  }
  err_varnotnum();
  return 0;
}

/*
 * setup a string variable
 */
void v_createstr(var_t * v, const char *src)
{
  int l;

  l = strlen(src) + 1;
  v->type = V_STR;
  v->v.p.ptr = tmp_alloc(l);
  v->v.p.size = l;
  strcpy(v->v.p.ptr, src);
}

/*
 * converts the variable to string-variable
 */
void v_tostr(var_t * arg)
{
  if (arg->type != V_STR) {
    char *tmp;
    int l;

    tmp = tmp_alloc(64);

    switch (arg->type) {
    case V_UDS:
      uds_to_str(arg, tmp, 64);
      uds_free(arg);
      break;
    case V_HASH:
      hash_to_str(arg, tmp, 64);
      hash_free(arg);
      break;
    case V_PTR:
      ltostr(arg->v.ap.p, tmp);
      break;
    case V_INT:
      ltostr(arg->v.i, tmp);
      break;
    case V_NUM:
      ftostr(arg->v.n, tmp);
      break;
    default:
      err_varisarray();
      tmp_free(tmp);
      return;
    }

    l = strlen(tmp) + 1;
    arg->type = V_STR;
    arg->v.p.ptr = tmp_alloc(l);
    arg->v.p.size = l;
    strcpy(arg->v.p.ptr, tmp);
    tmp_free(tmp);
  }
}

/*
 * set the value of 'var' to string
 */
void v_setstr(var_t * var, const char *string)
{
  v_free(var);
  var->type = V_STR;
  var->v.p.size = strlen(string) + 1;
  var->v.p.ptr = tmp_alloc(var->v.p.size);
  strcpy(var->v.p.ptr, string);
}

void v_setstrn(var_t * var, const char *string, int len)
{
  v_free(var);
  var->type = V_STR;
  var->v.p.size = len + 1;
  var->v.p.ptr = tmp_alloc(var->v.p.size);
  strncpy(var->v.p.ptr, string, len);
  var->v.p.ptr[len] = 0;
}

/*
 * set the value of 'var' to string
 */
void v_setstrf(var_t * var, const char *fmt, ...)
{
  char *buf;
  va_list ap;

  va_start(ap, fmt);
#if defined(OS_LIMITED)
  buf = tmp_alloc(1024);
#else
  buf = tmp_alloc(0x10000);
#endif
#if defined(_PalmOS)
  StrVPrintF(buf, fmt, ap);
#elif defined(_DOS)
  vsprintf(buf, fmt, ap);
#else
  vsnprintf(buf, 1024, fmt, ap);
#endif
  v_setstr(var, buf);
  tmp_free(buf);
  va_end(ap);
}

/*
 * adds a string to current string value
 */
void v_strcat(var_t * var, const char *string)
{
  if (var->type == V_INT || var->type == V_NUM) {
    v_tostr(var);
  }
  if (var->type == V_STR) {
    var->v.p.size = strlen((char *)var->v.p.ptr) + strlen(string) + 1;
    var->v.p.ptr = tmp_realloc(var->v.p.ptr, var->v.p.size);
    strcat((char *)var->v.p.ptr, string);
  }
  else {
    err_typemismatch();
  }
}

/*
 * set the value of 'var' to n
 */
void v_setreal(var_t * var, double n)
{
  v_free(var);
  var->type = V_NUM;
  var->v.n = n;
}

/*
 * set the value of 'var' to i
 */
void v_setint(var_t * var, int32 i)
{
  v_free(var);
  var->type = V_INT;
  var->v.i = i;
}

/*
 * return the string value of 'var'
 */
char *v_getstr(var_t * var)
{
  if (var->type != V_STR) {
    v_tostr(var);
  }
  return (char *)var->v.p.ptr;
}

/*
 * set the value of 'var' to 'itable' integer array
 */
void v_setintarray(var_t * var, int32 * itable, int count)
{
  int i;
  var_t *elem_p;

  v_toarray1(var, count);
  for (i = 0; i < count; i++) {
    elem_p = (var_t *) (var->v.a.ptr + sizeof(var_t) * i);
    v_setint(elem_p, itable[i]);
  }
}

/*
 * set the value of 'var' to 'itable' real's array
 */
void v_setrealarray(var_t * var, double *ntable, int count)
{
  int i;
  var_t *elem_p;

  v_toarray1(var, count);
  for (i = 0; i < count; i++) {
    elem_p = (var_t *) (var->v.a.ptr + sizeof(var_t) * i);
    v_setreal(elem_p, ntable[i]);
  }
}

/*
 * set the value of 'var' to 'itable' string array
 */
void v_setstrarray(var_t * var, char **ctable, int count)
{
  int i;
  var_t *elem_p;

  v_toarray1(var, count);
  for (i = 0; i < count; i++) {
    elem_p = (var_t *) (var->v.a.ptr + sizeof(var_t) * i);
    v_setstr(elem_p, ctable[i]);
  }
}

/*
 * set an empty string
 */
void v_zerostr(var_t * r)
{
  v_free(r);
  r->type = V_STR;
  r->v.p.ptr = tmp_alloc(1);
  r->v.p.ptr[0] = '\0';
  r->v.p.size = 1;
}

/*
 * convert's a user's string to variable
 *
 * its decides in what format to store the value
 * its used mostly by 'input' functions
 */
void v_input2var(const char *str, var_t * var)
{
  v_free(var);

  if (strlen(str) == 0) {       // no data
    v_setstr(var, str);
  }
  else {
    char *np, *sb;
    char buf[64];
    int type;
    long lv;
    double dv;

    sb = tmp_strdup(str);
    np = get_numexpr(sb, buf, &type, &lv, &dv);

    if (type == 1 && *np == '\0') {
      v_setint(var, lv);
    }
    else if (type == 2 && *np == '\0') {
      v_setreal(var, dv);
    }
    else {
      v_setstr(var, str);
    }
    tmp_free(sb);
  }
}
