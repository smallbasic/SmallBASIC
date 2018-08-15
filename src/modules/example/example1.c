/*
 * SmallBASIC - external library example
 *
 * 2001/12/07, Nicholas Christopoulos
 */

#include <stdio.h>
#include <string.h>
#include "var.h"
#include "module.h"

const char *sblib_get_module_name() {
  return "example";
}

/**
 * prints a var_t
 */
void print_var_t(int level, var_t *param) {
  // place tabs
  for (int i = 0; i < level; i ++ ) {
    printf("\t");
  }
  // print var_t
  switch ( param->type )  {
  case  V_STR:
    printf("STR  =\"%s\"\n", param->v.p.ptr);
    break;
  case  V_INT:
    printf("INT  = %ld\n",    param->v.i);
    break;
  case  V_NUM:
    printf("REAL = %.2f\n",  param->v.n);
    break;
  case  V_ARRAY:
    printf("ARRAY of %d 'var_t' elements\n", param->v.a.size);
    for (int i = 0; i < param->v.a.size; i ++ ) {
      var_t *element_p = (var_t *) (param->v.a.data + sizeof(var_t) * i);
      print_var_t(level+1, element_p);
    }
    break;
  }
}

/**
 * our procedure
 *
 * this procedure justs displays all of its parameters
 */
int proc_libtest(int param_count, slib_par_t *params, var_t *retval) {
  printf("=== LIBTEST: EXTERNAL PROCEDURE STARTED  ===\n");
  printf("param-count: %d\n", param_count);
  for (int i = 0; i < param_count; i ++ )  {
    var_t *param = params[i].var_p;
    print_var_t(0, param);
  }
  printf("=== LIBTEST: EXTERNAL PROCEDURE FINISHED ===\n");
  return 1; // success
}

/**
 * our function
 *
 * this function justs returns a string
 */
int func_libtest(int param_count, slib_par_t *params, var_t *retval) {
  v_setstr(retval, "libfunctest works!");
  return 1; // success
}

/**
 *
 * code that required by SB
 *
 */

/**
 * returns the number of the procedures
 */
int sblib_proc_count(void) {
  return 1;
}

/**
 * returns the number of the functions
 */
int sblib_func_count(void) {
  return 1;
}

/**
 * returns the 'index' procedure name
 */
int sblib_proc_getname(int index, char *proc_name) {
  switch (index) {
  case 0:
    strcpy(proc_name, "LIBTEST");
    return 1; // success
  }

  return 0; // error
}

/**
 * returns the 'index' function name
 */
int sblib_func_getname(int index, char *proc_name) {
  switch (index) {
  case 0:
    strcpy(proc_name, "LIBFUNCTEST");
    return 1; // success
  }
  return 0; // error
}

/**
 * execute the 'index' procedure
 */
int sblib_proc_exec(int index, int param_count, slib_par_t *params, var_t *retval) {
  int success = 0;

  switch (index) {
  case 0:
    success = proc_libtest(param_count, params, retval);
    break;
  default:
    v_setstr(retval, "example1: procedure does not exist!");
  }

  return success;
}

/**
 * execute the 'index' function
 */
int sblib_func_exec(int index, int param_count, slib_par_t *params, var_t *retval) {
  int success = 0;
  switch (index)  {
  case 0:
    success = func_libtest(param_count, params, retval);
    break;
  default:
    v_setstr(retval, "example1: function does not exist!");
  }
  return success;
}
