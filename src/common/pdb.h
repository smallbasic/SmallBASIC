// This file is part of SmallBASIC
//
// PDB for Non-PalmOS
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup utils Uitilies
 */

#if !defined(_pdb_h)
#define _pdb_h

#include "common/sys.h"

/**
 * @ingroup utils
 * @struct pdb_record_entry
 * PDB record
 */
struct pdb_record_entry {
  dword localChunkID; /* offset to where record starts */
  struct {
    int delete:1;
    int dirty :1;
    int busy :1;
    int secret :1;
    int category :4;
  } attributes;byte uniqueID[3];
};

typedef struct pdb_record_entry pdb_record_entry_t;

#define PDB_RECORD_ENTRY_SIZE   8

/**
 * @ingroup utils
 * @struct pdb_record_list
 * PDB record
 */
struct pdb_record_list {
  dword next_record_list_id;
  word num_records;
};
typedef struct pdb_record_list pdb_record_list_t;

#define PDB_RECORD_LIST_SIZE    6

/**
 * @ingroup utils
 * @struct pdb_database_hdr
 * PDB file header
 */
struct pdb_database_hdr {
  char name[32];
  word attributes;
  word version;
  dword creation_date;
  dword modification_date;
  dword last_backup_date;
  dword modification_number;
  dword app_info_id;
  dword sort_info_id;
  char type[4];
  char creator[4];
  dword unique_id_seed;
  pdb_record_list_t record_list;
};
typedef struct pdb_database_hdr pdb_database_hdr_t;

#define PDB_DATABASE_HDR_SIZE   78

#endif
