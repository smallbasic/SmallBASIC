// This file is part of SmallBASIC
//
// SmallBASIC plugin manager
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @ingroup mod
 * @page moddoc Modules
 *
 * The modules are common shared-libraries which can call back the SB's code.
 *
 * The module-manager loads the modules at the startup and unloads them
 * before SB's exit. Which module will be loaded is specified by the
 * user in SB's command-line parameters (option -m). The path of the modules
 * is predefined to /usr/lib/sbasic/modules and/or /usr/local/lib/sbasic/modules
 *
 * <b>Standard interface</b>
 *
 * All modules must provides at least the three following functions:
 * sblib_init(), sblib_close, sblib_type().
 *
 * the sblib_init() called on startup, the sblib_close() called on SB's exit,
 * and the sblib_type() called after sblib_init() to inform the module-manager
 * about the type of the module (slib_tp).
 *
 * <b>Library-modules</b>
 *
 * The SB before compiles the .bas program, the module-manager
 * asks every module about the number of the functions and procedures
 * which are supported. (sblib_proc_count, sblib_func_count)
 *
 * It continues by asking the name of each function and procedure and
 * updates the compiler. (sblib_proc_getname, sblib_func_getname)
 *
 * On the execution time, if the program wants to execute a library's
 * procedure or function, the module-manager builds the parameter table
 * and calls the sblib_proc_exec or the sblib_func_exec.
 *
 * See modules/example1.c
 * <b>Notes:</b>
 *
 * Procedure & functions names are limited to 32 characters (33 with \0)
 */

/**
 * @defgroup mod Module Manager
 */
/**
 * @defgroup modstd Module interface - Standard
 */
/**
 * @defgroup modlib Module interface - Library
 */

#if !defined(_sb_extlib_h)
#define _sb_extlib_h

#include "common/sys.h"
#include "common/var.h"
#include "common/device.h"
#include "include/module.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup mod
 *
 * Initialize module-manager
 *
 * default path /usr/lib/sbasic/modules/:/usr/local/lib/sbasic/modules/
 */
void slib_init();

/**
 * @ingroup mod
 *
 * close module manager
 */
void slib_close(void);

/**
 * @ingroup mod
 *
 * set the alias and returns a library's ID
 *
 * @param name is the name of the library (without the file-extention)
 * @param alias updates the internal name with the given alias
 * @return the id or -1 for error
 */
int slib_get_module_id(const char *name, const char *alias);

/**
 * @ingroup mod
 *
 * imports the modules routine and optionally updates the compiler
 * with the module (mid) keywords.
 */
void slib_import(int lib_id, int comp);

/**
 * @ingroup mod
 *
 * returns the ID of the keyword. used at run-time to assign BCs ID with slib_mgr's one
 */
int slib_get_kid(int lib_id, const char *name);

/**
 * @ingroup mod
 *
 * execute a library's procedure
 *
 * @param lib is the lib-id
 * @param index is the index of the procedure
 * @return non-zero on success
 */
int slib_procexec(int lib, int index);

/**
 * @ingroup mod
 *
 * execute a library's function
 *
 * @param lib is the lib-id
 * @param index is the index of the function
 * @param ret is the variable to store the result
 * @return non-zero on success
 */
int slib_funcexec(int lib, int index, var_t *ret);

/**
 * @ingroup mod
 *
 * returns the function from the first available module
 *
 * @param name the function name
 * @return non-zero on success
 */
void *slib_get_func(const char *name);

#if defined(__cplusplus)
}
#endif
#endif
