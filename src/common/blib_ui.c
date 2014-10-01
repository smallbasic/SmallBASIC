// This file is part of SmallBASIC
//
// User Interface Lib
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/blib_ui.h"
#include "common/sberr.h"

// check if driver implements the UI api
#ifndef IMPL_UI

void ui_reset(void) {
}

void cmd_button(void) {
  err_unsup();
}

void cmd_text(void) {
  err_unsup();
}

void cmd_doform(void) {
  err_unsup();
}

#endif
