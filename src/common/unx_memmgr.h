// This file is part of SmallBASIC
//
// Unix & PalmOS memory manager for Unix.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_unix_memmgr_h)
#define _unix_memmgr_h

#if defined(_WinBCB)
#include <string.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(MALLOC_LIMITED) 
// so, now it can included in makefile (for
// debug versions)

#if !defined(_WinBCB)
typedef void *MemPtr;
#else
typedef char *MemPtr;
#endif

void memmgr_init(void);
void memmgr_setabort(int v);
void memmgr_close(void);
int memmgr_getalloc(void);
int memmgr_getmaxalloc();

MemHandle MemHandleNewX(int size, const char *file, int line);
void MemHandleFreeX(MemHandle h, const char *file, int line);
int MemHandleResizeX(MemHandle h, int new_size, const char *file, int line);
int MemHandleSizeX(MemHandle h, const char *file, int line);
MemPtr MemHandleLockX(MemHandle h, const char *file, int line);
void MemHandleUnlockX(MemHandle h, const char *file, int line);
MemPtr MemPtrNewX(int size, const char *file, int line);
void MemPtrFreeX(MemPtr p, const char *file, int line);
int MemPtrSizeX(const MemPtr p, const char *file, int line);
MemHandle MemPtrRecoverHandleX(MemPtr p, const char *file, int line);
void CheckPtr(const void *ptr, int inside);

void MemMoveX(void *dst, const void *src, int size, const char *file, int line);
void MemSetX(void *dst, int size, int val, const char *file, int line);
int MemCmpX(const void *p1, const void *p2, int n, const char *file, int line);

char *StrRevChrX(const char *, int c, const char *file, int line);
int StrLenX(const char *, const char *file, int line);
char *StrCopyX(char *dst, const char *src, const char *file, int line);
char *StrCatX(char *dst, const char *src, const char *file, int line);
int StrCompareX(const char *a, const char *b, const char *file, int line);
char *StrChrX(const char *src, int c, const char *file, int line);
int StrCaselessCompareX(const char *s1, const char *s2, const char *file, int line);
int StrNCaselessCompareSM(const char *s1, const char *s2, int n);
int StrNCaselessCompareX(const char *s1, const char *s2, int n, const char *file, int line);
char *StrNCatX(char *dst, const char *src, int n, const char *file, int line);
char *StrNCopyX(char *dst, const char *src, int n, const char *file, int line);
int StrNCompareX(const char *a, const char *b, int n, const char *file, int line);
char *StrStrX(const char *s1, const char *s2, const char *file, int line);
char *StrErrorX(int err, const char *file, int line);

#if !defined(UMM_MODULE)
/*
 * C Memory Manager
 */
#if defined(CHECK_PTRS_LEV2)
#define   malloc(n)   MemPtrNewX((n), __FILE__, __LINE__)
#define   free(p)     MemPtrFreeX((p), __FILE__, __LINE__)
#define   realloc(p,n)  MemPtrResizeX((p), (n), __FILE__, __LINE__)
#else
#include <stdlib.h>
#endif

/*
 * C String Library
 */
#if defined(CHECK_PTRS_LEV2)
#define   memcpy(a,b,c) MemMoveX((a),(b),(c), __FILE__, __LINE__)
#define   memset(a,b,c) MemSetX((a),(c),(b), __FILE__, __LINE__)
#define   strlen(a)   StrLenX((a), __FILE__, __LINE__)
#define   strcpy(a,b)   StrCopyX((a),(b), __FILE__, __LINE__)
#define   strcat(a,b)   StrCatX((a),(b), __FILE__, __LINE__)
#define   strcmp(a,b)   StrCompareX((a),(b), __FILE__, __LINE__)
#define   strncmp(a,b,c)  StrNCompareX((a),(b),(c), __FILE__, __LINE__)
#define   strchr(a,b)   StrChrX((a),(b), __FILE__, __LINE__)
#define   stricmp(a,b)  StrCaselessCompareX((a),(b), __FILE__, __LINE__)
#define   strncpy(a,b,c)  StrNCopyX((a),(b),(c), __FILE__, __LINE__)
#define   strrchr(a,b)  StrRevChrX((a),(b), __FILE__, __LINE__)
#define   strerror(a)   StrErrorX((a), __FILE__, __LINE__)
#define   bzero(a,n)    MemSetX((a),(n),0, __FILE__, __LINE__)
#define   strstr(a,b)   StrStrX((a),(b), __FILE__, __LINE__)
#else
#include <string.h>
#endif

/*
 * PalmOS Memory Manager
 */
#define   MemHandleNew(h)     MemHandleNewX((h), __FILE__, __LINE__)
#define   MemHandleFree(h)    MemHandleFreeX((h), __FILE__, __LINE__)
#define   MemHandleResize(h,n)  MemHandleResizeX((h), (n), __FILE__, __LINE__)
#define   MemHandleSize(h)    MemHandleSizeX((h), __FILE__, __LINE__)
#define   MemHandleLock(h)    MemHandleLockX((h), __FILE__, __LINE__)
#define   MemHandleUnlock(h)    MemHandleUnlockX((h), __FILE__, __LINE__)
#define   MemPtrNew(n)      MemPtrNewX((n), __FILE__, __LINE__)
#define   MemPtrFree(p)     MemPtrFreeX((p), __FILE__, __LINE__)
#define   MemPtrSize(p)     MemPtrSizeX((p), __FILE__, __LINE__)
#define   MemPtrRecoverHandle(p)  MemPtrRecoverHandleX((p), __FILE__, __LINE__)

/*
 * PalmOS String Library
 */
#if defined(CHECK_PTRS_LEV2)
#define   MemMove(a,b,c)    MemMoveX((a),(b),(c), __FILE__, __LINE__)
#define   MemSet(a,b,c)   MemSetX((a),(b),(c), __FILE__, __LINE__)
#define   StrLen(a)     StrLenX((a), __FILE__, __LINE__)
#define   StrCopy(a,b)    StrCopyX((a),(b), __FILE__, __LINE__)
#define   StrCat(a,b)     StrCatX((a),(b), __FILE__, __LINE__)
#define   StrCompare(a,b)   StrCompareX((a),(b), __FILE__, __LINE__)
#define   StrNCompare(a,b,c)  StrNCompareX((a),(b),(c), __FILE__, __LINE__)
#define   StrChr(a,b)     StrChrX((a),(b), __FILE__, __LINE__)
#define   StrCaselessCompare(a,b) StrCaselessCompareX((a),(b), __FILE__, __LINE__)
#define   StrNCaselessCompare(a,b,c)  StrNCaselessCompareX((a),(b),(c), __FILE__, __LINE__)
#define   StrNCopy(a,b,c)   StrNCopyX((a),(b),(c), __FILE__, __LINE__)
#define   StrStr(a,b)     StrStrX((a),(b), __FILE__, __LINE__)
#else
#define   StrNCaselessCompare(a,b,c)  StrNCaselessCompareSM((a),(b),(c))
#endif

#endif

#else                           // MALLOC_LIMITED
void memmgr_init(void);
void memmgr_setabort(int v);
void memmgr_close(void);
int memmgr_getalloc(void);
int memmgr_getmaxalloc();
#endif

#if defined(__cplusplus)
}
#endif
#endif
