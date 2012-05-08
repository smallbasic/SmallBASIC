// This file is part of SmallBASIC
//
// User Interface Lib
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_sb_blib_ui_h)
#define _sb_blib_ui_h

#include "common/sys.h"

#if defined(__cplusplus)
extern "C" {
#endif

void ui_reset(void) SEC(BLIB);

void cmd_button(void) SEC(BLIB);
void cmd_text(void) SEC(BLIB);
void cmd_doform(void) SEC(BLIB);

#if defined(__cplusplus)
}
#endif

#endif
