// This file is part of SmallBASIC
//
// SmallBASIC, pseudo-compiler structures & globals
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup scan pseudo-compiler
 */

#if !defined(_sb_scan_h)
#define _sb_scan_h

#include "common/sys.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup scan
 * @struct keyword_s
 *
 * Generic keywords (basic bc-types & oldest code)
 */
struct keyword_s {
  const char name[16]; /**< keyword name */
  code_t code; /**< byte-code */
};

/**
 * @ingroup scan
 * @struct opr_keyword_s
 *
 * Operators (not the symbols)
 */
struct opr_keyword_s {
  const char name[16]; /**< keyword name */
  code_t code; /**< keyword code */
  code_t opr; /**< operator code */
};

/**
 * @ingroup scan
 * @struct spopr_keyword_s
 *
 * Special operators
 */
struct spopr_keyword_s {
  const char name[16]; /**< keyword name */
  code_t code; /**< keyword code */
};

/**
 * @ingroup scan
 * @struct func_keyword_s
 *
 * Buildin-functions
 */
struct func_keyword_s {
  const char name[16]; /**< keyword name */
  bid_t fcode; /**< keyword code */
};

/**
 * @ingroup scan
 * struct proc_keyword_s
 *
 * Buildin procedures
 */
struct proc_keyword_s {
  const char name[16]; /**< keyword name */
  bid_t pcode; /**< keyword code */
};

/**
 * @ingroup scan
 * @typedef ext_proc_node_t
 *
 * External procedure (Modules)
 */
typedef struct {
  char name[SB_KEYWORD_SIZE + 1]; /**< keyword name */
  int  lib_id; /**< library id */
  bid_t pcode; /**< keyword code */
  int symbol_index; /**< symbol index on symbol-table */
} ext_proc_node_t;

/**
 * @ingroup scan
 * @typedef ext_func_node_t
 *
 * External functions (Modules)
 */
typedef struct {
  char name[SB_KEYWORD_SIZE + 1]; /**< keyword name */
  int lib_id; /**< library id */
  bid_t fcode; /**< keyword code */
  int symbol_index; /**< symbol index on symbol-table */
} ext_func_node_t;

/**
 * @ingroup scan
 * @typedef comp_var_t
 *
 * compiler's variable node
 */
struct comp_var_s {
  char *name; /**< variable's name    @ingroup scan */
  byte dolar_sup; /**< used on system variables (so, COMMAND and COMMAND$ to be the same) @ingroup scan */
  int lib_id; /**< library id (if it is an external variable; otherwise -1) @ingroup scan */
  int symbol_index; /**< symbol index on symbol-table */
  int local_id;
  int local_proc_level;
};

typedef struct comp_var_s comp_var_t;

/**
 * @ingroup scan
 * @typedef comp_label_t
 *
 * compiler's label node
 */
struct comp_label_s {
  char name[SB_KEYWORD_SIZE + 1]; /**< label name    @ingroup scan */
  bcip_t ip; /**< address in BC @ingroup scan */
  byte level; /**< block level (used for GOTOs) @ingroup scan */
  bid_t block_id; /**< block_id (FOR-NEXT,IF-FI,etc) used for GOTOs @ingroup scan */
  bcip_t dp; /**< data pointer @ingroup scan */
};

typedef struct comp_label_s comp_label_t;

typedef struct {
  int count;
  int size;
  comp_label_t **elem;
} comp_label_table_t;

/**
 * @ingroup scan
 * @typedef comp_proc_t
 *
 * compiler's user-defined procedure/function node
 */
struct comp_proc_s {
  char *name; /**< procedure/function name  @ingroup scan */
  bcip_t ip; /**< address in BC            @ingroup scan */
  bid_t vid; /**< variable index (return variable-id; for functions) @ingroup scan */
  byte level; /**< block level (used for GOTOs) @ingroup scan */
  bid_t block_id; /**< block_id (FOR-NEXT,IF-FI,etc) used for GOTOs @ingroup scan */
  int pline; /**< source code line number      @ingroup scan */
};

typedef struct comp_proc_s comp_udp_t;

/*
 * @ingroup scan
 * @typedef comp_pass_node_t
 *
 * compiler's pass-2 stack node
 */
struct comp_pass_node_s {
  char sec[SB_KEYWORD_SIZE + 1]; /**< section-name (PalmOS) @ingroup scan */
  bcip_t pos; /**< address in BC         @ingroup scan */
  int line; /**< source code line number @ingroup scan */
  byte level; /**< block level             @ingroup scan */
  bid_t block_id; /**< block ID                @ingroup scan */
};

typedef struct comp_pass_node_s comp_pass_node_t;

typedef struct {
  int count;
  int size;
  comp_pass_node_t **elem;
} comp_pass_node_table_t;

#if !defined(SCAN_MODULE)       // actually static data
extern struct keyword_s keyword_table[]; /**< basic keywords             @ingroup scan */
extern struct opr_keyword_s opr_table[]; /**< operators table            @ingroup scan */
extern struct spopr_keyword_s spopr_table[]; /**< special operators table    @ingroup scan */
extern struct func_keyword_s func_table[]; /**< buildin functions table    @ingroup scan */
extern struct proc_keyword_s proc_table[]; /**< buildin procedures table   @ingroup scan */
#endif

/**
 * @ingroup scan
 *
 * clears all external-func/proc/var entries
 */
void comp_reset_externals(void);

/**
 * @ingroup scan
 *
 * adds a new external procedure to the compiler
 *
 * @param proc_name is the procedure name
 * @param lib_id is the ID of the library
 * @return the ID which will used in the compiler (and later on BC)
 */
int comp_add_external_proc(const char *proc_name, int lib_id);

/**
 * @ingroup scan
 *
 * adds a new external function to the compiler
 *
 * @param func_name is the function name
 * @param lib_id is the ID of the library
 * @return the ID which will used in the compiler (and later on BC)
 */
int comp_add_external_func(const char *func_name, int lib_id);

/**
 * @ingroup scan
 *
 * adds a new external variable to the compiler
 *
 * @param name is the variable name
 * @param lib_id is the ID of the library
 * @return the ID which will used in the compiler (and later on BC)
 */
int comp_add_external_var(const char *name, int lib_id);

/**
 * @ingroup scan
 *
 * returns true if the 'name' is a registered external procedure
 *
 * @param name is the procedure name
 * @return non-zero if found
 */
int comp_is_external_proc(const char *name);

/**
 * @ingroup scan
 *
 * returns true if the 'name' is a registered external function
 *
 * @param name is the function name
 * @return non-zero if found
 */
int comp_is_external_func(const char *name);

/**
 * @ingroup scan
 *
 * returns true if the 'name' is a base-level keyword (see kw)
 *
 * @param name is the keyword name
 * @return non-zero if found
 */
int comp_is_keyword(const char *name);

/**
 * @ingroup scan
 *
 * returns true if the 'name' is a build-in function
 *
 * @param name is the function name
 * @return non-zero if found
 */
bid_t comp_is_func(const char *name);

/**
 * @ingroup scan
 *
 * returns true if the 'name' is a build-in procedure
 *
 * @param name is the procedure name
 * @return non-zero if found
 */
bid_t comp_is_proc(const char *name);

/**
 * @ingroup scan
 *
 * returns true if the 'name' is a special operator
 *
 * @param name is the string
 * @return non-zero if found
 */
int comp_is_special_operator(const char *name);

/**
 * @ingroup scan
 *
 * returns true if the 'name' is an operator
 *
 * @param name is the string
 * @return non-zero if found
 */
int comp_is_operator(const char *name);

/**
 * @ingroup scan
 *
 * compiles a SB file... actually the only function that you'll need.
 *
 * related options: opt_nosave, opt_checksyntax
 *
 * @param sb_file_name the SB source file-name
 * @return non-zero on success
 */
int comp_compile(const char *sb_file_name);

/**
 * compiler - main
 *
 * @param source buffer
 * @return non-zero on success
 */
int comp_compile_buffer(const char *source);

/**
 * @ingroup scan
 *
 * loads a file for compile
 *
 * @param fileName the file
 * @return the text (newly allocated string)
 */
char *comp_load(const char *sb_file_name);

/**
 * @ingroup scan
 *
 * initialize compiler
 */
void comp_init();

/**
 * @ingroup scan
 *
 * clean-up compiler
 */
void comp_close();

/**
 * @ingroup scan
 *
 * compiles a program
 *
 * @param section is the section's name (use NULL for "main")
 * @param text is the text of the file
 */
int comp_pass1(const char *section, const char *text);

/**
 * @ingroup scan
 *
 * PASS-2
 */
int comp_pass2(void);

/**
 * @ingroup scan
 *
 * returns true if the SB-source file 'basfile' exists
 *
 * @param basfile the filename
 * @return non-zero if SB-source file 'basfile' exists
 */
int comp_bas_exist(const char *basfile);

/**
 * @ingroup scan
 *
 * setup prefered graphics mode (global variables)
 *
 * @param source is a string of form "WIDTHxHEIGHT[xBPP]"
 */
void comp_preproc_grmode(const char *source);

#if defined(__cplusplus)
}
#endif
#endif

