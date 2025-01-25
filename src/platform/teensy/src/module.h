// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

void error(var_p_t var, const char *field, int nMin, int nMax);
void error(var_p_t var, const char *field, int n);
void error(var_p_t var, const char *text);
int get_param_int(int argc, slib_par_t *params, int n, int def);

typedef struct API {
  const char *name;
  int (*command)(int, slib_par_t *, var_t *retval);
} API;

typedef struct FUNC_SIG {
  int _min;
  int _max;
  const char *_name;
  int (*_command)(int, slib_par_t *, var_t *retval);
} FUNC_SIG;

typedef int (*sblib_exec_fn)(int, int, slib_par_t *, var_t *);
typedef int (*sblib_getname_fn) (int, char *);
typedef int (*sblib_count_fn) (void);
typedef int (*sblib_init_fn) (const char *);
typedef int (*sblib_free_fn) (int, int);
typedef void (*sblib_close_fn) (void);

typedef struct {
  sblib_exec_fn _func_exec;
  sblib_count_fn _func_count;
  sblib_getname_fn _func_getname;
  sblib_exec_fn _proc_exec;
  sblib_count_fn _proc_count;
  sblib_getname_fn _proc_getname;
  sblib_free_fn _free;
} s_module;

s_module *get_teensy_module();

