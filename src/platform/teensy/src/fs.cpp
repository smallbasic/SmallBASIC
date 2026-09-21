// This file is part of SmallBASIC
//
// Copyright(C) 2026 Joerg Siebenmorgen
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <Arduino.h>
#include "common/device.h"
#include "common/pproc.h"
#include "config.h"
#include "languages/messages.en.h"
#include "include/var_map.h"
#include "module.h"
#include <SD.h>
#include <LittleFS.h>
#include <vector>

extern LittleFS_Program flashfs;

std::vector<File> fileStack;

/*
 * close file
 */
static int cmd_close(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  fileStack[self->v.m.id].flush();
  fileStack[self->v.m.id].close();
  return 1;
}

/*
 * if current file is a directory, getNextFilename() returns
 * the next filename in the directory
 */
static int cmd_getNextFilename(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  File file = fileStack[self->v.m.id].openNextFile();
  if (!file) {
    v_setint(retval, 0);
    return 1;
  }

  v_setstr(retval, file.name());

  file.close();
  return 1;
}

/*
 * check if file is a directory
 * isDirectory() returns true or false
 */
static int cmd_isDirectory(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  v_setint(retval, fileStack[self->v.m.id].isDirectory());
  return 1;
}

/*
 * return file size
 */
static int cmd_size(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  v_setint(retval, fileStack[self->v.m.id].size());
  return 1;
}

/*
 * return current read/write position
 */
static int cmd_position(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  v_setint(retval, fileStack[self->v.m.id].position());
  return 1;
}

/*
 * set read/write position
 * seek(pos)  -> 'pos' psition in file
 */
static int cmd_seek(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  uint64_t pos = get_param_int(argc, args, 0, 0);
  fileStack[self->v.m.id].seek(pos);
  return 1;
}

/*
 * read from file
 * 1. read() or read(0) -> read until '\n' and return as string
 * 2. read(1)           -> read one byte and return as integer
 * 2. read(n)           -> read n bytes and return as integer array
 */
static int cmd_read(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  uint32_t bytes = get_param_int(argc, args, 0, 0);
  int size;

  if (bytes == 0) {
    // Read until '\n' and return as string
    bytes = CDC_RX_SIZE_480;
    char buffer[bytes];
    size = fileStack[self->v.m.id].readBytesUntil('\n', buffer, bytes - 1);
    buffer[size] = '\0';
    v_setstr(retval, buffer);
  } else if (bytes > 1) {
    // Read number of bytes and return as array
    v_toarray1(retval, bytes);
    for (uint32_t ii = 0; ii < bytes; ii++) {
      v_setint(v_elem(retval, ii), fileStack[self->v.m.id].read());
    }
  } else {
    v_setint(retval, fileStack[self->v.m.id].read());
  }
  return 1;
}

/*
 * write to file
 * wrtie(buffer) -> 'buffer' can be integer, string or array
 */
static int cmd_write(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  switch (args[0].var_p->type) {
  case V_INT:{
      uint8_t value = get_param_int(argc, args, 0, 0);
      fileStack[self->v.m.id].write(value);
    }
    break;
  case V_STR:{
      const char *buffer = get_param_str(argc, args, 0, "");
      fileStack[self->v.m.id].print(buffer);
    }
    break;
  case V_ARRAY:{
      var_p_t array = args[1].var_p;    // Get array
      if (array->maxdim > 1) {
        v_setstr(retval, "ERROR: FS: Write requires 1D-array");
        return 0;
      }
      uint32_t bytes = v_ubound(array, 0) - v_lbound(array, 0) + 1;
      uint8_t *buffer = new uint8_t[bytes];
      for (uint32_t ii = 0; ii < bytes; ii++) {
        buffer[ii] = get_array_elem_int(array, ii);
      }
      fileStack[self->v.m.id].write(buffer, bytes);
      delete[]buffer;
    }
    break;
  }

  return 1;
}

/*
 * flush buffer and sync file
 */
static int cmd_flush(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  fileStack[self->v.m.id].flush();
  return 1;
}

/*
 * convert current file, which might contain data, to an empty file
 */
static int cmd_truncate(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  fileStack[self->v.m.id].truncate();
  fileStack[self->v.m.id].seek(0);
  return 1;
}

/*
 * returns bytes available for reading
 */
static int cmd_available(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  v_setint(retval, fileStack[self->v.m.id].available());
  return 1;
}

/*
 * open a file or directory
 * open(name, mode) -> open file 'name' with rwmode 'mode'; mode = 0 -> read; mode = 1 -> write
 * open() returns a map
 */
static int cmd_open(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  const char *name = get_param_str(argc, args, 0, "flash:");
  uint8_t mode =  get_param_int(argc, args, 1, FILE_READ);

  File file;
  char *namePtr;

  if (strncmp(name, "flash:", 6) == 0) {
    namePtr = (char *)name + 6;
    file = flashfs.open(namePtr, mode);
  } else if (strncmp(name, "sd:", 3) == 0) {
    // root directory of SD must be "/", "" will crash
    if (strlen(name) == 3) {
      file = SD.open("/", mode);
    } else {
      namePtr = (char *)name + 3;
      file = SD.open(namePtr, mode);
    }
  } else {
    file = flashfs.open(name, mode);
  }

  if (!file) {
    v_setint(retval, 0);
    return 1;
  }

  fileStack.push_back(file);

  map_init(retval);
  retval->v.m.id = fileStack.size() - 1;
  if (file.isDirectory()) {
    v_create_callback(retval, "getNextFilename", cmd_getNextFilename);
  } else {
    if (mode == FILE_READ) {
      v_create_callback(retval, "read", cmd_read);
      v_create_callback(retval, "available", cmd_available);
    } else if (mode == FILE_WRITE) {
      v_create_callback(retval, "write", cmd_write);
      v_create_callback(retval, "truncate", cmd_truncate);
      v_create_callback(retval, "flush", cmd_flush);
    }
    v_create_callback(retval, "size", cmd_size);
    v_create_callback(retval, "position", cmd_position);
    v_create_callback(retval, "seek", cmd_seek);
  }
  v_create_callback(retval, "isDirectory", cmd_isDirectory);
  v_create_callback(retval, "close", cmd_close);

  return 1;
}

/*
 * create directory
 * mkdir(name) -> 'name' directory name
 */

static int cmd_mkdir(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  const char *name = get_param_str(argc, args, 0, "");
  char *namePtr;

  if (strncmp(name, "flash:", 6) == 0) {
    namePtr = (char *)name + 6;
    v_setint(retval, flashfs.mkdir(namePtr));
    return 1;
  }
  if (strncmp(name, "sd:", 3) == 0) {
    namePtr = (char *)name + 3;
    v_setint(retval, SD.mkdir(namePtr));
    return 1;
  }

  v_setint(retval, flashfs.mkdir(name));
  return 1;
}

/*
 * Quickformate flash memory
 */
static int cmd_quickFormat(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  v_setint(retval, flashfs.quickFormat());
  return 1;
}

/*
 * check if file or directory exists
 * returns true or false
 */
static int cmd_exists(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  const char *name = get_param_str(argc, args, 0, "");
  char *namePtr;

  if (strncmp(name, "flash:", 6) == 0) {
    namePtr = (char *)name + 6;
    v_setint(retval, flashfs.exists(namePtr));
    return 1;
  }
  if (strncmp(name, "sd:", 3) == 0) {
    namePtr = (char *)name + 3;
    v_setint(retval, SD.exists(namePtr));
    return 1;
  }

  v_setint(retval, flashfs.exists(name));
  return 1;
}

/*
 * rename a file or directory
 * rename(name_old, name_new) -> 'name_old' current name; 'name_new' new name
 */
static int cmd_rename(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  const char *nameOld = get_param_str(argc, args, 0, "");
  const char *nameNew = get_param_str(argc, args, 1, "");
  char *nameOldPtr;
  char *nameNewPtr;

  if (strncmp(nameOld, "flash:", 6) == 0 && strncmp(nameNew, "flash:", 6) == 0) {
    nameOldPtr = (char *)nameOld + 6;
    nameNewPtr = (char *)nameNew + 6;
    v_setint(retval, flashfs.rename(nameOldPtr, nameNewPtr));
    return 1;
  }
  if (strncmp(nameOld, "sd:", 3) == 0 && strncmp(nameNew, "sd:", 3) == 0) {
    nameOldPtr = (char *)nameOld + 3;
    nameNewPtr = (char *)nameNew + 3;
    v_setint(retval, SD.rename(nameOldPtr, nameNewPtr));
    return 1;
  }

  v_setint(retval, flashfs.rename(nameOld, nameNew));
  return 1;
}

/*
 * remove file or directory
 * remove(name) -> 'name' file or directory
 */
static int cmd_remove(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  const char *name = get_param_str(argc, args, 0, "");
  char *namePtr;

  if (strncmp(name, "flash:", 6) == 0) {
    namePtr = (char *)name + 6;
    v_setint(retval, flashfs.remove(namePtr));
    return 1;
  }
  if (strncmp(name, "sd:", 3) == 0) {
    namePtr = (char *)name + 3;
    v_setint(retval, SD.remove(namePtr));
    return 1;
  }

  v_setint(retval, flashfs.remove(name));
  return 1;
}

/*
 * show file system usage
 * returns an array [FlashUsedSize, FlashTotalSize, SDUsedSize, SDTotalSize]
 */
static int cmd_free(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  v_toarray1(retval, 4);
  v_setint(v_elem(retval, 0), flashfs.usedSize());
  v_setint(v_elem(retval, 1), flashfs.totalSize());
  v_setint(v_elem(retval, 2), SD.usedSize());
  v_setint(v_elem(retval, 3), SD.totalSize());
  return 1;
}

int cmd_fs(int argc, slib_par_t *args, var_t *retval) {
  map_init(retval);
  v_create_callback(retval, "OPEN", cmd_open);
  v_create_callback(retval, "QUICKFORMAT", cmd_quickFormat);
  v_create_callback(retval, "EXISTS", cmd_exists);
  v_create_callback(retval, "MKDIR", cmd_mkdir);
  v_create_callback(retval, "RENAME", cmd_rename);
  v_create_callback(retval, "REMOVE", cmd_remove);
  v_create_callback(retval, "FREE", cmd_free);

  return 1;
}

int fs_close(void) {
  for(File& f : fileStack) {
    if (f) {
      f.close();
    }
  }
  return 1;
}
