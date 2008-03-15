/**
 * unx_memmgr.h
 *
 * Unix & PalmOS memory manager for Unix. This module is a bug killer.
 *
 * Nicholas Christopoulos
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 */

#if !defined(_unix_memmgr_h)
#define _unix_memmgr_h

#if defined(_WinBCB)
#include <string.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(MALLOC_LIMITED)    // so, now it can included in makefile (for
// debug versions)

typedef int MemHandle;
#if !defined(_WinBCB)
typedef void *MemPtr;
#else
typedef char *MemPtr;
#endif

void memmgr_init(void) SEC(MMGR);
void memmgr_setabort(int v) SEC(MMGR);
void memmgr_close(void) SEC(MMGR);
int memmgr_getalloc(void) SEC(MMGR);
int memmgr_getmaxalloc() SEC(MMGR);

MemHandle MemHandleNewX(int size, const char *file, int line) SEC(MMGR);
void MemHandleFreeX(MemHandle h, const char *file, int line) SEC(MMGR);
int MemHandleResizeX(MemHandle h, int new_size, const char *file,
                     int line) SEC(MMGR);
int MemHandleSizeX(MemHandle h, const char *file, int line) SEC(MMGR);
MemPtr MemHandleLockX(MemHandle h, const char *file, int line) SEC(MMGR);
void MemHandleUnlockX(MemHandle h, const char *file, int line) SEC(MMGR);
MemPtr MemPtrNewX(int size, const char *file, int line) SEC(MMGR);
void MemPtrFreeX(MemPtr p, const char *file, int line) SEC(MMGR);
int MemPtrSizeX(const MemPtr p, const char *file, int line) SEC(MMGR);
MemHandle MemPtrRecoverHandleX(MemPtr p, const char *file, int line) SEC(MMGR);
void CheckPtr(const void *ptr, int inside) SEC(MMGR);

void MemMoveX(void *dst, const void *src, int size, const char *file,
              int line) SEC(MMGR);
void MemSetX(void *dst, int size, int val, const char *file, int line) SEC(MMGR);
int MemCmpX(const void *p1, const void *p2, int n, const char *file,
            int line) SEC(MMGR);

char *StrRevChrX(const char *, int c, const char *file, int line) SEC(MMGR);
int StrLenX(const char *, const char *file, int line) SEC(MMGR);
char *StrCopyX(char *dst, const char *src, const char *file, int line) SEC(MMGR);
char *StrCatX(char *dst, const char *src, const char *file, int line) SEC(MMGR);
int StrCompareX(const char *a, const char *b, const char *file,
                int line) SEC(MMGR);
char *StrChrX(const char *src, int c, const char *file, int line) SEC(MMGR);
int StrCaselessCompareX(const char *s1, const char *s2, const char *file,
                        int line) SEC(MMGR);
int StrNCaselessCompareSM(const char *s1, const char *s2, int n) SEC(MMGR);
int StrNCaselessCompareX(const char *s1, const char *s2, int n, const char *file,
                         int line) SEC(MMGR);
char *StrNCatX(char *dst, const char *src, int n, const char *file,
               int line) SEC(MMGR);
char *StrNCopyX(char *dst, const char *src, int n, const char *file,
                int line) SEC(MMGR);
int StrNCompareX(const char *a, const char *b, int n, const char *file,
                 int line) SEC(MMGR);
char *StrStrX(const char *s1, const char *s2, const char *file,
              int line) SEC(MMGR);
char *StrErrorX(int err, const char *file, int line) SEC(MMGR);

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
void memmgr_init(void) SEC(MMGR);
void memmgr_setabort(int v) SEC(MMGR);
void memmgr_close(void) SEC(MMGR);
int memmgr_getalloc(void) SEC(MMGR);
int memmgr_getmaxalloc() SEC(MMGR);
#endif

#if defined(__cplusplus)
}
#endif
#endif
