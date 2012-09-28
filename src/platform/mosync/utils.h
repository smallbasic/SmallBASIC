// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <mavsprintf.h>

#ifndef max
#define max(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef min
#define min(a,b) ((a>b) ? (b) : (a))
#endif

#define OUTSIDE_RECT(px, py, x, y, w, h) \
  (px < (x) || py < (y) || px > ((x)+(w)) || py > ((y)+(h)))

#if defined(VARIANT_MOSYNC_EMULATOR) || defined(_DEBUG)
#define trace lprintfln
#define logEntered() trace("%s entered (%s %d)",               \
                           __FUNCTION__, __FILE__, __LINE__);
#define logLeaving() trace("%s leaving (%s %d)",               \
                           __FUNCTION__, __FILE__, __LINE__);
#else
#define trace(...)
#define logEntered()
#define logLeaving()
#endif

int get_text_width(char *s);

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }
