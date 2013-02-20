//
// Copyright(C) 2013 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <fltk3/run.h>
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

