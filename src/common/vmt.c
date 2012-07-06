// This file is part of SmallBASIC
//
// Tables
//
// VMTs are an easy solution for limited memory.
// It is a system to support dynamic arrays on "disk".
//
// + Each VMT is a dynamic table of variable-length records
// + Every VMT can be stored on "disk" or on memory
// + If the a record does not exists then a new one will be created
//
// Details:
//
// There are two files for each table, the .dbt which holds the data and the .dbi which holds database info
//
// .dbi file structure
//  [vmt_t // vmt data structure] 
//  [vmt_rec_t 0 // vmt record structure]
//  ...
//  [vmt_rec_t N // vmt record structure]
//
// Note: multiuser not supported yet, but it is easy to be added (lock/unlock the header record)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/vmt.h"
#include "common/smbas.h"

#if defined(OS_LIMITED)
#define  MAX_VMT_FILES 16
#else
#define  MAX_VMT_FILES 256
#endif

static vmt_t vmt[MAX_VMT_FILES];  // table of VMTs

#define MEM_VMT_GROWSIZE     128
#define FILE_VMT_GROWSIZE    64
#define VMT_DEFRAG_SIZE      32768

/**
 * update dbt header 
 */
void dbt_file_write_header(dbt_t t) {
  lseek(vmt[t].i_handle, 0, SEEK_SET);
  int n = write(vmt[t].i_handle, &vmt[t], sizeof(vmt_t));
}

/**
 * read dbt header 
 */
void dbt_file_read_header(dbt_t t) {
  lseek(vmt[t].i_handle, 0, SEEK_SET);
  int n = read(vmt[t].i_handle, &vmt[t], sizeof(vmt_t));
}

/**
 * read index-record
 */
void dbt_file_read_rec_t(dbt_t t, int index, vmt_rec_t * rec) {
  lseek(vmt[t].i_handle, sizeof(vmt_rec_t) * index + sizeof(vmt_t), SEEK_SET);
  int n = read(vmt[t].i_handle, rec, sizeof(vmt_rec_t));
}

/**
 * write index-record
 */
void dbt_file_write_rec_t(dbt_t t, int index, vmt_rec_t * rec) {
  lseek(vmt[t].i_handle, sizeof(vmt_rec_t) * index + sizeof(vmt_t), SEEK_SET);
  int n = write(vmt[t].i_handle, rec, sizeof(vmt_rec_t));
}

/**
 * read record data
 */
void dbt_file_read_data(dbt_t t, int index, char *data, int size) {
  vmt_rec_t rec;

  lseek(vmt[t].i_handle, sizeof(vmt_rec_t) * index + sizeof(vmt_t), SEEK_SET);
  int n = read(vmt[t].i_handle, &rec, sizeof(vmt_rec_t));
  lseek(vmt[t].f_handle, rec.offset, SEEK_SET);
  n = read(vmt[t].f_handle, data, size);

//      printf("\nvmt %d read: %d, %d\n", t, index, size);
//      hex_dump(data, size);
}

/**
 * write record data
 */
void dbt_file_write_data(dbt_t t, int index, char *data, int size) {
  vmt_rec_t rec;
  int n;

//      printf("\nvmt %d write: %d, %d\n", t, index, size);
//      hex_dump(data, size);

  lseek(vmt[t].i_handle, sizeof(vmt_rec_t) * index + sizeof(vmt_t), SEEK_SET);
  n = read(vmt[t].i_handle, &rec, sizeof(vmt_rec_t));
  lseek(vmt[t].i_handle, sizeof(vmt_rec_t) * index + sizeof(vmt_t), SEEK_SET);

  if (rec.size >= size) {
    // it is fits on that space
    rec.size = size;
    n = write(vmt[t].i_handle, &rec, sizeof(vmt_rec_t));

    lseek(vmt[t].f_handle, rec.offset, SEEK_SET);
//              printf("\noffset=%d\n", rec.offset);
    n = write(vmt[t].f_handle, data, size);
  } else {
    // new allocation required
    rec.offset = lseek(vmt[t].f_handle, 0, SEEK_END);
    rec.size = size;
    n = write(vmt[t].i_handle, &rec, sizeof(vmt_rec_t));
//              printf("\noffset(new)=%d\n", rec.offset);

//              lseek(vmt[t].f_handle, rec.offset, SEEK_SET);   // not needed
    n = write(vmt[t].f_handle, data, size);
  }
}

/**
 * allocates additional space on database for 'recs' records of 'recsize' size
 */
int dbt_file_append(dbt_t t, int recs, int recsize) {
  vmt_rec_t rec;
  int i;
  int n;

  rec.ver = 1;
  rec.size = recsize;

  // create empty records
  if (vmt[t].count + recs >= vmt[t].size) {
    int newsize;
    char *data;

    data = tmp_alloc(recsize);
    memset(data, 0, recsize);
    newsize = vmt[t].size + recs;

    for (i = vmt[t].size; i < newsize; i++) {
      lseek(vmt[t].i_handle, sizeof(vmt_rec_t) * i + sizeof(vmt_t), SEEK_SET);
      rec.offset = lseek(vmt[t].f_handle, 0, SEEK_END);
      n = write(vmt[t].i_handle, &rec, sizeof(vmt_rec_t));
      n = write(vmt[t].f_handle, data, recsize);
    }

    tmp_free(data);
    vmt[t].size = newsize;
  }

  vmt[t].count++;

  // 
  dbt_file_write_header(t);
  return vmt[t].count - 1;
}

/**
 * removes deleted chuncks (defrag)
 */
void dbt_file_pack(dbt_t t) {
  vmt_rec_t rec;
  int i, idx_offset, new_h, new_offset, n;
  char *data;
  char old_db_name[OS_PATHNAME_SIZE];
  char new_db_name[OS_PATHNAME_SIZE];

  sprintf(old_db_name, "%s.dbt", vmt[t].base);
  sprintf(new_db_name, "%s-new.dbt", vmt[t].base);
  new_h = open(new_db_name, O_BINARY | O_RDWR);
  for (i = 0; i < vmt[t].size; i++) {
    // read original record data
    idx_offset = i * sizeof(vmt_rec_t) + sizeof(vmt_t);
    lseek(vmt[t].i_handle, idx_offset, SEEK_SET);

    // read data
    data = tmp_alloc(rec.size);
    lseek(vmt[t].f_handle, rec.offset, rec.size);
    n = read(vmt[t].f_handle, data, rec.size);

    // copy record
    new_offset = lseek(new_h, 0, SEEK_END);
    rec.offset = new_offset;
    n = write(new_h, data, rec.size);
    tmp_free(data);

    // update index
    n = write(vmt[t].i_handle, &rec, sizeof(vmt_rec_t));
  }

  // swap databases and reopen
  close(vmt[t].f_handle);
  remove(old_db_name);
  close(new_h);
  rename(new_db_name, old_db_name);
  vmt[t].f_handle = open(old_db_name, O_BINARY | O_RDWR);
}

/**
 * create/open a table
 *
 * b0 = ON for non-memory file (file is created by the user, so, it must exists in disk)
 * b1 = ON for create always (truncate if it is exists); FALSE for open if exists or create if it does not exists
 */
dbt_t dbt_create(const char *fileName, int flags) {
  int i, t = -1;

  if (opt_usevmt) {
    flags |= 1;
  }

  // find a free VMT
  for (i = 0; i < MAX_VMT_FILES; i++) {
    if (vmt[i].used == 0) {
      t = i;
      vmt[t].sign = (intptr_t) "VMTH";
      vmt[t].ver = 1;
      vmt[t].flags = flags;
      vmt[t].count = 0;
      vmt[t].size = 0;
      break;
    }
  }
  if (t == -1)                  // no vmt handle free
    return -1;

  // create the table
  if (flags & 1) {
    // VMT: use file
    char fullfile[OS_PATHNAME_SIZE];
    int f_handle, i_handle;

    // open database
    strcpy(vmt[t].base, fileName);
    sprintf(fullfile, "%s.dbt", vmt[t].base);
    vmt[t].f_handle = f_handle = open(fullfile, O_RDWR | O_BINARY);

    if ((flags & 0x2) || vmt[t].f_handle == -1) { // create always
      vmt[t].f_handle = f_handle = open(fullfile, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0660);

      // create index
      sprintf(fullfile, "%s.dbi", vmt[t].base);
      vmt[t].i_handle = i_handle = open(fullfile, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0660);
    } else {
      // open index
      sprintf(fullfile, "%s.dbi", vmt[t].base);
      vmt[t].i_handle = i_handle = open(fullfile, O_RDWR | O_BINARY);
      dbt_file_read_header(t);
    }

    vmt[t].f_handle = f_handle;
    vmt[t].i_handle = i_handle;
    vmt[t].used = 1;

    if (flags & 4)
      vmt[t].count = 0;
    dbt_file_write_header(t);
  } else {
    // VMT: use memory
    int i;

    // preallocate some records
    vmt[t].size = MEM_VMT_GROWSIZE;
    vmt[t].m_handle = mem_alloc(sizeof(mem_t) * vmt[t].size);

    // reset new data
    vmt[t].m_table = (mem_t *)mem_lock(vmt[t].m_handle);
    for (i = 0; i < vmt[t].size; i++) {
      vmt[t].m_table[i] = 0;    // make them NULL
    }

    // 
    vmt[t].used = 1;
  }

  return t;                     // return vmt-handle
}

/**
 * Close table (warning: file does not deleted)
 */
void dbt_close(dbt_t t) {
  // close the file (if exists)
  if (vmt[t].flags & 1) {
    // VMT: use file
    long dblen;

    dblen = lseek(vmt[t].f_handle, 0, SEEK_END);
    close(vmt[t].f_handle);
    close(vmt[t].i_handle);
    vmt[t].used = 0;

    if (dblen > VMT_DEFRAG_SIZE) { // too big, defrag it
      dbt_file_pack(t);
    }
  } else {
    // VMT: use memory
    int i;

    // free records
    for (i = 0; i < vmt[t].size; i++) {
      if (vmt[t].m_table[i]) {
        mem_free(vmt[t].m_table[i]);
      }
    }

    // free table
    mem_unlock(vmt[t].m_handle);mem_free(vmt[t].m_handle);
    vmt[t].used = 0;
  }
}

/**
 * Allocate a standard size (use it at startup; good speed optimization)
 */
void dbt_prealloc(dbt_t t, int num, int recsize) {
  if (vmt[t].flags & 1) {
    // use file
    int newsize;

    newsize = num - vmt[t].size;
    if (newsize > 0) {
      dbt_file_append(t, newsize, recsize);
    }
  } else {
    // use memory
    int i;

    if (vmt[t].size < num) {
      mem_unlock(vmt[t].m_handle);
      vmt[t].m_handle = mem_realloc(vmt[t].m_handle, sizeof(mem_t) * num);

      vmt[t].m_table = (mem_t *)mem_lock(vmt[t].m_handle);
      for (i = vmt[t].size; i < num; i++) {
        vmt[t].m_table[i] = mem_alloc(recsize);
      }
      vmt[t].size = num;
    }
  }
}

/**
 * Store an element
 */
void dbt_write(dbt_t t, int index, void *ptr, int size) {
  if (vmt[t].flags & 1) {
    // use file
    if (vmt[t].size <= index)
      dbt_file_append(t, (index - vmt[t].count) + FILE_VMT_GROWSIZE, size);
    dbt_file_write_data(t, index, ptr, size);
  } else {
    // use memory
    int i;
    char *mp;

    // resize needed
    if (vmt[t].size <= index) {
      mem_unlock(vmt[t].m_handle);
      vmt[t].m_handle = mem_realloc(vmt[t].m_handle,
                                    sizeof(mem_t) * (vmt[t].size + MEM_VMT_GROWSIZE));

      vmt[t].m_table = (mem_t *)mem_lock(vmt[t].m_handle);
      for (i = vmt[t].size; i < vmt[t].size + MEM_VMT_GROWSIZE; i++) {
        vmt[t].m_table[i] = 0;
      }
      vmt[t].size += MEM_VMT_GROWSIZE;
    }

    // store data
    if (vmt[t].m_table[index] == 0) { // empty record
      vmt[t].m_table[index] = mem_alloc(size);
    } else if (size != mem_handle_size(vmt[t].m_table[index])) {  // resize
      // record
      mem_free(vmt[t].m_table[index]);
      vmt[t].m_table[index] = mem_alloc(size);
    }

    mp = mem_lock(vmt[t].m_table[index]);
    memcpy(mp, ptr, size);
    mem_unlock(vmt[t].m_table[index]);
  }

  // update counter
  if (vmt[t].count <= index) {
    vmt[t].count = index + 1;
    if (vmt[t].flags & 1) {
      dbt_file_write_header(t);
    }
  }
}

/**
 * Load an element
 */
void dbt_read(dbt_t t, int index, void *ptr, int size) {
  if (vmt[t].flags & 1) {
    // use file
    if (vmt[t].size <= index) {
      dbt_file_append(t, (index - vmt[t].count) + FILE_VMT_GROWSIZE, size);
    }
    dbt_file_read_data(t, index, ptr, size);
  } else {
    // use memory
    int i;
    char *mp;

    // resize needed
    if (vmt[t].size <= index) {
      mem_unlock(vmt[t].m_handle);
      vmt[t].m_handle = mem_realloc(vmt[t].m_handle,
                                    sizeof(mem_t) * (vmt[t].size + MEM_VMT_GROWSIZE));

      vmt[t].m_table = (mem_t *)mem_lock(vmt[t].m_handle);
      for (i = vmt[t].size; i < vmt[t].size + MEM_VMT_GROWSIZE; i++) {
        vmt[t].m_table[i] = 0;
      }
      vmt[t].size += MEM_VMT_GROWSIZE;
    }

    // get the data
    if (vmt[t].m_table[index] == 0) {
      vmt[t].m_table[index] = mem_alloc(size);
    } else if (size > mem_handle_size(vmt[t].m_table[index])) {
      vmt[t].m_table[index] = mem_realloc(vmt[t].m_table[index], size);
    }

    // copy data
    mp = mem_lock(vmt[t].m_table[index]);
    memcpy(ptr, mp, size);
    mem_unlock(vmt[t].m_table[index]);
  }
}

/**
 * return the number of records in VMT
 */
int dbt_count(dbt_t t) {
  return vmt[t].count;
}

/**
 * remove a record
 */
void dbt_remove(dbt_t t, int index) {
  // ignore it for now
}

/**
 * Using VMT's with keys (like environment-variables)
 *
 * sets a variable
 *
 * if varvalue == NULL, the variable will be deleted
 */
int dbt_setvar(dbt_t fh, const char *varname, const char *varvalue) {
  char *buf, *newrec = NULL;
  int i, idx;
  dbt_var_t nd, new_nd;

  if (varvalue) {
    // create the new header
    new_nd.sign = dbt_var_sign;
    new_nd.var_len = strlen(varname) + 1;
    new_nd.val_len = strlen(varvalue) + 1;
    new_nd.node_len = sizeof(new_nd) + new_nd.var_len + new_nd.val_len;

    // create record
    newrec = tmp_alloc(new_nd.node_len);
    memcpy(newrec, &new_nd, sizeof(new_nd));
    memcpy(newrec + sizeof(new_nd), varname, new_nd.var_len);
    memcpy(newrec + sizeof(new_nd) + new_nd.var_len, varvalue, new_nd.val_len);
  }

  // default index (new record)
  idx = dbt_count(fh);

  // find if already exists
  for (i = 0; i < dbt_count(fh); i++) {
    char *nd_var, *nd_val;

    // load the record
    dbt_read(fh, i, &nd, sizeof(nd));
    if (nd.sign == dbt_var_sign) {
      buf = tmp_alloc(nd.node_len);
      dbt_read(fh, i, buf, nd.node_len);
      nd_var = buf + sizeof(nd);
      nd_val = buf + sizeof(nd) + nd.var_len;

      // check varname
      if (strcmp(nd_var, varname) == 0) {
        idx = i;
        tmp_free(buf);
        break;
      }

      tmp_free(buf);
    }
  }

  if (varvalue) {
    // store the record
    dbt_write(fh, idx, newrec, new_nd.node_len);
    tmp_free(newrec);
  } else
    // delete the record
    dbt_remove(fh, idx);
  return 0;
}

/**
 * Using VMT's with keys (like environment-variables)
 *
 * gets a variable's value
 * if variable not found, returns NULL otherwise returns a newly created string with the value
 */
char *dbt_getvar(dbt_t fh, const char *varname) {
  char *buf, *retval = NULL;
  int i;
  dbt_var_t nd;

  // find if already exists
  for (i = 0; i < dbt_count(fh); i++) {
    char *nd_var, *nd_val;

    // load the record
    dbt_read(fh, i, &nd, sizeof(nd));
    if (nd.sign == dbt_var_sign) {
      buf = tmp_alloc(nd.node_len);
      dbt_read(fh, i, buf, nd.node_len);
      nd_var = buf + sizeof(nd);
      nd_val = buf + sizeof(nd) + nd.var_len;

      // check varname
      if (strcmp(nd_var, varname) == 0) {
        retval = tmp_strdup(nd_val);
        tmp_free(buf);
        break;
      }

      tmp_free(buf);
    }
  }

  return retval;
}

/**
 * return the size of the record
 */
int dbt_recsize(dbt_t t, int index) {
  int size;

  if (vmt[t].flags & 1) {
    // file
    vmt_rec_t rec;

    dbt_file_read_rec_t(t, index, &rec);
    size = rec.size;
  } else {
    // memory
    if (vmt[t].m_table[index])
      size = mem_handle_size(vmt[t].m_table[index]);
    else
      size = 0;
  }

  return size;
}
