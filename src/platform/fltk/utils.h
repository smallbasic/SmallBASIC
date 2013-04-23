// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef UTILS_H
#define UTILS_H

#if defined(__MINGW32__)
#define makedir(f) mkdir(f)
#else
#define makedir(f) mkdir(f, 0700)
#endif

#endif
