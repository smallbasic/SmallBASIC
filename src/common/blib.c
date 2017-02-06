// This file is part of SmallBASIC
//
// SmallBASIC RTL - STANDARD COMMANDS
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/kw.h"
#include "common/var.h"
#include "common/var_map.h"
#include "common/device.h"
#include "common/blib.h"
#include "common/pproc.h"
#include "common/smbas.h"
#include "common/fmt.h"
#include "common/keymap.h"
#include "common/messages.h"

#define STR_INIT_SIZE 256

/**
 * LET v[(x)] = any
 * CONST v[(x)] = any
 */
void cmd_let(int is_const) {
  var_t *v_left = code_getvarptr();

  if (!prog_error) {
    if (v_left->const_flag) {
      err_const();
      return;
    }

    if (prog_source[prog_ip] == kwTYPE_CMPOPR &&
        prog_source[prog_ip + 1] == '=') {
      code_skipopr();
    }

    if (code_peek() == kwBYREF) {
      code_skipnext();
      v_eval_ref(v_left);
    } else {
      var_t v_right;
      v_init(&v_right);
      eval(&v_right);
      if (!prog_error) {
        v_set(v_left, &v_right);
        v_left->const_flag = is_const;
      }
      v_free(&v_right);
    }
  }
}

/**
 *  DIM var([lower TO] uppper [, ...])
 */
void cmd_dim(int preserve) {
  int exitf = 0;

  do {
    var_t *var_p = NULL;
    var_t arg, array;
    int i, size;
    byte code, zaf = 0;

    code = code_peek();
    if (code == kwTYPE_LINE || code == kwTYPE_EOC) {
      exitf = 1;
    } else {
      array.v.a.maxdim = 0;
      if (code_peek() == kwTYPE_SEP) {
        code_skipnext();
        if (code_getnext() != ',') {
          err_missing_comma();
        }
      }

      var_p = code_getvarptr_parens(1);

      if (!prog_error) {
        if (code_peek() == kwTYPE_LEVEL_BEGIN) {
          code_skipnext();
          zaf = 0;
          do {
            v_init(&arg);
            eval(&arg);
            if (prog_error) {
              return;
            }
            if (code_peek() == kwTO) {
              array.v.a.lbound[array.v.a.maxdim] = v_getint(&arg);
              code_skipnext();
              eval(&arg);
              if (prog_error) {
                return;
              }
              array.v.a.ubound[array.v.a.maxdim] = v_getint(&arg);
            } else {
              array.v.a.lbound[array.v.a.maxdim] = opt_base;
              array.v.a.ubound[array.v.a.maxdim] = v_getint(&arg);
            }
            v_free(&arg);
            array.v.a.maxdim++;

            // skip separator
            if (code_peek() == kwTYPE_SEP) {
              code_skipnext();
              if (code_getnext() != ',') {
                err_missing_comma();
              }
            }

          } while (code_peek() != kwTYPE_LEVEL_END);

          code_skipnext();      // ')', level
        } else {
          zaf = 1;              // zero-length array
        }
      } else {
        rt_raise(ERR_SYNTAX);
      }

      //
      // run...
      //
      if (!prog_error) {
        if (zaf) {
          v_toarray1(var_p, 0);
        } else {
          size = 1;
          for (i = 0; i < array.v.a.maxdim; i++) {
            size = size * (ABS(array.v.a.ubound[i] - array.v.a.lbound[i]) + 1);
          }
          if (!preserve) {
            v_toarray1(var_p, size);
          } else {
            if (var_p->type == V_ARRAY) {
              v_resize_array(var_p, size);
            } else {
              v_toarray1(var_p, size);
            }
          }

          // dim
          var_p->v.a.maxdim = array.v.a.maxdim;
          for (i = 0; i < array.v.a.maxdim; i++) {
            var_p->v.a.lbound[i] = array.v.a.lbound[i];
            var_p->v.a.ubound[i] = array.v.a.ubound[i];
          }
        }
      } else {
        exitf = 1;
      }
    }
  } while (!exitf && !prog_error);
}

/**
 * REDIM x
 */
void cmd_redim() {
  cmd_dim(1);
}

/**
 * APPEND A, x1 [, x2, ...]
 * or
 * A << x1 [, x2, ...]
 */
void cmd_ladd() {
  var_t *var_p = code_getvarptr();
  if (prog_error) {
    return;
  }

  if (code_peek() == kwTYPE_CMPOPR && prog_source[prog_ip + 1] == '=') {
    // compatible with LET, operator format
    code_skipopr();
  } else if (code_peek() == kwTYPE_SEP && prog_source[prog_ip + 1] == ',') {
    // command style
    code_skipsep();
  } else {
    err_missing_comma();
    return;
  }

  // for each argument to append
  var_t *arg_p = v_new();
  do {
    // get the value to append
    v_free(arg_p);

    int byref = 0;
    if (code_peek() == kwBYREF) {
      code_skipnext();
      byref = 1;
    } else {
      eval(arg_p);
    }
    if (prog_error) {
      break;
    }

    // find the array element
    var_t *elem_p;
    if (var_p->type != V_ARRAY) {
      v_toarray1(var_p, 1);
      elem_p = v_elem(var_p, 0);
    } else {
      v_resize_array(var_p, var_p->v.a.size + 1);
      elem_p = v_elem(var_p, var_p->v.a.size - 1);
    }

    // set the value onto the element
    if (byref) {
      v_eval_ref(elem_p);
    } else {
      v_set(elem_p, arg_p);
    }

    // next parameter
    if (code_peek() != kwTYPE_SEP) {
      break;
    } else {
      par_getcomma();
      if (prog_error) {
        break;
      }
    }
  } while (1);

  // cleanup
  v_free(arg_p);
  v_detach(arg_p);
}

/**
 * INSERT A, index, v1 [, vN]
 */
void cmd_lins() {
  var_t *var_p = code_getvarptr();
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }

  // convert to array
  if (var_p->type != V_ARRAY) {
    v_toarray1(var_p, 0);
  }

  // get 'index'
  int idx = par_getint();
  if (prog_error) {
    return;
  }
  idx -= var_p->v.a.lbound[0];

  par_getcomma();
  if (prog_error) {
    return;
  }

  int ladd = 0;
  if (idx >= var_p->v.a.size) {
    // append
    ladd = 1;
    idx = var_p->v.a.size;
  } else if (idx <= 0) {
    // insert at top
    idx = 0;
  }

  // for each argument to insert
  var_t *arg_p = v_new();
  do {
    // get the value to append
    v_free(arg_p);

    int byref = 0;
    if (code_peek() == kwBYREF) {
      code_skipnext();
      byref = 1;
    } else {
      eval(arg_p);
    }
    if (prog_error) {
      break;
    }

    // resize +1
    v_resize_array(var_p, var_p->v.a.size + 1);

    // find the array element
    var_t *elem_p;
    if (ladd) {
      // append
      elem_p = v_elem(var_p, var_p->v.a.size - 1);
    } else {
      // move all form idx one down
      int i;
      for (i = var_p->v.a.size - 1; i > idx; i--) {
        // A(i) = A(i-1)
        v_set(v_elem(var_p, i), v_elem(var_p, i - 1));
      }
      elem_p = v_elem(var_p, idx);
    }

    // set the value onto the element
    if (byref) {
      v_eval_ref(elem_p);
    } else {
      v_set(elem_p, arg_p);
    }

    // next parameter
    if (code_peek() != kwTYPE_SEP) {
      break;
    } else {
      par_getcomma();
      if (prog_error) {
        break;
      }
    }
  } while (1);

  // cleanup
  v_free(arg_p);
  v_detach(arg_p);
}

/**
 * DELETE A, index[, count]
 */
void cmd_ldel() {
  var_t *var_p, *arg_p;
  int idx, count = 1;
  int i, j;

  var_p = code_getvarptr();
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }
  // only arrays
  if (var_p->type != V_ARRAY) {
    err_argerr();
    return;
  }
  // get 'index'
  idx = par_getint();
  if (prog_error) {
    return;
  }
  idx -= var_p->v.a.lbound[0];
  if ((idx >= var_p->v.a.size) || (idx < 0)) {
    err_out_of_range();
    return;
  }
  // get 'count'
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error) {
      return;
    }
    count = par_getint();
    if (prog_error) {
      return;
    }
    if (((count + idx) - 1 > var_p->v.a.size)) {
      err_out_of_range();
      return;
    } else if (count <= 0) {
      err_argerr();
    }
  }
  // data
  arg_p = v_clone(var_p);
  v_resize_array(var_p, var_p->v.a.size - count);

  // first part
  for (i = 0; i < idx; i++) {
    // A(i) = OLD(i)
    v_set(v_elem(var_p, i), v_elem(arg_p, i));
  }
  // second part
  for (i = idx + count, j = idx; i < arg_p->v.a.size; i++, j++) {
    // A(j) = OLD(i)
    v_set(v_elem(var_p, j), v_elem(arg_p, i));
  }
  // cleanup
  v_free(arg_p);
  v_detach(arg_p);
}

/**
 * ERASE var1 [, var2[, ...]]
 */
void cmd_erase() {
  byte code;
  var_t *var_p;

  do {
    if (prog_error) {
      break;
    }
    code = code_peek();

    if (code_isvar()) {
      var_p = code_getvarptr();
    } else {
      err_typemismatch();
      break;
    }

    if (var_p->type == V_ARRAY) {
      v_toarray1(var_p, 0);
    } else {
      v_free(var_p);
      v_init(var_p);
    }

    // next
    code = code_peek();
    if (code == kwTYPE_SEP) {
      par_getcomma();
    } else {
      break;
    }

  } while (1);
}

/**
 * PRINT ...
 */
void cmd_print(int output) {
  byte last_op = 0;
  byte exitf = 0;
  byte use_format = 0;
  int handle = 0;
  var_t var;

  // prefix - # (file)
  if (output == PV_FILE) {
    par_getsharp();
    if (prog_error) {
      return;
    }
    handle = par_getint();
    if (prog_error) {
      return;
    }
    if (code_peek() == kwTYPE_EOC || code_peek() == kwTYPE_LINE) {
      // There are no parameters
      if (dev_fstatus(handle)) {
        dev_fwrite(handle, (byte *) "\n", 1);
      } else {
        err_fopen();
      }
      return;
    }

    par_getsep();
    if (prog_error) {
      return;
    }
    if (!dev_fstatus(handle)) {
      err_fopen();
      return;
    }
  }

  // prefix: memory variable
  if (output == PV_STRING) {
    if (!code_isvar()) {
      err_argerr();
      return;
    }

    var_t *vuser_p = code_getvarptr();
    par_getsemicolon();
    if (prog_error) {
      return;
    }
    v_free(vuser_p);
    vuser_p->type = V_STR;
    vuser_p->v.p.ptr = NULL;
    vuser_p->v.p.length = 0;
    handle = (intptr_t)vuser_p;
  }

  // prefix - USING
  byte code = code_peek();
  if (code == kwUSING) {
    code_skipnext();
    if (code_peek() != kwTYPE_SEP) {
      v_init(&var);
      eval(&var);
      if (prog_error) {
        return;
      }
      if (var.type != V_STR) {
        rt_raise(ERR_FORMAT_INVALID_FORMAT);
        v_free(&var);
        return;
      } else {
        build_format((char *) var.v.p.ptr);
        v_free(&var);
      }
    }
    if (code_peek() == kwTYPE_SEP) {
      par_getsemicolon();
    }
    last_op = ';';
    if (prog_error) {
      return;
    }
    use_format = 1;
  }

  // PRINT
  do {
    code = code_peek();
    if (code == kwTYPE_SEP) {
      code_skipnext();
      last_op = code_getnext();
      if (!use_format) {
        if (last_op == ',') {
          pv_write("\t", output, handle);
        }
      }
    } else {
      if (kw_check_evexit(code) || code == kwTYPE_LEVEL_END) {
        exitf = 1;
        break;
      }

      last_op = 0;
      v_init(&var);
      eval(&var);
      if (!prog_error) {
        if (use_format) {
          switch (var.type) {
          case V_STR:
            fmt_printS((char *)var.v.p.ptr, output, handle);
            break;
          case V_INT:
            fmt_printN(var.v.i, output, handle);
            break;
          case V_NUM:
            fmt_printN(var.v.n, output, handle);
            break;
          default:
            err_typemismatch();
          }
        } else {
          pv_writevar(&var, output, handle);
        }
      }
      v_free(&var);
    };

    if (prog_error) {
      return;
    }
  } while (exitf == 0);

  if (last_op == 0) {
    pv_write("\n", output, handle);
  }
}

/**
 * INPUT ...
 */
void cmd_input(int input) {
  byte code;
  byte print_crlf = 1;
  byte next_is_const = 0;
  byte input_is_finished = 0;
  var_t prompt;
  var_t *vuser_p = NULL;
  int handle = 0;
  int cur_par_idx;
  int unused_vars;
  char *inps = NULL, *inp_p;
  int pcount = 0, redo = 0;
  par_t *ptable = NULL;
  par_t *par;
  char *p, lc;

  v_init(&prompt);
  // prefix - # (file)
  if (input == PV_FILE) {
    par_getsharp();
    if (prog_error) {
      return;
    }
    handle = par_getint();
    if (prog_error) {
      return;
    }
    par_getsep();
    if (prog_error) {
      return;
    }
    if (!dev_fstatus(handle)) {
      err_fopen();
      return;
    }
  }

  // prefix: memory variable
  if (input == PV_STRING) {
    if (!code_isvar()) {
      err_argerr();
      return;
    }

    vuser_p = code_getvarptr();
    par_getsemicolon();
    if (prog_error) {
      return;
    }
    if (vuser_p->type == V_INT || vuser_p->type == V_NUM) {
      v_tostr(vuser_p);
    }
    if (vuser_p->type != V_STR) {
      err_argerr();
      return;
    }

    vuser_p->v.p.pos = 0;
    handle = (intptr_t)vuser_p;
  }

  // prefix: prompt
  if (input == PV_CONSOLE) {
    v_setstr(&prompt, "");

    if (code_peek() != kwTYPE_STR) {
      print_crlf = (code_peeksep() != ';');
      if (!print_crlf) {
        code_skipsep();
      }
    }
    if (!code_isvar()) {
      v_free(&prompt);
      eval(&prompt);

      code = code_getsep();
      if (!prog_error) {
        if (code == ';') {
          v_strcat(&prompt, "? ");
        }
      }
    } else {
      // no prompt
      v_setstr(&prompt, "? ");
    }
  } else {
    print_crlf = 0;
  }

  // get list of parameters
  pcount = par_getpartable(&ptable, ",;");
  if (pcount == 0) {
    rt_raise(ERR_INPUT_NO_VARS);
  }
  // the INPUT itself
  if (!prog_error) {
    do {
      // "redo from start"
      if (input == PV_CONSOLE) {  // prompt
        if (prompt.v.p.ptr) {
          pv_write((char *) prompt.v.p.ptr, input, handle);
        }
      }

      // get user's input
      switch (input) {
      case PV_CONSOLE:
        // console
        inps = malloc(SB_TEXTLINE_SIZE + 1);
        if (prompt.v.p.ptr) {
          // prime output buffer with prompt text
          int prompt_len = strlen(prompt.v.p.ptr);
          int len = prompt_len < SB_TEXTLINE_SIZE ? prompt_len : SB_TEXTLINE_SIZE;
          strncpy(inps, prompt.v.p.ptr, len);
          inps[len] = 0;
        }
        dev_gets(inps, SB_TEXTLINE_SIZE);
        break;
      case PV_STRING:
        // string (SINPUT)
        inps = strdup((char *)vuser_p->v.p.ptr);
        break;
      case PV_FILE:
        // file (INPUT#)
      {
        int index, size;
        byte ch, quotes;

        size = STR_INIT_SIZE;
        inps = malloc(size);
        index = 0;
        quotes = 0;

        while (!dev_feof(handle)) {
          dev_fread(handle, &ch, 1);
          if (prog_error) {
            break;
          } else if (ch == '\n' && !quotes) {
            break;
          } else if (ch != '\r') {
            // store char
            if (index == (size - 2)) {
              size += STR_INIT_SIZE;
              inps = realloc(inps, size);
            }

            inps[index] = ch;
            index++;
            if (ch == '\"') {
              quotes = !quotes;
            }
          }
        }

        inps[index] = '\0';
      }
        break;
      }

      /*
       *      for each variable
       */
      cur_par_idx = 0;
      inp_p = inps;
      input_is_finished = 0;
      unused_vars = 0;

      while (cur_par_idx < pcount && !prog_error) {
        par = &ptable[cur_par_idx];

        if (input_is_finished) {
          // setup all remaining variables to "null"
          if (!(par->flags & PAR_BYVAL)) {
            v_setstr(par->var, "");
            unused_vars++;
          }
        } else {             // we continue to read
          if (par->flags & PAR_BYVAL) {
            // no constants are allowed
            err_typemismatch();
            break;
          } else {
            // check if user had specify a delimiter (next parameter is NOT a
            // variable)
            next_is_const = 0;
            if (cur_par_idx < (pcount - 1)) {
              if (ptable[cur_par_idx + 1].flags & PAR_BYVAL) {
                cur_par_idx++;
                // par = previous parameter
                // ptable[cur_par_idx] = the constant
                next_is_const = 1;
              }
            }
            // get next string
            v_free(par->var);

            // next_is_const = get the left string of the specified word
            // NOT next_is_const = get the left string of the specified
            // character
            // (,)
            if (pcount == 1) {
              p = (inp_p + strlen(inp_p));
            } else {
              p = q_strstr(inp_p, ((next_is_const) ? v_getstr(ptable[cur_par_idx].var) : ","), "\"\"");
            }
            if (p) {
              lc = *p;
              *p = '\0';
              v_input2var(inp_p, par->var);
              *p = lc;

              // next pos
              inp_p = p + ((next_is_const) ? strlen((char *) ptable[cur_par_idx].var->v.p.ptr) : 1);
              if (*p == '\0') {
                input_is_finished = 1;
              }
            } else {
              v_input2var(inp_p, par->var);
              inp_p = (inp_p + strlen(inp_p));  // go to '\0'
              input_is_finished = 1;
            }
          }
        }

        // next
        cur_par_idx++;
      }

      // REDO FROM START
      if (cur_par_idx == pcount) {
        input_is_finished = 1;
      }

      if (input_is_finished && (input == PV_CONSOLE) && (!prog_error)
          && (((pcount > 1) && (*inp_p || unused_vars)))) {
        redo = 1;
        free(inps);
        dev_printf("\n\a\033[7m * %s * \033[0m\n", WORD_INPUT_REDO);
      } else {
        redo = 0;
      }

    } while (redo && !prog_error);
  }

  // exit
  if (input == PV_CONSOLE) {
    if (print_crlf && (prog_error == 0)) {
      pv_write("\n", input, handle);
    }
  }

  if (inps) {
    free(inps);
  }
  par_freepartable(&ptable, pcount);
  v_free(&prompt);
}

/**
 * ON x GOTO|GOSUB ...
 */
void cmd_on_go() {
  bcip_t dest_ip;
  var_t var;
  stknode_t node;

  bcip_t next_ip = code_getaddr();
  code_skipaddr();
  code_t command = code_getnext();
  byte count = code_getnext();
  bcip_t table_ip = prog_ip;
  bcip_t expr_ip = prog_ip + (count * ADDRSZ);

  v_init(&var);
  prog_ip = expr_ip;
  eval(&var);

  bcip_t index = (v_igetval(&var) - 1);
  if (((int) index == -1) || ((int) index >= (int) count)) {
    // index == -1 (0 on BASIC) || index >= count do nothing
    command = kwNULL;
    prog_ip = next_ip;
    dest_ip = 0;
  } else if ((int) index < 0) {
    // QB: run-time-error on < 0 or > 255
    rt_raise(ERR_ONGOTO_RANGE, (command == kwGOTO) ? WORD_GOTO : WORD_GOSUB);
    dest_ip = 0;
  } else {
    // default
    memcpy(&dest_ip, prog_source + table_ip + (index * ADDRSZ), ADDRSZ);
  }

  switch (command) {
  case kwGOTO:
    code_jump(dest_ip);
    break;
  case kwGOSUB:
    node.type = kwGOSUB;
    node.x.vgosub.ret_ip = next_ip;
    code_jump(dest_ip);
    code_push(&node);
    break;
  case kwNULL:
    break;
  default:
    rt_raise("ON x: INTERNAL ERROR");
  }
}

/**
 * GOSUB label
 */
void cmd_gosub() {
  stknode_t node;
  bid_t goto_label = code_getaddr();
  node.type = kwGOSUB;
  node.x.vgosub.ret_ip = prog_ip;
  code_jump_label(goto_label);
  code_push(&node);
}

/**
 * Call a user-defined procedure or function
 *
 * What will happend to the stack
 * [param 1]
 * ...
 * [param N]
 * [udp-call node]
 *
 * p1...pN nodes will be removed by cmd_param()
 * cmd_param is the first UDP/F's command
 *
 * @param cmd is the type of the udp (function or procedure)
 * @param target sub/func
 * @param return-variable ID
 */
bcip_t cmd_push_args(int cmd, bcip_t goto_addr, bcip_t rvid) {
  stknode_t param;
  bcip_t ofs;
  bcip_t pcount = 0;
  var_t *arg = NULL;

  if (code_peek() == kwTYPE_LEVEL_BEGIN) {
    // kwTYPE_LEVEL_BEGIN (which means left-parenthesis)
    code_skipnext();

    if (code_peek() == kwTYPE_CALL_PTR) {
      // replace call address with address in first arg
      var_t var_ptr;
      code_skipnext();
      v_init(&var_ptr);
      eval(&var_ptr);
      if (var_ptr.type != V_PTR || var_ptr.v.ap.p == 0) {
        rt_raise("Invalid %s pointer variable", cmd == kwPROC ? "SUB" : "FUNC");
        return 0;
      }
      goto_addr = var_ptr.v.ap.p;
      rvid = var_ptr.v.ap.v;
    }

    byte ready = 0;
    do {
      byte code = code_peek();  // get next BC
      switch (code) {
      case kwTYPE_EOC:       // end of an expression (parameter)
        code_skipnext();     // ignore it
        break;
      case kwTYPE_SEP:       // separator (comma or semi-colon)
        code_skipsep();      // ignore it
        break;
      case kwTYPE_LEVEL_END: // (right-parenthesis) which means: end of parameters
        code_skipnext();
        ready = 1;           // finish flag
        break;

      case kwTYPE_VAR:       // the parameter is a variable
        ofs = prog_ip;       // keep expression's IP
        if (code_isvar()) {  // this parameter is a single variable (it is not an expression)
          param.type = kwTYPE_VAR;
          param.x.param.res = code_getvarptr(); // var_t pointer; the variable itself
          param.x.param.vcheck = 0x3; // parameter can be used 'by value' or 'by reference'
          code_push(&param); // push parameter
          pcount++;
          break;             // we finished with this parameter
        }

        prog_ip = ofs;       // back to the start of the expression
        // now we are sure, this parameter is not a single variable
        // no 'break' here

      default:
        // default: the parameter is an expression
        arg = v_new();       // create a new temporary variable; it is the
                             // by-val value 'arg' will be freed at udp's return
        eval(arg);           // execute the expression and store the result to 'arg'

        if (!prog_error) {
          param.type = kwTYPE_VAR;
          param.x.param.res = arg;  // var_t pointer; the variable itself
          param.x.param.vcheck = 1; // parameter can be used only as 'by value'
          code_push(&param); // push parameter
          pcount++;
        } else {             // error; clean up and return
          v_free(arg);
          v_detach(arg);
          return 0;
        }
      }
    } while (!ready);
  }

  // store call-info
  param.type = cmd;          // type of call (procedure or function)
  param.x.vcall.pcount = pcount;  // number parameter-nodes in the stack
  param.x.vcall.ret_ip = prog_ip; // where to go after exit (caller's next address)
  param.x.vcall.rvid = rvid; // return-variable ID

  if (rvid != INVALID_ADDR) {
    // if we call a function
    param.x.vcall.retvar = tvar[rvid];  // store previous data of RVID
    tvar[rvid] = v_new();    // create a temporary variable to store the function's result
                             // value will be restored on udp-return
  }

  param.x.vcall.task_id = -1;
  code_push(&param);         // store it to stack

  return goto_addr;
}

void cmd_udp(int cmd) {
  bcip_t goto_addr = code_getaddr();
  bcip_t rvid = code_getaddr();
  prog_ip = cmd_push_args(cmd, goto_addr, rvid);
}

/**
 * Call a user-defined procedure or function OF ANOTHER UNIT
 *
 * @param cmd is the type of the udp (function or procedure)
 * @param udp_tid is the UDP's task-id
 * @param goto_addr address of UDP
 * @param rvid return-var-id on callers task (this task)
 */
void cmd_call_unit_udp(int cmd, int udp_tid, bcip_t goto_addr, bcip_t rvid) {
  stknode_t param;
  bcip_t ofs;
  var_t *arg = NULL;
  bcip_t pcount = 0;
  int my_tid = ctask->tid;

  if (code_peek() == kwTYPE_LEVEL_BEGIN) {
    code_skipnext();         // kwTYPE_LEVEL_BEGIN (which means left-parenthesis)

    byte ready = 0;
    do {
      byte code = code_peek();    // get next BC
      switch (code) {
      case kwTYPE_EOC:       // end of an expression (parameter)
        code_skipnext();     // ignore it
        break;
      case kwTYPE_SEP:       // separator (comma or semi-colon)
        code_skipsep();      // ignore it
        break;
      case kwTYPE_LEVEL_END: // (right-parenthesis) which means: end of parameters
        code_skipnext();
        ready = 1;           // finish flag
        break;
      case kwTYPE_VAR:       // the parameter is a variable
        ofs = prog_ip;       // keep expression's IP

        if (code_isvar()) {  // this parameter is a single variable (not an expression)
          param.type = kwTYPE_VAR;
          param.x.param.res = code_getvarptr();
          // var_t pointer; the variable itself

          param.x.param.vcheck = 0x3; // parameter can be used 'by value' or 'by reference'

          activate_task(udp_tid);
          code_push(&param); // push parameter, on unit's task
          activate_task(my_tid);

          pcount++;
          break;             // we finished with this parameter
        }

        prog_ip = ofs;       // back to the start of the expression
        // now we are sure, this parameter is not a single variable
        // no 'break' here

      default:
        // default: the parameter is an expression
        arg = v_new();       // create a new temporary variable; it is the by-val value
                             // 'arg' will be freed at udp's return
        eval(arg);           // execute the expression and store the result to 'arg'

        if (!prog_error) {
          param.type = kwTYPE_VAR;
          param.x.param.res = arg;  // var_t pointer; the variable itself
          param.x.param.vcheck = 1; // parameter can be used only as 'by value'
          activate_task(udp_tid);
          code_push(&param);    // push parameter, on unit's task
          activate_task(my_tid);
          pcount++;
        } else {             // error; clean up and return
          v_free(arg);
          v_detach(arg);
          return;
        }
      }
    } while (!ready);
  }

  if (prog_error) {
    return;
  }
  // store call-info
  activate_task(udp_tid);
  if (prog_error) {
    return;
  }
  param.type = cmd;          // type of call (procedure or function)
  param.x.vcall.pcount = pcount;  // number of parameters which passed on
  // (the number of parameter-nodes in the stack)
  param.x.vcall.ret_ip = prog_ip; // where to go after exit (caller's next address)
  param.x.vcall.rvid = rvid; // return-variable ID

  if (rvid != INVALID_ADDR) {// if we call a function
    param.x.vcall.retvar = tvar[rvid];  // store previous data of RVID
    tvar[rvid] = v_new();    // create a temporary variable to store the
    // function's result value will be restored on udp-return
  }

  param.x.vcall.task_id = my_tid;
  code_push(&param);         // store it to stack, on unit's task
  prog_ip = goto_addr + ADDRSZ + 3; // jump to udp's code
}

/**
 * Create dynamic-variables (actually local-variables)
 */
void cmd_crvar() {
  int i;

  // number of variables to create
  int count = code_getnext();
  for (i = 0; i < count; i++) {
    // an ID on global-variable-table is used
    bcip_t vid = code_getaddr();
    stknode_t node;

    // store previous variable to stack
    // we will restore it at 'return'
    node.type = kwTYPE_CRVAR;
    node.x.vdvar.vid = vid;
    node.x.vdvar.vptr = tvar[vid];
    code_push(&node);

    // create a new variable with the same ID
    tvar[vid] = v_new();
  }
}

/**
 * user defined procedure or function - parse parameters code
 *
 * this code will be called by udp/f to pop parameter node
 * which was stored in stack by the cmd_udp (call to udp/f)
 *
 * 'by value' parameters are stored as local variables in the stack (kwTYPE_CRVAR)
 * 'by reference' parameters are stored as local variables in the stack (kwTYPE_BYREF)
 */
void cmd_param() {
  // get caller's info-node
  stknode_t *ncall = &prog_stack[prog_stack_count - 1];

  if (ncall->type != kwPROC && ncall->type != kwFUNC) {
    err_stackmess();
    return;
  }

  int pcount = code_getnext();
  if (pcount != ncall->x.vcall.pcount) {
    // the number of the parameters that are required by this procedure/function
    // are different from the number that was passed by the caller
    err_parm_num(ncall->x.vcall.pcount, pcount);
    return;
  }

  if (pcount) {
    int i;
    for (i = 0; i < pcount; i++) {
      // check parameters one-by-one
      byte vattr = code_getnext();
      bid_t vid = code_getaddr();
      int stack_pos = (prog_stack_count - 1 - pcount) + i;
      stknode_t *node = &prog_stack[stack_pos];
      var_t *param_var = node->x.param.res;

      if ((vattr & 0x80) == 0) {
        // UDP requires a 'by value' parameter
        node->type = kwTYPE_CRVAR;
        node->x.vdvar.vid = vid;
        node->x.vdvar.vptr = tvar[vid];

        // assign
        if (node->x.param.vcheck == 1) {
          // its already cloned by the CALL (expr)
          tvar[vid] = param_var;
        } else {
          tvar[vid] = v_clone(param_var);
        }
      } else if (node->x.param.vcheck == 1) {
        // error - the parameter can be used only 'by value'
        err_parm_byref(i);
        break;
      } else {
        // UDP requires 'by reference' parameter
        node->type = kwBYREF;
        node->x.vdvar.vid = vid;
        node->x.vdvar.vptr = tvar[vid];
        tvar[vid] = param_var;
      }
    }
  }
}

/**
 * Return from user-defined procedure or function
 */
void cmd_udpret() {
  stknode_t ncall;
  code_pop(&ncall, 0);

  // handle any values set with cmd_crvar()
  while (ncall.type == kwTYPE_CRVAR) {
    v_free(tvar[ncall.x.vdvar.vid]);
    v_detach(tvar[ncall.x.vdvar.vid]);
    tvar[ncall.x.vdvar.vid] = ncall.x.vdvar.vptr;
    code_pop(&ncall, 0);
  }

  // next node should be the call node
  if (ncall.type != kwPROC && ncall.type != kwFUNC) {
    rt_raise(ERR_SYNTAX);
    dump_stack();
    return;
  }

  // handle parameters
  int i;
  for (i = ncall.x.vcall.pcount; i > 0 && !prog_error; i--) {
    stknode_t node;
    code_pop(&node, 0);

    // local variable - cleanup
    if (node.type == kwTYPE_CRVAR) {
      // free local variable data
      v_free(tvar[node.x.vdvar.vid]);
      v_detach(tvar[node.x.vdvar.vid]);
      // restore ptr (replace to pre-call variable)
      tvar[node.x.vdvar.vid] = node.x.vdvar.vptr;
    } else if (node.type == kwBYREF) {
      // variable 'by reference', restore ptr
      tvar[node.x.vdvar.vid] = node.x.vdvar.vptr;
    }
  }

  // restore return value
  if (ncall.x.vcall.rvid != (bid_t) INVALID_ADDR) {
    // it is a function store value to stack
    stknode_t rval;
    rval.type = kwTYPE_RET;
    rval.x.vdvar.vptr = tvar[ncall.x.vcall.rvid];
    code_push(&rval);
    // restore ptr
    tvar[ncall.x.vcall.rvid] = ncall.x.vcall.retvar;
  }

  // jump to caller's next address
  prog_ip = ncall.x.vcall.ret_ip;
}

/**
 * EXIT [FOR|LOOP|FUNC|PROC]
 */
int cmd_exit() {
  stknode_t node;
  int ready = 0, exit_from_udp = 0;
  bcip_t addr = INVALID_ADDR;
  code_t code;

  code = code_getnext();
  do {
    code_pop(&node, 0);
    if (prog_error) {
      return 0;
    }
    switch (node.type) {
    case kwIF:
      break;
    case kwGOSUB:
      if (code == 0) {
        addr = node.x.vgosub.ret_ip;
        ready = 1;
      }
      break;
    case kwFOR:
      if (code == 0 || code == kwFORSEP) {
        addr = node.exit_ip;
        ready = 1;
        if (node.x.vfor.subtype == kwIN) {
          if (node.x.vfor.flags & 1) {  // allocated in for
            v_free(node.x.vfor.arr_ptr);
            v_detach(node.x.vfor.arr_ptr);
          }
        }
      }
      break;
    case kwWHILE:
      if (code == 0 || code == kwLOOPSEP) {
        addr = node.exit_ip;
        ready = 1;
      }
      break;
    case kwREPEAT:
      if (code == 0 || code == kwLOOPSEP) {
        addr = node.exit_ip;
        ready = 1;
      }
      break;
    case kwSELECT:
      // exiting loop from within select statement
      v_free(node.x.vcase.var_ptr);
      v_detach(node.x.vcase.var_ptr);
      break;
    case kwPROC:
    case kwFUNC:
    case kwTYPE_CRVAR:
    case kwBYREF:
    case kwTYPE_PARAM:
      if (code == 0 || code == kwPROCSEP || code == kwFUNCSEP) {
        code_push(&node);
        cmd_udpret();
        exit_from_udp = 1;
        addr = INVALID_ADDR;
        ready = 1;
      } else {
        if (code == kwFORSEP) {
          rt_raise(ERR_EXITFOR);
        } else {
          rt_raise(ERR_EXITLOOP);
        }
      }
      break;
    };
  } while (ready == 0);

  if (addr != INVALID_ADDR) {
    code_jump(addr);
  }
  return exit_from_udp;
}

/**
 * RETURN
 */
void cmd_return() {
  stknode_t node;

  // get return-address and remove any other item (sub items) from stack
  code_pop(&node, kwGOSUB);

  // 'GOTO'
  while (node.type != kwGOSUB) {
    code_pop(&node, kwGOSUB);
    if (prog_error) {
      return;
    }
  }

  if (node.type != kwGOSUB) {
    rt_raise(ERR_SYNTAX);
    dump_stack();
  }

  code_jump(node.x.vgosub.ret_ip);
}

/**
 * IF expr [THEN]
 */
void cmd_if() {
  bcip_t true_ip, false_ip;
  var_t var;
  stknode_t node;

  true_ip = code_getaddr();
  false_ip = code_getaddr();

  // expression
  v_init(&var);
  eval(&var);

  node.type = kwIF;
  node.x.vif.lcond = v_is_nonzero(&var);
  code_jump((node.x.vif.lcond) ? true_ip : false_ip);
  v_free(&var);

  code_push(&node);
}

/**
 * ELSE
 */
void cmd_else() {
  bcip_t true_ip, false_ip;
  stknode_t node;

  true_ip = code_getaddr();
  false_ip = code_getaddr();

  code_pop(&node, kwIF);

  // 'GOTO'
  while (node.type != kwIF) {
    code_pop(&node, kwIF);
    if (prog_error) {
      return;
    }
  }

  if (node.type != kwIF) {
    rt_raise(ERR_SYNTAX);
    dump_stack();
    return;
  }
  code_push(&node);

  code_jump((!node.x.vif.lcond) ? true_ip : false_ip);
}

/**
 * ELIF
 */
void cmd_elif() {
  bcip_t true_ip, false_ip;
  var_t var;
  stknode_t node;

  true_ip = code_getaddr();
  false_ip = code_getaddr();

  // else cond
  code_pop(&node, kwIF);

  // 'GOTO'
  while (node.type != kwIF) {
    code_pop(&node, kwIF);
    if (prog_error) {
      return;
    }
  }

  if (node.type != kwIF) {
    rt_raise(ERR_SYNTAX);
    dump_stack();
    return;
  }

  if (!node.x.vif.lcond) {
    // previous IF failed

    // expression
    v_init(&var);
    eval(&var);
    node.x.vif.lcond = v_is_nonzero(&var);
    code_jump((node.x.vif.lcond) ? true_ip : false_ip);
    v_free(&var);

    code_push(&node);
  } else {
    // previous IF succeded
    code_push(&node);
    code_jump(false_ip);
  }
}

void cmd_endif() {
  stknode_t node;

  code_pop(&node, kwIF);
  while (node.type != kwIF && !prog_error) {
    code_pop(&node, kwIF);
  }

  if (!prog_error) {
    prog_ip += (ADDRSZ + ADDRSZ);
  }
}

/**
 * FOR var = expr TO expr [STEP expr]
 */
void cmd_for() {
  byte code;
  bcip_t true_ip, false_ip;
  stknode_t node;
  var_t var, varstep;
  var_p_t array_p, var_p;

  true_ip = code_getaddr();
  false_ip = code_getaddr();

  code = code_peek();

  node.type = kwFOR;
  node.x.vfor.subtype = kwTO;
  node.exit_ip = false_ip + ADDRSZ + ADDRSZ + 1;
  node.x.vfor.jump_ip = true_ip;

  //
  // get FOR-variable
  //
  var_p = code_getvarptr();
  if (prog_error) {
    return;
  }
  node.x.vfor.var_ptr = var_p;
  v_free(var_p);

  v_init(&var);
  v_init(&varstep);

  // FOR-EACH or FOR-TO
  if (code_peek() != kwIN) {
    //
    // FOR v1=exp1 TO exp2 [STEP exp3]
    //

    // get the first expression
    eval(&var);
    if (!prog_error && (var.type == V_NUM || var.type == V_INT)) {
      //
      // assign FOR-variable
      //
      v_set(var_p, &var);

      if (code_getnext() == kwTO) {
        //
        // get TO-expression
        //
        node.x.vfor.to_expr_ip = prog_ip;
        v_init(&var);
        eval(&var);

        if (!prog_error && (var.type == V_NUM || var.type == V_INT)) {
          //
          // step
          //
          code = code_peek();
          if (code == kwSTEP) {
            code_skipnext();
            node.x.vfor.step_expr_ip = prog_ip;
            eval(&varstep);
            if (!(varstep.type == V_NUM || varstep.type == V_INT)) {
              if (!prog_error) {
                err_syntax(kwFOR, "%N");
              }
            }
          } else {
            node.x.vfor.step_expr_ip = INVALID_ADDR;
            varstep.type = V_INT;
            varstep.v.i = 1;
          }
        } else {
          if (!prog_error) {
            rt_raise(ERR_SYNTAX);
          }
        }
      } else {
        rt_raise(ERR_SYNTAX);
      }
    }
    //
    // run
    //
    if (!prog_error) {
      // var_p=FROM, var=TO
      int sign = v_sign(&varstep);
      int cmp = v_compare(var_p, &var);
      if (sign != 0) {
        bcip_t next_ip;
        if (sign < 0) {
          next_ip = cmp >= 0 ? true_ip : false_ip;
        } else {
          next_ip = cmp <= 0 ? true_ip : false_ip;
        }
        code_jump(next_ip);
        if (next_ip == false_ip) {
          // skip to after kwNEXT
          code_skipnext();
          code_jump(code_getaddr());
        } else {
          code_push(&node);
        }
      } else {
        rt_raise(ERR_SYNTAX);
      }
    }
  } else {
    //
    // FOR [EACH] v1 IN v2
    //
    code = code_getnext();      // kwIN
    node.x.vfor.subtype = kwIN;
    node.x.vfor.to_expr_ip = prog_ip;
    node.x.vfor.flags = 0;
    if (code_isvar()) {
      // array variable
      node.x.vfor.arr_ptr = array_p = code_getvarptr();
    } else {
      // expression
      var_t *new_var;

      new_var = v_new();
      eval(new_var);
      if (prog_error) {
        v_detach(new_var);
        return;
      }
      if (new_var->type != V_ARRAY) {
        v_free(new_var);
        v_detach(new_var);
        err_typemismatch();
        return;
      }

      node.x.vfor.flags = 1;    // allocated here
      node.x.vfor.arr_ptr = array_p = new_var;
    }

    if (!prog_error) {
      node.x.vfor.step_expr_ip = 0; // element-index

      var_p_t var_elem_ptr = 0;

      switch (array_p->type) {
      case V_MAP:
        var_elem_ptr = map_elem_key(array_p, 0);
        break;

      case V_ARRAY:
        if (array_p->v.a.size > 0) {
          var_elem_ptr = v_elem(array_p, 0);
        }
        break;

      default:
        break;
      }

      if (var_elem_ptr) {
        v_set(var_p, var_elem_ptr);
        code_jump(true_ip);
      } else {
        code_jump(false_ip);
      }

      code_push(&node);
    }
  }

  // clean up
  v_free(&var);
  v_free(&varstep);
}

/**
 * WHILE expr
 */
void cmd_while() {
  bcip_t true_ip, false_ip;
  var_t var;
  stknode_t node;

  true_ip = code_getaddr();
  false_ip = code_getaddr();

  // expression
  v_init(&var);
  eval(&var);
  if (v_sign(&var)) {
    code_jump(true_ip);
    node.type = kwWHILE;
    node.exit_ip = false_ip + ADDRSZ + ADDRSZ + 1;
    code_push(&node);
  } else {
    code_jump(false_ip + ADDRSZ + ADDRSZ + 1);
  }
  v_free(&var);
}

/**
 * WEND
 */
void cmd_wend() {
  stknode_t node;
  bcip_t jump_ip;

  code_skipaddr();
  jump_ip = code_getaddr();
  code_jump(jump_ip);
  code_pop(&node, kwWHILE);
}

/**
 * REPEAT ... UNTIL
 */
void cmd_repeat() {
  stknode_t node;
  bcip_t next_ip;

  node.type = kwREPEAT;
  code_skipaddr();
  next_ip = (code_getaddr()) + 1;
  node.exit_ip = code_peekaddr(next_ip);
  code_push(&node);
}

/**
 * UNTIL expr
 */
void cmd_until() {
  bcip_t jump_ip;
  var_t var;
  stknode_t node;

  code_pop(&node, kwREPEAT);
  code_skipaddr();
  jump_ip = code_getaddr();

  // expression
  v_init(&var);
  eval(&var);
  if (!v_sign(&var)) {
    code_jump(jump_ip);
  }
  v_free(&var);
}

/**
 * NEXT
 */
void cmd_next() {
  bcip_t next_ip, jump_ip;
  var_t var_to, var_step, *var_p, *array_p;
  var_t *var_elem_ptr;
  int check = 0;
  stknode_t node;

  next_ip = code_getaddr();
  jump_ip = code_getaddr();

  code_pop(&node, kwFOR);

  // 'GOTO'
  while (node.type != kwFOR) {
    code_pop(&node, kwFOR);
    if (prog_error) {
      return;
    }
  }

  if (node.type != kwFOR) {
    rt_raise(ERR_SYNTAX);
    dump_stack();
    return;
  }
  jump_ip = node.x.vfor.jump_ip;

  var_p = node.x.vfor.var_ptr;
  var_step.const_flag = 0;
  var_step.type = V_INT;
  var_step.v.i = 1;

  if (node.x.vfor.subtype == kwTO) {
    //
    // FOR v=exp1 TO exp2 [STEP exp3]
    //

    prog_ip = node.x.vfor.to_expr_ip;
    v_init(&var_to);
    eval(&var_to);

    if (!prog_error && (var_to.type == V_INT || var_to.type == V_NUM)) {
      // get step val
      if (node.x.vfor.step_expr_ip != INVALID_ADDR) {
        prog_ip = node.x.vfor.step_expr_ip;
        eval(&var_step);
      }

      if (!prog_error && (var_step.type == V_INT || var_step.type == V_NUM)) {
        v_inc(var_p, &var_step);
        if (v_sign(&var_step) < 0) {
          check = (v_compare(var_p, &var_to) >= 0);
        } else {
          check = (v_compare(var_p, &var_to) <= 0);
        }
      } else {
        if (!prog_error) {
          err_typemismatch();
        }
      }
    } else {
      if (!prog_error) {
        rt_raise("FOR-TO: TO v IS NOT A NUMBER");
      }
    }

    //
    // RUN
    //
    if (!prog_error) {
      if (check) {
        code_push(&node);
        code_jump(jump_ip);
      } else {
        code_jump(next_ip);
      }
    }
    v_free(&var_to);
  } else {
    //
    // FOR [EACH] v1 IN v2
    //
    array_p = node.x.vfor.arr_ptr;
    var_elem_ptr = 0;

    switch (array_p->type) {
    case V_MAP:
      node.x.vfor.step_expr_ip++; // element-index
      var_elem_ptr = map_elem_key(array_p, node.x.vfor.step_expr_ip);
      break;

    case V_ARRAY:
      node.x.vfor.step_expr_ip++; // element-index

      if (array_p->v.a.size > (int) node.x.vfor.step_expr_ip) {
        var_elem_ptr = v_elem(array_p, node.x.vfor.step_expr_ip);
      } else {
        if (node.x.vfor.flags & 1) {  // allocated in for
          v_free(node.x.vfor.arr_ptr);
          v_detach(node.x.vfor.arr_ptr);
        }
      }
      break;

    default:
      // if ( !prog_error ) rt_raise("FOR-IN: IN var IS NOT ARRAY");
      if (node.x.vfor.flags & 1) {  // allocated in for
        v_free(node.x.vfor.arr_ptr);
        v_detach(node.x.vfor.arr_ptr);
      }
      break;
    }

    if (var_elem_ptr) {
      v_set(var_p, var_elem_ptr);
      code_push(&node);
      code_jump(jump_ip);
    } else {
      code_jump(next_ip);
    }
  }

  // clean up
  v_free(&var_step);
}

/**
 * READ var [, var [, ...]]
 */
void cmd_read() {
  byte code, exitf = 0;
  var_t *vp = NULL;

  if (prog_dp == INVALID_ADDR) {
    rt_raise(ERR_READ_DATA_START);
    return;
  }

  do {
    code = code_peek();
    switch (code) {
    case kwTYPE_LINE:
    case kwTYPE_EOC:
      exitf = 1;
      break;
    case kwTYPE_SEP:
      code_skipsep();           // get 2 bytes
      break;
    default:
      vp = code_getvarptr();
      if (prog_error) {
        return;
      }
      if (!prog_error) {
        v_free(vp);

        if (prog_dp >= prog_length) {
          rt_raise(ERR_READ_DATA_INDEX);
          return;
        }

        switch (prog_source[prog_dp]) {
        case kwTYPE_EOC:       // null data
          vp->type = V_INT;
          vp->v.i = 0;
          break;
        case kwTYPE_INT:
          prog_dp++;
          vp->type = V_INT;
          memcpy(&vp->v.i, prog_source + prog_dp,OS_INTSZ);
          prog_dp += OS_INTSZ;
          break;
        case kwTYPE_NUM:
          prog_dp++;
          vp->type = V_NUM;
          memcpy(&vp->v.n, &prog_source[prog_dp], OS_REALSZ);
          prog_dp += OS_REALSZ;
          break;
        case kwTYPE_STR: {
          uint32_t len;
          prog_dp++;
          vp->type = V_STR;
          memcpy(&len, prog_source + prog_dp, OS_STRLEN);
          prog_dp += OS_STRLEN;

          vp->v.p.ptr = malloc(len + 1);
          memcpy(vp->v.p.ptr, prog_source + prog_dp, len);
          *((char *) (vp->v.p.ptr + len)) = '\0';
          vp->v.p.length = len;
          prog_dp += len;
        }
          break;
        default:
          rt_raise(ERR_READ_DATA_INDEX_FMT, prog_dp);
          return;
        }
        if (prog_source[prog_dp] == kwTYPE_EOC) {
          prog_dp++;
        }
      }
    };
  } while (!exitf);
}

/**
 * DATA ...
 */
void cmd_data() {
  rt_raise("CANNOT EXECUTE DATA");
}

/**
 * RESTORE label
 */
void cmd_restore() {
  prog_dp = code_getaddr();
}

/**
 * RANDOMIZE [num]
 */
void cmd_randomize() {
  long seed;

  byte code = code_peek();
  switch (code) {
  case kwTYPE_LINE:
  case kwTYPE_EOC:
    srand(clock());
    break;
  default:
    seed = par_getint();
    if (!prog_error) {
      srand(seed);
    }
  };
}

/**
 * DELAY
 */
void cmd_delay() {
  uint32_t ms = par_getint();
  if (prog_error) {
    return;
  }
  dev_delay(ms);
}

/**
 * AT x,y
 */
void cmd_at() {
  var_int_t x, y;

  par_massget("II", &x, &y);
  if (!prog_error) {
    dev_setxy(x, y, 1);
  }
}

/**
 * LOCATE y,x
 */
void cmd_locate() {
  var_int_t x, y;

  par_massget("II", &y, &x);
  if (prog_error) {
    return;
  }
  if (os_graphics) {
    dev_setxy(x * dev_textwidth("0"), y * dev_textheight("0"), 0);
  } else {
    dev_setxy(x, y, 0);
  }
}

/**
 * PAUSE [secs]
 */
void cmd_pause() {
  int evc;
  long start, now;

  var_int_t x = par_getval(0);
  dev_clrkb();
  if (x == 0) {
    while (dev_kbhit() == 0) {
      switch (dev_events(2)) {
      case 0:                  // no event
        break;
      case -2:                 // break
        brun_break();
      case -1:                 // break
        return;
      }
      dev_delay(CMD_PAUSE_DELAY);
    }
    dev_getch();
  } else if (x < 0) {
    dev_events(1);
  } else {
    struct tm tms;
    time_t timenow;
    time(&timenow);
    tms = *localtime(&timenow);
    start = tms.tm_hour * 3600L + tms.tm_min * 60L + tms.tm_sec;

    while (!dev_kbhit()) {
      switch ((evc = dev_events(0))) {
      case 0:                  // no event
        break;
      case -2:                 // break
        brun_break();
      case -1:                 // break
        return;
      }

      if (evc) {
        if (dev_kbhit()) {
          dev_getch();
        }
        break;
      }
      time(&timenow);
      tms = *localtime(&timenow);
      now = tms.tm_hour * 3600L + tms.tm_min * 60L + tms.tm_sec;

      if (now >= start + x) {
        break;
      }
      dev_delay(CMD_PAUSE_DELAY);
    }

    if (dev_kbhit()) {
      dev_getch();
    }
  }
}

/**
 * COLOR fg[,text_bg]
 */
void cmd_color() {
  int fg, bg = -1;

  fg = par_getint();
  if (prog_error) {
    return;
  }
  if (code_peek() == kwTYPE_SEP) {
    par_getcomma();
    if (prog_error) {
      return;
    }
    bg = par_getint();
    if (prog_error) {
      return;
    }
  }

  dev_settextcolor(fg, bg);
}

void cmd_split() {
  cmd_wsplit();
}

/**
 * SPLIT string, delimiters, array() [, pairs] [USE ...]
 */
void cmd_wsplit() {
  int count, i, wait_q;
  char *p, *ps, *new_text, *z;
  var_t *var_p, *elem_p;
  bcip_t use_ip, exit_ip = INVALID_ADDR;
  char *str = NULL, *del = NULL, *pairs = NULL;

  par_massget("SSPs", &str, &del, &var_p, &pairs);

  if (!prog_error) {
    // is there a use keyword ?
    if (code_peek() == kwUSE) {
      code_skipnext();
      use_ip = code_getaddr();
      exit_ip = code_getaddr();
    } else {
      use_ip = INVALID_ADDR;
    }
    //
    if (!pairs) {
      pairs = strdup("");
    }
    v_toarray1(var_p, 1);

    // reformat
    new_text = strdup(str);
    count = 0;
    wait_q = 0;
    ps = p = new_text;
    while (*p) {
      if (wait_q == *p) {
        wait_q = 0;
        p++;
      } else if (wait_q == 0 && (z = strchr(pairs, *p)) != NULL) {
        int open_q = ((z - pairs) + 1) % 2;
        if (open_q) {
          wait_q = *(z + 1);
        } else {
          wait_q = 0;
        }
        p++;
      } else if (wait_q == 0 && strchr(del, *p)) {
        *p = '\0';

        // add element (ps)
        if (var_p->v.a.size <= count) {
          // resize array
          v_resize_array(var_p, count + 16);
        }
        // store string
        elem_p = v_elem(var_p, count);
        v_setstr(elem_p, ps);
        count++;

        // next word
        p++;
        ps = p;
      } else {
        p++;
      }
    }

    // add the last element (ps)
    if (v_asize(var_p) <= count) {
      v_resize_array(var_p, count + 1);
    }
    elem_p = v_elem(var_p, count);
    if (*ps) {
      v_setstr(elem_p, ps);
    } else {
      v_setstr(elem_p, "");
    }
    count++;

    // final resize
    v_resize_array(var_p, count);

    // execute user's expression for each element
    if (use_ip != INVALID_ADDR) {
      for (i = 0; i < v_asize(var_p) && !prog_error; i++) {
        elem_p = v_elem(var_p, i);
        exec_usefunc(elem_p, use_ip);
      }

      // jmp to correct location
      code_jump(exit_ip);
    }
    // cleanup
    pfree3(str, del, pairs);
    free(new_text);
  }
}

/**
 * JOIN array(), delimiter, dest-var
 */
void cmd_wjoin() {
  var_t del, *var_p;;

  v_init(&del);
  var_p = par_getvarray();
  if (!var_p || var_p->type != V_ARRAY) {
    err_varisnotarray();
    return;
  }
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }
  par_getstr(&del);
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    v_free(&del);
    return;
  }
  if (!code_isvar()) {
    err_syntax(kwWJOIN, "%P,%P");
    v_free(&del);
    return;
  }

  var_t *str = code_getvarptr();
  int size = STR_INIT_SIZE;
  int len = 0;
  int del_len = v_strlen(&del);
  int i;

  v_free(str);
  str->type = V_STR;
  str->v.p.ptr = malloc(size);
  str->v.p.ptr[0] = '\0';

  for (i = 0; i < var_p->v.a.size; i++) {
    var_t *elem_p = v_elem(var_p, i);
    var_t e_str;

    v_init(&e_str);
    v_set(&e_str, elem_p);
    if (e_str.type != V_STR) {
      v_tostr(&e_str);
    }

    int el_len = v_strlen(&e_str);
    if (el_len + del_len + 1 >= (size - len)) {
      size += el_len + del_len + STR_INIT_SIZE;
      str->v.p.ptr = realloc(str->v.p.ptr, size);
    }

    len += el_len;
    strcat((char *)str->v.p.ptr, (char *)e_str.v.p.ptr);
    v_free(&e_str);

    if (i != var_p->v.p.length - 1) {
      strcat((char *)str->v.p.ptr, (char *)del.v.p.ptr);
      len += del_len;
    }
  }

  str->v.p.length = len;

  // cleanup
  v_free(&del);
}

/**
 * ENVIRON string
 */
void cmd_environ() {
  var_t str;

  par_getstr(&str);
  if (prog_error) {
    return;
  }
  char *eq = strchr(str.v.p.ptr, '=');
  if (eq == NULL) {
    rt_raise(ERR_PUTENV);
  } else {
    *eq = '\0';
    if (dev_setenv(str.v.p.ptr, eq + 1) == -1) {
      rt_raise(ERR_PUTENV);
    }
  }
  v_free(&str);
}

/**
 * DATEDMY string|julian, m, d, y
 */
void cmd_datedmy() {
  long d, m, y;
  var_t arg, *vd, *vm, *vy;

  v_init(&arg);
  eval(&arg);

  if (arg.type == V_STR) {
    date_str2dmy((char *) arg.v.p.ptr, &d, &m, &y);
    v_free(&arg);
  } else {
    // julian
    d = v_igetval(&arg);
    v_free(&arg);
    date_jul2dmy(d, &d, &m, &y);
  }

  // byref pars
  par_getcomma();
  if (prog_error) {
    return;
  }
  vd = par_getvar_ptr();
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }
  vm = par_getvar_ptr();
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }
  vy = par_getvar_ptr();
  if (prog_error) {
    return;
  }
  v_free(vd);
  v_free(vm);
  v_free(vy);

  vd->type = V_INT;
  vd->v.i = d;
  vm->type = V_INT;
  vm->v.i = m;
  vy->type = V_INT;
  vy->v.i = y;
}

/**
 * TIMEHMS string|timer, h, m, s
 */
void cmd_timehms() {
  long h, m, s;
  var_t arg, *vh, *vm, *vs;

  v_init(&arg);
  eval(&arg);

  if (arg.type == V_STR) {
    // string
    date_str2hms((char *) arg.v.p.ptr, &h, &m, &s);
    v_free(&arg);
  } else {
    // timer
    h = v_igetval(&arg);
    v_free(&arg);
    date_tim2hms(h, &h, &m, &s);
  }

  // byref pars
  par_getcomma();
  if (prog_error) {
    return;
  }
  vh = par_getvar_ptr();
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }
  vm = par_getvar_ptr();
  if (prog_error) {
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }
  vs = par_getvar_ptr();
  if (prog_error) {
    return;
  }
  v_free(vh);
  v_free(vm);
  v_free(vs);

  vh->type = V_INT;
  vh->v.i = h;
  vm->type = V_INT;
  vm->v.i = m;
  vs->type = V_INT;
  vs->v.i = s;
}

/**
 * SORT array [USE ...]
 */
int sb_qcmp(var_t *a, var_t *b, bcip_t use_ip) {
  if (use_ip == INVALID_ADDR) {
    return v_compare(a, b);
  } else {
    var_t v1, v2;
    int r;

    v_init(&v1);
    v_init(&v2);

    v_set(&v1, a);
    v_set(&v2, b);
    exec_usefunc2(&v1, &v2, use_ip);
    r = v_igetval(&v1);
    v_free(&v1);
    return r;
  }
}

// using C's qsort()
static bcip_t static_qsort_last_use_ip;

int qs_cmp(const void *a, const void *b) {
  var_t *ea = (var_t *)a;
  var_t *eb = (var_t *)b;
  return sb_qcmp(ea, eb, static_qsort_last_use_ip);
}

void cmd_sort() {
  bcip_t use_ip, exit_ip;
  var_t *var_p;
  int errf = 0;

  if (code_isvar()) {
    var_p = code_getvarptr();
    if (var_p->type != V_ARRAY) {
      errf = 1;
    }
  } else {
    err_typemismatch();
    return;
  }

  // USE
  if (code_peek() == kwUSE) {
    code_skipnext();
    use_ip = code_getaddr();
    exit_ip = code_getaddr();
  } else {
    use_ip = exit_ip = INVALID_ADDR;
  }
  // sort
  if (!errf) {
    if (var_p->v.a.size > 1) {
      static_qsort_last_use_ip = use_ip;
      qsort(var_p->v.a.data, var_p->v.a.size, sizeof(var_t), qs_cmp);
    }
  }
  // NO RTE anymore... there is no meaning on this because of empty
  // arrays/variables (example: TLOAD "data", V:SORT V)
  if (exit_ip != INVALID_ADDR) {
    code_jump(exit_ip);
  }
}

/**
 * SEARCH A(), key, BYREF ridx [USE ...]
 */
void cmd_search() {
  bcip_t use_ip, exit_ip;
  int i, bcmp;
  var_t *var_p, *elem_p, *rv_p;
  var_t vkey;
  int errf = 0;

  // parameters 1: the array
  if (code_isvar()) {
    var_p = code_getvarptr();
    if (var_p->type != V_ARRAY) {
      errf = 1;
    }
  } else {
    err_typemismatch();
    return;
  }

  // parameters 2: the key
  par_getcomma();
  if (prog_error) {
    return;
  }
  v_init(&vkey);
  eval(&vkey);
  if (prog_error) {
    return;
  }
  // parameters 3: the return-variable
  par_getcomma();
  if (prog_error) {
    v_free(&vkey);
    return;
  }
  if (code_isvar()) {
    rv_p = code_getvarptr();
    v_free(rv_p);
  } else {
    v_free(&vkey);
    err_typemismatch();
    return;
  }

  // USE
  if (code_peek() == kwUSE) {
    code_skipnext();
    use_ip = code_getaddr();
    exit_ip = code_getaddr();
  } else {
    use_ip = exit_ip = INVALID_ADDR;
  }
  // search
  if (!errf) {
    rv_p->v.i = var_p->v.a.lbound[0] - 1;
    for (i = 0; i < var_p->v.a.size; i++) {
      elem_p = v_elem(var_p, i);
      bcmp = sb_qcmp(elem_p, &vkey, use_ip);
      if (bcmp == 0) {
        rv_p->v.i = i + var_p->v.a.lbound[0];
        break;
      }
    }
  }
  // NO RTE anymore... there is no meaning on this because of empty
  // arrays/variables (example: TLOAD "data", V:SEARCH V...)
  // else
  // rt_raise("SEARCH: Not an array");
  else {
    rv_p->v.i = -1;
  }
  // return
  if (exit_ip != INVALID_ADDR) {
    code_jump(exit_ip);
  }
  v_free(&vkey);
}

/**
 * SWAP a, b
 */
void cmd_swap(void) {
  var_t *va, *vb, *vc;

  if (code_isvar()) {
    va = code_getvarptr();
  } else {
    err_typemismatch();
    return;
  }
  par_getcomma();
  if (prog_error) {
    return;
  }
  if (code_isvar()) {
    vb = code_getvarptr();
  } else {
    err_typemismatch();
    return;
  }

  vc = v_new();
  v_set(vc, va);
  v_set(va, vb);
  v_set(vb, vc);
  v_free(vc);
  v_detach(vc);
}

/**
 * EXPRSEQ @array, xmin, xmax, count USE f(x)
 */
void cmd_exprseq(void) {
  var_t *var_p, *elem_p;
  bcip_t use_ip, exit_ip = INVALID_ADDR;
  var_num_t xmin, xmax, dx, x;
  var_int_t count, i;

  par_massget("PFFI", &var_p, &xmin, &xmax, &count);

  if (!prog_error) {
    // is there a use keyword ?
    if (code_peek() != kwUSE) {
      rt_raise(ERR_EXPRSEQ_WITHOUT_EXPR);
      return;
    }
    // get expr info
    code_skipnext();
    use_ip = code_getaddr();
    exit_ip = code_getaddr();

    if (count > 1) {
      v_toarray1(var_p, count);
      dx = (xmax - xmin) / (count - 1);

      // add the entries
      for (i = 0, x = xmin; i < count; i++, x += dx) {
        elem_p = v_elem(var_p, i);
        v_setreal(elem_p, x);
        exec_usefunc(elem_p, use_ip);

        if (prog_error) {
          break;
        }
      }
    } else {
      v_toarray1(var_p, 0);
    }

    code_jump(exit_ip);
  }
}

/**
 * evaluate the select expression and then store it on the stack
 * syntax is:
 * select expr
 * case expr
 * stmt
 * case expr
 * stmt
 * case else
 * default stmt
 * end select
 */
void cmd_select() {
  stknode_t node;

  var_t *expr = v_new();
  v_init(expr);
  eval(expr);

  node.x.vcase.var_ptr = expr;
  node.x.vcase.flags = 0;
  node.type = kwSELECT;
  code_push(&node);
}

/**
 * compare the case expression with the saved select expression
 * if true then branch to true_ip otherwise branch to false_ip
 * which could either be another case line or "end select"
 */
void cmd_case() {
  var_t var_p;
  bcip_t true_ip = code_getaddr();     // matching case
  bcip_t false_ip = code_getaddr();    // non-matching case

  v_init(&var_p);
  eval(&var_p);

  stknode_t *node = code_stackpeek();

  if (node->type != kwSELECT) {
    rt_raise(ERR_SYNTAX);
    return;
  }

  if (node->x.vcase.flags) {
    // previous case already matches.
    code_jump(false_ip);
  } else {
    // compare select expr with case expr
    node->x.vcase.flags = v_compare(node->x.vcase.var_ptr, &var_p) == 0 ? 1 : 0;
    while (code_peek() == kwTYPE_SEP && node->x.vcase.flags == 0) {
      // evaluate futher comma separated items until there is a match
      code_skipnext();
      if (code_getnext() != ',') {
        err_missing_comma();
        break;
      }
      var_t vp_next;
      v_init(&vp_next);
      eval(&vp_next);
      node->x.vcase.flags = v_compare(node->x.vcase.var_ptr, &vp_next) == 0 ? 1 : 0;
      v_free(&vp_next);
    }
    code_jump(node->x.vcase.flags ? true_ip : false_ip);
  }

  v_free(&var_p);
}

/**
 * skip to cmd_end_select if a previous case was true
 */
void cmd_case_else() {
  bcip_t true_ip = code_getaddr();     // default block
  bcip_t false_ip = code_getaddr();    // end-select
  stknode_t *node = code_stackpeek();
  code_jump(node->x.vcase.flags ? false_ip : true_ip);
}

/**
 * free the stored select expression created in cmd_select()
 */
void cmd_end_select() {
  stknode_t node;

  code_pop_and_free(&node);
  if (node.type != kwSELECT) {
    err_stackmess();
  } else {
    code_jump(code_getaddr());
  }
}

/**
 * define keyboard event handling
 */
void cmd_definekey(void) {
  var_t var;

  v_init(&var);
  eval(&var);

  int key = v_igetval(&var);

  if (!prog_error) {
    par_getcomma();

    if (code_peek() != kwTYPE_CALL_UDF) {
      err_syntax(kwDEFINEKEY, "%I,%G");
    } else {
      keymap_add(key, prog_ip);

      // skip ahead to avoid immediate call
      prog_ip += BC_CTRLSZ + 1;
    }
  }
  v_free(&var);
}

/**
 * Try handler for try/catch
 */
void cmd_try() {
  stknode_t node;
  node.x.vtry.catch_ip = code_getaddr();
  node.type = kwTRY;
  code_push(&node);
}

/**
 * Handler for unthrown/uncaught exceptions
 */
void cmd_catch() {
  stknode_t node;
  code_pop_and_free(&node);
  if (node.type != kwTRY) {
    err_stackmess();
  } else {
    // skip to end-try
    code_jump(code_getaddr());
  }
}

/**
 * Handler for code block between CATCH and END_TRY
 */
void cmd_end_try() {
  stknode_t *node = code_stackpeek();
  if (node != NULL && node->type == kwCATCH) {
    code_pop_and_free(node);
  }
}

/**
 * Call to object method
 */
void cmd_call_vfunc() {
  var_t *map = NULL;
  var_t *v_func = code_getvarptr_map(&map);
  if (v_func == NULL || (v_func->type != V_FUNC && v_func->type != V_PTR)) {
    rt_raise(ERR_NO_FUNC);
  } else if (v_func->type == V_PTR) {
    prog_ip = cmd_push_args(kwPROC, v_func->v.ap.p, v_func->v.ap.v);
    if (code_peek() == kwTYPE_PARAM) {
      code_skipnext();
      cmd_param();
      v_set_self(map);
      bc_loop(2);
      v_set_self(NULL);
    } else {
      rt_raise(ERR_NO_FUNC);
    }
  } else {
    if (code_peek() == kwTYPE_LEVEL_BEGIN) {
      code_skipnext();
    }
    v_func->v.fn.cb(v_func->v.fn.self);
    if (code_peek() == kwTYPE_LEVEL_END) {
      code_skipnext();
    }
  }
}

/**
 * Adds a timer
 */
void cmd_timer() {
  var_t var;
  v_init(&var);
  eval(&var);

  var_num_t interval = v_getval(&var);
  if (!prog_error) {
    par_getcomma();
    if (code_peek() != kwTYPE_CALL_UDF) {
      err_syntax(kwTIMER, "%F,%G");
    } else {
      timer_add(interval, prog_ip);
      prog_ip += BC_CTRLSZ + 1;
    }
  }
  v_free(&var);
}
