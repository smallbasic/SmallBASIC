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
  maScreenSetOrientation(SCREEN_ORIENTATION_DYNAMIC);
  maScreenSetFullscreen(0);
  
  controller = new Controller();
  controller->pause(500); // de-bounce screen events
  controller->construct();

  bool home = true;
  sbasic_main(MAIN_BAS_RES);
  while (!controller->isExit()) {
    if (controller->isBack()) {
      if (home) {
        controller->setExit(false);
      } else {
        sbasic_main(MAIN_BAS_RES);
        home = true;
      }
    } else if (controller->getLoadPath() != NULL) {
      sbasic_main(controller->getLoadPath());
      home = false;
    } else {
      controller->setRunning(false);
      controller->processEvents(-1, -1);
    }
  }

  delete controller;
  return 0;
}
