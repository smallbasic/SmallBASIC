// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

extern "C" void trace(const char *format, ...);

#define MAIN_BAS_RES "main.bas"

#if defined(_DEBUG)
 #define logEntered() trace("%s entered (%s %d)", \
                            __FUNCTION__, __FILE__, __LINE__);
 #define logLeaving() trace("%s leaving (%s %d)", \
                            __FUNCTION__, __FILE__, __LINE__);
#else
 #define logEntered()
 #define logLeaving()
#endif

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }
