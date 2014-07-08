// This file is part of SmallBASIC
//
// SmallBASIC Unit (SB units) manager
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(__sb_units_h)
#define __sb_units_h

#include "common/sys.h"
#include "common/var.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define UID_UNIT_BIT    0x8000

/**
 * @ingroup exec
 *
 * @typedef sym_type_t
 * symbol type (export/import)
 */
typedef enum {
  stt_variable, stt_procedure, stt_function
} sym_type_t;

/**
 * @ingroup exec
 *
 * @typedef unit_sym_t
 * symbol structure
 */
typedef struct {
  char symbol[SB_KEYWORD_SIZE]; /**< symbol name */
  sym_type_t type; /**< type of symbol (function, procedure, variable) */
  addr_t address; /**< code address if proc/func */
  addr_t vid; /**< return Variable-ID if func; or Variable-ID if variable */
} unit_sym_t;

/**
 * @ingroup exec
 *
 * unit status
 */
typedef enum {
  unit_undefined, /**< unused record */
  unit_loaded, /**< unit is loaded */
  unit_nil
} unit_status_t;

/**
 * @ingroup exec
 * @typedef unit_file_t
 * unit: file header
 */
typedef struct {
  dword sign[4]; /**< Always "SBUn" */
  int version; /**< version of this structure, always 1 */
  char base[SB_KEYWORD_SIZE + 1]; /**< unit-base name */
  int  sym_count; /**< number of symbols */
} unit_file_t;

/**
 * @ingroup exec
 * @typedef unit_t
 * unit-memory structure
 */
typedef struct {
  unit_status_t status; /**< status of this record */

  char name[OS_FILENAME_SIZE + 1]; /**< unit/file name */
  unit_file_t hdr; /**< data from file */

  unit_sym_t *symbols; /**< table of symbols */
}unit_t;

/**
 * @ingroup exec
 *
 * initialization
 */
void unit_mgr_init();

/**
 * @ingroup exec
 *
 * close up
 */
void unit_mgr_close();

/**
 * @ingroup exec
 *
 * returns the full-pathname of unit
 *
 * @param name unit's name
 * @param file buffer to store the filename
 * @return non-zero on success
 */
int find_unit(const char *name, char *file);

/**
 * @ingroup exec
 *
 * open unit
 *
 * @param file is the filename
 * @return the unit handle or -1 on error
 */
int open_unit(const char *file);

/**
 * @ingroup exec
 *
 * closes a unit
 *
 * @param uid is the unit's handle
 * @return 0 on success
 */
int close_unit(int uid);

/**
 * @ingroup exec
 *
 * imports unit's names
 *
 * @param uid unit's handle
 * @return 0 on success
 */
int import_unit(int uid);

/**
 * @ingroup exec
 *
 * execute
 */
int unit_exec(int lib_id, int index, var_t *ret);

#if defined(__cplusplus)
}
#endif
#endif
