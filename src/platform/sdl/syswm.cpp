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

extern char g_appPath[];
extern int g_debugPort;

void appLog(const char *format, ...);

#if defined(_Win32)
#include <SDL_syswm.h>
#include <shellapi.h>

void loadIcon(SDL_Window *window) {
  HINSTANCE handle = ::GetModuleHandle(NULL);
  HICON icon = ::LoadIcon(handle, MAKEINTRESOURCE(101));
  if (icon != NULL) {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (SDL_GetWindowWMInfo(window, &wminfo) == 1) {
      HWND hwnd = wminfo.info.win.window;
      ::SendMessage(hwnd, WM_SETICON, 0, (LPARAM)icon);
      ::SendMessage(hwnd, WM_SETICON, 1, (LPARAM)icon);
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

void launchDebug(const char *file) {
  STARTUPINFO info = {sizeof(info)};
  PROCESS_INFORMATION processInfo;
  char cmd[1024];
  sprintf(cmd, "-p %d -d %s", g_debugPort, file);
  if (!CreateProcess(g_appPath, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo)) {
    appLog("failed to start %d %s %s\n", GetLastError(), g_appPath, cmd);
  }
}

void launch(const char *command, const char *file) {
  STARTUPINFO info = {sizeof(info)};
  PROCESS_INFORMATION processInfo;
  char cmd[1024];
  sprintf(cmd, "%s -x %s", command, file);
  if (!CreateProcess(command, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo)) {
    appLog("failed to start %d %s %s\n", GetLastError(), command, cmd);
  }
}

void browseFile(SDL_Window *window, const char *url) {
  SDL_SysWMinfo wminfo;
  SDL_VERSION(&wminfo.version);
  if (SDL_GetWindowWMInfo(window, &wminfo) == 1) {
    HWND hwnd = wminfo.info.win.window;
    ::ShellExecute(hwnd, "open", url, 0, 0, SW_SHOWNORMAL);
  }
}

#else
#include <unistd.h>
#include <errno.h>
#include "icon.h"
#include "lib/lodepng/lodepng.h"

void loadIcon(SDL_Window *window) {
  unsigned w, h;
  unsigned char *image;
  if (!lodepng_decode32(&image, &w, &h, sb_desktop_128x128_png, sb_desktop_128x128_png_len)) {
    SDL_Surface *surf =
      SDL_CreateRGBSurfaceFrom(image, w, h,
                               32, w * 4,
                               0x000000ff,
                               0x0000ff00,
                               0x00ff0000,
                               0xff000000);
    SDL_SetWindowIcon(window, surf);
    SDL_FreeSurface(surf);
    free(image);
  }
}

int getStartupFontSize(SDL_Window *window) {
  return DEFAULT_FONT_SIZE;
}

void launchDebug(const char *file) {
  pid_t pid = fork();
  char port[20];

  switch (pid) {
  case -1:
    // failed
    break;
  case 0:
    // child process
    sprintf(port, "-p %d", g_debugPort);
    if (execl(g_appPath, g_appPath, port, "-d", file, (char *)0) == -1) {
      fprintf(stderr, "exec failed [%s] %s\n", strerror(errno), g_appPath);
      exit(1);
    }
    break;
  default:
    // parent process - continue
    break;
  }
}

void launch(const char *command, const char *file) {
  pid_t pid = fork();

  switch (pid) {
  case -1:
    // failed
    break;
  case 0:
    // child process
    if (execl(command, command, "-x", file, (char *)0) == -1) {
      fprintf(stderr, "exec failed [%s] %s\n", strerror(errno), command);
      exit(1);
    }
    break;
  default:
    // parent process - continue
    break;
  }
}

void browseFile(SDL_Window *window, const char *url) {
  if (fork() == 0) {
    const char *browser[] = {
      "sensible-browser",
      "xdg-open",
      "gnome-open",
      "htmlview",
      "firefox",
      "google-chrome",
      NULL
    };
    for (int i = 0; browser[i] != NULL; i++) {
      execlp(browser[i], browser[i], url, NULL);
    }
    fprintf(stderr, "exec browser failed for %s\n", url);
    ::exit(1);
  }
}

#endif

void launchExec(const char *file) {
  launch(g_appPath, file);
}
