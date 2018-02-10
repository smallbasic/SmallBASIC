// This file is part of SmallBASIC
//
// SmallBASIC module header
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2018 Chris Warren-Smith

#include <stdint.h>

#if !defined(_INC_MODULE_H)
#define _INC_MODULE_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
  // the parameter
  var_t *var_p;

  // whether the parameter can be used by reference
  uint8_t byref;
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
int sblib_proc_exec(int index, int param_count, slib_par_t *params, var_t *retval);

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
int sblib_func_exec(int index, int param_count, slib_par_t *params, var_t *retval);

#if defined(__cplusplus)
}
#endif

#endif
