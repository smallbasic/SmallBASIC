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
#include <Wire.h>

#define USB_CLASS_ID 1002
#define MAX_HW_SERIAL 7
#define BT_BAUD 9600

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

static bool is_serial(int id) {
  return id >= 0 && id <= MAX_HW_SERIAL;
}

static bool is_serial_var(var_p_t var) {
  return var != nullptr && v_is_type(var, V_INT) && is_serial(var->v.i);
}

static bool is_serial_object(var_p_t var) {
  return var != nullptr && v_is_type(var, V_MAP) && is_serial(var->v.m.id);
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

static int cmd_openanaloginput(int argc, slib_par_t *args, var_t *retval) {
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

static int cmd_opendigitalinput(int argc, slib_par_t *args, var_t *retval) {
  int result = 1;
  int pin = get_param_int(argc, args, 0, -1);
  uint8_t mode = get_param_int(argc, args, 1, 1);
  if(mode) {
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

static int cmd_opendigitaloutput(int argc, slib_par_t *args, var_t *retval) {
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

static int cmd_openanalogoutput(int argc, slib_par_t *args, var_t *retval) {
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

static int cmd_get_temperature(int argc, slib_par_t *args, var_t *retval) {
  v_setint(retval, tempmonGetTemp());
  return 1;
}

static int cmd_get_cpu_speed(int argc, slib_par_t *args, var_t *retval) {
  v_setint(retval, F_CPU_ACTUAL/1000000);
  return 1;
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
    auto bufferSize = get_param_int(argc, args, 0, CDC_RX_SIZE_480);
    if (!bufferSize) {
      bufferSize = CDC_RX_SIZE_480;
    }
    char buffer[bufferSize];
    int size;
    int serialNo = self->v.m.id;

    if (serialNo == 0) {
      size = Serial.readBytes(buffer, bufferSize);
    } else {
      size = getSerial(serialNo)->readBytes(buffer, bufferSize);
    }
    buffer[size] = '\0';
    v_setstr(retval, buffer);
    result = 1;
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

static int cmd_openserial(int argc, slib_par_t *args, var_t *retval) {
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
      getSerial(serialNo)->begin(BT_BAUD);
      break;
    }
    result = 1;
  }
  return result;
}

static int cmd_i2c_write(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  uint8_t address = get_param_int(argc, args, 0, 0);
  uint8_t stop    = get_param_int(argc, args, 2, 1);

  if (address == 0 || argc < 2) {
    v_setstr(retval, ERR_PARAM);
    return 0;
  }

  int interfaceNumber = self->v.m.id;
  TwoWire *ptrWire;
  ptrWire = getI2C(interfaceNumber);

  ptrWire->beginTransmission(address);

  switch(args[1].var_p->type) {
    case V_INT: {
      int value = get_param_int(argc, args, 1, 0);
      ptrWire->write(value);
    }
    break;
    case V_STR: {
      const char *buffer = v_getstr(args[1].var_p);
      int length = v_strlen(args[1].var_p);
      ptrWire->write(buffer, length);
    }
    break;
    case V_ARRAY: {
      var_p_t array = args[1].var_p;  //Get array
      if (array->maxdim > 1) {
        v_setstr(retval, "ERROR: I2C: Write requires 1D-array");
        return 0;
      }
      uint32_t bytes  = v_ubound(array, 0) - v_lbound(array, 0) + 1;
      uint8_t *buffer = new uint8_t[bytes];
      for (uint32_t ii = 0; ii < bytes; ii++) {
        buffer[ii] = get_array_elem_int(array, ii);
      }
      ptrWire->write(buffer, bytes);
      delete[] buffer;
    }
    break;
  }

  if(ptrWire->endTransmission(stop)) {
    v_setstr(retval, "ERROR I2C: Transmission failed");
    return 0;
  }

  return 1;
}

static int cmd_i2c_read(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  uint8_t  address = get_param_int(argc, args, 0, 0);
  uint32_t bytes   = get_param_int(argc, args, 1, 1);
  uint8_t  stop    = get_param_int(argc, args, 2, 1);

  if (address == 0) {
    v_setstr(retval, ERR_PARAM);
    return 0;
  }

  uint8_t *buffer = new uint8_t[bytes];
  int interfaceNumber = self->v.m.id;
  TwoWire *ptrWire;
  ptrWire = getI2C(interfaceNumber);

  ptrWire->requestFrom(address, bytes, stop);
  ptrWire->readBytes(buffer, bytes);

  if (bytes > 1) {
    v_toarray1(retval, bytes);
    for (uint32_t ii = 0; ii < bytes; ii++) {
      v_setint(v_elem(retval, ii), buffer[ii]);
    }
  }
  else {
    v_setint(retval, buffer[0]);
  }

  delete[] buffer;
  return 1;
}

static int cmd_i2c_setClock(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  uint32_t  clockFrequency = get_param_int(argc, args, 0,  100000);

  if (clockFrequency != 100000 && clockFrequency != 400000 && clockFrequency != 1000000) {
    v_setstr(retval, "ERROR I2C: Clock freuqency not supported");
    return 0;
  }

  int interfaceNumber = self->v.m.id;
  getI2C(interfaceNumber)->setClock(clockFrequency);

  return 1;
}

static int cmd_openi2c(int argc, slib_par_t *args, var_t *retval) {
  uint8_t  interfaceNumber = get_param_int(argc, args, 0, 0);
  uint8_t  pinSDA = get_param_int(argc, args, 1, 0);
  uint8_t  pinSCL = get_param_int(argc, args, 2, 0);

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

static int cmd_set_interactive(int argc, slib_par_t *args, var_t *retval) {
  uint8_t  mode = get_param_int(argc, args, 0, 1);

  if (mode > 0) {
    setInteractive(1);
  } else {
    setInteractive(0);
  }

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
  {0, 3, "OPENI2C", cmd_openi2c}
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

static ModuleConfig teensyModule = {
  ._func_exec = teensy_func_exec,
  ._func_count = teensy_func_count,
  ._func_getname = teensy_func_getname,
  ._proc_exec = teensy_proc_exec,
  ._proc_count = teensy_proc_count,
  ._proc_getname = teensy_proc_getname,
  ._free = nullptr
};

ModuleConfig *get_teensy_module() {
  return &teensyModule;
}
