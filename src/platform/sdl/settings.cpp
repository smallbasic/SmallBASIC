// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>

#include "settings.h"
#include "ui/utils.h"

static const char *ENV_VARS[] = {
  "APPDATA", "HOME", "TMP", "TEMP", "TMPDIR"
};

#define PATH_MAX 256
#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_SCALE 100

#if defined(__MINGW32__)
#define makedir(f) mkdir(f)
#else
#define makedir(f) mkdir(f, 0700)
#endif

FILE *openConfig(const char *configName, const char *flags) {
  FILE *result = NULL;
  char path[PATH_MAX];
  int vars_len = sizeof(ENV_VARS) / sizeof(ENV_VARS[0]);

  path[0] = 0;
  for (int i = 0; i < vars_len && result == NULL; i++) {
    const char *home = getenv(ENV_VARS[i]);
    if (home && access(home, R_OK) == 0) {
      strcpy(path, home);
      if (i == 1) {
        // unix path
        strcat(path, "/.config");
        makedir(path);
      }
      strcat(path, "/");
      strcat(path, configName);
      makedir(path);

      strcat(path, "/settings.txt");
      result = fopen(path, flags);
    }
  }
  return result;
}

//
// returns the next integer from the file
//
int nextInteger(FILE *fp, int def) {
  int result = 0;
  for (int c = fgetc(fp); c != EOF && c != ',' && c != '\n'; c = fgetc(fp)) {
    result = (result * 10) + (c - '0');
  }
  if (!result) {
    result = def;
  }
  return result;
}

//
// restore window position
//
void restoreSettings(const char *configName, SDL_Rect &rect, int &fontScale) {
  FILE *fp = openConfig(configName, "r");
  if (fp) {
    rect.x = nextInteger(fp, SDL_WINDOWPOS_UNDEFINED);
    rect.y = nextInteger(fp, SDL_WINDOWPOS_UNDEFINED);
    rect.w = nextInteger(fp, DEFAULT_WIDTH);
    rect.h = nextInteger(fp, DEFAULT_HEIGHT);
    fontScale = nextInteger(fp, DEFAULT_SCALE);
    fclose(fp);
  } else {
    rect.x = SDL_WINDOWPOS_UNDEFINED;
    rect.y = SDL_WINDOWPOS_UNDEFINED;
    rect.w = DEFAULT_WIDTH;
    rect.h = DEFAULT_HEIGHT;
    fontScale = DEFAULT_SCALE;
  }
}

//
// save the window position
//
void saveSettings(const char *configName, SDL_Window *window, int fontScale) {
  FILE *fp = openConfig(configName, "w");
  if (fp) {
    int x, y, w, h;
    SDL_GetWindowPosition(window, &x, &y);
    SDL_GetWindowSize(window, &w, &h);
    fprintf(fp, "%d,%d,%d,%d,%d\n", x, y, w, h, fontScale);
  }
}

