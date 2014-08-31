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

#include "common/smbas.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define IF_ERR_BREAK if (prog_error) {break;}
#define IF_ERR_RETURN if (prog_error) {return;}
#define IF_ERR_RETURN_0 if (prog_error) {return 0;}

void sc_raise2(const char *sec, int scline, const char *buff);
void rt_raise(const char *fmt, ...);

#define err_syntax()     rt_raise("%s (%d): Syntax error!\n", __FILE__, __LINE__)
#define err_syntaxsep(c) rt_raise("%s (%d): Syntax error. Missing separator '%c'.\n", __FILE__, __LINE__, (c))
#define err_missing_rp() rt_raise("%s (%d): Missing ')'.\n", __FILE__, __LINE__)
#define err_matdim()     rt_raise("%s (%d): Dimension error.\n", __FILE__, __LINE__)
#define err_parm_num()   rt_raise("%s (%d): Parameters error.\n", __FILE__, __LINE__)

void err_typemismatch(void);
void err_stackmess(void);
void err_parm_byref(int n);
void err_notarray(void);
void err_out_of_range(void);
void err_missing_lp(void);
void err_missing_sep(void);
void err_division_by_zero(void);
void err_matop(void);
void err_argerr(void);
void err_stackoverflow(void);
void err_stackunderflow(void);
void err_arrmis_lp(void);
void err_arrmis_rp(void);
void err_arridx(int i, int m);
void err_varisarray(void);
void err_varisnotarray(void);
void err_vararridx(int i, int m);
void err_varnotnum(void);
void err_evsyntax(void);
void err_evtype(void);
void err_evargerr(void);
void err_unsup(void);
void err_file(dword code);
void err_matsig(void);
void err_stridx(int n);
void err_parfmt(const char *fmt);
void err_fopen(void);
void err_syntaxanysep(const char *seps);
void err_parsepoly(int idx, int mark);
void err_bfn_err(long code);
void err_gpf(addr_t addr, int bc);
void err_pcode_err(long pcode);
void err_chain_err(const char *file);
void err_const(void);
void err_notavar(void);
void err_run_err(const char *file);
void err_invkw(addr_t addr, byte code);
void err_ref_var();

#define err_type_mismatch() err_typemismatch()
#define err_syntax_error()  err_syntax()

// --- inf_xxx: information messages

void inf_done(void);
void inf_break(int pline);
void inf_comprq_dv(void);
void inf_comprq_dt(void);
void inf_comprq_prq(void);
void inf_low_battery(void);

#if defined(__cplusplus)
  }
#endif
#endif
