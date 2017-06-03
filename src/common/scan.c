// This file is part of SmallBASIC
//
// pseudo-compiler: Converts the source to byte-code.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#define SCAN_MODULE
#include "common/sys.h"
#include "common/device.h"
#include "common/kw.h"
#include "common/bc.h"
#include "common/scan.h"
#include "common/smbas.h"
#include "common/units.h"
#include "common/extlib.h"
#include "common/messages.h"
#include "languages/keywords.en.c"

char *comp_array_uds_field(char *p, bc_t *bc);
void comp_text_line(char *text, int addLineNo);
bcip_t comp_search_bc(bcip_t ip, code_t code);
extern void expr_parser(bc_t *bc);
extern void sc_raise2(const char *fmt, int line, const char *buff);

#define STRLEN(s) ((sizeof(s) / sizeof(s[0])) - 1)
#define LEN_OPTION     STRLEN(LCN_OPTION)
#define LEN_IMPORT     STRLEN(LCN_IMPORT_WRS)
#define LEN_UNIT       STRLEN(LCN_UNIT_WRS)
#define LEN_UNIT_PATH  STRLEN(LCN_UNIT_PATH)
#define LEN_INC        STRLEN(LCN_INC)
#define LEN_SUB_WRS    STRLEN(LCN_SUB_WRS)
#define LEN_FUNC_WRS   STRLEN(LCN_FUNC_WRS)
#define LEN_DEF_WRS    STRLEN(LCN_DEF_WRS)
#define LEN_END_WRS    STRLEN(LCN_END_WRS)
#define LEN_END_SELECT STRLEN(LCN_END_SELECT)
#define LEN_END_TRY    STRLEN(LCN_END_TRY)
#define LEN_PREDEF     STRLEN(LCN_PREDEF)
#define LEN_QUIET      STRLEN(LCN_QUIET)
#define LEN_GRMODE     STRLEN(LCN_GRMODE)
#define LEN_TEXTMODE   STRLEN(LCN_TEXTMODE)
#define LEN_COMMAND    STRLEN(LCN_COMMAND)
#define LEN_SHOWPAGE   STRLEN(LCN_SHOWPAGE)
#define LEN_ANTIALIAS  STRLEN(LCN_ANTIALIAS)
#define LEN_LDMODULES  STRLEN(LCN_LOAD_MODULES)
#define LEN_AUTOLOCAL  STRLEN(LCN_AUTOLOCAL)

#define SKIP_SPACES(p)                          \
  while (*p == ' ' || *p == '\t') {             \
    p++;                                        \
  }

#define CHKOPT(x)                               \
  (strncmp(p, (x), strlen((x))) == 0)

#define GROWSIZE 128
#define MAX_PARAMS 256

typedef struct  {
  byte *code;
  uint32_t size;
} byte_code;

void err_wrongproc(const char *name) {
  sc_raise(MSG_WRONG_PROCNAME, name);
}

void err_comp_missing_rp() {
  sc_raise(MSG_EXP_MIS_RP);
}

void err_comp_missing_lp() {
  sc_raise(MSG_EXP_MIS_LP);
}

void err_comp_missing_rb() {
  sc_raise(MSG_EXP_MIS_RB);
}

void err_comp_label_not_def(const char *name) {
  sc_raise(MSG_LABEL_NOT_DEFINED, name);
}

// strcat replacement when p is incremented from dest
void memcat(char *dest, char *p) {
  int lenb = strlen(dest);
  int lenp = strlen(p);
  memmove(dest + lenb, p, lenp);
  dest[lenb + lenp] = '\0';
}

/*
 * reset the external proc/func lists
 */
void comp_reset_externals(void) {
  // reset functions
  if (comp_extfunctable) {
    free(comp_extfunctable);
  }
  comp_extfunctable = NULL;
  comp_extfunccount = comp_extfuncsize = 0;

  // reset procedures
  if (comp_extproctable) {
    free(comp_extproctable);
  }
  comp_extproctable = NULL;
  comp_extproccount = comp_extprocsize = 0;
}

// update imports table
bc_symbol_rec_t *add_imptable_rec(const char *proc_name, int lib_id, int symbol_type) {
  bc_symbol_rec_t *sym = (bc_symbol_rec_t *)malloc(sizeof(bc_symbol_rec_t));
  memset(sym, 0, sizeof(bc_symbol_rec_t));

  strcpy(sym->symbol, proc_name);  // symbol name
  sym->type = symbol_type;         // symbol type
  sym->lib_id = lib_id;            // library id
  sym->sym_id = comp_impcount;     // symbol index

  if (comp_imptable.count) {
    comp_imptable.elem = (bc_symbol_rec_t **)realloc(comp_imptable.elem,
                                                     (comp_imptable.count + 1) * sizeof(bc_symbol_rec_t **));
  } else {
    comp_imptable.elem = (bc_symbol_rec_t **)malloc(sizeof(bc_symbol_rec_t **));
  }
  comp_imptable.elem[comp_imptable.count] = sym;
  comp_imptable.count++;
  return sym;
}

// store lib-record
void add_libtable_rec(const char *lib, int uid, int type) {
  bc_lib_rec_t *imlib = (bc_lib_rec_t *)malloc(sizeof(bc_lib_rec_t));
  memset(imlib, 0, sizeof(bc_lib_rec_t));

  strcpy(imlib->lib, lib);
  imlib->id = uid;
  imlib->type = type;

  if (comp_libtable.count) {
    comp_libtable.elem = (bc_lib_rec_t **)realloc(comp_libtable.elem,
                                                 (comp_libtable.count + 1) * sizeof(bc_lib_rec_t **));
  } else {
    comp_libtable.elem = (bc_lib_rec_t **)malloc(sizeof(bc_lib_rec_t **));
  }
  comp_libtable.elem[comp_libtable.count] = imlib;
  comp_libtable.count++;
}

/*
 * add an external procedure to the list
 */
int comp_add_external_proc(const char *proc_name, int lib_id) {
  // TODO: scan for conflicts
  if (comp_extproctable == NULL) {
    comp_extprocsize = 16;
    comp_extproctable = (ext_proc_node_t *)malloc(sizeof(ext_proc_node_t) * comp_extprocsize);
  } else if (comp_extprocsize <= (comp_extproccount + 1)) {
    comp_extprocsize += 16;
    comp_extproctable = (ext_proc_node_t *)realloc(comp_extproctable,
                                                   sizeof(ext_proc_node_t) * comp_extprocsize);
  }

  comp_extproctable[comp_extproccount].lib_id = lib_id;
  comp_extproctable[comp_extproccount].symbol_index = comp_impcount;
  strcpy(comp_extproctable[comp_extproccount].name, proc_name);
  strupper(comp_extproctable[comp_extproccount].name);
  comp_extproccount++;

  add_imptable_rec(proc_name, lib_id, stt_procedure);
  return comp_extproccount - 1;
}

/*
 * Add an external function to the list
 */
int comp_add_external_func(const char *func_name, int lib_id) {
  // TODO: scan for conflicts
  if (comp_extfunctable == NULL) {
    comp_extfuncsize = 16;
    comp_extfunctable = (ext_func_node_t *)malloc(sizeof(ext_func_node_t) * comp_extfuncsize);
  } else if (comp_extfuncsize <= (comp_extfunccount + 1)) {
    comp_extfuncsize += 16;
    comp_extfunctable = (ext_func_node_t *)realloc(comp_extfunctable,
                                                   sizeof(ext_func_node_t) * comp_extfuncsize);
  }

  comp_extfunctable[comp_extfunccount].lib_id = lib_id;
  comp_extfunctable[comp_extfunccount].symbol_index = comp_impcount;
  strcpy(comp_extfunctable[comp_extfunccount].name, func_name);
  strupper(comp_extfunctable[comp_extfunccount].name);
  comp_extfunccount++;

  add_imptable_rec(func_name, lib_id, stt_function);
  return comp_extfunccount - 1;
}

/*
 * returns the external procedure id
 */
int comp_is_external_proc(const char *name) {
  int i;

  for (i = 0; i < comp_extproccount; i++) {
    if (strcasecmp(comp_extproctable[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

/*
 * returns the external function id
 */
int comp_is_external_func(const char *name) {
  int i;

  for (i = 0; i < comp_extfunccount; i++) {
    if (strcasecmp(comp_extfunctable[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

/*
 * Notes:
 *  block_level = the depth of nested block
 *  block_id    = unique number of each block (based on stack use)
 *
 * Example:
 *  ? xxx           ' level 0, id 0
 *  for i=1 to 20   ' level 1, id 1
 *      ? yyy       ' level 1, id 1
 *      if a=1      ' level 2, id 2 (our IF uses stack)
 *          ...     ' level 2, id 2
 *      else        ' level 2, id 2 // not 3
 *          ...     ' level 2, id 2
 *      fi          ' level 2, id 2
 *      if a=2      ' level 2, id 3
 *          ...     ' level 2, id 3
 *      fi          ' level 2, id 3
 *      ? zzz       ' level 1, id 1
 *  next            ' level 1, id 1
 *  ? ooo           ' level 0, id 0
 */

/*
 * error messages
 */
void sc_raise(const char *fmt, ...) {
  if (!prog_error) {
    char *buff;
    va_list ap;

    va_start(ap, fmt);
    comp_error = 1;

    buff = malloc(SB_SOURCELINE_SIZE + 1);
    vsnprintf(buff, SB_SOURCELINE_SIZE, fmt, ap);
    va_end(ap);

    sc_raise2(comp_bc_sec, comp_line, buff);  // sberr.h
    free(buff);
  }
}

/*
 * prepare name (keywords, variables, labels, proc/func names)
 */
char *comp_prepare_name(char *dest, const char *source, int size) {
  char *p = (char *)source;
  SKIP_SPACES(p);

  strncpy(dest, p, size);
  dest[size] = '\0';
  p = dest;
  while (*p && (is_alpha(*p) || is_digit(*p) || *p == '$' || *p == '/' || *p == '_' || *p == '.')) {
    p++;
  }
  *p = '\0';

  str_alltrim(dest);
  return dest;
}

/*
 * returns the ID of the label. If there is no one, then it creates one
 */
bid_t comp_label_getID(const char *label_name) {
  bid_t idx = -1, i;
  char name[SB_KEYWORD_SIZE + 1];

  comp_prepare_name(name, label_name, SB_KEYWORD_SIZE);

  for (i = 0; i < comp_labtable.count; i++) {
    if (strcmp(comp_labtable.elem[i]->name, name) == 0) {
      idx = i;
      break;
    }
  }

  if (idx == -1) {
    if (opt_verbose) {
      log_printf(MSG_NEW_LABEL, comp_line, name, comp_labcount);
    }

    comp_label_t *label = (comp_label_t *)malloc(sizeof(comp_label_t));
    memset(label, 0, sizeof(comp_label_t));
    strcpy(label->name, name);
    label->ip = INVALID_ADDR;
    label->dp = INVALID_ADDR;
    label->level = comp_block_level;
    label->block_id = comp_block_id;

    if (comp_labtable.count == comp_labtable.size) {
      comp_labtable.size += GROWSIZE;
      comp_labtable.elem = realloc(comp_labtable.elem, comp_labtable.size * sizeof(comp_label_t *));
    }

    comp_labtable.elem[comp_labtable.count] = label;
    idx = comp_labtable.count;
    comp_labtable.count++;
  }

  return idx;
}

/*
 * set LABEL's position (IP)
 */
void comp_label_setip(bid_t idx) {
  if (idx < comp_labtable.count) {
    comp_labtable.elem[idx]->ip = comp_prog.count;
    comp_labtable.elem[idx]->dp = comp_data.count;
    comp_labtable.elem[idx]->level = comp_block_level;
    comp_labtable.elem[idx]->block_id = comp_block_id;
  }
}

/*
 * returns the full-path UDP/UDF name
 */
void comp_prepare_udp_name(char *dest, const char *basename) {
  char tmp[SB_SOURCELINE_SIZE + 1];

  comp_prepare_name(tmp, baseof(basename, '/'), SB_KEYWORD_SIZE);
  if (comp_proc_level) {
    sprintf(dest, "%s/%s", comp_bc_proc, tmp);
  } else {
    strcpy(dest, tmp);
  }
}

/*
 * returns the ID of the UDP/UDF
 */
bid_t comp_udp_id(const char *proc_name, int scan_tree) {
  bid_t i;
  char *name = comp_bc_temp, *p;
  char base[SB_KEYWORD_SIZE + 1];
  char *root;
  int len;

  if (scan_tree) {
    comp_prepare_name(base, baseof(proc_name, '/'), SB_KEYWORD_SIZE);
    root = strdup(comp_bc_proc);
    do {
      // (nested procs) move root down
      if ((len = strlen(root)) != 0) {
        sprintf(name, "%s/%s", root, base);
        p = strrchr(root, '/');
        if (p) {
          *p = '\0';
        } else {
          strcpy(root, "");
        }
      } else {
        strcpy(name, base);
      }
      // search on local
      for (i = 0; i < comp_udpcount; i++) {
        if (strcmp(comp_udptable[i].name, name) == 0) {
          free(root);
          return i;
        }
      }
    } while (len);

    // not found
    free(root);
  } else {
    comp_prepare_udp_name(name, proc_name);

    // search on local
    for (i = 0; i < comp_udpcount; i++) {
      if (strcmp(comp_udptable[i].name, name) == 0) {
        return i;
      }
    }
  }

  return -1;
}

/*
 * creates a new UDP/UDF node
 * and returns the new ID
 */
bid_t comp_add_udp(const char *proc_name) {
  char *name = comp_bc_temp;
  bid_t idx = -1, i;
  comp_prepare_udp_name(name, proc_name);

  /*
   * #if !defined(OS_LIMITED) // check variables for conflict for ( i =
   * 0; i < comp_varcount; i ++ ) { if ( strcmp(comp_vartable[i].name,
   * name) == 0 ) { sc_raise("User-defined function/procedure name,
   * '%s', conflicts with variable", name); break; } } #endif
   */

  // search
  for (i = 0; i < comp_udpcount; i++) {
    if (strcmp(comp_udptable[i].name, name) == 0) {
      idx = i;
      break;
    }
  }

  if (idx == -1) {
    if (comp_udpcount >= comp_udpsize) {
      comp_udpsize += GROWSIZE;
      comp_udptable = realloc(comp_udptable, comp_udpsize * sizeof(comp_udp_t));
    }

    if (!(is_alpha(name[0]) || name[0] == '_')) {
      err_wrongproc(name);
    } else {
      if (opt_verbose) {
        log_printf(MSG_NEW_UDP, comp_line, name, comp_udpcount);
      }
      comp_udptable[comp_udpcount].name = malloc(strlen(name) + 1);
      comp_udptable[comp_udpcount].ip = INVALID_ADDR; // bc_prog.count;
      comp_udptable[comp_udpcount].level = comp_block_level;
      comp_udptable[comp_udpcount].block_id = comp_block_id;
      comp_udptable[comp_udpcount].pline = comp_line;
      strcpy(comp_udptable[comp_udpcount].name, name);
      idx = comp_udpcount;
      comp_udpcount++;
    }
  }

  return idx;
}

/*
 * sets the IP of the user-defined-procedure (or function)
 */
bid_t comp_udp_setip(const char *proc_name, bcip_t ip) {
  bid_t idx;
  char *name = comp_bc_temp;

  comp_prepare_udp_name(name, proc_name);

  idx = comp_udp_id(name, 0);
  if (idx != -1) {
    comp_udptable[idx].ip = comp_prog.count;
    comp_udptable[idx].level = comp_block_level;
    comp_udptable[idx].block_id = comp_block_id;
  }
  return idx;
}

/*
 * Returns the IP of an UDP/UDF
 */
bcip_t comp_udp_getip(const char *proc_name) {
  bid_t idx;
  char *name = comp_bc_temp;

  comp_prepare_udp_name(name, proc_name);

  idx = comp_udp_id(name, 1);
  if (idx != -1) {
    return comp_udptable[idx].ip;
  }
  return INVALID_ADDR;
}

/*
 * parameters string-section
 */
char *get_param_sect(char *text, const char *delim, char *dest) {
  char *p = (char *)text;
  char *d = dest;
  int quotes = 0, level = 0, skip_ch = 0;
  int curley_brace = 0;

  if (p == NULL) {
    *dest = '\0';
    return 0;
  }

  while (is_space(*p)) {
    p++;
  }

  while (*p) {
    if (quotes) {
      if (*p == '\\' && (*(p + 1) == '\"' || *(p + 1) == '\\')) {
        // add the escaped quote or slash and continue
        *d++ = *p++;
      } else if (*p == '\"') {
        quotes = 0;
      }
    } else {
      switch (*p) {
      case '\"':
        quotes = 1;
        break;
      case '(':
        level++;
        break;
      case ')':
        level--;
        break;
      case '\n':
      case '\r':
        skip_ch = 1;
        break;
      case '{':
        curley_brace++;
        break;
      case '}':
        curley_brace--;
        break;
      };
    }

    // delim check
    if (delim != NULL && level <= 0 && quotes == 0 && curley_brace == 0) {
      if (strchr(delim, *p) != NULL) {
        break;
      }
    }
    // copy
    if (!skip_ch) {
      *d = *p;
      d++;
    } else {
      skip_ch = 0;
    }
    p++;
  }

  if (quotes) {
    *d++ = '\"';
  }

  *d = '\0';

  if (level > 0) {
    err_comp_missing_rp();
  }
  if (level < 0) {
    err_comp_missing_lp();
  }
  if (curley_brace  > 0) {
    err_comp_missing_rb();
  }
  str_alltrim(dest);
  return p;
}

/*
 * checking for missing labels
 */
int comp_check_labels() {
  bid_t i;

  for (i = 0; i < comp_labcount; i++) {
    if (comp_labtable.elem[i]->ip == INVALID_ADDR) {
      err_comp_label_not_def(comp_labtable.elem[i]->name);
      return 0;
    }
  }

  return 1;
}

/*
 * returns true if 'name' is a unit or c-module
 */
int comp_check_lib(const char *name) {
  char tmp[SB_KEYWORD_SIZE + 1];char *p;
  int i;

  strcpy(tmp, name);
  p = strchr(tmp, '.');
  if (p) {
    *p = '\0';
    for (i = 0; i < comp_libcount; i++) {
      bc_lib_rec_t *lib = comp_libtable.elem[i];

      // remove any file path component from the name
      char *dir_sep = strrchr(lib->lib, OS_DIRSEP);
      char *lib_name = dir_sep ? dir_sep + 1 : lib->lib;

      if (strcasecmp(lib_name, tmp) == 0) {
        return 1;
      }
    }
  }
  return 0;
}

/**
 * create a new variable
 */
int comp_create_var(const char *name) {
  int idx = -1;
  if (!(is_alpha(name[0]) || name[0] == '_')) {
    sc_raise(MSG_WRONG_VARNAME, name);
  } else {
    // realloc table if it is needed
    if (comp_varcount >= comp_varsize) {
      comp_varsize += GROWSIZE;
      comp_vartable = realloc(comp_vartable, comp_varsize * sizeof(comp_var_t));
    }
    if (opt_verbose) {
      log_printf(MSG_NEW_VAR, comp_line, name, comp_varcount);
    }
    comp_vartable[comp_varcount].name = malloc(strlen(name) + 1);
    strcpy(comp_vartable[comp_varcount].name, name);
    comp_vartable[comp_varcount].dolar_sup = 0;
    comp_vartable[comp_varcount].lib_id = -1;
    comp_vartable[comp_varcount].local_id = -1;
    comp_vartable[comp_varcount].local_proc_level = 0;
    idx = comp_varcount;
    comp_varcount++;
  }
  return idx;
}

/**
 * add external variable
 */
int comp_add_external_var(const char *name, int lib_id) {
  int idx;

  idx = comp_create_var(name);
  comp_vartable[idx].lib_id = lib_id;

  if (lib_id & UID_UNIT_BIT) {
    // update imports table
    bc_symbol_rec_t *sym = add_imptable_rec(name, lib_id, stt_variable);
    sym->var_id = idx;
  }

  return idx;
}

/*
 * returns the id of the variable 'name'
 *
 * if there is no such variable then creates a new one
 *
 * if a new variable must created then if the var_name includes the path then
 * the new variable created at local space otherwise at globale space
 */
bid_t comp_var_getID(const char *var_name) {
  bid_t idx = -1, i;
  char tmp[SB_KEYWORD_SIZE + 1];
  char *name = comp_bc_temp;

  comp_prepare_name(tmp, baseof(var_name, '/'), SB_KEYWORD_SIZE);

  char *dot = strchr(tmp, '.');
  if (dot != NULL && *(dot + 1) == 0) {
    // name ends with dot
    sc_raise(MSG_MEMBER_DOES_NOT_EXIST, tmp);
    return 0;
  }
  //
  // check for external
  // external variables are recognized by the 'class' name
  // example: my_unit.my_var
  //
  // If the name is not found in comp_libtable then it
  // is treated as a structure reference
  if (dot != NULL && comp_check_lib(tmp)) {
    for (i = 0; i < comp_varcount; i++) {
      if (strcasecmp(comp_vartable[i].name, tmp) == 0) {
        return i;
      }
    }

    sc_raise(MSG_MEMBER_DOES_NOT_EXIST, tmp);
    return 0;
  }
  //
  // search in global name-space
  //
  // Note: local space is dynamic,
  // however a global var-ID per var-name is required
  //
  strcpy(name, tmp);

  for (i = 0; i < comp_varcount; i++) {
    if (strcmp(comp_vartable[i].name, name) == 0) {
      idx = i;
      break;
    }

    if (comp_vartable[i].dolar_sup) {
      // system variables must be visible with or without '$' suffix
      char *dollar_name = malloc(strlen(comp_vartable[i].name) + 2);
      strcpy(dollar_name, comp_vartable[i].name);
      strcat(dollar_name, "$");
      if (strcmp(dollar_name, name) == 0) {
        idx = i;
        free(dollar_name);
        break;
      }
      free(dollar_name);
    }
  }

  if (opt_autolocal) {
    if (idx == -1) {
      idx = comp_create_var(tmp);
      if (comp_bc_proc[0]) {
        comp_vartable[idx].local_id = idx;
        comp_vartable[idx].local_proc_level = comp_proc_level;
      } else {
        comp_vartable[idx].local_id = -1;
      }
    } else if (comp_bc_proc[0] &&
               comp_vartable[idx].local_id != -1 &&
               comp_vartable[idx].local_proc_level > comp_proc_level) {
      // local at higher level now also local at this level
      comp_vartable[idx].local_proc_level = comp_proc_level;
    }
  } else if (idx == -1) {
    // variable not found; create a new one
    idx = comp_create_var(tmp);
  }
  return idx;
}

/*
 * add the named variable to the current position in the byte code stream
 *
 * if the name 'foo' has already been used in a struct context, eg 'foo.x'
 * then the foo variable is added as kwTYPE_UDS.
 *
 */
void comp_add_variable(bc_t *bc, const char *var_name) {
  char *dot = strchr(var_name, '.');

  if (dot != NULL && !comp_check_lib(var_name)) {
    // uds-element (or sub-element eg foo.x.y.z)
    // record the uds-parent

    int len = dot - var_name;
    comp_struct_t uds;
    strncpy(uds.name, var_name, len);
    uds.name[len] = 0;

    bid_t var_id = comp_var_getID(uds.name);
    bc_add_code(bc, kwTYPE_VAR);
    bc_add_addr(bc, var_id);

    while (dot && dot[0]) {
      char *dot_end = strchr(dot + 1, '.');
      if (dot_end) {
        // next sub-element
        len = (dot_end - dot) - 1;
      } else {
        // final element
        len = strlen(dot + 1);
      }

      bc_add_code(bc, kwTYPE_UDS_EL);
      bc_add_strn(bc, dot + 1, len);

      if (dot_end) {
        dot = dot_end;
      } else {
        dot = NULL;
      }
    }
  } else {
    // regular variable or uds-container
    bc_add_code(bc, kwTYPE_VAR);
    bc_add_addr(bc, comp_var_getID(var_name));
  }
}

/*
 * adds a mark in stack at the current code position
 */
void comp_push(bcip_t ip) {
  comp_pass_node_t *node = (comp_pass_node_t *)malloc(sizeof(comp_pass_node_t));
  memset(node, 0, sizeof(comp_pass_node_t));

  strcpy(node->sec, comp_bc_sec);
  node->pos = ip;
  node->level = comp_block_level;
  node->block_id = comp_block_id;
  node->line = comp_line;

  if (comp_stack.count == comp_stack.size) {
    comp_stack.size += GROWSIZE;
    comp_stack.elem = realloc(comp_stack.elem, comp_stack.size * sizeof(comp_pass_node_t *));
  }

  comp_stack.elem[comp_stack.count] = node;
  comp_stack.count++;
}

/*
 * returns the keyword code
 */
int comp_is_keyword(const char *name) {
  int i;
  byte dolar_sup = 0;

  if (name == NULL || name[0] == '\0') {
    return -1;
  }

  // Code to enable the $ but not for keywords (INKEY$=INKEY,
  // PRINT$=PRINT !!!)
  // I don't want to increase the size of keywords table.
  int idx = strlen(name) - 1;
  if (name[idx] == '$') {
    *((char *)(name + idx)) = '\0';
    dolar_sup++;
  }

  for (i = 0; keyword_table[i].name[0] != '\0'; i++) {
    if (strcmp(keyword_table[i].name, name) == 0) {
      return keyword_table[i].code;
    }
  }

  if (dolar_sup) {
    *((char *)(name + idx)) = '$';
  }
  return -1;
}

/*
 * returns the keyword code (buildin functions)
 */
bid_t comp_is_func(const char *name) {
  int i;
  byte dolar_sup = 0;

  if (name == NULL || name[0] == '\0') {
    return -1;
  }

  // Code to enable the $ but not for keywords (INKEY$=INKEY,
  // PRINT$=PRINT !!!)
  // I don't want to increase the size of keywords table.
  int idx = strlen(name) - 1;
  if (name[idx] == '$') {
    *((char *)(name + idx)) = '\0';
    dolar_sup++;
  }

  for (i = 0; func_table[i].name[0] != '\0'; i++) {
    if (strcmp(func_table[i].name, name) == 0) {
      return func_table[i].fcode;
    }
  }

  if (dolar_sup) {
    *((char *)(name + idx)) = '$';
  }

  return -1;
}

/*
 * returns the keyword code (buildin procedures)
 */
bid_t comp_is_proc(const char *name) {
  bid_t i;

  for (i = 0; proc_table[i].name[0] != '\0'; i++) {
    if (strcmp(proc_table[i].name, name) == 0) {
      return proc_table[i].pcode;
    }
  }

  return -1;
}

/*
 * returns the keyword code (special separators)
 */
int comp_is_special_operator(const char *name) {
  int i;

  for (i = 0; spopr_table[i].name[0] != '\0'; i++) {
    if (strcmp(spopr_table[i].name, name) == 0) {
      return spopr_table[i].code;
    }
  }

  return -1;
}

/*
 * returns the keyword code (operators)
 */
int comp_is_operator(const char *name) {
  int i;

  for (i = 0; opr_table[i].name[0] != '\0'; i++) {
    if (strcmp(opr_table[i].name, name) == 0) {
      return ((opr_table[i].code << 8) | opr_table[i].opr);
    }
  }

  return -1;
}

/*
 */
char *comp_next_char(char *source) {
  char *p = source;

  while (*p) {
    if (*p != ' ') {
      return p;
    }
    p++;
  }
  return p;
}

/**
 * get next word
 * if buffer's len is zero, then the next element is not a word
 *
 * @param text the source
 * @param dest the buffer to store the result
 * @return pointer of text to the next element
 */
const char *comp_next_word(const char *text, char *dest) {
  const char *p = text;
  char *d = dest;

  if (p == NULL) {
    *dest = '\0';
    return 0;
  }

  while (is_space(*p)) {
    p++;
  }
  if (*p == '?') {
    strcpy(dest, LCN_PRINT);
    p++;
    while (is_space(*p)) {
      p++;
    }
    return p;
  }

  if (*p == '\'' || *p == '#') {
    strcpy(dest, LCN_REM);
    p++;
    while (is_space(*p)) {
      p++;
    }
    return p;
  }

  if (is_alnum(*p) || *p == '_') {
    // don't forget the numeric-labels
    while (is_alnum(*p) || (*p == '_') || (*p == '.')) {
      *d = *p;
      d++;
      p++;
    }
  }
  // Code to kill the $
  // if ( *p == '$' )
  // p ++;
  // Code to enable the $
  if (*p == '$') {
    *d++ = *p++;
  }
  *d = '\0';
  while (is_space(*p)) {
    p++;
  }
  return p;
}

/*
 * skips past any leading empty "()" parentheses characters
 */
char *trim_empty_parentheses(char *text) {
  char *result = text;
  char *next = comp_next_char(text);
  if (*next == '(') {
    next = comp_next_char(next + 1);
    if (*next == ')') {
      result = comp_next_char(next + 1);
    }
  }
  return result;
}

/*
 * return whether the given name is a func or sub
 */
int comp_is_function(char *name) {
  int result = 0;
  if (comp_is_proc(name) != -1 ||
      comp_is_func(name) != -1 ||
      comp_is_external_proc(name) != -1 ||
      comp_is_external_func(name) != -1 ||
      comp_udp_id(comp_bc_name, 1) != -1) {
    result = 1;
  }
  return result;
}

/*
 * return whether the name is enclosed with parenthesis characters
 */
int comp_is_parenthesized(char *name) {
  int result = 0;
  char *p = comp_next_char(name);
  if (*p == '(') {
    char last = *p;
    while (*p) {
      if (*p != ' ') {
        last = *p;
      }
      p++;
    }
    result = (last == ')');
  }
  return result;
}

/*
 * return whether the following code is a code array declaration
 *
 * returns true for empty brackets []
 */
int comp_is_code_array(bc_t *bc, char *p) {
  int result = 0;
  if (comp_prog.ptr[comp_prog.count - 1] == '=' &&
      (!bc->count || bc->ptr[0] != kwTYPE_VAR)) {
    // variable assignment is always for code array
    result = 1;
  } else {
    int level = 1;
    int count = 0;
    while (*p && level) {
      switch(*p) {
      case '[':
        level++;
        break;
      case ']':
        level--;
        break;
      case ',':
      case ';':
        result = 1;
        break;
      default:
        count++;
        break;
      }
      p++;
    }
    if (!count) {
      result = 1;
    }
  }
  return result;
}

char *comp_scan_json(char *json, bc_t *bc) {
  int curley_brace = 1;
  char *p = json + 1;

  while (*p != '\0' && curley_brace > 0) {
    switch (*p) {
    case '{':
      curley_brace++;
      break;
    case '}':
      curley_brace--;
      break;
    case V_QUOTE:
      // revert hidden quote
      *p = '\"';
      break;
    case V_LINE:
      // revert hidden newline
      comp_line++;
      *p = '\n';
      break;
    default:
      break;
    }
    p++;
  }
  if (curley_brace == 0) {
    bc_add_fcode(bc, kwARRAY);
    bc_add_code(bc, kwTYPE_LEVEL_BEGIN);
    bc_add_strn(bc, json, p - json);
    bc_add_code(bc, kwTYPE_LEVEL_END);
  } else {
    err_comp_missing_rb();
  }
  return p;
}

/*
 * scan expression
 */
void comp_expression(char *expr, byte no_parser) {
  char *ptr = (char *)expr;
  int level = 0, check_udf = 0;
  int kw_exec_more = 0;
  var_int_t lv = 0;
  var_num_t dv = 0;
  int addr_opr = 0;
  bc_t bc;

  comp_use_global_vartable = 0; // check local-variables first
  str_alltrim(expr);
  if (*ptr == '\0') {
    return;
  }

  bc_create(&bc);

  while (*ptr) {
    if (is_digit(*ptr) || *ptr == '.' || (*ptr == '&' && strchr("XHOB", *(ptr + 1)))) {
      // a constant number
      int tp;
      ptr = get_numexpr(ptr, comp_bc_name, &tp, &lv, &dv);
      switch (tp) {
      case 1:
        bc_add_cint(&bc, lv);
        continue;
      case 2:
        bc_add_creal(&bc, dv);
        continue;
      default:
        sc_raise(MSG_EXP_GENERR);
      }
    } else if (*ptr == '\'') {
      // remarks
      break;
    } else if (is_alpha(*ptr) || *ptr == '?' || *ptr == '_') {
      // a name
      ptr = (char *)comp_next_word(ptr, comp_bc_name);
      bid_t idx = comp_is_func(comp_bc_name);
      // special case for input
      if (idx == kwINPUTF) {
        if (*comp_next_char(ptr) != '(') {
          // INPUT is special separator (OPEN...FOR INPUT...)
          idx = -1;
        }
      }

      if (idx != -1) {
        // is a function
        if (!kw_noarg_func(idx)) {
          if (*comp_next_char(ptr) != '(') {
            sc_raise(MSG_BF_ARGERR, comp_bc_name);
          }
        }
        if (idx == kwCALLCF) {
          bc_add_code(&bc, kwTYPE_CALL_UDF);
          bc_add_addr(&bc, idx);  // place holder
          bc_add_addr(&bc, 0);  // return-variable ID
          bc_add_code(&bc, kwTYPE_LEVEL_BEGIN);
          bc_add_code(&bc, kwTYPE_CALL_PTR);
          // next is address
          // skip next '(' since we already added kwTYPE_LEVEL_BEGIN
          // to allow kwTYPE_CALL_PTR to be the next code
          char *par = comp_next_char(ptr);
          if (*par == '(') {
            ptr = par + 1;
            level++;
          }
        } else {
          bc_add_fcode(&bc, idx);
        }
        check_udf++;
      } else {
        // check special separators
        idx = comp_is_special_operator(comp_bc_name);
        if (idx != -1) {
          if (idx == kwUSE) {
            bc_add_code(&bc, idx);
            bc_add_addr(&bc, 0);
            bc_add_addr(&bc, 0);
            comp_use_global_vartable = 1;
            // all the next variables are global (needed for X)
            check_udf++;
          } else if (idx == kwDO) {
            SKIP_SPACES(ptr);
            if (strlen(ptr)) {
              if (strlen(comp_do_close_cmd)) {
                kw_exec_more = 1;
                strcpy(comp_bc_tmp2, ptr);
                strcat(comp_bc_tmp2, ":");
                strcat(comp_bc_tmp2, comp_do_close_cmd);
                strcpy(comp_do_close_cmd, "");
              } else {
                sc_raise(MSG_KEYWORD_DO_ERR);
              }
            }
            break;
          } else {
            bc_add_code(&bc, idx);
          }
        } else {
          // not a command, check operators
          idx = comp_is_operator(comp_bc_name);
          if (idx != -1) {
            bc_add_code(&bc, idx >> 8);
            bc_add_code(&bc, idx & 0xFF);
          } else {
            // external function
            idx = comp_is_external_func(comp_bc_name);
            if (idx != -1) {
              bc_add_extfcode(&bc, comp_extfunctable[idx].lib_id, comp_extfunctable[idx].symbol_index);
            } else {
              idx = comp_is_keyword(comp_bc_name);
              if (idx == -1) {
                idx = comp_is_proc(comp_bc_name);
              }
              if (idx == kwBYREF) {
                bc_add_code(&bc, kwBYREF);
              } else if (idx != -1) {
                sc_raise(MSG_STATEMENT_ON_RIGHT, comp_bc_name);
              } else {
                // udf or variable
                int udf = comp_udp_id(comp_bc_name, 1);
                if (udf != -1) {
                  // udf
                  if (addr_opr != 0) {
                    // pointer to UDF
                    bc_add_code(&bc, kwTYPE_PTR);
                  } else {
                    bc_add_code(&bc, kwTYPE_CALL_UDF);
                  }
                  check_udf++;
                  bc_add_addr(&bc, udf);
                  bc_add_addr(&bc, 0);  // var place holder
                  ptr = trim_empty_parentheses(ptr);
                } else {
                  // variable
                  if (addr_opr != 0) {
                    bc_add_code(&bc, kwBYREF);
                  }
                  SKIP_SPACES(ptr);
                  comp_add_variable(&bc, comp_bc_name);
                  if (ptr[0] == '(' && ptr[1] == ')'
                      && strchr(comp_bc_name, '.') == NULL) {
                    // null array on non UDS
                    ptr += 2;
                  } else if (ptr[0] == '[') {
                    // array element using '['
                    ptr++;
                    level++;
                    bc_add_code(&bc, kwTYPE_LEVEL_BEGIN);
                  }
                }
              }
            }
          }
        }
      }
      addr_opr = 0;
      // end isalpha block
    } else if (*ptr == ',' || *ptr == ';' || *ptr == '#') {
      // parameter separator
      bc_add_code(&bc, kwTYPE_SEP);
      bc_add_code(&bc, *ptr);
      ptr++;
    } else if (*ptr == '\"') {
      // string
      ptr = bc_store_string(&bc, ptr);
    } else if (*ptr == '[') {
      // code-defined array
      ptr++;
      level++;
      if (comp_is_code_array(&bc, ptr)) {
        // otherwise treat as array index
        bc_add_fcode(&bc, kwCODEARRAY);
      }
      bc_add_code(&bc, kwTYPE_LEVEL_BEGIN);
    } else if (*ptr == '(') {
      // parenthesis
      ptr++;
      level++;
      bc_add_code(&bc, kwTYPE_LEVEL_BEGIN);
    } else if (*ptr == ')' || *ptr == ']') {
      // parenthesis
      bc_add_code(&bc, kwTYPE_LEVEL_END);
      level--;
      ptr++;
      if (*ptr == '.') {
        ptr = comp_array_uds_field(ptr + 1, &bc);
      }
    } else if (*ptr == '{') {
      ptr = comp_scan_json(ptr, &bc);
      check_udf++;
    } else if (*ptr == V_LINE) {
      comp_line++;
      ptr++;
    } else if (is_space(*ptr)) {
      // null characters
      ptr++;
    } else {
      // operators
      if (*ptr == '+' || *ptr == '-') {
        bc_add_code(&bc, kwTYPE_ADDOPR);
        bc_add_code(&bc, *ptr);
      } else if (*ptr == '*' || *ptr == '/' || *ptr == '\\' || *ptr == '%') {
        bc_add_code(&bc, kwTYPE_MULOPR);
        bc_add_code(&bc, *ptr);
      } else if (*ptr == '^') {
        bc_add_code(&bc, kwTYPE_POWOPR);
        bc_add_code(&bc, *ptr);
      } else if (strncmp(ptr, "<=", 2) == 0 || strncmp(ptr, "=<", 2) == 0) {
        bc_add_code(&bc, kwTYPE_CMPOPR);
        bc_add_code(&bc, OPLOG_LE);
        ptr++;
      } else if (strncmp(ptr, ">=", 2) == 0 || strncmp(ptr, "=>", 2) == 0) {
        bc_add_code(&bc, kwTYPE_CMPOPR);
        bc_add_code(&bc, OPLOG_GE);
        ptr++;
      } else if (strncmp(ptr, "<>", 2) == 0 || strncmp(ptr, "!=", 2) == 0) {
        bc_add_code(&bc, kwTYPE_CMPOPR);
        bc_add_code(&bc, OPLOG_NE);
        ptr++;
      } else if (strncmp(ptr, "<<", 2) == 0) {
        ptr += 2;
        SKIP_SPACES(ptr);
        if (strlen(ptr)) {
          kw_exec_more = 1;
          strcpy(comp_bc_tmp2, comp_bc_name);
          strcat(comp_bc_tmp2, " << ");
          strcat(comp_bc_tmp2, ptr);
        } else {
          sc_raise(MSG_OPR_APPEND_ERR);
        }
        break;
      } else if (strncmp(ptr, "==", 2) == 0) {
        // support == syntax to prevent java or c programmers
        // getting used to single = thus causing embarrasing
        // coding errors in their normal work :)
        bc_add_code(&bc, kwTYPE_CMPOPR);
        bc_add_code(&bc, *ptr);
        ptr++;
      } else if (*ptr == '=' || *ptr == '>' || *ptr == '<') {
        bc_add_code(&bc, kwTYPE_CMPOPR);
        bc_add_code(&bc, *ptr);
      } else if (strncmp(ptr, "&&", 2) == 0 || strncmp(ptr, "||", 2) == 0) {
        bc_add_code(&bc, kwTYPE_LOGOPR);
        bc_add_code(&bc, *ptr);
        ptr++;
      } else if (*ptr == '&') {
        bc_add_code(&bc, kwTYPE_LOGOPR);
        bc_add_code(&bc, OPLOG_BAND);
      } else if (*ptr == '|') {
        bc_add_code(&bc, kwTYPE_LOGOPR);
        bc_add_code(&bc, OPLOG_BOR);
      } else if (*ptr == '~') {
        bc_add_code(&bc, kwTYPE_UNROPR);
        bc_add_code(&bc, OPLOG_INV);
      } else if (*ptr == '!') {
        bc_add_code(&bc, kwTYPE_UNROPR);
        bc_add_code(&bc, *ptr);
      } else if (*ptr == '@') {
        addr_opr = 1;
      } else {
        sc_raise(MSG_WRONG_OPR, *ptr);
      }
      ptr++;
    }
  };

  if (level) {
    sc_raise(MSG_EXP_MIS_RP);
  }
  if (!comp_error) {
    if (no_parser == 0) {
      // optimization
      bc_eoc(&bc);
      // printf("=== before:\n"); hex_dump(bc.ptr, bc.count);
      expr_parser(&bc);
      // printf("=== after:\n"); hex_dump(bc.ptr, bc.count);
    }
    if (bc.count) {
      bcip_t stip = comp_prog.count;
      bc_append(&comp_prog, &bc); // merge code segments

      // update pass2 stack-nodes
      if (check_udf) {
        bcip_t cip = stip;
        while ((cip = comp_search_bc(cip, kwUSE)) != INVALID_ADDR) {
          comp_push(cip);
          cip += (1 + ADDRSZ + ADDRSZ);
        }

        cip = stip;
        while ((cip = comp_search_bc(cip, kwTYPE_CALL_UDF)) != INVALID_ADDR) {
          comp_push(cip);
          cip += (1 + ADDRSZ + ADDRSZ);
        }

        cip = stip;
        while ((cip = comp_search_bc(cip, kwTYPE_PTR)) != INVALID_ADDR) {
          comp_push(cip);
          cip += (1 + ADDRSZ + ADDRSZ);
        }
      }
    }

    bc_eoc(&comp_prog);
  }
  // clean-up
  comp_use_global_vartable = 0; // check local-variables first
  bc_destroy(&bc);

  // do additional steps
  if (kw_exec_more) {
    comp_text_line(comp_bc_tmp2, 0);
  }
}

/*
 * Converts DATA commands to bytecode
 */
void comp_data_seg(char *source) {
  char *ptr = source;
  char *commap;
  var_int_t lv = 0;
  var_num_t dv = 0;
  double sign = 1;
  char *tmp = comp_bc_temp;
  int quotes;
  int tp;

  while (*ptr) {
    SKIP_SPACES(ptr);

    if (*ptr == '\0') {
      break;
    } else if (*ptr == ',') {
      bc_eoc(&comp_data);
      ptr++;
    } else {
      // find the end of the element
      commap = ptr;
      quotes = 0;
      while (*commap) {
        if (*commap == '\"') {
          quotes = !quotes;
        } else if ((*commap == ',') && (quotes == 0)) {
          break;
        }
        commap++;
      }
      if (*commap == '\0') {
        commap = NULL;
      }
      if (commap != NULL) {
        *commap = '\0';
      }
      if ((*ptr == '-' || *ptr == '+') && strchr("0123456789.", *(ptr + 1))) {
        if (*ptr == '-') {
          sign = -1;
        }
        ptr++;
      } else {
        sign = 1;
      }
      if (is_digit(*ptr) || *ptr == '.' || (*ptr == '&' && strchr("XHOB", *(ptr + 1)))) {
        // number - constant
        ptr = get_numexpr(ptr, tmp, &tp, &lv, &dv);
        switch (tp) {
        case 1:
          bc_add_cint(&comp_data, lv * sign);
          break;
        case 2:
          bc_add_creal(&comp_data, dv * sign);
          break;
        default:
          sc_raise(MSG_EXP_GENERR);
        }
      } else {
        // add it as string
        if (*ptr != '\"') {
          strcpy(tmp, "\"");
          strcat(tmp, ptr);
          strcat(tmp, "\"");
          bc_store_string(&comp_data, tmp);
          if (commap) {
            ptr = commap;
          } else {
            ptr = ptr + strlen(ptr);
          }
        } else {
          ptr = bc_store_string(&comp_data, ptr);
        }
      }

      if (commap != NULL) {
        *commap = ',';
      }
    }
  }

  bc_eoc(&comp_data);
}

/*
 * Scans the 'source' for "names" separated by ',' and returns
 * the elements (pointer in source) into args array.
 *
 * Returns the number of items
 */
int comp_getlist(char *source, char_p_t *args, int maxarg) {
  int count = 0;
  char *p = source;
  char *ps = p;
  int square = 0;
  int round = 0;
  int brace = 0;

  while (*p && count < maxarg) {
    switch (*p) {
    case '[':
      square++;
      break;
    case ']':
      square--;
      break;
    case '(':
      round++;
      break;
    case ')':
      round--;
      break;
    case '{':
      brace++;
      break;
    case '}':
      brace--;
      break;
    case ',':
      if (!square && !round && !brace) {
        *p = '\0';
        SKIP_SPACES(ps);
        args[count] = ps;
        count++;
        ps = p + 1;
      }
      break;
    default:
      break;
    }
    p++;
  }

  if (*ps) {
    if (count == maxarg) {
      sc_raise(MSG_PARNUM_LIMIT, maxarg);
    } else {
      SKIP_SPACES(ps);
      if (*ps) {
        *p = '\0';
        args[count] = ps;
        count++;
      }
    }
  }
  return count;
}

/*
 * returns a list of names
 *
 * the list is included between sep[0] and sep[1] characters
 * each element is separated by ',' characters
 *
 * the 'source' is the raw string (null chars will be placed at the end of each name)
 * the 'args' is the names (pointers on the 'source')
 * maxarg is the maximum number of names (actually the size of args)
 * the count is the number of names which are found by this routine.
 *
 * returns the next position in 'source' (after the sep[1])
 */
char *comp_getlist_insep(char *source, char_p_t *args, char *sep, int maxarg, int *count) {
  char *p;
  char *ps;
  int level = 1;

  *count = 0;
  p = strchr(source, sep[0]);

  if (p) {
    ps = p + 1;
    p++;

    while (*p) {
      if (*p == sep[1]) {
        level--;
        if (level == 0) {
          break;
        }
      } else if (*p == sep[0]) {
        level++;
      }
      p++;
    }

    if (*p == sep[1]) {
      *p = '\0';
      if (strlen(ps)) {
        SKIP_SPACES(ps);
        if (strlen(ps)) {
          *count = comp_getlist(ps, args, maxarg);
        } else {
          sc_raise(MSG_NIL_PAR_ERR);
        }
      }
    } else {
      sc_raise(MSG_MISSING_CHAR, sep[1]);
    }
  } else {
    p = source;
  }
  return p;
}

/*
 * Single-line IFs
 *
 * converts the string from single-line IF to normal IF syntax
 * returns true if there is a single-line IF.
 *
 * IF expr THEN ... ---> IF expr THEN (:) .... (:FI)
 * IF expr THEN ... ELSE ... ---> IF expr THEN (:) .... (:ELSE:) ... (:FI)
 */
int comp_single_line_if(char *text) {
  // *text points to 'expr'
  char *p = (char *)text;
  char *pthen, *pelse;
  char buf[SB_SOURCELINE_SIZE + 1];

  if (comp_error) {
    return 0;
  }
  pthen = p;
  do {
    pthen = strstr(pthen + 1, LCN_THEN_WS);
    if (pthen) {
      // store the expression
      SKIP_SPACES(p);
      strcpy(buf, p);
      p = strstr(buf, LCN_THEN_WS);
      *p = '\0';

      // check for ':'
      p = pthen + 6;
      SKIP_SPACES(p);

      if (*p != ':' && *p != '\0') {
        // store the IF
        comp_block_level++;
        comp_block_id++;
        comp_push(comp_prog.count);
        bc_add_ctrl(&comp_prog, kwIF, 0, 0);

        comp_expression(buf, 0);
        if (comp_error) {
          return 0;
        }
        // store EOC
        bc_eoc(&comp_prog);

        // auto-goto
        p = pthen + 6;
        SKIP_SPACES(p);

        if (is_digit(*p)) {
          // add goto
          strcpy(buf, LCN_GOTO_WRS);
          strcat(buf, p);
        } else {
          strcpy(buf, p);
        }
        // ELSE command
        // If there are more inline-ifs (nested) the ELSE belongs
        // to the first IF (that's an error)
        pelse = strstr(buf + 1, LCN_ELSE);
        if (pelse) {
          do {
            if ((*(pelse - 1) == ' ' || *(pelse - 1) == '\t') &&
                (*(pelse + 4) == ' ' || *(pelse + 4) == '\t')) {
              *pelse = '\0';

              // scan the commands before ELSE
              comp_text_line(buf, 0);
              // add EOC
              bc_eoc(&comp_prog);

              // auto-goto
              strcpy(buf, LCN_ELSE);
              strcat(buf, ":");
              p = pelse + 4;
              SKIP_SPACES(p);
              if (is_digit(*p)) {
                // add goto
                strcat(buf, LCN_GOTO_WRS);
                memcat(buf, p);
              } else {
                memcat(buf, p);
              }
              break;
            } else {
              pelse = strstr(pelse + 1, LCN_ELSE);
            }
          } while (pelse != NULL);
        }
        // scan the remaining commands
        comp_text_line(buf, 0);
        // add EOC
        bc_eoc(&comp_prog);

        // add ENDIF
        comp_push(comp_prog.count);
        bc_add_ctrl(&comp_prog, kwENDIF, 0, 0);
        comp_block_level--;
        comp_block_id--;
        return 1;
      } else {
        // *p == ':'
        return 0;
      }
    } else {
      break;
    }
  } while (pthen != NULL);
  return 0;
}

/**
 * Referencing a UDS field via array, eg foo(10).x
 */
char *comp_array_uds_field(char *p, bc_t *bc) {
  char *p_begin = p;

  while (1) {
    if (*p == 0 || (*p != '_' && !isalnum(*p))) {
      int len = (p - p_begin);
      if (len) {
        bc_add_code(bc, kwTYPE_UDS_EL);
        bc_add_strn(bc, p_begin, len);
      }
      if (*p == '.') {
        p_begin = p + 1;
      } else {
        return p;
      }
    }
    p++;
  }

  return p;
}

/*
 * array's args
 */
char *comp_array_params(char *src, char exitChar) {
  char *p = src;
  char *ss = NULL;
  char *se = NULL;
  char closeBracket = '\0';
  int level = 0;

  while (*p) {
    switch (*p) {
    case '[':
    case '(':
      if (level == 0) {
        ss = p;
      }
      level++;
      closeBracket = *p == '(' ? ')' : ']';
      break;
    case ')':
    case ']':
      if (closeBracket == *p) {
        level--;
        if (level == 0) {
          se = p;
          // store this index
          if (!ss) {
            sc_raise(MSG_ARRAY_SE);
          } else {
            char ssSave = *ss;
            char seSave = *se;
            *ss = ' ';
            *se = '\0';
            bc_add_code(&comp_prog, kwTYPE_LEVEL_BEGIN);
            comp_expression(ss, 0);
            bc_store1(&comp_prog, comp_prog.count - 1, kwTYPE_LEVEL_END);
            *ss = ssSave;
            *se = seSave;
            ss = se = NULL;
          }
          if (*(p + 1) == '.') {
            p = comp_array_uds_field(p + 2, &comp_prog);
          }
        }
      }
      break;
    };
    if (*p != exitChar) {
      p++;
    }
    if (*p == exitChar) {
      p++;
      break;
    }
  }
  if (level > 0) {
    sc_raise(MSG_ARRAY_MIS_RP);
  } else if (level < 0) {
    sc_raise(MSG_ARRAY_MIS_LP);
  }
  return p;
}

/*
 * run-time options
 */
void comp_cmd_option(char *src) {
  char *p = src;

  if (CHKOPT(LCN_UICS_WRS)) {
    bc_add_code(&comp_prog, kwOPTION);
    bc_add_code(&comp_prog, OPTION_UICS);

    p += 5;
    while (is_space(*p)) {
      p++;
    }
    if (CHKOPT(LCN_CHARS)) {
      bc_add_addr(&comp_prog, OPTION_UICS_CHARS);
    } else if (CHKOPT(LCN_PIXELS)) {
      bc_add_addr(&comp_prog, OPTION_UICS_PIXELS);
    } else {
      sc_raise(MSG_OPT_UICS_ERR);
    }
  } else if (CHKOPT(LCN_BASE_WRS)) {
    bc_add_code(&comp_prog, kwOPTION);
    bc_add_code(&comp_prog, OPTION_BASE);
    bc_add_addr(&comp_prog, xstrtol(src + 5));
  } else if (CHKOPT(LCN_PCRE_CASELESS)) {
    bc_add_code(&comp_prog, kwOPTION);
    bc_add_code(&comp_prog, OPTION_MATCH);
    bc_add_addr(&comp_prog, 2);
  } else if (CHKOPT(LCN_PCRE)) {
    bc_add_code(&comp_prog, kwOPTION);
    bc_add_code(&comp_prog, OPTION_MATCH);
    bc_add_addr(&comp_prog, 1);
  } else if (CHKOPT(LCN_SIMPLE)) {
    bc_add_code(&comp_prog, kwOPTION);
    bc_add_code(&comp_prog, OPTION_MATCH);
    bc_add_addr(&comp_prog, 0);
  } else if (CHKOPT(LCN_PREDEF_WRS) || CHKOPT(LCN_IMPORT_WRS)) {
    // ignored
  } else {
    sc_raise(MSG_OPTION_ERR, src);
  }
}

int comp_error_if_keyword(const char *name) {
  // check if keyword
  if (!comp_error) {
    if ((comp_is_func(name) >= 0) || (comp_is_proc(name) >= 0) || (comp_is_special_operator(name) >= 0)
        || (comp_is_keyword(name) >= 0) || (comp_is_operator(name) >= 0)) {
      sc_raise(MSG_IT_IS_KEYWORD, name);
    }
  }
  return comp_error;
}

/**
 * stores export symbols (in pass2 will be checked again)
 */
void bc_store_exports(const char *slist) {
  char_p_t pars[MAX_PARAMS];
  int count = 0, i;
  char *newlist;

  newlist = (char *)malloc(strlen(slist) + 3);
  strcpy(newlist, "(");
  strcat(newlist, slist);
  strcat(newlist, ")");

  comp_getlist_insep(newlist, pars, "()", MAX_PARAMS, &count);

  int offset;
  if (comp_exptable.count) {
    offset = comp_exptable.count;
    comp_exptable.count += count;
    comp_exptable.elem = (unit_sym_t **)realloc(comp_exptable.elem,
                                                comp_exptable.count * sizeof(unit_sym_t **));
  } else {
    offset = 0;
    comp_exptable.count = count;
    comp_exptable.elem = (unit_sym_t **)malloc(comp_exptable.count * sizeof(unit_sym_t **));
  }

  for (i = 0; i < count; i++) {
    unit_sym_t *sym = (unit_sym_t *)malloc(sizeof(unit_sym_t));
    memset(sym, 0, sizeof(unit_sym_t));
    strcpy(sym->symbol, pars[i]);
    comp_exptable.elem[offset + i] = sym;
  }

  free(newlist);
}

void comp_get_unary(const char *p, int *ladd, int *linc, int *ldec, int *leqop) {
  *ladd = (strncmp(p, "<<", 2) == 0);
  *linc = (strncmp(p, "++", 2) == 0);
  *ldec = (strncmp(p, "--", 2) == 0);
  if (p[0] != '\0' && p[1] == '=' && strchr("-+/\\*^%&|", p[0])) {
    *leqop = p[0];
  } else {
    *leqop = 0;
  }
}

void comp_text_line_let(bid_t idx, int ladd, int linc, int ldec, int leqop) {
  char *p;
  char *parms = comp_bc_parm;
  char *array_index = NULL;
  int array_index_len = 0;
  int v_func = 0;
  char closeBracket = '\0';

  if (parms[0] == '(' || parms[0] == '[') {
    int level = 0;
    p = parms;
    while (*p) {
      switch(*p) {
      case '[':
        level++;
        closeBracket = ']';
        break;
      case '(':
        level++;
        closeBracket = ')';
        break;
      case ']':
        if (closeBracket == ']') {
          level--;
        }
        break;
      case ')':
        if (closeBracket == ')') {
          level--;
        }
        break;
      case '.':
        // advance beyond UDS element
        p++;
        while (*p == '_' || isalnum(*p)) {
          p++;
        }
        p--;
        break;
      }
      p++;
      if (level == 0 && *p != '[' && *p != '(' && *p != '.') {
        break;
      }
    }
    if (level == 0) {
      p = comp_next_char(p);
      if (*p != '=') {
        // array(n) unary-operator
        array_index_len = (p - parms);
        array_index = malloc(array_index_len + 1);
        memcpy(array_index, parms, array_index_len);
        array_index[array_index_len] = '\0';

        // store plain operator in comp_bc_parm
        int len = strlen(p);
        memmove(comp_bc_parm, p, len);
        comp_bc_parm[len] = '\0';

        comp_get_unary(comp_bc_parm, &ladd, &linc, &ldec, &leqop);
      } else if (idx == -1 && !comp_bc_name[0]) {
        // packed assignment - (a,b,c) = [1,2,3]
        bc_add_code(&comp_prog, kwPACKED_LET);
        comp_array_params(parms, '=');
        comp_expression(p + 1, 0);
        return;
      }
    }
  }

  if (idx == kwCONST) {
    // const a=10: b=10
    p = (char *)comp_next_word(comp_bc_parm, comp_bc_name);
    p = get_param_sect(p, ":", comp_bc_parm);
    parms = comp_bc_parm;
    bc_add_code(&comp_prog, kwCONST);
  } else if (ladd) {
    bc_add_code(&comp_prog, kwAPPEND);
    parms += 2;
  } else if (linc) {
    bc_add_code(&comp_prog, kwLET);
    strcpy(comp_bc_parm, "=");
    strcat(comp_bc_parm, comp_bc_name);
    if (array_index != NULL) {
      strcat(comp_bc_parm, array_index);
    }
    strcat(comp_bc_parm, "+1");
  } else if (ldec) {
    bc_add_code(&comp_prog, kwLET);
    strcpy(comp_bc_parm, "=");
    strcat(comp_bc_parm, comp_bc_name);
    if (array_index != NULL) {
      strcat(comp_bc_parm, array_index);
    }
    strcat(comp_bc_parm, "-1");
  } else if (leqop) {
    // a += 10: b -= 10 etc
    bc_add_code(&comp_prog, kwLET);
    int len = strlen(comp_bc_parm) + strlen(comp_bc_name) + 1;
    if (array_index != NULL) {
      len += array_index_len;
    }
    char *buf = malloc(len + 1);
    memset(buf, 0, len);
    strcpy(buf, "=");
    strcat(buf, comp_bc_name);
    if (array_index != NULL) {
      strcat(buf, array_index);
    }
    buf[strlen(buf)] = leqop;
    strcat(buf, comp_bc_parm + 2);
    strcpy(comp_bc_parm, buf);
    free(buf);
  } else if (idx != kwLET
             && array_index != NULL
             && strchr(comp_bc_name, '.') != NULL) {
    // no unary operator found with array index
    v_func = 1;
    bc_add_pcode(&comp_prog, kwTYPE_CALL_VFUNC);
  } else {
    bc_add_code(&comp_prog, kwLET);
  }

  comp_error_if_keyword(comp_bc_name);
  comp_add_variable(&comp_prog, comp_bc_name);

  if (!comp_error) {
    if (v_func) {
      // a.b.c()
      if (array_index != NULL && array_index_len > 2) {
        // more than empty brackets
        comp_array_params(array_index, 0);
      }
    }
    else if (parms[0] == '(' || parms[0] == '[') {
      if (parms[0] == '(' && *comp_next_char(parms + 1) == ')') {
        // vn()=fillarray
        p = strchr(parms, '=');
        comp_expression(p, 0);
      } else {
        // array(n) = expr
        p = comp_array_params(parms, '=');
        if (!comp_error) {
          bc_add_code(&comp_prog, kwTYPE_CMPOPR);
          bc_add_code(&comp_prog, '=');
          comp_expression(p, 0);
        }
      }
    } else {
      if (array_index != NULL) {
        comp_array_params(array_index, 0);
      }
      bc_add_code(&comp_prog, kwTYPE_CMPOPR);
      bc_add_code(&comp_prog, '=');
      comp_expression(parms + 1, 0);
    }
  }
  if (array_index != NULL) {
    free(array_index);
  }
}

// User-defined procedures/functions
void comp_text_line_func(bid_t idx, int decl) {
  char *lpar_ptr, *eq_ptr;
  int count;
  char pname[SB_KEYWORD_SIZE + 1];

  // single-line function (DEF FN)
  if ((eq_ptr = strchr(comp_bc_parm, '='))) {
    *eq_ptr = '\0';
  }
  // parameters start
  if ((lpar_ptr = strchr(comp_bc_parm, '('))) {
    *lpar_ptr = '\0';
  }

  comp_prepare_name(pname, baseof(comp_bc_parm, '/'), SB_KEYWORD_SIZE);
  comp_error_if_keyword(baseof(comp_bc_parm, '/'));

  if (decl) {
    // its only a declaration (DECLARE)
    if (comp_udp_getip(pname) == INVALID_ADDR) {
      comp_add_udp(pname);
    }
  } else {
    // func/sub
    if (comp_udp_getip(pname) != INVALID_ADDR) {
      sc_raise(MSG_UDP_ALREADY_EXISTS, pname);
    } else {
      // setup routine's address (and get an id)
      int pidx;
      if ((pidx = comp_udp_setip(pname, comp_prog.count)) == -1) {
        pidx = comp_add_udp(pname);
        comp_udp_setip(pname, comp_prog.count);
      }
      // put JMP to the next command after the END
      // (now we just keep the rq space, pass2 will update that)
      bc_add_code(&comp_prog, kwGOTO);
      bc_add_addr(&comp_prog, 0);
      bc_add_code(&comp_prog, 0);

      comp_block_level++;
      comp_block_id++;
      // keep it in stack for 'pass2'
      comp_push(comp_prog.count);
      // store (FUNC/PROC) code
      bc_add_code(&comp_prog, idx);

      // func/proc name (also, update comp_bc_proc)
      if (comp_proc_level) {
        strcat(comp_bc_proc, "/");
        strcat(comp_bc_proc, baseof(pname, '/'));
      } else {
        strcpy(comp_bc_proc, pname);
      }

      if (!comp_error) {
        comp_proc_level++;

        // if its a function,
        // setup the code for the return-value
        // (vid={F}/{F})
        if (idx == kwFUNC) {
          strcpy(comp_bc_tmp2, baseof(pname, '/'));
          comp_udptable[pidx].vid = comp_var_getID(comp_bc_tmp2);
        } else {
          // procedure, no return value here
          comp_udptable[pidx].vid = INVALID_ADDR;
        }

        // parameters
        if (lpar_ptr) {
          int i;
          int vattr;
          char vname[SB_KEYWORD_SIZE + 1];
          char_p_t pars[MAX_PARAMS];

          *lpar_ptr = '(';
          comp_getlist_insep(comp_bc_parm, pars, "()", MAX_PARAMS, &count);
          bc_add_code(&comp_prog, kwTYPE_PARAM);
          bc_add_code(&comp_prog, count);

          for (i = 0; i < count; i++) {
            if ((strncmp(pars[i], LCN_BYREF_WRS, 6) == 0) || (pars[i][0] == '@')) {
              if (pars[i][0] == '@') {
                comp_prepare_name(vname, pars[i] + 1, SB_KEYWORD_SIZE);
              } else {
                comp_prepare_name(vname, pars[i] + 6, SB_KEYWORD_SIZE);
              }
              vattr = 0x80;
            } else {
              comp_prepare_name(vname, pars[i], SB_KEYWORD_SIZE);
              vattr = 0;
            }
            if (strchr(pars[i], '(')) {
              vattr |= 1;
            }

            bc_add_code(&comp_prog, vattr);
            bc_add_addr(&comp_prog, comp_var_getID(vname));
          }
        } else {
          // no parameters
          bc_add_code(&comp_prog, kwTYPE_PARAM);
          // params
          bc_add_code(&comp_prog, 0);
          // pcount = 0
        }

        bc_eoc(&comp_prog);
        // scan for single-line function (DEF FN format)
        if (eq_ptr && idx == kwFUNC) {
          // *eq_ptr was '\0'
          eq_ptr++;
          SKIP_SPACES(eq_ptr);
          if (strlen(eq_ptr)) {
            char *macro = malloc(SB_SOURCELINE_SIZE + 1);
            sprintf(macro, "%s=%s:%s", pname, eq_ptr, LCN_END);
            // run comp_text_line again
            comp_text_line(macro, 0);
            free(macro);
          } else {
            sc_raise(MSG_MISSING_UDP_BODY);
          }
        }

        if (opt_autolocal) {
          // jump to local variable handler
          comp_push(comp_prog.count);
          bc_add_code(&comp_prog, kwGOTO);
          bc_add_addr(&comp_prog, 0);
          bc_add_code(&comp_prog, 0);
          bc_eoc(&comp_prog);
        }
      }
    }
  }
}

void comp_text_line_on() {
  char *p;
  int count, i, keep_ip;
  char_p_t pars[MAX_PARAMS];

  comp_push(comp_prog.count);
  bc_add_ctrl(&comp_prog, kwONJMP, 0, 0);

  if ((p = strstr(comp_bc_parm, LCN_GOTO_WS)) != NULL) {
    bc_add_code(&comp_prog, kwGOTO);
    // the command
    *p = '\0';
    p += 6;
    keep_ip = comp_prog.count;
    bc_add_code(&comp_prog, 0);
    count = comp_getlist(p, pars, MAX_PARAMS);
    for (i = 0; i < count; i++) {
      bc_add_addr(&comp_prog, comp_label_getID(pars[i])); // IDs
    }

    if (count == 0) {
      sc_raise(MSG_ON_GOTO_ERR);
    } else {
      comp_prog.ptr[keep_ip] = count;
    }

    comp_expression(comp_bc_parm, 0); // the expression
    bc_eoc(&comp_prog);
  } else if ((p = strstr(comp_bc_parm, LCN_GOSUB_WS)) != NULL) {
    bc_add_code(&comp_prog, kwGOSUB);
    // the command
    *p = '\0';
    p += 7;
    keep_ip = comp_prog.count;
    bc_add_code(&comp_prog, 0);
    // the counter

    // count = bc_scan_label_list(p);
    count = comp_getlist(p, pars, MAX_PARAMS);
    for (i = 0; i < count; i++) {
      bc_add_addr(&comp_prog, comp_label_getID(pars[i]));
    }
    if (count == 0) {
      sc_raise(MSG_ON_GOSUB_ERR);
    } else {
      comp_prog.ptr[keep_ip] = count;
    }
    comp_expression(comp_bc_parm, 0); // the expression
    bc_eoc(&comp_prog);
  } else {
    sc_raise(MSG_ON_NOTHING);
  }
}

void comp_text_line_for() {
  char *p = strchr(comp_bc_parm, '=');
  char *p_do = strstr(comp_bc_parm, LCN_DO_WS);

  // fix DO bug
  if (p_do) {
    if (p > p_do) {
      p = NULL;
    }
  }
  strcpy(comp_do_close_cmd, LCN_NEXT);
  comp_block_level++;
  comp_block_id++;
  comp_push(comp_prog.count);
  bc_add_ctrl(&comp_prog, kwFOR, 0, 0);

  if (!p) {
    // FOR [EACH] X IN Y
    if ((p = strstr(comp_bc_parm, LCN_IN_WS)) == NULL) {
      sc_raise(MSG_FOR_NOTHING);
    } else {
      *p = '\0';
      char *n = p;
      strcpy(comp_bc_name, comp_bc_parm);
      str_alltrim(comp_bc_name);
      if (!is_alpha(*comp_bc_name)) {
        sc_raise(MSG_FOR_COUNT_ERR, comp_bc_name);
      } else {
        char *p_lev = comp_bc_name;
        while (is_alnum(*p_lev) || *p_lev == ' ') {
          p_lev++;
        }
        if (*p_lev == '(') {
          sc_raise(MSG_FOR_ARR_COUNT, comp_bc_name);
        } else {
          if (!comp_error_if_keyword(comp_bc_name)) {
            comp_add_variable(&comp_prog, comp_bc_name);
            *n = ' ';
            bc_add_code(&comp_prog, kwIN);
            comp_expression(n + 4, 0);
          }
        }
      }
    }
  } else {
    // FOR X=Y TO Z [STEP L]
    *p = '\0';
    char *n = p;

    strcpy(comp_bc_name, comp_bc_parm);
    str_alltrim(comp_bc_name);
    if (!is_alpha(*comp_bc_name)) {
      sc_raise(MSG_FOR_COUNT_ERR, comp_bc_name);
    } else {
      char *p_lev = comp_bc_name;
      while (is_alnum(*p_lev) || *p_lev == ' ') {
        p_lev++;
      }
      if (*p_lev == '(') {
        sc_raise(MSG_FOR_ARR_COUNT, comp_bc_name);
      } else {
        if (!comp_error_if_keyword(comp_bc_name)) {
          comp_add_variable(&comp_prog, comp_bc_name);
          *n = '=';
          comp_expression(n + 1, 0);
        }
      }
    }
  }
}

/**
 * Insert the local variables detected during sub/func processing
 */
void comp_insert_locals() {
  int i;
  int count_local = 0;
  for (i = 0; i < comp_varcount; i++) {
    if (comp_vartable[i].local_id != -1 &&
        comp_vartable[i].local_proc_level == comp_proc_level) {
      count_local++;
    }
  }

  comp_pass_node_t *node;
  bcip_t pos_goto = INVALID_ADDR;
  for (i = comp_stack.count - 1; i >= 0; i--) {
    node = comp_stack.elem[i];
    if (comp_prog.ptr[node->pos] == kwGOTO &&
        node->block_id != -1 &&
        node->level == comp_block_level) {
      pos_goto = node->pos;
      node->block_id = -1;
      break;
    }
  }

  if (pos_goto == INVALID_ADDR) {
    sc_raise(ERR_SYNTAX);
  } else {
    if (!count_local) {
      // position the func GOTO to after the EOC
      bcip_t ip = - (pos_goto + 1 + ADDRSZ + 1 + 1);
      memcpy(comp_prog.ptr + pos_goto + 1, &ip, ADDRSZ);
    } else {
      // skip over the kwTYPE_CRVAR block
      comp_push(comp_prog.count);
      bc_add_code(&comp_prog, kwGOTO);
      bcip_t pos_end = comp_prog.count;
      bc_add_addr(&comp_prog, 0);
      bc_add_code(&comp_prog, 0);
      bc_eoc(&comp_prog);

      // make the func GOTO arrive here at the kwTYPE_CRVAR block
      bcip_t ip = -comp_prog.count;
      memcpy(comp_prog.ptr + pos_goto + 1, &ip, ADDRSZ);
      bc_add_code(&comp_prog, kwTYPE_CRVAR);
      bc_add_code(&comp_prog, count_local);
      for (i = 0; i < comp_varcount; i++) {
        if (comp_vartable[i].local_id != -1 &&
            comp_vartable[i].local_proc_level == comp_proc_level) {
          bc_add_addr(&comp_prog, comp_vartable[i].local_id);
        }
      }
      bc_eoc(&comp_prog);

      // go back to the start of the func
      bcip_t pos_func_start = pos_goto + 1 + ADDRSZ + 1 + 1;
      comp_push(comp_prog.count);
      bc_add_code(&comp_prog, kwGOTO);
      bc_add_addr(&comp_prog, -pos_func_start);
      bc_add_code(&comp_prog, 0);
      bc_eoc(&comp_prog);

      // complete the goto that skips the kwTYPE_CRVAR block
      bcip_t pos_func_end = -comp_prog.count;
      memcpy(comp_prog.ptr + pos_end, &pos_func_end, ADDRSZ);
    }
  }
}


void comp_text_line_end(bid_t idx) {
  if (strncmp(comp_bc_parm, LCN_IF, 2) == 0 ||
      strncmp(comp_bc_parm, LCN_TRY, 3) == 0 ||
      strncmp(comp_bc_parm, LCN_SELECT, 6) == 0) {
    idx = strncmp(comp_bc_parm, LCN_IF, 2) == 0 ? kwENDIF :
          strncmp(comp_bc_parm, LCN_TRY, 3) == 0 ? kwENDTRY : kwENDSELECT;
    comp_push(comp_prog.count);
    if (idx == kwENDTRY) {
      bc_add_code(&comp_prog, idx);
    } else {
      bc_add_ctrl(&comp_prog, idx, 0, 0);
    }
    comp_block_level--;
    comp_block_id--;
  } else if (comp_proc_level) {
    char *dol;

    // UDP/F RETURN
    dol = strrchr(comp_bc_proc, '/');
    if (dol) {
      *dol = '\0';
    } else {
      *comp_bc_proc = '\0';
    }
    if (opt_autolocal) {
      comp_insert_locals();
    }
    comp_push(comp_prog.count);
    bc_add_code(&comp_prog, kwTYPE_RET);
    comp_proc_level--;
    comp_block_level--;
    comp_block_id++; // advance to next block
  } else {
    // END OF PROG
    bc_add_code(&comp_prog, idx);
  }
}

// External or user-defined procedure
void comp_text_line_ext_func() {
  int udp = comp_is_external_proc(comp_bc_name);
  if (udp > -1) {
    bc_add_extpcode(&comp_prog, comp_extproctable[udp].lib_id,
                    comp_extproctable[udp].symbol_index);
    char *next = trim_empty_parentheses(comp_bc_parm);
    if (comp_is_parenthesized(next)) {
      comp_expression(next, 0);
    } else {
      bc_add_code(&comp_prog, kwTYPE_LEVEL_BEGIN);
      comp_expression(next, 0);
      bc_add_code(&comp_prog, kwTYPE_LEVEL_END);
    }
  } else {
    udp = comp_udp_id(comp_bc_name, 1);
    if (udp == -1) {
      udp = comp_add_udp(comp_bc_name);
    }
    comp_push(comp_prog.count);
    bc_add_ctrl(&comp_prog, kwTYPE_CALL_UDP, udp, 0);
    char *next = trim_empty_parentheses(comp_bc_parm);
    if (comp_is_parenthesized(next)) {
      comp_expression(next, 0);
    } else {
      bc_add_code(&comp_prog, kwTYPE_LEVEL_BEGIN);
      comp_expression(next, 0);
      bc_add_code(&comp_prog, kwTYPE_LEVEL_END);
    }
  }
}

int comp_text_line_command(bid_t idx, int decl, int sharp, char *last_cmd) {
  char_p_t pars[MAX_PARAMS];
  int count, i, index;
  char vname[SB_KEYWORD_SIZE + 1];
  int result = 1;

  switch (idx) {
  case kwLABEL:
    str_alltrim(comp_bc_parm);
    idx = comp_label_getID(comp_bc_parm);
    comp_label_setip(idx);
    break;

  case kwEXIT:
    bc_add_code(&comp_prog, idx);
    str_alltrim(comp_bc_parm);
    if (strlen(comp_bc_parm) && comp_bc_parm[0] != '\'') {
      idx = comp_is_special_operator(comp_bc_parm);
      if (idx == kwFORSEP || idx == kwLOOPSEP || idx == kwPROCSEP || idx == kwFUNCSEP) {
        bc_add_code(&comp_prog, idx);
      } else {
        sc_raise(MSG_EXIT_ERR);
      }
    } else {
      bc_add_code(&comp_prog, 0);
    }
    break;

  case kwDECLARE:
    break;

  case kwPROC:
  case kwFUNC:
    comp_text_line_func(idx, decl);
    break;

  case kwLOCAL:
    // local variables
    if (!opt_autolocal) {
      count = comp_getlist(comp_bc_parm, pars, MAX_PARAMS);
      bc_add_code(&comp_prog, kwTYPE_CRVAR);
      bc_add_code(&comp_prog, count);
      for (i = 0; i < count; i++) {
        comp_prepare_name(vname, pars[i], SB_KEYWORD_SIZE);
        bc_add_addr(&comp_prog, comp_var_getID(vname));
      }
      // handle same line variable assignment, eg local blah = foo
      for (i = 0; i < count; i++) {
        comp_prepare_name(vname, pars[i], SB_KEYWORD_SIZE);
        if (strlen(vname) != strlen(pars[i])) {
          // kwTYPE_LINE is required for executor
          comp_text_line(pars[i], 1);
        }
      }
    }
    break;

  case kwREM:
    result = 0;
    break;

  case kwEXPORT:             // export
    if (comp_unit_flag) {
      bc_store_exports(comp_bc_parm);
    } else {
      sc_raise(MSG_UNIT_NAME_MISSING);
    }
    break;

  case kwOPTION:
    comp_cmd_option(comp_bc_parm);
    break;

  case kwGOTO:
    str_alltrim(comp_bc_parm);
    comp_push(comp_prog.count);
    bc_add_code(&comp_prog, idx);
    bc_add_addr(&comp_prog, comp_label_getID(comp_bc_parm));
    bc_add_code(&comp_prog, comp_block_level);
    break;

  case kwGOSUB:
    str_alltrim(comp_bc_parm);
    bc_add_code(&comp_prog, idx);
    bc_add_addr(&comp_prog, comp_label_getID(comp_bc_parm));
    break;

  case kwIF:
    strcpy(comp_do_close_cmd, LCN_ENDIF);

    // from here, we can scan for inline IF
    if (comp_single_line_if(last_cmd)) {
      // inline-IFs
      result = 0;
    } else {
      comp_block_level++;
      comp_block_id++;
      comp_push(comp_prog.count);
      bc_add_ctrl(&comp_prog, idx, 0, 0);
      comp_expression(comp_bc_parm, 0);
      bc_eoc(&comp_prog);
    }
    break;

  case kwON:
    comp_text_line_on();
    break;

  case kwFOR:
    comp_text_line_for();
    break;

  case kwWHILE:
    strcpy(comp_do_close_cmd, LCN_WEND);
    comp_block_level++;
    comp_block_id++;
    comp_push(comp_prog.count);
    bc_add_ctrl(&comp_prog, idx, 0, 0);
    comp_expression(comp_bc_parm, 0);
    break;

  case kwREPEAT:
    // WHILE & REPEAT DOES NOT USE STACK
    comp_block_level++;
    comp_block_id++;
    comp_push(comp_prog.count);
    bc_add_ctrl(&comp_prog, idx, 0, 0);
    comp_expression(comp_bc_parm, 0);
    break;

  case kwSELECT:
    comp_block_level++;
    comp_block_id++;
    comp_push(comp_prog.count);
    bc_add_code(&comp_prog, idx);
    // if comp_bc_parm starts with "CASE ", then skip first 5 chars
    index = strncasecmp("CASE ", comp_bc_parm, 5) == 0 ? 5 : 0;
    comp_expression(comp_bc_parm + index, 0);
    break;

  case kwCASE:
    // link to matched block or next CASE/END-SELECT
    if (!comp_bc_parm || !comp_bc_parm[0] || strncasecmp(LCN_ELSE, comp_bc_parm, 4) == 0) {
      comp_push(comp_prog.count);
      bc_add_ctrl(&comp_prog, kwCASE_ELSE, 0, 0);
    } else {
      comp_push(comp_prog.count);
      bc_add_ctrl(&comp_prog, idx, 0, 0);
      comp_expression(comp_bc_parm, 0);
    }
    break;

  case kwTRY:
    comp_block_level++;
    comp_block_id++;
    comp_push(comp_prog.count);
    bc_add_code(&comp_prog, idx);
    bc_add_addr(&comp_prog, 0);
    comp_expression(comp_bc_parm, 0);
    break;

  case kwCATCH:
    comp_push(comp_prog.count);
    bc_add_ctrl(&comp_prog, idx, 0, 0);
    comp_expression(comp_bc_parm, 0);
    break;

  case kwELSE:
  case kwELIF:
    index = 0;
    // handle "ELSE IF"
    if (idx == kwELSE && strncasecmp(LCN_IF, comp_bc_parm, 2) == 0) {
      idx = kwELIF;
      index = 2;
    }
    // handle error for ELSE xxxx
    if (idx == kwELSE && comp_bc_parm[0]) {
      sc_raise(ERR_SYNTAX);
      break;
    }
    comp_push(comp_prog.count);
    bc_add_ctrl(&comp_prog, idx, 0, 0);
    comp_expression(comp_bc_parm + index, 0);
    break;

  case kwENDIF:
  case kwNEXT:
    comp_push(comp_prog.count);
    bc_add_ctrl(&comp_prog, idx, 0, 0);
    comp_block_level--;
    comp_block_id--;
    break;

  case kwWEND:
  case kwUNTIL:
    comp_push(comp_prog.count);
    bc_add_ctrl(&comp_prog, idx, 0, 0);
    comp_block_level--;
    comp_block_id--;
    comp_expression(comp_bc_parm, 0);
    break;

  case kwSTEP:
  case kwTO:
  case kwIN:
  case kwTHEN:
  case kwCOS:
  case kwSIN:
  case kwLEN:
  case kwLOOP:
    // functions...
    sc_raise(MSG_SPECIAL_KW_ERR, comp_bc_name);
    break;

  case kwRESTORE:
    comp_push(comp_prog.count);
    bc_add_code(&comp_prog, idx);
    bc_add_addr(&comp_prog, comp_label_getID(comp_bc_parm));
    break;

  case kwEND:
    comp_text_line_end(idx);
    break;

  case kwDATA:
    comp_data_seg(comp_bc_parm);
    break;

  case kwREAD:
    bc_add_code(&comp_prog, sharp ? kwFILEREAD : idx);
    comp_expression(comp_bc_parm, 0);
    break;

  case kwINPUT:
    bc_add_code(&comp_prog, sharp ? kwFILEINPUT : idx);
    comp_expression(comp_bc_parm, 0);
    break;

  case kwPRINT:
    bc_add_code(&comp_prog, sharp ? kwFILEPRINT : idx);
    comp_expression(comp_bc_parm, 0);
    break;

  case kwLINE:
    if (strncmp(comp_bc_parm, LCN_INPUT_WRS, 6) == 0) {
      bc_add_code(&comp_prog, kwLINEINPUT);
      comp_expression(comp_bc_parm + 6, 0);
    } else {
      bc_add_code(&comp_prog, idx);
      comp_expression(comp_bc_parm, 0);
    }
    break;

  case kwRETURN:
    if (comp_bc_proc[0]) {
      // synonym for FUNC=result
      if (comp_bc_parm[0]) {
        bc_add_code(&comp_prog, kwLET);
        comp_add_variable(&comp_prog, comp_bc_proc);
        bc_add_code(&comp_prog, kwTYPE_CMPOPR);
        bc_add_code(&comp_prog, '=');
        comp_expression(comp_bc_parm, 0);
      }
      bc_add_code(&comp_prog, kwRETURN);
      comp_push(comp_prog.count);
      bc_add_code(&comp_prog, kwFUNC_RETURN);
      bc_add_addr(&comp_prog, comp_proc_level);
    } else {
      // return from GOSUB
      bc_add_code(&comp_prog, idx);
      comp_expression(comp_bc_parm, 0);
    }
    break;

  case -1:
    comp_text_line_ext_func();
    break;

  default:
    // something else
    bc_add_code(&comp_prog, idx);
    comp_expression(comp_bc_parm, 0);
  }

  return result;
}

/*
 * Pass 1: scan source line
 */
void comp_text_line(char *text, int addLineNo) {
  bid_t idx;
  int decl = 0;

  if (comp_error) {
    return;
  }
  str_alltrim(text);
  char *p = text;

  // EOL
  if (*p == ':') {
    p++;
    comp_text_line(p, 0);
    return;
  }
  // remark
  if (*p == '\'' || *p == '#') {
    return;
  }
  // empty line
  if (*p == '\0') {
    return;
  }

  char *lb_end = (char *)comp_next_word(text, comp_bc_name);
  char *last_cmd = lb_end;
  p = get_param_sect(lb_end, ":", comp_bc_parm);

  // check old style labels
  if (is_all_digits(comp_bc_name)) {
    str_alltrim(comp_bc_name);
    idx = comp_label_getID(comp_bc_name);
    comp_label_setip(idx);
    if (comp_error) {
      return;
    }
    // continue
    last_cmd = p = (char *)comp_next_word(lb_end, comp_bc_name);
    if (strlen(comp_bc_name) == 0) {
      if (!p) {
        return;
      }
      if (*p == '\0') {
        return;
      }
    }
    p = get_param_sect(p, ":", comp_bc_parm);
  }

  idx = comp_is_keyword(comp_bc_name);
  if (idx == kwREM) {
    return;
  }
  if (addLineNo) {
    // add debug info: line-number
    bc_add_code(&comp_prog, kwTYPE_LINE);
    bc_add_addr(&comp_prog, comp_line);
  }
  if (idx == -1) {
    idx = comp_is_proc(comp_bc_name);
    if (idx != -1) {
      if (idx == kwCALLCP) {
        bc_add_code(&comp_prog, kwTYPE_CALL_UDP);
        bc_add_addr(&comp_prog, idx); // place holder
        bc_add_addr(&comp_prog, 0); // return-variable ID
        bc_add_code(&comp_prog, kwTYPE_LEVEL_BEGIN);
        // allow cmd_udp to find the initial var-ptr arg
        bc_add_code(&comp_prog, kwTYPE_CALL_PTR);
        char *next = trim_empty_parentheses(comp_bc_parm);
        comp_expression(next, 0);
        bc_add_code(&comp_prog, kwTYPE_LEVEL_END);
      } else {
        // simple buildin procedure
        // there is no need to check it more...
        // save it and return (go to next)
        bc_add_pcode(&comp_prog, idx);
        char *next = trim_empty_parentheses(comp_bc_parm);
        comp_expression(next, 0);
      }
      if (*p == ':') {
        // command separator
        bc_eoc(&comp_prog);
        p++;
        comp_text_line(p, 0);
      }
      return;
    }
  }
  if (idx == kwLET) {
    // old-style keyword LET
    idx = -1;
    char *p = (char *)comp_next_word(comp_bc_parm, comp_bc_name);
    if (p > comp_bc_parm) {
      // p is an offset of comp_bc_parm
      int len = strlen(p);
      memmove(comp_bc_parm, p, len);
      comp_bc_parm[len] = '\0';
    }
  } else if (idx == kwDECLARE) {
    // declaration
    decl = 1;
    char *p = (char *)comp_next_word(comp_bc_parm, comp_bc_name);
    idx = comp_is_keyword(comp_bc_name);
    if (idx == -1) {
      idx = comp_is_proc(comp_bc_name);
    }
    if (p > comp_bc_parm) {
      // p is an offset of comp_bc_parm
      int len = strlen(p);
      memmove(comp_bc_parm, p, len);
      comp_bc_parm[len] = '\0';
    }
    if (idx != kwPROC && idx != kwFUNC) {
      sc_raise(MSG_USE_DECL);
      return;
    }
  }
  if (idx == kwREM) {
    return;
  }

  int sharp, ladd,linc, ldec, leqop;
  sharp = (comp_bc_parm[0] == '#'); // if # -> file commands
  comp_get_unary(comp_bc_parm, &ladd, &linc, &ldec, &leqop);

  if ((comp_bc_parm[0] == '=' || ladd || linc || ldec || leqop) && (idx != -1)) {
    sc_raise(MSG_IT_IS_KEYWORD, comp_bc_name);
    return;
  }
  if ((idx == kwCONST) ||
      ((comp_bc_parm[0] == '=' ||
        ((comp_bc_parm[0] == '(' || comp_bc_parm[0] == '[')
         && !comp_is_function(comp_bc_name)) ||
        ladd || linc || ldec || leqop) && (idx == -1))) {
    comp_text_line_let(idx, ladd, linc, ldec, leqop);
  } else {
    if (!comp_text_line_command(idx, decl, sharp, last_cmd)) {
      p = NULL;
    }
  }
  if (p != NULL && *p == ':') {
    // command separator
    bc_eoc(&comp_prog);
    p++;
    comp_text_line(p, 0);
  }
}

/*
 * skip command bytes
 */
bcip_t comp_next_bc_cmd(bcip_t ip) {
  code_t code;
  uint32_t len;

  code = comp_prog.ptr[ip];
  ip++;

  switch (code) {
  case kwTYPE_INT:             // integer
    ip += OS_INTSZ;
    break;
  case kwTYPE_NUM:             // number
    ip += OS_REALSZ;
    break;
  case kwTYPE_STR:             // string: [2/4B-len][data]
    memcpy(&len, comp_prog.ptr + ip, OS_STRLEN);
    len += OS_STRLEN;
    ip += len;
    break;
  case kwTYPE_CALLF:
  case kwTYPE_CALLP:           // [bid_t]
    ip += CODESZ;
    break;
  case kwTYPE_CALLEXTF:
  case kwTYPE_CALLEXTP:        // [lib][index]
    ip += (ADDRSZ * 2);
    break;
  case kwEXIT:
  case kwTYPE_SEP:
  case kwTYPE_LOGOPR:
  case kwTYPE_CMPOPR:
  case kwTYPE_ADDOPR:
  case kwTYPE_MULOPR:
  case kwTYPE_POWOPR:
  case kwTYPE_UNROPR:          // [1B data]
    ip++;
    break;
  case kwTRY:
  case kwRESTORE:
  case kwGOSUB:
  case kwTYPE_LINE:
  case kwTYPE_VAR:             // [addr|id]
  case kwFUNC_RETURN:
    ip += ADDRSZ;
    break;
  case kwTYPE_PTR:
  case kwTYPE_CALL_UDP:
  case kwTYPE_CALL_UDF:        // [true-ip][false-ip]
    ip += BC_CTRLSZ;
    break;
  case kwGOTO:                 // [addr][pop-count]
    ip += (ADDRSZ + 1);
    break;
  case kwTYPE_CRVAR:           // [1B count][addr1][addr2]...
    len = comp_prog.ptr[ip];
    ip += ((len * ADDRSZ) + 1);
    break;
  case kwTYPE_PARAM:           // [1B count] {[1B-pattr][addr1]} ...
    len = comp_prog.ptr[ip];
    ip += ((len * (ADDRSZ + 1)) + 1);
    break;
  case kwONJMP:                // [true-ip][false-ip] [GOTO|GOSUB]
    // [count] [addr1]...
    ip += (BC_CTRLSZ + 1);
    ip += (comp_prog.ptr[ip] * ADDRSZ);
    break;
  case kwOPTION:               // [1B-optcode][addr-data]
    ip += (ADDRSZ + 1);
    break;
  case kwIF:
  case kwFOR:
  case kwWHILE:
  case kwREPEAT:
  case kwELSE:
  case kwELIF:
  case kwENDIF:
  case kwNEXT:
  case kwWEND:
  case kwUNTIL:
  case kwUSE:
  case kwCASE:
  case kwCASE_ELSE:
  case kwENDSELECT:
  case kwCATCH:
    ip += BC_CTRLSZ;
    break;
  case kwTYPE_EVAL_SC:
    ip += 2;                    // kwTYPE_LOGOPR+op
    ip += ADDRSZ;               // the shortcut address
    break;
  };
  return ip;
}

/*
 * search for command (in byte-code)
 */
bcip_t comp_search_bc(bcip_t ip, code_t code) {
  bcip_t i = ip;
  bcip_t result = INVALID_ADDR;
  do {
    if (i >= comp_prog.count) {
      break;
    } else if (code == comp_prog.ptr[i]) {
      result = i;
      break;
    }
    i = comp_next_bc_cmd(i);
  } while (i < comp_prog.count);
  return result;
}

/*
 * search for End-Of-Command mark
 */
bcip_t comp_search_bc_eoc(bcip_t ip) {
  bcip_t i = ip;
  code_t code;

  do {
    code = comp_prog.ptr[i];
    if (code == kwTYPE_EOC || code == kwTYPE_LINE) {
      return i;
    }
    i = comp_next_bc_cmd(i);
  } while (i < comp_prog.count);
  return comp_prog.count;
}

/*
 * search stack
 */
bcip_t comp_search_bc_stack(bcip_t start, code_t code, byte level, bid_t block_id) {
  bcip_t i;
  comp_pass_node_t *node;

  for (i = start; i < comp_sp; i++) {
    node = comp_stack.elem[i];
    if (comp_prog.ptr[node->pos] == code) {
      if (node->level == level && (block_id == -1 || block_id == node->block_id)) {
        return node->pos;
      }
    }
  }
  return INVALID_ADDR;
}

/*
 * search stack backward
 */
bcip_t comp_search_bc_stack_backward(bcip_t start, code_t code, byte level, bid_t block_id) {
  bcip_t i = start;
  comp_pass_node_t *node;

  for (; i < comp_sp; i--) {
    // WARNING: ITS UNSIGNED, SO WE'LL SEARCH
    // IN RANGE [0..STK_COUNT]
    node = comp_stack.elem[i];
    if (comp_prog.ptr[node->pos] == code) {
      if (node->level == level && (block_id == -1 || block_id == node->block_id)) {
        return node->pos;
      }
    }
  }
  return INVALID_ADDR;
}

/*
 * inspect the byte-code at the given location
 */
bcip_t comp_next_bc_peek(bcip_t start) {
  bcip_t result;
  if (start < comp_stack.count) {
    comp_pass_node_t *node = comp_stack.elem[start];
    result = comp_prog.ptr[node->pos];
  } else {
    result = -1;
  }
  return result;
}

/*
 * Advanced error messages:
 * Analyze LOOP-END errors
 */
void print_pass2_stack(bcip_t pos, code_t lcode, int level) {
  bcip_t ip;
  bcip_t i;
  int j, cs_idx;
  char cmd[16], cmd2[16];
  comp_pass_node_t *node;
  code_t ccode[256];
  int csum[256];
  int cs_count;
  code_t start_code[] = { kwWHILE, kwREPEAT, kwIF, kwFOR, kwFUNC, 0 };
  code_t end_code[] = { kwWEND, kwUNTIL, kwENDIF, kwNEXT, kwTYPE_RET, 0 };
  code_t code = lcode;

  if (opt_quiet || opt_interactive) {
    return;
  }

  kw_getcmdname(code, cmd);

  ip = comp_search_bc_stack(pos + 1, code, level - 1, -1);
  if (ip == INVALID_ADDR) {
    ip = comp_search_bc_stack(pos + 1, code, level + 1, -1);
    if (ip == INVALID_ADDR) {
      int cnt = 0;
      for (i = pos + 1; i < comp_sp; i++) {
        node = comp_stack.elem[i];
        if (comp_prog.ptr[node->pos] == code) {
          log_printf("\n%s found on level %d (@%d) instead of %d (@%d+)\n",
                     cmd, node->level, node->pos, level, pos);
          cnt++;
          if (cnt > 3) {
            break;
          }
        }
      }
    } else {
      log_printf("\n%s found on level %d instead of %d (@%d+)\n",
                 cmd, level + 1, level, pos);
    }
  } else {
    log_printf("\n%s found on level %d instead of %d (@%d+)\n",
               cmd, level - 1, level, pos);
  }

  // print stack
  cs_count = 0;
  log_printf("\n");
  log_printf("--- Pass 2 - stack ------------------------------------------------------\n");
  log_printf("%s%4s  %16s %16s %6s %6s %5s %5s %5s\n", "  ", "   i", "Command", "Section", "Addr", "Line",
             "Level", "BlkID", "Count");
  log_printf("-------------------------------------------------------------------------\n");

  for (i = 0; i < comp_sp; i++) {
    node = comp_stack.elem[i];
    code = comp_prog.ptr[node->pos];
    if (node->pos != INVALID_ADDR) {
      kw_getcmdname(code, cmd);
    } else {
      strcpy(cmd, "---");
    }
    // sum
    cs_idx = -1;
    for (j = 0; j < cs_count; j++) {
      if (ccode[j] == code) {
        cs_idx = j;
        csum[cs_idx]++;
        break;
      }
    }
    if (cs_idx == -1) {
      cs_idx = cs_count;
      cs_count++;
      ccode[cs_idx] = code;
      csum[cs_idx] = 1;
    }
    // info
    log_printf("%s%4d: %16s %16s %6d %6d %5d %5d %5d\n", ((i == pos) ? ">>" : "  "),
               i, cmd, node->sec, node->pos, node->line, node->level, node->block_id, csum[cs_idx]);
  }

  // sum
  log_printf("\n");
  log_printf("--- Sum -----------------------------------------------------------------\n");
  for (i = 0; i < cs_count; i++) {
    code = ccode[i];
    if (!kw_getcmdname(code, cmd))
      sprintf(cmd, "(%d)", code);
    log_printf("%16s - %5d\n", cmd, csum[i]);
  }

  // decide
  log_printf("\n");
  for (i = 0; start_code[i] != 0; i++) {
    int sa, sb;
    code_t ca, cb;

    ca = start_code[i];
    cb = end_code[i];

    sa = 0;
    for (j = 0; j < cs_count; j++) {
      if (ccode[j] == ca)
        sa = csum[j];
      if (ca == kwFUNC) {
        if (ccode[j] == kwPROC)
          sa += csum[j];
      }
    }

    sb = 0;
    for (j = 0; j < cs_count; j++) {
      if (ccode[j] == cb) {
        sb = csum[j];
        break;
      }
    }

    if (sa - sb != 0) {
      kw_getcmdname(ca, cmd);
      kw_getcmdname(cb, cmd2);
      if (sa > sb) {
        log_printf("Hint: Missing %d %s or there is/are %d more %s\n", sa - sb, cmd2, sa - sb, cmd);
      } else {
        log_printf("Hint: There is/are %d more %s or missing %d %s\n", sb - sa, cmd2, sb - sa, cmd);
      }
    }
  }

  log_printf("\n\n");
}

/*
 * PASS 2 (write jumps for IF,FOR,WHILE,REPEAT,etc)
 */
void comp_pass2_scan() {
  bcip_t i = 0, j, true_ip, false_ip, label_id, w;
  bcip_t a_ip, b_ip, c_ip, count;
  code_t code;
  byte level;
  comp_pass_node_t *node;
  comp_label_t *label;

  if (!opt_quiet && !opt_interactive) {
    log_printf(MSG_PASS2_COUNT, i, comp_sp);
  }

  // for each node in stack
  for (i = 0; i < comp_sp; i++) {
    if (!opt_quiet && !opt_interactive) {
      if ((i % SB_KEYWORD_SIZE) == 0) {
        log_printf(MSG_PASS2_COUNT, i, comp_sp);
      }
    }

    node = comp_stack.elem[i];
    comp_line = node->line;
    strcpy(comp_bc_sec, node->sec);
    code = comp_prog.ptr[node->pos];
    if (code == kwTYPE_EOC || code == kwTYPE_LINE) {
      continue;
    }

    // debug (node->pos = the address of the error)
    //
    // if (node->pos == 360 || node->pos == 361)
    // trace("=== stack code %d\n", code);

    if (code != kwGOTO &&
        code != kwRESTORE &&
        code != kwSELECT &&
        code != kwONJMP &&
        code != kwTYPE_PTR &&
        code != kwTYPE_CALL_UDP &&
        code != kwTYPE_CALL_UDF &&
        code != kwPROC &&
        code != kwFUNC &&
        code != kwTRY &&
        code != kwCATCH &&
        code != kwENDTRY &&
        code != kwFUNC_RETURN &&
        code != kwTYPE_RET) {
      // default - calculate true-ip
      true_ip = comp_search_bc_eoc(node->pos + (BC_CTRLSZ + 1));
      memcpy(comp_prog.ptr + node->pos + 1, &true_ip, ADDRSZ);
    }

    switch (code) {
    case kwPROC:
    case kwFUNC:
      // update start's GOTO
      true_ip = comp_search_bc_stack(i + 1, kwTYPE_RET, node->level, -1) + 1;
      if (true_ip == INVALID_ADDR) {
        sc_raise(MSG_UDP_MISSING_END);
        print_pass2_stack(i, kwTYPE_RET, node->level);
        return;
      }
      memcpy(comp_prog.ptr + node->pos - (ADDRSZ + 1), &true_ip, ADDRSZ);
      break;

    case kwRESTORE:
      // replace the label ID with the real IP
      memcpy(&label_id, comp_prog.ptr + node->pos + 1, ADDRSZ);
      label = comp_labtable.elem[label_id];
      count = comp_first_data_ip + label->dp;
      memcpy(comp_prog.ptr + node->pos + 1, &count, ADDRSZ);
      // change LABEL-ID with DataPointer
      break;

    case kwTYPE_PTR:
    case kwTYPE_CALL_UDP:
    case kwTYPE_CALL_UDF:
      memcpy(&label_id, comp_prog.ptr + node->pos + 1, ADDRSZ);
      if (label_id < comp_udpcount) {
        // update real IP
        true_ip = comp_udptable[label_id].ip + (ADDRSZ + 3);
        memcpy(comp_prog.ptr + node->pos + 1, &true_ip, ADDRSZ);
        // update return-var ID
        true_ip = comp_udptable[label_id].vid;
        memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &true_ip, ADDRSZ);
      } else if (label_id != kwCALLCF) {
        sc_raise(MSG_EXP_GENERR);
      }
      break;

    case kwONJMP:
      // kwONJMP:1 trueip:2 falseip:2 command:1 count:1 label1:2
      // label2:2 ...
      count = comp_prog.ptr[node->pos + (ADDRSZ + ADDRSZ + 2)];

      true_ip = comp_search_bc_eoc(node->pos + BC_CTRLSZ + (count * ADDRSZ) + 3);
      memcpy(comp_prog.ptr + node->pos + 1, &true_ip, ADDRSZ);

      // change label IDs with the real IPs
      for (j = 0; j < count; j++) {
        memcpy(&label_id, comp_prog.ptr + node->pos + (j * ADDRSZ) + (ADDRSZ + ADDRSZ + 3), ADDRSZ);
        label = comp_labtable.elem[label_id];
        w = label->ip;
        memcpy(comp_prog.ptr + node->pos + (j * ADDRSZ) + (ADDRSZ + ADDRSZ + 3), &w, ADDRSZ);
      }
      break;

    case kwGOTO:
      memcpy(&label_id, comp_prog.ptr + node->pos + 1, ADDRSZ);
      if ((int)label_id < 0) {
        // specific internal jump value
        w = -label_id;
      } else {
        // change LABEL-ID with IP
        label = comp_labtable.elem[label_id];
        w = label->ip;

        // number of POPs
        level = comp_prog.ptr[node->pos + (ADDRSZ + 1)];
        if (level >= label->level) {
          comp_prog.ptr[node->pos + (ADDRSZ + 1)] = level - label->level;
        } else {
          comp_prog.ptr[node->pos + (ADDRSZ + 1)] = 0;
        }
      }
      memcpy(comp_prog.ptr + node->pos + 1, &w, ADDRSZ);
      break;

    case kwFOR:
      a_ip = comp_search_bc(node->pos + (ADDRSZ + ADDRSZ + 1), kwTO);
      b_ip = comp_search_bc(node->pos + (ADDRSZ + ADDRSZ + 1), kwIN);
      if (a_ip < b_ip) {
        b_ip = INVALID_ADDR;
      } else if (a_ip > b_ip) {
        a_ip = b_ip;
      }
      false_ip = comp_search_bc_stack(i + 1, kwNEXT, node->level, -1);

      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_NEXT);
        print_pass2_stack(i, kwNEXT, node->level);
        return;
      }
      if (a_ip > false_ip || a_ip == INVALID_ADDR) {
        if (b_ip != INVALID_ADDR) {
          sc_raise(MSG_MISSING_IN);
        } else {
          sc_raise(MSG_MISSING_TO);
        }
        return;
      }
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwWHILE:
      false_ip = comp_search_bc_stack(i + 1, kwWEND, node->level, -1);

      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_WEND);
        print_pass2_stack(i, kwWEND, node->level);
        return;
      }
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwREPEAT:
      false_ip = comp_search_bc_stack(i + 1, kwUNTIL, node->level, -1);

      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_UNTIL);
        print_pass2_stack(i, kwUNTIL, node->level);
        return;
      }
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwUSE:
      true_ip = node->pos + (ADDRSZ + ADDRSZ + 1);
      false_ip = comp_search_bc_eoc(true_ip);
      memcpy(comp_prog.ptr + node->pos + 1, &true_ip, ADDRSZ);
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwIF:
    case kwELIF:
      a_ip = comp_search_bc_stack(i + 1, kwENDIF, node->level, -1);
      b_ip = comp_search_bc_stack(i + 1, kwELSE, node->level, -1);
      c_ip = comp_search_bc_stack(i + 1, kwELIF, node->level, -1);

      false_ip = a_ip;
      if (b_ip != INVALID_ADDR && b_ip < false_ip) {
        false_ip = b_ip;
      }
      if (c_ip != INVALID_ADDR && c_ip < false_ip) {
        false_ip = c_ip;
      }
      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_ENDIF_OR_ELSE);
        print_pass2_stack(i, kwENDIF, node->level);
        return;
      }

      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwELSE:
      false_ip = comp_search_bc_stack(i + 1, kwENDIF, node->level, -1);

      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_ENDIF);
        print_pass2_stack(i, kwENDIF, node->level);
        return;
      }

      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwTYPE_RET:
      break;

    case kwWEND:
      false_ip = comp_search_bc_stack_backward(i - 1, kwWHILE, node->level, -1);
      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_WHILE);
        print_pass2_stack(i, kwWHILE, node->level);
        return;
      }
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwUNTIL:
      false_ip = comp_search_bc_stack_backward(i - 1, kwREPEAT, node->level, -1);
      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_REPEAT);
        print_pass2_stack(i, kwREPEAT, node->level);
        return;
      }
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwNEXT:
      false_ip = comp_search_bc_stack_backward(i - 1, kwFOR, node->level, -1);
      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_FOR);
        print_pass2_stack(i, kwFOR, node->level);
        return;
      }
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwENDIF:
      false_ip = comp_search_bc_stack_backward(i - 1, kwIF, node->level, -1);
      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_IF);
        print_pass2_stack(i, kwIF, node->level);
        return;
      }
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwSELECT:
      // next instruction should be CASE
      false_ip = comp_next_bc_peek(i + 1);
      if (false_ip != kwCASE && false_ip != kwCASE_ELSE) {
        sc_raise(MSG_MISSING_CASE);
        print_pass2_stack(i, kwCASE, node->level);
        return;
      }
      break;

    case kwCASE:
      // false path is either next case statement or "end select"
      false_ip = comp_search_bc_stack(i + 1, kwCASE, node->level, node->block_id);

      // avoid finding another CASE or CASE ELSE on the same level, but after END SELECT
      j = comp_search_bc_stack(i + 1, kwENDSELECT, node->level, node->block_id);

      if (false_ip == INVALID_ADDR || false_ip > j) {
        false_ip = comp_search_bc_stack(i + 1, kwCASE_ELSE, node->level, node->block_id);
        if (false_ip == INVALID_ADDR || false_ip > j) {
          false_ip = j;
          if (false_ip == INVALID_ADDR) {
            sc_raise(MSG_MISSING_END_SELECT);
            print_pass2_stack(i, kwCASE, node->level);
            return;
          }
        }
      }

      // if expression returns false jump to the next case
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwCASE_ELSE:
      // check for END SELECT statement
      false_ip = comp_search_bc_stack(i + 1, kwENDSELECT, node->level, node->block_id);
      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_END_SELECT);
        print_pass2_stack(i, kwCASE, node->level);
        return;
      }
      // validate no futher CASE expr statements
      j = comp_search_bc_stack(i + 1, kwCASE, node->level, node->block_id);
      if (j != INVALID_ADDR && j < false_ip) {
        sc_raise(MSG_CASE_CASE_ELSE);
        print_pass2_stack(i, kwCASE, node->level);
        return;
      }
      // validate no futher CASE ELSE expr statements
      j = comp_search_bc_stack(i + 1, kwCASE_ELSE, node->level, node->block_id);
      if (j != INVALID_ADDR && j < false_ip) {
        sc_raise(MSG_CASE_CASE_ELSE);
        print_pass2_stack(i, kwCASE_ELSE, node->level);
        return;
      }
      // if the expression is false jump to the end-select
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwENDSELECT:
      false_ip = comp_search_bc_stack_backward(i - 1, kwSELECT, node->level, node->block_id);
      if (false_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_SELECT);
        print_pass2_stack(i, kwSELECT, node->level);
        return;
      }
      break;

    case kwTRY:
      true_ip = comp_search_bc_stack(i + 1, kwCATCH, node->level, node->block_id);
      if (true_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_CATCH);
        print_pass2_stack(i, kwTRY, node->level);
        return;
      }
      memcpy(comp_prog.ptr + node->pos + 1, &true_ip, ADDRSZ);
      break;

    case kwCATCH:
      true_ip = comp_search_bc_stack(i + 1, kwENDTRY, node->level, node->block_id);
      if (true_ip == INVALID_ADDR) {
        sc_raise(MSG_MISSING_ENDTRY);
        print_pass2_stack(i, kwENDTRY, node->level);
        return;
      }
      // address of the end-try
      memcpy(comp_prog.ptr + node->pos + 1, &true_ip, ADDRSZ);

      // address of the next catch in the same block
      false_ip = comp_search_bc_stack(i + 1, kwCATCH, node->level, node->block_id);
      if (false_ip > true_ip) {
        // not valid if found after end-try
        false_ip = INVALID_ADDR;
      }
      memcpy(comp_prog.ptr + node->pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
      break;

    case kwFUNC_RETURN:
      // address for the FUNCs kwTYPE_RET
      level = comp_prog.ptr[node->pos + 1];
      true_ip = comp_search_bc_stack(i + 1, kwTYPE_RET, level, -1);
      if (true_ip != INVALID_ADDR) {
        // otherwise error handled elsewhere
        memcpy(comp_prog.ptr + node->pos + 1, &true_ip, ADDRSZ);
      }
      break;
    };
  }

  if (!opt_quiet && !opt_interactive) {
    log_printf(MSG_PASS2_COUNT, comp_sp, comp_sp);
    log_printf("\n");
  }
}

int comp_read_goto(bcip_t ip, bcip_t *addr, code_t *level) {
  memcpy(addr, comp_prog.ptr + ip, sizeof(bcip_t));
  ip += sizeof(bcip_t);
  *level = comp_prog.ptr[ip];
  return ip + 1;
}

void comp_optimise() {
  // scan for repeated kwTYPE_LINE... kwGOTO blocks
  for (bcip_t ip = 0; !comp_error && ip < comp_prog.count; ip = comp_next_bc_cmd(ip)) {
    if (comp_prog.ptr[ip] == kwTYPE_LINE) {
      ip += 1 + sizeof(bcip_t);
      if (comp_prog.ptr[ip] == kwGOTO) {
        bcip_t addr;
        bcip_t new_addr = 0;
        bcip_t new_addr_ip = ip + 1;
        code_t level;

        ip = comp_read_goto(ip + 1, &addr, &level);
        bcip_t goto_ip = addr;
        if (comp_prog.ptr[goto_ip] == kwTYPE_EOC) {
          new_addr = goto_ip + 1;
        }
        while (goto_ip > 0 && comp_prog.ptr[goto_ip] == kwTYPE_LINE) {
          goto_ip += 1 + sizeof(bcip_t);
          if (comp_prog.ptr[goto_ip] == kwGOTO) {
            code_t next_level;
            comp_read_goto(goto_ip + 1, &addr, &next_level);
            goto_ip = addr;
            if (next_level == level) {
              // found replacement GOTO address
              new_addr = addr;
            }
          } else {
            break;
          }
        }
        if (new_addr != 0 && comp_prog.ptr[new_addr] == kwTYPE_EOC) {
          new_addr++;
        }
        if (new_addr != 0) {
          // patch in replacement address
          memcpy(comp_prog.ptr + new_addr_ip, &new_addr, sizeof(bcip_t));
        }
      }
    }
  }
}

/*
 * initialize compiler
 */
void comp_init() {
  comp_bc_sec = malloc(SB_KEYWORD_SIZE + 1);
  memset(comp_bc_sec, 0, SB_KEYWORD_SIZE + 1);
  comp_bc_name = malloc(SB_SOURCELINE_SIZE + 1);
  comp_bc_parm = malloc(SB_SOURCELINE_SIZE + 1);
  comp_bc_temp = malloc(SB_SOURCELINE_SIZE + 1);
  comp_bc_tmp2 = malloc(SB_SOURCELINE_SIZE + 1);
  comp_bc_proc = malloc(SB_SOURCELINE_SIZE + 1);

  comp_line = 0;
  comp_error = 0;
  comp_labcount = 0;
  comp_expcount = 0;
  comp_impcount = 0;
  comp_libcount = 0;
  comp_varcount = 0;
  comp_sp = 0;
  comp_udpcount = 0;
  comp_block_level = 0;
  comp_block_id = 0;
  comp_unit_flag = 0;
  comp_first_data_ip = INVALID_ADDR;
  comp_proc_level = 0;
  comp_bc_proc[0] = '\0';

  comp_vartable = (comp_var_t *)malloc(GROWSIZE * sizeof(comp_var_t));
  comp_udptable = (comp_udp_t *)malloc(GROWSIZE * sizeof(comp_udp_t));

  comp_labtable.count = 0;
  comp_labtable.size = 256;
  comp_labtable.elem = (comp_label_t **)malloc(comp_labtable.size * sizeof(comp_label_t **));

  comp_stack.count = 0;
  comp_stack.size = 256;
  comp_stack.elem = (comp_pass_node_t **)malloc(comp_stack.size * sizeof(comp_pass_node_t **));

  comp_libtable.count = 0;
  comp_libtable.elem = NULL;

  comp_imptable.count = 0;
  comp_imptable.elem = NULL;

  comp_exptable.count = 0;
  comp_exptable.elem = NULL;

  comp_varsize = comp_udpsize = GROWSIZE;
  comp_varcount = comp_labcount = comp_sp = comp_udpcount = 0;

  bc_create(&comp_prog);
  bc_create(&comp_data);

  // create system variables
  comp_var_getID(LCN_SV_SBVER);
  comp_var_getID(LCN_SV_XMAX);
  comp_var_getID(LCN_SV_YMAX);
  comp_var_getID(LCN_SV_TRUE);
  comp_var_getID(LCN_SV_FALSE);
  comp_vartable[comp_var_getID(LCN_SV_CWD)].dolar_sup = 1;
  comp_vartable[comp_var_getID(LCN_SV_HOME)].dolar_sup = 1;
  comp_vartable[comp_var_getID(LCN_SV_COMMAND)].dolar_sup = 1;
  comp_var_getID(LCN_SV_X);
  comp_var_getID(LCN_SV_Y);
  comp_var_getID(LCN_SV_SELF);
  comp_var_getID(LCN_SV_NIL);
  comp_var_getID(LCN_SV_MAXINT);
}

/*
 * clean up
 */
void comp_close() {
  int i;

  bc_destroy(&comp_prog);
  bc_destroy(&comp_data);

  for (i = 0; i < comp_varcount; i++) {
    free(comp_vartable[i].name);
  }
  free(comp_vartable);

  for (i = 0; i < comp_udpcount; i++) {
    free(comp_udptable[i].name);
  }
  free(comp_udptable);

  for (i = 0; i < comp_labtable.count; i++) {
    free(comp_labtable.elem[i]);
  }
  free(comp_labtable.elem);

  for (i = 0; i < comp_exptable.count; i++) {
    free(comp_exptable.elem[i]);
  }
  free(comp_exptable.elem);

  for (i = 0; i < comp_imptable.count; i++) {
    free(comp_imptable.elem[i]);
  }
  free(comp_imptable.elem);

  for (i = 0; i < comp_libtable.count; i++) {
    free(comp_libtable.elem[i]);
  }
  free(comp_libtable.elem);

  for (i = 0; i < comp_stack.count; i++) {
    free(comp_stack.elem[i]);
  }
  free(comp_stack.elem);

  comp_varcount = comp_labcount = comp_sp = comp_udpcount = 0;
  comp_libcount = comp_impcount = comp_expcount = 0;

  free(comp_bc_proc);
  free(comp_bc_tmp2);
  free(comp_bc_temp);
  free(comp_bc_parm);
  free(comp_bc_name);
  free(comp_bc_sec);
  comp_reset_externals();
}

/*
 * load a source file
 */
char *comp_load(const char *file_name) {
  char *buf;
  strcpy(comp_file_name, file_name);
#if defined(IMPL_DEV_READ)
  buf = dev_read(file_name);
#else
  int h = open(comp_file_name, O_BINARY | O_RDONLY, 0644);
  if (h == -1) {
    buf = NULL;
    panic(MSG_CANT_OPEN_FILE, comp_file_name);
  } else {
    int size;

    size = lseek(h, 0, SEEK_END);
    lseek(h, 0, SEEK_SET);

    buf = (char *)malloc(size + 1);
    read(h, buf, size);
    buf[size] = '\0';
    close(h);
  }
#endif
  return buf;
}

/**
 * format source-code text
 *
 * space-chars is the only the space
 * CR/LF are fixed
 * control chars are out
 * remove remarks (')
 *
 * returns a newly created string
 */
char *comp_format_text(const char *source) {
  const char *p;
  char *ps;
  int quotes = 0;
  char *new_text;
  int sl, last_ch = 0, i;
  char *last_nonsp_ptr;
  int adj_line_num = 0;
  int multi_line_string = 0;
  int curley_brace = 0;
  int square_brace = 0;

  sl = strlen(source);
  new_text = malloc(sl + 2);
  memset(new_text, 0, sl + 2);

  comp_line = 0;
  p = source;
  last_nonsp_ptr = ps = new_text;
  while (*p) {
    if (!quotes) {
      switch (*p) {
      case '\n':
        if (*last_nonsp_ptr == '&') {
          // join lines
          p++;
          *last_nonsp_ptr = ' ';
          if (*(last_nonsp_ptr - 1) == ' ') {
            ps = last_nonsp_ptr;
          } else {
            ps = last_nonsp_ptr + 1;
          }
          adj_line_num++;
          last_ch = '\n';
        } else if (square_brace) {
          // code array declared over multiple lines
          last_ch = *ps = V_LINE;
          ps++;
          p++;
        } else {
          for (i = 0; i <= adj_line_num; i++) {
            // at least one nl
            *ps++ = '\n';
          }
          adj_line_num = 0;
          p++;
          last_ch = '\n';
        }
        last_nonsp_ptr = ps - 1;
        SKIP_SPACES(p);
        break;

      case '\'':
        // remarks - skip the rest line
        while (*p) {
          if (*p == '\n') {
            break;
          }
          p++;
        }
        break;

      case ' ':
      case '\t':
        // spaces
        if (last_ch == ' ' || last_ch == '\n') {
          p++;
        } else {
          *ps++ = ' ';
          p++;
          last_ch = ' ';
        }
        break;

      case '\"':
        // quotes
        if (p[1] == '\"' && p[2] == '\"') {
          multi_line_string = 1;
          p += 2;
        }
        quotes = !quotes;
        last_nonsp_ptr = ps;
        *ps++ = last_ch = *p++;
        break;

      case '.':
        // advance beyond UDS element, copy same character case
        last_ch = *p;
        last_nonsp_ptr = ps;
        *ps++ = *p++;
        while (*p == '_' || isalnum(*p)) {
          last_ch = *p;
          last_nonsp_ptr = ps;
          *ps++ = *p++;
        }
        break;

      case '{':
        curley_brace++;
        quotes = 1;
        multi_line_string = 1;
        *ps++ = *p++;
        break;

      case '[':
        square_brace++;
        *ps++ = *p++;
        break;

      case ']':
        square_brace--;
        *ps++ = *p++;
        break;

      default:
        if ((strcaselessn(p, 5, LCN_REM_1, 5) == 0)
            || (strcaselessn(p, 5, LCN_REM_2, 5) == 0)
            || (strcaselessn(p, 4, LCN_REM_3, 4) == 0 && last_ch == '\n')
            || (strcaselessn(p, 4, LCN_REM_4, 4) == 0 && last_ch == '\n')) {
          // skip the rest line
          while (*p) {
            if (*p == '\n') {
              break;
            }
            p++;
          }
          break;
        } else {
          if ((*p > ' ') || (*p < 0)) {
            // simple code-character
            last_nonsp_ptr = ps;
            *ps++ = last_ch = to_upper(*p);
            p++;
          } else {
            // else ignore it (\r filtered here)
            p++;
          }
        }
      }
    } else {
      // in quotes
      if (*p == '\\' && (*(p + 1) == '\"' || *(p + 1) == '\\')) {
        // add the escaped quote or slash and continue
        *ps++ = *p++;
      } else if (multi_line_string) {
        if (p[0] == '\"' && p[1] == '\"' && p[2] == '\"') {
          // end of multi-line string
          quotes = 0;
          multi_line_string = 0;
          // add the single final quote character
          p += 2;
        } else if (p[0] == '\\' && (p[1] == '\r' || p[1] == '\n')) {
          // escape adding the newline
          if (p[1] == '\r' && p[2] == '\n') {
            p++;
          }
          p += 2;
          continue;
        } else if (p[0] == '\r') {
          p++;
          continue;
        } else if (p[0] == '\"') {
          // internal quote escape (see bc_store_string)
          *ps++ = V_QUOTE;
          p++;
          continue;
        } else if (p[0] == '\n') {
          // internal newline escape
          *ps++ = V_LINE;
          p++;
          continue;
        } else if (curley_brace && p[0] == '}') {
          if (--curley_brace == 0) {
            quotes = 0;
            multi_line_string = 0;
          }
        } else if (p[0] == '{') {
          curley_brace++;
        }
      } else if (*p == '\"' || *p == '\n') {
        // join to any adjacent quoted text
        const char *next = p + 1;
        while (is_space(*next)) {
          next++;
        }
        if (*next == '\"') {
          p = ++next;
          continue;
        }
        // new line auto-ends the quoted string
        quotes = !quotes;
      }
      *ps++ = *p++;
    }
  }

  // close
  *ps++ = '\n';
  *ps = '\0';

  return new_text;
}

/**
 * scans prefered graphics mode paramaters
 *
 * syntax: XXXXxYYYY[xBB]
 */
void err_grmode() {
  // log_printf() instead of sc_raise()... it is just a warning...
  log_printf(MSG_GRMODE_ERR);
}

void comp_preproc_grmode(const char *source) {
  char *p, *v;
  int x, y, b;
  char buffer[32];

  // prepare the string (copy it to buffer)
  // we use second buffer because we want to place some '\0' characters
  // into the buffer
  // in a non-SB code, there must be a dynamic allocation
  strncpy(buffer, source, 32);
  buffer[31] = '\0';
  p = buffer;

  // searching the end of the string
  while (*p) {
    // while *p is not '\0'
    if (*p == '\n' || *p == ':') {
      // yeap, we must close the string
      // here (enter or
      // command-seperator)
      // it is supposed that remarks had already removed from source
      *p = '\0';
      break;
    }
    p++;
  }

  // get parameters
  p = buffer;
  SKIP_SPACES(p);

  // the width
  v = p;                        // 'v' points to first letter of 'width',
  // (1024x768)
  // ........................................^ <- p, v
  p = strchr(v, 'X');           // search for the end of 'width' parameter
  //
  //
  // (1024x768). Remeber that the string is
  // in upper-case
  // .............................................^ <- p
  if (!p) {                     // we don't accept one parameter, the
    // width must followed by the height
    // so, if 'X' delimiter is omitted, there is no height parameter
    err_grmode();
    return;
  }
  *p = '\0';                    // we close the string at X position
  // (example: "1024x768" it will be
  // "1024\0768")
  x = xstrtol(v);               // now the v points to a string-of-digits,
  //
  //
  // we can perform atoi()
  // (xstrtol()=atoi())
  p++;
  v = p;                        // v points to first letter of 'height'
  // (1024x768x24)
  // ...........................................^ <- v

  // the height
  p = strchr(v, 'X');           // searching for the end of 'height'
  // (1024x768x24)
  // ...........................................^ <- p
  if (p) {                      // if there is a 'X' delimiter, then the
    // 'bpp' is followed, so, we need
    // different path
    *p = '\0';                  // we close the string at second's X
    // position
    y = xstrtol(v);             // now the v points to a string-of-digits,
    //
    //
    // we can perform atoi()
    // (xstrtol()=atoi())

    p++;
    v = p;                      // v points to first letter of 'bpp'
    // (1024x768x24)
    // ............................................^ <- v

    // the bits-per-pixel
    if (strlen(v))              // if *v != '\0', there is actually a
      // string
      b = xstrtol(v);           // integer value of (v). v points to a
    // string-of-digits...
    // btw, if the user pass some wrong characters (like a-z), the
    // xstrtol will return a value of zero
    else
      b = 0;                    // undefined = 0, user deserves a
    // compile-time error becase v is empty,
    // but we forgive him :)
    // remember that, the source, except of upper-case, is also
    // trimmed
  } else {                        // there was no 'X' delimiter after the
    // 'height', so, bpp is undefined
    y = xstrtol(v);             // now the v points to a string-of-digits,
    //
    //
    // we can perform atoi()
    // (xstrtol()=atoi())
    b = 0;                      // bpp is undefined (value 0)
  }

  // setup the globals
  opt_pref_width = x;
  opt_pref_height = y;
  opt_pref_bpp = b;
}

/**
 * copy the unit name from the source string to the given buffer
 */
const char *get_unit_name(const char *p, char *buf_p) {
  while (is_alnum(*p) || *p == '_' || *p == '.') {
    if (*p == '.') {
      *buf_p++ = OS_DIRSEP;
      p++;
    } else {
      *buf_p++ = *p++;
    }
  }

  *buf_p = '\0';
  return p;
}

/**
 * imports units
 */
void comp_preproc_import(const char *slist) {
  const char *p;
  char buf[OS_PATHNAME_SIZE + 1];
  int uid;

  p = slist;

  SKIP_SPACES(p);

  while (is_alpha(*p)) {
    // get name - "Import other.Foo => "other/Foo"
    p = get_unit_name(p, buf);

    // import name
    strlower(buf);
    if ((uid = slib_get_module_id(buf)) != -1) {  // C module
      // store C module lib-record
      slib_setup_comp(uid);
      add_libtable_rec(buf, uid, 0);
    } else {                      // SB unit
      uid = open_unit(buf);
      if (uid < 0) {
        sc_raise(MSG_UNIT_NOT_FOUND, buf);
        return;
      }

      if (import_unit(uid) < 0) {
        sc_raise(MSG_IMPORT_FAILED, buf);
        close_unit(uid);
        return;
      }
      // store lib-record
      add_libtable_rec(buf, uid, 1);

      // clean up
      close_unit(uid);
    }

    // skip spaces and commas
    while (*p == ' ' || *p == '\t' || *p == ',') {
      p++;
    }
  }
}

/**
 * makes the current line full of spaces
 */
void comp_preproc_remove_line(char *s, int cmd_sep_allowed) {
  char *p = s;

  if (cmd_sep_allowed) {
    while (*p != '\n' && *p != ':') {
      *p = ' ';
      p++;
    }
  } else {
    while (*p != '\n') {
      *p = ' ';
      p++;
    }
  }
}

/**
 * prepare compiler for UNIT-source
 */
void comp_preproc_unit(char *name) {
  const char *p = name;

  SKIP_SPACES(p);

  if (!is_alpha(*p)) {
    sc_raise(MSG_INVALID_UNIT_NAME);
  }

  p = get_unit_name(p, comp_unit_name);
  comp_unit_flag = 1;

  SKIP_SPACES(p);

  if (*p != '\n' && *p != ':') {
    sc_raise(MSG_UNIT_ALREADY_DEFINED);
  }
}

/**
 * Prepare compiler for INCLUDE source
 */
void comp_preproc_include(char *p) {
  char fileName[OS_PATHNAME_SIZE];
  char path[OS_PATHNAME_SIZE];

  SKIP_SPACES(p);
  if (*p == '\"') {
    p++;
  }
  char *fp = fileName;
  int size = 0;
  while (*p != '\n' &&
         *p != '\"' &&
         *p != '\0' &&
         ++size < OS_PATHNAME_SIZE) {
    *fp++ = *p++;
  }
  *fp = '\0';

  str_alltrim(fileName);
  strcpy(path, fileName);

  int basExists = (access(path, R_OK) == 0);
  if (!basExists && gsb_bas_dir[0]) {
    strcpy(path, gsb_bas_dir);
    strcat(path, fileName);
    basExists = (access(path, R_OK) == 0);
  }
  if (!basExists) {
    sc_raise(MSG_INC_FILE_DNE, comp_file_name, path);
  } else if (strcmp(comp_file_name, path) == 0) {
    sc_raise(MSG_INC_FILE_INC, comp_file_name, path);
  } else {
    char oldFileName[1024];
    char oldSec[SB_KEYWORD_SIZE + 1];
    strcpy(oldSec, comp_bc_sec);
    strcpy(oldFileName, comp_file_name);
    char *source = comp_load(path);
    if (source) {
      comp_pass1(NULL, source);
      free(source);
    }
    strcpy(comp_file_name, oldFileName);
    strcpy(comp_bc_sec, oldSec);
  }
}

/**
 * Handle OPTION environment parameters
 */
char *comp_preproc_options(char *p) {
  SKIP_SPACES(p);
  if (strncmp(LCN_PREDEF, p, LEN_PREDEF) == 0) {
    p += LEN_PREDEF;
    SKIP_SPACES(p);
    if (strncmp(LCN_QUIET, p, LEN_QUIET) == 0) {
      p += LEN_QUIET;
      SKIP_SPACES(p);
      opt_quiet = (strncmp("OFF", p, 3) != 0);
    } else if (strncmp(LCN_GRMODE, p, LEN_GRMODE) == 0) {
      p += LEN_GRMODE;
      comp_preproc_grmode(p);
      opt_graphics = 1;
    } else if (strncmp(LCN_TEXTMODE, p, LEN_TEXTMODE) == 0) {
      opt_graphics = 0;
    } else if (strncmp(LCN_ANTIALIAS, p, LEN_ANTIALIAS) == 0) {
      p += LEN_ANTIALIAS;
      SKIP_SPACES(p);
      opt_antialias = (strncmp("OFF", p, 3) != 0);
    } else if (strncmp(LCN_AUTOLOCAL, p, LEN_AUTOLOCAL) == 0) {
      p += LEN_AUTOLOCAL;
      opt_autolocal = 1;
    } else if (strncmp(LCN_COMMAND, p, LEN_COMMAND) == 0) {
      p += LEN_COMMAND;
      SKIP_SPACES(p);
      char *pe = p;
      while (*pe != '\0' && *pe != '\n') {
        pe++;
      }
      char lc = *pe;
      *pe = '\0';
      if (strlen(p) < OPT_CMD_SZ) {
        strcpy(opt_command, p);
      } else {
        memcpy(opt_command, p, OPT_CMD_SZ - 1);
        opt_command[OPT_CMD_SZ - 1] = '\0';
      }
      *pe = lc;
    } else if (strncmp(LCN_LOAD_MODULES, p, LEN_LDMODULES) == 0 &&
               opt_modlist[0] != '\0' && !opt_loadmod) {
      sblmgr_init(1, opt_modlist);
    } else {
      sc_raise(MSG_OPT_PREDEF_ERR, p);
    }
  }
  return p;
}

/**
 * Setup the UNITPATH environment variable.
 */
void comp_preproc_unit_path(char *p) {
  SKIP_SPACES(p);
  if (*p == '=') {
    p++;
    SKIP_SPACES(p);
  }
  if (*p == '\"') {
    p++;
    char upath[SB_SOURCELINE_SIZE + 1];
    char *up = upath;
    while (*p != '\n' && *p != '\"') {
      *up++ = *p++;
    }
    *up = '\0';
    dev_setenv(LCN_UNIT_PATH, upath);
  }
}

/**
 * SUB/FUNC/DEF - Automatic declaration - BEGIN
 */
char *comp_preproc_func_begin(char *p) {
  char *dp;
  int single_line_f = 0;
  char pname[SB_KEYWORD_SIZE + 1];

  if (strncmp(LCN_SUB_WRS, p, LEN_SUB_WRS) == 0) {
    p += LEN_SUB_WRS;
  } else if (strncmp(LCN_FUNC_WRS, p, LEN_FUNC_WRS) == 0) {
    p += LEN_FUNC_WRS;
  } else {
    p += LEN_DEF_WRS;
  }
  SKIP_SPACES(p);

  // copy proc/func name
  dp = pname;
  while (is_alnum(*p) || *p == '_') {
    *dp++ = *p++;
  }
  *dp = '\0';

  // search for '='
  while (*p != '\n' && *p != '=') {
    p++;
  }
  if (*p == '=') {
    single_line_f = 1;
    while (*p != '\n') {
      p++;
    }
  }

  // add declaration
  if (comp_udp_getip(pname) == INVALID_ADDR) {
    comp_add_udp(pname);
  } else {
    sc_raise(MSG_UDP_ALREADY_DECL, pname);
  }

  // func/proc name (also, update comp_bc_proc)
  if (comp_proc_level) {
    strcat(comp_bc_proc, "/");
    strcat(comp_bc_proc, baseof(pname, '/'));
  } else {
    strcpy(comp_bc_proc, pname);
  }

  if (!single_line_f) {
    comp_proc_level++;
  } else {
    // inline (DEF FN)
    char *dol = strrchr(comp_bc_proc, '/');
    if (dol) {
      *dol = '\0';
    } else {
      *comp_bc_proc = '\0';
    }
  }
  return p;
}

/**
 * SUB/FUNC/DEF - Automatic declaration - END
 */
void comp_preproc_func_end(char *p) {
  // avoid seeing "END SELECT" which doesn't end a SUB/FUNC
  if (strncmp(p, LCN_END_SELECT, LEN_END_SELECT) != 0 &&
      strncmp(p, LCN_END_TRY, LEN_END_TRY) != 0) {
    char *dol = strrchr(comp_bc_proc, '/');
    if (dol) {
      *dol = '\0';
    } else {
      *comp_bc_proc = '\0';
    }
    comp_proc_level--;
  }
}

/**
 * Preprocess handler for pass1
 */
void comp_preproc_pass1(char *p) {
  comp_proc_level = 0;
  *comp_bc_proc = '\0';

  while (*p) {
    if (strncmp(LCN_OPTION, p, LEN_OPTION) == 0) {
      // options
      p = comp_preproc_options(p + LEN_OPTION);
    } else if (strncmp(LCN_IMPORT_WRS, p, LEN_IMPORT) == 0) {
      // import
      comp_preproc_import(p + LEN_IMPORT);
      comp_preproc_remove_line(p, 1);
    } else if (strncmp(LCN_UNIT_WRS, p, LEN_UNIT) == 0) {
      // unit
      if (comp_unit_flag) {
        sc_raise(MSG_MANY_UNIT_DECL);
      } else {
        comp_preproc_unit(p + LEN_UNIT);
      }
      comp_preproc_remove_line(p, 1);
    } else if (strncmp(LCN_UNIT_PATH, p, LEN_UNIT_PATH) == 0) {
      // unitpath
      comp_preproc_unit_path(p + LEN_UNIT_PATH);
      comp_preproc_remove_line(p, 0);
    } else if (strncmp(LCN_INC, p, LEN_INC) == 0) {
      // include
      comp_preproc_include(p + LEN_INC);
      comp_preproc_remove_line(p, 0);
    } else if ((strncmp(LCN_SUB_WRS, p, LEN_SUB_WRS) == 0) ||
               (strncmp(LCN_FUNC_WRS, p, LEN_FUNC_WRS) == 0) ||
               (strncmp(LCN_DEF_WRS, p, LEN_DEF_WRS) == 0)) {
      // sub/func
      p = comp_preproc_func_begin(p);
    } else if (comp_proc_level &&
               (strncmp(LCN_END_WRS, p, LEN_END_WRS) == 0 ||
                strncmp(LCN_END_WNL, p, LEN_END_WRS) == 0)) {
      // end sub/func
      comp_preproc_func_end(p);
    } else if (strncasecmp(LCN_SHOWPAGE, p, LEN_SHOWPAGE) == 0) {
      opt_show_page = 1;
    }

    // skip text line
    while (*p != '\0' && *p != '\n') {
      p++;
    }

    if (*p) {
      p++;
    }
  }

  if (comp_proc_level) {
    sc_raise(MSG_UDP_MIS_END_2, comp_file_name, comp_bc_proc);
  }
  comp_proc_level = 0;
  *comp_bc_proc = '\0';

  if (!opt_quiet && !opt_interactive) {
    log_printf("%s: \033[1m%s\033[0m\n", WORD_FILE, comp_file_name);
  }
}

/**
 * PASS 1
 *
 * Check for:
 *  INCLUDE
 *  UNITS-DIR (#unit-path:)
 *  IMPORT
 *  UDF and UDP declarations
 *  PREDEF OPTIONS
 */
int comp_pass1(const char *section, const char *text) {
  memset(comp_bc_sec, 0, SB_KEYWORD_SIZE + 1);
  if (section) {
    strncpy(comp_bc_sec, section, SB_KEYWORD_SIZE);
  } else {
    strncpy(comp_bc_sec, SYS_MAIN_SECTION_NAME, SB_KEYWORD_SIZE);
  }

  char *code_line = malloc(SB_SOURCELINE_SIZE + 1);
  char *new_text = comp_format_text(text);

  comp_preproc_pass1(new_text);
  if (!comp_error) {
    if (!opt_quiet && !opt_interactive) {
      log_printf(MSG_PASS1_COUNT, comp_line + 1);
    }

    char *ps = new_text;
    char *p = ps;
    while (*p) {
      if (*p == '\n') {
        // proceed
        *p = '\0';
        comp_line++;
        if (!opt_quiet && !opt_interactive) {
          if ((comp_line % 256) == 0) {
            log_printf(MSG_PASS1_COUNT, comp_line);
          }
        }

        strcpy(code_line, ps);
        comp_text_line(code_line, 1);

        if (comp_error) {
          break;
        }
        ps = p + 1;
      }
      if (comp_error) {
        break;
      }
      p++;
    }
  }

  free(code_line);
  free(new_text);

  // undefined keywords by default are UDP - error if no UDP-body
  if (!comp_error) {
    int i;
    for (i = 0; i < comp_udpcount; i++) {
      if (comp_udptable[i].ip == INVALID_ADDR) {
        comp_line = comp_udptable[i].pline;
        char *dot = strchr(comp_udptable[i].name, '.');
        if (dot) {
          sc_raise(MSG_UNDEFINED_MAP, comp_udptable[i].name);
        } else {
          if (comp_is_func(comp_udptable[i].name) != -1) {
            sc_raise(MSG_FUNC_NOT_ASSIGNED, comp_udptable[i].name);
          } else {
            sc_raise(MSG_UNDEFINED_UDP, comp_udptable[i].name);
          }
        }
        break;
      }
    }
  }

  bc_eoc(&comp_prog);
  bc_resize(&comp_prog, comp_prog.count);
  if (!comp_error && !opt_quiet && !opt_interactive) {
    log_printf(MSG_PASS1_FIN, comp_line + 1);
    log_printf("\n");
  }

  return (comp_error == 0);
}

/**
 * setup export table
 */
int comp_pass2_exports() {
  int i, j;

  for (i = 0; i < comp_expcount; i++) {
    bid_t pid;
    unit_sym_t *sym = comp_exptable.elem[i];

    // look on procedures/functions
    if ((pid = comp_udp_id(sym->symbol, 0)) != -1) {
      if (comp_udptable[pid].vid == INVALID_ADDR) {
        sym->type = stt_procedure;
      } else {
        sym->type = stt_function;
      }
      sym->address = comp_udptable[pid].ip;
      sym->vid = comp_udptable[pid].vid;
    } else {
      // look on variables
      pid = -1;
      for (j = 0; j < comp_varcount; j++) {
        if (strcmp(comp_vartable[j].name, sym->symbol) == 0) {
          pid = j;
          break;
        }
      }

      if (pid != -1) {
        sym->type = stt_variable;
        sym->address = 0;
        sym->vid = j;
      } else {
        sc_raise(MSG_EXP_SYM_NOT_FOUND, sym->symbol);
        return 0;
      }
    }
  }

  return (comp_error == 0);
}

/*
 * PASS 2
 */
int comp_pass2() {
  if (!opt_quiet && !opt_interactive) {
    log_printf(MSG_PASS2);
  }

  if (comp_proc_level) {
    sc_raise(MSG_MISSING_END_3);
  } else if (comp_prog.size) {
    bc_add_code(&comp_prog, kwSTOP);
    comp_first_data_ip = comp_prog.count;
    comp_pass2_scan();
    comp_optimise();
  }

  if (comp_block_level && (comp_error == 0)) {
    sc_raise(MSG_LOOPS_OPEN, comp_block_level);
  }
  if (comp_data.count) {
    bc_append(&comp_prog, &comp_data);
  }
  if (comp_expcount) {
    comp_pass2_exports();
  }
  return (comp_error == 0);
}

/*
 * final, create bytecode
 */
byte_code comp_create_bin() {
  int i;
  byte_code bc;
  byte *cp;
  bc_head_t hdr;
  uint32_t size;
  unit_file_t uft;

  if (!opt_quiet && !opt_interactive) {
    if (comp_unit_flag) {
      log_printf(MSG_CREATING_UNIT, comp_unit_name);
    } else {
      log_printf(MSG_CREATING_BC);
    }
  }

  memcpy(&hdr.sign, "SBEx", 4);
  hdr.ver = 2;
  hdr.sbver = SB_DWORD_VER;
#if defined(CPU_BIGENDIAN)
  hdr.flags = 1;
#else
  hdr.flags = 0;
#endif

  // executable header
  hdr.flags |= 4;
  hdr.bc_count = comp_prog.count;
  hdr.var_count = comp_varcount;
  hdr.lab_count = comp_labcount;
  hdr.data_ip = comp_first_data_ip;
  hdr.size = sizeof(bc_head_t) + comp_prog.count + (comp_labcount * ADDRSZ) +
             sizeof(unit_sym_t) * comp_expcount +
             sizeof(bc_lib_rec_t) * comp_libcount +
             sizeof(bc_symbol_rec_t) * comp_impcount;
  if (comp_unit_flag) {
    hdr.size += sizeof(unit_file_t);
  }

  hdr.lib_count = comp_libcount;
  hdr.sym_count = comp_impcount;
  if (comp_unit_flag) {
    memset(&uft, 0, sizeof(unit_file_t));

    // it is a unit... add more info
    bc.size = hdr.size;
    bc.code = malloc(bc.size);

    // unit header
    memcpy(&uft.sign, "SBUn", 4);
    uft.version = SB_DWORD_VER;

    strcpy(uft.base, comp_unit_name);
    uft.sym_count = comp_expcount;

    memcpy(bc.code, &uft, sizeof(unit_file_t));
    cp = bc.code + sizeof(unit_file_t);

    // unit symbol table (export)
    for (i = 0; i < uft.sym_count; i++) {
      unit_sym_t *sym = comp_exptable.elem[i];
      memcpy(cp, sym, sizeof(unit_sym_t));
      cp += sizeof(unit_sym_t);
    }

    // normal file
    memcpy(cp, &hdr, sizeof(bc_head_t));
    cp += sizeof(bc_head_t);
  } else {
    // simple executable
    bc.size = hdr.size + 4;
    bc.code = malloc(bc.size);
    cp = bc.code;
    memcpy(cp, &hdr, sizeof(bc_head_t));
    cp += sizeof(bc_head_t);
  }

  // append label table
  for (i = 0; i < comp_labcount; i++) {
    comp_label_t *label = comp_labtable.elem[i];
    memcpy(cp, &label->ip, ADDRSZ);
    cp += ADDRSZ;
  }

  // append library table
  for (i = 0; i < comp_libcount; i++) {
    bc_lib_rec_t *lib = comp_libtable.elem[i];
    memcpy(cp, lib, sizeof(bc_lib_rec_t));
    cp += sizeof(bc_lib_rec_t);
  }

  // append symbol table
  for (i = 0; i < comp_impcount; i++) {
    bc_symbol_rec_t *sym = comp_imptable.elem[i];
    memcpy(cp, sym, sizeof(bc_symbol_rec_t));
    cp += sizeof(bc_symbol_rec_t);
  }

  size = cp - bc.code;

  // the program itself
  memcpy(cp, comp_prog.ptr, comp_prog.count);

  size += comp_prog.count;

  // print statistics
  if (!opt_quiet && !opt_interactive) {
    log_printf("\n");
    log_printf(RES_NUMBER_OF_VARS, comp_varcount, comp_varcount - 18);
    log_printf(RES_NUMBER_OF_LABS, comp_labcount);
    log_printf(RES_NUMBER_OF_UDPS, comp_udpcount);
    log_printf(RES_CODE_SIZE, comp_prog.count);
    log_printf("\n");
    log_printf(RES_IMPORTED_LIBS, comp_libcount);
    log_printf(RES_IMPORTED_SYMS, comp_impcount);
    log_printf(RES_EXPORTED_SYMS, comp_expcount);
    log_printf("\n");
    log_printf(RES_FINAL_SIZE, size);
    log_printf("\n");
  }

  return bc;
}

/**
 * save binary
 *
 * @param h_bc is the memory-handle of the bytecode (created by create_bin)
 * @return non-zero on success
 */
int comp_save_bin(byte_code bc) {
  int h;
  char fname[OS_FILENAME_SIZE + 1];
  char *p;
  int result = 1;

  if ((opt_nosave && !comp_unit_flag) || opt_syntaxcheck) {
    return 1;
  }

  strcpy(fname, comp_file_name);
  p = strrchr(fname, '.');
  if (p) {
    *p = '\0';
  }
  strcat(fname, comp_unit_flag ? ".sbu" : ".sbx");

  h = open(fname, O_BINARY | O_RDWR | O_TRUNC | O_CREAT, 0660);
  if (h != -1) {
    write(h, (char *)bc.code, bc.size);
    close(h);
    if (!opt_quiet && !opt_interactive) {
      log_printf(MSG_BC_FILE_CREATED, fname);
    }
  } else {
    // non-fatal error
    result = 0;
  }

  return result;
}

/**
 * compiler - main
 *
 * @param sb_file_name the source file-name
 * @return non-zero on success
 */
int comp_compile(const char *sb_file_name) {
  char *source;
  int tid, prev_tid;
  int success = 0;
  byte_code bc;

  bc.code = NULL;
  bc.size = 0;

  tid = create_task(sb_file_name);
  prev_tid = activate_task(tid);

  comp_reset_externals();
  comp_init();                  // initialize compiler

  source = comp_load(sb_file_name); // load file and run pre-processor
  if (source) {
    success = comp_pass1(NULL, source); // PASS1
    free(source);
    if (success) {
      success = comp_pass2();   // PASS2
    }
    if (success) {
      success = comp_check_labels();
    }
    if (success) {
      bc = comp_create_bin();
      success = comp_save_bin(bc);
    }
  }

  int is_unit = comp_unit_flag;
  comp_close();
  close_task(tid);
  activate_task(prev_tid);
  ctask->bc_type = is_unit ? 2 : 1;

  if (opt_nosave && !is_unit) {
    ctask->bytecode = bc.code;
  } else if (bc.code) {
    free(bc.code);
  }

  return success;
}

/**
 * compiler - main.
 *
 * @param source buffer
 * @return non-zero on success
 */
int comp_compile_buffer(const char *source) {
  comp_init();                  // initialize compiler
  int success = comp_pass1(NULL, source); // PASS1
  if (success) {
    success = comp_pass2();     // PASS2
  }
  if (success) {
    success = comp_check_labels();
  }
  if (success) {
    byte_code bc = comp_create_bin(); // update task's bytecode
    ctask->bytecode = bc.code;
  }
  comp_close();
  return success;
}
