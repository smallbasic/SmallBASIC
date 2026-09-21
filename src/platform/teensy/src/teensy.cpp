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
#include "languages/messages.en.h"
#include "include/var_map.h"
#include "common/var.h"
#include "common/device.h"
#include "device.h"
#include "module.h"
#include "serial.h"
#include "i2c.h"
#include "pinio.h"
#include "fs.h"
#include <Wire.h>
#include <malloc.h>

static int cmd_get_temperature(int argc, slib_par_t *args, var_t *retval) {
  v_setint(retval, tempmonGetTemp());
  return 1;
}

static int cmd_get_cpu_speed(int argc, slib_par_t *args, var_t *retval) {
  v_setint(retval, F_CPU_ACTUAL / 1000000);
  return 1;
}

static int cmd_set_interactive(int argc, slib_par_t *args, var_t *retval) {
  uint8_t mode = get_param_int(argc, args, 0, 1);

  if (mode > 0) {
    setInteractive(1);
  } else {
    setInteractive(0);
  }

  return 1;
}

static int cmd_free(int argc, slib_par_t *args, var_t *retval) {
  extern char _ebss[], _heap_end[], *__brkval;
  char *sp = (char *)__builtin_frame_address(0);
  auto stack = (sp - _ebss), heap = (_heap_end - __brkval);

  v_toarray1(retval, 2);
  v_setint(v_elem(retval, 0), stack);
  v_setint(v_elem(retval, 1), heap);

  return 1;
}

static FuncSpec lib_func[] = {
  {0, 0, "GETTEMP", cmd_get_temperature},
  {0, 0, "GETCPUSPEED", cmd_get_cpu_speed},
  {1, 1, "OPENANALOGINPUT", cmd_openanaloginput},
  {1, 1, "OPENANALOGOUTPUT", cmd_openanalogoutput},
  {1, 1, "OPENDIGITALINPUT", cmd_opendigitalinput},
  {1, 1, "OPENDIGITALOUTPUT", cmd_opendigitaloutput},
  {0, 1, "OPENSERIAL", cmd_openserial},
  {0, 3, "OPENI2C", cmd_openi2c},
  {0, 0, "FS", cmd_fs},
  {0, 0, "FREE", cmd_free}
};

static FuncSpec lib_proc[] = {
  {0, 1, "SETINTERACTIVE", cmd_set_interactive}
};

static int teensy_func_count(void) {
  return (sizeof(lib_func) / sizeof(lib_func[0]));
}

static int teensy_proc_count(void) {
  return (sizeof(lib_proc) / sizeof(lib_proc[0]));
}

static int teensy_func_getname(int index, char *func_name) {
  int result;
  if (index < teensy_func_count()) {
    strcpy(func_name, lib_func[index]._name);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static int teensy_proc_getname(int index, char *proc_name) {
  int result;
  if (index < teensy_proc_count()) {
    strcpy(proc_name, lib_proc[index]._name);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static int teensy_func_exec(int index, int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (index >= 0 && index < teensy_func_count()) {
    if (argc < lib_func[index]._min || argc > lib_func[index]._max) {
      if (lib_func[index]._min == lib_func[index]._max) {
        error(retval, lib_func[index]._name, lib_func[index]._min);
      } else {
        error(retval, lib_func[index]._name, lib_func[index]._min, lib_func[index]._max);
      }
      result = 0;
    } else {
      result = lib_func[index]._command(argc, args, retval);
    }
  } else {
    error(retval, "FUNC index error");
    result = 0;
  }
  return result;
}

static int teensy_proc_exec(int index, int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (index >= 0 && index < teensy_proc_count()) {
    if (argc < lib_proc[index]._min || argc > lib_proc[index]._max) {
      if (lib_proc[index]._min == lib_proc[index]._max) {
        error(retval, lib_proc[index]._name, lib_proc[index]._min);
      } else {
        error(retval, lib_proc[index]._name, lib_proc[index]._min, lib_proc[index]._max);
      }
      result = 0;
    } else {
      result = lib_proc[index]._command(argc, args, retval);
    }
  } else {
    error(retval, "PROC index error");
    result = 0;
  }
  return result;
}

void teensy_close(void) {
  fs_close();
}

static ModuleConfig teensyModule = {
  ._func_exec = teensy_func_exec,
  ._func_count = teensy_func_count,
  ._func_getname = teensy_func_getname,
  ._proc_exec = teensy_proc_exec,
  ._proc_count = teensy_proc_count,
  ._proc_getname = teensy_proc_getname,
  ._free = nullptr,
  ._close = teensy_close
};

ModuleConfig *get_teensy_module() {
  return &teensyModule;
}
