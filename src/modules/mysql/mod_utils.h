// This file is part of SmallBASIC
//
// Module support routines
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "var.h"
#include "module.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef char *mod_keyword_t;

int mod_parint(int n, slib_par_t *params, int param_count, int *val);
int mod_opt_parint(int n, slib_par_t *params, int param_count, int *val,
                   int def_val);
int mod_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr);
int mod_opt_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr,
                       const char *def_val);

#if defined(__cplusplus)
}
#endif
