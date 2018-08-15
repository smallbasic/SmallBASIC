// This file is part of SmallBASIC
//
// Copyright(C) 2001-2017 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>

#include "lib/str.h"
#include "ui/utils.h"
#include "ui/textedit.h"
#include "common/smbas.h"
#include "platform/sdl/settings.h"

using namespace strlib;

static const char *ENV_VARS[] = {
  "APPDATA", "HOME", "TMP", "TEMP", "TMPDIR", ""
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
extern String g_exportAddr;
extern String g_exportToken;
enum Settings {k_window, k_debug, k_export};

void createConfigPath(const char *var, const char *home, char *path, size_t size) {
  strlcpy(path, home, size);
  if (strcmp(var, "HOME") == 0) {
    // unix path
    strlcat(path, "/.config", size);
    makedir(path);
  }
  strlcat(path, "/SmallBASIC", size);
  makedir(path);
}

FILE *openConfig(bool readMode, Settings settings) {
  FILE *result = NULL;
  char path[FILENAME_MAX];
  const char *flags = readMode ? "rb" : "wb";

  path[0] = 0;
  for (int i = 0; ENV_VARS[i][0] != '\0' && result == NULL; i++) {
    const char *home = getenv(ENV_VARS[i]);
    if (home && access(home, R_OK) == 0) {
      createConfigPath(ENV_VARS[i], home, path, sizeof(path));
      switch (settings) {
      case k_debug:
        strcat(path, "/debug.txt");
        break;
      case k_window:
        strcat(path, "/settings.txt");
        break;
      case k_export:
        strcat(path, "/export.txt");
        break;
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
  FILE *fp = openConfig(true, debug ? k_debug : k_window);
  if (fp) {
    rect.x = nextInteger(fp, SDL_WINDOWPOS_UNDEFINED);
    rect.y = nextInteger(fp, SDL_WINDOWPOS_UNDEFINED);
    rect.w = nextInteger(fp, DEFAULT_WIDTH);
    rect.h = nextInteger(fp, DEFAULT_HEIGHT);
    fontScale = nextInteger(fp, DEFAULT_SCALE);
    opt_mute_audio = nextInteger(fp, 0);
    opt_ide = nextInteger(fp, 0) ? IDE_INTERNAL : IDE_NONE;
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

  // restore export settings
  if (!debug) {
    fp = openConfig(true, k_export);
    if (fp) {
      int len = nextString(fp);
      if (len > 1) {
        g_exportAddr.clear();
        g_exportAddr.append(fp, len);
      }
      len = nextString(fp);
      if (len > 1) {
        g_exportToken.clear();
        g_exportToken.append(fp, len);
      }
      fclose(fp);
    }
  }
}

//
// save the window position
//
void saveSettings(SDL_Window *window, int fontScale, bool debug) {
  FILE *fp = openConfig(false, debug ? k_debug : k_window);
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

  // save export settings
  if (!debug) {
    fp = openConfig(false, k_export);
    if (fp) {
      if (!g_exportAddr.empty()) {
        fprintf(fp, "%s\n", g_exportAddr.c_str());
      }
      if (!g_exportToken.empty()) {
        fprintf(fp, "%s\n", g_exportToken.c_str());
      }
      fclose(fp);
    }
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

String saveGist(const char *buffer, const char *fileName, const char *description) {
  String result;
  FILE *fp = NULL;
  char path[FILENAME_MAX];

  path[0] = 0;
  for (int i = 0; ENV_VARS[i][0] != '\0' && fp == NULL; i++) {
    const char *home = getenv(ENV_VARS[i]);
    if (home && access(home, R_OK) == 0) {
      createConfigPath(ENV_VARS[i], home, path, sizeof(path));
      strcat(path, "/gist.txt");
      fp = fopen(path, "wb");
    }
  }

  if (fp != NULL) {
    result.append(path);
    const char *format =
      "{\"description\":\"SmallBASIC: %s\",\"public\":true,"
      "\"files\":{\"%s\":{\"content\":\"";
    fprintf(fp, format, description, fileName);
    for (const char *p = buffer; *p != '\0'; p++) {
      if (*p == '\n') {
        fputs("\\n", fp);
      } else if (*p == '\"') {
        fputs("\\\"", fp);
      } else if (*p == '\\') {
        fputs("\\\\", fp);
      } else if (*p == '\t') {
        fputs("  ", fp);
      } else if (*p == '\r') {
        if (*(p+1) != '\n') {
          fputs("\\n", fp);
        }
      } else {
        fputc(*p, fp);
      }
    }
    fputs("\"}}}", fp);
    fclose(fp);
  }

  return result;
}

