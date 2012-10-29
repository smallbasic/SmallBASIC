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
  controller = new Controller();
  controller->construct();
 
  bool mainBas = true;
  sbasic_main("main.bas?welcome");

  while (!controller->isExit()) {
    if (controller->isBack()) {
      if (mainBas) {
        if (!opt_command[0] || !opt_command[1] || 
            !strcmp(opt_command, "welcome")) {
          controller->setExit(false);
        } else {
          opt_command[0] = '\0';
        }
      }
      if (!controller->isExit()) {
        mainBas = true;
        sbasic_main("main.bas");
      }
    } else if (controller->getLoadPath() != NULL) {
      mainBas = (strncmp(controller->getLoadPath(), "main.bas", 8) == 0);
      if (!mainBas) {
        set_path(controller->getLoadPath());
      }
      bool success = sbasic_main(controller->getLoadPath());
      if (!controller->isBack()) {
        if (!mainBas) {
          // display an indication the program has completed
          controller->showCompletion(success);
        }
        if (!success) {
          // highlight the error
          controller->showError();
        }
      }
    } else {
      controller->setRunning(false);
      controller->processEvents(-1, -1);
    }
  }

  delete controller;
  return 0;
}
