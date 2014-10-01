// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SETTINGS_H
#define SETTINGS_H

#include <SDL.h>

void restoreSettings(const char *configName, SDL_Rect &rect, int &fontScale);
void saveSettings(const char *configName, SDL_Window *window, int fontScale);

#endif

