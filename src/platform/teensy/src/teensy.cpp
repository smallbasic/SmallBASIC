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
#include "languages/messages.en.h"
#include "module.h"
#include "serial.h"

#define USB_OBJECT_ID 1001
#define USB_CLASS_ID 1002

static void set_pin(var_p_t var, uint8_t pin, uint8_t mode) {
  map_init(var);
  var->v.m.id = pin;
  pinMode(pin, mode);
}

static bool is_pin(int id) {
  return id >= 0 && id < CORE_NUM_TOTAL_PINS;
}

static bool is_pin_object(var_p_t var) {
  return var != nullptr && v_is_type(var, V_MAP) && is_pin(var->v.m.id);
}

static bool is_usb_object(var_p_t var) {
  return var != nullptr && v_is_type(var, V_MAP) && (var->v.m.id == USB_OBJECT_ID);
}

static int cmd_analoginput_read(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  int result = 0;
  if (argc != 0 || !is_pin_object(self)) {
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
  if (is_pin(pin)) {
    set_pin(retval, pin, INPUT);
    v_create_callback(retval, "read", cmd_analoginput_read);
  } else {
    result = 0;
  }
  return result;
}

static int cmd_digitalinput_read(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  int result = 0;
  if (argc != 0 || !is_pin_object(self)) {
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
  if (is_pin(pin)) {
    set_pin(retval, pin, INPUT);
    v_create_callback(retval, "read", cmd_digitalinput_read);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static int cmd_digitaloutput_write(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  int result = 0;
  if (argc != 1 || !is_pin_object(self)) {
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
  if (is_pin(pin)) {
    set_pin(retval, pin, OUTPUT);
    v_create_callback(retval, "write", cmd_digitaloutput_write);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static int cmd_get_temperature(int argc, slib_par_t *params, var_t *retval) {
  v_setint(retval, tempmonGetTemp());
  return 1;
}

static int cmd_get_cpu_speed(int argc, slib_par_t *params, var_t *retval) {
  v_setint(retval, F_CPU_ACTUAL/1000000);
  return 1;
}

static int cmd_usb_ready(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_usb_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    v_setint(retval, Serial.available());
    result = 1;
  }
  return result;
}

static int cmd_usb_receive(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_usb_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    char buffer[CDC_RX_SIZE_480];
    int size = Serial.readBytes(buffer, CDC_RX_SIZE_480);
    buffer[size] = '\0';
    v_setstr(retval, buffer);
    result = 1;
  }
  return result;
}

static int cmd_usb_send(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 1 || !is_usb_object(self) || !v_is_type(args[0].var_p, V_STR)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    const char *buffer = v_getstr(args[0].var_p);
    int length = v_strlen(args[0].var_p);
    Serial.write(buffer, length);
    result = 1;
  }
  return result;
}

static int cmd_openserial(int argc, slib_par_t *params, var_t *retval) {
  map_init(retval);
  retval->v.m.id = USB_OBJECT_ID;
  retval->v.m.cls_id = USB_CLASS_ID;
  v_create_callback(retval, "ready", cmd_usb_ready);
  v_create_callback(retval, "receive", cmd_usb_receive);
  v_create_callback(retval, "send", cmd_usb_send);
  serial_init();
  return 1;
}

static FuncSpec lib_func[] = {
  {0, 0, "GETTEMP", cmd_get_temperature},
  {0, 0, "GETCPUSPEED", cmd_get_cpu_speed},
  {1, 1, "OPENANALOGINPUT", cmd_openanaloginput},
  {1, 1, "OPENDIGITALINPUT", cmd_opendigitalinput},
  {1, 1, "OPENDIGITALOUTPUT", cmd_opendigitaloutput},
  {0, 0, "OPENUSBSERIAL", cmd_openserial},
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
