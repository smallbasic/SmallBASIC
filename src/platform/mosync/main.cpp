// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <ma.h>
#include <MAUtil/Environment.h>
#include <limits.h>

#include "stdio.h"
#include "config.h"
#include "MAHeaders.h"

#include "platform/mosync/ansiwidget.h"
#include "platform/mosync/utils.h"
#include "common/sbapp.h"
#include "common/osd.h"

AnsiWidget *output;
ExecState runMode;

void setupOptions() {
  opt_ide = IDE_NONE;
  opt_graphics = true;
  opt_pref_bpp = 0;
  opt_nosave = true;
  opt_interactive = true;
  opt_verbose = false;
  opt_quiet = true;
  opt_command[0] = 0;
  os_graphics = 1;
}

struct Environ : Environment {
  Environ() : Environment() {
  };
};

extern "C" int MAMain() {
  Environ e;
  MAExtent screenSize = maGetScrSize();
  output = new AnsiWidget(EXTENT_X(screenSize), EXTENT_Y(screenSize));
  output->construct();
  setupOptions();

  char path[PATH_MAX];
  runMode = init_state;

  do {
    //chdir(path);
    int success = sbasic_main("test.bas");
    if (!success) {
      
    }
  }
  while (runMode != quit_state);

  delete output;
  return 0;
}
