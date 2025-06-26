// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SB_SYSWM_H
#define SB_SYSWM_H

#include <SDL3/SDL_video.h>

void browseFile(SDL_Window *window, const char *url);
int  getStartupFontSize(SDL_Window *window);
void launch(const char *command, const char *file);
void launchDebug(const char *file);
void launchExec(const char *file);
void loadIcon(SDL_Window *window);
void setupAppPath(const char *path);

#endif
