// This file is part of SmallBASIC
//
// System information
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @ingroup sys
 * @page sbsys System macros (SB/OS/CPU) and Memory manager
 *
 * <b>CPU's FLAGS</b>
 *
 * CPU_BIGENDIAN       The words are stored reversed; first the low-byte followed by high-byte (Intel x86)
 *
 * CPU_LITTLEENDIAN    The words are stored normal; first the high-byte followed by low-byte (Motorola 68K)
 *
 * CPU_REG16           16bit registers (64KB code|data segments)
 *
 * <b>OS FLAGS</b>
 *
 * OS_PATHNAME_SIZE    Maximum full-path name size (DOS=64,Unix/Win32=1024)
 *
 * OS_FILENAME_SIZE    Maximum filename size (DOS=12,Unix/Win32=256)
 *
 * OS_NAME             OS name!
 *
 * OS_DIRSEP           OS directory separator (unix=/, win=\\)
 *
 * OS_FILEHANDLES      Number of file handles to use.
 *                     Because there is a static array with information about each handle,
 *                     use small numbers on limited systems (a value of 16 is good enough).
 *
 * OS_ADDR32           Use 32bit code, in anycase
 *
 * OS_ADDR16           Use 16bit code, in anycase
 *
 */

#if !defined(SB_SYS_H)
#define SB_SYS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(_Win32)
#include <windows.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__CYGWIN__)
typedef unsigned int addr_t;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <math.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#if defined(NONSTANDARD_PORT)
#  include <portdefs.h>
#endif

#if defined OS_PREC64
typedef long double var_num_t;
typedef long long int var_int_t;
#define VAR_INT_FMT     "%lld"
#define VAR_NUM_FMT     "%Lf"
#define VAR_INT_NUM_FMT "%.0Lf" // convert float to int
#else
#define OS_PREC32
typedef double var_num_t;
typedef long int var_int_t;
#define VAR_INT_FMT     "%ld"
#define VAR_NUM_FMT     "%f"
#define VAR_INT_NUM_FMT "%.0f"
#endif

#define OS_INTSZ  sizeof(var_int_t)  // size of integer
#define OS_REALSZ sizeof(var_num_t)  // size of real
#define OS_ADDR32

#define mem_lock(h) (void *)(h)
#define mem_unlock(h)

#if !HAVE_MALLOC_USABLE_SIZE && !defined(HAVE_MALLOC_USABLE_SIZE)

#define HAVE_MALLOC_USABLE_SIZE 1

#if defined(__MINGW32__)
#define malloc_usable_size _msize
#endif

// simplified versions of functions in mem.c
#define tmp_alloc(s) malloc(s)
#define tmp_realloc(ptr, size) realloc(ptr, size)
#define tmp_free(p)  free(p)
#define tmp_strdup(str) strdup(str)
#define mem_alloc(p) (mem_t)malloc(p)
#define mem_realloc(ptr, size) (mem_t)realloc((void *)ptr, size)
#define mem_free(h)  free((void *)h)
#define mem_handle_size(p) malloc_usable_size((void *)p)
#endif

#define OS_PATHNAME_SIZE    1024
#define OS_FILENAME_SIZE    256
#define OS_FILEHANDLES      256
#define OS_DIRSEP   '/'

// SB's constants
#define SB_STR_VER          VERSION
#define SB_DWORD_VER        0x908   // 00 (major) 08 (minor) 03 (patch)
#define SB_PANICMSG_SIZE    1023
#define SB_ERRMSG_SIZE      2048
#define SB_KEYWORD_SIZE     128
#define SB_SOURCELINE_SIZE  65536 // compiler
#define SB_TEXTLINE_SIZE    8192  // RTL
#define SB_EXEC_STACK_SIZE  256 // executor's stack size
#define SB_PI               3.14159265358979323846

// STD MACROS
#define ABS(x)    ( ((x) < 0) ? -(x) : (x) )            // absolute value
#define SGN(a)    ( ((a)<0)? -1 : 1 )                   // sign
#define ZSGN(a)   ( ((a)<0)? -1 : (((a)>0)? 1 : 0)  )   // sign which returns 0 for 0
#define SWAP(a,b,c) ( (c) = (a), (a) = (b), (b) = (c) ) // swap values
#define FLOOR(a)    ((a)>0 ? (int)(a) : -(int)(-a))     // floor
#define CEILING(a)  ((a)==(int)(a) ? (a) : (a)>0 ? 1+(int)(a) : -(1+(int)(-a)))
#define ROUND(a)    ((a)>0 ? (int)(a+0.5) : -(int)(0.5-a)) // round
#define ISWAP(a,b)  ( a^=b, b^=a, a^=b )                // integer swap
#define I2MIN(a,b)      ( ((a) < (b)) ? (a) : (b) )     // min
#define I2MAX(a,b)      ( ((a) > (b)) ? (a) : (b) )     // max
#define CLAMP(v,l,h)    ((v)<(l) ? (l) : (v) > (h) ? (h) : v) // clamp to specified range

#include "common/pmem.h"
#include "common/panic.h"
#include "common/str.h"

// global command-line args
extern char **g_argv; /**< global pointer to **argv   @ingroup sys */
extern int g_argc; /**< global to argc             @ingroup sys */

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

#if !defined(CMD_PAUSE_DELAY)
#define CMD_PAUSE_DELAY 50
#endif

#if defined(__cplusplus)
}
#endif
#endif
