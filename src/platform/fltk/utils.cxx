//
// Copyright(C) 2013 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <fltk3/run.h>
#include <fltk3/Group.h>
#include "utils.h"

fltk3::Font get_font(const char *name, fltk3::Font defaultFont) {
  fltk3::Font result = defaultFont;
  int numfonts = fltk3::set_fonts("-*");
  int length = strlen(name);
  for (int i = 0; i < numfonts; i++) {
    const char *fontname = fltk3::get_font(i);
    if (!strncasecmp(name, fontname, length)) {
      result = (fltk3::Font)i;
      break;
    }
  }
  return result;
}

fltk3::Color parse_color(const char *name) {
  fltk3::Color result = NO_COLOR;
  unsigned int r = 0;
  unsigned int g = 0;
  unsigned int b = 0;
  if (sscanf(name, "#%2x%2x%2x", &r, &g, &b) == 3) {
    result = fltk3::rgb_color(r, g, b);
  }
  return result;
}

void getHomeDir(char *fileName, bool appendSlash) {
  const char *vars[] = {
    "APPDATA", "HOME", "TMP", "TEMP", "TMPDIR"
  };

  int vars_len = sizeof(vars) / sizeof(vars[0]);

  fileName[0] = 0;

  for (int i = 0; i < vars_len; i++) {
    const char *home = ::getenv(vars[i]);
    if (home && ::access(home, R_OK) == 0) {
      strcpy(fileName, home);
      if (i == 1) {
        // unix path
        strcat(fileName, "/.config");
        makedir(fileName);
      }
      strcat(fileName, "/SmallBASIC");
      if (appendSlash) {
        strcat(fileName, "/");
      }
      makedir(fileName);
      break;
    }
  }
}

//--Debug support---------------------------------------------------------------

void inspect(fltk3::Group *w, int level) {
  int n = w->children();
  char indent[20];
  memset(indent, ' ', sizeof(indent));
  indent[level] = '\0';
  trace("%sP: id:%d resz:%d child:%d level:%d %s", 
        indent, w, w->resizable(), n, level, w->label());

  for (int i = 0; i < n; i++) {
    fltk3::Widget *child = w->child(i);
    fltk3::Group *group = child->as_group();
    trace("%s-%c: %d %s", indent, (group!=NULL ? 'G': 'C'), child, child->label());
    if (group) {
      inspect(group, level+1);
    }
  }
}

#if defined(WIN32)
#include <windows.h>
#endif
// see http://download.sysinternals.com/Files/DebugView.zip
// for the free DebugView program
// an alternative debug method is to use insight.exe which
// is included with cygwin.

extern "C" void trace(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof buf - 1, format, args);
  va_end(args);

  while (p > buf && isspace(p[-1])) {
    *--p = '\0';
  }

  *p++ = '\r';
  *p++ = '\n';
  *p = '\0';
#if defined(WIN32)
  OutputDebugString(buf);
#else
  fprintf(stderr, buf, 0);
#endif
}

