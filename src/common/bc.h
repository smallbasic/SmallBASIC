// This file is part of SmallBASIC
//
// bc module. Bytecode manipulation routines (bytecode segments API)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_bc_h)
#define _bc_h

#include "common/sys.h"
#include "common/kw.h"

#define BC_ALLOC_INCR 1024
#define V_QUOTE '\1'
#define V_LINE '\2'
#define V_JOIN_LINE '\3'

/**
 * @ingroup scan
 * @typedef bc_t
 *
 * byte-code segment
 */
typedef struct {
  code_t *ptr; /**< pointer to byte-code */
  bcip_t cp; /**< current position (used by readers not writers) */
  bcip_t size; /**< allocation size (optimization) */
  bcip_t count; /**< current size (used by writers as the current position) */
  bcip_t eoc_position; /**< position of most recent kwTYPE_EOC or kwTYPE_LINE*/
  bcip_t line_position; /**< position of most recent kwTYPE_LINE */
} bc_t;

/**
 * @ingroup scan
 *
 * raise compiler error
 *
 * @param fmt the printf's format
 * @param ... format's parameters
 */
void sc_raise(const char *fmt, ...);

/**
 * @ingroup scan
 *
 * create a byte-code segment
 *
 * @param bc the bc structure
 */
void bc_create(bc_t *bc);

/**
 * @ingroup scan
 *
 * destroy a byte-code segment
 *
 * @param bc the bc structure
 */
void bc_destroy(bc_t *bc);

/**
 * @ingroup scan
 *
 * resize a byte-code segment
 *
 * @param bc the bc structure
 * @param newsize the new size
 */
void bc_resize(bc_t *bc, uint32_t newsize);

/**
 * @ingroup scan
 *
 * add 1 byte to segment
 *
 * @param bc the bc structure
 * @param code the byte
 */
void bc_add1(bc_t *bc, char code);

/**
 * @ingroup scan
 *
 * add 1 uint32_t (4-bytes) to segment
 *
 * @param bc the bc structure
 * @param code the uint32_t
 */
void bc_add_dword(bc_t *bc, uint32_t code);

/**
 * @ingroup scan
 *
 * adds a string of the given length
 */
void bc_add_strn(bc_t *bc, const char *str, int len);

/**
 * @ingroup scan
 *
 * strores a string in the segment
 *
 * @param bc the bc structure
 * @param str the raw-string. the string must starts with \". the string must also contains the ending \".
 * @return a pointer of src to the next character after the second \"
 */
char *bc_store_string(bc_t *bc, char *src);

/**
 * @ingroup scan
 *
 * adds an EOC (end-of-command) mark to segment
 *
 * @param bc the bc structure
 */
void bc_eoc(bc_t *bc);

/**
 * @ingroup scan
 *
 * pops any EOC mark at the current position
 *
 * @param bc the bc segment
 * @return whether kwTYPE_EOC was popped
 */
int bc_pop_eoc(bc_t *bc);

/**
 * @ingroup scan
 *
 * joins two bc segments
 *
 * @param dest the destination
 * @param src the code to be appended to dest
 */
void bc_append(bc_t *dest, bc_t *src);

/**
 * @ingroup scan
 *
 * joins two bc segments
 *
 * @param dest the destination
 * @param src the code to be appended to dest
 * @param n the size of the src to be copied
 */
void bc_add_n(bc_t *dest, byte *src, uint32_t n);

/**
 * @ingroup scan
 *   add a code
 */
#define bc_add_code(d,i)    bc_add1((d),(i))

/**
 * @ingroup scan
 *
 * add a buildin function
 *
 * @param dest the bc segment
 * @param idx the index of the function
 */
void bc_add_fcode(bc_t *dest, bid_t idx);

/**
 * @ingroup scan
 *
 * add a buildin procedure
 *
 * @param dest the bc segment
 * @param idx the index of the procedure
 */
void bc_add_pcode(bc_t *dest, bid_t idx);

/**
 * @ingroup scan
 *
 * add an external function
 *
 * @param dest the bc segment
 * @param lib the index of the library
 * @param idx the index of the function
 */
void bc_add_extfcode(bc_t *dest, uint32_t lib, uint32_t idx);

/**
 * @ingroup scan
 *
 * add an external procedure
 *
 * @param dest the bc segment
 * @param lib the index of the library
 * @param idx the index of the procedure
 */
void bc_add_extpcode(bc_t *dest, uint32_t lib, uint32_t idx);

/**
 * @ingroup scan
 *
 * add an address
 *
 * @param bc the bc segment
 * @param idx the address
 */
void bc_add_addr(bc_t *bc, bcip_t idx);

/**
 * @ingroup scan
 *
 * add a control-code (if, repeat, while)
 *
 * @param bc the bc segment
 * @param code the control's index
 * @param true_ip the jump-address when on true
 * @param false_ip the jump-address when on false
 */
void bc_add_ctrl(bc_t *bc, code_t code, bcip_t true_ip, bcip_t false_ip);

/**
 * @ingroup scan
 *
 * add a real number
 *
 * @param bc the bc segment
 * @param v the number
 */
void bc_add_creal(bc_t *bc, var_num_t v);

/**
 * @ingroup scan
 *
 * add an integer number
 *
 * @param bc the bc segment
 * @param v the number
 */
void bc_add_cint(bc_t *bc, var_int_t v);

#endif
