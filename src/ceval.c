// $Id: ceval.c,v 1.5 2007-07-13 23:06:43 zeeb90au Exp $
// -*- c-file-style: "java" -*-
// This file is part of SmallBASIC
//
// pseudo-compiler: expressions (warning: the input is byte-code segment)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "smbas.h"
#include "bc.h"

extern void sc_raise(const char *fmt, ...) SEC(BCSCAN);
void cev_udp(void) SEC(TRASH);
void cev_missing_rp(void) SEC(TRASH);
void cev_opr_err(void) SEC(TRASH);
void cev_log(void) SEC(BCSCAN);
void cev_prim(void) SEC(BCSCAN);
void cev_parenth(void) SEC(BCSCAN);
void cev_unary(void) SEC(BCSCAN);
void cev_pow(void) SEC(BCSCAN);
void cev_mul(void) SEC(BCSCAN);
void cev_add(void) SEC(BCSCAN);
void cev_cmp(void) SEC(BCSCAN);
void expr_parser(bc_t * bc_src) SEC(BCSCAN);

#define IP            bc_in->cp
#define CODE(x)       bc_in->ptr[(x)]
#define CODE_PEEK()   CODE(IP)

void cev_udp(void)
{
    sc_raise("(EXPR): UDP INSIDE EXPR");
}

#if defined(_PalmOS)
void cev_missing_rp(void)
{
    sc_raise("(EXPR): MISSING ')'");
}

void cev_opr_err(void)
{
    sc_raise("(EXPR): SYNTAX ERROR (1st OP)");
}
#else

#define cev_missing_rp()        printf("C %s:%d, BAS: %d, EXPR: MISSING ')' (CS:IP=%d)\n", __FILE__, __LINE__, comp_line, CODE_PEEK() | comp_error++)
#define cev_opr_err()           printf("C %s:%d, BAS: %d, EXPR: SYNTAX ERROR (1st OP) (CS:IP=%d)\n", __FILE__, __LINE__, comp_line, CODE_PEEK() | comp_error++)
#endif

static bc_t *bc_in;
static bc_t *bc_out;

#define cev_add1(x)     bc_add_code(bc_out, (x))
#define cev_add2(x, y)  { bc_add1(bc_out, (x)); bc_add1(bc_out, (y)); }
#define cev_add_addr(x) bc_add_addr(bc_out, (x))

/*
*   prim
*/
void cev_prim()
{
    byte code;
#if defined(OS_ADDR16)
    word len;
#else
    dword len;
#endif

    if (comp_error) {
        return;
    }
    code = CODE(IP);
    IP++;
    cev_add1(code);

    switch (code) {
    case kwTYPE_INT:
        bc_add_n(bc_out, bc_in->ptr + bc_in->cp, OS_INTSZ);
        IP += OS_INTSZ;
        break;
    case kwTYPE_NUM:
        bc_add_n(bc_out, bc_in->ptr + bc_in->cp, OS_REALSZ);
        IP += OS_REALSZ;
        break;
    case kwTYPE_STR:
        memcpy(&len, bc_in->ptr + bc_in->cp, OS_STRLEN);
        IP += OS_STRLEN;
#if defined(OS_ADDR16)
        bc_add_word(bc_out, len);
#else
        bc_add_dword(bc_out, len);
#endif
        bc_add_n(bc_out, bc_in->ptr + bc_in->cp, len);

        IP += len;
        break;
    case kwTYPE_CALL_UDP:
        cev_udp();
        break;

    case kwTYPE_PTR:
        bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ);       // addr
        IP += ADDRSZ;
        bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ);       // return var
        IP += ADDRSZ;
        break;

    case kwTYPE_UDS:
    case kwTYPE_UDS_EL:
    case kwTYPE_VAR:
        bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ);       // 1 addr
        IP += ADDRSZ;

        // support multiple ()
        while (CODE_PEEK() == kwTYPE_LEVEL_BEGIN) {
            cev_add1(kwTYPE_LEVEL_BEGIN);
            IP++;
            if (CODE_PEEK() == kwTYPE_LEVEL_END) {      // NULL ARRAYS
                cev_add1(kwTYPE_LEVEL_END);
                IP++;
            } else {
                cev_log();

                while (CODE_PEEK() == kwTYPE_SEP || CODE_PEEK() == kwTO) {      // DIM X(A TO
                                                                                // B)
                    if (CODE_PEEK() == kwTYPE_SEP) {
                        cev_add1(CODE(IP));
                        IP++;
                    }
                    cev_add1(CODE(IP));
                    IP++;

                    cev_log();
                }

                if (CODE_PEEK() != kwTYPE_LEVEL_END)
                    cev_missing_rp();
                else {
                    cev_add1(kwTYPE_LEVEL_END);
                    IP++;
                }
            }
        }
        break;
    case kwTYPE_CALL_UDF:      // [udf1][addr2]
    case kwTYPE_CALLEXTF:      // [lib][index]
        bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ);
        IP += ADDRSZ;
        // no break here
    case kwTYPE_CALLF:         // [code]
        bc_add_n(bc_out, bc_in->ptr + bc_in->cp, ADDRSZ);
        IP += ADDRSZ;
        // no break here
    default:
        // function [(...)]
        if (CODE_PEEK() == kwTYPE_LEVEL_BEGIN) {
            cev_add1(kwTYPE_LEVEL_BEGIN);
            IP++;

            if (CODE_PEEK() == kwTYPE_CALL_PTR) {
                cev_add1(CODE(IP));
                IP++;
            }

            if (CODE_PEEK() != kwTYPE_SEP)      // empty parameter
                cev_log();

            while (CODE_PEEK() == kwTYPE_SEP) { // while parameters
                cev_add1(CODE(IP));
                IP++;
                cev_add1(CODE(IP));
                IP++;

                if (CODE_PEEK() != kwTYPE_LEVEL_END) {
                    if (CODE_PEEK() != kwTYPE_SEP)
                        cev_log();
                }
            }

            if (CODE_PEEK() != kwTYPE_LEVEL_END)
                cev_missing_rp();
            else {
                cev_add1(kwTYPE_LEVEL_END);
                IP++;
            }
        }
    };
}

/*
*   parenthesis
*/
void cev_parenth()
{
    if (comp_error) {
        return;
    }
    if (CODE_PEEK() == kwTYPE_LEVEL_BEGIN) {
        cev_add1(kwTYPE_LEVEL_BEGIN);
        IP++;

        cev_log();              // R = cev_log

        if (comp_error)
            return;

        if (CODE_PEEK() == kwTYPE_SEP) {
            cev_add1(kwTYPE_SEP);
            IP++;
            cev_add1(CODE(IP));
            IP++;
        } else if (CODE_PEEK() != kwTYPE_LEVEL_END) {
            cev_missing_rp();
            return;
        } else {
            cev_add1(kwTYPE_LEVEL_END);
            IP++;
        }
    } else
        cev_prim();
}

/*
*   unary
*/
void cev_unary()
{
    char op;

    if (comp_error) {
        return;
    }
    if (CODE(IP) == kwTYPE_UNROPR || CODE(IP) == kwTYPE_ADDOPR) {
        op = CODE(IP + 1);
        IP += 2;
    } else {
        op = 0;
    }
    cev_parenth();              // R = cev_parenth
    if (op) {
        cev_add1(kwTYPE_UNROPR);
        cev_add1(op);           // R = op R
    }
}

/*
*   pow
*/
void cev_pow()
{
    cev_unary();                // R = cev_unary

    if (comp_error) {
        return;
    }
    while (CODE(IP) == kwTYPE_POWOPR) {
        IP += 2;

        cev_add1(kwTYPE_EVPUSH);        // PUSH R

        cev_unary();            // R = cev_unary
        if (comp_error)
            return;

        cev_add1(kwTYPE_EVPOP); // POP LEFT
        cev_add2(kwTYPE_POWOPR, '^');   // R = LEFT op R
    }
}

/*
*   mul | div | mod
*/
void cev_mul()
{
    cev_pow();                  // R = cev_pow()

    if (comp_error) {
        return;
    }
    while (CODE(IP) == kwTYPE_MULOPR) {
        char op;

        op = CODE(++IP);
        IP++;
        cev_add1(kwTYPE_EVPUSH);        // PUSH R

        cev_pow();
        if (comp_error)
            return;

        cev_add1(kwTYPE_EVPOP); // POP LEFT
        cev_add2(kwTYPE_MULOPR, op);    // R = LEFT op R
    }
}

/*
*   add | sub
*/
void cev_add()
{
    cev_mul();                  // R = cev_mul()

    if (comp_error) {
        return;
    }
    while (CODE(IP) == kwTYPE_ADDOPR) {
        char op;

        IP++;
        op = CODE(IP);
        IP++;
        cev_add1(kwTYPE_EVPUSH);        // PUSH R

        cev_mul();              // R = cev_mul
        if (comp_error)
            return;

        cev_add1(kwTYPE_EVPOP); // POP LEFT
        cev_add2(kwTYPE_ADDOPR, op);    // R = LEFT op R
    }
}

/*
*   compare
*/
void cev_cmp()
{
    cev_add();                  // R = cev_add()

    if (comp_error) {
        return;
    }
    while (CODE(IP) == kwTYPE_CMPOPR) {
        char op;

        IP++;
        op = CODE(IP);
        IP++;
        cev_add1(kwTYPE_EVPUSH);        // PUSH R

        cev_add();              // R = cev_add()
        if (comp_error)
            return;

        cev_add1(kwTYPE_EVPOP); // POP LEFT
        cev_add2(kwTYPE_CMPOPR, op);    // R = LEFT op R
    }
}

/*
*   logical
*/
void cev_log(void)
{
    cev_cmp();                  // R = cev_cmp()
    if (comp_error) {
        return;
    }
    while (CODE(IP) == kwTYPE_LOGOPR) {
        char op;

        IP++;
        op = CODE(IP);
        IP++;
        cev_add1(kwTYPE_EVPUSH);        // PUSH R

        cev_cmp();              // right seg // R = cev_cmp()
        if (comp_error)
            return;

        cev_add1(kwTYPE_EVPOP); // POP LEFT
        cev_add2(kwTYPE_LOGOPR, op);    // R = LEFT op R
    }
}

/*
*   main
*/
void expr_parser(bc_t * bc_src)
{
    byte code;

    // init
    bc_in = bc_src;
    bc_out = tmp_alloc(sizeof(bc_t));
    bc_create(bc_out);

    code = CODE_PEEK();

    // 
    // empty!
    // 
    if (code == kwTYPE_LINE || code == kwTYPE_EOC) {
        bc_destroy(bc_out);
        tmp_free(bc_out);
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
            tmp_free(bc_out);
            return;
        } else {
            IP++;
            cev_add2(kwTYPE_CMPOPR, '=');
        }
    }
    // start
    code = CODE_PEEK();
    while (code != kwTYPE_EOC && code != kwTYPE_LINE && !comp_error) {
        if (kw_check_evexit(code)) {    // separator
            cev_add1(code);
            IP++;               // add sep.

            if (code == kwUSE) {
                cev_add_addr(0);        // USE needs 2 ips
                cev_add_addr(0);
                IP += (ADDRSZ + ADDRSZ);
            } else if (code == kwAS) {
                if (CODE_PEEK() == kwTYPE_SEP) {        // OPEN ... AS #1
                    cev_add1(kwTYPE_SEP);
                    IP++;
                    cev_add1(CODE(IP));
                    IP++;
                }
            } else {
                if (code == kwTYPE_SEP) {       // Normal separator (,;)
                    cev_add1(CODE(IP));
                    IP++;
                }
            }

            code = CODE_PEEK(); // next
            continue;
        }

        cev_log();              // do it
        code = CODE_PEEK();     // next
    }

    // finish
    if (bc_out->count) {
        bc_in->count = 0;
        bc_append(bc_in, bc_out);
    }

    bc_destroy(bc_out);
    tmp_free(bc_out);
}
