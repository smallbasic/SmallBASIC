// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <stdio.h>
#include "include/var.h"

void error(var_p_t var, const char *field, int nMin, int nMax) {
  char message[256];
  snprintf(message, sizeof(message), "Invalid Input: [%s] - expected [%d - %d] arguments", field, nMin, nMax);
  v_setstr(var, message);
}

void error(var_p_t var, const char *field, int n) {
  char message[256];
  snprintf(message, sizeof(message), "Invalid Input: [%s] - expected [%d] arguments", field, n);
  v_setstr(var, message);
}

void error(var_p_t var, const char *text) {
  char message[256];
  snprintf(message, sizeof(message), "Error: [%s]", text);
  v_setstr(var, message);
}

int get_param_int(int argc, slib_par_t *params, int n, int def) {
  int result;
  if (n >= 0 && n < argc) {
    switch (params[n].var_p->type) {
    case V_INT:
      result = params[n].var_p->v.i;
      break;
    case V_NUM:
      result = params[n].var_p->v.n;
      break;
    default:
      result = def;
    }
  } else {
    result = def;
  }
  return result;
}

const char *get_param_str(int argc, slib_par_t *params, int n, const char *def) {
  const char *result;
  static char buf[256];
  if (n >= 0 && n < argc) {
    switch (params[n].var_p->type) {
    case V_STR:
      result = params[n].var_p->v.p.ptr;
      break;
    case V_INT:
      sprintf(buf, "%lld", params[n].var_p->v.i);
      result = buf;
      break;
    case V_NUM:
      sprintf(buf, "%f", params[n].var_p->v.n);
      result = buf;
      break;
    default:
      result = "";
      break;
    }
  } else {
    result = def == nullptr ? "" : def;
  }
  return result;
}
