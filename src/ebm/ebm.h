// -*- c-file-style: "java" -*-
// $Id: ebm.h,v 1.2 2004-04-12 00:21:41 zeeb90au Exp $
// This file is part of SmallBASIC

// Copyright(C) 2001-2003 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "franklin.h"
#include "ebm_object.h"

#define CPU_LITTLEENDIAN
#define OS_PATHNAME_SIZE    EBO_LEN_NAME+EBO_LEN_EXT
#define OS_FILENAME_SIZE    EBO_LEN_NAME
#define OS_FILEHANDLES      16

#define OS_NAME     "EBM"
#define OS_DIRSEP   '/'
#define OS_LIMITED
#define ENABLE_TMPLIST
//#define	SB_KEYWORD_SIZE		32

#undef	OS_ADDR32
#define	OS_ADDR16

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <l4/ipc.h>
#include <timer.h>
#include <gui_types.h>
#include <malloc.h>
#include <pen.h>
#include <sigma.h>

// in ebm_main.cpp
void dev_exit(); 

// in ebm_fs.cpp
int dev_access(const char *path, int amode);
int dev_stat(const char *path, struct stat *buf);
int dev_fclose(int SBHandle);
int dev_fopen(int SBHandle, const char *name, int flags);
int brun_fopen(const char *name, int flags);
void* dev_get_record(int SBHandle, int index);

#ifndef FS_MODULE
#define exit dev_exit
#define access dev_access
#define stat(p,s) dev_stat(p,s)
#define open(path,flags,mode) brun_fopen(path, flags)
#define read dev_fread
#define write dev_fwrite
#define close dev_fclose
#define lseek dev_lseek
#endif

#define SEC(x)

// replacement code for unx_memmgr.h

// this further defines mem_t
typedef	void* MemHandle;

#define tmp_alloc(s) malloc(s)
#define MemPtrNew(p) malloc(p)
#define mem_alloc(p) malloc(p)
#define tmp_realloc(ptr, size) realloc(ptr, size)
#define mem_realloc(ptr, size) realloc(ptr, size)
#define tmp_free(p)  free(p)
#define mem_free(h)  free(h)
#define MemPtrFree(p) free(p)
#define tmp_strdup(str) strdup(str)
#define mem_lock(h) (h)
#define mem_unlock(h)
#define MemHandleSize(p) malloc_usable_size(p)
#define mem_handle_size(p) malloc_usable_size(p)

#define StrNCaselessCompareSM(s1,s2,n) strnicmp(s1,s2,n)
#define StrNCaselessCompare(s1,s2,n) strnicmp(s1,s2,n)
int memmgr_getmaxalloc(void);

//#ifndef SCAN_MODULE
//#include "var.h"

// scan.c and var.h are not compatible

// forward declarations
//void dev_pushkey(word key);

//#undef SEC
//#endif

// stop emacs from getting confused
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }


