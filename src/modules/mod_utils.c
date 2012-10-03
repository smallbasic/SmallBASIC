// This file is part of SmallBASIC
//
// Module Support Routines
//
// Also provides access to var_t routines for platforms that do 
// not support backlinking (such as windows).
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "mod_utils.h"

#if defined(__linux__) && defined(_UnixOS)
#define LNX_EXTLIB
#endif

int mod_parint(int n, slib_par_t *params, int param_count, int *val) {
  var_t *param;

  if (n < 0 || n >= param_count) {
    return 0;
  }
  param = params[n].var_p;
  *val = v_igetval(param);

  return 1;
}

int mod_opt_parint(int n, slib_par_t *params, int param_count, int *val,
                   int def_val) {
  var_t *param;

  if (n < 0) {
    return 0;
  }
  if (n < param_count) {
    param = params[n].var_p;
    *val = v_igetval(param);
  }
  else {
    *val = def_val;
  }

  return 1;
}

int mod_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr) {
  var_t *param;

  if (n < 0 || n >= param_count) {
    return 0;
  }
  param = params[n].var_p;

  if (param->type != V_STR) {
    v_tostr(param);
  }
  *ptr = param->v.p.ptr;

  return 1;
}

int mod_opt_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr,
                       const char *def_val) {
  var_t *param;

  if (n < 0) {
    return 0;
  }

  if (n < param_count) {
    param = params[n].var_p;

    if (param->type != V_STR) {
      v_tostr(param);
    }
    *ptr = param->v.p.ptr;
  }
  else {
    *ptr = def_val;
  }

  return 1;
}

#if !defined(LNX_EXTLIB)

int opt_base = 0;

/*
 *initialize a variable
 */
void v_init(var_t *v) {
  v->type = V_INT;
  v->const_flag = 0;
  v->v.i = 0;
}

void v_setint(var_t *var, int32 i) {
  v_free(var);
  var->type = V_INT;
  var->v.i = i;
}

/*
 *release variable
 */
void v_free(var_t *v) {
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
          elem = (var_t *) (v->v.a.ptr + (sizeof(var_t) *i));
          v_free(elem);
        }

        tmp_free(v->v.a.ptr);
        v->v.a.ptr = NULL;
        v->v.a.size = 0;
      }
    }
    break;
  case V_UDS:
    break;
  }

  v_init(v);
}

/*
 *returns the integer value of the variable v
 */
long v_igetval(var_t *v) {
  if (v == 0) {
    //err_evsyntax();
  }
  else {
    switch (v->type) {
    case V_UDS:
      return 0;;
    case V_PTR:
      return v->v.ap.p;
    case V_INT:
      return v->v.i;
    case V_NUM:
      return v->v.n;
    case V_STR:
      return 0;// numexpr_strtol((char *)v->v.p.ptr);
    default:
      //err_varisarray();
      break;
    }
  }
  return 0;
}

/*
 *converts the variable to string-variable
 */
void v_tostr(var_t *arg) {
  if (arg->type != V_STR) {
    char *tmp;
    int l;

    tmp = tmp_alloc(64);

    switch (arg->type) {
    case V_UDS:
      break;
    case V_PTR:
      //ltostr(arg->v.ap.p, tmp);
      break;
    case V_INT:
      //ltostr(arg->v.i, tmp);
      break;
    case V_NUM:
      //ftostr(arg->v.n, tmp);
      break;
    default:
      //err_varisarray();
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

void v_setstr(var_t *var, const char *string) {
  v_free(var);
  var->type = V_STR;
  var->v.p.size = strlen(string) + 1;
  var->v.p.ptr = tmp_alloc(var->v.p.size);
  strcpy(var->v.p.ptr, string);
}

/*
 *set the value of 'var' to string
 */
void v_setstrf(var_t *var, const char *fmt, ...) {
  char *buf;
  va_list ap;

  va_start(ap, fmt);
  buf = tmp_alloc(1024);
  vsnprintf(buf, 1024, fmt, ap);
  v_setstr(var, buf);
  tmp_free(buf);
  va_end(ap);
}

/*
 *create RxC array
 */
void v_tomatrix(var_t *v, int r, int c) {
  var_t *e;
  int i;

  v_free(v);
  v->type = V_ARRAY;

  // create data
  v->v.a.size = r *c;
  v->v.a.ptr = tmp_alloc(sizeof(var_t) *v->v.a.size);
  for (i = 0; i < r *c; i++) {
    e = (var_t *) (v->v.a.ptr + (sizeof(var_t) *i));
    v_init(e);
  }

  // array info
  v->v.a.lbound[0] = v->v.a.lbound[1] = opt_base;
  v->v.a.ubound[0] = opt_base + (r - 1);
  v->v.a.ubound[1] = opt_base + (c - 1);
  v->v.a.maxdim = 2;
}
#endif


