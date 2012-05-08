// This file is part of SmallBASIC
//
// SmallBASIC-executor: expressions
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/smbas.h"
#include "common/panic.h"
#include "common/pproc.h"
#include "common/str.h"
#include "common/kw.h"
#include "common/blib.h"
#include "common/device.h"
#include "common/extlib.h"

#define IP           prog_ip
#define CODE(x)      prog_source[(x)]
#define CODE_PEEK()  CODE(IP)
#define V_FREE(v)    if ( (v) ) { if ( (v)->type == V_STR || (v)->type == V_ARRAY ) v_free((v)); }

void eval(var_t *result);
void mat_op1(var_t *l, int op, var_num_t n) SEC(BMATH);
void mat_antithetos(var_t *v) SEC(BMATH);
void mat_mulN(var_t *v, var_num_t N) SEC(BMATH);
void mat_op2(var_t *l, var_t *r, int op) SEC(BMATH);
void mat_add(var_t *l, var_t *r) SEC(BMATH);
void mat_sub(var_t *l, var_t *r) SEC(BMATH);
void mat_mul(var_t *l, var_t *r) SEC(BMATH);
int v_wc_match(var_t* vwc, var_t *v) SEC(BMATH);

/**
 * matrix: convert var_t to double[r][c]
 */
var_num_t* mat_toc(var_t *v, int32 *rows, int32 *cols) {
  int i, j, pos;
  var_t *e;
  var_num_t *m;

  m = NULL;
  *rows = *cols = 0;

  if (!v) {
    // uninitialised variable
    return NULL;
  }

  if (v->v.a.maxdim > 2) {
    // too many dimensions
    err_matdim();
    return NULL;
  }
  *rows = ABS(v->v.a.lbound[0] - v->v.a.ubound[0]) + 1;

  if (v->v.a.maxdim == 2) {
    *cols = ABS(v->v.a.lbound[1] - v->v.a.ubound[1]) + 1;
  } else {
    *cols = *rows;
    *rows = 1;
  }

  m = (var_num_t*) tmp_alloc(((*rows) * (*cols)) * sizeof(var_num_t));
  for (i = 0; i < *rows; i++) {
    for (j = 0; j < *cols; j++) {
      pos = i * (*cols) + j;
      e = (var_t *) (v->v.a.ptr + (sizeof(var_t) * pos));
      m[pos] = v_getval(e);
    }
  }

  return m;
}

/**
 * matrix: conv. double[nr][nc] to var_t
 */
void mat_tov(var_t * v, var_num_t *m, int rows, int cols, int protect_col1) {
  var_t *e;
  int i, j, pos;

  if (cols > 1 || protect_col1) {
    v_tomatrix(v, rows, cols);
  } else {
    v_toarray1(v, rows);
  }

  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      pos = i * cols + j;
      e = (var_t *) (v->v.a.ptr + (sizeof(var_t) * pos));
      e->type = V_NUM;
      e->v.n = m[pos];
    }
  }
}

/**
 * matrix: 1op
 */
void mat_op1(var_t *l, int op, var_num_t n) {
  var_num_t *m1, *m;
  int lr, lc, pos;
  int i, j;

  m1 = mat_toc(l, &lr, &lc);
  if (m1) {
    m = (var_num_t*) tmp_alloc(sizeof(var_num_t) * lr * lc);
    for (i = 0; i < lr; i++) {
      for (j = 0; j < lc; j++) {
        pos = i * lc + j;
        switch (op) {
        case '*':
          m[pos] = m1[pos] * n;
          break;
        case 'A':
          m[pos] = -m1[pos];
          break;
        }
      }
    }

    // /
    mat_tov(l, m, lr, lc, 1);
    tmp_free(m1);
    tmp_free(m);
  }
}

/**
 * M = -A
 */
void mat_antithetos(var_t *v) {
  mat_op1(v, 'A', 0);
}

/**
 * M = �A
 */
void mat_mulN(var_t *v, var_num_t N) {
  mat_op1(v, '*', N);
}

/**
 * matrix - add/sub
 */
void mat_op2(var_t *l, var_t *r, int op) {
  var_num_t *m1, *m2, *m = NULL;
  int lr, lc, rr, rc, pos;
  int i, j;

  m1 = mat_toc(l, &lr, &lc);
  if (m1) {
    m2 = mat_toc(r, &rr, &rc);
    if (m2) {
      if (rc != lc || lr != rr) {
        err_matdim();
      } else {
        m = (var_num_t*) tmp_alloc(sizeof(var_num_t) * lr * lc);
        for (i = 0; i < lr; i++) {
          for (j = 0; j < lc; j++) {
            pos = i * lc + j;
            if (op == '+') {
              m[pos] = m1[pos] + m2[pos];
            } else {
              m[pos] = m2[pos] - m1[pos];
            }
            // array is comming reversed because of
            // where to store
          }
        }
      }

      // /
      tmp_free(m1);
      tmp_free(m2);
      if (m) {
        if (r->v.a.maxdim == 1)
          mat_tov(l, m, lc, 1, 0);
        else
          mat_tov(l, m, lr, lc, 1);
        tmp_free(m);
      }
    } else {
      tmp_free(m1);
    }
  }
}

void mat_add(var_t *l, var_t *r) {
  mat_op2(l, r, '+');
}

void mat_sub(var_t *l, var_t *r) {
  mat_op2(l, r, '-');
}

/**
 * matrix: multiply
 */
void mat_mul(var_t *l, var_t *r) {
  var_num_t *m1, *m2, *m = NULL;
  int lr, lc, rr, rc, pos;
  int mr = 0, mc = 0;
  int i, j, k;

  m1 = mat_toc(l, &lr, &lc);
  if (m1) {
    m2 = mat_toc(r, &rr, &rc);
    if (m2) {
      if (lc != rr)
        err_matdim();
      else {
        mr = lr;
        mc = rc;
        m = (var_num_t*) tmp_alloc(sizeof(var_num_t) * mr * mc);

        for (i = 0; i < mr; i++) {
          for (j = 0; j < mc; j++) {
            pos = i * mc + j;
            m[pos] = 0.0;
            for (k = 0; k < lc; k++)
              m[pos] = m[pos] + (m1[i * lc + k] * m2[k * rc + j]);
          }
        }
      }

      // /
      tmp_free(m1);
      tmp_free(m2);
      if (m) {
        mat_tov(r, m, mr, mc, 1);
        tmp_free(m);
      }
    } else {
      tmp_free(m1);
    }
  }
}

/**
 * The LIKE operator
 */
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
    var_t *elem_p;

    ri = 1;
    for (i = 0; i < v->v.a.size; i++) {
      elem_p = v_elem(v, i);
      if (v_wc_match(vwc, elem_p) == 0) {
        ri = 0;
        break;
      }
    }
  } else if (v->type == V_STR) {
    ri = wc_match((char *) vwc->v.p.ptr, (char *) v->v.p.ptr);
  } else if (v->type == V_NUM || v->type == V_INT) {
    var_t *vt;

    vt = v_clone(v);
    v_tostr(vt);
    if (!prog_error) {
      ri = wc_match((char *) vwc->v.p.ptr, (char *) vt->v.p.ptr);
    }
    v_free(vt);
    tmp_free(vt);
  }
  return ri;
}

/**
 * executes the expression (Code[IP]) and returns the result (r)
 */
void eval(var_t *r) {
  code_t code;
  byte op;
  byte level = 0;
  addr_t len;
  var_t *var_p;
  var_t *left = NULL, vtmp;
  addr_t eval_pos = eval_sp;
  int i;
  long fcode;
  var_int_t li = 0, ri = 0;
  var_num_t lf, rf;
  addr_t addr;

  // / bit ops
  var_int_t a, b;
  var_int_t ba, bb;

  r->const_flag = 0;
  r->type = V_INT;
  r->v.i = 0L;

  do {
    code = CODE(IP);
    IP++;
    switch (code) {
    case kwTYPE_INT:
      // //////////// integer - constant //////////////
      V_FREE(r);
      r->type = V_INT;
      r->v.i = code_getint();
      break;

    case kwTYPE_NUM:
      // //////////// double - constant //////////////
      V_FREE(r);
      r->type = V_NUM;
      r->v.n = code_getreal();
      break;

    case kwTYPE_STR:
      // //////////// string - constant //////////////
      V_FREE(r);
      r->type = V_STR;
      len = code_getstrlen();
      r->v.p.ptr = tmp_alloc(len + 1);
      memcpy(r->v.p.ptr, &prog_source[prog_ip], len);
      *((char *) (r->v.p.ptr + len)) = '\0';
      r->v.p.size = len;

      IP += len;
      break;

    case kwTYPE_PTR:
      // //////////// UDF pointer - constant ///////////
      V_FREE(r);
      r->type = V_PTR;
      r->const_flag = 1;
      r->v.ap.p = code_getaddr();
      r->v.ap.v = code_getaddr();
      break;

      // ////////////// variable ///////////////////
    case kwTYPE_UDS:
    case kwTYPE_VAR:
      V_FREE(r);

      IP--;
      var_p = code_getvarptr();

      if (prog_error) {
        return;
      }
      switch (var_p->type) {
      case V_PTR:
        r->type = var_p->type;
        r->v.ap.p = var_p->v.ap.p;
        r->v.ap.v = var_p->v.ap.v;
        break;
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
      case V_UDS:
      case V_HASH:
        v_set(r, var_p);
        break;
      }
      ;                        // switch(type)

      break;

      // ///////////// STACK //////////////////
    case kwTYPE_EVPUSH:        // PUSH RESULT
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
        len = r->v.p.size;
        eval_stk[eval_sp].type = V_STR;
        eval_stk[eval_sp].v.p.ptr = tmp_alloc(len + 1);
        strcpy(eval_stk[eval_sp].v.p.ptr, r->v.p.ptr);
        eval_stk[eval_sp].v.p.size = len;
        break;
      default:
        v_set(&eval_stk[eval_sp], r);
      }

      // expression-stack resize
      eval_sp++;
      if (eval_sp == eval_size) {
        eval_size += 64;
        eval_stk = tmp_realloc(eval_stk, sizeof(var_t) * eval_size);
        // rt_raise("EVAL: STACK OVERFLOW");
      }
      break;

    case kwTYPE_EVPOP:         // POP LEFT
      eval_sp--;
      left = &eval_stk[eval_sp];
      break;

      // ////////////// ADD ///////////////////
    case kwTYPE_ADDOPR:
      op = CODE(IP);
      IP++;

      if (r->type == V_INT && left->type == V_INT) {
        if (op == '+') {
          r->v.i += left->v.i;
        } else {
          r->v.i = left->v.i - r->v.i;
        }
      } else if (r->type == V_NUM && left->type == V_NUM) {
        r->type = V_NUM;
        if (op == '+') {
          r->v.n += left->v.n;
        } else {
          r->v.n = left->v.n - r->v.n;
        }
      } else if (r->type == V_INT && left->type == V_NUM) {
        r->type = V_NUM;
        if (op == '+') {
          r->v.n = r->v.i + left->v.n;
        } else {
          r->v.n = left->v.n - r->v.i;
        }
      } else if (r->type == V_NUM && left->type == V_INT) {
        if (op == '+') {
          r->v.n += left->v.i;
        } else {
          r->v.n = left->v.i - r->v.n;
        }
      } else {
        if (r->type == V_ARRAY || left->type == V_ARRAY) {
          // 
          // ARRAYS
          // 
          if (r->type == V_ARRAY && left->type == V_ARRAY) {
            if (op == '+') {
              mat_add(r, left);
            } else {
              mat_sub(r, left);
            }
          } else {
            err_matop();
          }
        } else {
          // 
          // NOT ARRAY
          // 
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
          v_set(r, &vtmp);
          V_FREE(&vtmp);
        }

        V_FREE(left);
      }

      break;

      // ////////////// MUL ///////////////////
    case kwTYPE_MULOPR:
      op = CODE(IP);
      IP++;

      if (r->type == V_ARRAY || left->type == V_ARRAY) {
        // 
        // ARRAYS
        // 
        if (r->type == V_ARRAY && left->type == V_ARRAY) {
          if (op == '*') {
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
        // 
        // NOT ARRAY
        // 
        lf = v_getval(left);
        V_FREE(left);
        rf = v_getval(r);
        V_FREE(r);

        r->type = V_NUM;        // double always
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

      break;

      // ////////////// UNARY ///////////////////
    case kwTYPE_UNROPR:
      op = CODE(IP);
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
      case OPLOG_INV:          // The result of ~ is always integer
        ri = v_igetval(r);
        V_FREE(r);
        r->type = V_INT;
        r->v.i = ~ri;
        break;
      case OPLOG_NOT:          // The result of ! is always integer
        ri = v_igetval(r);
        V_FREE(r);
        r->type = V_INT;
        r->v.i = !ri;
        break;
      }

      // cleanup
      break;

    case kwTYPE_EVAL_SC:
      // short-circuit evaluation
      // see cev_log() in ceval.c for layout details
      IP++;                     // skip code kwTYPE_LOGOPR
      op = CODE(IP);            // read operator
      IP++;
      li = v_igetval(&eval_stk[eval_sp - 1]); // read left side result
      ri = -1;

      addr = code_getaddr();    // read shortcut jump offset

      switch (op) {
      case OPLOG_AND:
        if (!li) {
          ri = 0;               // False AND blah => result is false
        }
        break;
      case OPLOG_OR:
        if (li) {
          ri = 1;               // True OR blah => result is true
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
        IP += (addr - ADDRSZ);  // jump to the shortcut offset
      }
      break;

      // ////////////// LOGICAL/BIT ///////////////////
    case kwTYPE_LOGOPR:
      op = CODE(IP);
      IP++;

      if (op != OPLOG_IN) {
        li = v_igetval(left);
        ri = v_igetval(r);
      }

      switch (op) {
      case OPLOG_AND:
        ri = (li && ri) ? -1 : 0;
        break;
      case OPLOG_OR:
        ri = (li || ri) ? -1 : 0;
        break;
      case OPLOG_EQV:
        a = li;
        b = ri;
        ri = 0;
        for (i = 0; i < sizeof(var_int_t); i++) {
          ba = a & (1 << i);
          bb = b & (1 << i);
          if ((ba && bb) || (!ba && !bb)) {
            ri |= (1 << i);
          }
        }
        break;
      case OPLOG_IMP:
        a = li;
        b = ri;
        ri = 0;
        for (i = 0; i < sizeof(var_int_t); i++) {
          ba = a & (1 << i);
          bb = b & (1 << i);
          if (!(ba && !bb)) {
            ri |= (1 << i);
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
      }
      ;

      // cleanup
      V_FREE(left);
      V_FREE(r);

      // 
      r->type = V_INT;
      r->v.i = ri;

      break;

      // ////////////// COMPARE ///////////////////
    case kwTYPE_CMPOPR:
      op = CODE(IP);
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
          var_t *elem_p;

          for (i = 0; i < r->v.a.size; i++) {
            elem_p = v_elem(r, i);
            if (v_compare(left, elem_p) == 0) {
              ri = i + 1;
              break;
            }
          }
        } else if (r->type == V_STR) {
          if (left->type == V_STR) {
            if (left->v.p.ptr[0] != '\0') {
              ri = (strstr(r->v.p.ptr, left->v.p.ptr) != NULL);
            } else {
              ri = 0;
            }
          } else if (left->type == V_NUM || left->type == V_INT) {
            var_t *v;

            v = v_clone(left);
            v_tostr(v);
            ri = (strstr(r->v.p.ptr, v->v.p.ptr) != NULL);
            v_free(v);
            tmp_free(v);
          }
        } else if (r->type == V_NUM || r->type == V_INT) {
          ri = (v_compare(left, r) == 0);
        }
        break;
      case OPLOG_LIKE:
        ri = v_wc_match(r, left);
        break;
      }

      // cleanup
      V_FREE(left);
      V_FREE(r);
      // 
      r->type = V_INT;
      r->v.i = ri;
      break;

      // ////////////// POW ///////////////////
    case kwTYPE_POWOPR:
      op = CODE(IP);
      IP++;

      rf = pow(v_getval(left), v_getval(r));
      V_FREE(r);
      r->type = V_NUM;
      r->v.n = rf;

      // cleanup
      V_FREE(left);
      break;

      // ///////////////////////////////////////////////////////////////////////////////

    case kwTYPE_LEVEL_BEGIN:   // left parenthesis
      level++;
      break;

    case kwTYPE_LEVEL_END:     // right parenthesis
      if (level == 0) {
        IP--;
        eval_sp = eval_pos;
        return;                 // warning: normal exit
      }

      level--;
      break;

      // ///////////////////////////////////////////////////////////////////////////////
      // / external functions
    case kwTYPE_CALLEXTF:      // [lib][index]
    {
      addr_t lib, idx;

      lib = code_getaddr();
      idx = code_getaddr();
      V_FREE(r);
      if (lib & UID_UNIT_BIT) {
        unit_exec(lib & (~UID_UNIT_BIT), idx, r);
        // if ( prog_error )
        // return;
      } else {
        sblmgr_funcexec(lib, prog_symtable[idx].exp_idx, r);
      }
    }
      break;
      // ///////////////////////////////////////////////////////////////////////////////
      // / buildin functions
    case kwTYPE_CALLF:

      fcode = code_getaddr();

      switch (fcode) {
      case kwVADDR:
        // Variable's address
        V_FREE(r);

        if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
          err_missing_lp();
        } else {
          var_t *vp;

          IP++;
          vp = code_getvarptr();
          if (!prog_error) {
            r->type = V_INT;
            r->v.i = (intptr_t) vp->v.p.ptr;
          }

          if (CODE_PEEK() != kwTYPE_LEVEL_END) {
            err_missing_rp();
          } else {
            IP++;
          }
        }

        break;

        // num FUNC(STR)
      case kwASC:
      case kwVAL:
      case kwTEXTWIDTH:
      case kwTEXTHEIGHT:
      case kwEXIST:
      case kwISFILE:
      case kwISDIR:
      case kwISLINK:
      case kwACCESSF:
        V_FREE(r);

        if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
          err_missing_lp();
        } else {
          IP++;

          v_init(&vtmp);
          eval(&vtmp);
          if (!prog_error) {
            cmd_ns1(fcode, &vtmp, r);
            V_FREE(&vtmp);

            if (CODE_PEEK() != kwTYPE_LEVEL_END) {
              err_missing_rp();
            } else {
              IP++;
            }
          }
        }
        break;

        // str FUNC(any)
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
      case kwBALLOC:
      case kwBCS:
      case kwCBS:

        V_FREE(r);

        if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
          err_missing_lp();
        } else {
          IP++;

          v_init(&vtmp);
          eval(&vtmp);
          if (!prog_error) {
            r->type = V_STR;
            r->v.p.ptr = NULL;
            cmd_str1(fcode, &vtmp, r);
            V_FREE(&vtmp);

            if (CODE_PEEK() != kwTYPE_LEVEL_END) {
              err_missing_rp();
            } else {
              IP++;
            }
          }
        }
        break;

        // str FUNC(...)
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
        V_FREE(r);

        if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
          err_missing_lp();
        } else {
          r->type = V_STR;
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
        break;

        // str FUNC(void)
      case kwINKEY:
      case kwTIME:
      case kwDATE:
        V_FREE(r);

        cmd_str0(fcode, r);
        break;

        // int FUNC(...)
      case kwINSTR:
      case kwRINSTR:
      case kwLBOUND:
      case kwUBOUND:
      case kwLEN:
      case kwEMPTY:
      case kwISARRAY:
      case kwISNUMBER:
      case kwISSTRING:
      case kwRGB:
      case kwRGBF:
      case kwIMGW:
      case kwIMGH:
        V_FREE(r);

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
        break;

        // fp FUNC(...)
      case kwATAN2:
      case kwPOW:
      case kwROUND:
        V_FREE(r);

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
        break;

        // fp FUNC(fp)
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
      case kwCDBL:
      case kwDEG:
      case kwRAD:
      case kwPENF:
      case kwFLOOR:
      case kwCEIL:
      case kwFRAC:
        V_FREE(r);

        if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
          err_missing_lp();
        } else {
          IP++;

          v_init(&vtmp);
          eval(&vtmp);
          if (!prog_error) {
            r->type = V_NUM;
            r->v.n = cmd_math1(fcode, &vtmp);
            V_FREE(&vtmp);
            if (!prog_error) {
              if (CODE_PEEK() != kwTYPE_LEVEL_END) {
                err_missing_rp();
              } else {
                IP++;
              }
            }
          }
        }
        break;

        // int FUNC(fp)
      case kwFRE:
      case kwSGN:
      case kwCINT:
      case kwEOF:
      case kwSEEKF:
      case kwLOF:
        V_FREE(r);

        if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
          err_missing_lp();
        } else {
          IP++;

          v_init(&vtmp);
          eval(&vtmp);
          if (!prog_error) {
            r->type = V_INT;
            r->v.i = cmd_imath1(fcode, &vtmp);

            if (CODE_PEEK() != kwTYPE_LEVEL_END) {
              err_missing_rp();
            } else {
              IP++;
            }
          }
        }
        break;

        // fp FUNC(void)
      case kwXPOS:
      case kwYPOS:
      case kwRND:
        V_FREE(r);

        vtmp.type = V_NUM;
        vtmp.v.n = 0;
        r->type = V_NUM;
        r->v.n = cmd_math1(fcode, &vtmp);
        break;

        // any FUNC(...)
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

        V_FREE(r);
        if (CODE_PEEK() != kwTYPE_LEVEL_BEGIN) {
          err_missing_lp();
        } else {
          IP++;

          cmd_genfunc(fcode, r);
          if (!prog_error) {
            if (CODE_PEEK() == kwTYPE_LEVEL_END) {
              IP++;
            }
          }
        }
        break;

        // int FUNC(void)
      case kwTICKS:
      case kwTICKSPERSEC:
      case kwTIMER:
      case kwPROGLINE:
        V_FREE(r);

        vtmp.type = V_INT;
        vtmp.v.i = 0;
        r->type = V_INT;
        r->v.i = cmd_imath1(fcode, &vtmp);
        break;

      case kwFREEFILE:
        V_FREE(r);

        r->type = V_INT;
        r->v.i = dev_freefilehandle();
        break;

      default:
        err_bfn_err(fcode);
      }
      ;

      // ////////////// funcs -- end /////////////////
      break;

    case kwTYPE_CALL_UDF:
      prog_ip--;
      bc_loop(1);

      if (!prog_error) {
        stknode_t udf_rv;

        code_pop(&udf_rv);
        if (udf_rv.type != kwTYPE_RET)
          err_stackmess();
        else {
          v_set(r, udf_rv.x.vdvar.vptr);
          v_free(udf_rv.x.vdvar.vptr);  // free ret-var
          tmp_free(udf_rv.x.vdvar.vptr);
        }
      }

      break;

      // ////////////////////////////////////
    default:
      if (kw_check_evexit(code)) {
        IP--;
        eval_sp = eval_pos;     // restore stack pointer
        return;                 // warning: normal exit
      }

      rt_raise("UNKNOWN FUNC=%d", code);
    };

    // run-time error check
    if (prog_error) {
      break;
    }

  } while (1);

  eval_sp = eval_pos;           // restore stack pointer
}
