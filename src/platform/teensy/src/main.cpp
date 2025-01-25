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
#include "main_bas.h"
#include "module.h"

#define MAIN_BAS "__main_bas__"

void *plugin_lib_open(const char *name) {
  void *result = nullptr;
  if (strcmp(name, "/libteensy") == 0) {
    result = get_teensy_module();
  }
  return result;
}

void *plugin_lib_address(void *handle, const char *name) {
  auto *pModule = (s_module *)handle;
  void *result = nullptr;
  if (strcmp(name, "sblib_func_exec") == 0) {
    result = (void *)pModule->_func_exec;
  } else if (strcmp(name, "sblib_proc_exec") == 0) {
    result = (void *)pModule->_proc_exec;
  } else if (strcmp(name, "sblib_free") == 0) {
    result = (void *)pModule->_free;
  } else if (strcmp(name, "sblib_proc_count")  == 0){
    result = (void *)pModule->_proc_count;
  } else if (strcmp(name, "sblib_proc_getname") == 0) {
    result = (void *)pModule->_proc_getname;
  } else if (strcmp(name, "sblib_func_count") == 0) {
    result = (void *)pModule->_func_count;
  } else if (strcmp(name, "sblib_func_getname") == 0) {
    result = (void *)pModule->_func_getname;
  }
  return result;
}

void plugin_lib_close(void *handle) {
  // unused
}

char *dev_read(const char *fileName) {
  char *buffer;
  if (strcmp(fileName, MAIN_BAS) == 0) {
    buffer = (char *)malloc(main_bas_len + 1);
    memcpy(buffer, main_bas, main_bas_len);
    buffer[main_bas_len] = '\0';
  } else {
    buffer = nullptr;
  }
  return buffer;
}

int sys_search_path(const char *path, const char *file, char *retbuf) {
  int result;
  if (strcmp(file, "libteensy") == 0) {
    strcpy(retbuf, "/");
    result = 1;
  }
  return result;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // wait
  }

  opt_autolocal = 0;
  opt_command[0] = '\0';
  opt_modpath[0] = '\0';
  opt_file_permitted = 0;
  opt_ide = 0;
  opt_nosave = 1;
  opt_pref_height = 0;
  opt_pref_width = 0;
  opt_quiet = 1;
  opt_verbose = 0;
  opt_graphics = 0;
}

extern "C" int main(void) {
  setup();
  if (!sbasic_main(MAIN_BAS)) {
    dev_print("Error: run failed\n");
    opt_quiet = 0;
    opt_verbose = 1;
    sbasic_main(MAIN_BAS);
  } else {
    dev_print("main.bas ended");
  }
}
