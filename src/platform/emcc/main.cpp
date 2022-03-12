// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "include/osd.h"
#include "common/sbapp.h"
#include "common/device.h"
#include "platform/emcc/runtime.h"

void appLog(const char *format, ...) {
  va_list args;
  va_start(args, format);
  unsigned size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (size) {
    char *buf = (char *)malloc(size + 3);
    buf[0] = '\0';
    va_start(args, format);
    vsnprintf(buf, size + 1, format, args);
    va_end(args);
    buf[size] = '\0';

    int i = size - 1;
    while (i >= 0 && isspace(buf[i])) {
      buf[i--] = '\0';
    }
    strcat(buf, "\r\n");
    fputs(buf, stderr);
    free(buf);
  }
}

void init() {
  opt_command[0] = '\0';
  opt_file_permitted = 0;
  opt_graphics = 1;
  opt_ide = 0;
  opt_modpath[0] = '\0';
  opt_nosave = 1;
  opt_pref_height = 0;
  opt_pref_width = 0;
  opt_quiet = 1;
  opt_verbose = 0;
  opt_autolocal = 0;
  os_graf_mx = 1024;
  os_graf_my = 768;
}

int main(int argc, char* argv[]) {
  logEntered();

  init();
  Runtime *runtime = new Runtime();
  runtime->runShell();
  delete runtime;

  logLeaving();
  return 0;
}
