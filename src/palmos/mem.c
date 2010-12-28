// $Id: mem.c 696 2009-11-22 11:46:30Z zeeb90au $
// This file is part of SmallBASIC
//
// Simple and Virtual Memory Manager 
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sys.h"
#include "pmem.h"
#include "panic.h"
#include "smbas.h"
#include "messages.h"

#if defined(_PalmOS) || defined(_VTOS)
#include "lopen_bridge.h"
#endif

#if !defined(O_BINARY)
#define O_BINARY    0
#endif

/*
 * MALLOC_LIMITED - limited systems without memory-handles
 * using memory block address as handle.
 * each memory block has an mbi_t structure on its head, 
 * the data pointer (which is used from the rest application)
 * is the address real_ptr + sizeof(mbi_t)
 */
#if defined(MALLOC_LIMITED)
typedef struct {
  dword size;
} mbi_t;
#endif

#if defined(_PalmOS)
static FileHand log_dev;        /* logfile file handle */
#elif defined(_VTOS)
static FILE *log_dev;
#else
static int log_dev;             /* logfile file handle */
static char log_name[OS_PATHNAME_SIZE + 1]; /* LOGFILE filename */
#endif

#if defined(ENABLE_VMM)
#if defined(_PalmOS)
static LocalID vm_lid;
static FileHand vm_dev;         /* VM swap file handle */
static FileHand vm_index;       /* VM index file handle */
#else
static int vm_dev;              /* VM swap file handle */
static int vm_index;            /* VM index file handle */
static char vm_name[OS_PATHNAME_SIZE + 1];  /* VM swap filename */
static char vm_idxname[OS_PATHNAME_SIZE + 1]; /* VM index filename */
#endif
static int vm_init_count;       /* VM initialization check */

/*
 * VMM node
 */
struct vm_node_s {
  void *ptr;                    /* pointer to local memory (only locked chunks
                                 * had pointer!=NULL) */
  dword offset;                 /* offset in file */
  word size;                    /* block size */
  word status;                  /* low byte = lock counter, high byte = [bit
                                 * 7=deleted] */
};
typedef struct vm_node_s vm_node;

static int vm_index_page_size;  /* VMM index page size (see vm_init(), index
                                 * page size = 10% of free memory */

static vm_node *vm_table;       /* VMM index page */
static int vm_count;            /* VMM number of nodes */
static word vm_bank;            /* VMM current index-page number (this one is
                                 * loaded into memory @'vm_table') */
static dword vm_fsize;          /* VMM data-file size; speed optimization */

#endif // ENABLE_VMM

/* ERROR MESSAGES */
void err_outofmem(void) SEC(TRASH);
void err_outofmem(void)
{
  panic(MEM_OUT_OF_MEM);
}

void err_tmpalloc(dword size) SEC(TRASH);
#if defined(_PalmOS)
void err_tmpalloc(dword size)
{
  panic("\nSB-MemMgr:\nOS refuses my request for %ld bytes", size);
}
#else
void err_tmpalloc(dword size)
{
  panic("tmp_alloc: OS refuses my request for %ld bytes", size);
}
#endif

void err_tmpfree(void) SEC(TRASH);
void err_tmpfree(void)
{
  panic("tmp_free: Cannot recover handle");
}

void err_tmprealloc1(void) SEC(TRASH);
void err_tmprealloc1(void)
{
  panic("tmp_realloc: Cannot recover handle");
}

void err_tmprealloc2(dword size) SEC(TRASH);
void err_tmprealloc2(dword size)
{
  panic("tmp_realloc: Cannot resize memory to %ld", size);
}

void err_memalloc1(dword size) SEC(TRASH);
void err_memalloc1(dword size)
{
  panic("mem_alloc: size=%ld\n", size);
}

void err_memrealloc1(void) SEC(TRASH);
void err_memrealloc1(void)
{
  panic("mem_realloc: Invalid handle");
}

void err_memrealloc2(dword size) SEC(TRASH);
void err_memrealloc2(dword size)
{
  panic("mem_realloc: Cannot resize memory to %ld", size);
}

void err_lock(mem_t h) SEC(TRASH);
void err_lock(mem_t h)
{
  panic("mem_lock: already locked (%ld)", (long)h);
}

void err_memunlock(void) SEC(TRASH);
void err_memunlock(void)
{
  panic("mem_unlock:MemHandleErr");
}

void err_memfree(void) SEC(TRASH);
void err_memfree(void)
{
  panic("mem_free:MemHandleErr");
}

void err_mlistadd(void) SEC(TRASH);
void err_mlistadd(void)
{
  panic("mlist_add: OUT OF MEMORY");
}

void err_tlistadd(void) SEC(TRASH);
void err_tlistadd(void)
{
  panic("tmplist_add: OUT OF MEMORY");
}

/*
 * Allocate local memory
 */
#if !defined(HAVE_C_MALLOC)

#if defined(_PalmOS)
void *tmp_alloc(dword size)
{
  void *ptr;
  MemHandle hdl;

  if (size < 0)
    err_tmpalloc(size);

  // hdl = MemHandleNew(size);
  // PalmOS-5 sim dr12... another weird bug,
  // if there is no convertion to (int) it will get a crazy rq for ~1MB
  hdl = MemHandleNew((long)size);
  if (hdl == 0)
    err_tmpalloc(size);
  ptr = MemHandleLock(hdl);

  return ptr;
}
#elif defined(MALLOC_LIMITED)   /* limited systems without memory handles */
void *tmp_alloc(dword size)
{
  byte *ptr;

  if (size < 0)
    err_tmpalloc(size);

  if ((ptr = malloc(size + sizeof(mbi_t))) == NULL)
    err_tmpalloc(size);

  ((mbi_t *) ptr)->size = size;
  return ptr + sizeof(mbi_t);
}
#else /* unix memmgr */
void *tmp_allocX(dword size, const char *file, int line)
{
  void *ptr;
  MemHandle hdl;

  if (size == 0)
    err_tmpalloc(size);

  hdl = MemHandleNewX(size, file, line);
  if (hdl == 0)
    err_tmpalloc(size);
  ptr = MemHandleLock(hdl);

  return ptr;
}
#endif

/*
 * Free local memory
 */
#if defined(_PalmOS)
void tmp_free(void *ptr)
{
  MemHandle hdl;

  if (ptr == NULL)
    return;

  MemPtrSize(ptr);              // check

  hdl = MemPtrRecoverHandle(ptr);
  if (hdl == 0)
    err_tmpfree();
  MemHandleUnlock(hdl);
  MemHandleFree(hdl);
}
#elif defined(MALLOC_LIMITED)   /* limited systems without memory handles */
void tmp_free(void *ptr)
{
  if (ptr == NULL) {
    err_tmpfree();
  }
  else {
    byte *p = (byte *) ptr;
    p -= sizeof(mbi_t);
    free(p);
  }
}
#else /* unix memmgr */
void tmp_freeX(void *ptr, const char *file, int line)
{
  MemHandle hdl;

  if (ptr == NULL)
    panic("%s (%d): tmp_free(): NULL ptr passed", file, line);

  hdl = MemPtrRecoverHandleX(ptr, file, line);
  if (hdl == 0)
    err_tmpfree();
  MemHandleUnlockX(hdl, file, line);
  MemHandleFreeX(hdl, file, line);
}
#endif

/*
 * Reallocate the size of a memory chunk
 */
#if defined(MALLOC_LIMITED)     /* limited systems without memory handles */
void *tmp_realloc(void *p, dword size)
{
  char *newp;
  dword old_size;
  byte *ptr = (byte *) p;

  if (ptr == NULL) {
    err_tmprealloc1();
    return 0;
  }

  // get the real ptr
  ptr -= sizeof(mbi_t);
  old_size = ((mbi_t *) ptr)->size;

  // allocate a new block
  newp = malloc(size + sizeof(mbi_t));
  if (newp == NULL)
    err_tmprealloc2(size);

  // copy data from old to the new block
  memcpy(newp + sizeof(mbi_t), ((byte *) ptr) + sizeof(mbi_t),
         I2MIN(size, old_size));
  ((mbi_t *) newp)->size = size;

  // free old
  free(ptr);

  // Notes: this will create a lot of spaces (fragmentation)
  newp += sizeof(mbi_t);
  return newp;
}
#else
void *tmp_realloc(void *ptr, dword size)
{
  MemHandle hdl;

  MemPtrSize(ptr);              // check
  hdl = MemPtrRecoverHandle(ptr);
  if (hdl == 0)
    err_tmprealloc1();

  MemHandleUnlock(hdl);

#if defined(_PalmOS)
  // memory defrag
  MemHeapCompact(0);
#endif

  if (MemHandleResize(hdl, size))
    err_tmprealloc2(size);

  return MemHandleLock(hdl);
}
#endif

/*
 */
char *tmp_strdup(const char *source)
{
  char *p;

  p = tmp_alloc(strlen(source) + 1);
  strcpy(p, source);
  return p;
}

#if defined(MALLOC_LIMITED)     /* limited systems without memory handles */
/*
 * returns the size of the block
 */
dword MemPtrSize(void *ptr)
{
  if (ptr == NULL)
    panic("MemPtrSize: invalid handle (zero)");
  return ((mbi_t *) ((byte *) ptr - sizeof(mbi_t)))->size;
}

dword MemHandleSize(mem_t h)
{
  return MemPtrSize((void *)h);
}
#endif

/*
 */
dword mem_handle_size(mem_t h)
{
  return MemHandleSize(h);
}

/*
 * Allocate a memory handle (a memory block on "storage area")
 */
#if defined(MALLOC_LIMITED)     /* limited systems without memory handles */
mem_t mem_alloc(dword size)
{
  mem_t h = 0;

  if (size == 0)
    err_memalloc1(size);

  h = (mem_t) tmp_alloc(size);
  if (h == 0)
    err_outofmem();
  return h;
}
#else
#if !defined(_PalmOS)
mem_t mem_allocX(dword size, const char *file, int line)
#else
mem_t mem_alloc(dword size)
#endif
{
  mem_t h = 0;

  if (size == 0)
    err_memalloc1(size);

#if !defined(_PalmOS)
  h = MemHandleNewX(size, file, line);
#else
  h = MemHandleNew(size);
#endif
  if (h == 0)
    err_outofmem();
  return h;
}
#endif

/*
 * lock a memory handle (moves the memory block to dynamic RAM)
 */
#if defined(MALLOC_LIMITED)     /* limited systems without memory handles */
void *mem_lock(mem_t h)
{
  return (void *)h;
}
#else
void *mem_lock(mem_t h)
{
  void *p;

  if (h <= 0)
    err_lock(h);
  p = MemHandleLock(h);
  if (p == NULL)
    err_lock(h);
  return p;
}
#endif

/*
 * unlock a memory handle
 */
#if defined(MALLOC_LIMITED)     /* limited systems without memory handles */
void mem_unlock(mem_t h)
{
  ;                             // do nothing
}
#else
void mem_unlock(mem_t h)
{
  if (h <= 0)
    err_memunlock();

  MemHandleUnlock(h);
}
#endif

/*
 * Reallocate the size of a memory chunk
 */
#if defined(MALLOC_LIMITED)     /* limited systems without memory handles */
mem_t mem_realloc(mem_t hdl, dword new_size)
{
  mem_t newh;

  if (hdl == 0)
    err_memrealloc1();
  newh = (mem_t) tmp_realloc((void *)hdl, new_size);
  return newh;                  // In true systems the realloc does not change
  // the handle,
  // but for the SB that is the rule (the handle will be changed)
}
#else
mem_t mem_realloc(mem_t hdl, dword new_size)
{
  if (hdl == 0)
    err_memrealloc1();

  if (MemHandleResize(hdl, new_size))
    err_memrealloc2(new_size);
  return hdl;
}
#endif

/*
 * free a memory block from storage RAM
 */
#if defined(MALLOC_LIMITED)     /* limited systems without memory handles */
void mem_free(mem_t h)
{
  if (h == 0)
    err_memfree();
  else
    tmp_free((void *)h);
}
#elif defined(_PalmOS)
void mem_free(mem_t h)
{
  if (h <= 0)
    err_memfree();

  MemHandleFree(h);
}
#else /* unx_memmgr */
void mem_freeX(mem_t h, const char *file, int line)
{
  if (h <= 0)
    err_memfree();

  MemHandleFreeX(h, file, line);
}
#endif

/*
 * creates a memory block in storage RAM for string 'text'
 */
mem_t mem_new_text(const char *text)
{
  mem_t h;
  int l;
  char *p;

  l = strlen(text) + 1;
  h = mem_alloc(l);
  p = (char *)mem_lock(h);
  strcpy(p, text);
  mem_unlock(h);
  return h;
}
#endif // HAVE_C_MALLOC

#if defined(ENABLE_MEMLIST)

/*
 * dynamic single-linked list in storage RAM
 *
 * list initialization
 */
void mlist_init(mlist_t * lst)
{
  lst->head = lst->tail = NULL;
  lst->count = 0;
}

void mlist_clear(mlist_t * lst)
{
  mnode_t *cur, *pre;

  cur = lst->head;
  while (cur) {
    pre = cur;
    cur = cur->next;

    mem_free(pre->data);
    MemPtrFree(pre);
  }

  mlist_init(lst);
}

mnode_t *mlist_add(mlist_t * lst, mem_t h)
{
  mnode_t *np;

  np = (mnode_t *) MemPtrNew(sizeof(mnode_t));
  if (!np)
    err_mlistadd();
  np->data = h;
  np->next = NULL;

  if (lst->head)
    (lst->tail->next = np, lst->tail = np);
  else
    lst->head = lst->tail = np;

  lst->count++;
  return np;
}

#endif // ENABLE_MEMLIST


#if defined(ENABLE_TMPLIST)

/*
 * dynamic single-linked list in dynamic RAM
 */

void tmplist_init(tmplist_t * lst)
{
  lst->head = lst->tail = NULL;
  lst->count = 0;
}

void tmplist_clear(tmplist_t * lst)
{
  tmpnode_t *cur, *pre;

  cur = lst->head;
  while (cur) {
    pre = cur;
    cur = cur->next;

    tmp_free(pre->data);
    MemPtrFree(pre);
  }

  tmplist_init(lst);
}

tmpnode_t *tmplist_add(tmplist_t * lst, void *data, int size)
{
  tmpnode_t *np;

  np = (tmpnode_t *) MemPtrNew(sizeof(tmpnode_t));
  if (!np)
    err_tlistadd();
  np->data = tmp_alloc(size);
  memcpy(np->data, data, size);
  np->next = NULL;

  if (lst->head)
    (lst->tail->next = np, lst->tail = np);
  else
    lst->head = lst->tail = np;

  lst->count++;
  return np;
}

#endif // ENABLE_TMPLIST

#if !defined(_FLTK)

/*
 * LOGFILE
 *
 */
void lwrite(const char *buf)
{
#if defined(_PalmOS)
  Err ferr;
#elif defined(_Win32)
  char *p;
#endif

  // //////
  // open
#if defined(_PalmOS)
  log_dev = FileOpen(0, "SB.LOG", ID_UFST, ID_SmBa, fileModeAppend, &ferr);
  if (ferr != 0)
    panic("LOG: Error on creating log-file");
#elif defined(_VTOS)
  log_dev = fopen("sb.log", "w+t");
  if (log_dev == NULL)
    panic("LOG: Error on creating log-file");
  fseek(log_dev, 0, SEEK_END);
#else
#if defined(_Win32) || defined(__MINGW32__)
  if (getenv("SBLOG"))
    strcpy(log_name, getenv("SBLOG"));
  else
    sprintf(log_name, "c:%csb.log", OS_DIRSEP);
#else /* a real OS */
  sprintf(log_name, "%ctmp%csb.log", OS_DIRSEP, OS_DIRSEP);
#endif

  log_dev = open(log_name, O_RDWR, 0660);
  lseek(log_dev, 0, SEEK_END);
  if (log_dev == -1)
    log_dev = open(log_name, O_CREAT | O_RDWR, 0660);
  if (log_dev == -1)
    panic("LOG: Error on creating log file");
#endif

  // /////////
  // write
#if defined(_PalmOS)
  FileWrite(log_dev, (char *)buf, strlen(buf), 1, &ferr);
  if (ferr) {
    if (ferr != fileErrEOF)
      panic("LOG: write failed (ERR:%d)", ferr);
  }
#elif defined(_VTOS)
  if (fwrite(buf, 1, strlen(buf), log_dev) != strlen(buf))
    panic("LOG: write failed");
#else
  if (write(log_dev, buf, strlen(buf)) == -1)
    panic("LOG: write failed");
#endif

  // / close
#if defined(_PalmOS)
  FileClose(log_dev);
#elif defined(_VTOS)
  fclose(log_dev);
#else
  close(log_dev);
#endif
}
#endif // NOT FLTK

//
void lprintf(const char *fmt, ...)
{
  va_list ap;
  char *buf;

  buf = tmp_alloc(SB_TEXTLINE_SIZE + 1);

  va_start(ap, fmt);
#if defined(_PalmOS)
  StrVPrintF(buf, fmt, ap);
#else
  vsprintf(buf, fmt, ap);
#endif
  va_end(ap);
  lwrite(buf);

  tmp_free(buf);
}

//
void lg(const char *fmt, ...)
{
  va_list ap;
  char *buf;
  // #if defined(_PalmOS)
  // ULong dts;
  // DateTimeType cur_date;
  // #else
  // struct tm tms;
  // time_t now;
  // #endif

  buf = tmp_alloc(SB_TEXTLINE_SIZE + 1);

  // ///////////
  // time/date
  // #if defined(_PalmOS)
  // dts = TimGetSeconds();
  // TimSecondsToDateTime(dts, &cur_date);
  // StrPrintF(buf, 
  // "%02d/%02d %02d:%02d:%02d: ", 
  // /* cur_date.year, */ cur_date.month, cur_date.day, 
  // cur_date.hour, cur_date.minute, cur_date.second);
  // #else
  // time(&now);
  // tms = *localtime(&now);
  // sprintf(buf, 
  // "%02d/%02d %02d:%02d:%02d: ", 
  // /* tms.tm_year+1900, */ tms.tm_mon+1, tms.tm_mday,
  // tms.tm_hour, tms.tm_min, tms.tm_sec);
  // #endif
  // logwrite(buf);

  lwrite("--- ");
  va_start(ap, fmt);
#if defined(_PalmOS)
  StrVPrintF(buf, fmt, ap);
#else
  vsprintf(buf, fmt, ap);
#endif
  va_end(ap);
  strcat(buf, "\n");
  lwrite(buf);

  tmp_free(buf);
}

/* -----------------------------------------------------------------------------------------------------------------------
 *
 * VIRTUAL MEMORY
 *
 * VMM works like DBMS
 *
 * Its has a table of vm_node (vm_index file) witch keeps information about each allocated handle
 * There is loaded (into the memory) only a part of this index of 'vmm_index_page_size' nodes witch located at vm_table
 * Each block of vmm_index_page_size nodes is called bank.
 */

#if defined(ENABLE_VMM)

/*
 * initialize
 */
void vm_init()
{
#if defined(_PalmOS)
  dword dwfre, dwmax;
  Err ferr;
#endif

  if (vm_init_count)
    vm_init_count++;
  else {

#if defined(_PalmOS)
    vm_lid = DmFindDatabase(0, "SB.SWP");
    if (vm_lid) {
      FileDelete(0, "SB.SWP");
      FileDelete(0, "SBIDX.SWP");
    }
    vm_dev =
      FileOpen(0, "SB.SWP", ID_DATA, ID_SmBa, fileModeReadWrite | fileModeTemporary,
               &ferr);
    vm_index =
      FileOpen(0, "SBIDX.SWP", ID_DATA, ID_SmBa,
               fileModeReadWrite | fileModeTemporary, &ferr);
    if (ferr != 0)
      panic("VMM: Error on creating swap file");
#else
#if defined(_Win32)
    sprintf(vm_name, "c:\sb%d.swp", 1);
    sprintf(vm_idxname, "c:\sb%di.swp", 1);
#else /* a real OS */
    sprintf(vm_name, "%ctmp%csb%d.swp", OS_DIRSEP, OS_DIRSEP, getpid());
    sprintf(vm_idxname, "%ctmp%csb%di.swp", OS_DIRSEP, OS_DIRSEP, getpid());
#endif
    remove(vm_name);
    remove(vm_idxname);
    vm_dev = open(vm_name, O_CREAT | O_RDWR | O_EXCL, 0660);
    vm_index = open(vm_idxname, O_CREAT | O_RDWR | O_EXCL, 0660);
    if (vm_dev < 0)
      panic("VMM: Error on creating swap file");
#endif

    // 
#if defined(_PalmOS)
    MemHeapFreeBytes(0, &dwfre, &dwmax);
    vm_index_page_size = ((dwmax / 10) / sizeof(vm_node));  // 10% of free
    // memory
#else
    vm_index_page_size = (65536 / sizeof(vm_node)); // 64KB
#endif
    vm_table = (vm_node *) tmp_alloc(sizeof(vm_node) * vm_index_page_size);
    vm_bank = 0;
    vm_count = 0;
    vm_fsize = 0;

    vm_init_count++;
  }
}

/*
 * Switch bank 
 */
void vm_swbank(int bank)
{
  int bsize = vm_index_page_size * sizeof(vm_node);
  int bmax = (vm_count - 1) / vm_index_page_size;
#if defined(_PalmOS)
  Err ferr;
#endif

  // printf("sw bank %d -> %d\n", vm_bank, bank);

  if (bank != vm_bank) {
    /*
     *   Save current 'bank' to disk
     */
#if defined(_PalmOS)
    FileSeek(vm_index, bsize * vm_bank, fileOriginBeginning);
    FileClearerr(vm_index);
    FileWrite(vm_index, vm_table, bsize, 1, &ferr);
    if (ferr) {
      if (ferr != fileErrEOF)
        panic("VMM: Swap page #1 (%d) failed", bank);
    }
#else
    lseek(vm_index, bsize * vm_bank, SEEK_SET);
    if (write(vm_index, vm_table, bsize) != bsize)
      panic("VMM: Swap page #1 (%d) failed", bank);
#endif
  }

#if defined(_PalmOS)
  FileSeek(vm_index, bsize * bank, fileOriginBeginning);
#else
  lseek(vm_index, bsize * bank, SEEK_SET);
#endif

  if (bank == (bmax + 1)) {
    /*
     *   We need a new bank. Create an empty bank and switch to the new bank
     */
    memset(vm_table, 0, bsize);

#if defined(_PalmOS)
    FileClearerr(vm_index);
    FileWrite(vm_index, vm_table, bsize, 1, &ferr);
    if (ferr) {
      if (ferr != fileErrEOF)
        panic("VMM: Swap page #2 failed (%d)", bank);
    }
#else
    if (write(vm_index, vm_table, bsize) != bsize)
      panic("VMM: Swap page #2 failed (%d)", bank);
#endif
  }
  else if (bank <= bmax) {
    /*
     *   load bank
     */
#if defined(_PalmOS)
    FileRead(vm_index, vm_table, bsize, 1, &ferr);
    if (ferr) {
      if (ferr != fileErrEOF)
        panic("VMM: load page failed (%d)", bank);
    }
#else
    if (read(vm_index, vm_table, bsize) != bsize)
      panic("VMM: load page failed (%d)", bank);
#endif
  }
  else
    panic("VMM: Invalid page rq (%d)", bank);

  vm_bank = bank;
}

/*
 * Returns the 'idx' VM index-node
 */
vm_node *vm_getnode(int idx)
{
  int bank = idx / vm_index_page_size;
  int ridx = idx - (bank * vm_index_page_size);

  if (idx < 0 || idx >= vm_count)
    panic("VMM: Invalid node rq (%d)", idx);

  if (bank != vm_bank)
    vm_swbank(bank);

  return &vm_table[ridx];
}

/*
 * Creates & returns a new VM index node
 */
vm_node *vm_newnode()
{
  int bank = vm_count / vm_index_page_size;

  if (bank != vm_bank)
    vm_swbank(bank);

  vm_count++;
  return vm_getnode(vm_count - 1);
}

/*
 * Destroy everything
 */
void vm_close()
{
  int i;
  vm_node *node = NULL;

  if (vm_init_count) {
    vm_init_count--;
    if (vm_init_count == 0) {

      /*
       *   cleanup - nodes
       */
      for (i = 0; i < vm_count; i++) {
        node = vm_getnode(i);
        if (node->status & 0xFF)
          tmp_free(node->ptr);
      }

      tmp_free(vm_table);

      /*
       *   cleanup
       */
      vm_table = NULL;
      vm_bank = 0;
      vm_count = 0;
      vm_fsize = 0;

#if defined(_PalmOS)
      FileClose(vm_index);
      FileClose(vm_dev);
      FileDelete(0, "SBIDX.SWP");
      FileDelete(0, "SB.SWP");
#else
      remove(vm_idxname);
      remove(vm_name);
#endif
    }
  }
}

/*
 * Returns the VM handle of ptr
 */
int vm_findptr(void *ptr)
{
  int i;
  vm_node *node = NULL;

  for (i = 0; i < vm_count; i++) {
    node = vm_getnode(i);
    if (node->ptr == ptr)
      return i;
  }

  return -1;
}

/*
 * Allocates a new memory block and returns the handle
 */
int vm_halloc(word size)
{
#if defined(_PalmOS)
  Err ferr;
#endif
  int i, idx;
  vm_node *node = NULL;

  idx = -1;

  // search for deleted record
  for (i = 0; i < vm_count; i++) {
    node = vm_getnode(i);
    if (node->status & 0x8000) {
      if (node->size >= size) {
        idx = i;                // found
        break;
      }
      // we can continue to looking for the best deleted block, but VM is
      // already too slow on PalmOS
    }
  }

  // 
  if (idx == -1) {
    // this is a new record
    node = vm_newnode();
    idx = vm_count - 1;

    node->offset = vm_fsize;
    vm_fsize = vm_fsize + (dword) size;
  }
  else {
    // this is replace/update
    if ((node->size - size) > 64) { // the remain chunk is large enought to 
      // keep it 
      node->offset += node->size;
      node->size -= size;

      node = vm_newnode();
      idx = vm_count - 1;
    }
    else                        // the remain chunk is too small, so we'll
      // replace it (actualy we going to append this
      // to the new)
      size = node->size;        // because the old size is eq or greater than
    // the rq size
  }

  // 
  node->size = size;
  node->status = 0;

  // write an empty record
  node->ptr = tmp_alloc(size);
  node->size = size;
  if (node->ptr == NULL)
    panic("VMM: Out of memory (new record), handle = %d, size = %d", idx,
          node->size);
  memset(node->ptr, 0, node->size);

#if defined(_PalmOS)
  FileSeek(vm_dev, node->offset, fileOriginBeginning);
  FileClearerr(vm_dev);
  FileWrite(vm_dev, node->ptr, node->size, 1, &ferr);
  if (ferr) {
    if (ferr != fileErrEOF)
      panic("VMM: Swap file corrupted #6 (%d)", idx);
  }
#else
  lseek(vm_dev, node->offset, SEEK_SET);
  if (write(vm_dev, node->ptr, node->size) != node->size)
    panic("VMM: Swap file corrupted #6 (%d)", idx);
#endif

  tmp_free(node->ptr);
  node->ptr = NULL;

  // 
  return idx;
}

/*
 * Release a memory block
 */
void vm_hfree(int idx)
{
  vm_node *node = NULL;

  node = vm_getnode(idx);
  if (node->status & 0xFF)
    panic("VMM: vm_hfree: Handle %d is locked", idx);
  node->status = 0x8000;
}

/*
 * lock handle
 *
 * (load data into memory)
 */
void *vm_lock(int idx)
{
  vm_node *node = NULL;
#if defined(_PalmOS)
  Err ferr;
#endif

  node = vm_getnode(idx);

  if (node->status & 0xFF)
    node->status = (node->status & 0xFF) + 1;
  else {
    node->ptr = tmp_alloc(node->size);
    if (!node->ptr)
      panic("VMM: Out of memory when I locking the %d handle", idx);

#if defined(_PalmOS)
    if (FileSeek(vm_dev, node->offset, fileOriginBeginning))
      panic("VMM: Swap file corrupted #1 (%d)", idx);
    FileRead(vm_dev, node->ptr, node->size, 1, &ferr);
    if (ferr) {
      if (ferr != fileErrEOF)
        panic("VMM: Swap file corrupted #2 (%d)", idx);
    }
#else
    if (lseek(vm_dev, node->offset, SEEK_SET) >= 0 || node->offset == 0) {
      if (read(vm_dev, node->ptr, node->size) != node->size)
        panic("VMM: Swap file corrupted #2 (%d)", idx);
    }
    else
      panic("VMM: Swap file corrupted #1 (%d)", idx);
#endif

    node->status++;
  }

  return node->ptr;
}

/*
 * unlock handle
 *
 * (write data to disk and free associeted memory)
 */
void vm_unlock(int idx)
{
  vm_node *node = NULL;
#if defined(_PalmOS)
  Err ferr;
#endif

  node = vm_getnode(idx);

  if (node->status & 0xFF) {
    node->status = (node->status & 0xFF) - 1;

    if ((node->status & 0xFF) == 0) {
#if defined(_PalmOS)
      if (FileSeek(vm_dev, node->offset, fileOriginBeginning))
        panic("VMM: Swap file corrupted #3 (%d)", idx);
      FileWrite(vm_dev, node->ptr, node->size, 1, &ferr);
      if (ferr) {
        if (ferr != fileErrEOF)
          panic("VMM: Swap file corrupted #4 (%d)", idx);
      }
#else
      if (lseek(vm_dev, node->offset, SEEK_SET) >= 0) {
        if (write(vm_dev, node->ptr, node->size) != node->size)
          panic("VMM: Swap file corrupted #4 (%d)", idx);
      }
      else
        panic("VMM: Swap file corrupted #3 (%d)", idx);
#endif

      tmp_free(node->ptr);
      node->ptr = NULL;
    }
  }
  else
    panic("VMM: Handle %d is unlocked", idx);
}

/*
 * Creates a new memory block and returns its pointer
 */
void *vm_alloc(word size)
{
  int idx;

  idx = vm_halloc(size);
  return vm_lock(idx);
}

/*
 * Release a memory block using its pointer
 */
void vm_free(void *ptr)
{
  int idx;

  idx = vm_findptr(ptr);
  if (idx >= 0) {
    vm_unlock(idx);
    vm_hfree(idx);
  }
  else
    panic("VMM: Invalid handle");
}

#endif // VMM

