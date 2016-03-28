// This file is part of SmallBASIC
//
// COMPILER/EXECUTOR/IDE COMMON DEFS
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_smbas_h)
#define _smbas_h

#include "common/sys.h"
#include "common/pmem.h"
#include "common/var.h"
#include "common/kw.h"
#include "common/scan.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup exec
 *
 * @typedef bc_head_t
 * byte-code header
 */
typedef struct {
  char sign[4]; /**< always "SBEx" */
  byte unused;
  word ver; /**< version of this structure */
  dword sbver; /**< version of SB */
  dword flags; /**< flags
   b0 = Big-endian CPU
   b1 = BC 16bit
   b2 = BC 32bit */

  dword size; /**< total size (include label-table and bc) */
  dword bc_count; /**< BC length */
  dword var_count; /**< number of variables */
  dword lab_count; /**< number of labels */
  dword data_ip; /**< default DATA position */
  dword uds_tab_ip; /**< location of struct mapping table */

  // ver 2
  word lib_count; /**< libraries count (needed units) */
  dword sym_count; /**< symbol count (linked-symbols) */
  char reserved[26];
} bc_head_t;

/**
 * @ingroup exec
 *
 * @typedef bc_unit_rec_t
 * byte-code linked-unit record
 */
typedef struct {
  char lib[OS_FILENAME_SIZE + 1]; /**< library name */
  int  type; /**< library type (unit, c-module) */
  int id; /**< lib-id in this byte-code */
  int tid; /**< task id (updated on loading) */
} bc_lib_rec_t;

typedef struct {
  int count;
  bc_lib_rec_t **elem;
} bc_lib_rec_table_t;

/**
 * @ingroup exec
 *
 * @typedef bc_symbol_rec_t
 * byte-code linked-symbol record
 */
typedef struct {
  char symbol[SB_KEYWORD_SIZE + 1]; /**< symbol name */
  int type; /**< symbol type */
  int lib_id; /**< lib-id in this byte-code */
  int sym_id; /**< symbol-id in this byte-code */
  int var_id; /**< related variable-id in this byte-code */
  int task_id; /**< task id which library is loaded (updated on loading) */
  int exp_idx; /**< export symbol-index in librarys space (updated on loading) */
} bc_symbol_rec_t;

typedef struct {
  int count;
  bc_symbol_rec_t **elem;
} bc_symbol_rec_table_t;

#define BRUN_RUNNING    0       /**< brun_status(), the program is still running  @ingroup exec */
#define BRUN_STOPPED    1       /**< brun_status(), an error or 'break' has already stoped the program @ingroup exec */

/*
 * compiler options
 */
#if defined(BRUN_MODULE)
#define EXTERN
#else
#define EXTERN  extern
#endif

#define OPT_CMD_SZ  1024
#define OPT_MOD_SZ  1024

EXTERN byte opt_graphics; /**< command-line option: start in graphics mode   */
EXTERN byte opt_quiet; /**< command-line option: quiet                       */
EXTERN int opt_retval; /**< return-value (ERRORLEVEL)                        */
EXTERN byte opt_decomp; /**< decompile                                       */
EXTERN byte opt_syntaxcheck; /**< syntax check only                          */
EXTERN char opt_command[OPT_CMD_SZ]; /**< command-line parameters (COMMAND$) */
EXTERN byte opt_usevmt; /**< using VMT on compilation by default             */
EXTERN int opt_base; /**< OPTION BASE x                                      */
EXTERN byte opt_uipos; /**< OPTION UICS {CHARS|PIXELS}                       */
EXTERN byte opt_loadmod; /**< load all modules                               */
EXTERN char opt_modlist[OPT_MOD_SZ]; /**< Modules list                       */
EXTERN int opt_verbose; /**< print some additional infos                     */
EXTERN int opt_ide; /**< 0=no IDE, 1=IDE is linked, 2=IDE is external exe)   */
EXTERN byte os_charset; /**< use charset encoding                            */
EXTERN int opt_pref_width; /**< prefered graphics mode width (0 = undefined) */
EXTERN int opt_pref_height; /**< prefered graphics mode height               */
EXTERN byte opt_pref_bpp; /**< prefered graphics mode bits-per-pixel )       */
EXTERN byte opt_nosave; /**< do not create .sbx files                        */
EXTERN byte opt_interactive; /**< interactive mode                           */
EXTERN byte opt_usepcre; /**< OPTION PREDEF PCRE                             */
EXTERN byte opt_file_permitted; /**< file system permission                  */
EXTERN byte opt_show_page; /**< SHOWPAGE graphics flush mode                 */
EXTERN byte opt_mute_audio; /**< whether to mute sounds                      */
EXTERN byte opt_antialias; /**< OPTION ANTIALIAS OFF                         */
EXTERN byte opt_trace_on; /**< initial value for the TRON command            */

#define IDE_NONE        0
#define IDE_INTERNAL    1
#define IDE_EXTERNAL    2

// globals
EXTERN int gsb_last_line; /**< source code line of the last error            */
EXTERN int gsb_last_error; /**< error code, 0 = no error,  < 0 = local messages (i.e. break), > 0 = error       */
EXTERN char gsb_last_file[OS_PATHNAME_SIZE + 1]; /**< source code file-name of the last error     */
EXTERN char gsb_bas_dir[OS_PATHNAME_SIZE + 1]; /**< source code home dir     */
EXTERN char gsb_last_errmsg[SB_ERRMSG_SIZE + 1]; /**< last error message     */

#include "common/units.h"
#include "common/tasks.h"

// emulation
#define prog_line           ctask->line
#define comp_line           ctask->line
#define prog_error          ctask->error
#define comp_error          ctask->error
#define prog_file           ctask->file
#define comp_file           prog_file
#define comp_errmsg         ctask->errmsg
#define prog_errmsg         ctask->errmsg
#define prog_length         ctask->sbe.exec.length
#define prog_ip             ctask->sbe.exec.ip
#define prog_source         ctask->sbe.exec.bytecode
#define prog_dp             ctask->sbe.exec.dp
#define data_org            ctask->sbe.exec.org
#define prog_stack          ctask->sbe.exec.stack
#define prog_stack_alloc    ctask->sbe.exec.stack_alloc
#define prog_sp             ctask->sbe.exec.sp
#define eval_stk            ctask->sbe.exec.eval_stk
#define eval_stk_size       ctask->sbe.exec.eval_stk_size
#define eval_sp             ctask->sbe.exec.eval_sp
#define prog_varcount       ctask->sbe.exec.varcount
#define prog_labcount       ctask->sbe.exec.labcount
#define prog_libcount       ctask->sbe.exec.libcount
#define prog_symcount       ctask->sbe.exec.symcount
#define prog_expcount       ctask->sbe.exec.expcount
#define prog_vartable       ctask->sbe.exec.vartable
#define prog_labtable       ctask->sbe.exec.labtable
#define prog_libtable       ctask->sbe.exec.libtable
#define prog_symtable       ctask->sbe.exec.symtable
#define prog_exptable       ctask->sbe.exec.exptable
#define prog_uds_tab_ip     ctask->sbe.exec.uds_tab_ip
#define prog_timer          ctask->sbe.exec.timer
#define comp_extfunctable   ctask->sbe.comp.extfunctable
#define comp_extfunccount   ctask->sbe.comp.extfunccount
#define comp_extfuncsize    ctask->sbe.comp.extfuncsize
#define comp_extproctable   ctask->sbe.comp.extproctable
#define comp_extproccount   ctask->sbe.comp.extproccount
#define comp_extprocsize    ctask->sbe.comp.extprocsize
#define comp_vartable       ctask->sbe.comp.vartable
#define comp_varcount       ctask->sbe.comp.varcount
#define comp_varsize        ctask->sbe.comp.varsize
#define comp_imptable       ctask->sbe.comp.imptable
#define comp_impcount       ctask->sbe.comp.imptable.count
#define comp_exptable       ctask->sbe.comp.exptable
#define comp_expcount       ctask->sbe.comp.exptable.count
#define comp_libtable       ctask->sbe.comp.libtable
#define comp_libcount       ctask->sbe.comp.libtable.count
#define comp_labtable       ctask->sbe.comp.labtable
#define comp_labcount       ctask->sbe.comp.labtable.count
#define comp_bc_sec         ctask->sbe.comp.bc_sec
#define comp_block_level    ctask->sbe.comp.block_level
#define comp_block_id       ctask->sbe.comp.block_id
#define comp_prog           ctask->sbe.comp.bc_prog
#define comp_data           ctask->sbe.comp.bc_data
#define comp_proc_level     ctask->sbe.comp.proc_level
#define comp_bc_proc        ctask->sbe.comp.bc_proc
#define comp_bc_temp        ctask->sbe.comp.bc_temp
#define comp_bc_tmp2        ctask->sbe.comp.bc_tmp2
#define comp_bc_name        ctask->sbe.comp.bc_name
#define comp_bc_parm        ctask->sbe.comp.bc_parm
#define comp_udptable       ctask->sbe.comp.udptable
#define comp_udpcount       ctask->sbe.comp.udpcount
#define comp_udpsize        ctask->sbe.comp.udpsize
#define comp_next_field_id  ctask->sbe.comp.next_field_id
#define comp_uds_tab_ip     ctask->sbe.comp.uds_tab_ip
#define comp_use_global_vartable    ctask->sbe.comp.use_global_vartable
#define comp_stack          ctask->sbe.comp.stack
#define comp_sp             ctask->sbe.comp.stack.count
#define comp_do_close_cmd   ctask->sbe.comp.do_close_cmd
#define comp_unit_flag      ctask->sbe.comp.unit_flag
#define comp_unit_name      ctask->sbe.comp.unit_name
#define comp_first_data_ip  ctask->sbe.comp.first_data_ip
#define comp_file_name      ctask->sbe.comp.file_name
#define tlab                prog_labtable
#define tvar                prog_vartable
#define eval_size           eval_stk_size
#define data_dp             prog_dp
#define brun_first_data_ip  data_org
#define prog_stack_sp       prog_sp
#define prog_stack_count    prog_stack_sp

#undef EXTERN

#include "common/hotspots.h"

/**
 * @ingroup exec
 *
 * create a 'break' - display message, too
 *
 * the 'break' will stops the program's execution
 */
void brun_break(void);

/**
 * @ingroup exec
 *
 * stops the program's execution
 */
void brun_stop(void);

/**
 * @ingroup exec
 *
 * returns the execution status (runing or stopped)
 *
 * @return BRUN_STOPPED or BRUN_RUNNING
 */
int brun_status(void);

/**
 * decompiler,
 * dumps the code in the current task
 *
 * @param output the output stream (FILE*)
 */
void dump_bytecode(FILE *output);

/**
 * returns the last-modified time of the file
 *
 * @param file the filename
 * @return the last-modified time of the file; on error returns 0L
 */
time_t sys_filetime(const char *file);

/**
 * search a set of directories for the given file
 * directories on path must be separated with symbol ':'
 *
 * @param path the path
 * @param file the file
 * @param retbuf a buffer to store the full-path-name file (can be NULL)
 * @return non-zero if found
 */
int sys_search_path(const char *path, const char *file, char *retbuf);

/**
 *   synchronize exported variables
 */
void exec_sync_variables(int dir);

#if defined(__cplusplus)
}
#endif
#endif
