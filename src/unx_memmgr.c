/*
*	unx_memmgr
*
*	PalmOS memory manager emulation for unix
*
*	Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include "sys.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define	UMM_MODULE
#include "unx_memmgr.h"
#include "panic.h"
#if defined(_BCB_W32_IDE)
#include "win32/bcb.h"
#endif

#if	!defined(comp_line)
#define	comp_line	0             // TODO: fix it
#define	prog_line	0
#endif

#if !defined(MALLOC_LIMITED)    // so, now it can included in makefile (for
                                // debug versions)

#if defined(_Win32)
extern void dev_printf(const char *fmt, ...);
#define	xprintf		dev_printf
#else
#define	xprintf		printf
#endif
#if defined(_VTOS)
#define malloc qmalloc
#define free qfree
#endif

extern byte opt_quiet;

struct palm_mem_node {
  char module[OS_FILENAME_SIZE + 1];
  int line;                     /* code line */
  int size;                     /* memory block size, in bytes */
  int lock;                     /* lock count */
  int sbpline;                  /* prog_line */
  int sbsline;                  /* scan_line */
#if defined(_WinBCB)
  char *ptr;                    /* data pointer */
#else
  void *ptr;                    /* data pointer */
#endif
};
typedef struct palm_mem_node memnode;

#if !defined(OS_LIMITED)
#define	GROWSIZE		1024
#else
#define	GROWSIZE		256
#endif

#define	MAX_UNDEF_SIZE	1024

#define	MAX(a,b)	( ((a) > (b)) ? (a) : (b) )

static memnode *memtable;
static int umm_size;
static int umm_count;
static int max_alloc;           /* maximum size of memory allocation */
static int cur_alloc;           /* remain allocated (size) */

static int opt_abort = 0;

#if defined(CHECK_PTRS_LEV2)
static char err_module[256];
static int err_line;
#endif

/*
*/
int memmgr_getalloc()
{
  return cur_alloc;
}

/*
*/
int memmgr_getmaxalloc()
{
  return max_alloc;
}

/*
*/
void uuerr(int expf, const char *fmt, ...)
{
#if !defined(OS_LIMITED)
  va_list ap;
#if defined(_BCB_W32_IDE) || defined(_Win32)
  char buf[1024];
#endif

  if (expf) {
#if defined(CHECK_PTRS_LEV2)
    xprintf("\n\n=== SB-MemMgr: BUG located at %s:%d ===\n", err_module, err_line);
#endif

    va_start(ap, fmt);
#if defined(_BCB_W32_IDE) || defined(_Win32)
    vsprintf(buf, fmt, ap);
    xprintf("%s", buf);
#else
    vprintf(fmt, ap);
#endif
    va_end(ap);

#if !defined(_WinBCB) && !defined(_VTOS)
    exit(1);
#endif
  }
#else
#if defined(_VTOS)
  MessageBox("Memory Manager", "uuerr punted", TRUE);
#else
  exit(1);
#endif
#endif
}

/*
*/
void memmgr_setabort(int v)
{
  opt_abort = v;
}

/*
*	memory manager
*/
void memmgr_close()
{
  int i;

#if !defined(OS_LIMITED) || defined(_Win32)
  if (!opt_quiet) {
    xprintf("\n");
    if (cur_alloc == 0)
      xprintf("SB-MemMgr: Maximum use of memory: %dKB\n", (max_alloc + 512) / 1024);
    else
      xprintf("SB-MemMgr: Maximum use of memory: %.3f KB, Memory leak: %d bytes\n",
              max_alloc / 1024.0, cur_alloc);
  }
#endif
  if (cur_alloc) {
#if !defined(OS_LIMITED)
    if (!opt_abort && !opt_quiet)
      xprintf("-------------------------------------------------------\n");
#endif
    for (i = 0; i < umm_count; i++) {
      if (memtable[i].size != 0) {
#if !defined(OS_LIMITED)
        if (!opt_abort && !opt_quiet) {
          xprintf
            ("%s:%d --- Handle: %d, Size: %d, LockCount: %d (SB lines COMP=%d RT=%d)\n",
             memtable[i].module, memtable[i].line, i + 1, memtable[i].size,
             memtable[i].lock, memtable[i].sbsline, memtable[i].sbpline);
          hex_dump(memtable[i].ptr, memtable[i].size);
        }
#endif
        free(memtable[i].ptr);
      }
    }
  }

  free(memtable);

  max_alloc = 0;
  cur_alloc = 0;
  umm_count = 0;
  umm_size = 0;
  memtable = NULL;
}

void memmgr_init()
{
  max_alloc = 0;
  cur_alloc = 0;
  umm_count = 0;
  umm_size = GROWSIZE;
  memtable = (memnode *) malloc(sizeof(memnode) * umm_size);
#if !defined(_WinBCB) && !defined(_VTOS)
  atexit(memmgr_close);
#endif
}

/*
*	allocate a memory block of "storage" RAM
*/
MemHandle MemHandleNewX(int size, const char *file, int line)
{
  int i;

#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
#endif

  if (size <= 0) {
    uuerr(1, "\n\aSB-MemMgr: MemHandleNewX(h), size <= 0\n");
    return 0;
  }

  for (i = 0; i < umm_count; i++) {
    if (memtable[i].ptr == NULL) {
      strcpy(memtable[i].module, file);
      memtable[i].line = line;
      memtable[i].sbsline = comp_line;
      memtable[i].sbpline = prog_line;

      memtable[i].ptr = (void *)malloc(size);
      memtable[i].size = size;
      memtable[i].lock = 0;
      cur_alloc += size;
      if (cur_alloc > max_alloc)
        max_alloc = cur_alloc;
      return i + 1;
    }
  }

  if (umm_count >= umm_size) {
#if defined(_VTOS)
    // No REALLOC!
    void *ptr;
    ptr = qmalloc(sizeof(memnode) * (umm_size + GROWSIZE));
    if (!ptr)
      MessageBox("UNX_MEMMGR", "REALLOC FAILED!", TRUE);
    memcpy(ptr, memtable, sizeof(memnode) * umm_size);
    umm_size += GROWSIZE;
    qfree(memtable);
    memtable = ptr;
#else
    umm_size += GROWSIZE;
    memtable = (memnode *) realloc(memtable, sizeof(memnode) * umm_size);
#endif
  }

  i = umm_count;
  umm_count++;

  strcpy(memtable[i].module, file);
  memtable[i].line = line;
  memtable[i].sbsline = comp_line;
  memtable[i].sbpline = prog_line;

  memtable[i].ptr = (void *)malloc(size);
  memtable[i].size = size;
  memtable[i].lock = 0;
  cur_alloc += size;
  if (cur_alloc > max_alloc)
    max_alloc = cur_alloc;

  return i + 1;
}

/*
*	deallocate a storage-RAM memory block
*/
void MemHandleFreeX(MemHandle h, const char *file, int line)
{
  int i = h - 1;

#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
#endif

  if (i >= 0 && i < umm_count) {
    if (memtable[i].ptr != NULL) {
      if (memtable[i].lock)
        xprintf("SB-MemMgr: (%s:%d) mem is locked\n", file, line);

      cur_alloc -= memtable[i].size;
      free(memtable[i].ptr);
      memtable[i].ptr = NULL;
      memtable[i].size = 0;
      memtable[i].lock = 0;
    }
    else
      uuerr(1, "\n\aSB-MemMgr: MemHandleFree(h), handle is empty\n");
  }
  else
    uuerr(1, "\n\aSB-MemMgr: MemHandleFree(h), handle does not exists\n");
}

/*
*	returns the handle of pointer!
*/
MemHandle MemPtrRecoverHandleX(MemPtr p, const char *file, int line)
{
  int i;

#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!p, "\n\aSB-MemMgr: MemPtrRecoverHandle(p), pointer is NULL\n");
#endif
  for (i = 0; i < umm_count; i++) {
    if (memtable[i].ptr == p)
      return i + 1;
  }

  printf("\n%s:%d\n", file, line);
  uuerr(1, "\n\aSB-MemMgr: MemPtrRecoverHandle(ptr), ptr does not exists\n");
  return 0;
}

/*
*	returns the size of the memory block in bytes
*/
int MemHandleSizeX(MemHandle h, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  if ((h < 1) || (h > umm_count))
    uuerr(1, "\n\aSB-MemMgr: MemHandleSize(h), invalid handle\n");
  if (memtable[h - 1].ptr == NULL)
    uuerr(1, "\n\aSB-MemMgr: MemHandleSize(h), deleted block\n");
#endif
  return memtable[h - 1].size;
}

/*
*	lock a memory block (move it to dynamic-RAM)
*	returns the data-pointer of the memory block
*/
MemPtr MemHandleLockX(MemHandle h, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  if ((h < 1) || (h > umm_count))
    uuerr(1, "\n\aSB-MemMgr: MemHandleLock(h), invalid handle\n");
  if (memtable[h - 1].ptr == NULL)
    uuerr(1, "\n\aSB-MemMgr: MemHandleLock(h), deleted block\n");
#endif
  memtable[h - 1].lock++;
  return memtable[h - 1].ptr;
}

/*
*	unlocks a memory block (move it back to storage-RAM)
*/
void MemHandleUnlockX(MemHandle h, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  if ((h < 1) || (h > umm_count))
    uuerr(1, "\n\aSB-MemMgr: MemHandleUnlock(h), invalid handle\n");
  if (memtable[h - 1].ptr == NULL)
    uuerr(1, "\n\aSB-MemMgr: MemHandleUnlock(h), deleted block\n");
  if (memtable[h - 1].lock < 1)
    uuerr(1, "\n\aSB-MemMgr: MemHandleUnlock(h), lock count < 0\n");
#endif
  memtable[h - 1].lock--;
}

/*
*	resize a memory block (storage RAM)
*/
int MemHandleResizeX(MemHandle h, int new_size, const char *file, int line)
{
  char *np;

#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  if ((h < 1) || (h > umm_count))
    uuerr(1, "\n\aSB-MemMgr: MemHandleResize(h), invalid handle\n");
  if (memtable[h - 1].ptr == NULL)
    uuerr(1, "\n\aSB-MemMgr: MemHandleResize(h), deleted block\n");
  if (memtable[h - 1].lock)
    uuerr(1, "\n\aSB-MemMgr: MemHandleResize(): handle is locked\n");
  if (new_size <= 0)
    uuerr(1, "\n\aSB-MemMgr: MemHandleResize(): size <= 0\n");
#endif

  cur_alloc -= memtable[h - 1].size;

  np = (char *)malloc(new_size);
  memcpy(np, memtable[h - 1].ptr,
         (new_size < memtable[h - 1].size) ? new_size : memtable[h - 1].size);
  free(memtable[h - 1].ptr);
  memtable[h - 1].ptr = np;
  memtable[h - 1].size = new_size;

  cur_alloc += new_size;
  return 0;
}

/*
*	allocate a memory block of dynamic-RAM
*/
MemPtr MemPtrNewX(int size, const char *file, int line)
{
  MemHandle h;

#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr((size <= 0), "\nWarning: MemPtrNew(%d)\n", size);
#endif
  h = MemHandleNewX(size, file, line);
  return MemHandleLockX(h, file, line);
}

/*
*	free a memory block of dynamic-RAM
*/
void MemPtrFreeX(MemPtr p, const char *file, int line)
{
  int i;

#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!p, "\n\aSB-MemMgr: MemPtrFree(p), pointer is NULL\n");
#endif
  for (i = 0; i < umm_count; i++) {
    if (memtable[i].ptr == p) {
      MemHandleUnlockX(i + 1, file, line);
      MemHandleFreeX(i + 1, file, line);
      return;
    }
  }
  uuerr(1, "\n\aSB-MemMgr: MemPtrFree(ptr), ptr does not exists\n");
}

/*
*	Check pointer p
*/
int iCheckSize(const MemPtr p)
{
  int i;

  uuerr(!p, "\n\aSB-MemMgr: iCheckSize(p), pointer is NULL\n");
  for (i = 0; i < umm_count; i++) {
    if (memtable[i].ptr == p)
      return memtable[i].size;
    else if ((p >= memtable[i].ptr) && (p <= (memtable[i].ptr + memtable[i].size)))
      return memtable[i].size - (p - memtable[i].ptr);
  }
  return MAX_UNDEF_SIZE;        // its not allocated (constant ?)
}

/*
*	Check pointer p
*/
void CheckPtr(const void *p, int inside)
{
  int i;

  uuerr(!p, "\n\aSB-MemMgr: CheckPtr(p), pointer is NULL\n");
  for (i = 0; i < umm_count; i++) {
    if (memtable[i].ptr == p)
      return;
    else if (inside) {
      if ((p >= memtable[i].ptr) && (p <= (memtable[i].ptr + memtable[i].size)))
        return;
    }
  }
  uuerr(1, "\n\aSB-MemMgr: CheckPtr(p), pointer is out of allocated space\n");
}

/*
*	returns the size of the memory block (dynamic-RAM)
*/
int MemPtrSizeX(const MemPtr p, const char *file, int line)
{
  int i;

#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!p, "\n\aSB-MemMgr: MemPtrSize(p), pointer is NULL\n");
#endif
  for (i = 0; i < umm_count; i++) {
    if (memtable[i].ptr == p)
      return memtable[i].size;
  }
  xprintf("\n\aSB-MemMgr: MemPtrSize(ptr), ptr does not exists\n");
  return 0;
}

/* =================================================================================
*	mem
*/
void MemMoveX(void *dst, const void *src, int size, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!src, "\n\aSB-MemMgr: MemMove(dst,src), src is NULL\n");
  uuerr(!dst, "\n\aSB-MemMgr: MemMove(dst,src), dst is NULL\n");
  if (iCheckSize((MemPtr) dst) < size)
    uuerr(1, "\n\aSB-MemMgr: MemMove(dst,src), dst size error\n");
  if (iCheckSize((MemPtr) src) < size)
    uuerr(1, "\n\aSB-MemMgr: MemMove(dst,src), src size error\n");
#endif
  memcpy(dst, src, size);
}

void MemSetX(void *dst, int size, int val, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!dst, "\n\aSB-MemMgr: MemSet(dst), dst is NULL\n");
  if (iCheckSize(dst) < size)
    uuerr(1, "\n\aSB-MemMgr: MemSet(), dst size error\n");
#endif
  memset(dst, val, size);
}

int MemCmpX(const void *p1, const void *p2, int n, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!p1, "\n\aSB-MemMgr: MemCmp(p1,p2,n), p1 is NULL\n");
  uuerr(!p2, "\n\aSB-MemMgr: MemCmp(p1,p2,n), p2 is NULL\n");
  uuerr((n <= 0), "\n\aSB-MemMgr: MemCmp(p1,p2,n), n <= 0\n");
#endif
  return memcmp(p1, p2, n);
}

/* ======================================================================================
*	string
*/
int StrLenX(const char *s, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!s, "\n\aSB-MemMgr: StrLen(s), s is NULL\n");
  if (strlen(s) > iCheckSize(s))
    uuerr(1, "\n\aSB-MemMgr: StrLen(s), s is strange\n");
#endif
  return strlen(s);
}

char *StrCopyX(char *dst, const char *src, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!src, "\n\aSB-MemMgr: StrCopy(dst,src), src is NULL\n");
  uuerr(!dst, "\n\aSB-MemMgr: StrCopy(dst,src), dst is NULL\n");
  if (iCheckSize(dst) <= strlen(src))
    uuerr(1, "\n\aSB-MemMgr: StrCopy(dst,src), dst size error\n");
#endif
  return strcpy(dst, src);
}

char *StrCatX(char *dst, const char *src, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!src, "\n\aSB-MemMgr: StrCat(dst,src), src is NULL\n");
  uuerr(!dst, "\n\aSB-MemMgr: StrCat(dst,src), dst is NULL\n");
  if (iCheckSize(dst) < (strlen(src) + strlen(dst) + 1))
    uuerr(1, "\n\aSB-MemMgr: StrCat(dst,src), dst is small\n");
#endif
  return strcat(dst, src);
}

int StrCompareX(const char *a, const char *b, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!a, "\n\aSB-MemMgr: StrCompare(a,b), a is NULL\n");
  uuerr(!b, "\n\aSB-MemMgr: StrCompare(a,b), b is NULL\n");
#endif
  return strcmp(a, b);
}

char *StrChrX(const char *src, int c, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!src, "\n\aSB-MemMgr: StrChr(src), src is NULL\n");
#endif
  return strchr(src, c);
}

int StrCaselessCompareX(const char *s1, const char *s2, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!s1, "\n\aSB-MemMgr: StrCaselessCompare(s1,s2), s1 is NULL\n");
  uuerr(!s2, "\n\aSB-MemMgr: StrCaselessCompare(s1,s2), s1 is NULL\n");
#endif
#if defined(_BCB_W32_IDE) || defined(_Win32)
  return stricmp(s1, s2);
#else
  return strcasecmp(s1, s2);
#endif
}

int StrNCaselessCompareX(const char *s1, const char *s2, int n, const char *file,
                         int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!s1, "\n\aSB-MemMgr: StrNCaselessCompare(s1,s2,n), s1 is NULL\n");
  uuerr(!s2, "\n\aSB-MemMgr: StrNCaselessCompare(s1,s2,n), s1 is NULL\n");
  uuerr((n <= 0), "\n\aSB-MemMgr: StrNCaselessCompare(s1,s2,n), n<=0\n");
#endif
#if defined(_BCB_W32_IDE) || defined(_Win32)
  return strnicmp(s1, s2, n);
#else
  return strncasecmp(s1, s2, n);
#endif
}

int StrNCaselessCompareSM(const char *s1, const char *s2, int n)
{
#if defined(_BCB_W32_IDE) || defined(_Win32)
  return strnicmp(s1, s2, n);
#else
  return strncasecmp(s1, s2, n);
#endif
}

char *StrNCatX(char *dst, const char *src, int n, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!src, "\n\aSB-MemMgr: StrNCat(dst,src,n), src is NULL\n");
  uuerr(!dst, "\n\aSB-MemMgr: StrNCat(dst,src,n), dst is NULL\n");
  uuerr((n <= 0), "\n\aSB-MemMgr: StrNCat(dst,src,n), n<=0\n");
  if (MemPtrSizeX(dst, file, line) < n)
    uuerr(1, "\n\aSB-MemMgr: StrNCat(), dst is small\n");
#endif
  return strncat(dst, src, n);
}

int StrNCompareX(const char *a, const char *b, int n, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!a, "\n\aSB-MemMgr: StrNCompare(a,b,n), a is NULL\n");
  uuerr(!b, "\n\aSB-MemMgr: StrNCompare(a,b,n), b is NULL\n");
  uuerr((n <= 0), "\n\aSB-MemMgr: StrNCompare(a,b,n), n<=0\n");
#endif
  return strncmp(a, b, n);
}

char *StrNCopyX(char *dst, const char *src, int n, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!src, "\n\aSB-MemMgr: StrNCopy(dst,src,n), src is NULL\n");
  uuerr(!dst, "\n\aSB-MemMgr: StrNCopy(dst,src,n), dst is NULL\n");
  uuerr((n <= 0), "\n\aSB-MemMgr: StrNCopy(dst,src,n), n<=0\n");
#endif
  return strncpy(dst, src, n);
}

char *StrStrX(const char *s1, const char *s2, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!s1, "\n\aSB-MemMgr: StrStr(s1,s2), s1 is NULL\n");
  uuerr(!s2, "\n\aSB-MemMgr: StrStr(s1,s2), s2 is NULL\n");
#endif
  return strstr(s1, s2);
}

char *StrRevChrX(const char *src, int c, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
  uuerr(!src, "\n\aSB-MemMgr: StrRevChr(src), src is NULL\n");
#endif
  return strrchr(src, c);
}

char *StrErrorX(int err, const char *file, int line)
{
#if defined(CHECK_PTRS_LEV2)
  strcpy(err_module, file);
  err_line = line;
#endif
  return strerror(err);
}

#else // MALLOC_LIMITED
void memmgr_init()
{
}

void memmgr_close()
{
}

void memmgr_setabort(int v)
{
}

int memmgr_getalloc()
{
  return 0;
}

int memmgr_getmaxalloc()
{
  return 0;
}
#endif
