// This file is part of SmallBASIC
//
// Evaluate variables from bytecode
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010-2014 Chris Warren-Smith. [http://tinyurl.com/ja2ss]

#include "common/sys.h"
#include "common/kw.h"
#include "common/pproc.h"
#include "common/sberr.h"
#include "common/smbas.h"
#include "common/var.h"
#include "common/var_eval.h"

/**
 * Convertion multi-dim index to one-dim index
 */
bcip_t getarrayidx(var_t *array, var_t **var_map_val) {
  bcip_t idx = 0;
  bcip_t lev = 0;
  bcip_t m = 0;
  var_t var;
  bcip_t i;

  do {
    v_init(&var);
    eval(&var);
    IF_ERR_RETURN_0;

    if (var.type == V_STR || array->type == V_MAP) {
      // array element is a string or element is addressing a map
      map_get_value(array, &var, var_map_val);

      if (code_peek() == kwTYPE_LEVEL_END) {
        code_skipnext();
      } else {
        err_missing_sep();
      }
      v_free(&var);
      return 0;
    } else {
      bcip_t idim = v_getint(&var);
      v_free(&var);
      IF_ERR_RETURN_0;

      idim = idim - array->v.a.lbound[lev];

      m = idim;
      for (i = lev + 1; i < array->v.a.maxdim; i++) {
        m = m * (ABS(array->v.a.ubound[i] - array->v.a.lbound[i]) + 1);
      }
      idx += m;

      // skip separator
      byte code = code_peek();
      if (code == kwTYPE_SEP) {
        code_skipnext();
        if (code_getnext() != ',') {
          err_missing_comma();
        }
      }
      // next
      lev++;
    }
  } while (code_peek() != kwTYPE_LEVEL_END);

  if (!prog_error) {
    if ((int) array->v.a.maxdim != lev) {
      err_missing_sep();
    }
  }
  return idx;
}

/**
 * Used by code_getvarptr() to retrieve an element ptr of an array
 */
var_t *code_getvarptr_arridx(var_t *basevar_p) {
  bcip_t array_index;
  var_t *var_p = NULL;

  if (code_peek() != kwTYPE_LEVEL_BEGIN) {
    err_arrmis_lp();
  } else {
    code_skipnext(); // '('
    array_index = getarrayidx(basevar_p, &var_p);

    if (var_p != NULL) {
      // map value
      return var_p;
    }

    if (!prog_error) {
      if ((int) array_index < basevar_p->v.a.size && (int) array_index >= 0) {
        var_p = (var_t *)(basevar_p->v.a.ptr + (array_index * sizeof(var_t)));

        if (code_peek() == kwTYPE_LEVEL_END) {
          code_skipnext();      // ')', ')' level
          if (code_peek() == kwTYPE_LEVEL_BEGIN) {
            // there is a second array inside
            if (var_p->type != V_ARRAY) {
              err_varisnotarray();
            } else {
              return code_getvarptr_arridx(var_p);
            }
          }
        } else {
          err_arrmis_rp();
        }
      } else {
        err_arridx(array_index, basevar_p->v.a.size);
      }
    }
  }
  return var_p;
}

/**
 * resolve a composite variable reference, eg: ar.ch(0).foo
 */
var_t *code_resolve_varptr(var_t *var_p, int until_parens) {
  int deref = 1;
  var_p = eval_ref_var(var_p);
  while (deref && var_p != NULL) {
    switch (code_peek()) {
    case kwTYPE_LEVEL_BEGIN:
      if (until_parens) {
        deref = 0;
      } else {
        var_p = code_getvarptr_arridx(var_p);
      }
      break;
    case kwTYPE_UDS_EL:
      var_p = map_resolve_fields(var_p);
      break;
    default:
      deref = 0;
    }
    var_p = eval_ref_var(var_p);
  }
  return var_p;
}

/**
 * Used by code_isvar() to retrieve an element ptr of an array
 */
var_t *code_isvar_arridx(var_t *basevar_p) {
  bcip_t array_index;
  var_t *var_p = NULL;

  if (code_peek() != kwTYPE_LEVEL_BEGIN) {
    return NULL;
  } else {
    code_skipnext(); // '('
    array_index = getarrayidx(basevar_p, &var_p);

    if (var_p != NULL) {
      // map value
      return var_p;
    }

    if (!prog_error) {
      if ((int) array_index < basevar_p->v.a.size) {
        var_p = (var_t *)(basevar_p->v.a.ptr + (array_index * sizeof(var_t)));

        if (code_peek() == kwTYPE_LEVEL_END) {
          code_skipnext();      // ')', ')' level
          if (code_peek() == kwTYPE_LEVEL_BEGIN) {
            // there is a second array inside
            if (var_p->type != V_ARRAY) {
              return NULL;
            } else {
              return code_isvar_arridx(var_p);
            }
          }
        } else {
          return NULL;
        }
      } else {
        return NULL;
      }
    }
  }

  return var_p;
}

/**
 * returns true if the next code is a variable. if the following code is an 
 * expression (no matter if the first item is a variable), returns false
 */
int code_isvar() {
  var_t *basevar_p;
  var_t *var_p = NULL;

  // store IP
  bcip_t cur_ip = prog_ip;

  if (code_peek() == kwTYPE_VAR) {
    code_skipnext();
    var_p = basevar_p = tvar[code_getaddr()];
    switch (basevar_p->type) {
    case V_MAP:
    case V_ARRAY:
      // variable is an array or map
      var_p = code_resolve_varptr(var_p, 0);
      break;
    default:
      if (code_peek() == kwTYPE_LEVEL_BEGIN) {
        var_p = NULL;
      }
    }
  }

  if (var_p) {
    if (kw_check_evexit(code_peek()) || code_peek() == kwTYPE_LEVEL_END) {
      // restore IP
      prog_ip = cur_ip;
      return 1;
    }
  }

  // restore IP
  prog_ip = cur_ip;
  return 0;
}

var_t *eval_ref_var(var_t *var_p) {
  var_t *result = var_p;
  while (result != NULL && result->type == V_REF) {
    if (result->v.ref == var_p) {
      // circular referance error
      result = NULL;
      err_ref_circ_var();
      break;
    } else {
      result = result->v.ref;
    }
  }
  return result;
}

var_t *resolve_var_ref(var_t *var_p) {
  switch (code_peek()) {
  case kwTYPE_LEVEL_BEGIN:
    var_p = resolve_var_ref(code_getvarptr_arridx(var_p));
    break;
  case kwTYPE_UDS_EL:
    var_p = resolve_var_ref(map_resolve_fields(var_p));
    break;
  }
  return var_p;
}

void v_eval_ref(var_t *v_left) {
  var_t *v_right = NULL;
  // can only reference regular non-dynamic variable
  if (code_peek() == kwTYPE_VAR) {
    code_skipnext();
    v_right = tvar[code_getaddr()];
    switch (v_right->type) {
    case V_ARRAY:
    case V_MAP:
      switch (code_peek()) {
      case kwTYPE_LEVEL_BEGIN:
      case kwTYPE_UDS_EL:
        // base variable or element reference only supported
        v_right = resolve_var_ref(v_right);
        if (v_right != NULL && v_right->type != V_REF) {
          v_right = NULL;
        }
        break;
      }
      break;
    default:
      break;
    }
    if (v_right != NULL) {
      int i;
      int level = 0;
      int left_level = -1;
      int right_level = -1;
      for (i = 0; i < prog_stack_count; i++) {
        stknode_t *cur_node = &prog_stack[i];
        var_t *v_stack = NULL;
        switch (cur_node->type) {
        case kwTYPE_CRVAR:
        case kwBYREF:
          v_stack = tvar[cur_node->x.vdvar.vid];
          break;
        case kwTYPE_VAR:
          v_stack = cur_node->x.param.res;
          break;
        case kwPROC:
        case kwFUNC:
        case kwFOR:
        case kwSELECT:
          level++;
          break;
        }
        if (v_stack != NULL) {
          int next_level = (cur_node->type == kwBYREF) ? level -1 : level;
          if (left_level == -1 && v_left == v_stack) {
            left_level = next_level;
          }
          if (right_level == -1 && v_right == v_stack) {
            right_level = next_level;
          }
        }
      }
      if (left_level < right_level && right_level > 0) {
        // cannot assign left to higher scope right variable
        v_right = NULL;
      }
    }
  }
  if (v_right == NULL) {
    err_ref_var();
  } else {
    v_free(v_left);
    v_left->type = V_REF;
    v_left->v.ref = v_right;
  }
}

/*
 * evaluate the pcode string
 */
void v_eval_str(var_p_t v) {
  int len = code_getstrlen();
  v->type = V_STR;
  v->v.p.size = len;
  v->v.p.ptr = malloc(len + 1);
  memcpy(v->v.p.ptr, &prog_source[prog_ip], len);
  *((char *)(v->v.p.ptr + len)) = '\0';
  prog_ip += len;
}

