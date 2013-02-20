//
// Copyright(C) 2013 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef UTILS_H
#define UTILS_H

#define NO_COLOR 0
#define setcolor fltk3::color
#define getcolor fltk3::color
#define getascent fltk3::height
#define getdescent fltk3::descent
#define fillrect(r) fltk3::rectf(r.x(),r.y(),r.w(),r.h())
#define setfont fltk3::font
#define drawtext fltk3::draw
#define drawline fltk3::line
#define drawpoint fltk3::point
#define getwidth fltk3::width

#define DAMAGE_CONTENTS 0x40
#define DAMAGE_HIGHLIGHT DAMAGE_SCROLL
#define DAMAGE_PUSHED DAMAGE_EXPOSE

const fltk3::Color GRAY45 = 42;
const fltk3::Color GRAY80 = 50;
const fltk3::Color GRAY99 = 55;

fltk3::Font get_font(const char *name, fltk3::Font defaultFont);
fltk3::Color parse_color(const char *name);

#endif
