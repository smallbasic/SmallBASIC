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

void err_title_msg(const char *seg, const char *file, int line) {
  gsb_last_line = line;
  gsb_last_error = prog_error;
  strlcpy(gsb_last_file, file, sizeof(gsb_last_file));
  log_printf("\n\033[0m");
  log_printf("\033[7m * %s-%s %s:%d * \033[0m\n\n", seg, WORD_ERROR_AT, file, line);
}

void err_detail_msg(const char *descr) {
  strlcpy(prog_errmsg, descr, sizeof(prog_errmsg));
  strlcpy(gsb_last_errmsg, prog_errmsg, sizeof(gsb_last_errmsg));
  log_printf("\033[4m%s:\033[0m\n%s\n", WORD_DESCRIPTION, descr);
  log_printf("\033[80m\033[0m");
}

void va_err_detail_msg(const char *format, va_list args, unsigned size) {
  char *buff = malloc(size + 1);
  buff[0] = '\0';
  vsnprintf(buff, size + 1, format, args);
  buff[size] = '\0';
  err_detail_msg(buff);
  free(buff);
}

void err_stack_msg() {
  int header = 0;

  // log the stack trace
  for (int i_stack = prog_stack_count; i_stack > 0; i_stack--) {
    stknode_t node = prog_stack[i_stack - 1];
    switch (node.type) {
    case 0xFF:
    case kwBYREF:
    case kwTYPE_CRVAR:
      // ignore these types
      break;

    default:
      for (int i_kw = 0; keyword_table[i_kw].name[0] != '\0'; i_kw++) {
        if (node.type == keyword_table[i_kw].code) {
          if (!header) {
            log_printf("\033[4mStack:\033[0m\n");
            header = 1;
          }
          dev_log_stack(keyword_table[i_kw].name, node.type, node.line);
          log_printf(" %s: %d", keyword_table[i_kw].name, node.line);
          break;
        }
      }
    }
  }
}

/**
 * raise a pre-execution/compiler error
 */
void sc_raise(const char *format, ...) {
  if (!comp_error) {
    comp_error = 1;

    va_list args;
    va_start(args, format);
    unsigned size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (comp_bc_sec) {
      err_title_msg(WORD_COMP, comp_bc_sec, comp_line);
    }
    if (size) {
      va_start(args, format);
      va_err_detail_msg(format, args, size);
      va_end(args);
    }
  }
}

/**
 * run-time error
 */
void rt_raise(const char *format, ...) {
  if (!gsb_last_error && ctask && !prog_error && prog_source) {
    prog_error = errRuntime;

    va_list args;
    va_start(args, format);
    unsigned size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    err_title_msg(WORD_RTE, prog_file, prog_line);
    if (size) {
      va_start(args, format);
      va_err_detail_msg(format, args, size);
      va_end(args);
    }
    err_stack_msg();
  }
}

/**
 * run-time syntax error
 */
void err_syntax(int keyword, const char *fmt) {
  if (!gsb_last_error && prog_source) {
    char *buff = malloc(SB_TEXTLINE_SIZE + 1);
    char *fmt_p = (char *)fmt;

    prog_error = errSyntax;
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

    err_title_msg(WORD_RTE, prog_file, prog_line);
    err_detail_msg(ERR_SYNTAX);
    err_stack_msg();
    log_printf("Expected:");
    log_printf(buff);
    log_printf("\n");
    free(buff);
  }
}

void err_missing_rp() {
  rt_raise(ERR_MISSING_RP);
}

void err_matdim() {
  rt_raise(ERR_MATRIX_DIM);
}

void err_noargs() {
  rt_raise(ERR_NO_ARGS);
}

void err_syntax_unknown() {
  rt_raise(ERR_SYNTAX);
}

void err_parm_num(int found, int expected) {
  rt_raise(ERR_PARAM_NUM, found, expected);
}

void err_parm_limit(int count) {
  rt_raise(MSG_PARNUM_LIMIT, count);
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
  err_throw(ERR_ARRAY_RANGE, i, m);
}

void err_typemismatch(void) {
  rt_raise(ERR_TYPE);
}

// parameter with wrong value
void err_argerr(void) {
  rt_raise(ERR_PARAM);
}

void err_varisarray(void) {
  err_throw(EVAL_VAR_IS_ARRAY);
}

void err_varisnotarray(void) {
  err_throw(EVAL_VAR_IS_NOT_ARRAY);
}

void err_vararridx(int i, int m) {
  err_throw(ERR_ARRAY_RANGE, i, m);
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

void err_out_of_range(void) {
  err_throw(ERR_RANGE);
}

void err_missing_sep(void) {
  rt_raise(ERR_MISSING_SEP_OR_PAR);
}

void err_division_by_zero(void) {
  err_throw(ERR_DIVZERO);
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

void err_memory() {
  rt_raise(ERR_MEMORY);
}

/**
 * the DONE message
 */
void inf_done() {
  dev_printf("\n\033[0m\033[80m\033[7m * %s * \033[0m\n", WORD_DONE);
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
int err_throw_catch(const char *err, var_t **catch_var) {
  var_t *arg;
  var_t v_catch;
  int caught = 1;
  switch (code_peek()) {
  case kwTYPE_VAR:
    arg = code_getvarptr();
    v_setstr(arg, err);
    *catch_var = arg;
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

// returns the position for the most TRY in the stack
int err_find_try(int position) {
  int result = -1;
  while (position) {
    if (prog_stack[--position].type == kwTRY) {
      result = position;
      break;
    }
  }
  return result;
}

// throw string
void err_throw_str(const char *err) {
  int caught = 0;
  int throw_sp = prog_stack_count;
  int try_sp = err_find_try(throw_sp);
  int trace_done = 0;
  var_t *catch_var = NULL;

  if (!prog_error && try_sp != -1) {
    bcip_t catch_ip = prog_stack[try_sp].x.vtry.catch_ip;
    // position after kwCATCH
    code_jump(catch_ip + 1);

    // skip "end try" address
    code_getaddr();

    // fetch next catch in the current block
    catch_ip = code_getaddr();

    caught = err_throw_catch(err, &catch_var);
    int reset_sp = try_sp;

    while (!caught && (catch_ip != INVALID_ADDR || try_sp != -1)) {
      // find in the current block
      while (!caught && catch_ip != INVALID_ADDR) {
        code_jump(catch_ip + 1);
        code_getaddr();
        catch_ip = code_getaddr();
        caught = err_throw_catch(err, &catch_var);
      }
      // find in the next outer block
      if (!caught && try_sp != -1) {
        try_sp = err_find_try(try_sp);
        if (try_sp != -1) {
          reset_sp = try_sp;
          catch_ip = prog_stack[try_sp].x.vtry.catch_ip;
          code_jump(catch_ip + 1);
          code_getaddr();
          catch_ip = code_getaddr();
          caught = err_throw_catch(err, &catch_var);
        }
      }
    }

    if (!caught) {
      err_title_msg(WORD_RTE, prog_file, prog_line);
      err_detail_msg(err);
      err_stack_msg();
      trace_done = 1;
    }

    // cleanup the stack
    int sp;
    int nodeType = 0;
    for (sp = throw_sp; sp > reset_sp; sp--) {
      nodeType = code_pop_and_free();
    }
    if (nodeType != kwTRY) {
      err_stackmess();
    }
  }
  if (!caught) {
    prog_error = errRuntime;
    if (!trace_done) {
      err_title_msg(WORD_RTE, prog_file, prog_line);
      err_detail_msg(err);
      err_stack_msg();
    }
  } else {
    stknode_t *node = code_push(kwCATCH);
    node->x.vcatch.catch_var = catch_var;
    prog_error = errThrow;
  }
}

// throw internal error
void err_throw(const char *fmt, ...) {
  if (!gsb_last_error && prog_source) {
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

void err_file(uint32_t code) {
  if (!gsb_last_error) {
    char *err = malloc(SB_TEXTLINE_SIZE + 1);
    sprintf(err, FSERR_FMT, code, strerror(code));
    strupper(err);
    err_throw_str(err);
    free(err);
  }
}

void err_file_not_found() {
  err_throw(FSERR_NOT_FOUND);
}

int err_handle_error(const char *err, var_p_t var) {
  int result;
  if (prog_error == errThrow) {
    result = 1;
  } else if (prog_error) {
    rt_raise(err);
    result = 1;
  } else {
    result = 0;
  }
  if (result && var) {
    v_free(var);
  }
  return result;
}
