// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include "common/var.h"

int cmd_opendigitalinput(int argc, slib_par_t *args, var_t *retval);
int cmd_opendigitaloutput(int argc, slib_par_t *args, var_t *retval);
int cmd_openanaloginput(int argc, slib_par_t *args, var_t *retval);
int cmd_openanalogoutput(int argc, slib_par_t *args, var_t *retval);
