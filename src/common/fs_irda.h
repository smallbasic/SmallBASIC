// This file is part of SmallBASIC
//
// IrDA I/O, driver
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_sbfs_irda_h)
#define _sbfs_irda_h

#include "common/sys.h"
#include "common/device.h"

int irda_open(dev_file_t * f) SEC(BIO);
int irda_close(dev_file_t * f) SEC(BIO);
int irda_write(dev_file_t * f, byte * data, dword size) SEC(BIO);
int irda_read(dev_file_t * f, byte * data, dword size) SEC(BIO);
dword irda_length(dev_file_t * f) SEC(BIO);
dword irda_eof(dev_file_t * f) SEC(BIO);

#endif
