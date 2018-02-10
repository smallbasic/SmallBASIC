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
typedef long int var_int_t;

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
#define V_INT       0 /**< variable type, 32bit integer                @ingroup var */
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
typedef void (*method) (struct var_s *self);

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
    } m;

    // reference variable
    struct var_s *ref;

    // object method
    struct {
      method cb;
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
      int8_t  lbound[MAXDIM];
      // number of dimensions
      uint8_t maxdim;
    } a;

    // next item in the free-list
    struct var_s *pool_next;
  } v;

  // variables type
  uint8_t type;

  // non-zero if constant
  uint8_t const_flag;

  // whether help in pooled memory
  uint8_t pooled;
} var_t;

typedef var_t *var_p_t;

// forward declaration of module callable functions
void v_setint(var_t *var, var_int_t integer);
void v_setstr(var_t *var, const char *string);
void v_tostr(var_t *arg);
void v_tomatrix(var_t *v, int r, int c);

#if defined(__cplusplus)
}
#endif

#endif
