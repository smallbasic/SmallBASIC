// This file is part of SmallBASIC
//
// high-level part: default declarations
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(SB_APP_H)
#define SB_APP_H

#include "common/sys.h"
#include "common/smbas.h"
#include "common/kw.h"
#include "common/pproc.h"
#include "common/var.h"
#include "common/extlib.h"
#include "common/units.h"

#if defined(__cplusplus)
extern "C" {
#endif

int sbasic_main(const char *file);

#if defined(__cplusplus)
}
#endif
#endif
