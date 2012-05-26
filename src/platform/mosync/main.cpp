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

extern "C" int MAMain() {
  controller = new Controller();

  // remember the initial path
  //   char path[PATH_MAX];

  do {
    // restore initial path
    //chdir(path);
    int success = sbasic_main(MAIN_BAS_RES);
    if (!success) {
      // allow the user to view any error until they
      // touch to continue
    }
  }
  while (!controller->isExit());

  delete controller;
  return 0;
}
