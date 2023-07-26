// This file is part of SmallBASIC
//
// SmallBASIC variant type
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2018 Chris Warren-Smith

#if !defined(_INC_VAR_T_H)
#define _INC_VAR_T_H

#include <stdint.h>

// https://en.wikipedia.org/wiki/Dependency_inversion_principle

typedef double var_num_t;
typedef long long int var_int_t;

#define MAXDIM 6
#define OS_INTSZ  sizeof(var_int_t)
#define OS_REALSZ sizeof(var_num_t)

#ifdef V_INT
#undef V_INT
#endif

#ifdef V_ARRAY
#undef V_ARRAY
#endif

/*
 * Variable - types
 */
#define V_INT       0 /**< variable type, 64bit integer                @ingroup var */
#define V_NUM       1 /**< variable type, 64bit float (same as V_NUM)  @ingroup var */
#define V_STR       2 /**< variable type, string                       @ingroup var */
#define V_ARRAY     3 /**< variable type, array of variables           @ingroup var */
#define V_PTR       4 /**< variable type, pointer to UDF or label      @ingroup var */
#define V_MAP       5 /**< variable type, associative array            @ingroup var */
#define V_REF       6 /**< variable type, reference another var        @ingroup var */
#define V_FUNC      7 /**< variable type, object method                @ingroup var */
#define V_NIL       8 /**< variable type, null value                   @ingroup var */

#if defined(__cplusplus)
extern "C" {
#endif

struct var_s;
typedef void (*method) (struct var_s *self, struct var_s *retval);

typedef struct var_s {
  union {
    // numeric
    var_num_t n;

    // integer
    var_int_t i;

    // pointer to sub/func variable
    struct {
      // address pointer
      uint32_t p;
      // return var ID
      uint32_t v;
    } ap;

    // associative array/map
    struct {
      // pointer the map structure
      void *map;

      uint32_t count;
      uint32_t size;
      uint32_t id;
      uint32_t lib_id;
      uint32_t cls_id;
    } m;

    // reference variable
    struct var_s *ref;

    // object method
    struct {
      method cb;
      uint32_t id;
    } fn;

    // generic ptr (string)
    struct {
      char *ptr;
      uint32_t length;
      uint8_t owner;
    } p;

    // array
    struct {
      struct var_s *data;
      // the number of elements
      uint32_t size;
      // the number of available element slots
      uint32_t capacity;
      // upper and lower bounds
      int32_t ubound[MAXDIM];
      int32_t lbound[MAXDIM];
    } a;

    // next item in the free-list
    struct var_s *pool_next;
  } v;

  // number of dimensions
  uint8_t maxdim;

  // variables type
  uint8_t type;

  // non-zero if constant
  uint8_t const_flag;

  // whether help in pooled memory
  uint8_t pooled;
} var_t;

typedef var_t *var_p_t;

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
 * creates a new variable array
 *
 * @return a newly created var_t array of the given size
 */
void v_new_array(var_t *var, unsigned size);

/**
 * @ingroup var
 *
 * frees memory associated with the given array
 */
void v_array_free(var_t *var);

/**
 * @ingroup var
 *
 * initialise the variable as a string of the given length
 */
void v_init_str(var_t *var, int length);

/**
 * @ingroup var
 *
 * takes ownership of the given allocated string
 */
void v_move_str(var_t *var, char *str);

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
 * assigning: dest = src
 *
 * @param dest the destination-var
 * @param src the source-var
 */
void v_move(var_t *dest, const var_t *src);

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
void v_resize_array(var_t *v, uint32_t size);

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
 * converts the variable v to an array of R elements.
 * R can be zero for zero-length arrays
 *
 * @param v the variable
 * @param r the number of the elements
 */
void v_toarray1(var_t *v, uint32_t r);

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
 * makes 'var' an empty string variable
 *
 * @param var is the variable
 */
void v_zerostr(var_t *var);

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
#define v_elem(var, i) &((var)->v.a.data[i])

/**
 * < the number of the elements of the array (x)
 * @ingroup var
 */
#define v_asize(x) ((x)->v.a.size)

/**
 * < the number of array dimensions (x)
 * @ingroup var
 */
#define v_maxdim(x) ((x)->maxdim)

/**
 * < the array lower bound of the given dimension (x)
 * @ingroup var
 */
#define v_lbound(x, i) ((x)->v.a.lbound[i])

/**
 * < the array upper bound of the given dimension (x)
 * @ingroup var
 */
#define v_ubound(x, i) ((x)->v.a.ubound[i])

/**
 * < the array data
 * @ingroup var
 */
#define v_data(x) ((x)->v.a.data)

/**
 * < the array capacity
 * @ingroup var
 */
#define v_capacity(x) ((x)->v.a.capacity)

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
 * returns the length of the var string
 *
 * @param v is the variable
 * @return the string length
 */
int v_strlen(const var_t *v);

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
 * setup a method on the map using the given name
 */
void v_create_func(var_p_t map, const char *name, method cb);

#if defined(__cplusplus)
}
#endif

#endif
