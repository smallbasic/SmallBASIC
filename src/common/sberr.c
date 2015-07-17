// This file is part of SmallBASIC
//
// SmallBASIC run-time errors
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/smbas.h"
#include "common/pproc.h"
#include "common/messages.h"
#include "common/var.h"
#include <string.h>
#include <errno.h>

/**
 * common message handler
 */
void err_common_msg(const char *seg, const char *file, int line, const char *descr) {
  gsb_last_line = line;
  gsb_last_error = prog_error;
  strcpy(gsb_last_file, file);
  strncpy(prog_errmsg, descr, SB_ERRMSG_SIZE);
  prog_errmsg[SB_ERRMSG_SIZE] = '\0';
  strcpy(gsb_last_errmsg, prog_errmsg);
  log_printf("\n\033[0m\033[80m\n");
  log_printf("\033[7m * %s-%s %s:%d * \033[0m\a\n\n", seg, WORD_ERROR_AT, file, line);
  log_printf("\033[4m%s:\033[0m\n%s\n", WORD_DESCRIPTION, descr);
  log_printf("\033[80m\033[0m");
}

/**
 * raise a compiler error
 */
void sc_raise2(const char *sec, int scline, const char *buff) {
  prog_error = 0x40;
  err_common_msg(WORD_COMP, sec, scline, buff);
}

/**
 * run-time error
 */
void rt_raise(const char *fmt, ...) {
  char *buff;
  va_list ap;
  int i_stack, i_kw;

  if (!gsb_last_error) {
    prog_error = 0x80;

    va_start(ap, fmt);
    buff = malloc(SB_TEXTLINE_SIZE + 1);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    err_common_msg(WORD_RTE, prog_file, prog_line, buff);
    free(buff);

    if (prog_stack_count) {
      log_printf("\033[4mStack:\033[0m\n");
    }

    // log the stack trace
    for (i_stack = prog_stack_count; i_stack > 0; i_stack--) {
      stknode_t node = prog_stack[i_stack];
      switch (node.type) {
        case 0xFF:
        case kwBYREF:
        case kwTYPE_CRVAR:
        // ignore these types
        break;

        default:
        for (i_kw = 0; keyword_table[i_kw].name[0] != '\0'; i_kw++) {
          if (node.type == keyword_table[i_kw].code) {
            log_printf(" %s: %d", keyword_table[i_kw].name, node.line);
          }
        }
      }
    }
    if (prog_stack_count) {
      log_printf("\n");
    }
  }
}

/**
 * run-time syntax error
 */
void err_syntax(int keyword, const char *fmt) {
  if (!gsb_last_error) {
    char *buff = malloc(SB_TEXTLINE_SIZE + 1);
    char *fmt_p = (char *)fmt;

    prog_error = 0x80;
    buff[0] = '\0';
    if (keyword != -1) {
      if (kw_getfuncname(keyword, buff) ||
          kw_getprocname(keyword, buff) ||
          kw_getcmdname(keyword, buff)) {
        strcpy(buff, " ");
      }
    }

    while (*fmt_p) {
      if (*fmt_p == '%') {
        fmt_p++;
        switch (*fmt_p) {
        case 'S':
          strcat(buff, "STRING");
          break;
        case 'I':
          strcat(buff, "INTEGER");
          break;
        case 'F':
          strcat(buff, "REAL");
          break;
        case 'P':
          strcat(buff, "VARIABLE");
          break;
        case 'G':
          strcat(buff, "FUNC");
          break;
        case 'N':
          strcat(buff, "NUM");
          break;
        }
      } else {
        int len = strlen(buff);
        buff[len] = *fmt_p;
        buff[len + 1] = '\0';
      }
      fmt_p++;
    }
    err_common_msg(WORD_RTE, prog_file, prog_line, ERR_SYNTAX);
    log_printf("Expected:");
    log_printf(buff);
    log_printf("\n");
    free(buff);
  }
}

void err_parm_num(int found, int expected) {
  rt_raise(ERR_PARAM_NUM, found, expected);
}

void err_file(dword code) {
  char buf[1024], *p;

  strcpy(buf, strerror(code));
  p = buf;
  while (*p) {
    *p = to_upper(*p);
    p++;
  }
  err_throw(FSERR_FMT, code, buf);
}

void err_stackoverflow(void) {
  rt_raise(ERR_STACK_OVERFLOW);
}

void err_stackunderflow(void) {
  rt_raise(ERR_STACK_UNDERFLOW);
}

// generic stack error
void err_stackmess() {
  rt_raise(ERR_STACK);
}

void err_arrmis_lp(void) {
  rt_raise(ERR_ARRAY_MISSING_LP);
}

void err_arrmis_rp(void) {
  rt_raise(ERR_ARRAY_MISSING_RP);
}

void err_arridx(int i, int m) {
  rt_raise(ERR_ARRAY_RANGE, i, m);
}

void err_typemismatch(void) {
  rt_raise(ERR_TYPE);
}

// parameter with wrong value
void err_argerr(void) {
  rt_raise(ERR_PARAM);
}

void err_varisarray(void) {
  rt_raise(EVAL_VAR_IS_ARRAY);
}

void err_varisnotarray(void) {
  rt_raise(EVAL_VAR_IS_NOT_ARRAY);
}

void err_vararridx(int i, int m) {
  rt_raise(ERR_ARRAY_RANGE, i, m);
}

void err_varnotnum(void) {
  rt_raise(EVAL_NOT_A_NUM);
}

void err_evsyntax(void) {
  rt_raise(EVAL_SYNTAX);
}

void err_evtype(void) {
  rt_raise(EVAL_TYPE);
}

void err_evargerr(void) {
  rt_raise(EVAL_PARAM);
}

void err_unsup(void) {
  rt_raise(ERR_UNSUPPORTED);
}

void err_const(void) {
  rt_raise(ERR_CONST);
}

void err_notavar(void) {
  rt_raise(ERR_NOT_A_VAR);
}

void err_notarray(void) {
  rt_raise(ERR_NOT_ARR_OR_FUNC);
}

void err_out_of_range(void) {
  rt_raise(ERR_RANGE);
}

void err_missing_sep(void) {
  rt_raise(ERR_MISSING_SEP_OR_PAR);
}

void err_division_by_zero(void) {
  rt_raise(ERR_DIVZERO);
}

void err_matop(void) {
  rt_raise(ERR_OPERATOR);
}

void err_matsig(void) {
  rt_raise(ERR_MATSIG);
}

void err_missing_lp(void) {
  rt_raise(ERR_MISSING_LP);
}

void err_parfmt(const char *fmt) {
  rt_raise(ERR_PARFMT, fmt);
}

// UDP/F: parameter is 'by reference' so const not allowed
void err_parm_byref(int n) {
  rt_raise(ERR_BYREF, n);
}

void err_stridx(int n) {
  rt_raise(ERR_STR_RANGE, n);
}

void err_fopen(void) {
  rt_raise(ERR_BAD_FILE_HANDLE);
}

// no separator found
void err_syntaxsep(const char *seps) {
  rt_raise(ERR_SEP_FMT, seps);
}

void err_missing_comma(void) {
  rt_raise(ERR_SEP_FMT, ",");
}

void err_parsepoly(int idx, int mark) {
  rt_raise(ERR_POLY, idx, mark);
}

void err_bfn_err(long code) {
  rt_raise(ERR_CRITICAL_MISSING_FUNC, code);
}

void err_pcode_err(long pcode) {
  rt_raise(ERR_CRITICAL_MISSING_PROC, pcode);
}

void err_chain_err(const char *file) {
  rt_raise(ERR_CHAIN_FILE, file);
}

void err_run_err(const char *file) {
  rt_raise(ERR_RUN_FILE, file);
}

void err_ref_var() {
  rt_raise(ERR_REF_VAR);
}

void err_ref_circ_var() {
  rt_raise(ERR_REF_CIRC_VAR);
}

void err_array() {
  rt_raise(MSG_ARRAY_SE);
}

void err_form_input() {
  err_throw(ERR_FORM_INPUT);
}

/**
 * the DONE message
 */
void inf_done() {
#if USE_TERM_IO
  if (!isatty(STDOUT_FILENO)) {
    fprintf(stdout, "\n* %s *\n", WORD_DONE);
  }
  else {
#endif

  dev_printf("\n\033[0m\033[80m\a\033[7m * %s * \033[0m\n", WORD_DONE);

#if USE_TERM_IO
}
#endif
}

// the BREAK message
void inf_break(int pline) {
  gsb_last_line = pline;
  gsb_last_error = prog_error;
  strcpy(gsb_last_file, prog_file);
  sprintf(gsb_last_errmsg, "%s %d", WORD_BREAK_AT, pline);

  dev_settextcolor(15, 0);
  log_printf("\n\033[0m\033[80m\033[7m * %s %d * \033[0m\n", WORD_BREAK_AT, pline);
}

// assign error to variable or match with next expression
int err_throw_catch(const char *err) {
  var_t *arg;
  var_t v_catch;
  int caught = 1;

  switch (code_peek()) {
  case kwTYPE_VAR:
    arg = code_getvarptr();
    v_setstr(arg, err);
    break;
  case kwTYPE_STR:
    v_init(&v_catch);
    eval(&v_catch);
    // catch is conditional on matching error
    caught = (v_catch.type == V_STR && strstr(err, v_catch.v.p.ptr) != NULL);
    v_free(&v_catch);
    break;
  case kwTYPE_EOC:
  case kwTYPE_LINE:
    break;
  default:
    rt_raise(ERR_INVALID_CATCH);
    break;
  }
  return caught;
}

// throw string
void err_throw_str(const char *err) {
  int caught = 0;
  if (!prog_error && prog_catch_ip != INVALID_ADDR && prog_catch_ip > prog_ip) {
    // position after kwCATCH
    code_jump(prog_catch_ip + 1);
    // skip "end try" address
    code_getaddr();
    // restore outer level
    prog_catch_ip = code_getaddr();
    // get the stack level
    byte level = code_getnext();

    caught = err_throw_catch(err);
    while (!caught && prog_catch_ip != INVALID_ADDR) {
      code_jump(prog_catch_ip + 1);
      code_getaddr();
      prog_catch_ip = code_getaddr();
      level = code_getnext();
      caught = err_throw_catch(err);
    }

    // cleanup the stack
    while (prog_stack_count > level) {
      code_pop_and_free(NULL);
    }
  }
  if (!caught) {
    prog_error = 0x80;
    err_common_msg(WORD_RTE, prog_file, prog_line, err);
  }
}

// throw internal error
void err_throw(const char *fmt, ...) {
  if (!gsb_last_error) {
    va_list ap;
    va_start(ap, fmt);
    char *err = malloc(SB_TEXTLINE_SIZE + 1);
    vsprintf(err, fmt, ap);
    va_end(ap);
    err_throw_str(err);
    free(err);
  }
}

// throw user error
void cmd_throw() {
  if (!gsb_last_error) {
    var_t v_throw;
    v_init(&v_throw);
    const char *err = "";
    byte code = code_peek();
    if (code != kwTYPE_EOC && code != kwTYPE_LINE) {
      eval(&v_throw);
      if (v_throw.type == V_STR) {
        err = v_throw.v.p.ptr;
      }
    }
    err_throw_str(err);
    v_free(&v_throw);
  }
}

