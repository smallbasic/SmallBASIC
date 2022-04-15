// This file is part of SmallBASIC
//
// SmallBASIC-executor: expressions
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/smbas.h"
#include "common/pproc.h"
#include "common/str.h"
#include "common/kw.h"
#include "common/blib.h"
#include "common/device.h"
#include "common/plugins.h"
#include "common/var_eval.h"

#define IP           prog_ip
#define CODE(x)      prog_source[(x)]
#define CODE_PEEK()  CODE(IP)
#define V_FREE(v)                               \
  if ((v) && ((v)->type == V_STR ||             \
              (v)->type == V_MAP ||             \
              (v)->type == V_ARRAY)) {          \
    v_free((v));                                \
  }
#define V_FREE2(v)                              \
  if (((v)->type == V_STR ||                    \
       (v)->type == V_MAP ||                    \
       (v)->type == V_ARRAY)) {                 \
    v_free((v));                                \
  }

//
// matrix: convert var_t to double[r][c]
//
var_num_t *mat_toc(var_t *v, int32_t *rows, int32_t *cols) {
  var_num_t *m = NULL;
  *rows = *cols = 0;

  if (!v) {
    // uninitialised variable
    return NULL;
  }

  if (v_maxdim(v) > 2) {
    // too many dimensions
    err_matdim();
    return NULL;
  }
  *rows = ABS(v_lbound(v, 0) - v_ubound(v, 0)) + 1;

  if (v_maxdim(v) == 2) {
    *cols = ABS(v_lbound(v, 1) - v_ubound(v, 1)) + 1;
  } else {
    *cols = *rows;
    *rows = 1;
  }

  m = (var_num_t *)malloc(((*rows) * (*cols)) * sizeof(var_num_t));
  for (int i = 0; i < *rows; i++) {
    for (int j = 0; j < *cols; j++) {
      int pos = i * (*cols) + j;
      var_t *e = v_elem(v, pos);
      m[pos] = v_getval(e);
    }
  }

  return m;
}

//
// matrix: conv. double[nr][nc] to var_t
//
void mat_tov(var_t *v, var_num_t *m, int rows, int cols, int protect_col1) {
  if (cols > 1 || protect_col1) {
    v_tomatrix(v, rows, cols);
  } else {
    v_toarray1(v, rows);
  }
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      int pos = i * cols + j;
      var_t *e = v_elem(v, pos);
      e->type = V_NUM;
      e->v.n = m[pos];
    }
  }
}

//
// matrix: 1op
//
void mat_op1(var_t *l, int op, var_num_t n) {
  int lr, lc;

  var_num_t *m1 = mat_toc(l, &lr, &lc);
  if (m1) {
    var_num_t *m = (var_num_t *)malloc(sizeof(var_num_t) * lr * lc);
    for (int i = 0; i < lr; i++) {
      for (int j = 0; j < lc; j++) {
        int pos = i * lc + j;
        switch (op) {
        case '*':
          m[pos] = m1[pos] * n;
          break;
        case 'A':
          m[pos] = -m1[pos];
          break;
        default:
          m[pos] = 0;
          break;
        }
      }
    }
    if (v_maxdim(l) == 1) {
      mat_tov(l, m, lc, 1, 0);
    } else {
      mat_tov(l, m, lr, lc, 1);
    }
    free(m1);
    free(m);
  }
}

//
// M = -A
//
void mat_antithetos(var_t *v) {
  mat_op1(v, 'A', 0);
}

//
// M = A
//
void mat_mulN(var_t *v, var_num_t N) {
  mat_op1(v, '*', N);
}

//
// matrix - add/sub
//
void mat_op2(var_t *l, var_t *r, int op) {
  int lr, lc, rr, rc;

  var_num_t *m1 = mat_toc(l, &lr, &lc);
  if (m1) {
    var_num_t *m2 = mat_toc(r, &rr, &rc);
    if (m2) {
      var_num_t *m = NULL;
      if (rc != lc || lr != rr) {
        err_matdim();
      } else {
        m = (var_num_t *)malloc(sizeof(var_num_t) * lr * lc);
        for (int i = 0; i < lr; i++) {
          for (int j = 0; j < lc; j++) {
            int pos = i * lc + j;
            if (op == '+') {
              m[pos] = m1[pos] + m2[pos];
            } else {
              m[pos] = m2[pos] - m1[pos];
            }
            // array is reversed because of where to store
          }
        }
      }

      free(m1);
      free(m2);
      if (m) {
        if (v_maxdim(r) == 1) {
          mat_tov(l, m, lc, 1, 0);
        } else {
          mat_tov(l, m, lr, lc, 1);
        }
        free(m);
      }
    } else {
      free(m1);
    }
  }
}

void mat_add(var_t *l, var_t *r) {
  mat_op2(l, r, '+');
}

void mat_sub(var_t *l, var_t *r) {
  mat_op2(l, r, '-');
}

//
// matrix: multiply two 1d arrays
//
void mat_mul_1d(var_t *l, var_t *r) {
  uint32_t size = v_asize(l);
  for (uint32_t i = 0; i < size; i++) {
    var_t *elem = v_elem(r, i);
    var_num_t v1 = v_getval(v_elem(l, i));
    var_num_t v2 = v_getval(elem);
    v_setreal(elem, (v1 * v2));
  }
}

//
// matrix: dot product of two 1d arrays
//
void mat_dot(var_t *l, var_t *r) {
  var_num_t result = 0;
  uint32_t size = v_asize(l);
  for (uint32_t i = 0; i < size; i++) {
    var_num_t v1 = v_getval(v_elem(l, i));
    var_num_t v2 = v_getval(v_elem(r, i));
    result += (v1 * v2);
  }
  v_setreal(r, result);
}

//
// matrix: multiply
//
void mat_mul(var_t *l, var_t *r) {
  int lr, lc, rr, rc;

  var_num_t *m1 = mat_toc(l, &lr, &lc);
  if (m1) {
    var_num_t *m2 = mat_toc(r, &rr, &rc);
    if (m2) {
      var_num_t *m = NULL;
      int mr = 0;
      int mc = 0;
      if (lc != rr) {
        err_matdim();
      } else {
        mr = lr;
        mc = rc;
        m = (var_num_t *)malloc(sizeof(var_num_t) * mr * mc);
        for (int i = 0; i < mr; i++) {
          for (int j = 0; j < mc; j++) {
            int pos = i * mc + j;
            m[pos] = 0.0;
            for (int k = 0; k < lc; k++) {
              m[pos] = m[pos] + (m1[i * lc + k] * m2[k * rc + j]);
            }
          }
        }
      }
      free(m1);
      free(m2);
      if (m) {
        mat_tov(r, m, mr, mc, 1);
        free(m);
      }
    } else {
      free(m1);
    }
  }
}

//
// The LIKE operator
//
int v_wc_match(var_t *vwc, var_t *v) {
  int ri;

  if (prog_error) {
    return 0;
  }
  if (vwc->type != V_STR) {
    err_typemismatch();
    return 0;
  }

  ri = 0;
  if (v->type == V_ARRAY) {
    int i;
    ri = 1;
    for (i = 0; i < v_asize(v); i++) {
      var_t *elem_p = v_elem(v, i);
      if (v_wc_match(vwc, elem_p) == 0) {
        ri = 0;
        break;
      }
    }
  } else if (v->type == V_STR) {
    ri = wc_match(vwc->v.p.ptr, v->v.p.ptr);
  } else if (v->type == V_NUM || v->type == V_INT) {
    var_t *vt = v_clone(v);
    v_tostr(vt);
    if (!prog_error) {
      ri = wc_match(vwc->v.p.ptr, vt->v.p.ptr);
    }
    V_FREE(vt);
    v_detach(vt);
  }
  return ri;
}

static inline void oper_add(var_t *r, var_t *left) {
  byte op = CODE(IP);
  IP++;
  if (r->type == V_INT && v_is_type(left, V_INT)) {
    if (op == '+') {
      r->v.i += left->v.i;
    } else {
      r->v.i = left->v.i - r->v.i;
    }
  } else if (r->type == V_NUM && v_is_type(left, V_NUM)) {
    r->type = V_NUM;
    if (op == '+') {
      r->v.n += left->v.n;
    } else {
      r->v.n = left->v.n - r->v.n;
    }
  } else if (r->type == V_INT && v_is_type(left, V_NUM)) {
    r->type = V_NUM;
    if (op == '+') {
      r->v.n = r->v.i + left->v.n;
    } else {
      r->v.n = left->v.n - r->v.i;
    }
  } else if (r->type == V_NUM && v_is_type(left, V_INT)) {
    if (op == '+') {
      r->v.n += left->v.i;
    } else {
      r->v.n = left->v.i - r->v.n;
    }
  } else {
    if (r->type == V_ARRAY || v_is_type(left, V_ARRAY)) {
      // arrays
      if (r->type == V_ARRAY && v_is_type(left, V_ARRAY)) {
        if (op == '+') {
          mat_add(r, left);
        } else {
          mat_sub(r, left);
        }
      } else {
        err_matop();
      }
    } else {
      // not array
      var_t vtmp;
      v_init(&vtmp);
      switch (op) {
      case '+':
        v_add(&vtmp, left, r);
        break;
      case '-':
        if (vtmp.type == V_NUM) {
          vtmp.type = V_NUM;
          vtmp.v.n = v_getval(left) - v_getval(r);
        } else {
          vtmp.type = V_INT;
          vtmp.v.i = v_igetval(left) - v_igetval(r);
        }
        break;
      }
      v_move(r, &vtmp);
    }
    V_FREE(left);
  }
}

static inline void oper_mul(var_t *r, var_t *left) {
  var_num_t lf;
  var_num_t rf;
  var_int_t li;
  var_int_t ri;

  byte op = CODE(IP);
  IP++;

  if (r->type == V_ARRAY || v_is_type(left, V_ARRAY)) {
    // arrays
    if (r->type == V_ARRAY && v_is_type(left, V_ARRAY)) {
      if (v_maxdim(left) == v_maxdim(r) && v_maxdim(r) == 1) {
        if (op == '*') {
          mat_mul_1d(left, r);
        } else if (op == '%') {
          mat_dot(left, r);
        } else {
          err_matop();
        }
      } else if (op == '*') {
        mat_mul(left, r);
      } else {
        err_matop();
      }
    } else {
      if (r->type == V_ARRAY) {
        if (op == '*') {
          mat_mulN(r, v_getval(left));
        } else {
          err_matop();
        }
      } else {
        rf = v_getval(r);
        v_set(r, left);
        if (op == '*') {
          mat_mulN(r, rf);
        } else {
          err_matop();
        }
      }
    }

    V_FREE(left);
  } else {
    // not array
    lf = v_getval(left);
    V_FREE(left);
    rf = v_getval(r);
    V_FREE(r);

    // double always
    r->type = V_NUM;
    switch (op) {
    case '*':
      r->v.n = lf * rf;
      break;
    case '/':
      if (ABS(rf) == 0) {
        err_division_by_zero();
      } else {
        r->v.n = lf / rf;
      }
      break;
    case '\\':
      li = lf;
      ri = rf;
      if (ri == 0) {
        err_division_by_zero();
      } else {
        r->v.i = li / ri;
      }
      r->type = V_INT;
      break;
    case '%':
    case OPLOG_MOD:
      if ((var_int_t) rf == 0) {
        err_division_by_zero();
      } else {
        // r->v.n = fmod(lf, rf);
        ri = rf;
        li = (lf < 0.0) ? -floor(-lf) : floor(lf);
        r->v.i = li - ri * (li / ri);
        r->type = V_INT;
      }
      break;
    case OPLOG_MDL:
      if (rf == 0) {
        err_division_by_zero();
      } else {
        r->v.n = fmod(lf, rf) + rf * (SGN(lf) != SGN(rf));
        r->type = V_NUM;
      }
      break;
    };
  }
}

static inline void oper_unary(var_t *r) {
  var_int_t ri;
  var_num_t rf;

  byte op = CODE(IP);
  IP++;

  switch (op) {
  case '-':
    if (r->type == V_INT) {
      r->v.i = -r->v.i;
    } else if (r->type == V_NUM) {
      r->v.n = -r->v.n;
    } else if (r->type == V_ARRAY) {
      mat_antithetos(r);
    } else {
      rf = v_getval(r);
      V_FREE(r);
      r->v.n = -rf;
      r->type = V_NUM;
    }
    break;
  case '+':
    break;
  case OPLOG_INV:
    // the result of ~ is always integer
    ri = v_igetval(r);
    V_FREE(r);
    r->type = V_INT;
    r->v.i = ~ri;
    break;
  case OPLOG_NOT:
    // the result of ! is always integer
    ri = v_igetval(r);
    V_FREE(r);
    r->type = V_INT;
    r->v.i = !ri;
    break;
  }
}

static inline void oper_log(var_t *r, var_t *left) {
  var_int_t li;
  var_int_t ri;
  var_int_t a, b;
  int i, set;

  // logical/bit
  byte op = CODE(IP);
  IP++;

  if (op != OPLOG_IN) {
    li = v_igetval(left);
    ri = v_igetval(r);
  } else {
    li = 0;
    ri = 0;
  }

  switch (op) {
  case OPLOG_AND:
    ri = (li && ri) ? 1 : 0;
    break;
  case OPLOG_OR:
    ri = (li || ri) ? 1 : 0;
    break;
  case OPLOG_EQV:
    a = li;
    b = ri;
    ri = 0;
    set = 0;
    for (i = (sizeof(var_int_t) * 8) - 1; i >= 0; i--) {
      int ba = ((a >> i) & 1);
      int bb = ((b >> i) & 1);
      if (ba || bb) {
        set = 1;
      }
      if (set && ba == bb) {
        ri |= (((var_int_t)1) << i);
      }
    }
    break;
  case OPLOG_IMP:
    a = li;
    b = ri;
    ri = 0;
    set = 0;
    for (i = (sizeof(var_int_t) * 8) - 1; i >= 0; i--) {
      int ba = ((a >> i) & 1);
      int bb = ((b >> i) & 1);
      if (ba || bb) {
        set = 1;
      }
      if (set && (!ba || bb)) {
        ri |= (((var_int_t)1) << i);
      }
    }
    break;
  case OPLOG_NAND:
    ri = ~(li & ri);
    break;
  case OPLOG_NOR:
    ri = ~(li | ri);
    break;
  case OPLOG_XNOR:
    ri = ~(li ^ ri);
    break;
  case OPLOG_BOR:
    ri = li | ri;
    break;
  case OPLOG_BAND:
    ri = li & ri;
    break;
  case OPLOG_XOR:
    ri = li ^ ri;
    break;
  case OPLOG_LSHIFT:
    ri = li << ri;
    break;
  case OPLOG_RSHIFT:
    ri = li >> ri;
    break;
  }

  // cleanup
  V_FREE(left);
  V_FREE(r);

  r->type = V_INT;
  r->v.i = ri;
}

static inline void oper_cmp(var_t *r, var_t *left) {
  var_int_t ri;

  // compare
  byte op = CODE(IP);
  IP++;

  switch (op) {
  case OPLOG_EQ:
    ri = (v_compare(left, r) == 0);
    break;
  case OPLOG_GT:
    ri = (v_compare(left, r) > 0);
    break;
  case OPLOG_GE:
    ri = (v_compare(left, r) >= 0);
    break;
  case OPLOG_LT:
    ri = (v_compare(left, r) < 0);
    break;
  case OPLOG_LE:
    ri = (v_compare(left, r) <= 0);
    break;
  case OPLOG_NE:
    ri = (v_compare(left, r) != 0);
    break;
  case OPLOG_IN:
    ri = 0;
    if (r->type == V_ARRAY) {
      int i;
      for (i = 0; i < v_asize(r); i++) {
        var_t *elem_p = v_elem(r, i);
        if (v_compare(left, elem_p) == 0) {
          ri = i + 1;
          break;
        }
      }
    } else if (r->type == V_STR) {
      if (v_is_type(left, V_STR)) {
        if (left->v.p.ptr[0] != '\0') {
          ri = (strstr(r->v.p.ptr, left->v.p.ptr) != NULL);
        } else {
          ri = 0;
        }
      } else if (v_is_type(left, V_NUM) || v_is_type(left, V_INT)) {
        var_t *v = v_clone(left);
        v_tostr(v);
        ri = (strstr(r->v.p.ptr, v->v.p.ptr) != NULL);
        V_FREE(v);
        v_detach(v);
      }
    } else if (r->type == V_NUM || r->type == V_INT) {
      ri = (v_compare(left, r) == 0);
    }
    break;
  case OPLOG_LIKE:
    ri = v_wc_match(r, left);
    break;
  default:
    ri = 0;
    break;
  }

  // cleanup
  V_FREE(left);
  V_FREE(r);

  r->type = V_INT;
  r->v.i = ri;
}

static inline void oper_powr(var_t *r, var_t *left) {
  var_num_t rf;

  // pow
  IP++;

  rf = pow(v_getval(left), v_getval(r));
  V_FREE(r);
  r->type = V_NUM;
  r->v.n = rf;

  // cleanup
  V_FREE(left);
}

static inline void eval_shortc(var_t *r) {
  // short-circuit evaluation
  // see cev_log() in ceval.c for layout details
  var_int_t li;
  var_int_t ri;

  // skip code kwTYPE_LOGOPR
  IP++;

  // read operator
  byte op = CODE(IP);
  IP++;

  // read shortcut jump offset
  bcip_t addr = code_getaddr();

  // read left side result
  li = v_igetval(&eval_stk[eval_sp - 1]);
  ri = -1;

  switch (op) {
  case OPLOG_AND:
    if (!li) {
      // False AND blah => result is false
      ri = 0;
    }
    break;
  case OPLOG_OR:
    if (li) {
      // True OR blah => result is true
      ri = 1;
    }
    break;
  }

  if (ri != -1) {
    // set the result into v - if there are more expressions
    // this will be kwTYPE_EVPUSH'd onto the stack
    // and kwTYPE_EVAL_SC will be called again to test the
    // subsequent boolean operator
    V_FREE(r);
    r->type = V_INT;
    r->v.i = ri;
    // jump to the shortcut offset
    IP += (addr - ADDRSZ);
  }
}

static inline void eval_call_udf(var_t *r) {
  bc_loop(1);
  if (!prog_error) {
    stknode_t udf_rv;
    code_pop(&udf_rv, kwTYPE_RET);
    if (udf_rv.type != kwTYPE_RET) {
      err_stackmess();
    } else {
      v_move(r, udf_rv.x.vdvar.vptr);
      // no free after v_move
      v_detach(udf_rv.x.vdvar.vptr);
    }
  }
}

static inline void eval_var_ptr(var_t *r, var_t *var_p) {
  r->type = V_PTR;
  r->v.ap.p = var_p->v.ap.p;
  r->v.ap.v = var_p->v.ap.v;
}

static inline void eval_var(var_t *r, var_t *var_p) {
  if (prog_error) {
    return;
  }

  switch (var_p->type) {
  case V_INT:
    r->type = V_INT;
    r->v.i = var_p->v.i;
    break;
  case V_NUM:
    r->type = V_NUM;
    r->v.n = var_p->v.n;
    break;
  case V_STR:
    v_set(r, var_p);
    break;
  case V_ARRAY:
    v_set(r, var_p);
    break;
  case V_PTR:
    eval_var_ptr(r, var_p);
    break;
  case V_MAP:
    v_set(r, var_p);
    break;
  case V_FUNC:
    var_p->v.fn.cb(var_p, r);
    break;
  case V_NIL:
    r->type = V_NIL;
    r->const_flag = 1;
    break;
  }
}

static inline void eval_push(var_t *r) {
  bcip_t len;

  switch (r->type) {
  case V_INT:
    eval_stk[eval_sp].type = V_INT;
    eval_stk[eval_sp].v.i = r->v.i;
    break;
  case V_NUM:
    eval_stk[eval_sp].type = V_NUM;
    eval_stk[eval_sp].v.n = r->v.n;
    break;
  case V_STR:
    len = r->v.p.length;
    eval_stk[eval_sp].type = V_STR;
    eval_stk[eval_sp].v.p.ptr = malloc(len + 1);
    eval_stk[eval_sp].v.p.owner = 1;
    strcpy(eval_stk[eval_sp].v.p.ptr, r->v.p.ptr);
    eval_stk[eval_sp].v.p.length = len;
    break;
  default:
    v_set(&eval_stk[eval_sp], r);
  }

  // expression-stack resize
  eval_sp++;
  if (eval_sp == eval_size) {
    eval_size += SB_EVAL_STACK_SIZE;
    eval_stk = realloc(eval_stk, sizeof(var_t) * eval_size);
    int i;
    for (i = eval_sp; i < eval_size; i++) {
      v_init(&eval_stk[i]);
    }
  }
}

static inline void eval_extf(var_t *r) {
  bcip_t lib;
  bcip_t idx;

  lib = code_getaddr();
  idx = code_getaddr();
  V_FREE(r);
  if (lib & UID_UNIT_BIT) {
    unit_exec(lib & (~UID_UNIT_BIT), idx, r);
  } else {
    plugin_funcexec(lib, prog_symtable[idx].exp_idx, r);
  }
}

static inline void eval_ptr(var_t *r) {
  V_FREE(r);
  r->type = V_PTR;
  r->const_flag = 1;
  r->v.ap.p = code_getaddr();
  r->v.ap.v = code_getaddr();
}

static inline void eval_callf_str1(long fcode, var_t *r) {
  var_t vtmp;
  // str FUNC(any)
  if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
    err_missing_lp();
  } else {
    IP++;
    v_init(&vtmp);
    eval(&vtmp);
    if (!prog_error) {
      if (CODE_PEEK() != kwTYPE_LEVEL_END) {
        err_missing_rp();
      } else {
        IP++;
        r->type = V_STR;
        r->v.p.ptr = NULL;
        r->v.p.owner = 1;
        cmd_str1(fcode, &vtmp, r);
        v_free(&vtmp);
      }
    }
  }
}

static inline void eval_callf_strn(long fcode, var_t *r) {
  // str FUNC(...)
  if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
    err_missing_lp();
  } else {
    r->type = V_STR;
    r->v.p.owner = 1;
    r->v.p.ptr = NULL;
    IP++;                 // '('
    cmd_strN(fcode, r);
    if (!prog_error) {
      if (CODE_PEEK() == kwTYPE_SEP) {
        IP++;             // ','
      } else if (CODE_PEEK() == kwTYPE_LEVEL_END) {
        IP++;             // ')'
      } else {
        err_missing_rp();
      }
    }
  }
}

static inline void eval_callf_int(long fcode, var_t *r) {
  // int FUNC(...)
  if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
    err_missing_lp();
  } else {
    r->type = V_INT;
    IP++;                 // '('
    cmd_intN(fcode, r);
    if (!prog_error) {
      if (CODE_PEEK() == kwTYPE_SEP) {
        IP++;             // ','
      } else if (CODE_PEEK() == kwTYPE_LEVEL_END) {
        IP++;             // ')'
      } else {
        err_missing_sep();
      }
    }
  }
}

static inline void eval_callf_num(long fcode, var_t *r) {
  // num FUNC(STR)
  var_t vtmp;
  if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
    err_missing_lp();
  } else {
    IP++;
    v_init(&vtmp);
    eval(&vtmp);
    if (!prog_error) {
      if (CODE_PEEK() != kwTYPE_LEVEL_END) {
        err_missing_rp();
      } else {
        IP++;
        cmd_ns1(fcode, &vtmp, r);
        V_FREE2(&vtmp);
      }
    }
  }
}

static inline void eval_callf_numN(long fcode, var_t *r) {
  // fp FUNC(...)
  if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
    err_missing_lp();
  } else {
    r->type = V_NUM;
    IP++;                 // '('
    cmd_numN(fcode, r);
    if (!prog_error) {
      if (CODE_PEEK() == kwTYPE_SEP) {
        IP++;             // ','
      } else if (CODE_PEEK() == kwTYPE_LEVEL_END) {
        IP++;             // ')' level
      } else {
        err_missing_sep();
      }
    }
  }
}

static inline void eval_callf_imathI1(long fcode, var_t *r) {
  // int FUNC(fp)
  var_t vtmp;
  if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
    err_missing_lp();
  } else {
    IP++;
    v_init(&vtmp);
    eval(&vtmp);
    if (!prog_error) {
      if (CODE_PEEK() != kwTYPE_LEVEL_END) {
        err_missing_rp();
      } else {
        IP++;
        r->type = V_INT;
        r->v.i = cmd_imath1(fcode, &vtmp);
      }
    }
  }
}

static inline void eval_callf_imathI2(long fcode, var_t *r) {
  // int FUNC(void)
  if (CODE_PEEK() == kwTYPE_LEVEL_BEGIN) {
    IP++;
    if (CODE_PEEK() != kwTYPE_LEVEL_END) {
      err_noargs();
    } else {
      IP++;
    }
  }
  if (!prog_error) {
    r->type = V_INT;
    r->v.i = cmd_imath0(fcode);
  }
}

static inline void eval_callf_mathN1(long fcode, var_t *r) {
  var_t vtmp;
  if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
    err_missing_lp();
  } else {
    IP++;
    v_init(&vtmp);
    eval(&vtmp);
    if (!prog_error) {
      if (CODE_PEEK() != kwTYPE_LEVEL_END) {
        err_missing_rp();
      } else {
        IP++;
        r->type = V_NUM;
        r->v.n = cmd_math1(fcode, &vtmp);
        V_FREE2(&vtmp);
      }
    }
  }
}

static inline void eval_callf_mathN2(long fcode, var_t *r) {
  // fp FUNC(void)
  if (CODE_PEEK() == kwTYPE_LEVEL_BEGIN) {
    IP++;
    if (CODE_PEEK() != kwTYPE_LEVEL_END) {
      err_noargs();
    } else {
      IP++;
    }
  }
  if (!prog_error) {
    r->type = V_NUM;
    r->v.n = cmd_math0(fcode);
  }
}

static inline void eval_callf_genfunc(long fcode, var_t *r) {
  // any FUNC(...)
  if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
    err_missing_lp();
  } else {
    IP++;
    cmd_genfunc(fcode, r);
    if (!prog_error && CODE_PEEK() == kwTYPE_LEVEL_END) {
      IP++;
    }
  }
}

static inline void eval_callf_free(var_t *r) {
  r->type = V_INT;
  r->v.i = dev_freefilehandle();
}

static inline void eval_callf(var_t *r) {
  long fcode = code_getaddr();
  V_FREE(r);

  switch (fcode) {
  case kwASC:
  case kwVAL:
  case kwTEXTWIDTH:
  case kwTEXTHEIGHT:
  case kwEXIST:
  case kwISFILE:
  case kwISDIR:
  case kwISLINK:
  case kwACCESSF:
    eval_callf_num(fcode, r);
    break;
  case kwCHR:
  case kwSTR:
  case kwOCT:
  case kwHEX:
  case kwBIN:
  case kwLCASE:
  case kwUCASE:
  case kwLTRIM:
  case kwRTRIM:
  case kwSPACE:
  case kwTAB:
  case kwCAT:
  case kwENVIRONF:
  case kwTRIM:
  case kwBCS:
  case kwCBS:
  case kwTIMESTAMP:
    eval_callf_str1(fcode, r);
    break;
  case kwTRANSLATEF:
  case kwSTRING:
  case kwSQUEEZE:
  case kwLEFT:
  case kwRIGHT:
  case kwLEFTOF:
  case kwRIGHTOF:
  case kwLEFTOFLAST:
  case kwRIGHTOFLAST:
  case kwMID:
  case kwREPLACE:
  case kwCHOP:
  case kwRUNF:
  case kwENCLOSE:
  case kwDISCLOSE:
    eval_callf_strn(fcode, r);
    break;
  case kwINKEY:
  case kwTIME:
  case kwDATE:
    cmd_str0(fcode, r);
    break;
  case kwINSTR:
  case kwRINSTR:
  case kwLBOUND:
  case kwUBOUND:
  case kwLEN:
  case kwEMPTY:
  case kwISARRAY:
  case kwISMAP:
  case kwISNUMBER:
  case kwISSTRING:
  case kwRGB:
  case kwRGBF:
    eval_callf_int(fcode, r);
    break;
  case kwATAN2:
  case kwPOW:
  case kwROUND:
    eval_callf_numN(fcode, r);
    break;
  case kwCOS:
  case kwSIN:
  case kwTAN:
  case kwCOSH:
  case kwSINH:
  case kwTANH:
  case kwACOS:
  case kwASIN:
  case kwATAN:
  case kwACOSH:
  case kwASINH:
  case kwATANH:
  case kwSEC:
  case kwSECH:
  case kwASEC:
  case kwASECH:
  case kwCSC:
  case kwCSCH:
  case kwACSC:
  case kwACSCH:
  case kwCOT:
  case kwCOTH:
  case kwACOT:
  case kwACOTH:
  case kwSQR:
  case kwABS:
  case kwEXP:
  case kwLOG:
  case kwLOG10:
  case kwFIX:
  case kwINT:
  case kwDEG:
  case kwRAD:
  case kwPENF:
  case kwFLOOR:
  case kwCEIL:
  case kwFRAC:
    eval_callf_mathN1(fcode, r);
    break;
  case kwFRE:
  case kwSGN:
  case kwEOF:
  case kwSEEKF:
  case kwLOF:
    eval_callf_imathI1(fcode, r);
    break;
  case kwXPOS:
  case kwYPOS:
  case kwRND:
    eval_callf_mathN2(fcode, r);
    break;
  case kwMAX:
  case kwMIN:
  case kwABSMAX:
  case kwABSMIN:
  case kwSUM:
  case kwSUMSV:
  case kwSTATMEAN:
  case kwSTATMEANDEV:
  case kwSTATSPREADS:
  case kwSTATSPREADP:
  case kwSEGCOS:
  case kwSEGSIN:
  case kwSEGLEN:
  case kwPOLYAREA:
  case kwPOLYCENT:
  case kwPTDISTSEG:
  case kwPTSIGN:
  case kwPTDISTLN:
  case kwPOINT:
  case kwINPUTF:
  case kwCODEARRAY:
  case kwGAUSSJORDAN:
  case kwFILES:
  case kwINVERSE:
  case kwDETERM:
  case kwJULIAN:
  case kwDATEFMT:
  case kwWDAY:
  case kwIFF:
  case kwFORMAT:
  case kwBGETC:
  case kwSEQ:
  case kwIMAGE:
  case kwFORM:
  case kwWINDOW:
    eval_callf_genfunc(fcode, r);
    break;
  case kwTICKS:
  case kwTIMER:
  case kwPROGLINE:
    eval_callf_imathI2(fcode, r);
    break;
  case kwFREEFILE:
    eval_callf_free(r);
    break;
  case kwARRAY:
    map_from_str(r);
    break;
  default:
    err_bfn_err(fcode);
  }
}

//
// executes the expression (Code[IP]) and returns the result (r)
//
void eval(var_t *r) {
  var_t *left = NULL;
  bcip_t eval_pos = eval_sp;
  byte level = 0;

  while (!prog_error) {
    byte code = prog_source[prog_ip];
    switch (code) {
    case kwTYPE_INT:
      // integer - constant
      IP++;
      V_FREE(r);
      r->type = V_INT;
      r->v.i = code_getint();
      break;

    case kwTYPE_NUM:
      // double - constant
      IP++;
      V_FREE(r);
      r->type = V_NUM;
      r->v.n = code_getreal();
      break;

    case kwTYPE_ADDOPR:
      IP++;
      oper_add(r, left);
      break;

    case kwTYPE_MULOPR:
      IP++;
      oper_mul(r, left);
      break;

    case kwTYPE_VAR:
      // variable
      V_FREE(r);
      eval_var(r, code_getvarptr());
      break;

    case kwTYPE_LEVEL_BEGIN:
      // left parenthesis
      IP++;
      level++;
      break;

    case kwTYPE_LEVEL_END:
      // right parenthesis
      if (level == 0) {
        eval_sp = eval_pos;
        // warning: normal exit
        return;
      }
      level--;
      IP++;
      break;

    case kwTYPE_EVPUSH:
      // stack = push result
      IP++;
      eval_push(r);
      break;

    case kwTYPE_EVPOP:
      // pop left
      IP++;
      if (!eval_sp) {
        err_syntax_unknown();
        return;
      }
      eval_sp--;
      left = &eval_stk[eval_sp];
      break;

    case kwTYPE_CALLF:
      // built-in functions
      IP++;
      eval_callf(r);
      break;

    case kwTYPE_STR:
      // string - constant
      IP++;
      V_FREE(r);
      v_eval_str(r);
      break;

    case kwTYPE_LOGOPR:
      IP++;
      oper_log(r, left);
      break;

    case kwTYPE_CMPOPR:
      IP++;
      oper_cmp(r, left);
      break;

    case kwTYPE_POWOPR:
      IP++;
      oper_powr(r, left);
      break;

    case kwTYPE_UNROPR:
      // unary
      IP++;
      oper_unary(r);
      break;

    case kwTYPE_EVAL_SC:
      IP++;
      eval_shortc(r);
      break;

    case kwTYPE_CALL_UDF:
      eval_call_udf(r);
      break;

    case kwTYPE_CALLEXTF:
      // [lib][index] external functions
      IP++;
      eval_extf(r);
      break;

    case kwTYPE_PTR:
      // UDF pointer - constant
      IP++;
      eval_ptr(r);
      break;

    case kwBYREF:
      // unexpected code
      err_evsyntax();
      return;

    default: {
      if (code == kwTYPE_LINE ||
          code == kwTYPE_SEP ||
          code == kwTO ||
          code == kwTHEN ||
          code == kwSTEP ||
          kw_check_evexit(code)) {
        // restore stack pointer
        eval_sp = eval_pos;

        // normal exit
        return;
      }
      rt_raise("UNKNOWN ERROR. IP:%d=0x%02X", IP, code);
      if (!opt_quiet) {
        hex_dump(prog_source, prog_length);
      }
    }};
  }

  // restore stack pointer
  eval_sp = eval_pos;
}
