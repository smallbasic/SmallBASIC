// This file is part of SmallBASIC
//
// SmallBASIC RTL - FILESYSTEM, FILE and DEVICE I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/kw.h"
#include "common/var.h"
#include "common/pproc.h"
#include "common/device.h"
#include "common/blib.h"
#include "common/messages.h"
#include "common/fs_socket_client.h"

#if defined(__BORLANDC__)
#define F_OK    0
#define R_OK    4
#define W_OK    6
#endif

void write_encoded_var(int handle, var_t * var) SEC(BIO);
int read_encoded_var(int handle, var_t * var) SEC(BIO);

struct file_encoded_var {
  byte sign;                    // always '$'
  byte version;                 //
  byte type;
  dword size;
};

/*
 * OPEN "file" [FOR {INPUT|OUTPUT|APPEND}] AS #fileN
 */
void cmd_fopen() {
  var_t file_name;
  int flags = 0;
  int handle;

  // filename
  par_getstr(&file_name);
  if (prog_error)
    return;

  // mode
  if (code_peek() == kwFORSEP) {
    code_skipnext();
    switch (code_peek()) {
    case kwINPUTSEP:
      flags = DEV_FILE_INPUT;
      break;
    case kwOUTPUTSEP:
      flags = DEV_FILE_OUTPUT;
      break;
    case kwAPPENDSEP:
      flags = DEV_FILE_APPEND;
      break;
    default:
      err_syntax();
      v_free(&file_name);
      return;
    }

    code_skipnext();
  } else {
    flags = 0;                  // ????
  }

  // file handle
  if (code_peek() == kwAS) {
    code_skipnext();

    par_getsharp();
    if (!prog_error) {
      handle = par_getint();
      if (!prog_error) {
        if (dev_fstatus(handle) == 0)
          dev_fopen(handle, (char *) file_name.v.p.ptr, flags);
        else
          rt_raise("OPEN: FILE IS ALREADY OPENED");
      }
    }
  } else {
    err_syntax();
  }
  v_free(&file_name);
}

/*
 * CLOSE #fileN
 */
void cmd_fclose() {
  int handle;

  // file handle
  par_getsharp();
  if (!prog_error) {
    handle = par_getint();
    if (!prog_error) {
      if (dev_fstatus(handle)) {
        dev_fclose(handle);
      } else {
        rt_raise("CLOSE: FILE IS NOT OPENED");
      }
    }
  }
}

/*
 * SEEK #fileN, pos
 */
void cmd_fseek() {
  int handle;
  dword pos;

  // file handle
  par_getsharp();
  if (!prog_error) {
    handle = par_getint();
    if (!prog_error) {
      if (dev_fstatus(handle)) {
        par_getsep();
        if (!prog_error) {
          pos = par_getint();
          if (!prog_error) {
            dev_fseek(handle, pos);
          }
        }
      } else {
        rt_raise("SEEK: FILE IS NOT OPENED");
      }
    }
  }
}

/*
 * PRINT #fileN; var1 [, varN]
 */
void cmd_fprint() {
  cmd_print(PV_FILE);
}

/*
 * store a variable in binary form
 */
void write_encoded_var(int handle, var_t * var) {
  struct file_encoded_var fv;
  var_t *elem;
  int i;

  fv.sign = '$';
  fv.version = 1;
  fv.type = var->type;
  switch (var->type) {
  case V_INT:
    fv.size = 4;
    dev_fwrite(handle, (byte *) &fv, sizeof(struct file_encoded_var));
    dev_fwrite(handle, (byte *) &var->v.i, fv.size);
    break;
  case V_NUM:
    fv.size = 8;
    dev_fwrite(handle, (byte *) &fv, sizeof(struct file_encoded_var));
    dev_fwrite(handle, (byte *) &var->v.n, fv.size);
    break;
  case V_STR:
    fv.size = strlen((char *) var->v.p.ptr);
    dev_fwrite(handle, (byte *) &fv, sizeof(struct file_encoded_var));
    dev_fwrite(handle, (byte *) var->v.p.ptr, fv.size);
    break;
  case V_ARRAY:
    fv.size = var->v.a.size;
    dev_fwrite(handle, (byte *) &fv, sizeof(struct file_encoded_var));

    // write additional data about array
    dev_fwrite(handle, &var->v.a.maxdim, 1);
    for (i = 0; i < var->v.a.maxdim; i++) {
      dev_fwrite(handle, (byte *) &var->v.a.lbound[i], sizeof(int));
      dev_fwrite(handle, (byte *) &var->v.a.ubound[i], sizeof(int));
    }

    // write elements
    for (i = 0; i < var->v.a.size; i++) {
      elem = (var_t *) (var->v.a.ptr + sizeof(var_t) * i);
      write_encoded_var(handle, elem);
    }
    break;
  };
}

/*
 * read a variable from a binary form
 */
int read_encoded_var(int handle, var_t * var) {
  struct file_encoded_var fv;
  var_t *elem;
  int i;

  dev_fread(handle, (byte *) &fv, sizeof(struct file_encoded_var));
  if (fv.sign != '$') {
    rt_raise("READ: BAD SIGNATURE");
    return -1;                  // bad signature
  }

  v_free(var);
  switch (fv.type) {
  case V_INT:
    var->type = V_INT;
    dev_fread(handle, (byte *) &var->v.i, fv.size);
    break;
  case V_NUM:
    var->type = V_NUM;
    dev_fread(handle, (byte *) &var->v.n, fv.size);
    break;
  case V_STR:
    var->type = V_STR;
    var->v.p.ptr = tmp_alloc(fv.size + 1);
    dev_fread(handle, (byte *) var->v.p.ptr, fv.size);
    var->v.p.ptr[fv.size] = '\0';
    break;
  case V_ARRAY:
    var->type = V_ARRAY;
    var->v.a.ptr = tmp_alloc(fv.size * sizeof(var_t));
    var->v.a.size = fv.size;

    // read additional data about array
    dev_fread(handle, (byte *) &var->v.a.maxdim, 1);
    for (i = 0; i < var->v.a.maxdim; i++) {
      dev_fread(handle, (byte *) &var->v.a.lbound[i], sizeof(int));
      dev_fread(handle, (byte *) &var->v.a.ubound[i], sizeof(int));
    }

    // write elements
    for (i = 0; i < var->v.a.size; i++) {
      elem = (var_t *) (var->v.a.ptr + sizeof(var_t) * i);
      v_init(elem);
      read_encoded_var(handle, elem);
    }
    break;
  default:
    return -2;                  // unknown data-type
  };

  return 0;
}

/*
 * WRITE #fileN; var1 [, varN]
 */
void cmd_fwrite() {
  int handle;

  // file handle
  par_getsharp();
  if (!prog_error) {
    handle = par_getint();
    if (!prog_error) {

      if (code_peek() == kwTYPE_EOC || code_peek() == kwTYPE_LINE) {  // There
        // are
        // no
        // parameters
        if (!dev_fstatus(handle)) {
          // dev_fwrite(handle, "\n", 1);
          ;
        } else {
          rt_raise("FIO: FILE IS NOT OPENED");
        }
        return;
      }

      par_getsep();             // allow commas

      if (!prog_error) {

        if (dev_fstatus(handle)) {

          byte code, exitf = 0;
          var_t *var_p;

          do {
            code = code_peek();
            switch (code) {
            case kwTYPE_LINE:
            case kwTYPE_EOC:
              exitf = 1;
              break;
            case kwTYPE_SEP:
              code_skipsep();
              break;
            case kwTYPE_VAR:
              var_p = par_getvar_ptr();
              if (!prog_error)
                write_encoded_var(handle, var_p);
              break;
            default:
              rt_raise("WRITE: ONLY VARIABLES ARE ALLOWED");
            };

            if (prog_error) {
              return;
            }
          } while (!exitf);
        } else {
          rt_raise("FIO: FILE IS NOT OPENED");
        }
      }
    }
  }
}

/*
 * READ #fileN; var1 [, var2 [, ...]]
 */
void cmd_fread() {
  int handle;
  var_t *var_p;
  byte code;

  // file handle
  par_getsharp();
  if (!prog_error) {
    handle = par_getint();
    if (prog_error)
      return;

    par_getsep();               // allow commas
    if (prog_error)
      return;

    if (dev_fstatus(handle)) {

      // get the variables
      do {
        if (prog_error)
          return;

        // get variable's ptr
        var_p = par_getvar_ptr();
        if (prog_error)
          return;

        read_encoded_var(handle, var_p);
        if (prog_error)
          return;

        // next
        code = code_peek();
        if (code == kwTYPE_SEP)
          par_getsep();         // allow commas
        else
          break;
      } while (1);
    } else
      rt_raise("FIO: FILE IS NOT OPENED");
  }
}

/*
 * LINE INPUT [#fileN;] var$
 */
void cmd_flineinput() {
  int handle, size, index;
  var_t *var_p;
  byte code, ch;

  if (code_peek() == kwTYPE_SEP) {
    //
    // FILE OR DEVICE
    //

    // file handle
    par_getsharp();
    if (!prog_error) {

      handle = par_getint();
      if (!prog_error) {
        // par_getsemicolon();
        par_getsep();           // allow commas

        if (!prog_error) {

          if (dev_fstatus(handle)) {

            // get the variable
            code = code_peek();
            if (code != kwTYPE_VAR) {
              err_syntax();
              return;
            }
            var_p = code_getvarptr();

            if (!prog_error) {
              v_free(var_p);
              size = 256;
              index = 0;
              var_p->type = V_STR;
              var_p->v.p.ptr = tmp_alloc(size);

              // READ IT
              while (!dev_feof(handle)) {
                dev_fread(handle, &ch, 1);
                if (prog_error) {
                  v_free(var_p);
                  var_p->type = V_INT;
                  var_p->v.i = -1;
                  return;
                } else if (ch == '\n')
                  break;
                else if (ch != '\r') {
                  // store char
                  if (index == (size - 1)) {
                    size += 256;
                    var_p->v.p.ptr = tmp_realloc(var_p->v.p.ptr, size);
                  }
                  var_p->v.p.ptr[index] = ch;
                  index++;
                }
              }

              //
              var_p->v.p.ptr[index] = '\0';
              var_p->v.p.size = index + 1;

            }                   // read
            else
              rt_raise("FIO: FILE IS NOT OPENED");
          }
        }
      }
    }
  } else {
    //
    // CONSOLE
    //
    var_p = par_getvar_ptr();
    if (!prog_error) {
      v_free(var_p);
      var_p->type = V_STR;
      var_p->v.p.ptr = tmp_alloc(SB_TEXTLINE_SIZE + 1);
      var_p->v.p.size = SB_TEXTLINE_SIZE + 1;
      ((char*) var_p->v.p.ptr)[0] = 0;
      dev_gets((char *) var_p->v.p.ptr, SB_TEXTLINE_SIZE);
      dev_print("\n");
    }
  }
}

/*
 * INPUT #fileN; var$ [, var2 [, ...]]
 */
void cmd_finput() {
  cmd_input(PV_FILE);
}

/*
 * KILL filename
 */
void cmd_fkill() {
  var_t file_name;

  // filename
  v_init(&file_name);
  par_getstr(&file_name);
  if (prog_error)
    return;
  if (dev_fexists((char *) file_name.v.p.ptr))
    dev_fremove((char *) file_name.v.p.ptr);
  // else
  // rt_raise("KILL: FILE DOES NOT EXIST");
  v_free(&file_name);
}

/*
 * COPY/RENAME filem newfile
 */
void cmd_filecp(int mv) {
  var_t src, dst;

  // filename
  v_init(&src);
  v_init(&dst);
  par_getstr(&src);
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error) {
    v_free(&src);
    return;
  }
  par_getstr(&dst);
  if (prog_error) {
    v_free(&src);
    return;
  }

  if (dev_fexists((char *) src.v.p.ptr)) {
    if (!mv) {
      dev_fcopy((char *) src.v.p.ptr, (char *) dst.v.p.ptr);
    } else {
      dev_frename((char *) src.v.p.ptr, (char *) dst.v.p.ptr);
    }
  } else {
    rt_raise("COPY/RENAME: FILE DOES NOT EXIST");
  }
  v_free(&src);
  v_free(&dst);
}

/*
 * change the current directory
 */
void cmd_chdir() {
  var_t dir;

  // filename
  v_init(&dir);
  par_getstr(&dir);
  if (prog_error)
    return;
  dev_chdir((char *) dir.v.p.ptr);
  v_free(&dir);
}

/*
 * remove directory
 */
void cmd_rmdir() {
  var_t dir;

  // filename
  v_init(&dir);
  par_getstr(&dir);
  if (prog_error)
    return;
  dev_rmdir((char *) dir.v.p.ptr);
  v_free(&dir);
}

/*
 * create directory
 */
void cmd_mkdir() {
  var_t dir;

  // filename
  v_init(&dir);
  par_getstr(&dir);
  if (prog_error)
    return;
  dev_mkdir((char *) dir.v.p.ptr);
  v_free(&dir);
}

/*
 *   load text-file to string or to array
 *   Modified 2-May-2002 Chris Warren-Smith. Implemented buffered read
 *
 *   TLOAD filename, variable [, type]
 */
#if defined(OS_LIMITED)
#define LDLN_INC    16
#define GROW_SIZE    128
#define BUFMAX       128
#else
#define LDLN_INC    256
#define GROW_SIZE   1024
#define BUFMAX       256
#endif

#define CHK_ERR_CLEANUP(s)                      \
  if (prog_error) {                             \
    v_free(&file_name);                         \
    rt_raise(s);                                \
    return;                                     \
  }
#define CHK_ERR(s)                              \
  if (prog_error) {                             \
    rt_raise(s);                                \
    return;                                     \
  }

void cmd_floadln() {
  var_t file_name, *array_p = NULL, *var_p = NULL;
  int flags = DEV_FILE_INPUT;
  int handle, index, bcount, size, array_size;
  byte ch, type = 0;
  char buf[BUFMAX];
  int eof, eol, bufLen, bufIndex;
  dword unreadBytes;

  if (code_peek() == kwTYPE_SEP) {
    // "filename" is an already open file number
    flags = 0;
    par_getsharp();
    CHK_ERR(FSERR_INVALID_PARAMETER);
    handle = par_getint();
    CHK_ERR(FSERR_INVALID_PARAMETER);
    par_getcomma();
    CHK_ERR(FSERR_INVALID_PARAMETER);
    array_p = var_p = code_getvarptr();
    CHK_ERR(FSERR_INVALID_PARAMETER);
    if (code_peek() == kwTYPE_SEP) {
      par_getcomma();
      CHK_ERR(FSERR_INVALID_PARAMETER);
      type = par_getint();
    }

    dev_file_t *f = dev_getfileptr(handle);
    if (f->type == ft_http_client) {
      http_read(f, var_p, type);  // TLOAD #1, html_str
      return;
    }
  } else {
    // filename
    par_getstr(&file_name);
    CHK_ERR(FSERR_INVALID_PARAMETER);
    par_getcomma();
    CHK_ERR_CLEANUP(FSERR_INVALID_PARAMETER);
    array_p = var_p = code_getvarptr();
    CHK_ERR_CLEANUP(FSERR_INVALID_PARAMETER);
    if (code_peek() == kwTYPE_SEP) {
      par_getcomma();
      CHK_ERR_CLEANUP(FSERR_INVALID_PARAMETER);
      type = par_getint();
    }

    handle = dev_freefilehandle();
    CHK_ERR_CLEANUP(FSERR_GENERIC);
    if (dev_fstatus(handle)) {
      v_free(&file_name);
      rt_raise(FSERR_GENERIC);
      return;
    }

    dev_fopen(handle, (char *) file_name.v.p.ptr, flags);
    v_free(&file_name);
    CHK_ERR(FSERR_GENERIC);
  }

  if (type == 0) {
    // build array
    array_size = LDLN_INC;
    v_toarray1(array_p, array_size);  // v_free() is here
    index = 0;

    eof = dev_feof(handle);
    unreadBytes = eof ? 0 : dev_flength(handle);
    bufIndex = bufLen = 0;

    while (!eof) {
      // build var for line
      var_p = (var_t *) (array_p->v.a.ptr + (sizeof(var_t) * index));
      size = GROW_SIZE;
      var_p->type = V_STR;
      var_p->v.p.ptr = tmp_alloc(size);
      index++;

      // process the next line
      bcount = 0;
      eol = 0;
      while (!eof && !eol) {
        if (bufIndex == bufLen) { // read into empty buffer
          if (dev_feof(handle) || unreadBytes == 0) {
            eof = 1;
            break;
          }
          bufLen = (unreadBytes > BUFMAX) ? BUFMAX : unreadBytes;
          bufIndex = 0;
          unreadBytes -= bufLen;

          dev_fread(handle, (byte *) buf, bufLen);
          if (prog_error) {
            eof = 1;
            break;
          }
        }

        ch = buf[bufIndex++];
        if (ch == '\n') {
          eol = 1;
          break;
        } else if (ch != '\r') {  // store char
          if (bcount >= (size - 1)) {
            size += GROW_SIZE;
            var_p->v.p.ptr = tmp_realloc(var_p->v.p.ptr, size);
          }
          var_p->v.p.ptr[bcount] = ch;
          bcount++;
        }
      }                         // read line

      if (prog_error) {         // clear & exit
        v_free(array_p);
        v_init(array_p);
        break;
      }

      // store text-line
      var_p->v.p.ptr[bcount] = '\0';
      var_p->v.p.size = bcount + 1;
      var_p->v.p.ptr = tmp_realloc(var_p->v.p.ptr, var_p->v.p.size);

      // resize array
      if (index >= (array_size - 1)) {
        array_size += LDLN_INC;
        v_resize_array(array_p, array_size);
      }
    }                           // read file

    if (index) {
      v_resize_array(array_p, index);
    } else {
      v_resize_array(array_p, 0); // v_free() is here
    }
  } else {                        // if type=1
    // build string
    v_free(var_p);
    var_p->type = V_STR;
    var_p->v.p.size = dev_flength(handle) + 1;
    var_p->v.p.ptr = tmp_alloc(var_p->v.p.size);
    if (var_p->v.p.size > 1) {
      dev_fread(handle, var_p->v.p.ptr, var_p->v.p.size - 1);
    }
    var_p->v.p.ptr[var_p->v.p.size - 1] = '\0';
  }
  if (flags == DEV_FILE_INPUT) {
    dev_fclose(handle);
  }
}

/*
 * save text file
 *
 * TSAVE filename, array/string
 */
void cmd_fsaveln() {
  var_t file_name, *array_p = NULL, *var_p = NULL;
  int flags = DEV_FILE_OUTPUT;
  int handle, i;

  if (code_peek() == kwTYPE_SEP) {
    // "filename" is an already open file number
    flags = 0;
    par_getsharp();
    CHK_ERR(FSERR_INVALID_PARAMETER);
    handle = par_getint();
    CHK_ERR(FSERR_INVALID_PARAMETER);
    par_getcomma();
    CHK_ERR(FSERR_INVALID_PARAMETER);
    array_p = var_p = code_getvarptr();
    CHK_ERR(FSERR_INVALID_PARAMETER);
  } else {
    // filename
    par_getstr(&file_name);
    CHK_ERR(FSERR_INVALID_PARAMETER);
    par_getcomma();
    CHK_ERR_CLEANUP(FSERR_INVALID_PARAMETER);
    array_p = var_p = code_getvarptr();
    CHK_ERR_CLEANUP(FSERR_INVALID_PARAMETER);
    handle = dev_freefilehandle();
    CHK_ERR_CLEANUP(FSERR_GENERIC);
    if (dev_fstatus(handle)) {
      v_free(&file_name);
      rt_raise(FSERR_GENERIC);
      return;
    }

    dev_fopen(handle, (char *) file_name.v.p.ptr, flags);
    v_free(&file_name);
    CHK_ERR(FSERR_GENERIC);
  }

  if (var_p->type == V_ARRAY) {
    // parameter is an array
    for (i = 0; i < array_p->v.a.size; i++) {
      var_p = (var_t *) (array_p->v.a.ptr + (sizeof(var_t) * i));
      fprint_var(handle, var_p);
      dev_fwrite(handle, (byte *) "\n", 1);
    }
  } else {
    // parameter is an string
    fprint_var(handle, var_p);
  }

  if (flags == DEV_FILE_OUTPUT) {
    dev_fclose(handle);
  }
}

/*
 * TODO: lock a record or an area
 *
 * LOCK #1, [record]|[start TO end]
 */
void cmd_flock() {
  var_t str;

  par_getstr(&str);
  if (prog_error)
    return;
  v_free(&str);
}

/*
 * CHMOD file, mode
 */
void cmd_chmod() {
  var_t str;
  dword mode;

  par_getstr(&str);
  if (prog_error)
    return;
  par_getcomma();
  if (prog_error) {
    v_free(&str);
    return;
  }
  mode = par_getint();
  if (prog_error) {
    v_free(&str);
    return;
  }

  chmod((char *) str.v.p.ptr, mode);
  v_free(&str);
}

/*
 * walk on dirs
 */
#if defined(_UnixOS) || defined(_DOS) || defined(_Win32)
void dirwalk(char *dir, char *wc, addr_t use_ip)
{
  char name[OS_PATHNAME_SIZE];
  struct dirent *dp;
  DIR *dfd;
  struct stat st;
  int callusr, contf = 1;

  if ((dfd = opendir(dir)) == NULL) {
    log_printf("DIRWALK: can't open %s", dir);
    return;
  }

  while ((dp = readdir(dfd)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
      continue; /* skip self and parent */
    }
    if (strlen(dir) + strlen(dp->d_name) + 2 > sizeof(name)) {
      rt_raise("DIRWALK: name %s/%s too long", dir, dp->d_name);
    }
    else {
      sprintf(name, "%s%c%s", dir, OS_DIRSEP, dp->d_name);

      /*
       *   check filename
       */
      if (!wc) {
        callusr = 1;
      }
      else {
        callusr = wc_match(wc, dp->d_name);
      }
      if (callusr) {
        /*
         *   call user's function
         */
        var_t *var;

        var = v_new();
        v_createstr(var, name);
        exec_usefunc(var, use_ip);
        contf = v_getint(var);
        v_free(var);
        tmp_free(var);
      }
      if (!contf) {
        break;
      }
      /*
       *   proceed to the next
       */
      if (access(name, R_OK) == 0) {  // user-func, possible it is deleted
        stat(name, &st);
        if (st.st_mode & S_IFDIR) {
          dirwalk(name, wc, use_ip);
        }
      }
    }
  }
  closedir(dfd);
}
#endif

/*
 * walking on directories
 *
 * DIRWALK "/home" [, "*"] USE MYPRN(x)
 */
void cmd_dirwalk() {
  addr_t use_ip, exit_ip;
  char *dir = NULL, *wc = NULL;

  par_massget("Ss", &dir, &wc);
  if (!prog_error) {

    // USE
    if (code_peek() == kwUSE) {
      code_skipnext();
      use_ip = code_getaddr();
      exit_ip = code_getaddr();
    } else {
      use_ip = exit_ip = INVALID_ADDR;
    }
    //
#if defined(_UnixOS) || defined(_DOS) || defined(_Win32)
    dirwalk(dir, wc, use_ip);
#endif

    //
    if (exit_ip != INVALID_ADDR) {
      code_jump(exit_ip);
    }
  }

  pfree2(dir, wc);
}

/*
 * write a byte to a stream
 *
 * BPUTC #file, byte
 */
void cmd_bputc() {
  int handle;
  var_t *var_p;
  byte code;

  // file handle
  par_getsharp();
  if (!prog_error) {
    handle = par_getint();
    if (prog_error)
      return;

    par_getsep();               // allow commas
    if (prog_error)
      return;

    if (dev_fstatus(handle)) {
      // get variable's ptr
      var_p = par_getvar_ptr();
      if (prog_error)
        return;

      code = v_getint(var_p);
      dev_fwrite(handle, &code, 1);
      if (prog_error)
        return;
    }
  }
}

/*
 * load from file to a memory address
 *
 * BLOAD file[, offset]
 */
void cmd_bload() {
  var_int_t ofs = -1, idata;
  char *fname = NULL;

  par_massget("Si", &fname, &ofs);
  if (!prog_error) {
    int flags = DEV_FILE_INPUT;
    int handle = dev_freefilehandle();
    var_int_t len;

    if (!prog_error) {
      if (dev_fstatus(handle) == 0) {
        dev_fopen(handle, fname, flags);
        if (!prog_error) {
          var_int_t *data;

          dev_fread(handle, (byte*) &idata, sizeof(idata));
          if (ofs == -1) {
            ofs = idata;
          }
          dev_fread(handle, (byte*) &len, sizeof(len));
          data = &ofs;
          dev_fread(handle, (byte*) data, len);
          dev_fclose(handle);
        }
      }
    }
  }

  pfree(fname);
}

/*
 * save memory contents to a file
 *
 * BSAVE file, offset, length
 */
void cmd_bsave() {
  var_int_t ofs = 0, len = 0;
  char *fname = NULL;

  par_massget("SII", &fname, &ofs, &len);
  if (!prog_error) {
    int flags = DEV_FILE_OUTPUT;
    int handle = dev_freefilehandle();
    if (!prog_error) {
      if (dev_fstatus(handle) == 0) {
        dev_fopen(handle, fname, flags);
        if (!prog_error) {
          var_int_t *data;

          dev_fwrite(handle, (byte*) &ofs, sizeof(ofs));
          dev_fwrite(handle, (byte*) &len, sizeof(len));
          data = &ofs;
          dev_fwrite(handle, (byte*) data, len);
          dev_fclose(handle);
        }
      }
    }
  }

  pfree(fname);
}

