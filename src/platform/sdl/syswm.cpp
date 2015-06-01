// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "platform/sdl/syswm.h"

#define DEFAULT_FONT_SIZE 12
#define DEFAULT_FONT_SIZE_PTS 11

#if defined(_Win32)
#include <SDL_syswm.h>

void loadIcon(SDL_Window *window) {
  HINSTANCE handle = ::GetModuleHandle(NULL);
  HICON icon = ::LoadIcon(handle, MAKEINTRESOURCE(101));
  if (icon != NULL) {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (SDL_GetWindowWMInfo(window, &wminfo) == 1) {
      HWND hwnd = wminfo.info.win.window;
      ::SetClassLong(hwnd, GCL_HICON, reinterpret_cast<LONG>(icon));
    }
  }
}

int getStartupFontSize(SDL_Window *window) {
  int result = DEFAULT_FONT_SIZE;
  SDL_SysWMinfo wminfo;
  SDL_VERSION(&wminfo.version);
  if (SDL_GetWindowWMInfo(window, &wminfo) == 1) {
    HWND hwnd = wminfo.info.win.window;
    HDC hdc = GetDC(hwnd);
    result = MulDiv(DEFAULT_FONT_SIZE_PTS, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(hwnd, hdc);
  }
  return result;
}

#else

void loadIcon(SDL_Window *window) {
}

int getStartupFontSize(SDL_Window *window) {
  return DEFAULT_FONT_SIZE;
}

#endif
