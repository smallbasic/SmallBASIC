// This file is part of SmallBASIC
//
// PalmOS MEMO/DB driver
// On non-PalmOS systems, names are created with prefix "memo_"
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_sbfs_memo_h)
#define _sbfs_memo_h

#include "common/sys.h"
#include "common/device.h"

int memo_mount(void);
int memo_umount(void);
int memo_open(dev_file_t *f);
int memo_close(dev_file_t *f);
int memo_write(dev_file_t *f, byte *data, dword size);
int memo_read(dev_file_t *f, byte *data, dword size);
dword memo_length(dev_file_t *f);
dword memo_eof(dev_file_t *f);
int memo_delete(const char *name);
int memo_exist(const char *name);
dword memo_seek(dev_file_t *f, dword offset);
char_p_t *memo_create_file_list(const char *wc, int *count);
int memo_access(const char *name);
int memo_fattr(const char *name);

#endif
