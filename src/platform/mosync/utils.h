// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#define MAIN_BAS_RES "main.bas"
#define ERROR_BAS "? \"Failed to open program file\":pause"

#if defined(_DEBUG)
#define logEntered() printf("%s entered (%s %d)",               \
                            __FUNCTION__, __FILE__, __LINE__);
#define logLeaving() printf("%s leaving (%s %d)",               \
                            __FUNCTION__, __FILE__, __LINE__);
#else
#define logEntered()
#define logLeaving()
#endif

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }
