// This file is part of SmallBASIC
//
// serial I/O, driver
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_sbfs_serial_h)
#define _sbfs_serial_h

#include "common/sys.h"
#include "common/device.h"

int serial_open(dev_file_t *f);
int serial_close(dev_file_t *f);
int serial_write(dev_file_t *f, byte *data, dword size);
int serial_read(dev_file_t *f, byte *data, dword size);
dword serial_length(dev_file_t *f);
dword serial_eof(dev_file_t *f);

#endif
