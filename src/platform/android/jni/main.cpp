/// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "platform/common/utils.h"
#include "platform/android/jni/runtime.h"

void android_main(android_app *app) {
  // make sure glue isn't stripped.
  app_dummy();

  Runtime *runtime = new Runtime(app);
  runtime->construct();
  runtime->runShell();
  delete runtime;
}
