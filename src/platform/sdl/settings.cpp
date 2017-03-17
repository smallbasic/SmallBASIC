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

#include "ui/utils.h"
#include "ui/textedit.h"
#include "common/smbas.h"
#include "platform/sdl/settings.h"

using namespace strlib;

static const char *ENV_VARS[] = {
  "APPDATA", "HOME", "TMP", "TEMP", "TMPDIR"
};

#if !defined(FILENAME_MAX)
  #define FILENAME_MAX 256
#endif

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_SCALE 100
#define NUM_RECENT_ITEMS 9

#if defined(__MINGW32__)
#define makedir(f) mkdir(f)
#else
#define makedir(f) mkdir(f, 0700)
#endif

String recentPath[NUM_RECENT_ITEMS];
int recentPosition[NUM_RECENT_ITEMS];

FILE *openConfig(const char *flags, bool debug) {
  FILE *result = NULL;
  char path[FILENAME_MAX];
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
      strcat(path, "/SmallBASIC");
      makedir(path);
      if (debug) {
        strcat(path, "/settings_debug.txt");
      } else {
        strcat(path, "/settings.txt");
      }
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
    if (c != '\r') {
      result = (result * 10) + (c - '0');
    }
  }
  if (!result) {
    result = def;
  }
  return result;
}

//
// returns the next hex value from the file
//
int nextHex(FILE *fp, int def) {
  int result = 0;
  for (int c = fgetc(fp); c != EOF && c != ',' && c != '\n'; c = fgetc(fp)) {
    if (c != '\r') {
      int val = (c >= 'a') ? (10 + (c - 'a')) : (c - '0');
      result = (result * 16) + val;
    }
  }
  if (!result) {
    result = def;
  }
  return result;
}

//
// returns the length of the next string
//
int nextString(FILE *fp) {
  int pos = ftell(fp);
  int len = 0;
  for (int c = fgetc(fp); c != EOF; c = fgetc(fp)) {
    if (c == '\n' || c == '\r') {
      if (len > 0) {
        // string terminator
        break;
      } else {
        // skip previous terminator
        pos++;
        continue;
      }
    }
    len++;
  }
  fseek(fp, pos, SEEK_SET);
  return len;
}

//
// sets the next string in the file as the current working directory
//
void restorePath(FILE *fp, bool restoreDir) {
  int len = nextString(fp);
  if (len > 0) {
    String path;
    path.append(fp, len);
    if (restoreDir) {
      chdir(path.c_str());
    }
  }
}

//
// restore window position
//
void restoreSettings(SDL_Rect &rect, int &fontScale, bool debug, bool restoreDir) {
  FILE *fp = openConfig("rb", debug);
  if (fp) {
    rect.x = nextInteger(fp, SDL_WINDOWPOS_UNDEFINED);
    rect.y = nextInteger(fp, SDL_WINDOWPOS_UNDEFINED);
    rect.w = nextInteger(fp, DEFAULT_WIDTH);
    rect.h = nextInteger(fp, DEFAULT_HEIGHT);
    fontScale = nextInteger(fp, DEFAULT_SCALE);
    opt_mute_audio = nextInteger(fp, 0);
    opt_ide = nextInteger(fp, 0);
    g_themeId = nextInteger(fp, 0);
    for (int i = 0; i < THEME_COLOURS; i++) {
      g_user_theme[i] = nextHex(fp, g_user_theme[i]);
    }
    restorePath(fp, restoreDir);

    // restore recent paths
    for (int i = 0; i < NUM_RECENT_ITEMS; i++) {
      int len = nextString(fp);
      if (len > 1) {
        recentPath[i].clear();
        recentPath[i].append(fp, len);
      } else {
        break;
      }
    }
    fclose(fp);
  } else {
    rect.x = SDL_WINDOWPOS_UNDEFINED;
    rect.y = SDL_WINDOWPOS_UNDEFINED;
    rect.w = DEFAULT_WIDTH;
    rect.h = DEFAULT_HEIGHT;
    fontScale = DEFAULT_SCALE;
    opt_mute_audio = 0;
    opt_ide = IDE_INTERNAL;
    g_themeId = 0;
  }
}

//
// save the window position
//
void saveSettings(SDL_Window *window, int fontScale, bool debug) {
  FILE *fp = openConfig("wb", debug);
  if (fp) {
    int x, y, w, h;
    SDL_GetWindowPosition(window, &x, &y);
    SDL_GetWindowSize(window, &w, &h);
    fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d\n", x, y, w, h,
            fontScale, opt_mute_audio, opt_ide, g_themeId);
    // print user theme colours on the second line
    for (int i = 0; i < THEME_COLOURS; i++) {
      fprintf(fp, (i + 1 < THEME_COLOURS ? "%06x," : "%06x"), g_user_theme[i]);
    }
    fprintf(fp, "\n");

    // save the current working directory
    char path[FILENAME_MAX + 1];
    getcwd(path, FILENAME_MAX);
    if (path[1] == ':' && path[2] == '\\') {
      for (int i = 2; path[i] != '\0'; i++) {
        if (path[i] == '\\') {
          path[i] = '/';
        }
      }
    }
    fprintf(fp, "%s\n", path);

    // save the recent editor paths
    for (int i = 0; i < NUM_RECENT_ITEMS; i++) {
      if (!recentPath[i].empty()) {
        fprintf(fp, "%s\n", recentPath[i].c_str());
      }
    }

    fclose(fp);
  }
}

bool getRecentFile(String &path, unsigned position) {
  bool result = false;
  if (position < NUM_RECENT_ITEMS &&
      !recentPath[position].empty() &&
      !recentPath[position].equals(path)) {
    path.clear();
    path.append(recentPath[position]);
    result = true;
  }
  return result;
}

void getRecentFileList(String &fileList, String &current) {
  for (int i = 0; i < NUM_RECENT_ITEMS; i++) {
    if (!recentPath[i].empty()) {
      if (recentPath[i].equals(current)) {
        fileList.append(">> ");
      }
      fileList.append(i + 1).append(" ");
      fileList.append(recentPath[i]).append("\n\n");
    }
  }
}

void setRecentFile(const char *filename) {
  bool found = false;
  for (int i = 0; i < NUM_RECENT_ITEMS && !found; i++) {
    if (recentPath[i].equals(filename, false)) {
      found = true;
    }
  }
  if (!found) {
    // shift items downwards
    for (int i = NUM_RECENT_ITEMS - 1; i > 0; i--) {
      recentPath[i].clear();
      recentPath[i].append(recentPath[i - 1]);
    }

    // create new item in first position
    recentPath[0].clear();
    recentPath[0].append(filename);
  }
}
