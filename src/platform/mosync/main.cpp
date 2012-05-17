// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <ma.h>
#include <conprint.h>

#include "ansiwidget.h"

AnsiWidget *output;

extern "C" int MAMain() {
  InitConsole();
	gConsoleLogging = 1;

  printf("there");
  MAExtent screenSize = maGetScrSize();
  output = new AnsiWidget(EXTENT_X(screenSize), EXTENT_Y(screenSize));
  output->construct();

  output->print("hello");

	bool run = true;
	bool focus = true;
	while (run) {
		if (focus) {
      
		} else {
			maWait(0);
		}

		// Get any available events.
		// If MAK_FIRE is pressed, change mode.
		// On Close event or MAK_0 press, close program.
		MAEvent event;
		while (maGetEvent(&event)) {
      switch (event.type) {
      case EVENT_TYPE_KEY_PRESSED:
				switch(event.key) {
        case MAK_FIRE:
        case MAK_5:
          break;
        case MAK_SOFTRIGHT:
        case MAK_0:
        case MAK_BACK:
          run = false;
          break;
				}
        break;
      case EVENT_TYPE_SCREEN_CHANGED:
        break;
      case EVENT_TYPE_POINTER_PRESSED:
        break;
			case EVENT_TYPE_CLOSE:
				run = false;
        break;
			case EVENT_TYPE_FOCUS_LOST:
				focus = false;
        break;
      case EVENT_TYPE_FOCUS_GAINED:
				focus = true;
      default:
        break;
			}
		}
	}
	return 0;
}
