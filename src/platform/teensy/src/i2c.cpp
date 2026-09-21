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
#include <Wire.h>

static TwoWire *getI2C(int interfaceNumber) {
  TwoWire *result;
  switch (interfaceNumber) {
  case 0:
    result = &Wire;
    break;
  case 1:
    result = &Wire1;
    break;
  case 2:
    result = &Wire2;
    break;
  default:
    result = &Wire;
    break;
  }
  return result;
}

static int cmd_i2c_write(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  uint8_t address = get_param_int(argc, args, 0, 0);
  uint8_t stop = get_param_int(argc, args, 2, 1);

  if (address == 0 || argc < 2) {
    v_setstr(retval, ERR_PARAM);
    return 0;
  }

  int interfaceNumber = self->v.m.id;
  TwoWire *ptrWire;
  ptrWire = getI2C(interfaceNumber);

  ptrWire->beginTransmission(address);

  switch (args[1].var_p->type) {
  case V_INT:{
      int value = get_param_int(argc, args, 1, 0);
      ptrWire->write(value);
    }
    break;
  case V_STR:{
      const char *buffer = v_getstr(args[1].var_p);
      int length = v_strlen(args[1].var_p);
      ptrWire->write(buffer, length);
    }
    break;
  case V_ARRAY:{
      var_p_t array = args[1].var_p;    // Get array
      if (array->maxdim > 1) {
        v_setstr(retval, "ERROR: I2C: Write requires 1D-array");
        return 0;
      }
      uint32_t bytes = v_ubound(array, 0) - v_lbound(array, 0) + 1;
      uint8_t *buffer = new uint8_t[bytes];
      for (uint32_t ii = 0; ii < bytes; ii++) {
        buffer[ii] = get_array_elem_int(array, ii);
      }
      ptrWire->write(buffer, bytes);
      delete[]buffer;
    }
    break;
  }

  if (ptrWire->endTransmission(stop)) {
    v_setstr(retval, "ERROR I2C: Transmission failed");
    return 0;
  }

  return 1;
}

static int cmd_i2c_read(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  uint8_t address = get_param_int(argc, args, 0, 0);
  uint32_t bytes = get_param_int(argc, args, 1, 1);
  uint8_t stop = get_param_int(argc, args, 2, 1);

  if (address == 0) {
    v_setstr(retval, ERR_PARAM);
    return 0;
  }

  int interfaceNumber = self->v.m.id;
  TwoWire *ptrWire;
  ptrWire = getI2C(interfaceNumber);
  ptrWire->requestFrom(address, bytes, stop);

  if (bytes > 1) {
    v_toarray1(retval, bytes);
    for (uint32_t ii = 0; ii < bytes; ii++) {
      v_setint(v_elem(retval, ii), ptrWire->read());
    }
  } else {
    v_setint(retval, ptrWire->read());
  }
  return 1;
}

static int cmd_i2c_setClock(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  uint32_t clockFrequency = get_param_int(argc, args, 0, 100000);

  if (clockFrequency != 100000 && clockFrequency != 400000 && clockFrequency != 1000000) {
    v_setstr(retval, "ERROR I2C: Clock freuqency not supported");
    return 0;
  }

  int interfaceNumber = self->v.m.id;
  getI2C(interfaceNumber)->setClock(clockFrequency);

  return 1;
}

int cmd_openi2c(int argc, slib_par_t *args, var_t *retval) {
  uint8_t interfaceNumber = get_param_int(argc, args, 0, 0);
  uint8_t pinSDA = get_param_int(argc, args, 1, 0);
  uint8_t pinSCL = get_param_int(argc, args, 2, 0);

  if (interfaceNumber > 2) {
    v_setstr(retval, ERR_PARAM);
    return 0;
  }

  map_init(retval);
  v_create_callback(retval, "write", cmd_i2c_write);
  v_create_callback(retval, "read", cmd_i2c_read);
  v_create_callback(retval, "setClock", cmd_i2c_setClock);
  retval->v.m.id = interfaceNumber;

  TwoWire *ptrWire;
  ptrWire = getI2C(interfaceNumber);

  if (pinSDA > 0 && pinSCL > 0) {
    ptrWire->setSDA(pinSDA);
    ptrWire->setSCL(pinSCL);
  }

  ptrWire->begin();

  return 1;
}
