// This file is part of SmallBASIC
//
// Simple and Virtual Memory Manager Unix/Palm
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/pmem.h"
#include "common/panic.h"
#include "common/smbas.h"
#include "common/messages.h"

#if !HAVE_MALLOC_USABLE_SIZE

/**
 * malloc_usable_size() not available on this platform
 *
 * Each memory block has an mbi_t structure on its head, 
 * the data pointer (which is used from the rest application)
 * is the address real_ptr + sizeof(mbi_t)
 */
typedef struct {
  dword size;
} mbi_t;

void err_outofmem(void) {
  panic(MEM_OUT_OF_MEM);
}

void err_tmpalloc(dword size) {
  panic("tmp_alloc: OS refuses my request for %ld bytes", size);
}

void err_tmpfree(void) {
  panic("tmp_free: Cannot recover handle");
}

void err_tmprealloc1(void) {
  panic("tmp_realloc: Cannot recover handle");
}

void err_tmprealloc2(dword size) {
  panic("tmp_realloc: Cannot resize memory to %ld", size);
}

void err_memalloc1(dword size) {
  panic("mem_alloc: size=%ld\n", size);
}

void err_memrealloc1(void) {
  panic("mem_realloc: Invalid handle");
}

void err_memfree(void) {
  panic("mem_free:MemHandleErr");
}

/**
 * Allocate local memory
 */
void *tmp_alloc(dword size)
{
  byte *ptr;

  if (size < 0) {
    err_tmpalloc(size);
  }

  if ((ptr = malloc(size + sizeof(mbi_t))) == NULL) {
    err_tmpalloc(size);
  }

  ((mbi_t *) ptr)->size = size;
  return ptr + sizeof(mbi_t);
}

/**
 * Free local memory
 */
void tmp_free(void *ptr) {
  if (ptr == NULL) {
    err_tmpfree();
  }
  else {
    byte *p = (byte *) ptr;
    p -= sizeof(mbi_t);
    free(p);
  }
}

/**
 * Reallocate the size of a memory chunk
 */
void *tmp_realloc(void *p, dword size) {
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
  if (newp == NULL) {
    err_tmprealloc2(size);
  }

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

/**
 * returns the size of the block
 */
dword mem_handle_size(mem_t h) {
  void *ptr = (void *)h;
  if (ptr == NULL) {
    panic("MemPtrSize: invalid handle (zero)");
  }
  return ((mbi_t *) ((byte *) ptr - sizeof(mbi_t)))->size;
}

/**
 * Allocate a memory handle (a memory block on "storage area")
 */
mem_t mem_alloc(dword size) {
  mem_t h = 0;

  if (size == 0) {
    err_memalloc1(size);
  }

  h = (mem_t) tmp_alloc(size);
  if (h == 0) {
    err_outofmem();
  }
  return h;
}

/**
 * Reallocate the size of a memory chunk
 */
mem_t mem_realloc(mem_t hdl, dword new_size) {
  mem_t newh;

  if (hdl == 0) {
    err_memrealloc1();
  }

  newh = (mem_t) tmp_realloc((void *)hdl, new_size);

  // In true systems the realloc does not change
  // the handle but for the SB that is the rule (the handle will be changed)
  return newh;
}

/**
 * free a memory block from storage RAM
 */
void mem_free(mem_t h) {
  if (h == 0) {
    err_memfree();
  }
  else {
    tmp_free((void *)h);
  }
}

/**
 * wrapper for strdup
 */
char *tmp_strdup(const char *source) {
  char* p = tmp_alloc(strlen(source) + 1);
  strcpy(p, source);
  return p;
}

#endif // !HAVE_MALLOC_USABLE_SIZE

void err_tlistadd(void) {
  panic("tmplist_add: OUT OF MEMORY");
}

void tmplist_init(tmplist_t * lst) {
  lst->head = lst->tail = NULL;
  lst->count = 0;
}

void tmplist_clear(tmplist_t * lst) {
  tmpnode_t *cur, *pre;

  cur = lst->head;
  while (cur) {
    pre = cur;
    cur = cur->next;

    tmp_free(pre->data);
    tmp_free(pre);
  }

  tmplist_init(lst);
}

tmpnode_t *tmplist_add(tmplist_t * lst, void *data, int size) {
  tmpnode_t *np;

  np = (tmpnode_t *) tmp_alloc(sizeof(tmpnode_t));
  if (!np) {
    err_tlistadd();
  }

  np->data = tmp_alloc(size);
  memcpy(np->data, data, size);
  np->next = NULL;

  if (lst->head) {
    (lst->tail->next = np, lst->tail = np);
  } else {
    lst->head = lst->tail = np;
  }

  lst->count++;
  return np;
}

