// This file is part of SmallBASIC
//
// pseudo-compiler: expressions (warning: the input is byte-code segment)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/smbas.h"
#include "common/bc.h"

static bc_t *bc_in;
static bc_t *bc_out;

#define cev_add1(x)     bc_add_code(bc_out, (x))
#define cev_add2(x, y)  { bc_add1(bc_out, (x)); bc_add1(bc_out, (y)); }
#define cev_add_addr(x) bc_add_addr(bc_out, (x))
#define IP              bc_in->cp
#define CODE(x)         bc_in->ptr[(x)]
#define CODE_PEEK()     CODE(IP)
#define IF_ERR_RTN      if (comp_error) return

void cev_log(void);

void cev_udp(void) {
  sc_raise("(EXPR): UDP INSIDE EXPR");
}

void cev_missing_rp(void) {
  sc_raise("(EXPR): MISSING ')'");
}

void cev_opr_err(void) {
  sc_raise("(EXPR): SYNTAX ERROR (%d)", CODE(IP));
}

void cev_prim_str() {
  uint32_t len;
  memcpy(&len, bc_in->ptr + bc_in->cp, OS_STRLEN);
  IP += OS_STRLEN;
  bc_add_dword(bc_out, len);
  bc_add_n(bc_out, bc_in->ptr + bc_in->cp, len);
  IP += len;
}

void cev_prim_uds() {
  while (CODE_PEEK() == kwTYPE_UDS_EL) {
    cev_add1(kwTYPE_UDS_EL);
    cev_add1(kwTYPE_STR);
    IP += 2;
    cev_prim_str();
  }
}

void cev_prim_var() {
  bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ);
  IP += ADDRSZ;

  cev_prim_uds();

  // support multiple ()
  while (CODE_PEEK() == kwTYPE_LEVEL_BEGIN) {
    cev_add1(kwTYPE_LEVEL_BEGIN);
    IP++;
    if (CODE_PEEK() == kwTYPE_LEVEL_END) {
      // NULL ARRAYS
      cev_add1(kwTYPE_LEVEL_END);
      IP++;
    } else {
      cev_log();

      while (CODE_PEEK() == kwTYPE_SEP || CODE_PEEK() == kwTO) {
        // DIM X(A TO B)
        if (CODE_PEEK() == kwTYPE_SEP) {
          cev_add1(CODE(IP));
          IP++;
        }
        cev_add1(CODE(IP));
        IP++;

        cev_log();
      }

      if (CODE_PEEK() != kwTYPE_LEVEL_END) {
        cev_missing_rp();
      } else {
        cev_add1(kwTYPE_LEVEL_END);
        IP++;
        cev_prim_uds();
      }
    }
  }
}

void cev_empty_args() {
  cev_add1(kwTYPE_LEVEL_BEGIN);
  cev_add1(kwTYPE_LEVEL_END);
  IP += 2;
}

// function [(...)]
void cev_prim_args() {
  cev_add1(kwTYPE_LEVEL_BEGIN);
  IP++;

  if (CODE_PEEK() == kwTYPE_CALL_PTR) {
    cev_add1(CODE(IP));
    IP++;
  }

  if (CODE_PEEK() != kwTYPE_SEP) {
    // empty parameter
    cev_log();
  }
  while (CODE_PEEK() == kwTYPE_SEP) {
    // while parameters
    cev_add1(CODE(IP));
    IP++;
    cev_add1(CODE(IP));
    IP++;

    if (CODE_PEEK() != kwTYPE_LEVEL_END) {
      if (CODE_PEEK() != kwTYPE_SEP) {
        cev_log();
      }
    }
  }

  // after (), check for UDS field, eg foo(10).x
  if (CODE_PEEK() == kwTYPE_UDS_EL) {
    cev_prim_uds();
    cev_log();
  }

  if (CODE_PEEK() != kwTYPE_LEVEL_END) {
    cev_missing_rp();
  } else {
    cev_add1(kwTYPE_LEVEL_END);
    IP++;
  }
}

// test for repeated primatives
void cev_check_dup_prim() {
  switch (CODE(IP)) {
  case kwTYPE_INT:
  case kwTYPE_NUM:
  case kwTYPE_STR:
  case kwTYPE_VAR:
  case kwTYPE_CALLF:
    cev_opr_err();
    break;
  default:
    break;
  }
}

/*
 * prim
 */
void cev_prim() {
  IF_ERR_RTN;
  byte code = CODE(IP);
  IP++;
  cev_add1(code);
  switch (code) {
  case kwTYPE_INT:
    bc_add_n(bc_out, bc_in->ptr + bc_in->cp, OS_INTSZ);
    IP += OS_INTSZ;
    cev_check_dup_prim();
    break;
  case kwTYPE_NUM:
    bc_add_n(bc_out, bc_in->ptr + bc_in->cp, OS_REALSZ);
    IP += OS_REALSZ;
    cev_check_dup_prim();
    break;
  case kwTYPE_STR:
    cev_prim_str();
    cev_check_dup_prim();
    break;
  case kwTYPE_CALL_UDP:
    cev_udp();
    cev_check_dup_prim();
    break;
  case kwTYPE_PTR:
    bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ); // addr
    IP += ADDRSZ;
    bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ); // return var
    IP += ADDRSZ;
    cev_check_dup_prim();
    break;
  case kwTYPE_VAR:
    cev_prim_var();
    cev_check_dup_prim();
    break;
  case kwTYPE_CALL_UDF:        // [udf1][addr2]
  case kwTYPE_CALLEXTF:        // [lib][index]
    bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ);
    IP += ADDRSZ;              // no break here
  case kwTYPE_CALLF:           // [code]
    bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ);
    IP += ADDRSZ;              // no break here
  default:
    if (CODE_PEEK() == kwTYPE_LEVEL_BEGIN) {
      if (CODE(IP + 1) == kwTYPE_LEVEL_END) {
        cev_empty_args();
      } else {
        cev_prim_args();
      }
    }
    if (code != kwBYREF) {
      cev_check_dup_prim();
    }
    break;
  };
}

/*
 * parenthesis
 */
void cev_parenth() {
  IF_ERR_RTN;
  if (CODE_PEEK() == kwTYPE_LEVEL_BEGIN) {
    if (CODE(IP + 1) == kwTYPE_LEVEL_END) {
      cev_empty_args();
    } else {
      cev_prim_args();
    }
  } else {
    cev_prim();
  }
}

/*
 * unary
 */
void cev_unary() {
  char op;

  IF_ERR_RTN;
  if (CODE(IP) == kwTYPE_UNROPR || CODE(IP) == kwTYPE_ADDOPR) {
    op = CODE(IP + 1);
    IP += 2;
  } else {
    op = 0;
  }
  cev_parenth();        // R = cev_parenth
  if (op) {
    cev_add1(kwTYPE_UNROPR);
    cev_add1(op);       // R = op R
  }
}

/*
 * pow
 */
void cev_pow() {
  cev_unary();                  // R = cev_unary

  IF_ERR_RTN;
  while (CODE(IP) == kwTYPE_POWOPR) {
    IP += 2;

    cev_add1(kwTYPE_EVPUSH);    // PUSH R
    cev_unary();                // R = cev_unary
    IF_ERR_RTN;
    cev_add1(kwTYPE_EVPOP);     // POP LEFT
    cev_add2(kwTYPE_POWOPR, '^'); // R = LEFT op R
  }
}

/*
 * mul | div | mod
 */
void cev_mul() {
  cev_pow();                    // R = cev_pow()

  IF_ERR_RTN;
  while (CODE(IP) == kwTYPE_MULOPR) {
    char op;

    op = CODE(++IP);
    IP++;
    cev_add1(kwTYPE_EVPUSH);    // PUSH R

    cev_pow();
    IF_ERR_RTN;
    cev_add1(kwTYPE_EVPOP);      // POP LEFT
    cev_add2(kwTYPE_MULOPR, op); // R = LEFT op R
  }
}

/*
 * add | sub
 */
void cev_add() {
  cev_mul();                    // R = cev_mul()

  IF_ERR_RTN;
  while (CODE(IP) == kwTYPE_ADDOPR) {
    char op;

    IP++;
    op = CODE(IP);
    IP++;
    cev_add1(kwTYPE_EVPUSH);    // PUSH R

    cev_mul();                  // R = cev_mul
    IF_ERR_RTN;

    cev_add1(kwTYPE_EVPOP);    // POP LEFT
    cev_add2(kwTYPE_ADDOPR, op); // R = LEFT op R
  }
}

/*
 * compare
 */
void cev_cmp() {
  cev_add();                    // R = cev_add()

  IF_ERR_RTN;
  while (CODE(IP) == kwTYPE_CMPOPR) {
    char op;

    IP++;
    op = CODE(IP);
    IP++;
    cev_add1(kwTYPE_EVPUSH);    // PUSH R
    cev_add();                  // R = cev_add()
    IF_ERR_RTN;
    cev_add1(kwTYPE_EVPOP);         // POP LEFT
    cev_add2(kwTYPE_CMPOPR, op);    // R = LEFT op R
  }
}

/*
 * logical
 */
void cev_log(void) {
  cev_cmp();                    // R = cev_cmp()
  IF_ERR_RTN;
  while (CODE(IP) == kwTYPE_LOGOPR) {
    byte op;
    bcip_t shortcut;
    bcip_t shortcut_offs;

    IP++;
    op = CODE(IP);
    IP++;

    cev_add1(kwTYPE_EVPUSH);    // PUSH R (push the left side result
    cev_add1(kwTYPE_EVAL_SC);
    cev_add2(kwTYPE_LOGOPR, op);
    shortcut = bc_out->count;   // shortcut jump target (calculated below)
    cev_add_addr(0);

    cev_cmp();                  // right seg // R = cev_cmp()
    IF_ERR_RTN;
    cev_add1(kwTYPE_EVPOP);    // POP LEFT
    cev_add2(kwTYPE_LOGOPR, op); // R = LEFT op R

    shortcut_offs = bc_out->count - shortcut;
    memcpy(bc_out->ptr + shortcut, &shortcut_offs, ADDRSZ);
  }
}

/*
 * main
 */
void expr_parser(bc_t *bc_src) {
  // init
  bc_in = bc_src;
  bc_out = malloc(sizeof(bc_t));
  bc_create(bc_out);

  byte code = CODE_PEEK();

  //
  // empty!
  //
  if (code == kwTYPE_LINE || code == kwTYPE_EOC) {
    bc_destroy(bc_out);
    free(bc_out);
    return;
  }
  //
  // LET|CONST special code
  //
  if (code == kwTYPE_CMPOPR) {
    IP++;
    if (CODE(IP) != '=') {
      cev_opr_err();
      bc_destroy(bc_out);
      free(bc_out);
      return;
    } else {
      IP++;
      cev_add2(kwTYPE_CMPOPR, '=');
    }
  }
  // start
  code = CODE_PEEK();
  while (code != kwTYPE_EOC && code != kwTYPE_LINE && !comp_error) {
    if (kw_check_evexit(code)) {  // separator
      cev_add1(code);
      IP++;                     // add sep.
      if (code == kwUSE) {
        cev_add_addr(0);
        // USE needs 2 ips
        cev_add_addr(0);
        IP += (ADDRSZ + ADDRSZ);
      } else if (code == kwAS) {
        if (CODE_PEEK() == kwTYPE_SEP) {  // OPEN ... AS #1
          cev_add1(kwTYPE_SEP);
          IP++;
          cev_add1(CODE(IP));
          IP++;
        }
      } else {
        if (code == kwTYPE_SEP) { // Normal separator (,;)
          cev_add1(CODE(IP));
          IP++;
        }
      }
      code = CODE_PEEK();       // next
      continue;
    }
    cev_log();                  // do it
    code = CODE_PEEK();         // next
  }

  // finish
  if (bc_out->count) {
    bc_in->count = 0;
    bc_append(bc_in, bc_out);
  }

  bc_destroy(bc_out);
  free(bc_out);
}
