// This file is part of SmallBASIC
//
// SmallBASIC Code & Variable Manager.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup var Variables
 */
/**
 * @defgroup exec Executor
 */
/**
 * @ingroup var
 * @page var_12_2001 Var API (Dec 2001)
 *
 * @code
 * Use these routines
 *
 * Memory free/alloc is contolled inside these functions
 * The only thing that you must care of, is when you declare local var_t elements
 *
 * Auto-type-convertion is controlled inside these functions,
 * So if you want a string value of an integer you just do strcpy(buf,v_getstr(&myvar));
 * or a numeric value of a string R = v_getreal(&myvar);
 *
 * Using variables on code:
 *
 * void myfunc() {    // using them in stack
 *  var_t    myvar;
 *  v_init(&myvar);  // DO NOT FORGET THIS! local variables are had random data
 *  ...
 *  v_setstr(&myvar, "Hello, world");
 *  ...
 *  v_set(&myvar, &another_var); // copy variables (LET myvar = another_var)
 *  ...
 *  v_setint(&myvar, 0x100);     // Variable will be cleared automatically
 *  ...
 *  v_free(&myvar);
 * }
 *
 * void myfunc() {                   // using dynamic memory
 *  var_t *myvar_p;
 *
 *  myvar_p = v_new();               //  create a new variable
 *  ...
 *  v_setstr(myvar_p, "Hello, world");
 *  ...
 *  v_setint(myvar_p, 0x100);        // Variable will be cleared automatically
 *  ...
 *  v_free(myvar_p);             // clear variable's data
 *  v_detach(myvar_p);               // free the variable
 * }
 *
 * @endcode
 */

#if !defined(_sb_cvm_h)
#define _sb_cvm_h

#include "common/sys.h"

/*
 *   predefined system variables - index
 */
#define SYSVAR_SBVER        0  /**< system variable, SBVER     @ingroup var */
#define SYSVAR_PI           1  /**< system variable, PI        @ingroup var */
#define SYSVAR_XMAX         2  /**< system variable, XMAX      @ingroup var */
#define SYSVAR_YMAX         3  /**< system variable, YMAX      @ingroup var */
#define SYSVAR_TRUE         4  /**< system variable, TRUE      @ingroup var */
#define SYSVAR_FALSE        5  /**< system variable, FALSE     @ingroup var */
#define SYSVAR_CWD          6  /**< system variable, CWD$      @ingroup var */
#define SYSVAR_HOME         7  /**< system variable, HOME$     @ingroup var */
#define SYSVAR_COMMAND      8  /**< system variable, COMMAND$  @ingroup var */
#define SYSVAR_X            9  /**< system variable, X         @ingroup var */
#define SYSVAR_Y            10 /**< system variable, Y         @ingroup var */
#define SYSVAR_SELF         11 /**< system variable, SELF      @ingroup var */
#define SYSVAR_NONE         12 /**< system variable, NONE      @ingroup var */
#define SYSVAR_MAXINT       13 /**< system variable, INTMAX    @ingroup var */
#define SYSVAR_COUNT        14

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * label
 */
typedef struct lab_s {
  bcip_t ip;
} lab_t;

/**
 * @ingroup exec
 * @struct stknode_s
 *
 * EXECUTOR's STACK NODE
 */
typedef struct stknode_s {
  union {
    /**
     *  FOR-TO-NEXT
     */
    struct {
      var_t *var_ptr; /**< 'FOR' variable */
      var_t *arr_ptr; /**< FOR-IN array-variable */
      bcip_t to_expr_ip; /**< IP of 'TO' expression */
      bcip_t step_expr_ip; /**< IP of 'STEP' expression (FOR-IN = current element) */
      bcip_t jump_ip; /**< code block IP */
      bcip_t exit_ip; /**< EXIT command IP to go */
      code_t subtype; /**< kwTO | kwIN */
      byte flags; /**< ... */
    } vfor;

    /**
     *  REPEAT/WHILE
     */
    struct {
      bcip_t exit_ip; /**< EXIT command IP to go */
    } vloop;

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
      var_t *retvar;   /**< return-variable data */
      bcip_t ret_ip;   /**< return ip */
      bid_t rvid;      /**< return-variable ID */
      int task_id; /**< task_id or -1 (this task) */
      uint16_t pcount; /**< number of parameters */
    } vcall;

    /**
     *  Create dynamic variable (LOCAL or PARAMETER)
     */
    struct {
      var_t *vptr; /**< previous variable */
      bid_t vid; /**< variable index in tvar */
    } vdvar;

    /**
     *  parameter (CALL UDP/F)
     */
    struct {
      var_t *res; /**< variable pointer (for BYVAL this is a clone) */
      uint16_t vcheck; /**< checks (1=BYVAL ONLY, 3=BYVAL|BYREF, 2=BYREF ONLY) */
    } param;

    /**
     * try/catch
     */
    struct {
      bcip_t catch_ip;
    } vtry;

    struct {
      var_t *catch_var;
    } vcatch;
  } x;

  int line; /** line number of current execution **/
  code_t type; /**< type of node (keyword id, i.e. kwGOSUB, kwFOR, etc) */
} stknode_t;

/**
 * @ingroup var
 *
 * intialises the var pool
 */
void v_init_pool(void);

/**
 * @ingroup var
 *
 * free pooled var
 */
void v_pool_free(var_t *var);

/**
 * @ingroup var
 *
 * creates a new variable
 *
 * @return a newly created var_t object
 */
var_t *v_new(void);

/**
 * < returns the integer value of variable v
 * @ingroup var
 */
#define v_getint(v) v_igetval((v))

/**
 * < returns the real value of variable v
 * @ingroup var
 */
#define v_getreal(v) v_getval((v))

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

/**
 * @ingroup exec
 *
 * skips over the label
 *
 * @param label_id
 */
void code_jump_label(uint16_t label_id);  // IP <- LABEL_IP_TABLE[label_id]

/**
 * @ingroup exec
 *
 * skips to the specified address
 *
 * @param newip
 */
#define code_jump(newip) prog_ip=(newip) /**< IP <- NewIP @ingroup exec */

/**
 * @ingroup exec
 *
 * stores a node to stack
 *
 * @param node the stack node
 */
stknode_t *code_push(code_t type);

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
 * POPs and frees the topmost node from stack and returns the node type
 *
 */
int code_pop_and_free();

/**
 * @ingroup exec
 *
 * returns the node at the top of the stack. does not change the stack.
 *
 */
stknode_t *code_stackpeek();

/**
 * @ingroup var
 *
 * Returns the varptr of the next variable. if the variable is an array
 * returns the element ptr
 */
#define code_getvarptr() code_getvarptr_parens(0)

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
#define code_getnext16()    (*((uint16_t *)(prog_source+(prog_ip+=2)-2)))
#define code_peeknext16()   (*((uint16_t *)(prog_source+prog_ip)))
#define code_peek16(o)      (*((uint16_t *)(prog_source+(o))))
#define code_peek32(o)      (*((uint32_t *)(prog_source+(o))))
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
