// This file is part of SmallBASIC
//
// Virtual Dynamic Tables
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup mem memory manager
 */

/**
 * VMTs are an easy solution for PalmOS limited memory.
 * It is a system to support dynamic arrays on "disk".
 *
 * + Each VMT is a dynamic table of variable-length records
 * + Every VMT can be stored on "disk" or on memory
 * + If the a record does not exists then a new one will be created
 *
 * Details:
 *
 * There are two files for each table, the .dbt which holds the data and the .dbi which holds database info
 *
 * .dbi file structure
 * [vmt_t // vmt data structure]
 * [vmt_rec_t 0 // vmt record structure]
 * ...
 * [vmt_rec_t N // vmt record structure]
 *
 * Note: multiuser not supported yet, but it is easy to be added (lock/unlock the header record)
 */

#if !defined(_vmt_h)
#define _vmt_h

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @ingroup mem
 *
 * VMT structure
 */
typedef struct {
  dword sign;                 // signature: VMTH
  int ver;                    // version: 1
  int used;                   // true if this structure is used

  int flags;                  // open/create flags
  // bit 0 = true for file, false for memory
  // bit 1 = true for create always (truncate if it is exists);
  // false for open if exists or create if it does not exists
  // bit 2 = true for reset count variable
  int count;                  // number of records
  int size;                   // allocated size (in records)

  char base[OS_PATHNAME_SIZE];

  int f_handle;               // data file handle
  int i_handle;               // index file handle
  mem_t m_handle;             // memory handle (table)
  mem_t *m_table;             // optimization: locked memory handles table
} vmt_t;

/**
 * @ingroup mem
 *
 * VMT record structure (disk records only)
 */
typedef struct {
  int ver;                    // always 1
  int size;                   // size of record's data in bytes
  int offset;                 // offset of data in database (dbt)
} vmt_rec_t;

typedef int dbt_t;

typedef struct {
  dword sign;
  int node_len;
  int var_len;
  int val_len;
} dbt_var_t;
#define dbt_var_sign    0x56654e56  // 'VeNV'
/**
 * @ingroup mem
 *
 * create a virtual-table
 *
 * flags:
 * bit 0 = true for file, false for memory
 * bit 1 = true for create always (truncate if it is exists);
 *     false for open if exists or create if it does not exists
 * bit 2 = true for reset count variable
 *
 * @param filename is the filename
 * @param flags is the open-mode flags
 * @return a handle to the table
 */
dbt_t dbt_create(const char *fileName, int flags)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * closes a virtual-table
 *
 * @param f the table's handle
 */
void dbt_close(dbt_t f)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * writes a memory-block to virtual-table
 *
 * @param f the table's handle
 * @param index is the index of the element
 * @param ptr is the data pointer
 * @param size is the size of the data
 */
void dbt_write(dbt_t f, int index, void *ptr, int size)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * reads a memory-block from a virtual-table
 *
 * @param f the table's handle
 * @param index is the index of the element
 * @param ptr is the data pointer
 * @param size is the size of the data
 */
void dbt_read(dbt_t f, int index, void *ptr, int size)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * returns the size of the record
 *
 * @param f the table's handle
 * @param index is the index of the element
 * @return the size of the record
 */
int dbt_recsize(dbt_t f, int index)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * pre-allocate 'num' elements of 'size' size.
 * this routine is used for speed optimization.
 *
 * @param f the table's handle
 * @param num is the number of the elements
 * @param size is the size per element
 */
void dbt_prealloc(dbt_t f, int num, int size)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * Returns the number of records in the table
 *
 * @param f the table's handle
 * @return the number of records in VMT
 */
int dbt_count(dbt_t f)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * Removes a node from the table.
 * The rest records will shifted up.
 * @param f the table's handle
 * @param index is the index of the element
 */
void dbt_remove(dbt_t f, int index)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * Using VMT's like environment-variables
 *
 * Sets a variable
 * if varvalue == NULL, the variable will be deleted
 *
 * @param f the table's handle
 * @param varname the variable's name
 * @param varvalue the variable's value
 * @return always 0
 */
int dbt_setvar(dbt_t f, const char *varname, const char *varvalue)
SEC(PALMFS);

/**
 * @ingroup mem
 *
 * Using VMT's like environment-variables
 *
 * Gets variable's value
 *
 * @param f the table's handle
 * @param varname the variable's name
 * @return if variable not found, returns NULL otherwise returns a newly allocated string with the value
 */
char *dbt_getvar(dbt_t f, const char *varname)
SEC(PALMFS);

#if defined(__cplusplus)
}
#endif
#endif
