// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FLTK_UTILS_H
#define FLTK_UTILS_H

#include <stdint.h>
#include "common/pproc.h"
#include "common/fs_socket_client.h"
#include <FL/Fl.H>

#define DAMAGE_HIGHLIGHT FL_DAMAGE_USER1
#define DAMAGE_PUSHED    FL_DAMAGE_USER2
#define SCROLL_W 15
#define SCROLL_H 15
#define SCROLL_X SCROLL_W - 2
#define HSCROLL_W 80
#define DEF_FONT_SIZE 12
#define TAB_BORDER 4
#define TTY_ROWS 1000
#define MENU_HEIGHT 24
#define NUM_RECENT_ITEMS 9
#define STATUS_HEIGHT (MENU_HEIGHT + 2)
#define LINE_NUMBER_WIDTH 40

// currently missing from Enumerations.H
#define FL_Multiply    0xffaa
#define FL_AddKey      0xffab
#define FL_SubtractKey 0xffad
#define FL_DivideKey   0xffaf

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }

#ifndef MAX
#define MAX(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef MIN
#define MIN(a,b) ((a>b) ? (b) : (a))
#endif

#if defined(__MINGW32__)
#define makedir(f) mkdir(f)
#else
#define makedir(f) mkdir(f, 0700)
#endif

Fl_Color get_color(const char *name, Fl_Color def);
Fl_Color get_color(int argb);
Fl_Font get_font(const char *name);
void getHomeDir(char *fileName, size_t size, bool appendSlash = true);
bool cacheLink(dev_file_t *df, char *localFile, size_t size);
void vsncat(char *buffer, size_t size, ...);
void launchExec(const char *file);

#endif
