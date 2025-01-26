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
#include "include/var_map.h"
#include "module.h"

static bool isPin(int id) {
  return id >= 0 && id < CORE_NUM_TOTAL_PINS;
}

static bool isObject(var_p_t var) {
  return var != nullptr && v_is_type(var, V_MAP) && isPin(var->v.m.id);
}

static void setPin(var_p_t var, uint8_t pin, uint8_t mode) {
  map_init(var);
  var->v.m.id = pin;
  pinMode(pin, mode);
}

static int cmd_analoginput_read(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  int result = 0;
  if (argc != 0 || !isObject(self)) {
    error(retval, "AnalogInput.read", 0);
  } else {
    int pin = self->v.m.id;
    v_setint(retval, analogRead(pin));
    result = 1;
  }
  return result;
}

static int cmd_openanaloginput(int argc, slib_par_t *params, var_t *retval) {
  int result = 1;
  int pin = get_param_int(argc, params, 0, -1);
  if (isPin(pin)) {
    setPin(retval, pin, INPUT);
    v_create_callback(retval, "read", cmd_analoginput_read);
  } else {
    result = 0;
  }
  return result;
}

static int cmd_digitalinput_read(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  int result = 0;
  if (argc != 0 || !isObject(self)) {
    error(retval, "DigitalInput.read", 0);
  } else {
    int pin = self->v.m.id;
    v_setint(retval, digitalRead(pin));
    result = 1;
  }
  return result;
}

static int cmd_opendigitalinput(int argc, slib_par_t *params, var_t *retval) {
  int result = 1;
  int pin = get_param_int(argc, params, 0, -1);
  if (isPin(pin)) {
    setPin(retval, pin, INPUT);
    v_create_callback(retval, "read", cmd_digitalinput_read);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static int cmd_digitaloutput_write(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  int result = 0;
  if (argc != 1 || !isObject(self)) {
    error(retval, "DigitalOutput.write", 1);
  } else {
    int pin = self->v.m.id;
    int value = get_param_int(argc, arg, 0, 0);
    digitalWrite(pin, value);
    result = 1;
  }
  return result;
}

static int cmd_opendigitaloutput(int argc, slib_par_t *params, var_t *retval) {
  int result;
  int pin = get_param_int(argc, params, 0, -1);
  if (isPin(pin)) {
    setPin(retval, pin, OUTPUT);
    v_create_callback(retval, "write", cmd_digitaloutput_write);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static FuncSpec lib_func[] = {
  {1, 1, "OPENANALOGINPUT", cmd_openanaloginput},
  {1, 1, "OPENDIGITALINPUT", cmd_opendigitalinput},
  {1, 1, "OPENDIGITALOUTPUT", cmd_opendigitaloutput},
};

static int teensy_func_count(void) {
  return (sizeof(lib_func) / sizeof(lib_func[0]));
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

static int teensy_func_exec(int index, int argc, slib_par_t *params, var_t *retval) {
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
      result = lib_func[index]._command(argc, params, retval);
    }
  } else {
    error(retval, "FUNC index error");
    result = 0;
  }
  return result;
}

static ModuleConfig teensyModule = {
  ._func_exec = teensy_func_exec,
  ._func_count = teensy_func_count,
  ._func_getname = teensy_func_getname,
  ._proc_exec = nullptr,
  ._proc_count = nullptr,
  ._proc_getname = nullptr,
  ._free = nullptr
};

ModuleConfig *get_teensy_module() {
  return &teensyModule;
}
