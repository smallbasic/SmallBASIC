//
// Copyright(C) 2013 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef UTILS_H
#define UTILS_H

#define NO_COLOR 0
#define DAMAGE_CONTENTS 0x40
#define DAMAGE_HIGHLIGHT DAMAGE_SCROLL
#define DAMAGE_PUSHED DAMAGE_EXPOSE

#if defined(__MINGW32__)
#define makedir(f) mkdir(f)
#else
#define makedir(f) mkdir(f, 0700)
#endif

const fltk3::Color GRAY45 = 42;
const fltk3::Color GRAY80 = 50;
const fltk3::Color GRAY99 = 55;

fltk3::Font get_font(const char *name, fltk3::Font defaultFont);
fltk3::Color parse_color(const char *name);

void getHomeDir(char *filename, bool appendSlash = true);

// debugging
extern "C" void trace(const char *format, ...);
void inspect(fltk3::Group *w, int level=0);

#endif
