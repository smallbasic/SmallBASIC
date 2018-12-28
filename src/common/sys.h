// This file is part of SmallBASIC
//
// System information
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

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

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#if !defined DBL_EPSILON
#define DBL_EPSILON 0.00000000000001
#endif
#define EPSILON DBL_EPSILON
#define OS_PREC64

#define VAR_MAX_INT     LONG_MAX
#define VAR_NUM_FMT     "%f"
#define VAR_INT_FMT     "%ld"
#define VAR_INT_NUM_FMT "%.0f"

#define OS_PATHNAME_SIZE    1024
#define OS_FILENAME_SIZE    256
#define OS_FILEHANDLES      256
#define OS_DIRSEP   '/'

#if defined(_Win32)
 #define SB_VERSYS "Win"
#else
 #define SB_VERSYS "Unix"
#endif

#if UINTPTR_MAX == 0xffffffff
  #define SB_BIT_SZ "_32 "
#else
  #define SB_BIT_SZ "_64 "
#endif

// SB's constants
#if defined(_SDL)
 #define SB_STR_VER VERSION " SDL " SB_VERSYS SB_BIT_SZ BUILD_DATE
#elif defined (_ANDROID)
  #define SB_STR_VER VERSION " Android " BUILD_DATE
#else
  #define SB_STR_VER VERSION " Console " SB_VERSYS SB_BIT_SZ BUILD_DATE
#endif
#define SB_ERRMSG_SIZE      2048
#define SB_KEYWORD_SIZE     128
#define SB_SOURCELINE_SIZE  65536 // compiler
#define SB_TEXTLINE_SIZE    8192  // RTL
#define SB_EXEC_STACK_SIZE  512   // executor's stack size
#define SB_EVAL_STACK_SIZE  16    // evaluation stack size
#define SB_KW_NONE_STR "Nil"

// STD MACROS
#define ABS(x)    ( ((x) < 0) ? -(x) : (x) )            // absolute value
#define SGN(a)    ( ((a)<0)? -1 : 1 )                   // sign
#define ZSGN(a)   ( ((a)<0)? -1 : (((a)>0)? 1 : 0)  )   // sign which returns 0 for 0
#define SWAP(a,b,c) ( (c) = (a), (a) = (b), (b) = (c) ) // swap values
#define FLOOR(a)    ((a)>0 ? (int)(a) : -(int)(-a))     // floor
#define CEILING(a)  ((a)==(int)(a) ? (a) : (a)>0 ? 1+(int)(a) : -(1+(int)(-a)))

#if !defined(NULL)
#define NULL (void*)0L
#endif

/*
 * data-types
 */
typedef char *char_p_t;
typedef uint8_t byte;
typedef uint8_t code_t;
typedef int32_t bid_t;
typedef uint32_t bcip_t;

#define INVALID_ADDR    0xFFFFFFFF
#define OS_ADDRSZ   4   // size of address pointer (always 4 for 32b addresses)
#define OS_CODESZ   4   // size of buildin func/proc ptrs (always 4 for 32b mode)
#define OS_STRLEN   4   // size of strings

#define ADDRSZ      OS_ADDRSZ
#define CODESZ      OS_CODESZ
#define BC_CTRLSZ   (ADDRSZ+ADDRSZ)

#include "include/var.h"
#include "common/str.h"

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
