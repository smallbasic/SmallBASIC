/**
 * SmallBASIC, high-level part: default declarations
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 *
 * Nicholas Christopoulos
 */

#if !defined(__sb_app_h)
#define __sb_app_h

#include "sys.h"
#include "smbas.h"
#include "kw.h"
#include "pproc.h"
#include "var.h"
#include "extlib.h"
#include "units.h"

#if defined(__cplusplus)
extern "C" {
#endif

int sbasic_main(const char *file) SEC(BEXEC);

#if defined(__cplusplus)
}
#endif
#endif
