// This file is part of SmallBASIC
//
// byte-code executor
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#define BRUN_MODULE

#include "config.h"

#include "common/sys.h"
#include "common/blib.h"
#include "common/str.h"
#include "common/fmt.h"
#include "common/plugins.h"
#include "common/units.h"
#include "common/kw.h"
#include "common/var.h"
#include "common/scan.h"
#include "common/smbas.h"
#include "common/messages.h"
#include "common/device.h"
#include "common/pproc.h"
#include "common/keymap.h"

int brun_create_task(const char *filename, byte *preloaded_bc, int libf);
int exec_close_task();
void sys_before_comp();

static char fileName[OS_FILENAME_SIZE + 1];
static stknode_t err_node;

#define EVT_CHECK_EVERY 50
#define IF_ERR_BREAK if (prog_error) { \
  if (prog_error == errThrow)       \
      prog_error = errNone; else break;}

/**
 * jump to label
 */
void code_jump_label(uint16_t label_id) {
  prog_ip = tlab[label_id].ip;
}

/**
 * Push a new node onto the stack
 */
stknode_t *code_push(code_t type) {
  stknode_t *result;
  if (prog_stack_count + 1 >= prog_stack_alloc) {
    err_stackoverflow();
    result = &err_node;
  } else {
    result = &prog_stack[prog_stack_count++];
    result->type = type;
    result->line = prog_line;
  }
  return result;
}

void free_node(stknode_t *node) {
  switch (node->type) {
  case kwTYPE_CRVAR:
    // free local variable data and retore ptr
    if (node->x.vdvar.vptr != tvar[node->x.vdvar.vid]) {
      v_free(tvar[node->x.vdvar.vid]);
      v_detach(tvar[node->x.vdvar.vid]);
      tvar[node->x.vdvar.vid] = node->x.vdvar.vptr;
    }
    break;

  case kwBYREF:
    tvar[node->x.vdvar.vid] = node->x.vdvar.vptr;
    break;

  case kwTYPE_VAR:
    if ((node->x.param.vcheck == 1) || (node->x.param.vcheck == 0x81)) {
      v_free(node->x.param.res);
      v_detach(node->x.param.res);
    }
    break;

  case kwTYPE_RET:
    v_free(node->x.vdvar.vptr); // free ret-var
    v_detach(node->x.vdvar.vptr);
    break;

  case kwFUNC:
  case kwPROC:
    if (node->x.vcall.rvid != INVALID_ADDR) {
      v_detach(tvar[node->x.vcall.rvid]);
      tvar[node->x.vcall.rvid] = node->x.vcall.retvar;
    }
    break;

  case kwFOR:
    if (node->x.vfor.subtype == kwIN) {
      if (node->x.vfor.flags & 1) {
        // allocated in for
        v_free(node->x.vfor.arr_ptr);
        v_detach(node->x.vfor.arr_ptr);
      }
    }
    break;

  case kwSELECT:
    v_free(node->x.vcase.var_ptr);
    v_detach(node->x.vcase.var_ptr);
    break;

  case kwCATCH:
    if (node->x.vcatch.catch_var != NULL) {
      // clear the catch variable once out of scope
      v_free(node->x.vcatch.catch_var);
    }
    break;

  default:
    break;
  }
}

/**
 * Returns and deletes the topmost node from stack (POP)
 */
void code_pop(stknode_t *node, int expected_type) {
  if (prog_stack_count) {
    prog_stack_count--;
    if (node) {
      *node = prog_stack[prog_stack_count];
    }
    if (node != NULL && expected_type != 0 && node->type != expected_type) {
      free_node(node);
      switch (node->type) {
      case kwTYPE_RET:
        // a FUNC result was not previously consumed
        rt_raise(MSG_RETURN_NOT_ASSIGNED, node->line);
        break;
      case kwTYPE_CRVAR:
        // pop local variable and continue
        free_node(node);
        code_pop(node, expected_type);
        break;
      default:
        break;
      }
    }
  } else {
    if (node) {
      err_stackunderflow();
      node->type = 0xFF;
    }
  }
}

/**
 * POPs and frees the topmost node from stack and returns the node type
 */
int code_pop_and_free() {
  int type;
  if (prog_stack_count) {
    prog_stack_count--;
    type = prog_stack[prog_stack_count].type;
    free_node(&prog_stack[prog_stack_count]);
  } else {
    type = 0xFF;
  }
  return type;
}

/**
 * Peek the topmost node of stack
 */
stknode_t *code_stackpeek() {
  if (prog_stack_count) {
    return &prog_stack[prog_stack_count - 1];
  }
  return NULL;
}

/**
 * sets the value of an system-variable with the given type
 */
void setsysvar_var(int index, var_int_t value, int type) {
  int tid;
  int i;

  tid = ctask->tid;
  for (i = 0; i < count_tasks(); i++) {
    activate_task(i);
    if (ctask->has_sysvars) {
      var_t *var_p = tvar[index];
      var_p->type = type;
      var_p->const_flag = 1;
      var_p->v.i = value;
    }
  }
  activate_task(tid);
}

/**
 * sets the value of an integer system-variable
 */
void setsysvar_int(int index, var_int_t value) {
  setsysvar_var(index, value, V_INT);
}

/**
 * sets the value of a real system-variable
 */
void setsysvar_num(int index, var_num_t value) {
  int tid;
  int i;

  tid = ctask->tid;
  for (i = 0; i < count_tasks(); i++) {
    activate_task(i);
    if (ctask->has_sysvars) {
      var_t *var_p = tvar[index];
      var_p->type = V_NUM;
      var_p->const_flag = 1;
      var_p->v.n = value;
    }
  }
  activate_task(tid);
}

/**
 * sets the value of an string system-variable
 */
void setsysvar_str(int index, const char *value) {
  int i;
  int tid = ctask->tid;

  for (i = 0; i < count_tasks(); i++) {
    activate_task(i);
    if (ctask->has_sysvars) {
      var_t *var_p = tvar[index];
      v_free(var_p);
      v_createstr(var_p, value);
      var_p->const_flag = 1;
    }
  }
  activate_task(tid);
}

/**
 * create predefined system variables for this task
 */
void exec_setup_predefined_variables() {
  char homedir[OS_PATHNAME_SIZE + 1];
  homedir[0] = '\0';

  // needed here (otherwise task will not updated)
  ctask->has_sysvars = 1;
  setsysvar_str(SYSVAR_SBVER, SB_STR_VER);
  setsysvar_num(SYSVAR_PI, M_PI);
  setsysvar_int(SYSVAR_XMAX, os_graf_mx - 1);
  setsysvar_int(SYSVAR_YMAX, os_graf_my - 1);
  setsysvar_int(SYSVAR_TRUE, 1);
  setsysvar_int(SYSVAR_FALSE, 0);
  setsysvar_str(SYSVAR_CWD, dev_getcwd());
  setsysvar_str(SYSVAR_COMMAND, opt_command);
  setsysvar_int(SYSVAR_SELF, 0);
  setsysvar_var(SYSVAR_NONE, 0, V_NIL);
  setsysvar_int(SYSVAR_MAXINT, VAR_MAX_INT);

#if defined(_ANDROID)
  if (getenv("HOME_DIR")) {
    strlcpy(homedir, getenv("HOME_DIR"), sizeof(homedir));
  }
#elif defined(_Win32)
  if (getenv("HOMEPATH")) {
    if (getenv("HOMEDRIVE")) {
      strlcpy(homedir, getenv("HOMEDRIVE"), sizeof(homedir));
      strlcat(homedir, getenv("HOMEPATH"), sizeof(homedir));
    } else {
      strlcpy(homedir, getenv("HOMEPATH"), sizeof(homedir));
    }
  } else {
    GetModuleFileName(NULL, homedir, sizeof(homedir) - 1);
    char *p = strrchr(homedir, '\\');
    if (p) {
      *p = '\0';
    }
  }
#elif defined(_UnixOS)
  if (getenv("HOME")) {
    strlcpy(homedir, getenv("HOME"), sizeof(homedir));
  } else {
    strcpy(homedir, "/tmp/");
  }
#endif
  // don't end with trailing slash
  int l = strlen(homedir);
  if (homedir[l - 1] == OS_DIRSEP) {
    homedir[l - 1] = '\0';
  }
  setsysvar_str(SYSVAR_HOME, homedir);
}

/**
 * BREAK
 */
void brun_stop() {
  prog_error = errBreak;
}

/**
 * returns the status of executor (runing or stopped)
 */
int brun_status() {
  if (prog_error) {
    return BRUN_STOPPED;
  }
  return BRUN_RUNNING;
}

/**
 * BREAK - display message, too
 */
void brun_break() {
  if (brun_status() == BRUN_RUNNING) {
    inf_break(prog_line);
  }
  brun_stop();
}

/**
 * CHAIN sb-source
 */
void cmd_chain(void) {
  var_t var;
  char *code = NULL;

  v_init(&var);
  eval(&var);

  if (prog_error) {
    v_free(&var);
    return;
  }

  if (var.type == V_STR) {
    if (access(var.v.p.ptr, R_OK) == 0) {
      // argument is a file name
      int h = open(var.v.p.ptr, O_BINARY | O_RDONLY);
      if (h != -1) {
        struct stat st;
        if (fstat(h, &st) == 0) {
          int len = st.st_size;
          code = (char *)malloc(len + 1);
          len = read(h, code, len);
          code[len] = '\0';
        }
        close(h);
      }
    }
    if (!code) {
      code = strdup(var.v.p.ptr);
    }
  } else if (var.type == V_ARRAY) {
    int len = 0;
    uint32_t size = v_asize(&var);
    for (int el = 0; el < size; el++) {
      var_t *el_p = v_elem(&var, el);
      if (el_p->type == V_STR) {
        int str_len = strlen(el_p->v.p.ptr) + 2;
        if (len) {
          code = realloc(code, len + str_len);
          strcat(code, el_p->v.p.ptr);
        } else {
          code = malloc(str_len);
          strcpy(code, el_p->v.p.ptr);
        }
        strcat(code, "\n");
        len += str_len + 1;
      }
    }
  }

  if (code == NULL) {
    v_free(&var);
    err_typemismatch();
    return;
  }

  v_free(&var);

  int tid_base = create_task("CH_BASE");
  int tid_prev = activate_task(tid_base);

  // compile the buffer
  sys_before_comp();
  int success = comp_compile_buffer(code);

  free(code);
  code = NULL;

  if (success == 0) {
    close_task(tid_base);
    activate_task(tid_prev);
    prog_error = errCompile;
    return;
  }

  char last_file[OS_PATHNAME_SIZE + 1];
  char bas_dir[OS_PATHNAME_SIZE + 1];
  strcpy(last_file, gsb_last_file);
  strcpy(bas_dir, gsb_bas_dir);

  int tid_main = brun_create_task("CH_MAIN", ctask->bytecode, 0);
  exec_sync_variables(0);

  bc_loop(0);
  success = prog_error;         // save tid_main status
  exec_close_task();            // cleanup task data - tid_main
  close_task(tid_main);         // cleanup task container
  close_task(tid_base);         // cleanup task container
  activate_task(tid_prev);      // resume calling task

  // reset globals
  gsb_last_line = 0;
  gsb_last_error = 0;
  strcpy(gsb_last_file, last_file);
  strcpy(gsb_bas_dir, bas_dir);
  strcpy(gsb_last_errmsg, "");

  if (success == 0) {
    prog_error = errRuntime;
  }
}

/**
 * RUN "program"
 */
void cmd_run(int retf) {
  var_t var;
  v_init(&var);
  eval(&var);
  IF_ERR_RETURN;

  if (var.type != V_STR) {
    err_typemismatch();
    return;
  } else {
    strlcpy(fileName, var.v.p.ptr, sizeof(fileName));
    v_free(&var);
  }

  if (!dev_run(fileName, NULL, retf)) {
    err_run_err(fileName);
  }
}

/**
 * OPTION (run-time part) keyword
 */
void cmd_options(void) {
  byte c;
  bcip_t data;

  c = code_getnext();
  data = code_getaddr();
  switch (c) {
  case OPTION_BASE:
    opt_base = data;
    break;
  case OPTION_MATCH:
    opt_usepcre = data;
    break;
  };
}

static inline void bc_loop_call_proc() {
  bcip_t pcode = code_getaddr();
  switch (pcode) {
  case kwCLS:
    dev_cls();
    break;
  case kwTHROW:
    cmd_throw();
    break;
  case kwENVIRON:
    cmd_environ();
    break;
  case kwLOCATE:
    cmd_locate();
    break;
  case kwAT:
    cmd_at();
    break;
  case kwPEN:
    cmd_pen();
    break;
  case kwDATEDMY:
    cmd_datedmy();
    break;
  case kwTIMEHMS:
    cmd_timehms();
    break;
  case kwBEEP:
    cmd_beep();
    break;
  case kwSOUND:
    cmd_sound();
    break;
  case kwNOSOUND:
    cmd_nosound();
    break;
  case kwPSET:
    cmd_pset();
    break;
  case kwRECT:
    cmd_rect();
    break;
  case kwCIRCLE:
    cmd_circle();
    break;
  case kwRANDOMIZE:
    cmd_randomize();
    break;
  case kwSPLIT:
    cmd_split();
    break;
  case kwWJOIN:
    cmd_wjoin();
    break;
  case kwPAUSE:
    cmd_pause();
    break;
  case kwDELAY:
    cmd_delay();
    break;
  case kwARC:
    cmd_arc();
    break;
  case kwDRAW:
    cmd_draw();
    break;
  case kwPAINT:
    cmd_paint();
    break;
  case kwPLAY:
    cmd_play();
    break;
  case kwSORT:
    cmd_sort();
    break;
  case kwSEARCH:
    cmd_search();
    break;
  case kwROOT:
    cmd_root();
    break;
  case kwDIFFEQ:
    cmd_diffeq();
    break;
  case kwCHART:
    cmd_chart();
    break;
  case kwWINDOW:
    cmd_window();
    break;
  case kwVIEW:
    cmd_view();
    break;
  case kwDRAWPOLY:
    cmd_drawpoly();
    break;
  case kwM3IDENT:
    cmd_m3ident();
    break;
  case kwM3ROTATE:
    cmd_m3rotate();
    break;
  case kwM3SCALE:
    cmd_m3scale();
    break;
  case kwM3TRANSLATE:
    cmd_m3translate();
    break;
  case kwM3APPLY:
    cmd_m3apply();
    break;
  case kwSEGINTERSECT:
    cmd_intersect();
    break;
  case kwPOLYEXT:
    cmd_polyext();
    break;
  case kwDERIV:
    cmd_deriv();
    break;
  case kwLOADLN:
    cmd_floadln();
    break;
  case kwSAVELN:
    cmd_fsaveln();
    break;
  case kwKILL:
    cmd_fkill();
    break;
  case kwRENAME:
    cmd_filecp(1);
    break;
  case kwCOPY:
    cmd_filecp(0);
    break;
  case kwCHDIR:
    cmd_chdir();
    break;
  case kwMKDIR:
    cmd_mkdir();
    break;
  case kwRMDIR:
    cmd_rmdir();
    break;
  case kwFLOCK:
    cmd_flock();
    break;
  case kwCHMOD:
    cmd_chmod();
    break;
  case kwPLOT:
    cmd_plot();
    break;
  case kwSWAP:
    cmd_swap();
    break;
  case kwDIRWALK:
    cmd_dirwalk();
    break;
  case kwBPUTC:
    cmd_bputc();
    break;
  case kwBSAVE:
    cmd_bsave();
    break;
  case kwBLOAD:
    cmd_bload();
    break;
  case kwEXPRSEQ:
    cmd_exprseq();
    break;
  case kwSTKDUMP:
    dev_print("\nSTKDUMP:\n");
    dump_stack();
    // end of program
    prog_error = errEnd;
    break;
  case kwDEFINEKEY:
    cmd_definekey();
    break;
  case kwSHOWPAGE:
    dev_show_page();
    break;
  case kwTYPE_CALL_VFUNC:
    cmd_call_vfunc();
    break;
  case kwTIMER:
    cmd_timer();
    break;
  default:
    err_pcode_err(pcode);
  }

  if (!prog_error && prog_source[prog_ip] == kwTYPE_LEVEL_END) {
    // allow redundant close bracket around function call
    prog_ip++;
  }
}

static inline void bc_loop_call_extp() {
  bcip_t lib = code_getaddr();
  bcip_t idx = code_getaddr();
  if (lib & UID_UNIT_BIT) {
    unit_exec(lib & (~UID_UNIT_BIT), idx, NULL);
    if (gsb_last_error) {
      prog_error = gsb_last_error;
    }
  } else {
    plugin_procexec(lib, prog_symtable[idx].exp_idx);
  }
}

static inline void bc_loop_end() {
  // end of program
  prog_error = errEnd;
}

void bc_loop_goto() {
  bcip_t next_ip = code_getaddr();

  // clear the stack
  byte pops = code_getnext();
  while (pops > 0) {
    code_pop_and_free();
    pops--;
  }

  // jump
  prog_ip = next_ip;
}

/**
 * execute commands (loop)
 *
 * @param isf if 1, the program must return if found return (by level <= 0);
 * otherwise an RTE will generated
 * if 2; like 1, but increase the proc_level because UDF call executed internaly
 */
void bc_loop(int isf) {
  byte pops;
  int i;
  int proc_level = 0;
  byte code = 0;

  // setup event checker time = 50ms
  uint32_t now = dev_get_millisecond_count();
  uint32_t next_check = now + EVT_CHECK_EVERY;

  /**
   * For commands that change the IP use
   *
   * case mycommand:
   *   command();
   *   if  ( prog_error )  break;
   *   continue;
   */
  if (isf == 2) {
    proc_level++;
  }
  while (prog_ip < prog_length) {
    switch (code) {
    case kwLABEL:
    case kwREM:
    case kwTYPE_EOC:
    case kwTYPE_LINE:
      break;
    default:
      now = dev_get_millisecond_count();
      break;
    }

    // check events every ~50ms
    if (now >= next_check) {
      next_check = now + EVT_CHECK_EVERY;

      switch (dev_events(0)) {
      case -1:
        // break event
        break;
      case -2:
        prog_error = errBreak;
        inf_break(prog_line);
        break;
      default:
        if (prog_timer) {
          timer_run(now);
        }
      };
    }

    // proceed to the next command
    if (!prog_error) {
      code = prog_source[prog_ip++];
      switch (code) {
      case kwLABEL:
      case kwREM:
      case kwTYPE_EOC:
        continue;
      case kwTYPE_LINE:
        prog_line = code_getaddr();
        if (opt_trace_on) {
          dev_trace_line(prog_line);
        }
        continue;
      case kwLET:
        cmd_let(0);
        break;
      case kwLET_OPT:
        cmd_let_opt();
        break;
      case kwCONST:
        cmd_let(1);
        break;
      case kwPACKED_LET:
        cmd_packed_let();
        break;
      case kwGOTO:
        bc_loop_goto();
        continue;
      case kwGOSUB:
        cmd_gosub();
        IF_ERR_BREAK;
        continue;
      case kwRETURN:
        cmd_return();
        IF_ERR_BREAK;
        continue;
      case kwONJMP:
        cmd_on_go();
        IF_ERR_BREAK;
        continue;
      case kwPRINT:
        cmd_print(PV_CONSOLE);
        break;
      case kwINPUT:
        cmd_input(PV_CONSOLE);
        break;
      case kwIF:
        cmd_if();
        IF_ERR_BREAK;
        continue;
      case kwELIF:
        cmd_elif();
        IF_ERR_BREAK;
        continue;
      case kwELSE:
        cmd_else();
        IF_ERR_BREAK;
        continue;
      case kwENDIF:
        cmd_endif();
        IF_ERR_BREAK;
        continue;
      case kwFOR:
        cmd_for();
        IF_ERR_BREAK;
        continue;
      case kwNEXT:
        cmd_next();
        IF_ERR_BREAK;
        continue;
      case kwWHILE:
        cmd_while();
        IF_ERR_BREAK;
        continue;
      case kwWEND:
        cmd_wend();
        IF_ERR_BREAK;
        continue;
      case kwREPEAT:
        cmd_repeat();
        IF_ERR_BREAK;
        continue;
      case kwUNTIL:
        cmd_until();
        IF_ERR_BREAK;
        continue;
      case kwSELECT:
        cmd_select();
        IF_ERR_BREAK;
        continue;
      case kwCASE:
        cmd_case();
        IF_ERR_BREAK;
        continue;
      case kwCASE_ELSE:
        cmd_case_else();
        IF_ERR_BREAK;
        continue;
      case kwENDSELECT:
        cmd_end_select();
        IF_ERR_BREAK;
        continue;
      case kwDIM:
        cmd_dim(0);
        break;
      case kwREDIM:
        cmd_redim();
        break;
      case kwAPPEND:
        cmd_append();
        break;
      case kwINSERT:
        cmd_lins();
        break;
      case kwDELETE:
        cmd_ldel();
        break;
      case kwERASE:
        cmd_erase();
        break;
      case kwREAD:
        cmd_read();
        break;
      case kwDATA:
        cmd_data();
        break;
      case kwRESTORE:
        cmd_restore();
        break;
      case kwOPTION:
        cmd_options();
        break;
      case kwTYPE_CALLEXTP:
        bc_loop_call_extp();
        IF_ERR_BREAK;
        continue;
      case kwTYPE_CALLP:
        bc_loop_call_proc();
        break;
      case kwTYPE_CALL_UDP:
        cmd_udp(kwPROC);
        if (isf) {
          proc_level++;
        }
        IF_ERR_BREAK;
        continue;
      case kwTYPE_CALL_UDF:
        if (isf) {
          cmd_udp(kwFUNC);
          proc_level++;
        } else {
          err_syntax(kwTYPE_CALL_UDF, "%G");
        }
        IF_ERR_BREAK;
        continue;
      case kwTYPE_RET:
        cmd_udpret();
        if (isf) {
          proc_level--;
          if (proc_level == 0) {
            return;
          }
        }
        IF_ERR_BREAK;
        continue;
      case kwTYPE_CRVAR:
        cmd_crvar();
        break;
      case kwTYPE_PARAM:
        cmd_param();
        break;
      case kwEXIT:
        pops = cmd_exit();
        if (isf && pops) {
          proc_level--;
          if (proc_level == 0) {
            return;
          }
        }
        IF_ERR_BREAK;
        continue;
      case kwLINE:
        cmd_line();
        break;
      case kwCOLOR:
        cmd_color();
        break;
      case kwOPEN:
        cmd_fopen();
        break;
      case kwCLOSE:
        cmd_fclose();
        break;
      case kwFILEWRITE:
        cmd_fwrite();
        break;
      case kwFILEREAD:
        cmd_fread();
        break;
      case kwLOGPRINT:
        cmd_print(PV_LOG);
        break;
      case kwFILEPRINT:
        cmd_print(PV_FILE);
        break;
      case kwSPRINT:
        cmd_print(PV_STRING);
        break;
      case kwLINEINPUT:
        cmd_flineinput();
        break;
      case kwSINPUT:
        cmd_input(PV_STRING);
        break;
      case kwFILEINPUT:
        cmd_input(PV_FILE);
        break;
      case kwSEEK:
        cmd_fseek();
        break;
      case kwTRON:
        opt_trace_on = 1;
        continue;
      case kwTROFF:
        opt_trace_on = 0;
        continue;
      case kwSTOP:
      case kwEND:
        bc_loop_end();
        break;
      case kwCHAIN:
        cmd_chain();
        break;
      case kwRUN:
        cmd_run(1);
        break;
      case kwEXEC:
        cmd_run(0);
        break;
      case kwTRY:
        cmd_try();
        IF_ERR_BREAK;
        continue;
      case kwCATCH:
        cmd_catch();
        IF_ERR_BREAK;
        continue;
      case kwENDTRY:
        cmd_end_try();
        continue;
      default:
        log_printf("OUT OF ADDRESS SPACE\n");
        for (i = 0; keyword_table[i].name[0] != '\0'; i++) {
          if (prog_source[prog_ip] == keyword_table[i].code) {
            log_printf("OR ILLEGAL CALL TO '%s'\n", keyword_table[i].name);
            break;
          }
        }
        if (!opt_quiet) {
          hex_dump(prog_source, prog_length);
        }
        rt_raise("SEG:CODE[%x]=%02x", prog_ip, prog_source[prog_ip]);
      }
    }
    if (prog_ip < prog_length) {
      code = prog_source[prog_ip++];
      if (code == kwTYPE_LINE) {
        prog_line = code_getaddr();
        if (opt_trace_on) {
          dev_trace_line(prog_line);
        }
      } else if (code != kwTYPE_EOC) {
        if (!opt_quiet) {
          hex_dump(prog_source, prog_length);
        }
        prog_ip--;
        if (code == kwTYPE_SEP) {
          rt_raise("COMMAND SEPARATOR '%c' FOUND", prog_source[prog_ip + 1]);
        } else {
          rt_raise("PARAM COUNT ERROR @%d=%X %d", prog_ip, prog_source[prog_ip], code);
        }
      }
    }
    // quit on error
    IF_ERR_BREAK;
  }
}

/**
 * debug info
 * stack dump
 */
void dump_stack() {
  stknode_t node;
  int i;

  do {
    code_pop(&node, 0);
    if (node.type != 0xFF) {
      for (i = 0; keyword_table[i].name[0] != '\0'; i++) {
        if (node.type == keyword_table[i].code) {
          dev_printf("\t%s", keyword_table[i].name);
          switch (node.type) {
          case kwGOSUB:
            dev_printf(" RIP: %d", node.x.vgosub.ret_ip);
            if (prog_source[node.x.vgosub.ret_ip] == kwTYPE_LINE) {
              dev_printf(" = LI %d", (*((uint16_t *)(prog_source + node.x.vgosub.ret_ip + 1))) - 1);
            }
            break;
          }
          dev_print("\n");
          break;
        }
      }
    } else {
      break;
    }
  } while (1);
}

// load libraries - each library is loaded on new task
void brun_load_libraries(int tid) {
  // reset symbol mapping
  for (int i = 0; i < prog_symcount; i++) {
    prog_symtable[i].task_id = prog_symtable[i].exp_idx = -1;
  }
  // for each library
  for (int i = 0; i < prog_libcount; i++) {
    if (prog_libtable[i].type == 1) {
      // === SB Unit ===
      // create task
      int lib_tid = brun_create_task(prog_libtable[i].lib, 0, 1);
      activate_task(tid);

      // update lib-table's task-id field (in this code; not in lib's code)
      prog_libtable[i].tid = lib_tid;

      // update lib-symbols's task-id field (in this code; not in lib's code)
      for (int j = 0; j < prog_symcount; j++) {
        char *pname = strrchr(prog_symtable[j].symbol, '.') + 1;
        // the name without the 'class'
        if ((prog_symtable[j].lib_id & (~UID_UNIT_BIT)) == prog_libtable[i].id) {
          // find symbol by name (for sure) and update it
          // this is required because lib may be newer than
          // parent
          for (int k = 0; k < taskinfo(lib_tid)->sbe.exec.expcount; k++) {
            if (strcmp(pname, taskinfo(lib_tid)->sbe.exec.exptable[k].symbol) == 0) {
              prog_symtable[j].exp_idx = k;
              // adjust sid (sid is <-> exp_idx in lib)
              prog_symtable[j].task_id = lib_tid;
              // connect the library
              break;
            }
          }
        }
      }
    } else {
      // === C Module ===
      // update lib-table's task-id field (in this code; not in lib's code)
      prog_libtable[i].tid = -1;  // not a task
      int lib_id = prog_libtable[i].id;
      plugin_open(prog_libtable[i].lib, lib_id);

      // update lib-symbols's task-id field (in this code; not in lib's code)
      for (int j = 0; j < prog_symcount; j++) {
        if (prog_symtable[j].lib_id == lib_id) {
          prog_symtable[j].exp_idx = plugin_get_kid(lib_id, prog_symtable[j].symbol);
          prog_symtable[j].task_id = -1;
        }
      }
    }

    // return
    activate_task(tid);
  }
}

/*
 * RUN byte-code
 *
 * ByteCode Structure (executables, not units):
 *
 * [header (bc_head_t)]
 * [label-table (ADDRSZ) * label_count]
 * [import-lib-table (bc_lib_rec_t) * lib_count]
 * [import-symbol-table (bc_symbol_rec_t) * symbol_count]
 * [the bytecode itself]
 *
 * brun_init(source)
 * ...brun_create_task(source)
 *
 * brun()
 *
 * exec_close()
 * ...exec_close_task()
 */
int brun_create_task(const char *filename, byte *preloaded_bc, int libf) {
  bc_head_t hdr;
  unit_file_t uft;
  byte *source;
  char fname[OS_PATHNAME_SIZE + 1];

  if (preloaded_bc) {
    // I have already BC
    source = preloaded_bc;
    strlcpy(fname, filename, sizeof(fname));
  } else {
    // prepare filename
    if (!libf) {
      char *p;
      strlcpy(fname, filename, sizeof(fname));
      p = strrchr(fname, '.');
      if (p) {
        *p = '\0';
      }
      strlcat(fname, ".sbx", sizeof(fname));
    } else {
      find_unit(filename, fname);
    }
    if (access(fname, R_OK)) {
      panic("File '%s' not found", fname);
    }
    // look if it is already loaded
    if (search_task(fname) != -1) {
      return search_task(fname);
    }
    // open & load
    int h = open(fname, O_RDWR | O_BINARY);
    if (h == -1) {
      panic("File '%s' not found", fname);
    }
    // load it
    if (libf) {
      read(h, &uft, sizeof(unit_file_t));
      lseek(h, sizeof(unit_sym_t) * uft.sym_count, SEEK_CUR);
    }
    read(h, &hdr, sizeof(bc_head_t));
    if (hdr.sbver != SB_DWORD_VER) {
      panic("File '%s' version incorrect", fname);
    }
    source = malloc(hdr.size + 4);
    lseek(h, 0, SEEK_SET);
    read(h, source, hdr.size);
    close(h);
  }

  // create task
  int tid = create_task(fname); // create a task
  activate_task(tid);           // make it active
  ctask->bytecode = source;
  byte *cp = source;

  if (memcmp(source, "SBUn", 4) == 0) { // load a unit
    memcpy(&uft, cp, sizeof(unit_file_t));
    cp += sizeof(unit_file_t);
    prog_expcount = uft.sym_count;

    // copy export-symbols from BC
    if (prog_expcount) {
      prog_exptable = (unit_sym_t *)malloc(prog_expcount * sizeof(unit_sym_t));
      for (int i = 0; i < prog_expcount; i++) {
        memcpy(&prog_exptable[i], cp, sizeof(unit_sym_t));
        cp += sizeof(unit_sym_t);
      }
    }
  } else if (memcmp(source, "SBEx", 4) == 0) {
    // load an executable
  } else {
    // signature error
    panic("Wrong bytecode signature");
  }

  // build executor's task
  memcpy(&hdr, cp, sizeof(bc_head_t));
  cp += sizeof(bc_head_t);

  prog_varcount = hdr.var_count;
  prog_labcount = hdr.lab_count;
  prog_libcount = hdr.lib_count;
  prog_symcount = hdr.sym_count;

  // create variable-table
  if (prog_varcount == 0) {
    prog_varcount++;
  }
  tvar = malloc(sizeof(var_t *) * prog_varcount);
  for (int i = 0; i < prog_varcount; i++) {
    tvar[i] = v_new();
  }
  // create label-table
  if (prog_labcount) {
    tlab = malloc(sizeof(lab_t) * prog_labcount);
    for (int i = 0; i < prog_labcount; i++) {
      // copy labels from BC
      memcpy(&tlab[i].ip, cp, ADDRSZ);
      cp += ADDRSZ;
    }
  }
  // build import-lib table
  if (prog_libcount) {
    prog_libtable = (bc_lib_rec_t *)malloc(prog_libcount * sizeof(bc_lib_rec_t));
    for (int i = 0; i < prog_libcount; i++) {
      memcpy(&prog_libtable[i], cp, sizeof(bc_lib_rec_t));
      cp += sizeof(bc_lib_rec_t);
    }
  }

  // build import-symbol table
  if (prog_symcount) {
    prog_symtable = (bc_symbol_rec_t *)malloc(prog_symcount * sizeof(bc_symbol_rec_t));
    for (int i = 0; i < prog_symcount; i++) {
      memcpy(&prog_symtable[i], cp, sizeof(bc_symbol_rec_t));
      cp += sizeof(bc_symbol_rec_t);
    }
  }

  // create system stack
  prog_stack_alloc = SB_EXEC_STACK_SIZE;
  prog_stack = malloc(sizeof(stknode_t) * prog_stack_alloc);
  prog_stack_count = 0;
  prog_timer = NULL;

  // create eval's stack
  eval_size = SB_EVAL_STACK_SIZE;
  eval_stk = malloc(sizeof(var_t) * eval_size);
  memset(eval_stk, 0, sizeof(var_t) * eval_size);
  eval_sp = 0;

  // initialize the rest tasks globals
  prog_error = errNone;
  prog_line = 0;
  prog_dp = data_org = hdr.data_ip;
  prog_length = hdr.bc_count;
  prog_source = cp;
  prog_ip = 0;

  exec_setup_predefined_variables();
  if (prog_libcount) {
    brun_load_libraries(tid);
  }

  return tid;
}

/**
 * clean up the current task's (executor's) data
 */
int exec_close_task() {
  if (ctask->bytecode) {
    // clean up - format list
    free_format();

    // clean up - eval stack
    free(eval_stk);
    eval_size = 0;
    eval_sp = 0;

    // clean up - prog stack
    while (prog_stack_count > 0) {
      code_pop_and_free();
    }
    free(prog_stack);
    // clean up - variables
    for (int i = 0; i < (int) prog_varcount; i++) {
      // do not free imported variables
      int shared = -1;
      for (int j = 0; j < prog_symcount; j++) {
        if (prog_symtable[j].type == stt_variable &&
            prog_symtable[j].var_id == i) {
          shared = j;
          break;
        }
      }

      // free this variable
      if (shared == -1) {
        v_free(tvar[i]);
        v_detach(tvar[i]);
      }
    }

    free(tvar);
    ctask->has_sysvars = 0;

    // clean up - rest tables
    if (prog_expcount) {
      free(prog_exptable);
    }
    if (prog_libcount) {
      free(prog_libtable);
    }
    if (prog_symcount) {
      free(prog_symtable);
    }
    if (prog_labcount) {
      free(tlab);
    }

    // clean up - the rest
    free(ctask->bytecode);
    ctask->bytecode = NULL;

    // cleanup the keyboard map
    keymap_free();

    // cleanup timers
    timer_free(prog_timer);
    prog_timer = NULL;
  }

  if (prog_error != errEnd && prog_error != errNone) {
    return 1;
  }
  return 0;
}

/**
 * close the executor
 */
int exec_close(int tid) {
  int prev_tid, i;

  prev_tid = activate_task(tid);

  for (i = 0; i < count_tasks(); i++) {
    activate_task(i);

    if (ctask->status == tsk_ready && ctask->parent == tid) {
      exec_close(ctask->tid);
    }
  }

  activate_task(tid);
  exec_close_task();
  close_task(tid);
  activate_task(prev_tid);
  return 1;
}

/**
 * update common variables
 *
 * if dir = 0, source is the unit, dir = 1 source is this program
 *
 * actually dir has no meaning, but it is more logical (dir=0 always is also correct)
 */
void exec_sync_variables(int dir) {
  int i, tid;
  unit_sym_t *us;               // unit's symbol data
  bc_symbol_rec_t *ps;          // program's symbol data
  var_t *vp;                    // variable for swap

  tid = ctask->tid;

  for (i = 0; i < prog_symcount; i++) {
    if (prog_symtable[i].type == stt_variable) {
      ps = &prog_symtable[i];
      us = &(taskinfo(ps->task_id)->sbe.exec.exptable[ps->exp_idx]);

      if (dir == 0) {
        activate_task(ps->task_id);
        vp = tvar[us->vid];

        // pointer assignment (shared var_t pointer)
        activate_task(tid);
        if (tvar[ps->var_id] != vp) {
          v_free(tvar[ps->var_id]);
          v_detach(tvar[ps->var_id]);
        }
        tvar[ps->var_id] = vp;
      } else {
        activate_task(tid);
        vp = tvar[ps->var_id];

        activate_task(ps->task_id);
        tvar[us->vid] = vp;

        activate_task(tid);
      }
    }
  }
}

/**
 * system specific things - before compilation
 */
void sys_before_comp() {
  // setup prefered screen mode variables
  if (getenv("SBGRAF")) {
    if (getenv("SBGRAF")) {
      comp_preproc_grmode(getenv("SBGRAF"));
    }
    opt_graphics = 2;
  }
}

/**
 * execute the code on this task
 */
int sbasic_exec_task(int tid) {
  int prev_tid, success;

  prev_tid = activate_task(tid);

  bc_loop(0);
  success = (prog_error == errNone || prog_error == errEnd);
  if (success) {
    prog_error = errNone;
  }
  activate_task(prev_tid);
  return success;
}

/**
 * run libraries and main-code
 */
int sbasic_recursive_exec(int tid) {
  int i, success = 1;
  int prev_tid;

  prev_tid = activate_task(tid);

  for (i = 0; i < count_tasks(); i++) {
    if (taskinfo(i)->parent == tid) {
      // do the same for the childs
      activate_task(i);
      success = sbasic_recursive_exec(i);
      if (!success) {
        break;
      }
    }
  }

  if (success) {
    activate_task(tid);
    exec_sync_variables(0);

    // run
    if (!opt_quiet) {
      dev_printf("Initializing #%d (%s) ...\n", ctask->tid, ctask->file);
    }
    success = sbasic_exec_task(ctask->tid);
  }

  activate_task(prev_tid);
  return success;
}

/**
 * compile the given file into bytecode
 */
int sbasic_compile(const char *file) {
  int comp_rq = 0;              // compilation required = 0
  int success = 1;

  if (strstr(file, ".sbx") == file + strlen(file) - 4) {
    return success;             // file is an executable
  }

  if (opt_nosave) {
    comp_rq = 1;
  } else {
    char exename[OS_PATHNAME_SIZE + 1];
    char *p;

    // executable name
    strcpy(exename, file);
    p = strrchr(exename, '.');
    if (p) {
      *p = '\0';
    }
    strcat(exename, ".sbx");

    if ((access(exename, R_OK) == 0)) {
      time_t bin_date = 0, src_date = 0;

      // compare dates
      if ((bin_date = sys_filetime(exename)) == 0L) {
        comp_rq = 1;
      }
      else if ((src_date = sys_filetime(file)) == 0L) {
        comp_rq = 1;
      }
      if (bin_date >= src_date) {
        // TODO: check binary version
        ;
      } else {
        comp_rq = 1;
      }
    } else {
      comp_rq = 1;
    }
  }

  // compile it
  if (comp_rq) {
    sys_before_comp();  // system specific preparations for compilation
    success = comp_compile(file);
  }
  return success;
}

/**
 * initialize executor and run a binary
 */
int sbasic_exec_prepare(const char *filename) {
  int taskId;

  v_init_pool();

  // load source
  if (opt_nosave) {
    taskId = brun_create_task(filename, ctask->bytecode, 0);
  } else {
    taskId = brun_create_task(filename, 0, 0);
  }
  // reset system
  cmd_play_reset();
  graph_reset();
  return taskId;
}

/*
 * remember the directory location of the running program
 */
void sbasic_set_bas_dir(const char *bas_file) {
  const char *sep = strrchr(bas_file, OS_DIRSEP);
  int path_len =  sep == NULL ? 0 : (sep - bas_file);
  char cwd[OS_PATHNAME_SIZE + 1];

  cwd[0] = '\0';
  gsb_bas_dir[0] = '\0';
  getcwd(cwd, sizeof(cwd) - 1);
  char *ch;
  for (ch = cwd; *ch != '\0'; ch++) {
    if (*ch == '\\') {
      *ch = '/';
    }
  }
  if (bas_file[0] == '/' ||
      (bas_file[1] == ':' && ((bas_file[2] == '\\') || bas_file[2] == '/'))) {
    // full path
    strncat(gsb_bas_dir, bas_file, path_len + 1);
  } else if (path_len > 0) {
    // relative path
    // append the non file part of bas_file to cwd
    strcat(gsb_bas_dir, cwd);
    if (gsb_bas_dir[strlen(gsb_bas_dir) - 1] != '/') {
      strcat(gsb_bas_dir, "/");
    }
    strncat(gsb_bas_dir, bas_file, path_len + 1);
  } else {
    // in current dir
    strcat(gsb_bas_dir, cwd);
    if (gsb_bas_dir[strlen(gsb_bas_dir) - 1] != '/') {
      strcat(gsb_bas_dir, "/");
    }
  }
}

/**
 * this is the main 'execute' routine; its work depended on opt_xxx flags
 * use it instead of sbasic_main if managers are already initialized
 *
 * @param file the source file
 * @return true on success
 */
int sbasic_exec(const char *file) {
  int success = 0;
  int exec_rq = 1;

  // init compile-time options
  opt_pref_width = 0;
  opt_pref_height = 0;
  opt_show_page = 0;

  // setup global values
  gsb_last_line = gsb_last_error = 0;
  strlcpy(gsb_last_file, file, sizeof(gsb_last_file));
  strcpy(gsb_last_errmsg, "");
  sbasic_set_bas_dir(file);
  success = sbasic_compile(file);

  if (ctask->bc_type == 2) {
    // cannot run a unit
    exec_rq = 0;
    gsb_last_error = 1;
  } else if (!success) {        // there was some errors; do not continue
    exec_rq = 0;
    gsb_last_error = 1;
  }

  if (exec_rq) {                // we will run it
    // load everything
    int exec_tid = sbasic_exec_prepare(file);

    dev_init(opt_graphics, 0);  // initialize output device for graphics
    srand(clock());             // randomize

    // run
    sbasic_recursive_exec(exec_tid);

    // normal exit
    if (!opt_quiet) {
      inf_done();
    }

    exec_close(exec_tid);       // clean up executor's garbages
    dev_restore();              // restore device
  }

  // return compilation errors as failure
  return !success ? 0 : !gsb_last_error;
}

/**
 * this is the main routine; its work depended on opt_xxx flags
 *
 * @param file the source file
 * @return true on success
 */
int sbasic_main(const char *file) {
  int success;

  // initialize managers
  init_tasks();
  unit_mgr_init();
  plugin_init();

  if (prog_error) {
    success = 0;
  } else {
    success = sbasic_exec(file);
  }

  // clean up managers
  plugin_close();
  unit_mgr_close();
  destroy_tasks();

  return success;
}

