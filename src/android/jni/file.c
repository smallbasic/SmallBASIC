// $Id var_hash.c 783 2010-03-21 122127Z zeeb90au $
// This file is part of SmallBASIC
//
// Unsupported functionality due to platform limitations
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2010 Chris Warren-Smith. [http//tinyurl.com/ja2ss]

#include "sys.h"
#include "device.h"
#include "fs_stream.h"

int stream_open(dev_file_t * f) {
  return 0;
}

int stream_close(dev_file_t * f) {
  return 0;
}

int stream_write(dev_file_t * f, byte * data, dword size) {
  return 0;
}

int stream_read(dev_file_t * f, byte * data, dword size) {
  return 0;
}

dword stream_tell(dev_file_t * f) {
  return 0;
}

dword stream_length(dev_file_t * f) {
  return 0;
}

dword stream_seek(dev_file_t * f, dword offset) {
  return 0;
}

int stream_eof(dev_file_t * f) {
  return 0;
}


