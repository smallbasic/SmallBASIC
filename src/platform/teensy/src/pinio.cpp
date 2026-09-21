// This file is part of SmallBASIC
//
// Copyright(C) 2026 Joerg Siebenmorgen
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <Arduino.h>
#include "config.h"
#include "languages/messages.en.h"
#include "include/var_map.h"
#include "module.h"

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

int cmd_openanaloginput(int argc, slib_par_t *args, var_t *retval) {
  int result = 1;
  int pin = get_param_int(argc, args, 0, -1);
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

int cmd_opendigitalinput(int argc, slib_par_t *args, var_t *retval) {
  int result = 1;
  int pin = get_param_int(argc, args, 0, -1);
  uint8_t mode = get_param_int(argc, args, 1, 1);
  if (mode) {
    mode = INPUT_PULLUP;
  } else {
    mode = INPUT;
  }
  if (is_pin(pin)) {
    set_pin(retval, pin, mode);
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

int cmd_opendigitaloutput(int argc, slib_par_t *args, var_t *retval) {
  int result;
  int pin = get_param_int(argc, args, 0, -1);
  if (is_pin(pin)) {
    set_pin(retval, pin, OUTPUT);
    v_create_callback(retval, "write", cmd_digitaloutput_write);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

static int cmd_analogoutput_write(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  if (argc != 1 || !is_pin_object(self)) {
    error(retval, "AnalogOutput.write", 1);
    return 0;
  } else {
    int pin = self->v.m.id;
    int value = get_param_int(argc, arg, 0, 0);
    analogWrite(pin, value);
  }
  return 1;
}

static int cmd_analogoutput_frequency(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  if (argc != 1 || !is_pin_object(self)) {
    error(retval, "AnalogOutput.frequency", 1);
    return 0;
  } else {
    int pin = self->v.m.id;
    double value = get_param_num(argc, arg, 0, 0.0);
    analogWriteFrequency(pin, value);
  }
  return 1;
}

static int cmd_analogoutput_resolution(var_s *self, int argc, slib_par_t *arg, var_s *retval) {
  if (argc != 1 || !is_pin_object(self)) {
    error(retval, "AnalogOutput.resolution", 1);
    return 0;
  } else {
    int value = get_param_int(argc, arg, 0, 0);
    analogWriteResolution(value);
  }
  return 1;
}

int cmd_openanalogoutput(int argc, slib_par_t *args, var_t *retval) {
  int result;
  int pin = get_param_int(argc, args, 0, -1);
  if (is_pin(pin)) {
    set_pin(retval, pin, OUTPUT);
    v_create_callback(retval, "write", cmd_analogoutput_write);
    v_create_callback(retval, "frequency", cmd_analogoutput_frequency);
    v_create_callback(retval, "resolution", cmd_analogoutput_resolution);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}
