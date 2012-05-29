// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <ma.h>

#include "platform/mosync/controller.h"
#include "platform/mosync/utils.h"

Controller *controller;

extern "C" void trace(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof buf - 1, format, args);
  va_end(args);

  while (p > buf && isspace(p[-1])) {
    *--p = '\0';
  }

  *p++ = '\r';
  *p++ = '\n';
  *p = '\0';

  maWriteLog(buf, strlen(buf));
}

extern "C" int MAMain() {
  controller = new Controller();

  // remember the initial path
  //   char path[PATH_MAX];

  //  do {
    // restore initial path
    //chdir(path);
    //    int success = sbasic_main("http://smallbasic.sourceforge.net/?q=export/code/827");//MAIN_BAS_RES);
    int success = sbasic_main(MAIN_BAS_RES);
    if (!success) {
      // allow the user to view any error until they
      // touch to continue
    }
    //  }
//  while (!controller->isExit());

  delete controller;
  return 0;
}
