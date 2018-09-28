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

typedef enum {
  errNone = 0,
  errEnd,
  errBreak,
  errThrow,
  errCompile,
  errRuntime,
  errSyntax
} ErrorState;

#define IF_ERR_RETURN if (prog_error) {return;}
#define IF_ERR_RETURN_0 if (prog_error) {return 0;}

void sc_raise(const char *fmt, ...);
void rt_raise(const char *fmt, ...);
void err_missing_rp();
void err_matdim();
void err_noargs();
void err_syntax(int keyword, const char *fmt);
void err_syntax_unknown();
void err_parm_num(int found, int expected);
void err_parm_limit(int count);
void err_typemismatch(void);
void err_stackmess(void);
void err_parm_byref(int n);
void err_out_of_range(void);
void err_missing_lp(void);
void err_missing_sep(void);
void err_missing_comma(void);
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
void err_file(uint32_t code);
void err_file_not_found();
void err_matsig(void);
void err_parfmt(const char *fmt);
void err_fopen(void);
void err_syntaxsep(const char *seps);
void err_parsepoly(int idx, int mark);
void err_bfn_err(long code);
void err_pcode_err(long pcode);
void err_const(void);
void err_notavar(void);
void err_run_err(const char *file);
void err_ref_var();
void err_ref_circ_var();
void err_array();
void err_form_input();
void err_memory();
void err_throw(const char *fmt, ...);
int  err_handle_error(const char *err, var_p_t var);
void inf_done(void);
void inf_break(int pline);
void cmd_throw();

#if defined(__cplusplus)
  }
#endif
#endif
