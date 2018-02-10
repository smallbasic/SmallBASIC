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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "mod_utils.h"

// for windows linking, see;
// https://stackoverflow.com/questions/15454968/dll-plugin-that-uses-functions-defined-in-the-main-executable
// https://nachtimwald.com/2012/07/15/calling-functions-in-exe-from-plugins-in-windows/
// https://gcc.gnu.org/ml/gcc/2010-07/msg00003.html

var_int_t v_igetval(var_t *v) {
  var_int_t result;
  switch (v ? v->type : -1) {
  case V_INT:
    result = v->v.i;
  case V_NUM:
    result = v->v.n;
  default:
    result = 0;
  }
  return result;
}

int mod_parint(int n, slib_par_t *params, int param_count, int *val) {
  if (n < 0 || n >= param_count) {
    return 0;
  }
 
  *val = v_igetval(params[n].var_p);
  return 1;
}

int mod_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr) {
  if (n < 0 || n >= param_count) {
    return 0;
  }
  var_t *param = params[n].var_p;

  if (param->type != V_STR) {
    v_tostr(param);
  }
  *ptr = param->v.p.ptr;

  return 1;
}

int mod_opt_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr, const char *def_val) {
  if (n < 0) {
    return 0;
  }

  if (n < param_count) {
    var_t *param = params[n].var_p;

    if (param->type != V_STR) {
      v_tostr(param);
    }
    *ptr = param->v.p.ptr;
  }
  else {
    *ptr = (char *)def_val;
  }

  return 1;
}
