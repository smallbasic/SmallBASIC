// $Id: sberr.h,v 1.3 2006-10-24 21:05:47 zeeb90au Exp $
// -*- c-file-style: "java" -*-
// This file is part of SmallBASIC
//
// SmallBASIC run-time errors
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_sberr_h)
#define _sberr_h

#include "smbas.h"

#if defined(__cplusplus)
extern "C" {
#endif

void    sc_raise2(const char *sec, int scline, const char *buff);
void    rt_raise(const char *fmt, ...);

#if defined(OS_LIMITED)
void    err_syntax(void)            SEC(TRASH);     // SYNTAX ERROR
void    err_syntaxsep(int c)        SEC(TRASH);     // MISSING SEPARATOR 'c'
void    err_matdim(void)            SEC(TRASH);
void    err_missing_rp(void)        SEC(TRASH);
void    err_parm_num(void)          SEC(TRASH);     // UDP/F: PARAMETERS NUMBER INCORRECT
#else
#define err_syntax()                rt_raise("%s (%d): Syntax error!\n", __FILE__, __LINE__)
#define err_syntaxsep(c)            rt_raise("%s (%d): Syntax error. Missing separator '%c'.\n", __FILE__, __LINE__, (c))
#define err_missing_rp()            rt_raise("%s (%d): Missing ')'.\n", __FILE__, __LINE__)
#define err_matdim()                rt_raise("%s (%d): Dimension error.\n", __FILE__, __LINE__)
#define err_parm_num()              rt_raise("%s (%d): Parameters error.\n", __FILE__, __LINE__)
#endif
void    err_typemismatch(void)      SEC(TRASH);     // TYPE MISMATCH
void    err_stackmess(void)         SEC(TRASH);     // GENERIC STACK ERROR (if you play with GOTOs, you can create it easily)
void    err_parm_byref(int n)       SEC(TRASH);     // UDP/F: PARAMETER IS 'BY REFERENCE' SO CONSTANTS DOES NOT ALLOWED
void    err_notarray(void)          SEC(TRASH);
void    err_out_of_range(void)      SEC(TRASH);
void    err_missing_lp(void)        SEC(TRASH);
void    err_missing_sep(void)       SEC(TRASH);
void    err_division_by_zero(void)  SEC(TRASH);
void    err_matop(void)             SEC(TRASH);
void    err_argerr(void)            SEC(TRASH);     // Parameter with wrong value
void    err_stackoverflow(void)     SEC(TRASH);
void    err_stackunderflow(void)    SEC(TRASH);
void    err_arrmis_lp(void)         SEC(TRASH);
void    err_arrmis_rp(void)         SEC(TRASH);
void    err_arridx(int i, int m)    SEC(TRASH);
void    err_varisarray(void)        SEC(TRASH);
void    err_varisnotarray(void)     SEC(TRASH);
void    err_vararridx(int i, int m) SEC(TRASH);
void    err_varnotnum(void)         SEC(TRASH);
void    err_evsyntax(void)          SEC(TRASH);
void    err_evtype(void)            SEC(TRASH);
void    err_evargerr(void)          SEC(TRASH);
void    err_unsup(void)             SEC(TRASH);
void    err_file(dword code)        SEC(TRASH);
void    err_matsig(void)            SEC(TRASH);
void    err_stridx(void)            SEC(TRASH);
void    err_parfmt(const char *fmt) SEC(TRASH);
void    err_fopen(void)             SEC(TRASH);
void    err_syntaxanysep(const char *seps)  SEC(TRASH);     // No separator found
void    err_parsepoly(int idx, int mark)    SEC(TRASH);
void    err_bfn_err(long code)              SEC(TRASH);
void    err_gpf(addr_t addr, int bc)        SEC(TRASH);
void    err_pcode_err(long pcode)           SEC(TRASH);
void    err_chain_err(const char *file)     SEC(TRASH);
void    err_const(void)                     SEC(TRASH);
void    err_notavar(void)                   SEC(TRASH);
void    err_run_err(const char *file)       SEC(TRASH);
void    err_invkw(addr_t addr, byte code)   SEC(TRASH);

#define err_type_mismatch()         err_typemismatch()
#define err_syntax_error()          err_syntax()

// --- inf_xxx: information messages 

void    inf_done(void)          SEC(TRASH);
void    inf_break(int pline)    SEC(TRASH);
void    inf_comprq_dv(void)     SEC(TRASH);
void    inf_comprq_dt(void)     SEC(TRASH);
void    inf_comprq_prq(void)    SEC(TRASH);
void    inf_low_battery(void)   SEC(TRASH);

#if defined(__cplusplus)
}
#endif

#endif

