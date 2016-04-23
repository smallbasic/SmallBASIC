// This file is part of SmallBASIC
//
// SmallBasic Code & Variable Manager.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 *   @defgroup var Variables
 */
/**
 *   @defgroup exec Executor
 */

#if !defined(_sb_cvm_h)
#define _sb_cvm_h

#include "common/sys.h"

#ifdef V_INT
#undef V_INT
#endif

#ifdef V_ARRAY
#undef V_ARRAY
#endif

/*
 * Variable - types
 */
#define V_INT       0 /**< variable type, 32bit integer                @ingroup var */
#define V_NUM       1 /**< variable type, 64bit float (same as V_NUM)  @ingroup var */
#define V_STR       2 /**< variable type, string                       @ingroup var */
#define V_ARRAY     3 /**< variable type, array of variables           @ingroup var */
#define V_PTR       4 /**< variable type, pointer to UDF or label      @ingroup var */
#define V_MAP       5 /**< variable type, associative array            @ingroup var */
#define V_REF       6 /**< variable type, reference another var        @ingroup var */
#define V_FUNC      7 /**< variable type, object method                @ingroup var */

/*
 *   predefined system variables - index
 */
#define SYSVAR_SBVER        0  /**< system variable, SBVER     @ingroup var */
#define SYSVAR_PI           1  /**< system variable, PI        @ingroup var */
#define SYSVAR_XMAX         2  /**< system variable, XMAX      @ingroup var */
#define SYSVAR_YMAX         3  /**< system variable, YMAX      @ingroup var */
#define SYSVAR_TRUE         4  /**< system variable, TRUE      @ingroup var */
#define SYSVAR_FALSE        5  /**< system variable, FALSE     @ingroup var */
#define SYSVAR_PWD          6  /**< system variable, PWD$      @ingroup var */
#define SYSVAR_HOME         7  /**< system variable, HOME$     @ingroup var */
#define SYSVAR_COMMAND      8  /**< system variable, COMMAND$  @ingroup var */
#define SYSVAR_X            9  /**< system variable, X         @ingroup var */
#define SYSVAR_Y            10 /**< system variable, Y         @ingroup var */
#define SYSVAR_Z            11 /**< system variable, Z         @ingroup var */
#define SYSVAR_COUNT        12

/**
 * @ingroup var
 * @def MAXDIM Maxium number of array-dimensions
 */
#define MAXDIM 6     // think before increase this, (possible stack overflow)

#if defined(__cplusplus)
extern "C" {
#endif

struct var_s;
typedef void (*method) (struct var_s *self);

/**
 * @ingroup var
 * @typedef var_s
 *
 * VARIANT DATA TYPE
 */
struct var_s {
  byte type; /**< variable's type */
  byte const_flag; /**< non-zero if constants */

  // value
  union {
    var_num_t n; /**< numeric value */
    var_int_t i; /**< integer value */

    // pointer to sub/func variable
    struct {
      bcip_t p; /** address pointer */
      bcip_t v; /** return-var ID */
    } ap;

    // associative array/map
    struct {
      void *map; /** pointer the map structure */
      int32_t size;
    } m;

    // reference variable
    struct var_s *ref;

    // object method
    struct {
      method cb;
      struct var_s *self;
    } fn;

    // generic ptr (string)
    struct {
      char *ptr; /**< data ptr (possibly, string pointer) */
      int32_t size; /**< the size of string */
      int32_t pos; /**< position in string (used by pv_* functions) */
    } p;

    // array
    struct {
      byte *ptr; /**< array data ptr (sizeof(var_t) * size) */
      int32_t size; /**< the number of elements */
      int32_t lbound[MAXDIM]; /**< lower bound */
      int32_t ubound[MAXDIM]; /**< upper bound */
      byte maxdim; /**< number of dimensions */
    } a;
  } v;
};

typedef struct var_s var_t;
typedef var_t *var_p_t;

/*
 * label
 */
struct lab_s {
  bcip_t ip;
};
typedef struct lab_s lab_t;

/**
 * @ingroup exec
 * @struct stknode_s
 *
 * EXECUTOR's STACK NODE
 */
struct stknode_s {
  code_t type; /**< type of node (keyword id, i.e. kwGOSUB, kwFOR, etc) */
  bcip_t exit_ip; /**< EXIT command IP to go */
  int line; /** line number of current execution **/

  union {
    /**
     *  FOR-TO-NEXT
     */
    struct {
      code_t subtype; /**< kwTO | kwIN */
      var_t *var_ptr; /**< 'FOR' variable */
      var_t *arr_ptr; /**< FOR-IN array-variable */
      bcip_t to_expr_ip; /**< IP of 'TO' expression */
      bcip_t step_expr_ip; /**< IP of 'STEP' expression (FOR-IN = current element) */
      bcip_t jump_ip; /**< code block IP */
      byte flags; /**< ... */
    } vfor;

    /**
     *  IF/ELIF
     */
    struct {
      bcip_t lcond; /**< result of the last condition */
    } vif;

    /**
     *  SELECT CASE
     */
    struct {
      var_t *var_ptr;
      byte flags;
    } vcase;

    /**
     *  GOSUB
     */
    struct {
      bcip_t ret_ip; /**< return ip */
    } vgosub;

    /**
     *  CALL UDP/F
     */
    struct {
      bcip_t ret_ip; /**< return ip */
      word pcount; /**< number of parameters */
      bid_t rvid; /**< return-variable ID */
      var_t *retvar; /**< return-variable data */

      // unit - version
      int task_id; /**< task_id or -1 (this task) */
    } vcall;

    /**
     *  Create dynamic variable (LOCAL or PARAMETER)
     */
    struct {
      bid_t vid; /**< variable index in tvar */
      var_t *vptr; /**< previous variable */
    } vdvar;

    /**
     *  parameter (CALL UDP/F)
     */
    struct {
      word vcheck; /**< checks (1=BYVAL ONLY, 3=BYVAL|BYREF, 2=BYREF ONLY) */
      var_t *res; /**< variable pointer (for BYVAL this is a clone) */
    } param;

    /**
     * try/catch
     */
    struct {
      bcip_t catch_ip;
    } vtry;
  } x;
};
typedef struct stknode_s stknode_t;

/*
 * Basic variable's API
 */

/**
 * @ingroup var
 *
 * creates a new variable
 *
 * @return a newly created var_t object
 */
var_t *v_new(void);

/**
 * @ingroup var
 *
 * returns true if the value is not 0/NULL
 *
 * @param v the variable
 * @return true if the value is not 0/NULL
 */
int v_is_nonzero(var_t *v);

/**
 * @ingroup var
 *
 * compares two variables
 *
 * @param a the left-side variable
 * @param b the right-side variable
 * @return 0 if a = b, <0 if a < b, >0 if a > b
 */
int v_compare(var_t *a, var_t *b);

/**
 * @ingroup var
 *
 * calculates the result type of the addition of two variables
 *
 * @param a the left-side variable
 * @param b the right-side variable
 * @return the type of the new variable
 */
int v_addtype(var_t *a, var_t *b);

/**
 * @ingroup var
 *
 * adds two variables
 *
 * @param result the result
 * @param a the left-side variable
 * @param b the right-side variable
 */
void v_add(var_t *result, var_t *a, var_t *b);

/**
 * @ingroup var
 *
 * assigning: dest = src
 *
 * @param dest the destination-var
 * @param src the source-var
 */
void v_set(var_t *dest, const var_t *src);

/**
 * @ingroup var
 *
 * increase the value of variable a by b
 * (similar to v_add())
 *
 * @param a is the variable
 * @param b is the increment
 */
void v_inc(var_t *a, var_t *b);

/**
 * @ingroup var
 *
 * returns the sign of a variable
 *
 * @param x the variable
 * @return the sign
 */
int v_sign(var_t *x);

/**
 * @ingroup var
 *
 * returns the var_t pointer of an array element
 *
 * @param v is the array-variable
 * @param index is the element's index number
 * @return the var_t pointer of an array element
 */
var_t *v_getelemptr(var_t *v, dword index);

/**
 * @ingroup var
 *
 * create a string variable (with value str)
 *
 * @param v is the variable
 * @param src is the string
 */
void v_createstr(var_t *v, const char *src);

/**
 * @ingroup var
 *
 * print variable as string
 *
 * @param arg is the variable
 */
char *v_str(var_t *arg);

/**
 * @ingroup var
 *
 * convert variable to string
 *
 * @param arg is the variable
 */
void v_tostr(var_t *arg);

/**
 * @ingroup var
 *
 * copies data from one user defined structure to another
 */
void v_set_uds(bcip_t dst_ip, bcip_t src_ip);

/**
 * @ingroup var
 *
 * clones data from one user defined structure to another. pushes
 * replaced variables onto the stack for later clean
 *
 */
void v_clone_uds(bcip_t dst_ip, bcip_t src_ip);

/*
 * returns the starting address for the uds of the given id
 */
bcip_t v_get_uds_ip(bcip_t var_id);

/**
 * @ingroup var
 *
 * creates a new variable which is a clone of 'source'.
 *
 * @param source is the source
 * @return a newly created var_t object, clone of 'source'
 */
var_t *v_clone(const var_t *source);

/**
 * @ingroup var
 *
 * resizes an array-variable to 1-dimension array of 'size' elements
 *
 * @param v the variable
 * @param size the number of the elements
 */
void v_resize_array(var_t *v, dword size);

/**
 * @ingroup var
 *
 * convert variable v to a RxC matrix
 *
 * @param v the variable
 * @param r the number of the rows
 * @param c the number of the columns
 */
void v_tomatrix(var_t *v, int r, int c);

/**
 * @ingroup var
 *
 * creates and returns a new matrix (array RxC) variable
 *
 * @param r the number of the rows
 * @param c the number of the columns
 * @return a newly created variable
 */
var_t *v_new_matrix(int r, int c);

/**
 * @ingroup var
 *
 * converts the variable v to an array of R elements.
 * R can be zero for zero-length arrays
 *
 * @param v the variable
 * @param r the number of the elements
 */
void v_toarray1(var_t *v, dword r);

/**
 * @ingroup var
 *
 * returns true if the 'v' is empty (see EMPTY())
 *
 * @param v the variable
 * @return non-zero if v is not 'empty'
 */
int v_isempty(var_t *v);

/**
 * @ingroup var
 *
 * returns the length of the variable (see LEN())
 *
 * @param v the variable
 * @return the length of the variable
 */
int v_length(var_t *v);

/**
 * @ingroup var
 * @page var_12_2001 Var API (Dec 2001)
 *
 @code
 Use these routines

 Memory free/alloc is contolled inside these functions
 The only thing that you must care of, is when you declare local var_t elements

 Auto-type-convertion is controlled inside these functions,
 So if you want a string value of an integer you just do strcpy(buf,v_getstr(&myvar));
 or a numeric value of a string R = v_getnum(&myvar);

 Using variables on code:

 void    myfunc()    // using them in stack
 {
 var_t    myvar;
 v_init(&myvar);  // DO NOT FORGET THIS! local variables are had random data
 ...
 v_setstr(&myvar, "Hello, world");
 ...
 v_set(&myvar, &another_var); // copy variables (LET myvar = another_var)
 ...
 v_setint(&myvar, 0x100);     // Variable will be cleared automatically
 ...
 v_free(&myvar);
 }

 void    myfunc()                    // using dynamic memory
 {
 var_t    *myvar_p;

 myvar_p = v_new();               //  create a new variable
 ...
 v_setstr(myvar_p, "Hello, world");
 ...
 v_setint(myvar_p, 0x100);        // Variable will be cleared automatically
 ...
 v_free(myvar_p);             // clear variable's data
 free(myvar_p);               // free the variable
 }

 Old API routines that is good to use them:

 v_init(), v_free()                  --- basic
 v_new(), v_clone(), v_new_matrix()  --- create vars in heap, dont forget the free()

 v_resize_array()                    --- resize an 1-dim array
 v_tomatrix(), v_toarray1()          --- convertion to matrix (RxC) or 1D array

 @endcode
 */

/**
 * @ingroup var
 *
 * sets a string value to variable 'var'
 *
 * @param var is the variable
 * @param string is the string
 */
void v_setstr(var_t *var, const char *string);

/**
 * @ingroup var
 *
 * sets a string value to variable 'var' to the given length
 *
 * @param var is the variable
 * @param string is the string
 */
void v_setstrn(var_t *var, const char *string, int len);

/**
 * @ingroup var
 *
 * Sets a string value to variable 'var'.
 * printf() style.
 *
 * @param var is the variable
 * @param fmt is the the format
 */
void v_setstrf(var_t *var, const char *fmt, ...);

/**
 * @ingroup var
 *
 * concate string to variable 'var'
 *
 * @param var is the variable
 * @param string is the string
 */
void v_strcat(var_t *var, const char *string);

/**
 * @ingroup var
 *
 * sets a real-number value to variable 'var'
 *
 * @param var is the variable
 * @param number is the number
 */
void v_setreal(var_t *var, var_num_t number);

/**
 * @ingroup var
 *
 * sets an integer value to variable 'var'
 *
 * @param var is the variable
 * @param integer is the integer
 */
void v_setint(var_t *var, var_int_t integer);

/**
 * @ingroup var
 *
 * makes 'var' an array of integers and copies the itable elements to it
 *
 * @param var is the variable
 * @param itable is the table of integers
 * @param count the number of the elements
 */
void v_setintarray(var_t *var, int32_t *itable, int count);

/**
 * @ingroup var
 *
 * makes 'var' an array of reals and copies the ntable elements to it
 *
 * @param var is the variable
 * @param ntable is the table of doubles
 * @param count the number of the elements
 */
void v_setrealarray(var_t *var, var_num_t *ntable, int count);

/**
 * @ingroup var
 *
 * makes 'var' an array of strings and copies the ctable elements to it
 *
 * @param var is the variable
 * @param ctable is the table of strings
 * @param count the number of the elements
 */
void v_setstrarray(var_t *var, char **ctable, int count);

/**
 * @ingroup var
 *
 * makes 'var' an empty string variable
 *
 * @param var is the variable
 */
void v_zerostr(var_t *var);

/**< makes 'var' an empry integer variable
 @ingroup var
 */
#define v_zeroint(r) v_init((r))

/**
 * @ingroup var
 *
 * assign value 'str' to var. the final type of the var will be decided
 * on that function (numeric if the str is a numeric-constant string or string)
 *
 * @note used by INPUT to convert the variables
 *
 * @param str is the string
 * @param var is the variable
 */
void v_input2var(const char *str, var_t *var);

/**
 *< returns the var_t pointer of the element i
 * on the array x. i is a zero-based, one dim, index.
 * @ingroup var
*/
#define v_elem(x,i)     (var_t *) ( (x)->v.a.ptr + (sizeof(var_t) * (i)))

/**
 * < the number of the elements of the array (x)
 * @ingroup var
 */
#define v_asize(x)      ((x)->v.a.size)

/**
 * < returns the integer value of variable v
 * @ingroup var
 */
#define v_getint(v)  v_igetval((v))

/**
 * < returns the real value of variable v
 * @ingroup var
 */
#define v_getreal(v)  v_getval((v))

/**
 * @ingroup var
 *
 * returns the string-pointer of variable v
 *
 * @param v is the variable
 * @return the pointer of the string
 */
char *v_getstr(var_t *v);

/**
 * @ingroup var
 *
 * returns whether the variable is of the given type
 *
 * @param v is the variable
 * @return whether the variable is of the given type
 */
#define v_is_type(v, t) (v != NULL && v->type == t)

/**
 * @ingroup var
 *
 * creates an system image object
 *
 * @param v is the variable
 */
void v_create_image(var_p_t var);

/**
 * @ingroup var
 *
 * creates an system form object
 *
 * @param v is the variable
 */
void v_create_form(var_p_t var);

/**
 * @ingroup var
 *
 * creates an system window object
 *
 * @param v is the variable
 */
void v_create_window(var_p_t var);

void code_jump_label(word label_id);  // IP <- LABEL_IP_TABLE[label_id]

#define code_jump(newip) prog_ip=(newip) /**< IP <- NewIP @ingroup exec */

/**
 * @ingroup exec
 *
 * stores a node to stack
 *
 * @param node the stack node
 */
void code_push(stknode_t *node);

/**
 * @ingroup exec
 *
 * restores the topmost node from stack
 *
 * @param node the stack node
 */
void code_pop(stknode_t *node, int expected_type);

/**
 * @ingroup exec
 *
 * Returns and deletes the topmost node from stack (POP)
 *
 * @param node the stack node
 */
void code_pop_and_free(stknode_t *node);

/**
 * @ingroup exec
 *
 * returns the node at the top of the stack. does not change the stack.
 *
 */
stknode_t *code_stackpeek();

#define code_peek()         prog_source[prog_ip]    /**< R(byte) <- Code[IP]          @ingroup exec */
#define code_getnext()      prog_source[prog_ip++]  /**< R(byte) <- Code[IP]; IP ++;  @ingroup exec */

#define code_skipnext()     prog_ip++   /**< IP ++;   @ingroup exec */
#define code_skipnext16()   prog_ip+=2  /**< IP += 2; @ingroup exec */
#define code_skipnext32()   prog_ip+=4  /**< IP += 4; @ingroup exec */
#define code_skipnext64()   prog_ip+=8  /**< IP += 8; @ingroup exec */

#if defined(CPU_BIGENDIAN)
#define code_getnext16()    (prog_ip+=2, (prog_source[prog_ip-2]<<8)|prog_source[prog_ip-1])
#define code_peeknext16()   ((prog_source[prog_ip]<<8)|prog_source[prog_ip+1])
#define code_peek16(o)      ((prog_source[(o)]<<8)|prog_source[(o)+1])
#define code_peek32(o)      (((bcip_t)code_peek16((o)) << 16) + (bcip_t)code_peek16((o)+2))
#else
#define code_getnext16()    (*((word *)(prog_source+(prog_ip+=2)-2)))
#define code_peeknext16()   (*((word *)(prog_source+prog_ip)))
#define code_peek16(o)      (*((word *)(prog_source+(o))))
#define code_peek32(o)      (*((dword *)(prog_source+(o))))
#endif

#define code_skipopr()   code_skipnext16()    /**< skip operator  @ingroup exec */
#define code_skipsep()   code_skipnext16()    /**< skip separator @ingroup exec */
  /**< returns the separator and advance (IP) to next command @ingroup exec */
#define code_getsep()    (prog_ip ++, prog_source[prog_ip++])
#define code_peeksep()   (prog_source[prog_ip+1])

#define code_getaddr()   code_getnext32()  /**< get address value and advance        @ingroup exec */
#define code_skipaddr()  code_skipnext32() /**< skip address field                   @ingroup exec */
#define code_getstrlen() code_getnext32()  /**< get strlen (kwTYPE_STR) and advance  @ingroup exec */
#define code_peekaddr(i) code_peek32((i))  /**< peek address field at offset i       @ingroup exec */

/**
 * @ingroup var
 * @page sysvar System variables
 * @code
 * System variables (osname, osver, bpp, xmax, etc)
 *
 * The variables must be defined
 * a) here (see in first lines) - (variable's index)
 * b) in scan.c (variable's name)
 * c) in brun.c or device.c (variable's value)
 *
 * DO NOT LOSE THE ORDER
 * @endcode
 */

/**
 * @ingroup var
 *
 * sets an integer value to a system variable (constant)
 *
 * @param index is the system variable's index
 * @param val the value
 */
void setsysvar_int(int index, var_int_t val);

/**
 * @ingroup var
 *
 * sets a double value to a system variable (constant)
 *
 * @param index is the system variable's index
 * @param val the value
 */
void setsysvar_num(int index, var_num_t val);

/**
 * @ingroup var
 *
 * sets a string value to a system variable (constant)
 *
 * @param index is the system variable's index
 * @param val the value
 */
void setsysvar_str(int index, const char *value);

/*
 * in eval.c
 */
var_num_t *mat_toc(var_t *v, int32_t *rows, int32_t *cols);

void mat_tov(var_t *v, var_num_t *m, int32_t rows, int32_t cols,
             int protect_col1);

#if defined(__cplusplus)
}
#endif

#endif
