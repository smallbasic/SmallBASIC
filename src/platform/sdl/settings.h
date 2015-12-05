// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SETTINGS_H
#define SETTINGS_H

#include <SDL.h>

void restoreSettings(SDL_Rect &rect, int &fontScale, bool debug, bool restoreDir);
void saveSettings(SDL_Window *window, int fontScale, bool debug);
void setRecentFile(const char *path);
bool getRecentFile(strlib::String &path, unsigned position);

#endif

