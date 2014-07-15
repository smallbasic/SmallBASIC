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

void ui_reset(void);
void cmd_button(void);
void cmd_text(void);
void cmd_doform(void);

#if defined(__cplusplus)
}
#endif

#endif
