// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <Arduino.h>

#include "config.h"
#include "common/sbapp.h"
#include "module.h"

static int cmd_openanaloginput(int argc, slib_par_t *params, var_t *retval) {
  return 0;
}

static int cmd_opendigitalinput(int argc, slib_par_t *params, var_t *retval) {
  return 0;
}

static int cmd_opendigitaloutput(int argc, slib_par_t *params, var_t *retval) {
  return 0;
}

FUNC_SIG lib_func[] = {
  {1, 1, "OPENANALOGINPUT", cmd_openanaloginput},
  {1, 1, "OPENDIGITALINPUT", cmd_opendigitalinput},
  {1, 1, "OPENDIGITALOUTPUT", cmd_opendigitaloutput},
};

int ssd1306_proc_count(void) {
  return 0;
}

int ssd1306_proc_getname(int index, char *proc_name) {
  return 0;
}

int ssd1306_proc_exec(int index, int param_count, slib_par_t *params, var_t *retval) {
  return 0;
}

int ssd1306_func_count(void) {
  return (sizeof(lib_func) / sizeof(lib_func[0]));
}

int ssd1306_func_getname(int index, char *func_name) {
  return 0;
}

int ssd1306_func_exec(int index, int param_count, slib_par_t *params, var_t *retval) {
  return 0;
}

int ssd1306_free(int cls_id, int id) {
  return 0;
}
