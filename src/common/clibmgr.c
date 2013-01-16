// This file is part of SmallBASIC
//
// external C-Lib support
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"

/**
 *	@defgroup clib C-Libraries manager
 */

#if USE_CLIB
#define	MAX_CLIBS	256

typedef struct {
  void *handle;
} clib_cnode;

static clib_node table[MAX_CLIBS];  // table of library nodes, the first
// element allocated for system purposes
static int clib_count;// number of loaded libs

#endif

/**
 *	@ingroup clib
 *
 *	initializes the C-library manager
 *
 *	@return 0 for success; -1 for error (actually: 'not supported' error) 
 */
int clib_init() {
#if USE_CLIB
  clib_count = 1;               // handle 0 is reserved
  table[0].handle = NULL;
  return 0;
#else
  return -1;
#endif
}

/**
 *	@ingroup clib
 *
 *	de-initializes the C-library manager (closes all open handles)
 *
 *	@return 0 for success; -1 for error (actually: 'not supported' error) 
 */
int clib_restore() {
#if USE_CLIB
  int i;

  for (i = 1; i < clib_count; i++)
  clib_unloadlib(i);
  return 0;
#else
  return -1;
#endif
}

/**
 *	@ingroup clib
 *
 *	loads a C-library
 *
 *	@param libname the filename
 *	@return > 0 the library handle; -1 for error; 0 means static-linked (example: OS API) 
 */
int clib_loadlib(const char *libname) {
#if USE_CLIB
#if defined(_UnixOS)
  table[clib_count].handle = dlopen(libname, RTLD_LAZY);
  if (table[clib_count].handle == NULL) {
    rt_raise("CALL: CANNOT LOAD LIBRARY '%s'\nSystem reports: %s", libname,
        dlerror());
    return -1;
  }

  clib_count++;
  return clib_count - 1;

#elif defined(_Win32)
  table[clib_count].handle = (void *)LoadLibrary(libname);
  if (table[clib_count].handle == NULL) {
    rt_raise("CALL: CANNOT LOAD LIBRARY '%s'", libname);
    return -1;
  }

  clib_count++;
  return clib_count - 1;

#endif

#else
  rt_raise("SUBSYSTEM NOT SUPPORTED");
  return -1;                    // not supported
#endif
}

/**
 *	@ingroup clib
 *
 *	unloads a C-library
 *
 *	@param handle the lib-handle
 */
void clib_unloadlib(int handle) {
#if USE_CLIB
  if (handle > 0 && handle < clib_count) {
    if (table[handle].handle) {
#if defined(_UnixOS)
      dlclose(table[handle].handle);
#elif defined(_Win32)
      FreeLibrary((HMODULE) table[handle].handle);
#endif
      table[handle].handle = NULL;
    }
  }

#else
  rt_raise("SUBSYSTEM NOT SUPPORTED");
#endif
}

/**
 *	@ingroup clib
 *
 *	call a C-function
 *
 *	@param handle the lib-handle
 *	@param name the function name
 *	@return depented on function
 */
void *clib_call_func(int handle, const char *name, ...) {
  // NDC information
  // 
  // It is valid for C to call a function with parameters
  // even if it is declared without parameters.
  // 
  // the ... operator works like all others, but there is
  // a problem because compiler requires one parameters to be
  // declared
  // 
  // The compiler pushes the arguments on the stack with reverse order
  // and after the call it is removes the allocated size from the stack
  // 
  // Example:
  // 
  // f(a,b,c) produces
  // 
  // push c
  // push b
  // push a
  // call f
  // add sp, size(a)+size(b)+size(c)
  // ; that means substract from stack pointer the correct size
  // 
  // now, the push statement (at least on compilers output that I have
  // study) uses default CPU's words (1 CPU word = 1 int). 
  // Real numbers or any bigest value uses 2 or more words. Smallest
  // values (like 1 byte) are also uses 1 word. That is depented on
  // CPU architecture, and it is correct. Unfortunately I don't know
  // if there is a different way.
  // 
  // The problem is the returned value. The default is to returned on
  // accumulator, but that is not the rule (in any case there are no
  // rules). For example the linux's gcc returns a 'int' the value on 
  // the stack. Turbo C uses AX (DX:AX for long ints).
  // 
  // I had to found a cross-compiled code to call a C function, so,
  // that is the idea.
  // 
  // a) we will convert the parameters to CPU words (ptable, pcount)
  // b) we will use the C function-pointer (f) without declaring the parameters
  // c) we will call that function based on number of the parameters that we
  // have
  // d) we got the return value as int.
  // 
  // Problems:
  // We can't get a 'double' return value

#if USE_CLIB
    void *(f) ();
    void *proc_ptr;

    int ptable[256];
    int pcount;

    if (handle > 0 && handle < clib_count) {
      if (table[handle].handle) {
#if defined(_UnixOS)
    proc_ptr = dlsym(table[handle].handle, name);
#elif defined(_Win32)
    proc_ptr = GetProcAddress((HMODULE) table[handle].handle, name);
#endif
    if (proc_ptr == NULL)
    rt_raise("CALL: FUNCTION '%s' DOES NOT EXISTS", name);
    else {
      f = proc_ptr;

    }
  }
}

#else
    rt_raise("SUBSYSTEM NOT SUPPORTED");
    return NULL;
#endif
  }

void clib_call_proc(int handle, const char *name, ...) {
}

/* --- Glue with SB ------------------------------------------------------------------------------------------------------- */

/**
 *	@ingroup clib
 *
 *	loads a C-library (SB-RTL glue)
 *
 *	SB Syntax: handle = LOADLIB(filename)
 */
void blib_loadlib() {
}

/**
 *	@ingroup clib
 *
 *	unloads a C-library (SB-RTL glue)
 *
 *	SB Syntax: UNLOADLIB handle
 */
void blib_unloadlib() {
}

/**
 *	@ingroup clib
 *
 *	Calls a C function (SB-RTL glue)
 *
 *	SB Syntax: r = CALL(handle, func_name, ...)
 */
void blib_call_cfunc() {
}

/**
 *	@ingroup clib
 *
 *	Calls a C procedure (SB-RTL glue)
 *
 *	SB Syntax: CALL handle, proc_name, ...
 */
void blib_call_cproc() {
}
