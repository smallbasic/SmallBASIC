// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <ma.h>

#include "stdio.h"
#include "config.h"
#include "MAHeaders.h"

#include "platform/mosync/ansiwidget.h"
#include "platform/mosync/utils.h"
#include "common/sbapp.h"

AnsiWidget *output;

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

extern "C" int MAMain() {
  MAExtent screenSize = maGetScrSize();
  output = new AnsiWidget(EXTENT_X(screenSize), EXTENT_Y(screenSize));
  output->construct();
  output->print("Welcome to SmallBASIC");
  setupOptions();

  char path[MAX_PATH];
  int success;
  int restart;

  do {
    restart = false;
    //chdir(path);
    success = sbasic_main("main.bas");
  }
  while (restart);

  delete output;
  return 0;
}
