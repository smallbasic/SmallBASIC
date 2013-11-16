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
    int events;
    android_poll_source* source = NULL;
    while (ALooper_pollAll(10, NULL, &events, (void**)&source) >= 0) {
      if (source != NULL) {
        source->process(app, source);
      }
    }
  }
  if (!runtime->isClosing()) {
    runtime->runShell();
  }
  delete runtime;
}
