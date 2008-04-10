// $Id: sys.h,v 1.13 2007/07/13 23:06:43 zeeb90au Exp $
// -*- c-file-style: "java" -*-
// This file is part of SmallBASIC
//
// System information
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 *   @defgroup sys System/CPU
 */

#if !defined(_sb_sys_h)
#define _sb_sys_h

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__BORLANDC__)
#pragma warn -8012              // comparing signed & unsinged
#endif

#if defined(__CYGWIN__)
#define __addr_t_defined
typedef unsigned int addr_t;

// there is conflict on cygwin's headers sys/types.h
// they reserve the 'addr_t' (sys/types.h:typedef char * addr_t)
// I did a grep -r -w at /usr/include to find if it is used somewhere else
// and it didn't.
//
// So, comment out that line in /usr/include/sys/types.h, at least for now...
//
// Another conflict is for V_ARRAY which is used on windows OLE subsystem

//#include <w32api/windows.h>
//#undef V_ARRAY
#endif

/**
 *   @ingroup sys
 *   @page sbsys System macros (SB/OS/CPU) and Memory manager
 *
 *  <b>CPU's FLAGS</b>
 *
 *   CPU_BIGENDIAN       The words are stored reversed; first the low-byte followed by high-byte (Intel x86)
 *
 *   CPU_LITTLEENDIAN    The words are stored normal; first the high-byte followed by low-byte (Motorola 68K)
 *
 *   CPU_CODESEG64K      64KB code size limit
 *
 *   CPU_REG16           16bit registers (64KB code|data segments)
 *
 *   <b>OS FLAGS</b>
 *
 *   OS_PATHNAME_SIZE    Maximum full-path name size (DOS=64,Unix/Win32=1024)
 *
 *   OS_FILENAME_SIZE    Maximum filename size (DOS=12,Unix/Win32=256)
 *
 *   OS_NAME             OS name!
 *
 *   OS_DIRSEP           OS directory separator (unix=/, win=\\)
 *
 *   OS_LIMITED          Use few resources.
 *                       SB runs on handhelds but also runs on linux.
 *                       On linux to create a static array of 256 elements (like the one for the file-handles) is a joke,
 *                       but on handhelds that is a real pain.
 *                       Also, there are speed optimized code that uses a lot of memory for favour of speed.
 *                       Use this macro for handhelds to reduce the use of memory.
 *
 *   OS_FILEHANDLES      Number of file handles to use.
 *                       Because there is a static array with information about each handle,
 *                       use small numbers on limited systems (a value of 16 is good enough).
 *
 *   OS_ADDR32           Use 32bit code, in anycase
 *
 *   OS_ADDR16           Use 16bit code, in anycase
 *
 *   <b>Special flags</b>
 *
 *   MALLOC_LIMITED      Use special SB's memory manager for limited systems without memory handles.
 *                       This driver must be used on final releases for desktops because it is speeds up
 *                       the SB about 4-8 times, but does not protect the system from memory-leaks or
 *                       other memory-related problems.
 *
 *   <b>Memory managers</b>
 *
 *   At this time, there are three memory manager on SB.
 *
 *   a) Standard C (mem.c, unx_memmgr.c)
 *
 *   This is the default memory manager.
 *   This manager can protect the system from memory problems
 *   and can display a lot of debug information.
 *   Especially, if the CHECK_PTRS_LEV2 macro is defined,
 *   it can checks almost all the pointers in almost all memory/string related routines.
 *
 *   This manager do a real good work but it is slow.
 *   Especially, if the CHECK_PTRS_LEV2 macro is defined, it is *extremly* slow.
 *
 *   b) Standard C limited (mem.c/MALLOC_LIMITED)
 *
 *   This manager is *extremly* fast. Also, it uses an fast emulation of memory handles.
 *   It puts all required information on the start of the memory block.
 *
 *   This manager it does not protect you from memory-leaks or pointer overuns.
 *
 *   c) PalmOS (mem.c)
 *
 *   The typical PalmOS memory manager. It is slow enough but it is not needs much memory.
 *   Do not use its string-related routines, are slow too (use glib's). Also, it has a
 *   small protection mechanism.
 *
 */

#define OS_PREC32
//#define OS_PREC64

// default
#define OS_ADDR32

//#define MALLOC_LIMITED

#if defined(_PalmOS) || defined(_ArmPalmOS)
/* ------------------------------------------------------------------------------------ */

/*
 *   PalmOS
 */

#define CPU_LITTLEENDIAN

#define OS_PATHNAME_SIZE    64
#define OS_FILENAME_SIZE    64
#define OS_FILEHANDLES      16

#define OS_DIRSEP   '/'

#define OS_LIMITED

#if defined(_ArmPalmOS)
#define OS_NAME     "PalmOS/ARM"
#undef  CPU_CODESEG64K
#else
#undef  OS_ADDR32
#define OS_ADDR16
#define OS_NAME     "PalmOS/68k"
#define CPU_CODESEG64K
//      #define SMART_LINKER    // multilink
#endif

// Creator ID
#define ID_SmBa     0x536D4261

// Type IDs
#define ID_DATA     0x44415441  // SB's internal
#define ID_UFST     0x55465354  // User's file ID

#include <PalmOS.h>
#include <PalmCompatibility.h>
#if defined(SONY_CLIE)
#include "sony_sdk_patch.h"
#endif
#include <string.h>
#include <stdarg.h>
#include "mathlib.h"
#include "syslib.h"

#elif defined(_VTOS)
/* ------------------------------------------------------------------------------------ */

/*
 *   Helio: Cygnus-based GCC and MY_STDLIB interface libraries required
 */

#define CPU_BIGENDIAN
//  #define CPU_CODESEG64K

#undef  OS_ADDR32
#define OS_ADDR16

#define OS_PATHNAME_SIZE    256
#define OS_FILENAME_SIZE    32
#define OS_FILEHANDLES      16

#define OS_NAME     "VTOS"
#define OS_VER          0x010300
#define OS_DIRSEP   '/'

//  #define OS_LIMITED

#include <system.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "my_stdio.h"
extern void MessageBox(char *title, char *msg, int fatal);

#elif defined(_FRANKLIN_EBM)

#include "ebm.h"
#define HAVE_C_MALLOC

#elif defined(_DOS)
/* ------------------------------------------------------------------------------------ */

/*
 *   DOS/DJGPP
 */
#define CPU_BIGENDIAN

// use long-filenames for DOS (its works on DJGPP; but only under windows)
#define _DOS_LFN
#define MALLOC_LIMITED

#if defined(_DOS_LFN)
#define OS_PATHNAME_SIZE    1024
#define OS_FILENAME_SIZE    256
#define OS_FILEHANDLES      256
#else
#define OS_PATHNAME_SIZE    256
#define OS_FILENAME_SIZE    12
#define OS_FILEHANDLES      20
#endif

#define OS_NAME     "DOS32/DJGPP"
//  #define OS_DIRSEP   '\\'
#define OS_DIRSEP   '/'         // djgpp uses /

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <math.h>
#include <time.h>
#include <io.h>
#include <unistd.h>             // djgpp only
#include <sys/stat.h>
#include <fcntl.h>
#include <dos.h>
#include <dirent.h>
#elif defined(_AMIDOS)
/* ------------------------------------------------------------------------------------ */

/*
 *   AMIGA
 */
#define CPU_LITTLEENDIAN
#define MALLOC_LIMITED

#define OS_PATHNAME_SIZE    1024
#define OS_FILENAME_SIZE    256
#define OS_FILEHANDLES      256

#define OS_NAME     "AMIDOS"
#define OS_DIRSEP   '/'

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <math.h>
#include <time.h>
#include <io.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#else
/* ------------------------------------------------------------------------------------ */

/*
 *   Linux/Win32
 */

#if defined(__MINGW32__)
#define malloc_usable_size _msize
#endif

#include <string.h>

// TODO: check other CPU's
#define CPU_BIGENDIAN

#ifdef HAVE_C_MALLOC
#define tmp_alloc(s) malloc(s)
#define tmp_realloc(ptr, size) realloc(ptr, size)
#define tmp_free(p)  free(p)
#define mem_alloc(p) (mem_t)malloc(p)
#define mem_realloc(ptr, size) (mem_t)realloc((void*)ptr, size)
#define mem_free(h)  free((void*)h)
#define tmp_strdup(str) strdup(str)
#define mem_lock(h) (void*)(h)
#define mem_unlock(h)
#define mem_handle_size(p) malloc_usable_size((void*)p)
#define MemHandleSize(p) malloc_usable_size((void*)p)
#define MemPtrSize(p) malloc_usable_size(p)
#define MemHandleSizeX(p,f,l) malloc_usable_size(p)
#define memmgr_setabort(v) (v)
#define memmgr_getmaxalloc(v) (0)
#define MemPtrNew(x) malloc(x)
#define MemPtrFree(x) free(x)

#elif !defined(UNIX_MEMMGR)
#define MALLOC_LIMITED
#endif

#define OS_PATHNAME_SIZE    1024
#define OS_FILENAME_SIZE    256
#define OS_FILEHANDLES      256

#if defined(_UnixOS)
#define OS_NAME     "Unix"
#define OS_DIRSEP   '/'
#elif defined(_Win32)
#define OS_NAME     "Win32"
//      #define OS_DIRSEP   '\\'
#if defined(_WinBCB)
#define OS_DIRSEP   '\\'
#else
#define OS_DIRSEP   '/'
#endif
#else
#define OS_NAME     "Unknown"
#define OS_DIRSEP   '/'
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <math.h>
#include <time.h>
#include <utime.h>
#if defined(_UnixOS)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <assert.h>
#include <malloc.h>

#if defined(__BORLANDC__)
#include <dir.h>
#define F_OK    0
#define X_OK    1
#define W_OK    2
#define R_OK    4
#endif

#endif
#if defined(_BCB_W32_IDE)
#include "win32/bcb.h"
#endif

//#if defined(CPU_REG16) && !defined(CPU_CODESEG64K)
//  #define CPU_CODESEG64K
//#endif

#if defined(CPU_CODESEG64K) && !defined(SMART_LINKER)
#define SEC(x)  __attribute__((section(#x)))
#else
#define SEC(x)
#endif

#if defined(CPU_BIGENDIAN)      // __i386__
#define BS16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define BS32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
#else
#define BS16(x) (x)
#define BS32(x) (x)
#endif

/*
 *   SB's constants
 */

#define SB_DWORD_VER    0x908   // 00 (major) 08 (minor) 03 (patch)

#if defined(VERSION)
#define SB_STR_VER VERSION
#else
#define SB_STR_VER      "0.9.8"
#endif

#if defined(OS_LIMITED)
#define SB_PANICMSG_SIZE    255
#else
#define SB_PANICMSG_SIZE    1023
#endif

#if defined(OS_LIMITED)
#define SB_ERRMSG_SIZE  256
#else
#define SB_ERRMSG_SIZE  2048
#endif

#if defined(OS_LIMITED)
#define SB_KEYWORD_SIZE     64
#else
#define SB_KEYWORD_SIZE     128
#endif

#if defined(OS_LIMITED)
#define SB_SOURCELINE_SIZE  511 // compiler - max. text line
#define SB_TEXTLINE_SIZE    255 // RTL
#else
#define SB_SOURCELINE_SIZE  65536 // compiler
#define SB_TEXTLINE_SIZE    8192  // RTL
#endif

#define SB_EXEC_STACK_SIZE  256 // executor's stack size

#define SB_PI   3.14159265358979323846

// STD MACROS
#define ABS(x)          ( ((x) < 0) ? -(x) : (x) )                                      /**< absolute value             @ingroup sys */
#define SGN(a)          ( ((a)<0)? -1 : 1 )                                             /**< sign                       @ingroup sys */
#define ZSGN(a)         ( ((a)<0)? -1 : (((a)>0)? 1 : 0)  )                             /**< sign which returns 0 for 0 @ingroup sys */
#define SWAP(a,b,c)     ( (c) = (a), (a) = (b), (b) = (c) )                             /**< swap values                @ingroup sys */
#define FLOOR(a)        ((a)>0 ? (int)(a) : -(int)(-a))                                 /**< floor                      @ingroup sys */
#define CEILING(a)      ((a)==(int)(a) ? (a) : (a)>0 ? 1+(int)(a) : -(1+(int)(-a)))     /**< ceil                       @ingroup sys */
#define ROUND(a)        ((a)>0 ? (int)(a+0.5) : -(int)(0.5-a))                          /**< round                      @ingroup sys */
#define ISWAP(a,b)      ( a^=b, b^=a, a^=b )                                            /**< integer swap               @ingroup sys */
#define I2MIN(a,b)      ( ((a) < (b)) ? (a) : (b) )                                     /**< min                        @ingroup sys */
#define I2MAX(a,b)      ( ((a) > (b)) ? (a) : (b) )                                     /**< max                        @ingroup sys */
/* clamp the input to the specified range */
#define CLAMP(v,l,h)    ((v)<(l) ? (l) : (v) > (h) ? (h) : v)                           /**< range check                @ingroup sys */

//#define ENABLE_VMM
#if !defined(_PalmOS) && !defined(HAVE_C_MALLOC)
#include "unx_memmgr.h"         // on MALLOC_LIMITED it is has empty routines
#endif
#include "pmem.h"
#include "panic.h"
#include "str.h"

#if !defined(_PalmOS)
// global command-line args
extern char **g_argv;                                                                 /**< global pointer to **argv   @ingroup sys */
extern int g_argc;                                                                    /**< global to argc             @ingroup sys */
#endif

// fix-up open/close/read/write command set
#if defined(_PalmOS) || defined(_VTOS)
#include "lopen_bridge.h"
#endif

#if !defined(O_BINARY)
#define O_BINARY    0
#endif

#if !defined(CLOCKS_PER_SEC) && defined(_Win32)
#define CLOCKS_PER_SEC  1000
#endif

#if defined(__cplusplus)
}
#endif
#endif
