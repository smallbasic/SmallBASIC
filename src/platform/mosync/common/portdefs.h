// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#if defined(__cplusplus)
extern "C" {
#endif

int dev_clock();
 
#undef clock
#define clock dev_clock

#if defined(__cplusplus)
}
#endif
