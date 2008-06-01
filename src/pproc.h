/**
 * @file pproc.h
 * SmallBASIC, RTL API (Parameter's API)
 *
 * Nicholas Christopoulos
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 */

/**
 * @defgroup par Parameters
 */

#if !defined(_sb_proc_h)
#define _sb_proc_h

#include "sys.h"
#include "str.h"
#include "kw.h"
#include "panic.h"
#include "var.h"
#include "device.h"
#include "blib.h"
#include "sberr.h"
#include "smbas.h"

#if !defined(PI)
#define PI  3.14159265358979323846  /**< isn't the ð ? :) @ingroup sys */
#endif

/*
 * known also, as 'output' or 'method' for pv_* functions
 */
#define PV_CONSOLE        0
#define PV_FILE         1
#define PV_LOG          2
#define PV_STRING       3

#if defined(__cplusplus)
extern "C" {
#endif

#define pfree(a)    { if ((a))    tmp_free((a));    }       /**< simple macro for free() @ingroup par */
#define pfree2(a,b)   { pfree((a)); pfree((b));       }       /**< simple macro for free() 2 ptrs @ingroup par */
#define pfree3(a,b,c) { pfree2((a),(b)); pfree((c));    }       /**< simple macro for free() 3 ptrs @ingroup par */
#define pfree4(a,b,c,d) { pfree3((a),(b),(c)); pfree((d));  }       /**< simple macro for free() 4 ptrs @ingroup par */

/*
 * low-level
 */

/**
 * @ingroup exec
 *
 * proceed to a recursive execution-loop
 *
 * @note avoid to use it
 */
void bc_loop(int isf);

/**
 * @ingroup exec
 *
 * evaluate the next expression (starting from current instruction).
 * put the result on 'result' variable.
 *
 * @param result the variable to store the result.
 */
void eval(var_t * result);

/**
 * @ingroup exec
 *
 * allocate stack for eval(). used only by brun().
 *
 * @note avoid to use it
 */
void eval_alloc_stack(void);

/**
 * @ingroup exec
 *
 * free eval()'s stack. used only by brun().
 *
 * @note avoid to use it
 */
void eval_free_stack(void);

/**
 * @ingroup exec
 *
 * sets the data-p. the data-p is used for READ/DATA commands.
 * actually it is points to the next position of which the READ will use.
 */
void set_dataip(word label_id) SEC(BLIB);

#if defined(_PalmOS)
void pv_write(char *str, int method, unsigned long int handle) SEC(BIO2);
void pv_writevar(var_t * var, int method, unsigned long int handle) SEC(BIO2);
#else

/**
 * @ingroup exec
 *
 * write a string by using a specific method
 *
 * @note avoid to use it
 */
void pv_write(char *str, int method, int handle) SEC(BIO2);

/**
 * @ingroup exec
 *
 * write a variable-contents by using a specific method
 *
 * @note avoid to use it
 */
void pv_writevar(var_t * var, int method, int handle) SEC(BIO2);
#endif

/**
 * @ingroup exec
 *
 * prints a variable's contents in console
 *
 * @param var is the variable
 */
void print_var(var_t * var) SEC(BIO2);

/**
 * @ingroup exec
 *
 * writes a variable's contents in a file
 *
 * @param var is the variable
 * @param handle is the file-handle
 */
void fprint_var(int handle, var_t * var) SEC(BIO2);

/**
 * @ingroup exec
 *
 * prints a variable's contents to log-file
 *
 * @param var is the variable
 */
void logprint_var(var_t * var) SEC(BIO2);

/*
 * Parameter's API
 *
 * Use these functions
 */

/**
 * @ingroup par
 * @typedef par_t
 * Parameter structure, used in 'partables'
 */
typedef struct {
  var_t *var;                 /**< a var_t pointer; the data */

  byte prev_sep;              /**< previous separator (default '\0') */
  byte next_sep;              /**< next separator (default '\0') */

  byte flags;                 /**< 0x1 = its a 'byval' and the 'var' must be released */
} par_t;

/* par_t flags */
#define PAR_BYVAL 1       /**< pat_t::flags,  parameter was an expression (var = the temporary copy of the result) @ingroup par */

/*
 * low-level parameters parser
 */

/**
 * @ingroup par
 *
 * get next parameter as any-value (variable).
 * moves IP to the next position.
 *
 * @param var the variable to copy the data
 */
void par_getvar(var_t * var) SEC(BLIB);

/**
 * @ingroup par
 *
 * get next parameter as variable's pointer.
 * moves IP to the next position.
 *
 * @return the var_t pointer
 */
var_t *par_getvar_ptr(void) SEC(BLIB);

/**
 * @ingroup par
 *
 * get next parameter as string-var_t
 * moves IP to the next position.
 *
 * @param var the variable to copy the data
 */
void par_getstr(var_t * var) SEC(BLIB);

/**
 * @ingroup par
 *
 * get next parameter as integer
 * moves IP to the next position.
 *
 * @return the integer
 */
long par_getint(void) SEC(BLIB);

/**
 * @ingroup par
 *
 * get next parameter as double
 * moves IP to the next position.
 *
 * @return the number
 */
double par_getnum(void) SEC(BLIB);
#define par_getreal()   par_getnum()

/**
 * @ingroup par
 *
 * no-error if the next byte is a separator.
 * moves IP to the next position.
 *
 * @return the separator
 */
int par_getsep(void) SEC(BLIB);

/**
 * @ingroup par
 *
 * no-error if the next byte is the separator ','.
 * moves IP to the next position.
 */
void par_getcomma(void) SEC(BLIB);

/**
 * @ingroup par
 *
 * no-error if the next byte is the separator '#'.
 * moves IP to the next position.
 */
void par_getsharp(void) SEC(BLIB);

/**
 * @ingroup par
 *
 * no-error if the next byte is the separator ';'.
 * moves IP to the next position.
 */
void par_getsemicolon(void) SEC(BLIB);

/**
 * @ingroup par
 *
 * get next parameter as variable-pointer of an array.
 * moves IP to the next position.
 *
 * @return the var_t pointer
 */
var_t *par_getvarray(void) SEC(BLIB);

/**
 * @ingroup par
 *
 * returns true if the following code is descibing one var code
 * usefull for optimization
 * (one var can be used by the pointer; more than one it must be evaluated)
 *
 * @note ignore it for now
 * @return true if the following code is descibing one var code
 */
int par_isonevar(void) SEC(BLIB);

/**
 * @ingroup par
 *
 * skip parameter.
 * moves IP to the next position.
 */
void par_skip(void) SEC(BLIB);

/*
 * high-level parameters parser
 */

/**
 * @ingroup par
 *
 * builds a parameter table
 *
 * ptable_pp = pointer to an ptable
 * valid_sep = valid separators (,;)
 *
 * returns the number of the parameters, OR, -1 on error
 *
 * YOU MUST FREE THAT TABLE BY USING par_freepartable()
 * IF THERE IS AN ERROR THE CALL TO par_freepartable IS NOT NEEDED
 *
 * moves IP to the next position.
 *
 * @param ptable_pp pointer to a par_t table
 * @param valid_sep the valid parameter separators
 * @return on success the number of the parameters; otherwise -1
 * @see par_freepartable, par_massget
 */
int par_getpartable(par_t ** ptable_pp, const char *valid_sep) SEC(BLIB);

/**
 * @ingroup par
 *
 * frees a parameters-table
 *
 * @param ptable_pp pointer to a par_t table
 * @param pcount the number of the parameters
 * @see par_getpartable, par_massget
 */
void par_freepartable(par_t ** ptable_pp, int pcount) SEC(BLIB);

/**
 * @ingroup par
 *
 * parsing parameters with scanf-style. returns the parameter-count or -1 (error)
 *
 * this is the preferred method for any command except statements.
 *
 <pre>
 * Format:
 * --------
 * capital character = the parameter is required
 * small character   = optional parameter
 *
 * I = integer     (int32  )
 * F = double      (double*)
 * S = string      (char*  )
 * P = variable's ptr  (var_t* )
 </pre>
 *
 * <b>Example</b>
 *
 @code
 int32 i1, i2 = -1;  // -1 is the default value for i2
 char  *s1 = NULL;   // NULL is the default value for s1
 var_t *v  = NULL;   // NULL is the default value for v

 // the first integer is required, the second is optional
 // the string is optional too
 pc = par_massget("Iisp", &i1, &i2, &s1, &v);

 if ( pc != -1 ) { // no error; also, you can use prog_error because par_massget() will call rt_raise() on error
 printf("required integer = %d\n", i1);

 // if there is no optional parameters, the default value will be returned
 if  ( i2 != -1 )  printf("optional integer found = %d\n", i2);
 if  ( s1 )      printf("optional string found = %s\n", s1);
 if  ( v )   { printf("optional variable's ptr found");  v_free(v);  }
 }

 pfree2(s1, v);
 @endcode
 *
 * moves IP to the next position.
 *
 * @param fmt the par_massget's format
 * @param ... the format's parameters
 * @return on success the parameter-count; otherwise -1
 */
int par_massget(const char *fmt, ...) SEC(BLIB);

/**
 * @ingroup par
 *
 * execute a user's expression (using one variable).
 * the result will be stored in 'var'.
 *
 * @note the keyword USE
 *
 * @param var the variable (the X)
 * @param ip the expression's address
 */
void exec_usefunc(var_t * var, addr_t ip) SEC(BLIB);  // one parameter (x)

/**
 * @ingroup par
 *
 * execute a user's expression (using two variables).
 * the result will be stored in 'var1'.
 *
 * @note the keyword USE
 *
 * @param var1 the variable (the X)
 * @param var2 the variable (the Y)
 * @param ip the expression's address
 */
void exec_usefunc2(var_t * var1, var_t * var2, addr_t ip) SEC(BLIB);  // two
// parameters
// (x,y)

/**
 * @ingroup par
 *
 * execute a user's expression (using three variables).
 * the result will be stored in 'var1'.
 *
 * @note the keyword USE
 *
 * @param var1 the variable (the X)
 * @param var2 the variable (the Y)
 * @param var3 the variable (the Z)
 * @param ip the expression's address
 */
void exec_usefunc3(var_t * var1, var_t * var2, var_t * var3, addr_t ip) SEC(BLIB);  // three
//
// parameters
// (x,y,z)

/*
 * Special parameters
 */

/**
 * @ingroup par
 *
 * retrieve a 2D point (double).
 * moves IP to the next position.
 *
 * @return a pt_t point structure
 */
pt_t par_getpt(void) SEC(BIO2); // get a point parameter

/**
 * @ingroup par
 *
 * retrieve a 2D polyline (double).
 * moves IP to the next position.
 *
 * @param poly pointer to a table of real-points
 * @return on success the number of points; otherwise 0
 */
int par_getpoly(pt_t ** poly) SEC(BIO2);  // get a polyline

/**
 * @ingroup par
 *
 * retrieve a 2D point (integer).
 * moves IP to the next position.
 *
 * @return a ipt_t point structure
 */
ipt_t par_getipt(void) SEC(BIO2);

/**
 * @ingroup par
 *
 * retrieve a 2D polyline (integers).
 * moves IP to the next position.
 *
 * @param poly pointer to a table of integer-points
 * @return on success the number of points; otherwise 0
 */
int par_getipoly(ipt_t ** poly) SEC(BIO2);  // get a polyline (integers)

#if defined(__cplusplus)
}
#endif
#endif
