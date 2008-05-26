// $Id$
// This file is part of SmallBASIC
//
// Module support routines
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "../extlib.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define tmp_alloc(s) malloc(s)
#define tmp_realloc(ptr, size) realloc(ptr, size)
#define tmp_free(p)  free(p)
#define mem_alloc(p) (mem_t)malloc(p)
#define mem_realloc(ptr, size) (mem_t)realloc((void*)ptr, size)
#define mem_free(h)  free((void*)h)
#define tmp_strdup(str) strdup(str)
#define mem_lock(h) (void*)(h)
#define mem_unlock(h)
#define mem_handle_size(p) malloc_usable_size((void*)p)
#define MemHandleSize(p) malloc_usable_size((void*)p)
#define MemPtrSize(p) malloc_usable_size(p)
#define MemHandleSizeX(p,f,l) malloc_usable_size(p)
#define memmgr_setabort(v) (v)
#define memmgr_getmaxalloc(v) (0)
#define MemPtrNew(x) malloc(x)
#define MemPtrFree(x) free(x)

typedef char* mod_keyword_t;

int mod_parint(int n, slib_par_t * params, int param_count, int *val);
int mod_opt_parint(int n, slib_par_t * params, int param_count, int *val,
                   int def_val);
int mod_parstr_ptr(int n, slib_par_t * params, int param_count, char **ptr);
int mod_opt_parstr_ptr(int n, slib_par_t * params, int param_count, char **ptr,
                       const char *def_val);

#if defined(__cplusplus)
}
#endif
