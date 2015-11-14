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

void loadIcon(SDL_Window *window) {
  HINSTANCE handle = ::GetModuleHandle(NULL);
  HICON icon = ::LoadIcon(handle, MAKEINTRESOURCE(101));
  if (icon != NULL) {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (SDL_GetWindowWMInfo(window, &wminfo) == 1) {
      HWND hwnd = wminfo.info.win.window;
      ::SendMessage(hwnd, WM_SETICON, 0, reinterpret_cast<LONG>(icon));
      ::SendMessage(hwnd, WM_SETICON, 1, reinterpret_cast<LONG>(icon));
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
  STARTUPINFO info={sizeof(info)};
  PROCESS_INFORMATION processInfo;
  char cmd[1024];
  sprintf(cmd, "-p %d -d %s", g_debugPort, file);
  if (!CreateProcess(g_appPath, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo)) {
    appLog("failed to start %d %s %s\n", GetLastError(), g_appPath, cmd);
  }
}

#if defined(WIN32)
#include <windows.h>
#include <fltk/Window.h>
#include <fltk/win32.h>
#endif

void browseFile(const char *url) {
  SDL_SysWMinfo wminfo;
  SDL_VERSION(&wminfo.version);
  if (SDL_GetWindowWMInfo(window, &wminfo) == 1) {
    HWND hwnd = wminfo.info.win.window;
    ShellExecute(hwnd, "open", url, 0, 0, SW_SHOWNORMAL);
  }
}

#else
#include <unistd.h>
#include <errno.h>

void loadIcon(SDL_Window *window) {
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

void browseFile(const char *url) {
  if (fork() == 0) {
    fclose(stderr);
    fclose(stdin);
    fclose(stdout);
    execlp("htmlview", "htmlview", url, NULL);
    execlp("google-chrome", "google-chrome", url, NULL);
    execlp("chromium-browser", "chromium-browser", url, NULL);
    execlp("firefox", "firefox", url, NULL);
    execlp("mozilla", "mozilla", url, NULL);
    // exit in case exec failed
    ::exit(0);
  }
}

#endif
