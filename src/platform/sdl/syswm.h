// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SB_SYSWM_H
#define SB_SYSWM_H

#include <SDL_video.h>

void loadIcon(SDL_Window *window);
int getStartupFontSize(SDL_Window *window);
void launchDebug(const char *file);
void browseFile(SDL_Window *window, const char *url);

#endif
