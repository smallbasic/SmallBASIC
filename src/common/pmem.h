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

#if !defined(SB_MEM_H)
#define SB_MEM_H

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
typedef intptr_t mem_t;

#if !defined(byte)
  typedef unsigned char byte;
#endif

typedef byte *byte_p_t;
typedef char *char_p_t;

// 16-bit integer
typedef unsigned short word;

// 32-bit integer
typedef unsigned int dword;

// basic code unit
typedef byte code_t;

typedef int32_t fcode_t;
typedef int32_t pcode_t;
typedef int32_t bid_t;
typedef dword bcip_t;

#define INVALID_ADDR    0xFFFFFFFF
#define OS_ADDRSZ   4   // size of address pointer (always 4 for 32b addresses)
#define OS_CODESZ   4   // size of buildin func/proc ptrs (always 4 for 32b mode)
#define OS_STRLEN   4   // size of strings

#define ADDRSZ      OS_ADDRSZ
#define CODESZ      OS_CODESZ
#define BC_CTRLSZ   (ADDRSZ+ADDRSZ)

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

#endif
