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
  controller->construct();

  sbasic_main(MAIN_BAS_RES);
  while (!controller->isExit()) {
    controller->processEvents(-1, -1);
    if (controller->getLoadPath() != NULL) {
      sbasic_main(controller->getLoadPath());
    } else if (!controller->hasUI()) {
      sbasic_main(MAIN_BAS_RES);
    }
  }

  delete controller;
  return 0;
}
