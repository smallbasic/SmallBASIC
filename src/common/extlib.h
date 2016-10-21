// This file is part of SmallBASIC
//
// SmallBASIC module (shared-lib) manager
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

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup mod
 * @typedef slib_t
 * shared-lib information structure
 */
typedef struct {
  int id; /**< library ID */
  char name[256]; /**< name of library (basename) */
  char fullname[1024]; /**< full pathname */
  void *handle; /**< handle to the lib */
  dword flags; /**< flags */

  /*
   * ------------------ Language ------------------
   */
  int proc_count; /**< if lang.ext.; number of procedures */
  int func_count; /**< if lang.ext.; number of functions */
  int first_proc;
  int first_func;
} slib_t;

/*
 *   slib_t flags
 */
/**
 * @ingroup mod
 *
 * Initialize module-manager
 *
 * default path /usr/lib/sbasic/modules/:/usr/local/lib/sbasic/modules/
 *
 * @param mcount non-zero for check for modules
 * @param list the list of the modules or "" for auto-search/load-all
 */
void sblmgr_init(int mcount, const char *list);

/**
 * @ingroup mod
 *
 * close module manager
 */
void sblmgr_close(void);

/**
 * @ingroup mod
 *
 * returns a library's ID
 *
 * @param name is the name of the library (without the file-extention)
 * @return the id or -1 for error
 */
int slib_get_module_id(const char *name);

/**
 * @ingroup mod
 *
 * updates the compiler with the module (mid) keywords
 */
void slib_setup_comp(int mid);

/**
 * @ingroup mod
 *
 * returns the ID of the keyword. used at run-time to assign BCs ID with slib_mgr's one
 */
int slib_get_kid(const char *name);

/**
 * @ingroup mod
 *
 * returns a library's function name
 *
 * @param lib is the lib-id
 * @param index is the index of the function
 * @param buf is the buffer to store the name
 * @return non-zero on success
 */
int sblmgr_getfuncname(int lib, int index, char *buf);

/**
 * @ingroup mod
 *
 * returns a library's procedure name
 *
 * @param lib is the lib-id
 * @param index is the index of the procedure
 * @param buf is the buffer to store the name
 * @return non-zero on success
 */
int sblmgr_getprocname(int lib, int index, char *buf);

/**
 * @ingroup mod
 *
 * execute a library's procedure
 *
 * @param lib is the lib-id
 * @param index is the index of the procedure
 * @return non-zero on success
 */
int sblmgr_procexec(int lib, int index);

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
int sblmgr_funcexec(int lib, int index, var_t * ret);

/* ---------- Common interface ---------- */

/**
 * @ingroup modstd
 * @typedef slib_par_t
 *
 * Parameter structure
 */
typedef struct {
  var_t *var_p; /**< the parameter itself */
  byte byref; /**< parameter can be used as byref */
} slib_par_t;

/**
 * @ingroup modstd
 *
 * Initialize the library. Called by module manager on loading.
 *
 * @return non-zero on success
 */
int sblib_init(void);

/**
 * @ingroup modlib
 *
 * returns the module name
 *
 * @return module name
 */
const char *sblib_get_module_name();

/**
 * @ingroup modstd
 *
 * Closes the library. Called by module manager on unload.
 */
void sblib_close(void);

/**
 * @ingroup modstd
 *
 * returns the type of the library (slib_tp)
 *
 * @return the type of the library
 */
int sblib_type(void);

/**
 * @ingroup modlib
 *
 * returns the number of procedures that are supported by the library
 *
 * @return the number of the procedures
 */
int sblib_proc_count(void);

/**
 * @ingroup modlib
 *
 * returns the name of the procedure 'index'
 *
 * @param index the procedure's index
 * @param proc_name the buffer to store the name
 * @return non-zero on success
 */
int sblib_proc_getname(int index, char *proc_name);

/**
 * @ingroup modlib
 *
 * executes a procedure
 *
 * the retval can be used to returns an error-message
 * in case of an error.
 *
 * @param index the procedure's index
 * @param param_count the number of the parameters
 * @param params the parameters table
 * @param retval a var_t object to set the return value
 * @return non-zero on success
 */
int sblib_proc_exec(int index, int param_count,
                    slib_par_t *params, var_t *retval);

/**
 * @ingroup modlib
 *
 * returns the number of functions that are supported by the library
 *
 * @return the number of the functions
 */
int sblib_func_count(void);

/**
 * @ingroup modlib
 *
 * returns the name of the function 'index'
 *
 * @param index the function's index
 * @param func_name the buffer to store the name
 * @return non-zero on success
 */
int sblib_func_getname(int index, char *func_name);

/**
 * @ingroup modlib
 *
 * executes a function
 *
 * @param index the procedure's index
 * @param param_count the number of the parameters
 * @param params the parameters table
 * @param retval a var_t object to set the return value
 * @return non-zero on success
 */
int sblib_func_exec(int index, int param_count,
                    slib_par_t *params, var_t *retval);

#if defined(__cplusplus)
  }
#endif
#endif
