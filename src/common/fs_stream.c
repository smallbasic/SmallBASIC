// This file is part of SmallBASIC
//
// SmallBASIC streams (normal files), driver
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/device.h"
#include "common/pproc.h"

#include <errno.h>

#if defined(_UnixOS)
#include <sys/time.h>
#include <unistd.h>
#endif
#include <dirent.h>

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

#include "common/fs_stream.h"

/*
 * open a file
 */
int stream_open(dev_file_t *f) {
  int osflags, osshare;

  if (f->open_flags == DEV_FILE_OUTPUT) {
    remove(f->name);
  }

  if (f->open_flags & DEV_FILE_EXCL) {
    osshare = 0;
  } else {
    osshare = S_IRUSR;
  }

  // take care not to set any write flags when simply reading a file.
  // the file may be open in another program (such as excel) which has
  // a write lock on the file causing the bas program to needlessly fail.
  osflags = (O_RDONLY | O_BINARY);

  if (f->open_flags & DEV_FILE_OUTPUT) {
    osflags |= (O_CREAT | O_WRONLY);
    osshare |= S_IWUSR;
  }
  if (f->open_flags & DEV_FILE_APPEND) {
    osflags |= (O_CREAT | O_APPEND | O_WRONLY);
    osshare |= S_IWUSR;
  }

#if defined(_UnixOS)
  if (strcmp(f->name, "SDIN:") == 0) {
    f->handle = 0;
  }
  else if (strcmp(f->name, "SOUT:") == 0) {
    f->handle = 1;
  }
  else if (strcmp(f->name, "SERR:") == 0) {
    f->handle = 2;
  }
  else {
    f->handle = open(f->name, osflags, osshare);
    if (f->handle < 0 && gsb_bas_dir[0]) {
      char file[OS_PATHNAME_SIZE];
      strcpy(file, gsb_bas_dir);
      strcat(file, f->name);
      f->handle = open(file, osflags, osshare);
    }
  }
#else
  f->handle = open(f->name, osflags);
#endif

  if (f->handle < 0) {
    err_file((f->last_error = errno));
  }
  return (f->handle >= 0);
}

/*
 *   close the stream
 */
int stream_close(dev_file_t *f) {
  int r;

  r = close(f->handle);
  f->handle = -1;
  if (r) {
    err_file((f->last_error = errno));
  }
  return (r == 0);
}

/*
 */
int stream_write(dev_file_t *f, byte *data, uint32_t size) {
  int r;

  r = write(f->handle, data, size);
  if (r != (int) size) {
    err_file((f->last_error = errno));
  }
  return (r == (int) size);
}

/*
 */
int stream_read(dev_file_t *f, byte *data, uint32_t size) {
  int r;

  r = read(f->handle, data, size);
  if (r != (int) size) {
    err_file((f->last_error = errno));
  }
  return (r == (int) size);
}

/*
 * returns the current position
 */
uint32_t stream_tell(dev_file_t *f) {
  return lseek(f->handle, 0, SEEK_CUR);
}

/*
 * returns the file-length
 */
uint32_t stream_length(dev_file_t *f) {
  long pos, endpos;

  pos = lseek(f->handle, 0, SEEK_CUR);
  if (pos != -1) {
    endpos = lseek(f->handle, 0, SEEK_END);
    lseek(f->handle, pos, SEEK_SET);
    return endpos;
  } else {
    err_file((f->last_error = errno));
  }
  return 0;
}

/*
 */
uint32_t stream_seek(dev_file_t *f, uint32_t offset) {
  return lseek(f->handle, offset, SEEK_SET);
}

/*
 */
int stream_eof(dev_file_t *f) {
  long pos, endpos;

  pos = lseek(f->handle, 0, SEEK_CUR);
  if (pos != -1) {
    endpos = lseek(f->handle, 0, SEEK_END);
    lseek(f->handle, pos, SEEK_SET);
    return (pos == endpos);
  } else {
    err_file((f->last_error = errno));
  }
  return 1;
}
