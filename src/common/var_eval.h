// This file is part of SmallBASIC
//
// Evaluate code variables
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010-2014 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#ifndef VAR_EVAL_H
#define VAR_EVAL_H

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup var
 *
 * resolve var pointer
 */
var_t *code_resolve_varptr(var_t* var_p, int until_parens);

/**
 * @ingroup var
 *
 * evaluate variable reference assignment
 */
var_t *eval_ref_var(var_t *var_p);

/**
 * @ingroup var
 *
 * evaluate variable reference assignment
 */
void v_eval_ref(var_t *l_value);

/**
 * @ingroup exec
 *
 * returns true if the next code is a single variable
 *
 * @return non-zero if the following code is a variable
 */
int code_isvar(void);

/**
 * @ingroup var
 *
 * populates the var to string from bytecode
 *
 * @param v is the variable
 */
void v_eval_str(var_p_t v);

#if defined(__cplusplus)
}
#endif

#endif

