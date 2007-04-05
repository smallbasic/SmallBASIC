// -*- c-file-style: "java" -*-
// $Id: scan.c,v 1.23 2007-04-05 20:56:44 zeeb90au Exp $
// This file is part of SmallBASIC
//
// pseudo-compiler: Converts the source to byte-code.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#define SCAN_MODULE
#include "sys.h"
#include "device.h"
#include "kw.h"
#include "bc.h"
#include "scan.h"
#include "smbas.h"
#include "units.h"
#include "extlib.h"
#include "messages.h"

#if defined(_UnixOS)
#include <assert.h>
#endif

void comp_text_line(char *text) SEC(BCSC3);
int comp_single_line_if(char *text) SEC(BCSC3);
addr_t comp_search_bc(addr_t ip, code_t code) SEC(BCSC3);
char *comp_prepare_name(char *dest, const char *source, int size) SEC(BCSC3);
bid_t comp_label_getID(const char *label_name) SEC(BCSC2);
void comp_label_setip(bid_t idx) SEC(BCSC2);
void comp_prepare_udp_name(char *dest, const char *basename) SEC(BCSC2);
bid_t comp_udp_id(const char *proc_name, int scan_tree) SEC(BCSC3);
bid_t comp_add_udp(const char *proc_name) SEC(BCSC3);
bid_t comp_udp_setip(const char *proc_name, addr_t ip) SEC(BCSC2);
addr_t comp_udp_getip(const char *proc_name) SEC(BCSC3);
char *get_param_sect(char *text, const char *delim, char *dest) SEC(BCSC3);
int comp_check_labels(void) SEC(BCSC3);
int comp_check_lib(const char *name) SEC(BCSC3);
int comp_create_var(const char *name) SEC(BCSC2);
bid_t comp_var_getID(const char *var_name) SEC(BCSC3);
void comp_push(addr_t ip) SEC(BCSC3);
char *comp_next_char(char *source) SEC(BCSC3);
char *comp_prev_char(const char *root, const char *ptr) SEC(BCSC3);
const char *comp_next_word(const char *text, char *dest) SEC(BCSC3);
void comp_expression(char *expr, byte no_parser) SEC(BCSC3);
void comp_data_seg(char *source) SEC(BCSC2);
int comp_getlist(char *source, char_p_t * args, char *delims,
                 int maxarg) SEC(BCSC2);
char *comp_getlist_insep(char *source, char_p_t * args, char *sep, char *delims,
                         int maxarg, int *count) SEC(BCSC2);
void comp_array_params(char *src) SEC(BCSC2);
void comp_cmd_option(char *src) SEC(BCSC2);
int comp_error_if_keyword(const char *name) SEC(BCSC3);
void bc_store_exports(const char *slist) SEC(BCSC2);
addr_t comp_next_bc_cmd(addr_t ip) SEC(BCSC3);
addr_t comp_search_bc_eoc(addr_t ip) SEC(BCSC3);
addr_t comp_search_bc_stack(addr_t start, code_t code, int level) SEC(BCSC3);
addr_t comp_search_bc_stack_backward(addr_t start, code_t code,
                                     int level) SEC(BCSC3);
void print_pass2_stack(addr_t pos, code_t lcode, int level) SEC(BCSC2);
void comp_pass2_scan(void) SEC(BCSC3);
char *comp_format_text(const char *source) SEC(BCSC2);
void err_grmode() SEC(BCSC2);
void comp_preproc_import(const char *slist) SEC(BCSC2);
void comp_preproc_remove_line(char *s, int cmd_sep_allowed) SEC(BCSC2);
void comp_preproc_unit(char *name) SEC(BCSC2);
int comp_pass2_exports(void) SEC(BCSC2);
int comp_save_bin(mem_t h_bc) SEC(BCSC2);

extern void expr_parser(bc_t * bc) SEC(BCSCAN);

void err_comp_label_not_def(const char *name) SEC(BCSC2);
void err_comp_missing_lp() SEC(BCSC2);
void err_comp_missing_rp() SEC(BCSC2);
void err_wrongproc(const char *name) SEC(BCSC2);

extern void sc_raise2(const char *fmt, int line, const char *buff); // sberr

#if defined(_WinBCB)
// Win32GUI progress
extern void bcb_comp(int pass, int pmin, int pmax);     
#endif

#define SKIP_SPACES(p) \
    while (*p == ' ' || *p == '\t') { \
        p++; \
    }  

#define CHKOPT(x) \
   (strncmp(p, (x), strlen((x))) == 0)

#define GROWSIZE  128

void err_wrongproc(const char *name)
{
    sc_raise(MSG_WRONG_PROCNAME, name);
}

void err_comp_missing_rp()
{
    sc_raise(MSG_EXP_MIS_RP);
}

void err_comp_missing_lp()
{
    sc_raise(MSG_EXP_MIS_LP);
}

void err_comp_label_not_def(const char *name)
{
    sc_raise(MSG_LABEL_NOT_DEFINED, name);
}

#include "keywords.c"

/*
 * reset the external proc/func lists
 */
void comp_reset_externals(void)
{
    // reset functions
    if (comp_extfunctable) {
        tmp_free(comp_extfunctable);
    }
    comp_extfunctable = NULL;
    comp_extfunccount = comp_extfuncsize = 0;

    // reset procedures
    if (comp_extproctable) {
        tmp_free(comp_extproctable);
    }
    comp_extproctable = NULL;
    comp_extproccount = comp_extprocsize = 0;
}

/*
 * add an external procedure to the list
 */
int comp_add_external_proc(const char *proc_name, int lib_id)
{
    // TODO: scan for conflicts
    if (comp_extproctable == NULL) {
        comp_extprocsize = 16;
        comp_extproctable =
            (ext_proc_node_t *) tmp_alloc(sizeof(ext_proc_node_t) *
                                          comp_extprocsize);
    } else if (comp_extprocsize <= (comp_extproccount + 1)) {
        comp_extprocsize += 16;
        comp_extproctable =
            (ext_proc_node_t *) tmp_realloc(comp_extproctable,
                                            sizeof(ext_proc_node_t) *
                                            comp_extprocsize);
    }

    comp_extproctable[comp_extproccount].lib_id = lib_id;
    comp_extproctable[comp_extproccount].symbol_index = comp_impcount;
    strcpy(comp_extproctable[comp_extproccount].name, proc_name);
    strupper(comp_extproctable[comp_extproccount].name);

    // update imports table
    bc_symbol_rec_t sym;

    strcpy(sym.symbol, proc_name);  // symbol name
    sym.type = stt_procedure;       // symbol type
    sym.lib_id = lib_id;    // library id
    sym.sym_id = comp_impcount;     // symbol index
    
    // store it
    dbt_write(comp_imptable, comp_impcount, &sym, sizeof(bc_symbol_rec_t));
    comp_impcount++;
    comp_extproccount++;
    return comp_extproccount - 1;
}

/*
 * Add an external function to the list
 */
int comp_add_external_func(const char *func_name, int lib_id)
{
    // TODO: scan for conflicts
    if (comp_extfunctable == NULL) {
        comp_extfuncsize = 16;
        comp_extfunctable =
            (ext_func_node_t *) tmp_alloc(sizeof(ext_func_node_t) *
                                          comp_extfuncsize);
    } else if (comp_extfuncsize <= (comp_extfunccount + 1)) {
        comp_extfuncsize += 16;
        comp_extfunctable =
            (ext_func_node_t *) tmp_realloc(comp_extfunctable,
                                            sizeof(ext_func_node_t) *
                                            comp_extfuncsize);
    }

    comp_extfunctable[comp_extfunccount].lib_id = lib_id;
    comp_extfunctable[comp_extfunccount].symbol_index = comp_impcount;
    strcpy(comp_extfunctable[comp_extfunccount].name, func_name);
    strupper(comp_extfunctable[comp_extfunccount].name);

    // update imports table
    bc_symbol_rec_t sym;
    
    strcpy(sym.symbol, func_name);  // symbol name
    sym.type = stt_function;        // symbol type
    sym.lib_id = lib_id;    // library id
    sym.sym_id = comp_impcount;     // symbol index
    
    // store it
    dbt_write(comp_imptable, comp_impcount, &sym, sizeof(bc_symbol_rec_t));
    comp_impcount++;
    comp_extfunccount++;
    return comp_extfunccount - 1;
}

/*
 * returns the external procedure id
 */
int comp_is_external_proc(const char *name)
{
    int i;

    for (i = 0; i < comp_extproccount; i++) {
        if (strcmp(comp_extproctable[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * returns the external function id
 */
int comp_is_external_func(const char *name)
{
    int i;

    for (i = 0; i < comp_extfunccount; i++) {
        if (strcmp(comp_extfunctable[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 *   Notes:
 *       block_level = the depth of nested block
 *       block_id    = unique number of each block (based on stack use)
 *
 *       Example:
 *       ? xxx           ' level 0, id 0
 *       for i=1 to 20   ' level 1, id 1
 *           ? yyy       ' level 1, id 1
 *           if a=1      ' level 2, id 2 (our IF uses stack)
 *               ...     ' level 2, id 2
 *           else        ' level 2, id 2 // not 3
 *               ...     ' level 2, id 2
 *           fi          ' level 2, id 2
 *           if a=2      ' level 2, id 3
 *               ...     ' level 2, id 3
 *           fi          ' level 2, id 3
 *           ? zzz       ' level 1, id 1
 *       next            ' level 1, id 1
 *       ? ooo           ' level 0, id 0
 */
/*
 * error messages
 */
void sc_raise(const char *fmt, ...)
{
    char *buff;
    va_list ap;

    va_start(ap, fmt);

    comp_error = 1;

    buff = tmp_alloc(SB_SOURCELINE_SIZE + 1);
#if defined(_PalmOS)
    StrVPrintF(buff, fmt, ap);
#elif defined(_DOS)
    vsprintf(buff, fmt, ap);
#else
    vsnprintf(buff, SB_SOURCELINE_SIZE, fmt, ap);
#endif
    va_end(ap);

    sc_raise2(comp_bc_sec, comp_line, buff);    // sberr.h
    tmp_free(buff);
}

/*
 * prepare name (keywords, variables, labels, proc/func names)
 */
char *comp_prepare_name(char *dest, const char *source, int size)
{
    char *p = (char *)source;
    SKIP_SPACES(p);

    strncpy(dest, p, size);
    dest[size] = '\0';
    p = dest;
    while (*p &&
           (is_alpha(*p) || 
            is_digit(*p) || 
            *p == '$' || 
            *p == '/' || 
            *p == '_' ||
            *p == '.')) {
        p++;
    }
    *p = '\0';

    str_alltrim(dest);
    return dest;
}

/*
 * returns the ID of the label. If there is no one, then it creates one
 */
bid_t comp_label_getID(const char *label_name)
{
    bid_t idx = -1, i;
    char name[SB_KEYWORD_SIZE + 1];
    comp_label_t label;

    comp_prepare_name(name, label_name, SB_KEYWORD_SIZE);

    for (i = 0; i < comp_labcount; i++) {
        dbt_read(comp_labtable, i, &label, sizeof(comp_label_t));
        if (strcmp(label.name, name) == 0) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
#if !defined(OS_LIMITED)
        if (opt_verbose) {
            dev_printf(MSG_NEW_LABEL, comp_line, name, comp_labcount);
        }
#endif
        strcpy(label.name, name);
        label.ip = INVALID_ADDR;
        label.dp = INVALID_ADDR;
        label.level = comp_block_level;
        label.block_id = comp_block_id;

        dbt_write(comp_labtable, comp_labcount, &label, sizeof(comp_label_t));
        idx = comp_labcount;
        comp_labcount++;
    }

    return idx;
}

/*
 * set LABEL's position (IP)
 */
void comp_label_setip(bid_t idx)
{
    comp_label_t label;

    dbt_read(comp_labtable, idx, &label, sizeof(comp_label_t));
    label.ip = comp_prog.count;
    label.dp = comp_data.count;
    label.level = comp_block_level;
    label.block_id = comp_block_id;
    dbt_write(comp_labtable, idx, &label, sizeof(comp_label_t));
}

/*
 * returns the full-path UDP/UDF name
 */
void comp_prepare_udp_name(char *dest, const char *basename)
{
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
bid_t comp_udp_id(const char *proc_name, int scan_tree)
{
    bid_t i;
    char *name = comp_bc_temp, *p;
    char base[SB_KEYWORD_SIZE + 1];
    char *root;
    int len;

    if (scan_tree) {
        comp_prepare_name(base, baseof(proc_name, '/'), SB_KEYWORD_SIZE);
        root = tmp_strdup(comp_bc_proc);
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
                    tmp_free(root);
                    return i;
                }
            }

        } while (len);

        // not found
        tmp_free(root);
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
bid_t comp_add_udp(const char *proc_name)
{
    char *name = comp_bc_temp;
    bid_t idx = -1, i;

    comp_prepare_udp_name(name, proc_name);

    /*
      #if !defined(OS_LIMITED)
      // check variables for conflict
      for ( i = 0; i < comp_varcount; i ++ )  {
      if  ( strcmp(comp_vartable[i].name, name) == 0 )    {
      sc_raise("User-defined function/procedure name, '%s', conflicts with variable", name);
      break;
      }
      }
      #endif
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
            comp_udptable = tmp_realloc(comp_udptable, comp_udpsize * sizeof(comp_udp_t));
        }

        if (!(is_alpha(name[0]) || name[0] == '_')) {
            err_wrongproc(name);
        } else {
#if !defined(OS_LIMITED)
            if (opt_verbose)
                dev_printf(MSG_NEW_UDP, comp_line, name, comp_udpcount);
#endif
            comp_udptable[comp_udpcount].name = tmp_alloc(strlen(name) + 1);
            comp_udptable[comp_udpcount].ip = INVALID_ADDR;     // bc_prog.count;
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
bid_t comp_udp_setip(const char *proc_name, addr_t ip)
{
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
addr_t comp_udp_getip(const char *proc_name)
{
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
char *get_param_sect(char *text, const char *delim, char *dest)
{
    char *p = (char *)text;
    char *d = dest;
    int quotes = 0, level = 0, skip_ch = 0;

    if (p == NULL) {
        *dest = '\0';
        return 0;
    }

    while (is_space(*p)) {
        p++;
    }

    while (*p) {
        if (quotes) {
            if (*p == '\\' && *(p + 1) == '\"') {
                // add the escaped quote and continue
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
            };
        }

        // delim check
        if (delim != NULL && level <= 0 && quotes == 0) {
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
    str_alltrim(dest);
    return p;
}

/*
 */
int comp_geterror()
{
    return comp_error;
}

/*
 * checking for missing labels
 */
int comp_check_labels()
{
    bid_t i;
    comp_label_t label;

    for (i = 0; i < comp_labcount; i++) {
        dbt_read(comp_labtable, i, &label, sizeof(comp_label_t));
        if (label.ip == INVALID_ADDR) {
            err_comp_label_not_def(label.name);
            return 0;
        }
    }

    return 1;
}

/*
 * returns true if 'name' is a unit or c-module
 */
int comp_check_lib(const char *name)
{
    char tmp[SB_KEYWORD_SIZE + 1];
    char *p;
    int i;

    strcpy(tmp, name);
    p = strchr(tmp, '.');
    if (p) {
        *p = '\0';
        for (i = 0; i < comp_libcount; i++) {
            bc_lib_rec_t lib;
            dbt_read(comp_libtable, i, &lib, sizeof(bc_lib_rec_t));
            if (strcasecmp(lib.lib, tmp) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

/*
 */
int comp_create_var(const char *name)
{
    int idx = -1;

    if (!(is_alpha(name[0]) || name[0] == '_'))
        sc_raise(MSG_WRONG_VARNAME, name);
    else {
        // realloc table if it is needed
        if (comp_varcount >= comp_varsize) {
            comp_varsize += GROWSIZE;
            comp_vartable = tmp_realloc(comp_vartable, comp_varsize * sizeof(comp_var_t));
        }
#if !defined(OS_LIMITED)
        if (opt_verbose) {
            dev_printf(MSG_NEW_VAR, comp_line, name, comp_varcount);
        }
#endif
        comp_vartable[comp_varcount].name = tmp_alloc(strlen(name) + 1);
        strcpy(comp_vartable[comp_varcount].name, name);
        comp_vartable[comp_varcount].dolar_sup = 0;
        comp_vartable[comp_varcount].lib_id = -1;
        idx = comp_varcount;
        comp_varcount++;
    }
    return idx;
}

/*
 */
int comp_add_external_var(const char *name, int lib_id)
{
    int idx;

    idx = comp_create_var(name);
    comp_vartable[idx].lib_id = lib_id;

    if (lib_id & UID_UNIT_BIT) {
        // update imports table
        bc_symbol_rec_t sym;

        strcpy(sym.symbol, name);       // symbol name
        sym.type = stt_variable;        // symbol type
        sym.lib_id = lib_id;    // library id
        sym.sym_id = comp_impcount;     // symbol index
        sym.var_id = idx;       // variable index

        // store it
        dbt_write(comp_imptable, comp_impcount, &sym, sizeof(bc_symbol_rec_t));
        comp_impcount++;
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
bid_t comp_var_getID(const char *var_name)
{
    bid_t idx = -1, i;
    char tmp[SB_KEYWORD_SIZE + 1];
    char *name = comp_bc_temp;

    comp_prepare_name(tmp, baseof(var_name, '/'), SB_KEYWORD_SIZE);

    char* dot = strchr(tmp, '.');
    if (dot != 0 && *(dot+1) == 0) {
        // name ends with dot
        sc_raise(MSG_MEMBER_DOES_NOT_EXISTS, tmp);
        return 0;
    }

    // 
    // check for external
    // external variables are recognized by the 'class' name
    // example: my_unit.my_var
    // 
    // If the name is not found in comp_libtable then it 
    // is treated as a structure reference
    if (dot != 0 && comp_check_lib(tmp)) {
        for (i = 0; i < comp_varcount; i++) {
            if (strcmp(comp_vartable[i].name, tmp) == 0) {
                return i;
            }
        }

        sc_raise(MSG_MEMBER_DOES_NOT_EXISTS, tmp);
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
            char* dollar_name = tmp_alloc(strlen(comp_vartable[i].name) + 2);
            strcpy(dollar_name, comp_vartable[i].name);
            strcat(dollar_name, "$");
            if (strcmp(dollar_name, name) == 0) {
                idx = i;
                tmp_free(dollar_name);
                break;
            }
            tmp_free(dollar_name);
        }
    }

    if (idx == -1) {
        // variable not found; create a new one
        idx = comp_create_var(tmp);
    }
    return idx;
}

/*
 * returns true if 'name' is a user defined structure
 */
int comp_check_uds(const char *name, int name_len, comp_struct_t* uds, int ignore_dot)
{
    int i, cmp_len;
    for (i = 0; i < comp_udscount; i++) {
        dbt_read(comp_udstable, i, uds, sizeof(comp_struct_t));
        if (ignore_dot) {
            // compare up to but not including any final dot chars
            char* dot = strrchr(uds->name, '.');
            cmp_len = dot ? dot-uds->name : uds->name_len;
        } else {
            cmp_len = uds->name_len;
        }

        if (cmp_len == name_len && strncasecmp(uds->name, name, cmp_len) == 0) {
            return 1;
        }
    }
    return 0;
}

/*
 * returns the shared field_id for the given field name
 */
int comp_get_uds_field_id(const char* field_name)
{
    int i, cmp_len;
    comp_struct_t uds;
    for (i = 0; i < comp_udscount; i++) {
        dbt_read(comp_udstable, i, &uds, sizeof(comp_struct_t));
        char* dot = strrchr(uds.name, '.');
        if (dot && *(dot+1) && strcasecmp(dot+1, field_name) == 0) {
            return uds.field_id;
        }
    }
    return ++comp_next_field_id;
}

/*
 * add the named variable to the current position in the byte code stream
 * 
 * if the name 'foo' has already been used in a struct context, eg 'foo.x'
 * then the foo variable is added as kwTYPE_UDS. the actual structure is
 * appended to the byte code as a series of variable addresses. the byte 
 * following the kwTYPE_UDS var_id and the value of the struct variable
 * contain the struct starting address.
 * 
 */
void comp_add_variable(bc_t* bc, const char *var_name) {
    bid_t var_id = comp_var_getID(var_name);
    comp_struct_t uds, usd_old;
    
    if (comp_error) {
        return;
    }

    int name_len = strlen(var_name);
    char* dot = strrchr(var_name, '.');

    // compare all of var_name to the dot-less portion of existing variables
    if (comp_check_uds(var_name, name_len, &uds, 1)) {
        // bare name previously used in struct context
        bc_add_code(bc, kwTYPE_UDS);
        bc_add_addr(bc, var_id);
        bc_add_addr(bc, 0); // IP to struct block placeholder

        if (!comp_check_uds(var_name, name_len, &usd_old, 0)) {
            // base_id is copied from the found child element
            uds.var_id = var_id;
            uds.is_container = 1;
            uds.name_len = name_len;
            strcpy(uds.name, var_name);
            dbt_write(comp_udstable, comp_udscount, &uds, sizeof(comp_struct_t));
            comp_udscount++;
        }
        return;
    }

    bc_add_code(bc, kwTYPE_VAR);
    bc_add_addr(bc, var_id);

    if (dot != 0 && !comp_check_lib(var_name)) { 
        // not a module or unit
        // compare all of var_name to existing fields
        if (!comp_check_uds(var_name, name_len, &uds, 0)) {
            // unseen full-name in struct context, eg foo.x
            int base_id = var_id;

            // find any root sibling and use its base_id, eg foo.y
            if (comp_check_uds(var_name, dot-var_name, &uds, 1)) {
                base_id = uds.base_id;
            } else {
                // take any bare variable as the base_id
                int i;
                for (i = 0; i < comp_varcount; i++) {
                    if (dot-var_name == strlen(comp_vartable[i].name) &&
                        strncmp(comp_vartable[i].name, var_name, dot-var_name) == 0) {
                        base_id = i;
                        break;
                    }
                }
            }

            uds.name_len = name_len;
            uds.var_id = var_id;
            uds.is_container = 0;
            uds.base_id = base_id;
            uds.field_id = comp_get_uds_field_id(dot+1);
            strcpy(uds.name, var_name);
            dbt_write(comp_udstable, comp_udscount, &uds, sizeof(comp_struct_t));
            comp_udscount++;
        }
    }
}

/*
 * adds a mark in stack at the current code position
 */
void comp_push(addr_t ip)
{
    comp_pass_node_t node;

    strcpy(node.sec, comp_bc_sec);
    node.pos = ip;
    node.level = comp_block_level;
    node.block_id = comp_block_id;
    node.line = comp_line;
    dbt_write(comp_stack, comp_sp, &node, sizeof(comp_pass_node_t));
    comp_sp++;
}

/*
 * returns the keyword code 
 */
int comp_is_keyword(const char *name)
{
    int i, idx;
    byte dolar_sup = 0;

    // Code to enable the $ but not for keywords (INKEY$=INKEY, PRINT$=PRINT !!!)
    // I don't want to increase the size of keywords table.
    idx = strlen(name) - 1;
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
fcode_t comp_is_func(const char *name)
{
    fcode_t i;
    int idx;
    byte dolar_sup = 0;

    //      Code to enable the $ but not for keywords (INKEY$=INKEY, PRINT$=PRINT !!!)
    //      I don't want to increase the size of keywords table.
    idx = strlen(name) - 1;
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
pcode_t comp_is_proc(const char *name)
{
    pcode_t i;

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
int comp_is_special_operator(const char *name)
{
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
long comp_is_operator(const char *name)
{
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
char *comp_next_char(char *source)
{
    char *p = source;

    while (*p) {
        if (*p != ' ') {
            return p;
        }
        p++;
    }
    return p;
}

/*
 */
char *comp_prev_char(const char *root, const char *ptr)
{
    char *p = (char *)ptr;

    if (p > root) {
        p--;
    } else {
        return (char *)root;
    }
    while (p > root) {
        if (*p != ' ') {
            return p;
        }
        p--;
    }
    return p;
}

/**
 *   get next word
 *   if buffer's len is zero, then the next element is not a word
 *
 *   @param text the source 
 *   @param dest the buffer to store the result
 *   @return pointer of text to the next element
 */
const char *comp_next_word(const char *text, char *dest)
{
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

    if (is_alnum(*p) || *p == '_') {    // don't forget the numeric-labels
        while (is_alnum(*p) || (*p == '_') || (*p == '.')) {
            *d = *p;
            d++;
            p++;
        }
    }
    //      Code to kill the $
    //      if      ( *p == '$' )   
    //              p ++;
    //      Code to enable the $
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
 * scan expression
 */
void comp_expression(char *expr, byte no_parser)
{
    char *ptr = (char *)expr;
    long idx;
    int level = 0, check_udf = 0;
    int kw_exec_more = 0;
    int tp;
    addr_t w, stip, cip;
    long lv = 0;
    double dv = 0;
    bc_t bc;
    int addr_opr = 0;

    comp_use_global_vartable = 0;       // check local-variables first
    str_alltrim(expr);
    if (*ptr == '\0') {
        return;
    }

    bc_create(&bc);

    while (*ptr) {
        if (is_digit(*ptr) || *ptr == '.' ||
            (*ptr == '&' && strchr("XHOB", *(ptr + 1)))) {
            // A CONSTANT NUMBER
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
        } else if (*ptr == '\'' /* || *ptr == '#' */ ) {  // remarks
            break;
        } else if (is_alpha(*ptr) || *ptr == '?' || *ptr == '_') {
            // A NAME 
            ptr = (char *)comp_next_word(ptr, comp_bc_name);
            idx = comp_is_func(comp_bc_name);
            // special case for INPUT
            if (idx == kwINPUTF) {
                if (*comp_next_char(ptr) != '(') {
                    idx = -1;   // INPUT is SPECIAL SEPARATOR (OPEN...FOR INPUT...)
                }
            }

            if (idx != -1) {
                // IS A FUNCTION
                if (!kw_noarg_func(idx)) {
                    if (*comp_next_char(ptr) != '(') {
                        sc_raise(MSG_BF_ARGERR, comp_bc_name);
                    }
                }
                if (idx == kwCALLCF) {
                    bc_add_code(&bc, kwTYPE_CALL_UDF);
                    bc_add_addr(&bc, idx); // place holder
                    bc_add_addr(&bc, 0); // return-variable ID
                    bc_add_code(&bc, kwTYPE_LEVEL_BEGIN);
                    bc_add_code(&bc, kwTYPE_CALL_PTR); // next is address
                    // skip next ( since we already added kwTYPE_LEVEL_BEGIN
                    // to allow kwTYPE_CALL_PTR to be the next code
                    char* par = comp_next_char(ptr);
                    if (*par == '(') {
                        ptr = par+1;
                        level++;
                    }
                } else {
                    bc_add_fcode(&bc, idx);
                }
                check_udf++;
            } else {
                // CHECK SPECIAL SEPARATORS
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
                    // NOT A COMMAND, CHECK OPERATORS
                    idx = comp_is_operator(comp_bc_name);
                    if (idx != -1) {
                        bc_add_code(&bc, idx >> 8);
                        bc_add_code(&bc, idx & 0xFF);
                    } else {
                        // EXTERNAL FUNCTION
                        idx = comp_is_external_func(comp_bc_name);
                        if (idx != -1) {
                            bc_add_extfcode(&bc, comp_extfunctable[idx].lib_id,
                                            comp_extfunctable[idx].
                                            symbol_index);
                        } else {
                            idx = comp_is_keyword(comp_bc_name);
                            if (idx == -1) {
                                idx = comp_is_proc(comp_bc_name);
                            }
                            if (idx != -1) {
                                sc_raise(MSG_STATEMENT_ON_RIGHT, comp_bc_name);
                            } else {
                                // UDF OR VARIABLE
                                int udf = comp_udp_id(comp_bc_name, 1);
                                if (udf != -1) {
                                    // UDF
                                    if (addr_opr != 0) {
                                        // pointer to UDF
                                        bc_add_code(&bc, kwTYPE_PTR); 
                                    } else {
                                        bc_add_code(&bc, kwTYPE_CALL_UDF);
                                    }
                                    check_udf++;
                                    bc_add_addr(&bc, udf);
                                    bc_add_addr(&bc, 0);  // var place holder
                                } else {
                                    // VARIABLE
                                    if (addr_opr != 0) {
                                        sc_raise("PTR to invalid SUB/FUNC");
                                        kw_exec_more = level = 0;
                                        break;
                                    }
                                    SKIP_SPACES(ptr);
                                    if (*ptr == '(') {
                                        if (*(ptr + 1) == ')') {
                                            // null array
                                            ptr += 2;
                                        }
                                    }
                                    comp_add_variable(&bc, comp_bc_name);
                                }
                            }   // kw
                        }       // extf
                    }           // opr
                }               // sp. sep
            }                   // check sep
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
        } else if (*ptr == '[') {       // code-defined array
            ptr++;
            level++;
            bc_add_fcode(&bc, kwCODEARRAY);
            bc_add_code(&bc, kwTYPE_LEVEL_BEGIN);
        } else if (*ptr == '(') {
            // parenthesis
            level++;
            bc_add_code(&bc, kwTYPE_LEVEL_BEGIN);
            ptr++;
        } else if (*ptr == ')' || *ptr == ']') {
            // parenthesis
            bc_add_code(&bc, kwTYPE_LEVEL_END);
            level--;
            ptr++;
        } else if (is_space(*ptr)) {
            // null characters
            ptr++;
        } else {
            // operators
            if (*ptr == '+' || *ptr == '-') {
                bc_add_code(&bc, kwTYPE_ADDOPR);
                bc_add_code(&bc, *ptr);
            } else if (*ptr == '*' || *ptr == '/' || *ptr == '\\' ||
                       *ptr == '%') {
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
            bc_add_code(&bc, kwTYPE_EOC);
            //printf("=== before:\n"); hex_dump(bc.ptr, bc.count);
            expr_parser(&bc);
            //printf("=== after:\n");  hex_dump(bc.ptr, bc.count);
        }
        if (bc.count) {
            stip = comp_prog.count;
            bc_append(&comp_prog, &bc); // merge code segments

            // update pass2 stack-nodes
            if (check_udf) {
                cip = stip;
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
    comp_use_global_vartable = 0;       // check local-variables first
    bc_destroy(&bc);

    // do additional steps
    if (kw_exec_more) {
        comp_text_line(comp_bc_tmp2);
    }
}

/*
 * Converts DATA commands to bytecode
 */
void comp_data_seg(char *source)
{
    char *ptr = source;
    char *commap;
    long lv = 0;
    double dv = 0, sign = 1;
    char *tmp = comp_bc_temp;
    int quotes;
    int tp;

    while (*ptr) {
        SKIP_SPACES(ptr);

        if (*ptr == '\0') {
            break;
        } else if (*ptr == ',') {
            bc_add_code(&comp_data, kwTYPE_EOC);
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
            if ((*ptr == '-' || *ptr == '+') &&
                strchr("0123456789.", *(ptr + 1))) {
                if (*ptr == '-') {
                    sign = -1;
                }
                ptr++;
            } else {
                sign = 1;
            }
            if (is_digit(*ptr) || *ptr == '.'
                || (*ptr == '&' && strchr("XHOB", *(ptr + 1)))) {

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

    bc_add_code(&comp_data, kwTYPE_EOC);        // no bc_eoc
}

/*
 * Scans the 'source' for "names" separated by 'delims' and returns 
 * the elements (pointer in source) into args array.
 *
 * Returns the number of items
 */
int comp_getlist(char *source, char_p_t * args, char *delims, int maxarg)
{
    char *p, *ps;
    int count = 0;

    ps = p = source;
    while (*p) {
        if (strchr(delims, *p)) {
            *p = '\0';
            SKIP_SPACES(ps);
            args[count] = ps;
            count++;
            if (count == maxarg) {
                if (*ps) {
                    sc_raise(MSG_PARNUM_LIMIT, maxarg);
                }
                return count;
            }
            ps = p + 1;
        }

        p++;
    }

    if (*ps) {
        SKIP_SPACES(ps);
        if (*ps) {
            *p = '\0';
            args[count] = ps;
            count++;
        }
    }

    return count;
}

/*
 * returns a list of names
 *
 * the list is included between sep[0] and sep[1] characters
 * each element is separated by 'delims' characters
 *
 * the 'source' is the raw string (null chars will be placed at the end of each name)
 * the 'args' is the names (pointers on the 'source')
 * maxarg is the maximum number of names (actually the size of args)
 * the count is the number of names which are found by this routine.
 *
 * returns the next position in 'source' (after the sep[1])
 */
char *comp_getlist_insep(char *source, char_p_t * args, char *sep, char *delims,
                         int maxarg, int *count)
{
    char *p = source;
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
                    *count = comp_getlist(ps, args, delims, maxarg);
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
int comp_single_line_if(char *text)
{
    char *p = (char *)text;     // *text points to 'expr'
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
                bc_add_code(&comp_prog, kwTYPE_EOC);    // bc_eoc();

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
                            comp_text_line(buf);
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
                                strcat(buf, p);
                            } else
                                strcat(buf, p);

                            // 
                            break;
                        } else {
                            pelse = strstr(pelse + 1, LCN_ELSE);
                        }
                    } while (pelse != NULL);
                }
                // scan the rest commands
                comp_text_line(buf);
                // add EOC
                bc_eoc(&comp_prog);

                // add ENDIF
                comp_push(comp_prog.count);
                bc_add_ctrl(&comp_prog, kwENDIF, 0, 0);
                comp_block_level--;
                return 1;
            } else {             // *p == ':'
                return 0;
            }
        } else {
            break;
        }
    } while (pthen != NULL);

    return 0;                   // false
}

/*
 * array's args 
 */
void comp_array_params(char *src)
{
    char *p = src;
    char *ss = NULL, *se = NULL;
    int level = 0;

    while (*p) {
        switch (*p) {
        case '(':
            if (level == 0) {
                ss = p;
            }
            level++;
            break;
        case ')':
            level--;

            if (level == 0) {
                se = p;
                // store this index
                if (!ss) {
                    sc_raise(MSG_ARRAY_SE);
                } else {
                    *ss = ' ';
                    *se = '\0';

                    bc_add_code(&comp_prog, kwTYPE_LEVEL_BEGIN);
                    comp_expression(ss, 0);
                    bc_store1(&comp_prog, comp_prog.count - 1,
                              kwTYPE_LEVEL_END);

                    *ss = '(';
                    *se = ')';
                    ss = se = NULL;
                }
            }                   // lev = 0
            break;
        };

        p++;
    }

    // 
    if (level > 0) {
        sc_raise(MSG_ARRAY_MIS_RP);
    } else if (level < 0) {
        sc_raise(MSG_ARRAY_MIS_LP);
    }
}

/*
 * run-time options
 */
void comp_cmd_option(char *src)
{
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
        ;// ignore it 
    } else {
        sc_raise(MSG_OPTION_ERR, src);
    }
}

int comp_error_if_keyword(const char *name)
{
    // check if keyword
    if (!comp_error) {
        if ((comp_is_func(name) >= 0) ||
            (comp_is_proc(name) >= 0) ||
            (comp_is_special_operator(name) >= 0) ||
            (comp_is_keyword(name) >= 0) || (comp_is_operator(name) >= 0)) {
            sc_raise(MSG_IT_IS_KEYWORD, name);
        }
    }
    return comp_error;
}

/**
 * stores export symbols (in pass2 will be checked again)
 */
void bc_store_exports(const char *slist)
{
#if defined(OS_LIMITED)
    char_p_t pars[32];
#else
    char_p_t pars[256];
#endif
    int count = 0, i;
    char *newlist;
    unit_sym_t sym;

    newlist = (char *)tmp_alloc(strlen(slist) + 3);
    strcpy(newlist, "(");
    strcat(newlist, slist);
    strcat(newlist, ")");

#if defined(OS_LIMITED)
    comp_getlist_insep(newlist, pars, "()", ",", 32, &count);
#else
    comp_getlist_insep(newlist, pars, "()", ",", 256, &count);
#endif

    for (i = 0; i < count; i++) {
        strcpy(sym.symbol, pars[i]);
        dbt_write(comp_exptable, comp_expcount, &sym, sizeof(unit_sym_t));
        comp_expcount++;
    }

    tmp_free(newlist);
}

/*
 * PASS1: scan source line
 */
void comp_text_line(char *text)
{
    char *p;
    char *lb_end;
    char *last_cmd;
#if defined(OS_ADDR16)
    int idx;
#else
    long idx;
#endif
    int sharp, ladd, linc, ldec, decl = 0, vattr;
    int leqop;
    char pname[SB_KEYWORD_SIZE + 1], vname[SB_KEYWORD_SIZE + 1];

    if (comp_error) {
        return;
    }
    str_alltrim(text);
    p = text;
    
    // EOL
    if (*p == ':') {
        p++;
        comp_text_line(p);
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

    lb_end = p = (char *)comp_next_word(text, comp_bc_name);
    last_cmd = p;
    p = get_param_sect(p, ":", comp_bc_parm);

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

    // what's this ? 
    idx = comp_is_keyword(comp_bc_name);
    if (idx == kwREM) {
        return;                 // remarks... return
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
                comp_expression(comp_bc_parm, 0);
                bc_add_code(&comp_prog, kwTYPE_LEVEL_END);
            } else {
                // simple buildin procedure
                // there is no need to check it more...
                // save it and return (go to next)
                bc_add_pcode(&comp_prog, idx);
                comp_expression(comp_bc_parm, 0);
            }

            if (*p == ':') {    // command separator
                bc_eoc(&comp_prog);
                p++;
                comp_text_line(p);
            }
            return;
        }
    }

    if (idx == kwLET) {         // old-style keyword LET
        char *p;
        idx = -1;
        p = (char *)comp_next_word(comp_bc_parm, comp_bc_name);
        strcpy(comp_bc_parm, p);
    } else if (idx == kwDECLARE) {      // declaration
        char *p;
        decl = 1;
        p = (char *)comp_next_word(comp_bc_parm, comp_bc_name);
        idx = comp_is_keyword(comp_bc_name);
        if (idx == -1)
            idx = comp_is_proc(comp_bc_name);
        strcpy(comp_bc_parm, p);
        if (idx != kwPROC && idx != kwFUNC) {
            sc_raise(MSG_USE_DECL);
            return;
        }
    }
    if (idx == kwREM) {
        return;
    }

    sharp = (comp_bc_parm[0] == '#');   // if # -> file commands
    ladd = (strncmp(comp_bc_parm, "<<", 2) == 0);       // if << -> array, 

    // append
    linc = (strncmp(comp_bc_parm, "++", 2) == 0);
    ldec = (strncmp(comp_bc_parm, "--", 2) == 0);
    
    if (comp_bc_parm[1] == '=' && strchr("-+/\\*^%&|", comp_bc_parm[0])) {
        leqop = comp_bc_parm[0];
    } else {
        leqop = 0;
    }
    if ((comp_bc_parm[0] == '=' || ladd || linc || ldec || leqop) && (idx != -1)) {
        sc_raise(MSG_IT_IS_KEYWORD, comp_bc_name);
        return;
    }
    
    if ((idx == kwCONST) || 
        ((comp_bc_parm[0] == '=' || comp_bc_parm[0] == '(' ||
          ladd || linc || ldec || leqop) && (idx == -1))) {
        // 
        // LET/CONST commands
        // 
        char *parms = comp_bc_parm;
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
            strcat(comp_bc_parm, "+1");
        } else if (ldec) {
            bc_add_code(&comp_prog, kwLET);
            strcpy(comp_bc_parm, "=");
            strcat(comp_bc_parm, comp_bc_name);
            strcat(comp_bc_parm, "-1");
        } else if (leqop) {
            char *buf;
            int l;
            
            // a += 10: b -= 10 etc
            bc_add_code(&comp_prog, kwLET);
            l = strlen(comp_bc_parm) + strlen(comp_bc_name) + 1;
            buf = tmp_alloc(l);
            memset(buf, 0, l);
            strcpy(buf, "=");
            strcat(buf, comp_bc_name);
            buf[strlen(buf)] = leqop;
            strcat(buf, comp_bc_parm + 2);
            strcpy(comp_bc_parm, buf);
            tmp_free(buf);
        } else {
            bc_add_code(&comp_prog, kwLET);
        }

        comp_error_if_keyword(comp_bc_name);
        comp_add_variable(&comp_prog, comp_bc_name);

        if (!comp_error) {
            if (parms[0] == '(') {
                char *p = strchr(parms, '=');
                if (!p)
                    sc_raise(MSG_LET_MISSING_EQ);
                else {
                    if (*comp_next_char(parms + 1) == ')') {
                        // its the variable's name only
                        comp_expression(p, 0);
                    } else {
                        // ARRAY (LEFT)
                        *p = '\0';
                        comp_array_params(parms);

                        *p = '=';
                        if (!comp_error) {
                            bc_add_code(&comp_prog, kwTYPE_CMPOPR);
                            bc_add_code(&comp_prog, '=');
                            comp_expression(p + 1, 0);
                        }
                    }
                }
            } else {
                bc_add_code(&comp_prog, kwTYPE_CMPOPR);
                bc_add_code(&comp_prog, '=');
                comp_expression(parms + 1, 0);
            }
        }
    } else {
        // add generic command
#if defined(OS_LIMITED)
        char_p_t pars[32];
#else
        char_p_t pars[256];
#endif
        char *lpar_ptr, *eq_ptr, *p, *p_do, *n;
        int  keep_ip, udp, count, i;

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
                if (idx == kwFORSEP || idx == kwLOOPSEP || idx == kwPROCSEP
                    || idx == kwFUNCSEP) {
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
            // 
            // USER-DEFINED PROCEDURES/FUNCTIONS
            // 
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
                    // (now we just keep the rq space, pass2 will
                    // update that)
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
                            *lpar_ptr = '(';
#if defined(OS_LIMITED)
                            comp_getlist_insep(comp_bc_parm, pars, "()",
                                               ",", 32, &count);
#else
                            comp_getlist_insep(comp_bc_parm, pars, "()",
                                               ",", 256, &count);
#endif
                            bc_add_code(&comp_prog, kwTYPE_PARAM);
                            bc_add_code(&comp_prog, count);
                            
                            for (i = 0; i < count; i++) {
                                if ((strncmp(pars[i], LCN_BYREF_WRS, 6) == 0) ||
                                    (pars[i][0] == '@')) {
                                    if (pars[i][0] == '@') {
                                        comp_prepare_name(vname, pars[i] + 1,
                                                          SB_KEYWORD_SIZE);
                                    } else {
                                        comp_prepare_name(vname, pars[i] + 6,
                                                          SB_KEYWORD_SIZE);
                                    }
                                    vattr = 0x80;
                                } else {
                                    comp_prepare_name(vname, pars[i],
                                                      SB_KEYWORD_SIZE);
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
                            bc_add_code(&comp_prog, kwTYPE_PARAM);  // params
                            bc_add_code(&comp_prog, 0);     // pcount
                            // = 0
                        }

                        bc_eoc(&comp_prog); // EOC
                        // -----------------------------------------------
                        // scan for single-line function (DEF FN
                        // format)
                        if (eq_ptr && idx == kwFUNC) {
                            eq_ptr++;       // *eq_ptr was '\0'
                            SKIP_SPACES(eq_ptr);
                            if (strlen(eq_ptr)) {
                                char *macro = tmp_alloc(SB_SOURCELINE_SIZE + 1);
                                sprintf(macro, "%s=%s:%s", pname, eq_ptr, LCN_END);

                                // run comp_text_line again
                                comp_text_line(macro);
                                tmp_free(macro);
                            } else {
                                sc_raise(MSG_MISSING_UDP_BODY);
                            }
                        }
                    }
                }
            }
            break;
            
        case kwLOCAL:
            // local variables
#if defined(OS_LIMITED)
            count = comp_getlist(comp_bc_parm, pars, ",", 32);
#else
            count = comp_getlist(comp_bc_parm, pars, ",", 256);
#endif
            bc_add_code(&comp_prog, kwTYPE_CRVAR);
            bc_add_code(&comp_prog, count);
            for (i = 0; i < count; i++) {
                comp_prepare_name(vname, pars[i], SB_KEYWORD_SIZE);
                bc_add_addr(&comp_prog, comp_var_getID(vname));
            }
            break;
            
        case kwREM:
            return;

        case kwEXPORT: // export
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
                return;
            } else {
                comp_block_level++;
                comp_block_id++;
                comp_push(comp_prog.count);
                bc_add_ctrl(&comp_prog, idx, 0, 0);
                comp_expression(comp_bc_parm, 0);
                bc_add_code(&comp_prog, kwTYPE_EOC);        // bc_eoc();
            }
            break;

        case kwON:
            // 
            // ON x GOTO|GOSUB ...
            // 
            idx = kwONJMP;  // WARNING!
            comp_push(comp_prog.count);
            bc_add_ctrl(&comp_prog, idx, 0, 0);
            
            if ((p = strstr(comp_bc_parm, LCN_GOTO_WS)) != NULL) {
                bc_add_code(&comp_prog, kwGOTO);    // the command
                *p = '\0';
                p += 6;
                keep_ip = comp_prog.count;
                bc_add_code(&comp_prog, 0); // the counter
                
                // count = bc_scan_label_list(p);
#if defined(OS_LIMITED)
                count = comp_getlist(p, pars, ",", 32);
#else
                count = comp_getlist(p, pars, ",", 256);
#endif
                for (i = 0; i < count; i++) {
                    bc_add_addr(&comp_prog, comp_label_getID(pars[i]));     // IDs
                }

                if (count == 0) {
                    sc_raise(MSG_ON_GOTO_ERR);
                } else {
                    comp_prog.ptr[keep_ip] = count;
                }

                comp_expression(comp_bc_parm, 0);   // the expression
                bc_eoc(&comp_prog);
            } else if ((p = strstr(comp_bc_parm, LCN_GOSUB_WS)) != NULL) {
                bc_add_code(&comp_prog, kwGOSUB);   // the command
                *p = '\0';
                p += 7;
                keep_ip = comp_prog.count;
                bc_add_code(&comp_prog, 0); // the counter
                
                // count = bc_scan_label_list(p);
#if defined(OS_LIMITED)
                count = comp_getlist(p, pars, ",", 32);
#else
                count = comp_getlist(p, pars, ",", 256);
#endif
                for (i = 0; i < count; i++) {
                    bc_add_addr(&comp_prog, comp_label_getID(pars[i]));
                }
                if (count == 0) {
                    sc_raise(MSG_ON_GOSUB_ERR);
                } else {
                    comp_prog.ptr[keep_ip] = count;
                }
                comp_expression(comp_bc_parm, 0);   // the expression
                bc_eoc(&comp_prog);
            } else {
                sc_raise(MSG_ON_NOTHING);
            }
            break;

        case kwFOR:
            // 
            // FOR
            // 
            p = strchr(comp_bc_parm, '=');
            p_do = strstr(comp_bc_parm, LCN_DO_WS);

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
                    char* n = p;
                    strcpy(comp_bc_name, comp_bc_parm);
                    str_alltrim(comp_bc_name);
                    if (!is_alpha(*comp_bc_name)) {
                        sc_raise(MSG_FOR_COUNT_ERR, comp_bc_name);
                    } else {
                        char* p_lev = comp_bc_name;
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
                char* n = p;
                
                strcpy(comp_bc_name, comp_bc_parm);
                str_alltrim(comp_bc_name);
                if (!is_alpha(*comp_bc_name)) {
                    sc_raise(MSG_FOR_COUNT_ERR, comp_bc_name);
                } else {
                    char* p_lev = comp_bc_name;
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
            int index = strncasecmp("CASE ", comp_bc_parm, 5) == 0 ? 5 : 0;
            comp_expression(comp_bc_parm + index, 0);
            break;
            
        case kwCASE:
            // link to matched block or next CASE/END-SELECT
            if (!comp_bc_parm ||
                !comp_bc_parm[0] ||
                strncasecmp(LCN_ELSE, comp_bc_parm, 4) == 0) {
                comp_push(comp_prog.count);
                bc_add_ctrl(&comp_prog, kwCASE_ELSE, 0, 0);
            } else {
                comp_push(comp_prog.count);
                bc_add_ctrl(&comp_prog, idx, 0, 0);
                comp_expression(comp_bc_parm, 0);
            }
            break;

        case kwELSE:
        case kwELIF:
            comp_push(comp_prog.count);
            bc_add_ctrl(&comp_prog, idx, 0, 0);
            comp_expression(comp_bc_parm, 0);
            break;

        case kwENDIF:
        case kwNEXT:
            comp_push(comp_prog.count);
            bc_add_ctrl(&comp_prog, idx, 0, 0);
            comp_block_level--;
            break;

        case kwWEND:
        case kwUNTIL:
            comp_push(comp_prog.count);
            bc_add_ctrl(&comp_prog, idx, 0, 0);
            comp_block_level--;
            comp_expression(comp_bc_parm, 0);
            break;

        case kwSTEP:
        case kwTO:
        case kwIN:
        case kwTHEN:
        case kwCOS:
        case kwSIN:
        case kwLEN:
        case kwLOOP:   // functions...
            sc_raise(MSG_SPECIAL_KW_ERR, comp_bc_name);
            break;

        case kwRESTORE:
            comp_push(comp_prog.count);
            bc_add_code(&comp_prog, idx);
            bc_add_addr(&comp_prog, comp_label_getID(comp_bc_parm));
            break;

        case kwEND:
            if (strncmp(comp_bc_parm, LCN_IF, 2) == 0 ||
                strncmp(comp_bc_parm, LCN_SELECT, 6) == 0) {
                idx = strncmp(comp_bc_parm, LCN_IF, 2) == 0 ?
                    kwENDIF : kwENDSELECT;
                comp_push(comp_prog.count);
                bc_add_ctrl(&comp_prog, idx, 0, 0);
                comp_block_level--;
            } else if (comp_proc_level) {
                char *dol;
                
                // UDP/F RETURN
                dol = strrchr(comp_bc_proc, '/');
                if (dol) {
                    *dol = '\0';
                } else {
                    *comp_bc_proc = '\0';
                }
                comp_push(comp_prog.count);
                bc_add_code(&comp_prog, kwTYPE_RET);
                
                comp_proc_level--;
                comp_block_level--;
                comp_block_id++;
            } else {
                // END OF PROG
                bc_add_code(&comp_prog, idx);
            }
            break;

        case kwDATA:
            comp_data_seg(comp_bc_parm);
            break;

        case kwREAD:
            bc_add_code(&comp_prog, sharp? kwFILEREAD : idx);
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

        case -1:
            // EXTERNAL OR USER-DEFINED PROCEDURE
            udp = comp_is_external_proc(comp_bc_name);
            if (udp > -1) {
                bc_add_extpcode(&comp_prog, comp_extproctable[udp].lib_id,
                                comp_extproctable[udp].symbol_index);
                bc_add_code(&comp_prog, kwTYPE_LEVEL_BEGIN);
                comp_expression(comp_bc_parm, 0);
                bc_add_code(&comp_prog, kwTYPE_LEVEL_END);
            } else {
                udp = comp_udp_id(comp_bc_name, 1);
                if (udp == -1) {
                    udp = comp_add_udp(comp_bc_name);
                }
                comp_push(comp_prog.count);
                bc_add_ctrl(&comp_prog, kwTYPE_CALL_UDP, udp, 0);
                bc_add_code(&comp_prog, kwTYPE_LEVEL_BEGIN);
                comp_expression(comp_bc_parm, 0);
                bc_add_code(&comp_prog, kwTYPE_LEVEL_END);
            }
            break;
        
        default:
            // something else
            bc_add_code(&comp_prog, idx);
            comp_expression(comp_bc_parm, 0);
        }
    }

    if (*p == ':') {
        // command separator
        bc_eoc(&comp_prog);
        p++;
        comp_text_line(p);
    }
}

/*
 * skip command bytes
 */
addr_t comp_next_bc_cmd(addr_t ip)
{
    code_t code;
#if defined(ADDR16)
    word len;
#else
    dword len;
#endif

    code = comp_prog.ptr[ip];
    ip++;

    switch (code) {
    case kwTYPE_INT:           // integer
        ip += OS_INTSZ;
        break;
    case kwTYPE_NUM:           // number
        ip += OS_REALSZ;
        break;
    case kwTYPE_STR:           // string: [2/4B-len][data]
        memcpy(&len, comp_prog.ptr + ip, OS_STRLEN);
        len += OS_STRLEN;
        ip += len;
        break;
    case kwTYPE_CALLF:
    case kwTYPE_CALLP:         // [fcode_t]
        ip += CODESZ;
        break;

    case kwTYPE_UDS:
    case kwTYPE_CALLEXTF:
    case kwTYPE_CALLEXTP:      // [lib][index]
        ip += (ADDRSZ * 2);
        break;
    case kwEXIT:
    case kwTYPE_SEP:
    case kwTYPE_LOGOPR:
    case kwTYPE_CMPOPR:
    case kwTYPE_ADDOPR:
    case kwTYPE_MULOPR:
    case kwTYPE_POWOPR:
    case kwTYPE_UNROPR:        // [1B data]
        ip++;
        break;

    case kwRESTORE:
    case kwGOSUB:
    case kwTYPE_LINE:
    case kwTYPE_VAR:           // [addr|id]
    case kwSELECT:
        ip += ADDRSZ;
        break;
    case kwTYPE_PTR:
    case kwTYPE_CALL_UDP:
    case kwTYPE_CALL_UDF:      // [true-ip][false-ip]
        ip += BC_CTRLSZ;
        break;
    case kwGOTO:               // [addr][pop-count]
        ip += (ADDRSZ + 1);
        break;
    case kwTYPE_CRVAR:         // [1B count][addr1][addr2]...
        len = comp_prog.ptr[ip];
        ip += ((len * ADDRSZ) + 1);
        break;
    case kwTYPE_PARAM:         // [1B count] {[1B-pattr][addr1]} ...
        len = comp_prog.ptr[ip];
        ip += ((len * (ADDRSZ + 1)) + 1);
        break;
    case kwONJMP:              // [true-ip][false-ip] [GOTO|GOSUB]
        // [count] [addr1]...
        ip += (BC_CTRLSZ + 1);
        ip += (comp_prog.ptr[ip] * ADDRSZ);
        break;
    case kwOPTION:             // [1B-optcode][addr-data]
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
        ip += BC_CTRLSZ;
        break;
    };
    return ip;
}

/*
 * search for command (in byte-code)
 */
addr_t comp_search_bc(addr_t ip, code_t code)
{
    addr_t i = ip;

    do {
        if (code == comp_prog.ptr[i]) {
            return i;
        }
        i = comp_next_bc_cmd(i);
    } while (i < comp_prog.count);
    return INVALID_ADDR;
}

/*
 * search for End-Of-Command mark
 */
addr_t comp_search_bc_eoc(addr_t ip)
{
    addr_t i = ip;
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
addr_t comp_search_bc_stack(addr_t start, code_t code, int level)
{
    addr_t i;
    comp_pass_node_t node;

    for (i = start; i < comp_sp; i++) {
        dbt_read(comp_stack, i, &node, sizeof(comp_pass_node_t));

        if (comp_prog.ptr[node.pos] == code) {
            if (node.level == level) {
                return node.pos;
            }
        }
    }
    return INVALID_ADDR;
}

/*
 * search stack backward
 */
addr_t comp_search_bc_stack_backward(addr_t start, code_t code, int level)
{
    addr_t i = start;
    comp_pass_node_t node;

    for (; i < comp_sp; i--) {  
        // WARNING: ITS UNSIGNED, SO WE'LL SEARCH
        // IN RANGE [0..STK_COUNT]
        dbt_read(comp_stack, i, &node, sizeof(comp_pass_node_t));
        if (comp_prog.ptr[node.pos] == code) {
            if (node.level == level) {
                return node.pos;
            }
        }
    }
    return INVALID_ADDR;
}

/*
 * inspect the byte-code at the given location
 */
addr_t comp_next_bc_peek(addr_t start)
{
    comp_pass_node_t node;
    dbt_read(comp_stack, start, &node, sizeof(comp_pass_node_t));
    return comp_prog.ptr[node.pos];
}

/*
 * Advanced error messages:
 * Analyze LOOP-END errors
*/
void print_pass2_stack(addr_t pos, code_t lcode, int level)
{
#if !defined(OS_LIMITED)
    addr_t ip;
#endif
    addr_t i;
    int j, cs_idx;
    char cmd[16], cmd2[16];
    comp_pass_node_t node;
    code_t ccode[256], code;
    int csum[256];
    int cs_count;
    code_t start_code[] = { kwWHILE, kwREPEAT, kwIF, kwFOR, kwFUNC, 0 };
    code_t end_code[] = { kwWEND, kwUNTIL, kwENDIF, kwNEXT, kwTYPE_RET, 0 };
#if !defined(OS_LIMITED)
    int details = 1;
#endif
    char buff[256];
    code = lcode;

    kw_getcmdname(code, cmd);

    // search for closest keyword (forward)
#if !defined(OS_LIMITED)
    buff[0] = 0;
    dev_printf(MSG_DETAILED_REPORT_Q);
    dev_gets(buff, sizeof(buff));
    details = (buff[0] == 'y' || buff[0] == 'Y');

    if (details) {
        ip = comp_search_bc_stack(pos + 1, code, level - 1);
        if (ip == INVALID_ADDR) {
            ip = comp_search_bc_stack(pos + 1, code, level + 1);
            if (ip == INVALID_ADDR) {
                int cnt = 0;
                for (i = pos + 1; i < comp_sp; i++) {
                    dbt_read(comp_stack, i, &node, sizeof(comp_pass_node_t));
                    if (comp_prog.ptr[node.pos] == code) {
                        dev_printf
                            ("\n%s found on level %d (@%d) instead of %d (@%d+)\n",
                             cmd, node.level, node.pos, level, pos);
                        cnt++;
                        if (cnt > 3) {
                            break;
                        }
                    }
                }
            } else {
                dev_printf
                    ("\n%s found on level %d (@%d) instead of %d (@%d+)\n", cmd,
                     level + 1, node.pos, level, pos);
            }
        } else {
            dev_printf("\n%s found on level %d (@%d) instead of %d (@%d+)\n",
                       cmd, level - 1, node.pos, level, pos);
        }
    }
#endif

    // print stack
    cs_count = 0;
#if !defined(OS_LIMITED)
    if (details) {
        dev_printf("\n");
        dev_printf
            ("--- Pass 2 - stack ------------------------------------------------------\n");
        dev_printf("%s%4s  %16s %16s %6s %6s %5s %5s %5s\n", "  ", "   i",
                   "Command", "Section", "Addr", "Line", "Level", "BlkID",
                   "Count");
        dev_printf
            ("-------------------------------------------------------------------------\n");
    }
#endif
    for (i = 0; i < comp_sp; i++) {
        dbt_read(comp_stack, i, &node, sizeof(comp_pass_node_t));

        code = comp_prog.ptr[node.pos];
        if (node.pos != INVALID_ADDR) {
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
#if !defined(OS_LIMITED)
        if (details) {
            // info
            dev_printf("%s%4d: %16s %16s %6d %6d %5d %5d %5d\n",
                       ((i == pos) ? ">>" : "  "), i, cmd, node.sec, node.pos,
                       node.line, node.level, node.block_id, csum[cs_idx]);
        }
#endif
    }

    // sum
#if !defined(OS_LIMITED)
    if (details) {
        dev_printf("\n");
        dev_printf
            ("--- Sum -----------------------------------------------------------------\n");
        for (i = 0; i < cs_count; i++) {
            code = ccode[i];
            if (!kw_getcmdname(code, cmd))
                sprintf(cmd, "(%d)", code);
            dev_printf("%16s - %5d\n", cmd, csum[i]);
        }
    }
#endif

    // decide
    dev_printf("\n");
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
                dev_printf("Hint: Missing %d %s or there is/are %d more %s\n",
                           sa - sb, cmd2, sa - sb, cmd);
            } else {
                dev_printf("Hint: There is/are %d more %s or missing %d %s\n",
                           sb - sa, cmd2, sb - sa, cmd);
            }
        }
    }

    dev_printf("\n\n");
}

/*
 * PASS 2 (write jumps for IF,FOR,WHILE,REPEAT,etc)
 */
void comp_pass2_scan()
{
    addr_t i = 0, j, true_ip, false_ip, label_id, w;
    addr_t a_ip, b_ip, c_ip, count;
    code_t code;
    byte level;
    comp_pass_node_t node;
    comp_label_t label;

    if (!opt_quiet && !opt_interactive) {
#if defined(_UnixOS)
        if (isatty(STDOUT_FILENO))
#endif
            dev_printf(MSG_PASS2_COUNT, i, comp_sp);
    }
#if defined(_WinBCB)
    bcb_comp(2, i, comp_sp);
#endif

    // for each node in stack
    for (i = 0; i < comp_sp; i++) {
        if (!opt_quiet && !opt_interactive) {
#if defined(_UnixOS)
            if (isatty(STDOUT_FILENO))
#endif
                if ((i % SB_KEYWORD_SIZE) == 0) {
                    dev_printf(MSG_PASS2_COUNT, i, comp_sp);
                }
        }
#if defined(_WinBCB)
        if ((i % SB_KEYWORD_SIZE) == 0) {
            bcb_comp(2, i, comp_sp);
        }
#endif

        dbt_read(comp_stack, i, &node, sizeof(comp_pass_node_t));
        comp_line = node.line;
        strcpy(comp_bc_sec, node.sec);
        code = comp_prog.ptr[node.pos];

        if (code == kwTYPE_EOC || code == kwTYPE_LINE) {
            continue;
        }
//              debug (node.pos = the address of the error)
//
//              if      ( node.pos == 360 || node.pos == 361 )
//                      printf("=== stack code %d\n", code);

        if (code != kwGOTO &&
            code != kwRESTORE &&
            code != kwSELECT &&
            code != kwONJMP &&
            code != kwTYPE_PTR &&
            code != kwTYPE_CALL_UDP &&
            code != kwTYPE_CALL_UDF &&
            code != kwPROC && code != kwFUNC && code != kwTYPE_RET) {
            // default - calculate true-ip
            true_ip = comp_search_bc_eoc(node.pos + (BC_CTRLSZ + 1));
            memcpy(comp_prog.ptr + node.pos + 1, &true_ip, ADDRSZ);
        }

        switch (code) {
        case kwPROC:
        case kwFUNC:
            // update start's GOTO
            true_ip = comp_search_bc_stack(i + 1, kwTYPE_RET, node.level) + 1;
            if (true_ip == INVALID_ADDR) {
                sc_raise(MSG_UDP_MISSING_END);
                print_pass2_stack(i, kwTYPE_RET, node.level);
                return;
            }
            memcpy(comp_prog.ptr + node.pos - (ADDRSZ + 1), &true_ip, ADDRSZ);
            break;

        case kwRESTORE:
            // replace the label ID with the real IP
            memcpy(&label_id, comp_prog.ptr + node.pos + 1, ADDRSZ);
            dbt_read(comp_labtable, label_id, &label, sizeof(comp_label_t));
            count = comp_first_data_ip + label.dp;
            memcpy(comp_prog.ptr + node.pos + 1, &count, ADDRSZ);       
            // change LABEL-ID with DataPointer
            break;

        case kwTYPE_PTR:
        case kwTYPE_CALL_UDP:
        case kwTYPE_CALL_UDF:
            // update real IP
            memcpy(&label_id, comp_prog.ptr + node.pos + 1, ADDRSZ);
            true_ip = comp_udptable[label_id].ip + (ADDRSZ + 3);
            memcpy(comp_prog.ptr + node.pos + 1, &true_ip, ADDRSZ);

            // update return-var ID
            true_ip = comp_udptable[label_id].vid;
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &true_ip, ADDRSZ);
            break;

        case kwONJMP:
            // kwONJMP:1 trueip:2 falseip:2 command:1 count:1 label1:2
            // label2:2 ...
            count = comp_prog.ptr[node.pos + (ADDRSZ + ADDRSZ + 2)];

            true_ip = comp_search_bc_eoc(node.pos + BC_CTRLSZ + (count * ADDRSZ) + 3);
            memcpy(comp_prog.ptr + node.pos + 1, &true_ip, ADDRSZ);

            // change label IDs with the real IPs
            for (j = 0; j < count; j++) {
                memcpy(&label_id,
                       comp_prog.ptr + node.pos + (j * ADDRSZ) + (ADDRSZ +
                                                                  ADDRSZ + 3),
                       ADDRSZ);
                dbt_read(comp_labtable, label_id, &label, sizeof(comp_label_t));
                w = label.ip;
                memcpy(comp_prog.ptr + node.pos + (j * ADDRSZ) +
                       (ADDRSZ + ADDRSZ + 3), &w, ADDRSZ);
            }
            break;

        case kwGOTO:           // LONG JUMPS
            memcpy(&label_id, comp_prog.ptr + node.pos + 1, ADDRSZ);
            dbt_read(comp_labtable, label_id, &label, sizeof(comp_label_t));
            w = label.ip;
            memcpy(comp_prog.ptr + node.pos + 1, &w, ADDRSZ);   
            // change LABEL-ID with IP
            level = comp_prog.ptr[node.pos + (ADDRSZ + 1)];
            comp_prog.ptr[node.pos + (ADDRSZ + 1)] = 0; // number of POPs

            if (level >= label.level) {
                // number of POPs
                comp_prog.ptr[node.pos + (ADDRSZ + 1)] = level - label.level;
            } else {
                // number of POPs 
                comp_prog.ptr[node.pos + (ADDRSZ + 1)] = 0;   
            } 
            break;

        case kwFOR:
            a_ip = comp_search_bc(node.pos + (ADDRSZ + ADDRSZ + 1), kwTO);
            b_ip = comp_search_bc(node.pos + (ADDRSZ + ADDRSZ + 1), kwIN);
            if (a_ip < b_ip) {
                b_ip = INVALID_ADDR;
            } else if (a_ip > b_ip) {
                a_ip = b_ip;
            }
            false_ip = comp_search_bc_stack(i + 1, kwNEXT, node.level);

            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_NEXT);
                print_pass2_stack(i, kwNEXT, node.level);
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
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwWHILE:
            false_ip = comp_search_bc_stack(i + 1, kwWEND, node.level);

            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_WEND);
                print_pass2_stack(i, kwWEND, node.level);
                return;
            }
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwREPEAT:
            false_ip = comp_search_bc_stack(i + 1, kwUNTIL, node.level);

            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_UNTIL);
                print_pass2_stack(i, kwUNTIL, node.level);
                return;
            }
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwUSE:
            true_ip = node.pos + (ADDRSZ + ADDRSZ + 1);
            false_ip = comp_search_bc_eoc(true_ip);
            memcpy(comp_prog.ptr + node.pos + 1, &true_ip, ADDRSZ);
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwIF:
        case kwELIF:
            a_ip = comp_search_bc_stack(i + 1, kwENDIF, node.level);
            b_ip = comp_search_bc_stack(i + 1, kwELSE, node.level);
            c_ip = comp_search_bc_stack(i + 1, kwELIF, node.level);

            false_ip = a_ip;
            if (b_ip != INVALID_ADDR && b_ip < false_ip) {
                false_ip = b_ip;
            }
            if (c_ip != INVALID_ADDR && c_ip < false_ip) {
                false_ip = c_ip;
            }
            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_ENDIF_OR_ELSE);
                print_pass2_stack(i, kwENDIF, node.level);
                return;
            }

            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwELSE:
            false_ip = comp_search_bc_stack(i + 1, kwENDIF, node.level);

            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_ENDIF);
                print_pass2_stack(i, kwENDIF, node.level);
                return;
            }

            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwTYPE_RET:
            break;

        case kwWEND:
            false_ip =
                comp_search_bc_stack_backward(i - 1, kwWHILE, node.level);
            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_WHILE);
                print_pass2_stack(i, kwWHILE, node.level);
                return;
            }
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwUNTIL:
            false_ip =
                comp_search_bc_stack_backward(i - 1, kwREPEAT, node.level);
            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_REPEAT);
                print_pass2_stack(i, kwREPEAT, node.level);
                return;
            }
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwNEXT:
            false_ip = comp_search_bc_stack_backward(i - 1, kwFOR, node.level);
            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_FOR);
                print_pass2_stack(i, kwFOR, node.level);
                return;
            }
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwENDIF:
            false_ip = comp_search_bc_stack_backward(i - 1, kwIF, node.level);
            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_IF);
                print_pass2_stack(i, kwIF, node.level);
                return;
            }
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwSELECT:
            // next instruction should be CASE
            false_ip = comp_next_bc_peek(i + 1);
            if (false_ip != kwCASE && false_ip != kwCASE_ELSE) {
                sc_raise(MSG_MISSING_CASE);
                print_pass2_stack(i, kwCASE, node.level);
                return;
            }
            break;

        case kwCASE:
            // false path is either next case statement or "end select"
            false_ip = comp_search_bc_stack(i + 1, kwCASE, node.level);
            if (false_ip == INVALID_ADDR) {
                false_ip = comp_search_bc_stack(i + 1, kwCASE_ELSE, node.level);
                if (false_ip == INVALID_ADDR) {
                    false_ip = comp_search_bc_stack(i + 1, kwENDSELECT, node.level);
                    if (false_ip == INVALID_ADDR) {
                        sc_raise(MSG_MISSING_END_SELECT);
                        print_pass2_stack(i, kwCASE, node.level);
                        return;
                    }
                }
            }
            // if expression returns false jump to the next case
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwCASE_ELSE:
            // check for END SELECT statement
            false_ip = comp_search_bc_stack(i + 1, kwENDSELECT, node.level);
            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_END_SELECT);
                print_pass2_stack(i, kwCASE, node.level);
                return;
            }
            // validate no futher CASE expr statements
            j = comp_search_bc_stack(i + 1, kwCASE, node.level);
            if (j != INVALID_ADDR && j < false_ip) {
                sc_raise(MSG_CASE_CASE_ELSE);
                print_pass2_stack(i, kwCASE, node.level);
                return;
            }
            // validate no futher CASE ELSE expr statements
            j = comp_search_bc_stack(i + 1, kwCASE_ELSE, node.level);
            if (j != INVALID_ADDR && j < false_ip) {
                sc_raise(MSG_CASE_CASE_ELSE);
                print_pass2_stack(i, kwCASE_ELSE, node.level);
                return;
            }
            // if the expression is false jump to the end-select
            memcpy(comp_prog.ptr + node.pos + (ADDRSZ + 1), &false_ip, ADDRSZ);
            break;

        case kwENDSELECT:
            false_ip = comp_search_bc_stack_backward(i - 1, kwSELECT, node.level);
            if (false_ip == INVALID_ADDR) {
                sc_raise(MSG_MISSING_SELECT);
                print_pass2_stack(i, kwSELECT, node.level);
                return;
            }
            break;
        };
    }

    if (!opt_quiet && !opt_interactive) {
        dev_printf(MSG_PASS2_COUNT, comp_sp, comp_sp);
        dev_printf("\n");
    }
#if defined(_WinBCB)
    bcb_comp(2, comp_sp, comp_sp);
#endif
}

/*
 * initialize compiler
 */
void comp_init()
{
    comp_bc_sec = tmp_alloc(SB_KEYWORD_SIZE + 1);
    memset(comp_bc_sec, 0, SB_KEYWORD_SIZE + 1);
    comp_bc_name = tmp_alloc(SB_SOURCELINE_SIZE + 1);
    comp_bc_parm = tmp_alloc(SB_SOURCELINE_SIZE + 1);
    comp_bc_temp = tmp_alloc(SB_SOURCELINE_SIZE + 1);
    comp_bc_tmp2 = tmp_alloc(SB_SOURCELINE_SIZE + 1);
    comp_bc_proc = tmp_alloc(SB_SOURCELINE_SIZE + 1);

    comp_line = 0;
    comp_error = 0;
    comp_labcount = 0;
    comp_expcount = 0;
    comp_impcount = 0;
    comp_libcount = 0;
    comp_varcount = 0;
    comp_udscount = 0;
    comp_next_field_id = 0;
    comp_sp = 0;
    comp_udpcount = 0;
    comp_block_level = 0;
    comp_block_id = 0;
    comp_unit_flag = 0;
    comp_first_data_ip = INVALID_ADDR;
    comp_proc_level = 0;
    comp_bc_proc[0] = '\0';

    comp_vartable = (comp_var_t *) tmp_alloc(GROWSIZE * sizeof(comp_var_t));
    comp_udptable = (comp_udp_t *) tmp_alloc(GROWSIZE * sizeof(comp_udp_t));

    sprintf(comp_bc_temp, "SBI-LBL%d", ctask->tid);
    comp_labtable = dbt_create(comp_bc_temp, 0);
    sprintf(comp_bc_temp, "SBI-STK%d", ctask->tid);
    comp_stack = dbt_create(comp_bc_temp, 0);
    sprintf(comp_bc_temp, "SBI-ILB%d", ctask->tid);
    comp_libtable = dbt_create(comp_bc_temp, 0);
    sprintf(comp_bc_temp, "SBI-IMP%d", ctask->tid);
    comp_imptable = dbt_create(comp_bc_temp, 0);
    sprintf(comp_bc_temp, "SBI-EXP%d", ctask->tid);
    comp_exptable = dbt_create(comp_bc_temp, 0);
    sprintf(comp_bc_temp, "SBI-UDS%d", ctask->tid);
    comp_udstable = dbt_create(comp_bc_temp, 0);

    comp_varsize = comp_udpsize = GROWSIZE;
    comp_varcount = comp_labcount = comp_sp = comp_udpcount = 0;

    bc_create(&comp_prog);
    bc_create(&comp_data);

#if !defined(_UnixOS)
    if (!comp_vartable ||
        comp_imptable == -1 ||
        comp_libtable == -1 ||
        comp_exptable == -1 ||
        comp_udstable == -1 ||
        comp_labtable == -1 || !comp_udptable || comp_stack == -1)
        panic("comp_init(): OUT OF MEMORY");
#else
    assert(comp_vartable != 0);
    assert(comp_imptable != -1);
    assert(comp_libtable != -1);
    assert(comp_exptable != -1);
    assert(comp_labtable != -1);
    assert(comp_udptable != 0);
    assert(comp_stack != -1);
    assert(comp_udstable != -1);
#endif

    dbt_prealloc(comp_labtable, os_cclabs1, sizeof(comp_label_t));
    dbt_prealloc(comp_stack, os_ccpass2, sizeof(comp_pass_node_t));

    // create system variables
    comp_var_getID(LCN_SV_OSVER);
    comp_vartable[comp_var_getID(LCN_SV_OSNAME)].dolar_sup = 1;
    comp_var_getID(LCN_SV_SBVER);
    comp_var_getID(LCN_SV_PI);
    comp_var_getID(LCN_SV_XMAX);
    comp_var_getID(LCN_SV_YMAX);
    comp_var_getID(LCN_SV_BPP);
    comp_var_getID(LCN_SV_TRUE);
    comp_var_getID(LCN_SV_FALSE);
    comp_var_getID(LCN_SV_LINECHART);
    comp_var_getID(LCN_SV_BARCHART);
    comp_vartable[comp_var_getID(LCN_SV_CWD)].dolar_sup = 1;
    comp_vartable[comp_var_getID(LCN_SV_HOME)].dolar_sup = 1;
    comp_vartable[comp_var_getID(LCN_SV_COMMAND)].dolar_sup = 1;
    comp_var_getID(LCN_SV_X);   // USE keyword
    comp_var_getID(LCN_SV_Y);   // USE keyword
    comp_var_getID(LCN_SV_Z);   // USE keyword
    comp_var_getID(LCN_SV_VADR);
}

/*
 * clean up
 */
void comp_close()
{
    int i;

    bc_destroy(&comp_prog);
    bc_destroy(&comp_data);

    for (i = 0; i < comp_varcount; i++) {
        tmp_free(comp_vartable[i].name);
    }
    for (i = 0; i < comp_udpcount; i++) {
        tmp_free(comp_udptable[i].name);
    }
    tmp_free(comp_vartable);
    dbt_close(comp_labtable);
    dbt_close(comp_exptable);
    dbt_close(comp_imptable);
    dbt_close(comp_libtable);
    dbt_close(comp_stack);
    dbt_close(comp_udstable);
    tmp_free(comp_udptable);

    comp_varcount = comp_labcount = comp_sp = comp_udpcount = 0;
    comp_libcount = comp_impcount = comp_expcount = 0;

    tmp_free(comp_bc_proc);
    tmp_free(comp_bc_tmp2);
    tmp_free(comp_bc_temp);
    tmp_free(comp_bc_parm);
    tmp_free(comp_bc_name);
    tmp_free(comp_bc_sec);
    comp_reset_externals();
}

/*
 * returns true if the 'fileName' exists
 */
int comp_bas_exist(const char *basfile)
{
    int check = 0;
    char *p, *fileName;
#if defined(_PalmOS)
    LocalID lid;
#endif

    fileName = tmp_alloc(strlen(basfile) + 5);
    strcpy(fileName, basfile);

    p = strchr(fileName, '.');
    if (!p) {
        strcat(fileName, ".bas");
    }
#if defined(_PalmOS)
    lid = DmFindDatabase(0, fileName);
    check = (lid != 0);
#elif defined(_VTOS)
    {
        FILE *fp;
        check = FALSE;
        fp = fopen(fileName, "rb");
        if (fp) {
            fclose(fp);
            check = TRUE;
        }
    }
#else
#if !defined(_UnixOS)
    check = (access(fileName, 0) == 0);
#else
    check = (access(fileName, R_OK) == 0);
#endif
#endif

    tmp_free(fileName);
    return check;
}

/*
 * load a source file
 */
char *comp_load(const char *file_name)
{
    char *buf = NULL;

#if defined(_PalmOS)
    // PalmOS uses databases instead of stream
    DmOpenRef fp;
    LocalID lid;
    int l, i, size = 0;
    VoidPtr rec_p = NULL;
    VoidHand rec_h = NULL;

    strcpy(comp_file_name, file_name);
    lid = DmFindDatabase(0, (char *)file_name);
    fp = DmOpenDatabase(0, lid, dmModeReadWrite);
    if (!fp) {
        panic("LOAD: CAN'T OPEN FILE %s", file_name);
        return NULL;
    }
    l = DmNumRecords(fp) - 1;
    if (l <= 0) {
        panic("LOAD: BAD FILE STRUCTURE %s", file_name);
        return NULL;
    }

    for (i = 0; i < l; i++) {
        rec_h = DmGetRecord(fp, i + 1);
        if (!rec_h) {
            panic("LOAD: CAN'T GET RECORD %s", file_name);
        }
        rec_p = mem_lock(rec_h);
        if (!rec_p)
            panic("LOAD: CAN'T LOCK RECORD %s", file_name);

//              if      ( i == 0 && strlen(rec_p+6) == 0 )
//                      bc_scan("Main", rec_p+70);      // + sizeof(sec_t);
//              else
//                      bc_scan(rec_p+6, rec_p+70);     // + sizeof(sec_t);

        if (i == 0) {
            size = strlen(rec_p + 70) + 1;
            buf = tmp_alloc(size + 1);
            strcpy(buf, rec_p + 70);
            buf[size] = '\0';
        } else {
            size += strlen(rec_p + 70);
            buf = tmp_realloc(buf, size);
            strcat(buf, rec_p + 70);
            buf[size] = '\0';
        }

        mem_unlock(rec_h);
        DmReleaseRecord(fp, i + 1, 0);
    }

    DmCloseDatabase(fp);
    return buf;

#else
    // --- normal ---
    int h;

    strcpy(comp_file_name, file_name);
    h = open(comp_file_name, O_BINARY | O_RDWR, 0660);
    if (h == -1) {
#if defined(__CYGWIN__)
        char temp[1024];
        getcwd(temp, 1024);
        panic(MSG_CANT_OPEN_FILE_AT, comp_file_name, temp);
#else
        panic(MSG_CANT_OPEN_FILE, comp_file_name);
#endif
    } else {
        int size;

        size = lseek(h, 0, SEEK_END);
        lseek(h, 0, SEEK_SET);

        buf = (char *)tmp_alloc(size + 1);
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
 * TODO: join-lines character (&)
 *
 * returns a newly created string
 */
char *comp_format_text(const char *source)
{
    const char *p;
    char *ps;
    int quotes = 0;
    char *new_text;
    int sl, last_ch = 0, i;
    char *last_nonsp_ptr;
    int adj_line_num = 0;

    sl = strlen(source);
    new_text = tmp_alloc(sl + 2);
    memset(new_text, 0, sl + 2);

    comp_line = 0;
    p = source;
    last_nonsp_ptr = ps = new_text;
    while (*p) {
        if (!quotes) {
            switch (*p) {
            case '\n':         // new line
                if (*last_nonsp_ptr == '&') {   // join lines
                    p++;
                    *last_nonsp_ptr = ' ';
                    if (*(last_nonsp_ptr - 1) == ' ') {
                        ps = last_nonsp_ptr;
                    } else {
                        ps = last_nonsp_ptr + 1;
                    }
                    adj_line_num++;
                } else {
                    for (i = 0; i <= adj_line_num; i++) {
                        *ps++ = '\n';   // at least one nl
                    }
                    adj_line_num = 0;
                    p++;
                }

                SKIP_SPACES(p);
                comp_line++;
                last_ch = '\n';
                last_nonsp_ptr = ps - 1;
                break;

            case '\'':         // remarks
                // skip the rest line
                while (*p) {
                    if (*p == '\n') {
                        break;
                    }
                    p++;
                }
                break;

            case ' ':          // spaces
            case '\t':
                if (last_ch == ' ' || last_ch == '\n') {
                    p++;
                } else {
                    *ps++ = ' ';
                    p++;
                    last_ch = ' ';
                }
                break;

            case '\"':         // quotes
                quotes = !quotes;
                last_nonsp_ptr = ps;
                *ps++ = last_ch = *p++;
                break;

            default:
                if ((strcaselessn(p, LCN_REM_1, 5) == 0) ||
                    (strcaselessn(p, LCN_REM_2, 5) == 0) ||
                    (strcaselessn(p, LCN_REM_3, 4) == 0 && last_ch == '\n') ||
                    (strcaselessn(p, LCN_REM_4, 4) == 0 && last_ch == '\n')
                    ) {

                    // skip the rest line
                    while (*p) {
                        if (*p == '\n')
                            break;
                        p++;
                    }
                    break;
                } else {
                    if ((*p > ' ') || (*p < 0)) {       // simple
                        // code-character
                        last_nonsp_ptr = ps;
                        *ps++ = last_ch = to_upper(*p);
                        p++;
                    } else
                        p++;
                    // else ignore it
                }
            }
        } else {                // in quotes
            if (*p == '\\' && *(p + 1) == '\"') {
                // add the escaped quote and continue
                *ps++ = *p++;
            } else if (*p == '\"' || *p == '\n') {
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
void err_grmode()
{
    // dev_printf() instead of sc_raise()... it is just a warning...
#if !defined(OS_LIMITED)
    dev_printf(MSG_GRMODE_ERR);
#endif
}

void comp_preproc_grmode(const char *source)
{
    char *p, *v;
    int x, y, b;

    // prepare the string (copy it to comp_bc_tmp2)
    // we use second buffer because we want to place some '\0' characters
    // into the buffer
    // in a non-SB code, there must be a dynamic allocation
    strncpy(comp_bc_tmp2, source, 32);
    comp_bc_tmp2[31] = '\0';    // safe paranoia
    p = comp_bc_tmp2;

    // searching the end of the string
    while (*p) {                // while *p is not '\0'
        if (*p == '\n' || *p == ':') {  // yeap, we must close the string
            // here (enter or
            // command-seperator) 
            // it is supposed that remarks had already removed from source
            *p = '\0';          // terminate the string
            break;
        }
        p++;                    // next
    }

    // get parameters
    p = comp_bc_tmp2;
    SKIP_SPACES(p);

    // the width
    v = p;                      // 'v' points to first letter of 'width',
    // (1024x768)
    // ........................................^ <- p, v
    p = strchr(v, 'X');         // search for the end of 'width' parameter 
                                // 
    // (1024x768). Remeber that the string is
    // in upper-case
    // .............................................^ <- p
    if (!p) {                   // we don't accept one parameter, the
        // width must followed by the height
        // so, if 'X' delimiter is omitted, there is no height parameter
        err_grmode();
        return;
    }
    *p = '\0';                  // we close the string at X position
    // (example: "1024x768" it will be
    // "1024\0768")
    x = xstrtol(v);             // now the v points to a string-of-digits, 
                                // 
    // we can perform atoi()
    // (xstrtol()=atoi())
    p++;
    v = p;                      // v points to first letter of 'height'
    // (1024x768x24)
    // ...........................................^ <- v

    // the height
    p = strchr(v, 'X');         // searching for the end of 'height'
    // (1024x768x24)
    // ...........................................^ <- p
    if (p) {                    // if there is a 'X' delimiter, then the
        // 'bpp' is followed, so, we need
        // different path
        *p = '\0';              // we close the string at second's X
        // position
        y = xstrtol(v);         // now the v points to a string-of-digits, 
                                // 
        // we can perform atoi()
        // (xstrtol()=atoi())

        p++;
        v = p;                  // v points to first letter of 'bpp'
        // (1024x768x24)
        // ............................................^ <- v

        // the bits-per-pixel
        if (strlen(v))          // if *v != '\0', there is actually a
            // string
            b = xstrtol(v);     // integer value of (v). v points to a
        // string-of-digits...
        // btw, if the user pass some wrong characters (like a-z), the
        // xstrtol will return a value of zero
        else
            b = 0;              // undefined = 0, user deserves a
        // compile-time error becase v is empty,
        // but we forgive him :)
        // remember that, the source, except of upper-case, is also
        // trimmed
    } else {                    // there was no 'X' delimiter after the
        // 'height', so, bpp is undefined
        y = xstrtol(v);         // now the v points to a string-of-digits, 
                                // 
        // we can perform atoi()
        // (xstrtol()=atoi())
        b = 0;                  // bpp is undefined (value 0)
    }

    // setup the globals
    opt_pref_width = x;
    opt_pref_height = y;
    opt_pref_bpp = b;
}

/**
 * imports units
 */
void comp_preproc_import(const char *slist)
{
    const char *p;
    char *d;
    char buf[OS_PATHNAME_SIZE + 1];
    int uid;
    bc_lib_rec_t imlib;

    p = slist;

    SKIP_SPACES(p);

    while (is_alpha(*p)) {
        // get name
        d = buf;
        while (is_alnum(*p) || *p == '_') {
            *d++ = *p++;
        }
        *d = '\0';

        // import name
        strlower(buf);
        if ((uid = slib_get_module_id(buf)) != -1) {    // C module
            // store lib-record
            strcpy(imlib.lib, buf);
            imlib.id = uid;
            imlib.type = 0;     // C module

            slib_setup_comp(uid);
            dbt_write(comp_libtable, comp_libcount, &imlib, sizeof(bc_lib_rec_t));
            comp_libcount++;
        } else {                // SB unit
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
            strcpy(imlib.lib, buf);
            imlib.id = uid;
            imlib.type = 1;     // unit

            dbt_write(comp_libtable, comp_libcount, &imlib, sizeof(bc_lib_rec_t));
            comp_libcount++;

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
void comp_preproc_remove_line(char *s, int cmd_sep_allowed)
{
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
void comp_preproc_unit(char *name)
{
    char *p = name;
    char *d;

    d = comp_unit_name;
    SKIP_SPACES(p);
    if (!is_alpha(*p)) {
        sc_raise(MSG_INVALID_UNIT_NAME);
    }
    while (is_alpha(*p) || *p == '_') {
        *d++ = *p++;
    }
    *d = '\0';
    comp_unit_flag = 1;
    SKIP_SPACES(p);

    if (*p != '\n' && *p != ':') {
        sc_raise(MSG_UNIT_ALREADY_DEFINED);
    }
}

/**
 * PASS 1
 */
int comp_pass1(const char *section, const char *text)
{
    char *ps, *p, lc = 0;
    int i;
    char pname[SB_KEYWORD_SIZE + 1];
    char *code_line;
    char *new_text;
    int len_option, len_import, len_unit, len_unit_path, len_inc;
    int len_sub, len_func, len_def, len_end;

    code_line = tmp_alloc(SB_SOURCELINE_SIZE + 1);
    memset(comp_bc_sec, 0, SB_KEYWORD_SIZE + 1);
    if (section) {
        strncpy(comp_bc_sec, section, SB_KEYWORD_SIZE);
    } else {
        strncpy(comp_bc_sec, SYS_MAIN_SECTION_NAME, SB_KEYWORD_SIZE);
    }
    new_text = comp_format_text(text);

    /*
     *      second (we can change it to support preprocessor)
     *
     *      Check for:
     *      include (#inc:)
     *      units-dir (#unit-path:)
     *      IMPORT
     *      UDF and UDP declarations
     *      PREDEF OPTIONS
     */
    p = ps = new_text;
    comp_proc_level = 0;
    *comp_bc_proc = '\0';

    len_option = strlen(LCN_OPTION);
    len_import = strlen(LCN_IMPORT_WRS);
    len_unit = strlen(LCN_UNIT_WRS);
    len_unit_path = strlen(LCN_UNIT_PATH);
    len_inc = strlen(LCN_INC);

    len_sub = strlen(LCN_SUB_WRS);
    len_func = strlen(LCN_FUNC_WRS);
    len_def = strlen(LCN_DEF_WRS);
    len_end = strlen(LCN_END_WRS);

    while (*p) {
        // OPTION environment parameters
        if (strncmp(LCN_OPTION, p, len_option) == 0) {
            p += len_option;
            SKIP_SPACES(p);
            if (strncmp(LCN_PREDEF, p, strlen(LCN_PREDEF)) == 0) {
                p += strlen(LCN_PREDEF);
                SKIP_SPACES(p);
                if (strncmp(LCN_QUIET, p, strlen(LCN_QUIET)) == 0) {
                    opt_quiet = 1;
                } else if (strncmp(LCN_GRMODE, p, strlen(LCN_GRMODE)) == 0) {
                    p += strlen(LCN_GRMODE);
                    comp_preproc_grmode(p);
                    opt_graphics = 1;
                } else if (strncmp(LCN_TEXTMODE, p, strlen(LCN_TEXTMODE)) == 0) {
                    opt_graphics = 0;
                } else if (strncmp(LCN_CSTR, p, strlen(LCN_CSTR)) == 0) {
                    opt_cstr = 1;
                } else if (strncmp(LCN_COMMAND, p, strlen(LCN_COMMAND)) == 0) {
                    char *pe;
                    p += strlen(LCN_COMMAND);
                    SKIP_SPACES(p);
                    pe = p;
                    while (*pe != '\0' && *pe != '\n') {
                        pe++;
                    }
                    lc = *pe;
                    *pe = '\0';
                    if (strlen(p) < OPT_CMD_SZ) {
                        strcpy(opt_command, p);
                    } else {
                        memcpy(opt_command, p, OPT_CMD_SZ - 1);
                        opt_command[OPT_CMD_SZ - 1] = '\0';
                    }

                    *pe = lc;
                } else {
                    sc_raise(MSG_OPT_PREDEF_ERR, p);
                }
            }
        } else if (strncmp(LCN_IMPORT_WRS, p, len_import) == 0) {
            // IMPORT units
            comp_preproc_import(p + len_import);
            comp_preproc_remove_line(p, 1);
        } else if (strncmp(LCN_UNIT_WRS, p, len_unit) == 0) {
            // UNIT name
            if (comp_unit_flag) {
                sc_raise(MSG_MANY_UNIT_DECL);
            } else {
                comp_preproc_unit(p + len_unit);
            }
            comp_preproc_remove_line(p, 1);
        } else if (strncmp(LCN_UNIT_PATH, p, len_unit_path) == 0) {
            // UNIT-PATH name
#if defined(_UnixOS) || defined(_DOS) || defined(_Win32)
            char upath[SB_SOURCELINE_SIZE + 1], *up;
            char *ps;

            ps = p;
            p += len_unit_path;
            SKIP_SPACES(p);
            if (*p == '\"') {
                p++;
            }
            up = upath;
            while (*p != '\n' && *p != '\"') {
                *up++ = *p++;
            }
            *up = '\0';

            sprintf(comp_bc_temp, "SB_UNIT_PATH=%s", upath);
            putenv(strdup(comp_bc_temp));
            p = ps;
            comp_preproc_remove_line(p, 0);
#else // supported OSes
            comp_preproc_remove_line(p, 0);
#endif
        } else {
            // INCLUDE FILE
            // this is not a normal way but needs less memory
            if (strncmp(LCN_INC, p, len_inc) == 0) {
                char *crp = NULL;
                p += len_inc;
                if (*p == '\"') {
                    p++;

                    crp = p;
                    while (*crp != '\0' && *crp != '\"') {
                        crp++;
                    }
                    if (*crp == '\0') {
                        sc_raise(MSG_INC_MIS_DQ);
                        break;
                    }

                    (lc = *crp, *crp = '\0');
                } else {
                    crp = strchr(p, '\n');
                    *crp = '\0';
                    lc = '\n';
                }

                strcpy(code_line, p);
                *crp = lc;
                str_alltrim(code_line);
#if defined(_PalmOS)
                       {
#else
                if (!comp_bas_exist(code_line)) {
                    sc_raise(MSG_INC_FILE_DNE, comp_file_name, code_line);
                } else {
#endif
#if defined(_PalmOS)
                    char fileName[65];
                    char sec[64];
#else
                    char fileName[1024];
                    char sec[SB_KEYWORD_SIZE + 1];
#endif
                    strcpy(sec, comp_bc_sec);
                    strcpy(fileName, comp_file_name);
                    if (strchr(code_line, '.') == NULL) {
                        strcat(code_line, ".bas");
                    }
                    comp_load(code_line);
                    strcpy(comp_file_name, fileName);
                    strcpy(comp_bc_sec, sec);
                }
            } 
            if ((strncmp(LCN_SUB_WRS, p, len_sub) == 0) ||
                (strncmp(LCN_FUNC_WRS, p, len_func) == 0) ||
                (strncmp(LCN_DEF_WRS, p, len_def) == 0)) {
                // SUB/FUNC/DEF - Automatic declaration - BEGIN
                char *dp;
                int single_line_f = 0;
                
                if (strncmp(LCN_SUB_WRS, p, len_sub) == 0) {
                    p += len_sub;
                } else if (strncmp(LCN_FUNC_WRS, p, len_func) == 0) {
                    p += len_func;
                } else {
                    p += len_def;
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
                    char* dol = strrchr(comp_bc_proc, '/');
                    if (dol) {
                        *dol = '\0';
                    } else {
                        *comp_bc_proc = '\0';
                    }
                }
            } else if (comp_proc_level) {
                // SUB/FUNC/DEF - Automatic declaration - END
                if (strncmp(LCN_END_WRS, p, len_end) == 0 ||
                    strncmp(LCN_END_WNL, p, len_end) == 0) {
                    char* dol = strrchr(comp_bc_proc, '/');
                    if (dol) {
                        *dol = '\0';
                    } else {
                        *comp_bc_proc = '\0';
                    }
                    comp_proc_level--;
                }
            }
        } // OPTION

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
#if defined(_UnixOS)
        if (!isatty(STDOUT_FILENO)) {
            fprintf(stdout, "%s: %s\n", WORD_FILE, comp_file_name);
        } else {
            dev_printf("%s: \033[1m%s\033[0m\n", WORD_FILE, comp_file_name);
        }
#elif defined(_PalmOS)          // if (code-sections)
        dev_printf
            ("%s: \033[1m%s\033[0m\n\033[80m%s: \033[1m%s\033[0m\033[80m\n",
             WORD_FILE, comp_file_name, WORD_SECTION, comp_bc_sec);
#else
        dev_printf("%s: \033[1m%s\033[0m\n", WORD_FILE, comp_file_name);
#endif
    }

    // Start
    if (!comp_error) {
        comp_line = 0;
        if (!opt_quiet && !opt_interactive) {
#if defined(_UnixOS)
            if (!isatty(STDOUT_FILENO)) {
                fprintf(stdout, MSG_PASS1);
            } else {
#endif
                dev_printf(MSG_PASS1_COUNT, comp_line + 1);

#if defined(_UnixOS)
            }
#endif
        }
#if defined(_WinBCB)
        bcb_comp(1, comp_line + 1, 0);
#endif

        ps = p = new_text;
        while (*p) {
            if (*p == '\n') {
                // proceed
                *p = '\0';
                comp_line++;
                if (!opt_quiet && !opt_interactive) {
#if defined(_UnixOS)
                    if (isatty(STDOUT_FILENO)) {
#endif

#if defined(_PalmOS)
                        if ((comp_line % 16) == 0) {
                            if ((comp_line % 64) == 0)
                                dev_printf(MSG_PASS1_COUNT, comp_line);
                            if (dev_events(0) < 0) {
                                dev_print("\n\n\a*** interrupted ***\n");
                                comp_error = -1;
                            }
                        }
#else
                        if ((comp_line % 256) == 0) {
                            dev_printf(MSG_PASS1_COUNT, comp_line);
                        }
#endif

#if defined(_UnixOS)
                    }
#endif
                }
#if defined(_WinBCB)
                if ((comp_line % 256) == 0) {
                    bcb_comp(1, comp_line + 1, 0);
                }
#endif

                // add debug info: line-number
                bc_add_code(&comp_prog, kwTYPE_LINE);
                bc_add_addr(&comp_prog, comp_line);

                strcpy(code_line, ps);
                comp_text_line(code_line);
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

    tmp_free(code_line);
    tmp_free(new_text);

    // undefined keywords... by default are UDP, but if there is no
    // UDP-boddy then ring the bell
    if (!comp_error) {
        for (i = 0; i < comp_udpcount; i++) {
            if (comp_udptable[i].ip == INVALID_ADDR) {
                comp_line = comp_udptable[i].pline;
                sc_raise(MSG_UNDEFINED_UDP, comp_udptable[i].name);
            }
        }
    }

    bc_eoc(&comp_prog);
    bc_resize(&comp_prog, comp_prog.count);
    if (!comp_error) {
        if (!opt_quiet && !opt_interactive) {
            dev_printf(MSG_PASS1_FIN, comp_line + 1);
#if !defined(_PalmOS)
#if !defined(MALLOC_LIMITED)
            dev_printf("\rSB-MemMgr: Maximum use of memory: %dKB\n",
                       (memmgr_getmaxalloc() + 512) / 1024);
#endif
#endif
            dev_printf("\n");
        }
    }

    return (comp_error == 0);
}

/**
 * setup export table
 */
int comp_pass2_exports()
{
    int i, j;

    for (i = 0; i < comp_expcount; i++) {
        unit_sym_t sym;
        bid_t pid;

        dbt_read(comp_exptable, i, &sym, sizeof(unit_sym_t));

        // look on procedures/functions
        if ((pid = comp_udp_id(sym.symbol, 0)) != -1) {
            if (comp_udptable[pid].vid == INVALID_ADDR) {
                sym.type = stt_procedure;
            } else {
                sym.type = stt_function;
            }
            sym.address = comp_udptable[pid].ip;
            sym.vid = comp_udptable[pid].vid;
        } else {
            // look on variables 
            pid = -1;
            for (j = 0; j < comp_varcount; j++) {
                if (strcmp(comp_vartable[j].name, sym.symbol) == 0) {
                    pid = j;
                    break;
                }
            }

            if (pid != -1) {
                sym.type = stt_variable;
                sym.address = 0;
                sym.vid = j;
            } else {
                sc_raise(MSG_EXP_SYM_NOT_FOUND, sym.symbol);
                return 0;
            }
        }

        dbt_write(comp_exptable, i, &sym, sizeof(unit_sym_t));
    }

    return (comp_error == 0);
}

/**
 * setup user defined structures
 */
int comp_pass2_uds()
{
    comp_struct_t uds;

    // remember which comp_struct_t's have already been processed
    int* visited = tmp_alloc(sizeof(int)*comp_udscount);
    memset(visited, 0, sizeof(int)*comp_udscount);

    // hide the structure details from the executor
    bc_add_addr(&comp_prog, kwTYPE_EOC);
    addr_t struct_ip = comp_prog.count;
    bid_t curr_struct_id = -1;
    bid_t next_struct_id = -1;
    int n_visited = 0;
    int i = 0;
    int n_fields = 0;
    
    // create the structure lookup table. this can be used to find 
    // structure fields when only the var_id of a structure is known.
    // the lookup contains repeating elements of struct_id+struct_ptr
    bc_t bc_uds_tab;
    bc_create(&bc_uds_tab);

    while (n_visited < comp_udscount) {
        if (!visited[i]) {
            dbt_read(comp_udstable, i, &uds, sizeof(comp_struct_t));
            if (curr_struct_id == -1 || curr_struct_id == uds.base_id) {
                curr_struct_id = uds.base_id;
                next_struct_id = -1; // find next non-matching baseid
                visited[i] = 1;
                n_visited++;
                if (uds.is_container) {
                    // update all uds variables (foo) of the same id
                    // allowing the kwTYPE_UDS to find its fields
                    addr_t var_id, addr;
                    for (addr = comp_search_bc(0, kwTYPE_UDS);
                         addr != INVALID_ADDR;
                         addr = comp_search_bc(addr+1+ADDRSZ+ADDRSZ, kwTYPE_UDS)) {
                        memcpy(&var_id, comp_prog.ptr+addr+1, ADDRSZ);
                        if (var_id == uds.var_id) {
                            memcpy(comp_prog.ptr+addr+1+ADDRSZ, &struct_ip, ADDRSZ);                        
                        }
                    }
                } else {
                    // object member reference (foo.x)
                    if (n_fields == 0) {
                        bc_add_addr(&bc_uds_tab, curr_struct_id);
                        bc_add_addr(&bc_uds_tab, struct_ip);
                    }
                    bc_add_addr(&comp_prog, uds.field_id);
                    bc_add_addr(&comp_prog, uds.var_id);
                    n_fields++;
                }
            } else if (next_struct_id == -1) {
                next_struct_id = uds.base_id;
            }
        }

        if (++i == comp_udscount) {
            // no more structure fields
            bc_add_addr(&comp_prog, -1);

            // visit next structure
            struct_ip = comp_prog.count;
            curr_struct_id = next_struct_id;
            i = 0;
            n_fields = 0;
        }
    }
    bc_add_addr(&comp_prog, -1);
    comp_uds_tab_ip = comp_prog.count;
    bc_append(&comp_prog, &bc_uds_tab);
    bc_destroy(&bc_uds_tab);
    tmp_free(visited);
}

/*
 * PASS 2
 */
int comp_pass2()
{
    if (!opt_quiet && !opt_interactive) {
#if defined(_UnixOS)
        if (!isatty(STDOUT_FILENO)) {
            fprintf(stdout, "Pass2...\n");
        } else {
#endif
            dev_printf(MSG_PASS2);
#if defined(_UnixOS)
        }
#endif
    }

    if (comp_proc_level) {
        sc_raise(MSG_MISSING_END_3);
    } else {
        bc_add_code(&comp_prog, kwSTOP);
        comp_first_data_ip = comp_prog.count;
        comp_pass2_scan();
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
    if (comp_udscount > 0) {
        comp_pass2_uds();
    }
    return (comp_error == 0);
}

/*
 * final, create bytecode
 */
mem_t comp_create_bin()
{
    int i;
    mem_t buff_h;
    byte *buff, *cp;
    bc_head_t hdr;
    dword size;
    unit_file_t uft;
    unit_sym_t sym;


    if (!opt_quiet && !opt_interactive) {
        if (comp_unit_flag) {
            dev_printf(MSG_CREATING_UNIT, comp_unit_name);
        } else {
            dev_printf(MSG_CREATING_BC);
        }
    }
    // 
    memcpy(&hdr.sign, "SBEx", 4);
    hdr.ver = 2;
    hdr.sbver = SB_DWORD_VER;
#if defined(CPU_BIGENDIAN)
    hdr.flags = 1;
#else
    hdr.flags = 0;
#endif
#if defined(OS_ADDR16)
    hdr.flags |= 2;
#elif defined(OS_ADDR32)
    hdr.flags |= 4;
#endif

    // executable header
    hdr.bc_count = comp_prog.count;
    hdr.var_count = comp_varcount;
    hdr.lab_count = comp_labcount;
    hdr.data_ip = comp_first_data_ip;
    hdr.uds_tab_ip = comp_uds_tab_ip;

    hdr.size = sizeof(bc_head_t)
        + comp_prog.count + (comp_labcount * ADDRSZ)
        + sizeof(unit_sym_t) * comp_expcount
        + sizeof(bc_lib_rec_t) * comp_libcount
        + sizeof(bc_symbol_rec_t) * comp_impcount;

    if (comp_unit_flag)
        hdr.size += sizeof(unit_file_t);

    hdr.lib_count = comp_libcount;
    hdr.sym_count = comp_impcount;

    if (comp_unit_flag) {
        // it is a unit... add more info
        buff_h = mem_alloc(hdr.size + 4); // +4
        buff = mem_lock(buff_h);

        // unit header
        memcpy(&uft.sign, "SBUn", 4);
        uft.version = 1;
        strcpy(uft.base, comp_unit_name);
        uft.sym_count = comp_expcount;

        cp = buff;
        memcpy(cp, &uft, sizeof(unit_file_t));
        cp += sizeof(unit_file_t);

        // unit symbol table (export)
        for (i = 0; i < uft.sym_count; i++) {
            dbt_read(comp_exptable, i, &sym, sizeof(unit_sym_t));
            memcpy(cp, &sym, sizeof(unit_sym_t));
            cp += sizeof(unit_sym_t);
        }

        // normal file
        memcpy(cp, &hdr, sizeof(bc_head_t));
        cp += sizeof(bc_head_t);
    } else {
        // simple executable
        buff_h = mem_alloc(hdr.size + 4);       // +4
        buff = mem_lock(buff_h);

        cp = buff;
        memcpy(cp, &hdr, sizeof(bc_head_t));
        cp += sizeof(bc_head_t);
    }

    // append label table
    for (i = 0; i < comp_labcount; i++) {
        comp_label_t label;

        dbt_read(comp_labtable, i, &label, sizeof(comp_label_t));
        memcpy(cp, &label.ip, ADDRSZ);
        cp += ADDRSZ;
    }

    // append library table
    for (i = 0; i < comp_libcount; i++) {
        bc_lib_rec_t lib;

        dbt_read(comp_libtable, i, &lib, sizeof(bc_lib_rec_t));
        memcpy(cp, &lib, sizeof(bc_lib_rec_t));
        cp += sizeof(bc_lib_rec_t);
    }

    // append symbol table
    for (i = 0; i < comp_impcount; i++) {
        bc_symbol_rec_t sym;

        dbt_read(comp_imptable, i, &sym, sizeof(bc_symbol_rec_t));
        memcpy(cp, &sym, sizeof(bc_symbol_rec_t));
        cp += sizeof(bc_symbol_rec_t);
    }

    size = cp - buff;

    // the program itself
    memcpy(cp, comp_prog.ptr, comp_prog.count);
    mem_unlock(buff_h);

    size += comp_prog.count;

    // print statistics
    if (!opt_quiet && !opt_interactive) {
        dev_printf("\n");
        dev_printf(RES_NUMBER_OF_VARS, comp_varcount, comp_varcount - 18);
        // system variables
        dev_printf(RES_NUMBER_OF_LABS, comp_labcount);
        dev_printf(RES_NUMBER_OF_UDPS, comp_udpcount);
        dev_printf(RES_CODE_SIZE, comp_prog.count);
        dev_printf("\n");
        dev_printf(RES_IMPORTED_LIBS, comp_libcount);
        dev_printf(RES_IMPORTED_SYMS, comp_impcount);
        dev_printf(RES_EXPORTED_SYMS, comp_expcount);
        dev_printf("\n");
        dev_printf(RES_FINAL_SIZE, size);
        dev_printf("\n");
    }

    return buff_h;
}

/**
 * save binary
 *
 * @param h_bc is the memory-handle of the bytecode (created by create_bin)
 * @return non-zero on success
 */
int comp_save_bin(mem_t h_bc)
{
    int h;
    char fname[OS_FILENAME_SIZE + 1];
    char *buf;

    if ((opt_nosave && !comp_unit_flag) || opt_syntaxcheck) {
        return 1;
    }
    if (comp_unit_flag) {
        strcpy(fname, comp_unit_name);
        strlower(fname);

        if (!opt_quiet && !opt_interactive) {
#if defined(_Win32) || defined(_DOS)
            if (strncasecmp(fname, comp_file_name, strlen(fname)) != 0)
#else
            if (strncmp(fname, comp_file_name, strlen(fname)) != 0)
#endif
                dev_printf(MSG_UNIT_NAME_DIF_THAN_SRC);
        }

        strcat(fname, ".sbu");  // add ext
    } else {
        char *p;

        strcpy(fname, comp_file_name);
        p = strrchr(fname, '.');
        if (p) {
            *p = '\0';
        }
        strcat(fname, ".sbx");
    }

    h = open(fname, O_BINARY | O_RDWR | O_TRUNC | O_CREAT, 0660);
    if (h != -1) {
        buf = (char *)mem_lock(h_bc);
        write(h, buf, mem_handle_size(h_bc));
        close(h);
        mem_unlock(h_bc);
        if (!opt_quiet && !opt_interactive) {
            dev_printf(MSG_BC_FILE_CREATED, fname);
        }
    } else {
        panic(MSG_BC_FILE_ERROR);
    }

    return 1;
}

/**
 * compiler - main
 *
 * @param sb_file_name the source file-name
 * @return non-zero on success
 */
int comp_compile(const char *sb_file_name)
{
    char *source;
    int tid, prev_tid;
    int success = 0;
    mem_t h_bc = 0;

#if defined(_WinBCB)
    bcb_comp(0, 0, 0);
#endif

    tid = create_task(sb_file_name);
    prev_tid = activate_task(tid);

    comp_reset_externals();
    comp_init();                // initialize compiler

    source = comp_load(sb_file_name);   // load file and run pre-processor
    if (source) {
        success = comp_pass1(NULL, source);     // PASS1
        tmp_free(source);
        if (success) {
            success = comp_pass2();     // PASS2
        }
        if (success) {
            success = comp_check_labels();
        }
        if (success) {
            success = ((h_bc = comp_create_bin()) != 0);
        }
        if (success) {
            success = comp_save_bin(h_bc);
        }
    }

    comp_close();
    close_task(tid);
    activate_task(prev_tid);

    if (opt_nosave) {
        bytecode_h = h_bc;      // update task's bytecode
    } else if (h_bc) {
        mem_free(h_bc);
    }
#if defined(_WinBCB)
    bcb_comp(3, success, 0);
#endif

    return success;
}

/**
 * compiler - main.
 *
 * @param source buffer
 * @return non-zero on success
 */
int comp_compile_buffer(const char *source)
{
    comp_init();                // initialize compiler
    int success = comp_pass1(NULL, source);     // PASS1
    if (success) {
        success = comp_pass2(); // PASS2
    }
    if (success) {
        success = comp_check_labels();
    }
    if (success) {
        bytecode_h = comp_create_bin(); // update task's bytecode
    }
    comp_close();
    return (success && bytecode_h);
}
