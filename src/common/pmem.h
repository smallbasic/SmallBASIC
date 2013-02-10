// This file is part of SmallBASIC
//
// Memory manager unix/palm
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup mem memory manager
 */

#if !defined(_sb_mem_h)
#define _sb_mem_h

#if defined(MALLOC_LIMITED)
#include <string.h>
#endif

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(NULL)
#define NULL (void*)0L
#endif

/*
 * data-types
 */
typedef intptr_t MemHandle;
typedef MemHandle mem_t;

#if !defined(byte)
  #define byte unsigned char
#endif
typedef byte *byte_p_t;
typedef char *char_p_t;

// 16-bit integer
typedef int short int16;
typedef unsigned short word;

// 32-bit integer
typedef int int32;
typedef unsigned int dword;

// code
typedef byte code_t; /**< basic code unit      (unsigned)    @ingroup mem */

#if defined(OS_ADDR16)          // work with 16bits
#define ADDR16                  // just for short defs
typedef int16 fcode_t;          // buildin function code (signed)
typedef int16 pcode_t;          // buildin procedure code (signed)
typedef word addr_t;            // memory address (unsigned)
#define INVALID_ADDR    0xFFFF  // invalid address value (unsigned)
typedef int16 bid_t;            // IDs (labels, variables, etc) (signed)
#define OS_ADDRSZ   2           // size of address pointer (always 2
                                // for 16b mode)
#define OS_CODESZ   2           // size of buildin func/proc ptrs
                                // (always 2 for 16b mode)
#define OS_STRLEN   2           // size of strings
#else

typedef int32 fcode_t;
typedef int32 pcode_t;
typedef int32 bid_t;

#ifndef __CYGWIN__
typedef dword addr_t;
#endif

#define INVALID_ADDR    0xFFFFFFFF
#define OS_ADDRSZ   4   // size of address pointer (always 4 for 32b addresses)
#define OS_CODESZ   4   // size of buildin func/proc ptrs (always 4 for 32b mode)
#define OS_STRLEN   4   // size of strings
#endif

#define ADDRSZ      OS_ADDRSZ
#define CODESZ      OS_CODESZ
#define BC_CTRLSZ   (ADDRSZ+ADDRSZ)

#if !HAVE_MALLOC_USABLE_SIZE

/**
 * Allocate local memory
 */
void *tmp_alloc(dword size);

/**
 * Free allocated memory
 */
void tmp_free(void *ptr);

/**
 * @ingroup mem
 *
 * reallocate
 *
 * @param ptr a pointer to the block
 * @param size is the size of the new block
 * @return a pointer to newly allocated block
 */
void *tmp_realloc(void *ptr, dword size);

/**
 * @ingroup mem
 *
 * clones a string
 *
 * @param str the string
 * @return a pointer to newly allocated string
 */
char *tmp_strdup(const char *source);

/**
 * @ingroup mem
 *
 * allocate a memory block (use mem_alloc())
 *
 * @param size is the block's size
 * @return a handle to the block
 */
mem_t mem_alloc(dword size);

/**
 * @ingroup mem
 *
 * reallocates a memory block
 *
 * @param handle the block's handle
 * @param size is the size of the new block
 * @return a handle to the new block
 */
mem_t mem_realloc(mem_t handle, dword new_size);

/**
 * @ingroup mem
 *
 * frees a memory block
 *
 * @param handle the block's handle
 */
void mem_free(mem_t h);

/**
 * @ingroup mem
 *
 * returns the size of the memory bloc
 *
 * @param h the handle
 * @return the size of the memory bloc
 */
dword mem_handle_size(mem_t h);

#endif // HAVE_MALLOC_USABLE_SIZE

/**
 * @ingroup mem
 *
 *   @struct tmpmem_list_node_s
 *
 * all-purpose list using memory-pointer. node structure
 */
struct tmpmem_list_node_s {
  void *data;
  struct tmpmem_list_node_s *next;
};

typedef struct tmpmem_list_node_s tmpnode_t;

/**
 * @ingroup mem
 *
 *   @struct tmpmem_list_s
 *
 * all-purpose list using malloc().
 */
struct tmpmem_list_s {
  tmpnode_t *head, *tail;
  int count;
};

typedef struct tmpmem_list_s tmplist_t;

/**
 * @ingroup mem
 *
 * create an all-purpose temporary list.
 * temporary lists are using non-handle-related allocation routines.
 *
 * @param lst the list
 */
void tmplist_init(tmplist_t *lst);

/**
 * @ingroup mem
 *
 * destroys a temporary list.
 * also, frees all memory blocks that are stored in the list.
 *
 * @param lst the list
 */
void tmplist_clear(tmplist_t *lst);

/**
 * @ingroup mem
 *
 * adds an element to the list.
 * the memory block will not be copied, so, do not free it.
 *
 * @param lst the list
 * @param ptr the data pointer
 * @param size the size of the data
 */
tmpnode_t *tmplist_add(tmplist_t *lst, void *ptr, int size);

/**
 * @ingroup mem
 *
 * write to logfile
 *
 * @param buf is the string to write
 */
void lwrite(const char *buf);

/**
 * @ingroup mem
 *
 * write to logfile (printf-style)
 *
 * @param fmt is the format
 * @param ... the format's parameters
 */
void lprintf(const char *fmt, ...);

#if defined(__cplusplus)
}
#endif
#include "common/vmt.h"
#endif
