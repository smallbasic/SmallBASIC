// This file is part of SmallBASIC
//
// inlined hotspots
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2014 Chris Warren-Smith

#include "common/var_map.h"
#include "common/var_eval.h"

void err_evsyntax(void);
void err_varisarray(void);
void err_varisnotarray(void);
void err_notavar(void);

/**
 * @ingroup var
 *
 * returns the next integer and moves the IP 4 bytes forward.
 *
 * R(long int) <- Code[IP]; IP+=j
 */
static inline uint32_t code_getnext32(void) {
  uint32_t v;
  memcpy(&v, prog_source + prog_ip, 4);
  prog_ip += 4;
  return v;
}

/**
 * @ingroup var
 *
 * returns the next 64bit and moves the instruction pointer to the next instruction
 *
 * R(double)   <- Code[IP]; IP+=j
 */
static inline var_num_t code_getreal() {
  var_num_t v;
  memcpy(&v, prog_source + prog_ip, sizeof(var_num_t));
  prog_ip += sizeof(var_num_t);
  return v;
}

/**
 * @ingroup var
 *
 * returns the next var_int_t and moves the instruction pointer to the next instruction
 */
static inline var_int_t code_getint() {
  var_int_t v;
  memcpy(&v, prog_source + prog_ip, sizeof(var_int_t));
  prog_ip += sizeof(var_int_t);
  return v;
}

/**
 * @ingroup var
 *
 * returns the floating-point value of a var.
 * if v is string it will converted to double.
 *
 * @param v the variable
 * @return the numeric value of a variable
 */
static inline var_num_t v_getval(var_t *v) {
  switch (v ? v->type : -1) {
  case V_INT:
    return v->v.i;
  case V_NUM:
    return v->v.n;
  case V_STR:
    return numexpr_sb_strtof((char *) v->v.p.ptr);
  case V_PTR:
    return v->v.ap.p;
  case V_MAP:
    return map_to_int(v);
  default:
    if (v == NULL) {
      err_evsyntax();
    } else {
      err_varisarray();
    }
  }
  return 0;
}

/**
 * @ingroup var
 *
 * returns the integer value of a var.
 * if v is string it will converted to integer.
 *
 * @param v the variable
 * @return the integer value of a variable
 */
static inline var_int_t v_igetval(var_t *v) {
  switch (v ? v->type : -1) {
  case V_INT:
    return v->v.i;
  case V_NUM:
    return v->v.n;
  case V_STR:
    return numexpr_strtol((char *) v->v.p.ptr);
  case V_PTR:
    return v->v.ap.p;
  case V_MAP:
    return map_to_int(v);
  default:
    if (v == NULL) {
      err_evsyntax();
    } else {
      err_varisarray();
    }
  }
  return 0;
}

/**
 * @ingroup exec
 *
 * variant of code_getvarptr() derefence until left parenthesis found
 *
 * R(var_t*) <- Code[IP]; IP += 2;
 *
 * @return the var_t*
 */
static inline var_t *code_getvarptr_parens(int until_parens) {
  var_t *var_p = NULL;

  if (code_peek() == kwTYPE_VAR) {
    code_skipnext();
    var_p = tvar[code_getaddr()];
    if (code_peek() == kwTYPE_UDS_EL) {
      var_p = code_resolve_map(var_p, until_parens);
    } else {
      switch (var_p->type) {
      case V_MAP:
        var_p = code_resolve_map(var_p, until_parens);
        break;
      case V_ARRAY:
        var_p = code_resolve_varptr(var_p, until_parens);
        break;
      default:
        if (!until_parens && code_peek() == kwTYPE_LEVEL_BEGIN) {
          err_varisnotarray();
        }
      }
    }
  }

  if (var_p == NULL && !prog_error) {
    err_notavar();
    return tvar[0];
  }

  return var_p;
}

/**
 * @ingroup var
 *
 * initialize a variable
 *
 * @param v the variable
 */
static inline void v_init(var_t *v) {
  v->type = V_INT;
  v->const_flag = 0;
  v->v.i = 0;
}

/**
 * @ingroup var
 *
 * frees the var or releases it back into the pool
 */
static inline void v_detach(var_t *v) {
  if (v->pooled) {
    v_pool_free(v);
  } else {
    free(v);
  }
}

/**
 * @ingroup var
 *
 * free variable's data.
 *
 * @warning it frees only the data, not the variable
 *
 * @param v the variable
 */
static inline void v_free(var_t *v) {
  switch (v->type) {
  case V_STR:
    if (v->v.p.owner) {
      free(v->v.p.ptr);
    }
    break;
  case V_ARRAY:
    v_array_free(v);
    break;
  case V_MAP:
    map_free(v);
    break;
  }
  v_init(v);
}
