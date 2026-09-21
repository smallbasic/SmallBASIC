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
#include "serial.h"

#define USB_CLASS_ID 1002
#define MAX_HW_SERIAL 7
#define SERIAL_DEFAULT_BAUD 9600

static HardwareSerialIMXRT *getSerial(int serialNo) {
  HardwareSerialIMXRT *result;
  switch (serialNo) {
  case 1:
    result = &Serial1;
    break;
  case 2:
    result = &Serial2;
    break;
  case 3:
    result = &Serial3;
    break;
  case 4:
    result = &Serial4;
    break;
  case 5:
    result = &Serial5;
    break;
  case 6:
    result = &Serial6;
    break;
  case 7:
    result = &Serial7;
    break;
  default:
    result = &Serial1;
    break;
  }
  return result;
}

static bool is_serial(int id) {
  return id >= 0 && id <= MAX_HW_SERIAL;
}

static bool is_serial_var(var_p_t var) {
  return var != nullptr && v_is_type(var, V_INT) && is_serial(var->v.i);
}

static bool is_serial_object(var_p_t var) {
  return var != nullptr && v_is_type(var, V_MAP) && is_serial(var->v.m.id);
}

static int cmd_serial_ready(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_serial_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    int serialNo = self->v.m.id;
    if (serialNo == 0) {
      v_setint(retval, Serial.available());
    } else {
      v_setint(retval, getSerial(serialNo)->available());
    }
    result = 1;
  }
  return result;
}

static int cmd_serial_receive(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc > 1 || !is_serial_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    int bufferSize = get_param_int(argc, args, 0, 0);
    int size;
    int serialNo = self->v.m.id;

    if (bufferSize == 0) {
      // Read until '\n' and return as string
      bufferSize = CDC_RX_SIZE_480;
      char buffer[bufferSize];

      if (serialNo == 0) {
        size = Serial.readBytesUntil('\n', buffer, bufferSize - 1);
      } else {
        size = getSerial(serialNo)->readBytesUntil('\n', buffer, bufferSize - 1);
      }
      buffer[size] = '\0';
      v_setstr(retval, buffer);
      result = 1;
    } else {
      // Read number of bytes and return as array
      char buffer[bufferSize];
      if (serialNo == 0) {
        size = Serial.readBytes(buffer, bufferSize);
      } else {
        size = getSerial(serialNo)->readBytes(buffer, bufferSize);
      }
      if (bufferSize > 1) {
        v_toarray1(retval, bufferSize);
        for (int32_t ii = 0; ii < size; ii++) {
          v_setint(v_elem(retval, ii), buffer[ii]);
        }
      } else {
        v_setint(retval, buffer[0]);
      }
      result = 1;
    }
  }
  return result;
}

static int cmd_serial_send(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 1 || !is_serial_object(self) || !v_is_type(args[0].var_p, V_STR)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    const char *buffer = v_getstr(args[0].var_p);
    int length = v_strlen(args[0].var_p);
    int serialNo = self->v.m.id;
    switch (serialNo) {
    case 0:
      Serial.write(buffer, length);
      break;
    default:
      getSerial(serialNo)->write(buffer, length);
      break;
    }
    result = 1;
  }
  return result;
}

int cmd_openserial(int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (!(argc == 0 || (argc == 1 && is_serial_var(args[0].var_p)))) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    int serialNo = argc == 0 ? 0 : args[0].var_p->v.i;
    map_init(retval);
    retval->v.m.id = serialNo;
    retval->v.m.cls_id = USB_CLASS_ID;
    v_create_callback(retval, "ready", cmd_serial_ready);
    v_create_callback(retval, "receive", cmd_serial_receive);
    v_create_callback(retval, "send", cmd_serial_send);
    switch (serialNo) {
    case 0:
      serial_init();
      break;
    default:
      int serialSpeed = get_param_int(argc, args, 1, SERIAL_DEFAULT_BAUD);
      getSerial(serialNo)->begin(serialSpeed);
      break;
    }
    result = 1;
  }
  return result;
}