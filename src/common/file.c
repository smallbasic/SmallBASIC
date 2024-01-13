// This file is part of SmallBASIC
//
// SmallBASIC - file.c - Low-level file system support
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/device.h"
#include "common/pproc.h"
#include "common/messages.h"

#include <errno.h>
#include <dirent.h>

#if USE_TERM_IO
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#endif

// drivers
#include "common/fs_stream.h"
#include "common/fs_serial.h"
#include "common/fs_socket_client.h"
#include "lib/match.h"

// FILE TABLE
static dev_file_t file_table[OS_FILEHANDLES];

/**
 * Basic wild-cards
 */
int wc_match(const char *mask, char *name) {
  if (mask == NULL) {
    return 1;
  }
  if (*mask == '\0') {
    return 1;
  }
  if (strcmp(mask, "*") == 0) {
    return 1;
  }
  return (reg_match(mask, name) == 0);
}

/**
 * initialize file system
 */
int dev_initfs() {
  for (int i = 0; i < OS_FILEHANDLES; i++) {
    file_table[i].handle = -1;
  }

  return 1;
}

/**
 * cleanup file system
 */
void dev_closefs() {
  for (int i = 0; i < OS_FILEHANDLES; i++) {
    if (file_table[i].handle != -1) {
      dev_fclose(i + 1);
    }
  }
}

/**
 * returns a free file handle for user's commands
 */
int dev_freefilehandle() {
  for (int i = 0; i < OS_FILEHANDLES; i++) {
    if (file_table[i].handle == -1) {
      // Note: BASIC's handles starting from 1
      return i + 1;
    }
  }

  rt_raise(FSERR_TOO_MANY_FILES);
  return -1;
}

/**
 * returns a file pointer for the given BASIC handle
 */
dev_file_t *dev_getfileptr(const int handle) {
  dev_file_t *result;
  // BASIC handles start from 1
  int hnd = handle - 1;
  if (hnd < 0 || hnd >= OS_FILEHANDLES) {
    rt_raise(FSERR_HANDLE);
    result = NULL;
  } else {
    result = &file_table[hnd];
  }
  return result;
}

/**
 * returns true if the file is opened
 */
int dev_fstatus(int handle) {
  dev_file_t *f;

  if ((f = dev_getfileptr(handle)) == NULL) {
    return 0;
  }

  return (f->handle != -1);
}

/**
 * opens a file
 *
 * returns true on success
 */
int dev_fopen(int sb_handle, const char *name, int flags) {
  dev_file_t *f;

  if ((f = dev_getfileptr(sb_handle)) == NULL) {
    return 0;
  }

  memset(f, 0, sizeof(dev_file_t));

  f->handle = -1;
  f->open_flags = flags;
  f->drv_data = NULL;

  if (name[0] == '~') {
    if (getenv("HOME")) {
      strlcpy(f->name, getenv("HOME"), sizeof(f->name));
    } else {
      strlcpy(f->name, getenv("HOMEDRIVE"), sizeof(f->name));
      strlcpy(f->name, getenv("HOMEPATH"), sizeof(f->name));
    }
    strlcpy(f->name, "/", sizeof(f->name));
    strlcpy(f->name, name + 1, sizeof(f->name));
  } else {
    strlcpy(f->name, name, sizeof(f->name));
  }

  f->type = ft_stream;

  //
  // special devices
  //
  if (strlen(f->name) > 4) {
    if (f->name[4] == ':') {
      for (int i = 0; i < 5; i++) {
        f->name[i] = to_upper(f->name[i]);
      }
      if (strncmp(f->name, "COM", 3) == 0) {
        f->type = ft_serial_port;
        f->devspeed = 9600;
        f->port = f->name[3] - '0';
        if (f->port < 0) {
          f->port = 10;
        }
        if (strlen(f->name) > 5) {
          f->devspeed = xstrtol(f->name + 5);
        }
      } else if (strncmp(f->name, "SOCL:", 5) == 0) {
        f->type = ft_socket_client;
      } else if (strncasecmp(f->name, "HTTP:", 5) == 0) {
        f->type = ft_http_client;
      } else if (strncmp(f->name, "SOUT:", 5) == 0 ||
                 strncmp(f->name, "SDIN:", 5) == 0 ||
                 strncmp(f->name, "SERR:", 5) == 0) {
        f->type = ft_stream;
      }
    } else if (f->name[3] == ':') {
      for (int i = 0; i < 4; i++) {
        f->name[i] = to_upper(f->name[i]);
      }

      if (strncmp(f->name, "CON:", 4) == 0) {
        strcpy(f->name, "SOUT:");
        f->type = ft_stream;
      } else if (strncmp(f->name, "KBD:", 4) == 0) {
        strcpy(f->name, "SDIN:");
        f->type = ft_stream;
      }
    } else if (strncmp(f->name, "/dev/tty", 8) == 0) {
      f->type = ft_serial_port;
      f->devspeed = 9600;      
      const char *ptrSpeed = strchr(f->name, ':');
      if(ptrSpeed) {
        if(strlen(ptrSpeed) > 1) {
          f->devspeed = xstrtol(ptrSpeed + 1);
        }       
        f->name[ptrSpeed - f->name] = '\0';        
      }
    }
    if (f->name[5] == ':') {
      for (int i = 0; i < 4; i++) {
        f->name[i] = to_upper(f->name[i]);
      }
      if (strncmp(f->name, "COM", 3) == 0) {
        f->type = ft_serial_port;
        f->devspeed = 9600;
        f->port = (f->name[3] - '0') * 10 + (f->name[4] - '0');
        if (f->port < 0) {
          f->port = 10;
        }
        if (strlen(f->name) > 6) {
          f->devspeed = xstrtol(f->name + 6);
        }
      }
    }
  } // device

  if (!opt_file_permitted && f->type != ft_http_client) {
    rt_raise(ERR_FILE_PERM);
    return 0;
  }

  //
  // open
  //
  switch (f->type) {
  case ft_stream:
    return stream_open(f);
  case ft_socket_client:
    return sockcl_open(f);
  case ft_http_client:
    return http_open(f);
  case ft_serial_port:
    return serial_open(f);
  default:
    err_unsup();
  };

  return 0;
}

/**
 * returns true on success
 */
int dev_fclose(int sb_handle) {
  dev_file_t *f;

  if ((f = dev_getfileptr(sb_handle)) == NULL) {
    return 0;
  }

  switch (f->type) {
  case ft_stream:
    return stream_close(f);
  case ft_serial_port:
    return serial_close(f);
  case ft_socket_client:
  case ft_http_client:
    return sockcl_close(f);
  default:
    err_unsup();
  }
  return 0;
}

/**
 * returns true on success
 */
int dev_fwrite(int sb_handle, byte *data, uint32_t size) {
  dev_file_t *f;

  if ((f = dev_getfileptr(sb_handle)) == NULL) {
    return 0;
  }

  switch (f->type) {
  case ft_stream:
    return stream_write(f, data, size);
  case ft_serial_port:
    return serial_write(f, data, size);
  case ft_socket_client:
  case ft_http_client:
    return sockcl_write(f, data, size);
  default:
    err_unsup();
  };
  return 0;
}

/**
 * returns true on success
 */
int dev_fread(int sb_handle, byte *data, uint32_t size) {
  dev_file_t *f;

  if ((f = dev_getfileptr(sb_handle)) == NULL) {
    return 0;
  }

  switch (f->type) {
  case ft_stream:
    return stream_read(f, data, size);
  case ft_serial_port:
    return serial_read(f, data, size);
  case ft_socket_client:
  case ft_http_client:
    return sockcl_read(f, data, size);
  default:
    err_unsup();
  }
  return 0;
}

/**
 *
 */
uint32_t dev_ftell(int sb_handle) {
  dev_file_t *f;

  if ((f = dev_getfileptr(sb_handle)) == NULL) {
    return 0;
  }

  switch (f->type) {
  case ft_stream:
    return stream_tell(f);
  default:
    err_unsup();
  };
  return 0;
}

/**
 *
 */
uint32_t dev_flength(int sb_handle) {
  dev_file_t *f;

  if ((f = dev_getfileptr(sb_handle)) == NULL) {
    return 0;
  }

  switch (f->type) {
  case ft_stream:
    return stream_length(f);
  case ft_serial_port:
    return serial_length(f);
  case ft_socket_client:
  case ft_http_client:
    return sockcl_length(f);
  default:
    err_unsup();
  };
  return 0;
}

/**
 *
 */
uint32_t dev_fseek(int sb_handle, uint32_t offset) {
  dev_file_t *f;

  if ((f = dev_getfileptr(sb_handle)) == NULL) {
    return 0;
  }

  switch (f->type) {
  case ft_stream:
    return stream_seek(f, offset);
  default:
    err_unsup();
  };
  return -1;
}

/**
 *
 */
int dev_feof(int sb_handle) {
  dev_file_t *f;

  if ((f = dev_getfileptr(sb_handle)) == NULL) {
    return 0;
  }

  switch (f->type) {
  case ft_stream:
    return stream_eof(f);
  case ft_serial_port:
    return serial_eof(f);
  case ft_socket_client:
  case ft_http_client:
    return sockcl_eof(f);
  default:
    err_unsup();
  };

  return 0;
}

/**
 * deletes a file
 * returns true on success
 */
int dev_fremove(const char *file) {
  int success;

  if (!opt_file_permitted) {
    rt_raise(ERR_FILE_PERM);
    return 0;
  }

  success = (remove(file) == 0);
  if (!success) {
    err_throw(FSERR_ACCESS);
  }
  return success;
}

/**
 * returns true if the file exists
 */
int dev_fexists(const char *file) {
  if (!opt_file_permitted) {
    rt_raise(ERR_FILE_PERM);
    return 0;
  }

  return (access(file, 0) == 0);
}

/**
 * copy file
 * returns true on success
 */
int dev_fcopy(const char *file, const char *newfile) {
  if (!opt_file_permitted) {
    rt_raise(ERR_FILE_PERM);
    return 0;
  }

  if (dev_fexists(file)) {
    if (dev_fexists(newfile)) {
      if (!dev_fremove(newfile)) {
        return 0;               // cannot delete target-file
      }
    }

    int src = dev_freefilehandle();
    if (prog_error) {
      return 0;
    }
    dev_fopen(src, file, DEV_FILE_INPUT);
    if (prog_error) {
      return 0;
    }
    int dst = dev_freefilehandle();
    if (prog_error) {
      return 0;
    }
    dev_fopen(dst, newfile, DEV_FILE_OUTPUT);
    if (prog_error) {
      return 0;
    }

    uint32_t file_len = dev_flength(src);
    if (file_len != -1 && file_len > 0) {
      uint32_t block_size = 1024;
      uint32_t block_num = file_len / block_size;
      uint32_t remain = file_len - (block_num * block_size);
      byte *buf = malloc(block_size);

      for (int i = 0; i < block_num; i++) {
        dev_fread(src, buf, block_size);
        if (prog_error) {
          free(buf);
          return 0;
        }
        dev_fwrite(dst, buf, block_size);
        if (prog_error) {
          free(buf);
          return 0;
        }
      }

      if (remain) {
        dev_fread(src, buf, remain);
        if (prog_error) {
          free(buf);
          return 0;
        }
        dev_fwrite(dst, buf, remain);
        if (prog_error) {
          free(buf);
          return 0;
        }
      }
      free(buf);
    }

    dev_fclose(src);
    if (prog_error) {
      return 0;
    }
    dev_fclose(dst);
    if (prog_error) {
      return 0;
    }
    return 1;
  }
  return 0;  // source file does not exists
}

/**
 * rename file
 * returns true on success
 */
int dev_frename(const char *file, const char *newname) {
  if (!opt_file_permitted) {
    rt_raise(ERR_FILE_PERM);
    return 0;
  }
  if (dev_fcopy(file, newname)) {
    return dev_fremove(file);
  }
  return 0;
}

/**
 * create a directory
 * BUG: no drivers supported
 */
void dev_mkdir(const char *dir) {
  if (!opt_file_permitted) {
    rt_raise(ERR_FILE_PERM);
    return;
  }
#if (defined(_Win32) || defined(__MINGW32__)) && !defined(__CYGWIN__)
  if (mkdir(dir) != 0) {
    err_file(errno);
  }
#else
  if (mkdir(dir, 0777) != 0) {
    err_file(errno);
  }
#endif
}

/**
 * removes a directory
 */
void dev_rmdir(const char *dir) {
  if (!opt_file_permitted) {
    rt_raise(ERR_FILE_PERM);
    return;
  }
  if (rmdir(dir) != 0) {
    err_file(errno);
  }
}

/**
 * changes the current directory
 */
void dev_chdir(const char *dir) {
  if (chdir(dir) != 0) {
    err_file(errno);
  }
  setsysvar_str(SYSVAR_CWD, dev_getcwd());
}

/**
 * create a file-list using wildcards
 */
char_p_t *dev_create_file_list(const char *wc, int *count) {
  DIR *dp;
  struct dirent *e;
  char wc2[OS_FILENAME_SIZE + 1];
  char path[OS_PATHNAME_SIZE + 1];
  int l, size;
  char_p_t *list;

  if (!opt_file_permitted) {
    rt_raise(ERR_FILE_PERM);
    return NULL;
  }

  if (wc) {
    strlcpy(path, wc, sizeof(path));
    char *p = strrchr(path, OS_DIRSEP);
    if (p == NULL) {
      getcwd(path, OS_PATHNAME_SIZE);
      if (path[(l = strlen(path))] != OS_DIRSEP) {
        path[l] = OS_DIRSEP;
        path[l + 1] = '\0';
      }
      strlcpy(wc2, wc, sizeof(wc2));
    } else {
      strlcpy(wc2, p + 1, sizeof(wc2));
      *(p + 1) = '\0';
      if (strlen(wc2) == 0) {
        strcpy(wc2, "*");
      }
    }
  } else {
    getcwd(path, OS_PATHNAME_SIZE);
    if (path[(l = strlen(path))] != OS_DIRSEP) {
      path[l] = OS_DIRSEP;
      path[l + 1] = '\0';
    }
    wc2[0] = '\0';
  }

  *count = 0;
  size = 256;
  list = malloc(sizeof(char_p_t) * size);

  if ((dp = opendir(path)) == NULL) {
    return list;
  }

  while ((e = readdir(dp)) != NULL) {
    char *name = e->d_name;
    if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0)) {
      continue;
    }
    if (wc_match(wc2, name)) {
      if ((*count + 1) == size) {
        size += 256;
        list = realloc(list, sizeof(char_p_t) * size);
      }
      list[*count] = (char *) malloc(strlen(name) + 1);
      strcpy(list[*count], name);
      *count = *count + 1;
    }
  }

  closedir(dp);

  // common for all, if there are no files, return NULL
  if (*count == 0) {
    if (list) {
      free(list);
    }
    list = NULL;
  }
  return list;
}

/**
 * destroy the file-list
 */
void dev_destroy_file_list(char_p_t *list, int count) {
  for (int i = 0; i < count; i++) {
    free(list[i]);
  }
  free(list);
}

/**
 * returns the current directory
 * BUG: no drivers supported
 */
char *dev_getcwd() {
  static char retbuf[OS_PATHNAME_SIZE + 1];
  getcwd(retbuf, OS_PATHNAME_SIZE);
  int l = strlen(retbuf);
  if (retbuf[l - 1] != OS_DIRSEP) {
    retbuf[l] = OS_DIRSEP;
    retbuf[l + 1] = '\0';
  }
  return retbuf;
}

/**
 * returns the file attributes
 * 1-1-1 = link - directory - regular file
 */
int dev_fattr(const char *file) {
  struct stat st;
  int r = 0;

  if (stat(file, &st) == 0) {
    r |= ((S_ISREG(st.st_mode)) ? VFS_ATTR_FILE : 0);
    r |= ((S_ISDIR(st.st_mode)) ? VFS_ATTR_DIR : 0);
#if defined(_UnixOS) && !defined(__MINGW32__)
    r |= ((S_ISLNK(st.st_mode)) ? VFS_ATTR_LINK : 0);
#endif
  }
  return r;
}

/**
 * returns the access rights of the file
 */
int dev_faccess(const char *file) {
  struct stat st;

  if (!opt_file_permitted) {
    rt_raise(ERR_FILE_PERM);
    return 0;
  }

  if (stat(file, &st) == 0) {
    return st.st_mode;
  }
  return 0;
}

/**
 * returns the last-modified time for a file as a string
 */
int dev_filemtime(var_t *v, char **buffer) {
  time_t time = 0;
  int size = 0;

  if (v_is_type(v, V_INT)) {
    time = v->v.i;
  } else if (v_is_type(v, V_STR)) {
    const char *file = v_str(v);
    struct stat st;
    if (!opt_file_permitted) {
      rt_raise(ERR_FILE_PERM);
    } else if (stat(file, &st) == 0) {
      time = st.st_mtime;
    } else {
      err_file_not_found();
    }
  } else {
    err_argerr();
  }

  if (prog_error) {
    *buffer = malloc(1);
    *buffer[0] = '\0';
  } else {
    // size for '2016-02-20 05:23 PM'
    size = 20;
    *buffer = malloc(size);
    size = strftime(*buffer, size, "%Y-%m-%d %I:%M %p", localtime(&time));
  }
  return size;
}
