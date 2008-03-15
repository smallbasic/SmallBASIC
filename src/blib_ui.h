/*
 * SmallBASIC Run-Time Library - User interface engine
 *
 * Nicholas Christopoulos, 22/02/2002
 */

#if !defined(_sb_blib_ui_h)
#define _sb_blib_ui_h

#include "sys.h"

void ui_reset(void) SEC(BLIB);

void cmd_button(void) SEC(BLIB);
void cmd_text(void) SEC(BLIB);
void cmd_doform(void) SEC(BLIB);

#endif
