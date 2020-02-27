// This file is part of SmallBASIC
//
// Copyright(C) 2001-2020 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#if defined(_Win32)
#include <windows.h>
#include <shellapi.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#endif
#include <stdint.h>
#include "lib/str.h"
#include "platform/fltk/utils.h"
#include "ui/utils.h"

#define RX_BUFFER_SIZE 1024
#if defined(_Win32)
static char appName[OS_PATHNAME_SIZE + 1];
#endif

Fl_Color get_color(int argb) {
  // Fl_Color => 0xrrggbbii
  return (argb << 8) & 0xffffff00;
}

Fl_Color get_color(const char *name, Fl_Color def) {
  Fl_Color result = def;
  if (!name || name[0] == '\0') {
    result = def;
  } else if (name[0] == '#') {
    // do hex color lookup
    int rgb = strtol(name + 1, NULL, 16);
    if (!rgb) {
      result = FL_BLACK;
    } else {
      uchar r = rgb >> 16;
      uchar g = (rgb >> 8) & 255;
      uchar b = rgb & 255;
      result = fl_rgb_color(r, g, b);
    }
  } else if (strcasecmp(name, "black") == 0) {
    result = FL_BLACK;
  } else if (strcasecmp(name, "red") == 0) {
    result = FL_RED;
  } else if (strcasecmp(name, "green") == 0) {
    result = fl_rgb_color(0, 0x80, 0);
  } else if (strcasecmp(name, "yellow") == 0) {
    result = FL_YELLOW;
  } else if (strcasecmp(name, "blue") == 0) {
    result = FL_BLUE;
  } else if (strcasecmp(name, "magenta") == 0 ||
             strcasecmp(name, "fuchsia") == 0) {
    result = FL_MAGENTA;
  } else if (strcasecmp(name, "cyan") == 0 ||
             strcasecmp(name, "aqua") == 0) {
    result = FL_CYAN;
  } else if (strcasecmp(name, "white") == 0) {
    result = FL_WHITE;
  } else if (strcasecmp(name, "gray") == 0 ||
             strcasecmp(name, "grey") == 0) {
    result = fl_rgb_color(0x80, 0x80, 0x80);
  } else if (strcasecmp(name, "lime") == 0) {
    result = FL_GREEN;
  } else if (strcasecmp(name, "maroon") == 0) {
    result = fl_rgb_color(0x80, 0, 0);
  } else if (strcasecmp(name, "navy") == 0) {
    result = fl_rgb_color(0, 0, 0x80);
  } else if (strcasecmp(name, "olive") == 0) {
    result = fl_rgb_color(0x80, 0x80, 0);
  } else if (strcasecmp(name, "purple") == 0) {
    result = fl_rgb_color(0x80, 0, 0x80);
  } else if (strcasecmp(name, "silver") == 0) {
    result = fl_rgb_color(0xc0, 0xc0, 0xc0);
  } else if (strcasecmp(name, "teal") == 0) {
    result = fl_rgb_color(0, 0x80, 0x80);
  }
  return result;
}

Fl_Font get_font(const char *name) {
  Fl_Font result = FL_COURIER;
  if (strcasecmp(name, "helvetica") == 0) {
    result = FL_HELVETICA;
  } else if (strcasecmp(name, "times") == 0) {
    result = FL_TIMES;
  }
  return result;
}

void getHomeDir(char *fileName, size_t size, bool appendSlash) {
  const int homeIndex = 1;
  static const char *envVars[] = {
    "APPDATA", "HOME", "TMP", "TEMP", "TMPDIR", ""
  };

  fileName[0] = '\0';

  for (int i = 0; envVars[i][0] != '\0' && fileName[0] == '\0'; i++) {
    const char *home = getenv(envVars[i]);
    if (home && access(home, R_OK) == 0) {
      strlcpy(fileName, home, size);
      if (i == homeIndex) {
        strlcat(fileName, "/.config", size);
        makedir(fileName);
      }
      strlcat(fileName, "/SmallBASIC", size);
      if (appendSlash) {
        strlcat(fileName, "/", size);
      }
      makedir(fileName);
      break;
    }
  }
}

// copy the url into the local cache
bool cacheLink(dev_file_t *df, char *localFile, size_t size) {
  char rxbuff[RX_BUFFER_SIZE];
  FILE *fp;
  const char *url = df->name;
  const char *pathBegin = strchr(url + 7, '/');
  const char *pathEnd = strrchr(url + 7, '/');
  const char *pathNext;
  bool inHeader = true;
  bool httpOK = false;

  getHomeDir(localFile, size, true);
  strlcat(localFile, "cache/", size);
  makedir(localFile);

  // create host name component
  strncat(localFile, url + 7, pathBegin - url - 7);
  strlcat(localFile, "/", size);
  makedir(localFile);

  if (pathBegin != 0 && pathBegin < pathEnd) {
    // re-create the server path in cache
    int level = 0;
    pathBegin++;
    do {
      pathNext = strchr(pathBegin, '/');
      strncat(localFile, pathBegin, pathNext - pathBegin + 1);
      makedir(localFile);
      pathBegin = pathNext + 1;
    }
    while (pathBegin < pathEnd && ++level < 20);
  }
  if (pathEnd == 0 || pathEnd[1] == 0 || pathEnd[1] == '?') {
    strlcat(localFile, "index.html", size);
  } else {
    strlcat(localFile, pathEnd + 1, size);
  }

  fp = fopen(localFile, "wb");
  if (fp == 0) {
    if (df->handle != -1) {
      shutdown(df->handle, df->handle);
    }
    return false;
  }

  if (df->handle == -1) {
    // pass the cache file modified time to the HTTP server
    struct stat st;
    if (stat(localFile, &st) == 0) {
      df->drv_dw[2] = st.st_mtime;
    }
    if (http_open(df) == 0) {
      fclose(fp);
      return false;
    }
  }

  while (true) {
    int bytes = recv(df->handle, (char *)rxbuff, sizeof(rxbuff), 0);
    if (bytes == 0) {
      break;                    // no more data
    }
    // assumes http header < 1024 bytes
    if (inHeader) {
      int i = 0;
      while (true) {
        int iattr = i;
        while (rxbuff[i] != 0 && rxbuff[i] != '\n') {
          i++;
        }
        if (rxbuff[i] == 0) {
          inHeader = false;
          break;                // no end delimiter
        }
        if (rxbuff[i + 2] == '\n') {
          if (!fwrite(rxbuff + i + 3, bytes - i - 3, 1, fp)) {
            break;
          }
          inHeader = false;
          break;                // found start of content
        }
        // null terminate attribute (in \r)
        rxbuff[i - 1] = 0;
        i++;
        if (strstr(rxbuff + iattr, "200 OK") != 0) {
          httpOK = true;
        }
        if (strncmp(rxbuff + iattr, "Location: ", 10) == 0) {
          // handle redirection
          shutdown(df->handle, df->handle);
          strcpy(df->name, rxbuff + iattr + 10);
          if (http_open(df) == 0) {
            fclose(fp);
            return false;
          }
          break;                // scan next header
        }
      }
    } else if (!fwrite(rxbuff, bytes, 1, fp)) {
      break;
    }
  }

  // cleanup
  fclose(fp);
  shutdown(df->handle, df->handle);
  return httpOK;
}

void vsncat(char *buffer, size_t size, ...) {
  va_list args;
  va_start(args, size);
  strlcpy(buffer, va_arg(args, char *), size);
  for (char *next = va_arg(args, char *);
       next != NULL;
       next = va_arg(args, char *)) {
    strlcat(buffer, next, size);
  }
  va_end(args);
}

void setAppName(const char *path) {
#if defined(_Win32)
  appName[0] = '\0';
  if (path[0] == '/' ||
      (path[1] == ':' && ((path[2] == '\\') || path[2] == '/'))) {
    // full path or C:/
    strlcpy(appName, path, sizeof(appName));
  } else {
    // relative path
    char cwd[OS_PATHNAME_SIZE + 1];
    cwd[0] = '\0';
    getcwd(cwd, sizeof(cwd) - 1);
    strlcpy(appName, cwd, sizeof(appName));
    strlcat(appName, "/", sizeof(appName));
    strlcat(appName, path, sizeof(appName));
  }
  const auto file = "sbasici.exe";
  char *exe = strstr(appName, file);
  if (exe) {
    strcpy(exe, "sbasicg.exe");
  }
#endif
}

#if defined(_Win32)
void launchExec(const char *file) {
  STARTUPINFO info = {sizeof(info)};
  PROCESS_INFORMATION processInfo;
  char cmd[1024];
  sprintf(cmd, " -x %s", file);
  if (!CreateProcess(appName, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo)) {
    appLog("failed to start %d %s %s\n", GetLastError(), appName, cmd);
  }
}
#else
void launchExec(const char *file) {
  pid_t pid = fork();
  auto app = "/usr/bin/sbasicg";

  switch (pid) {
  case -1:
    // failed
    break;
  case 0:
    // child process
    if (execl(app, app, "-x", file, (char *)0) == -1) {
      fprintf(stderr, "exec failed [%s] %s\n", strerror(errno), app);
      exit(1);
    }
    break;
  default:
    // parent process - continue
    break;
  }
}
#endif
