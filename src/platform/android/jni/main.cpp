/// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "platform/android/jni/runtime.h"

void android_main(android_app *app) {
  logEntered();

  // make sure glue isn't stripped.
  app_dummy();

  Runtime *runtime = new Runtime(app);

  // pump events until startup has completed
  while (runtime->isInitial()) {
    runtime->pollEvents(true);
  }
  if (!runtime->isClosing()) {
    runtime->runShell();
  }

  ANativeActivity_finish(app->activity);
  do {
    runtime->pollEvents(true);
  }
  while (app->destroyRequested != 0);

  delete runtime;
  logLeaving();
  exit(0);
}
