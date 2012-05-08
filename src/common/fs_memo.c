// This file is part of SmallBASIC
//
// PalmOS MEMO/DB driver
// On non-PalmOS systems, names are created with prefix "memo_"
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/device.h"
#include "common/pproc.h"
#include "common/match.h"

#include <errno.h>
#include <dirent.h>

#if USE_TERM_IO
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#endif

typedef int FileHand;

#include "common/fs_stream.h"
#include "common/fs_memo.h"

#if !defined(F_OK)
#define F_OK  0
#endif

/*
 *	memo_mount:
 *		create file list (idea: category works like directory)
 *
 *	memo_umount:
 *		delete file list
 *
 *	memo_open:
 *		loads memo to drv_data (4KB)
 *
 *	memo_close:
 *		flush drv_data to memodb
 *
 *	TODO:
 *	* open for input, must no allow write to the file (also, close() must no write the memo field)
 */

// maximum size of memo-record
#define MEMO_LIMIT_FULL	4000
// maximum size of memo-record for data
#define	MEMO_LIMIT		(MEMO_LIMIT_FULL-OS_FILENAME_SIZE)  // or
// 4096?

#if defined(_PalmOS)
static DmOpenRef memo_ref;
static dbt_t memo_tree;
static int memo_count;

struct memo_node {
  byte name[OS_FILENAME_SIZE + 1];
  int32 pos;
  byte deleted;
};
#endif

void err_memolimit(void) SEC(TRASH);
void err_memolimit(void) {
  rt_raise("MEMO: OVERFLOW");
}

/*
 */
int memo_mount() {
#if defined(_PalmOS)
  LocalID lid;
  int32 i, len;
  mem_t rec_h;
  byte *rec_p, *p;
  struct memo_node node;

  // create vmt
  memo_tree = dbt_create("SBI-Memo", 0);

  // open MemoDB
  lid = DmFindDatabase(0, "MemoDB");
  if (!lid) {
    memo_ref = (DmOpenRef) - 1;
    return 0;
  }
  memo_ref = DmOpenDatabase(0, lid, dmModeReadWrite);
  if (memo_ref == 0) {
    memo_ref = (DmOpenRef) - 1;
    return 0;
  }

  memo_count = DmNumRecords(memo_ref);
  dbt_prealloc(memo_tree, memo_count + 8, sizeof(struct memo_node));

  // for each record
  for (i = 0; i < memo_count; i++) {
    // get record
    rec_h = DmGetRecord(memo_ref, i);
    if (rec_h) {
      rec_p = mem_lock(rec_h);
      if (rec_p == NULL)
      panic("fs_memo, record %d: can't lock record", i);

      len = MemHandleSize(rec_h);
      if (len > MEMO_LIMIT_FULL) {
//                              dev_printf("WARNING: MemoDB record %ld\n  returns size %ld !!!\n", i, len);
        len = OS_FILENAME_SIZE;// Weird MemoDB record !!!
      }

      // copy data (build filename)
      memcpy(node.name, rec_p, MIN(len, OS_FILENAME_SIZE));
      node.name[OS_FILENAME_SIZE] = '\0';
      p = node.name;
      while (*p) {
        if (*p == '\n' || *p == '\r') {
          *p = '\0';
          break;
        }
        p++;
      }
      node.pos = 0;

      // release record
      mem_unlock(rec_h);
      DmReleaseRecord(memo_ref, i, 0);

      // store info
      node.deleted = 0;
      dbt_write(memo_tree, i, &node, sizeof(struct memo_node));
    }
    else {
      strcpy(node.name, "$$$DELETED$$$");
      node.pos = 0;
      node.deleted = 1;
      dbt_write(memo_tree, i, &node, sizeof(struct memo_node));
    }
  }
#endif
  return 1;
}

/*
 *	umount
 */
int memo_umount() {
#if defined(_PalmOS)
  dbt_close(memo_tree);
  if (memo_ref != (DmOpenRef) - 1)
  DmCloseDatabase(memo_ref);
#endif
  return 1;
}

/*
 */
#if defined(_PalmOS)
long memo_findfile(const char *name) SEC(BIO);
long memo_findfile(const char *name)
{
  struct memo_node node;
  int i;

  for (i = 0; i < memo_count; i++) {
    dbt_read(memo_tree, i, &node, sizeof(struct memo_node));

    if ((strcmp(node.name, name) == 0) && (node.deleted == 0))
    return i;
  }

  return -1;
}
#endif

/*
 */
int memo_open(dev_file_t * f) {
#if defined(_PalmOS)
  // /////////////////////////////////////////////////////////////////////////////////////////
  // PalmOS
  struct memo_node node;
  int32 len, name_len;
  byte *rec_p;
  mem_t rec_h;
  dword size = 0;

  memcpy(f->name, f->name + 5, strlen(f->name + 5) + 1);

  // search for filename
  f->handle = memo_findfile(f->name);

  // if there is no record... create one (new file)
  if (f->handle == -1) {
    word index;
    int d;
    char buf[OS_FILENAME_SIZE + 2];

    // default record
    strcpy(buf, node.name);
    strcat(buf, "\012");// 0xA
    size = name_len = strlen(buf) + 1;

    // create memo-record
    index = dmMaxRecordIndex;
    rec_h = DmNewRecord(memo_ref, &index, size);
    if (rec_h == 0)
    panic("fs_memo: can't create record");
    f->handle = index;

    rec_p = mem_lock(rec_h);
    if (rec_p == NULL)
    panic("fs_memo: can't lock record");
    d = DmWrite(rec_p, 0, buf, name_len);
    if (d != 0)
    panic("fs_memo: can't write new-record");
    mem_unlock(rec_h);
    DmReleaseRecord(memo_ref, index, 1);

    // create memo-tree entry
    memset(&node, 0, sizeof(struct memo_node));
    strcpy(node.name, f->name);
    node.pos = 0;
    dbt_write(memo_tree, index, &node, sizeof(struct memo_node));
    if (index >= memo_count)
    memo_count++;
  }
  // open existing file
  else {
    dbt_read(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
    node.pos = 0;
    dbt_write(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  }

  // create buffer 
  f->drv_data = tmp_alloc(MEMO_LIMIT + 1);
  memset(f->drv_data, 0, MEMO_LIMIT + 1);

  // load record
  rec_h = DmGetRecord(memo_ref, (long)f->handle);
  if (rec_h == 0)
  panic("fs_memo: can't get record");
  rec_p = mem_lock(rec_h);
  if (rec_p == NULL)
  panic("fs_memo: can't lock record");

  len = MemHandleSize(rec_h);

  // copy data (build filename)
  name_len = strlen(f->name) + 1;
  len -= name_len;
  if (len > 0) {
    rec_p += name_len;
    memcpy(f->drv_data, rec_p, MIN(len, MEMO_LIMIT));
  }

  // sets the file-pointer (open for 'append')
  if (f->open_flags & DEV_FILE_APPEND) {
    node.pos = strlen(f->drv_data);
    dbt_write(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  }

  // release record
  mem_unlock(rec_h);
  DmReleaseRecord(memo_ref, (long)f->handle, 0);
  return 1;
#else
  // /////////////////////////////////////////////////////////////////////////////////////////
  char tmp[OS_FILENAME_SIZE + 1];

strcpy  (tmp, f->name + 5);
#if	defined(_DOS)
  strcat(tmp, ".mem");
#else
  strcat(tmp, ".memo");
#endif
  strcpy(f->name, tmp);
  return stream_open(f);
#endif
}

/*
 */
int memo_close(dev_file_t * f) {
#if defined(_PalmOS)
  struct memo_node node;
  word index;
  int32 size, d;
  mem_t rec_h;
  byte *rec_p, *buf;
  dword src_sz;

  dbt_read(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));

  // flush data
  index = (long)f->handle;
  rec_h = DmGetRecord(memo_ref, index);
  if (rec_h == 0)
  panic("fs_memo: can't update record");
  size = strlen(node.name) + 1 + strlen(f->drv_data) + 2;
  src_sz = MemHandleSize(rec_h);
  if (src_sz != size)
  MemHandleResize(rec_h, size);

  // build final record
  buf = tmp_alloc(size);
  memset(buf, 0, size);
  strcpy(buf, node.name);
  strcat(buf, "\012");
  strcat(buf, f->drv_data);

  // write
  rec_p = mem_lock(rec_h);
  if (rec_p == NULL)
  panic("fs_memo: can't lock record");
  d = DmWrite(rec_p, 0, buf, size);
  if (d != 0)
  panic("fs_memo: can't write record");
  mem_unlock(rec_h);
  DmReleaseRecord(memo_ref, index, 1);

  // clean up
  tmp_free(buf);

  node.pos = 0;
  dbt_write(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));

  tmp_free(f->drv_data);
  f->drv_data = NULL;

  f->handle = -1;
  return 1;
#else
  return stream_close(f);
#endif
}

/*
 */
int memo_write(dev_file_t * f, byte * data, dword size) {
#if defined(_PalmOS)
  dword new_size;
  struct memo_node node;

  // read table
  dbt_read(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  new_size = node.pos + size;
  if (new_size >= MEMO_LIMIT) {
    err_memolimit();
    return 0;
  }

  // write
  memcpy(f->drv_data + node.pos, data, size);

  // update table
  node.pos += size;
  dbt_write(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  return 1;
#else
  return stream_write(f, data, size);
#endif
}

/*
 */
int memo_read(dev_file_t * f, byte * data, dword size) {
#if defined(_PalmOS)
  dword new_size;
  struct memo_node node;

  // read table
  dbt_read(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  new_size = node.pos + size;
  if (new_size >= MEMO_LIMIT)
  size = new_size - node.pos;

  // read
  memcpy(data, f->drv_data + node.pos, size);

  // update table
  node.pos += size;
  dbt_write(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  return 1;
#else
  return stream_read(f, data, size);
#endif
}

/*
 */
dword memo_length(dev_file_t * f) {
#if defined(_PalmOS)
  return strlen(f->drv_data);
#else
  return stream_length(f);
#endif
}

/*
 */
dword memo_eof(dev_file_t * f) {
#if defined(_PalmOS)
  struct memo_node node;

  dbt_read(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  if (node.pos >= strlen(f->drv_data))
  return 1;
  return 0;
#else
  return stream_eof(f);
#endif
}

/*
 */
dword memo_seek(dev_file_t * f, dword offset) {
#if defined(_PalmOS)
  struct memo_node node;

  dbt_read(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  if (node.pos + offset >= strlen(f->drv_data))
  return (dword) - 1;

  node.pos += offset;
  dbt_write(memo_tree, (long)f->handle, &node, sizeof(struct memo_node));
  return node.pos;
#else
  return stream_seek(f, offset);
#endif
}

/*
 */
int memo_delete(const char *name) {
#if defined(_PalmOS)
  word index = (word) memo_findfile(name + 5);
  if (index != (word) - 1) {
    struct memo_node node;

    strcpy(node.name, "$$$DELETED$$$");
    node.deleted = 1;
    dbt_write(memo_tree, index, &node, sizeof(struct memo_node));
    return (DmDeleteRecord(memo_ref, index) == errNone);
  }
  return 0;
#else
  char tmp[OS_FILENAME_SIZE + 1];

strcpy  (tmp, name + 5);
#if	defined(_DOS)
  strcat(tmp, ".mem");
#else
  strcat(tmp, ".memo");
#endif
#if defined(_VTOS)
  return (unlink(name) != 0);
#else
  return (remove(name) == 0);
#endif
#endif
}

/*
 *	returns true if the memo file exists
 */
int memo_exist(const char *name) {
#if defined(_PalmOS)
  return (memo_findfile(name + 5) != -1);
#else
  char tmp[OS_FILENAME_SIZE + 1];

strcpy  (tmp, name + 5);
#if	defined(_DOS)
  strcat(tmp, ".mem");
#else
  strcat(tmp, ".memo");
#endif
#if defined(_VTOS)
  return (fexists(name) != 0);
#else
  return (access(name, F_OK) == 0);
#endif
#endif
}

/*
 *	returns the access rights of the file
 */
int memo_access(const char *name) {
#if defined(_PalmOS)
  return 0666;
#else
  char tmp[OS_FILENAME_SIZE + 1];

strcpy  (tmp, name + 5);
#if	defined(_DOS)
  strcat(tmp, ".mem");
#else
  strcat(tmp, ".memo");
#endif
  return dev_faccess(tmp);
#endif
}

/*
 *	returns the attributes of the file
 */
int memo_fattr(const char *name) {
#if defined(_PalmOS)
  if (memo_exist(name))
  return 0666;
  return 0;
#else
  char tmp[OS_FILENAME_SIZE + 1];

strcpy  (tmp, name + 5);
#if	defined(_DOS)
  strcat(tmp, ".mem");
#else
  strcat(tmp, ".memo");
#endif
  return dev_fattr(tmp);
#endif
}

/*
 */
char_p_t *memo_create_file_list(const char *wc, int *count) {
#if defined(_PalmOS)
  struct memo_node node;
  char_p_t *list;
  int i;

  if (memo_count) {
    list = tmp_alloc(memo_count * sizeof(char_p_t));
    *count = 0;

    for (i = 0; i < memo_count; i++) {
      dbt_read(memo_tree, i, &node, sizeof(struct memo_node));

      if (node.deleted == 0) {
        if (wc_match(wc, node.name)) {
          list[*count] = (char *)tmp_alloc(strlen(node.name) + 1);
          strcpy(list[*count], node.name);
          *count = *count + 1;
        }                       // wc
      }
    }                           // for
    if (*count)
    return list;
    tmp_free(list);
    return NULL;
  }
  else {
    *count = 0;
    return NULL;
  }
#else
  char new_wc[OS_PATHNAME_SIZE + 1];char_p_t *list;
  int i;

  strcpy(new_wc, wc);
#if	defined(_DOS)
  strcat(new_wc, ".mem");
#else
  strcat(new_wc, ".memo");
#endif
  list = dev_create_file_list(new_wc, count);

  // remove suffix (".memo")
  for (i = 0; i < *count; i++) {
    char *p;

    p = strrchr(list[i], '.');
    if (p)
    *p = '\0';
  }
  return list;
#endif
}
