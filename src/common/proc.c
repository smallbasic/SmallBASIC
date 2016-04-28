// This file is part of SmallBASIC
//
// SmallBASIC, library API
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/pproc.h"
#include "common/messages.h"
#include "common/inet.h"

#include <limits.h>
#include <dirent.h>

/*
 * returns the last-modified time of the file
 *
 * on error returns 0L
 */
time_t sys_filetime(const char *file) {
  struct stat st;

  if (stat(file, &st) == 0) {
    return st.st_mtime;
  }
  return 0L;
}

/*
 * search a set of directories for the given file
 * directories on path must be separated with symbol ':' (unix) or ';' (dos/win)
 *
 * @param path the path
 * @param file the file
 * @param retbuf a buffer to store the full-path-name file (can be NULL)
 * @return non-zero if found
 */
int sys_search_path(const char *path, const char *file, char *retbuf) {
  const char *ps, *p;
  char cur_path[OS_PATHNAME_SIZE + 1];
  int found = 0;

  if (path == NULL || strlen(path) == 0) {
    return 0;
  }
  ps = path;
  do {
    // next element, build cur_path
#if defined(_UnixOS)
    p = strchr(ps, ':');
#else
    p = strchr(ps, ';');
#endif
    if (!p) {
      strcpy(cur_path, ps);
    } else {
      strncpy(cur_path, ps, p - ps);
      cur_path[p - ps] = '\0';
      ps = p + 1;
    }

    // fix home directory
    if (cur_path[0] == '~') {
      char *old_path = malloc(strlen(cur_path));
      strcpy(old_path, cur_path + 1);
#if defined(_UnixOS)
      sprintf(cur_path, "%s/%s", getenv("HOME"), old_path);
#else
      if (getenv("HOME")) {
        sprintf(cur_path, "%s\\%s", getenv("HOME"), old_path);
      } else {
        sprintf(cur_path, "%s\\%s", getenv("HOMEPATH"), old_path);
      }
#endif
      free(old_path);
    }

    DIR *dp = opendir(cur_path);
    if (dp != NULL) {
      struct dirent *entry;
      while (!found && (entry = readdir(dp)) != NULL) {
        if (strcasecmp(entry->d_name, file) == 0) {
          int end = strlen(cur_path);
#if defined(_UnixOS)
          strcat(cur_path, "/");
#else
          strcat(cur_path, "\\");
#endif
          strcat(cur_path, entry->d_name);
          if (access(cur_path, R_OK) == 0) {
            if (retbuf) {
              strcpy(retbuf, cur_path);
            }
            found = 1;
          } else {
            cur_path[end] = '\0';
          }
        }
      }
      closedir(dp);
    }
  } while (p && !found);
  return found;
}

/*
 * execute a user's expression (using one variable)
 * (note: keyword USE)
 *
 * var - the variable (the X)
 * ip  - expression's address
 */
void exec_usefunc(var_t *var, bcip_t ip) {
  var_t *old_x;

  // save X
  old_x = v_clone(tvar[SYSVAR_X]);

  // run
  v_set(tvar[SYSVAR_X], var);
  v_free(var);
  code_jump(ip);
  eval(var);

  // restore X
  v_set(tvar[SYSVAR_X], old_x);
  v_free(old_x);
  free(old_x);
}

/*
 * execute a user's expression (using two variable)
 *
 * var1 - the first variable (the X)
 * var2 - the second variable (the Y)
 * ip   - expression's address
 */
void exec_usefunc2(var_t *var1, var_t *var2, bcip_t ip) {
  var_t *old_x, *old_y;

  // save X
  old_x = v_clone(tvar[SYSVAR_X]);
  old_y = v_clone(tvar[SYSVAR_Y]);

  // run
  v_set(tvar[SYSVAR_X], var1);
  v_free(var1);
  v_set(tvar[SYSVAR_Y], var2);
  v_free(var2);
  code_jump(ip);
  eval(var1);

  // restore X,Y
  v_set(tvar[SYSVAR_X], old_x);
  v_free(old_x);
  free(old_x);
  v_set(tvar[SYSVAR_Y], old_y);
  v_free(old_y);
  free(old_y);
}

/*
 * execute a user's expression (using three variable)
 *
 * var1 - the first variable (the X)
 * var2 - the second variable (the Y)
 * var3 - the thrid variable (the Z)
 * ip   - expression's address
 */
void exec_usefunc3(var_t *var1, var_t *var2, var_t *var3, bcip_t ip) {
  var_t *old_x, *old_y, *old_z;

  // save X
  old_x = v_clone(tvar[SYSVAR_X]);
  old_y = v_clone(tvar[SYSVAR_Y]);
  old_z = v_clone(tvar[SYSVAR_Z]);

  // run
  v_set(tvar[SYSVAR_X], var1);
  v_free(var1);
  v_set(tvar[SYSVAR_Y], var2);
  v_free(var2);
  v_set(tvar[SYSVAR_Z], var3);
  v_free(var3);
  code_jump(ip);
  eval(var1);

  // restore X,Y
  v_set(tvar[SYSVAR_X], old_x);
  v_free(old_x);
  free(old_x);
  v_set(tvar[SYSVAR_Y], old_y);
  v_free(old_y);
  free(old_y);
  v_set(tvar[SYSVAR_Z], old_z);
  v_free(old_z);
  free(old_z);
}

#ifndef IMPL_LOG_WRITE

/**
 * Write to the log file
 */
void lwrite(const char *buf) {
  int log_dev; /* logfile file handle */
  char log_name[OS_PATHNAME_SIZE + 1]; /* LOGFILE filename */

  // open
#if defined(_Win32) || defined(__MINGW32__)
  if (getenv("SBLOG")) {
    strcpy(log_name, getenv("SBLOG"));
  }
  else {
    sprintf(log_name, "c:%csb.log", OS_DIRSEP);
  }
#else
  sprintf(log_name, "%ctmp%csb.log", OS_DIRSEP, OS_DIRSEP);
#endif

  log_dev = open(log_name, O_RDWR, 0660);
  lseek(log_dev, 0, SEEK_END);
  if (log_dev == -1) {
    log_dev = open(log_name, O_CREAT | O_RDWR, 0660);
  }
  if (log_dev == -1) {
    panic("LOG: Error on creating log file");
  }

  // write
  if (write(log_dev, buf, strlen(buf)) == -1) {
    panic("LOG: write failed");
  }

  close(log_dev);
}
#endif // NOT IMPL_LOG_WRITE

/*
 * Write string to output device
 */
void pv_write(char *str, int method, int handle) {
  var_t *vp;
  int l;

  switch (method) {
  case PV_FILE:
    dev_fwrite(handle, (byte *)str, strlen(str));
    break;
  case PV_LOG:
    lwrite(str);
    break;
  case PV_STRING:
    vp = (var_t*) (intptr_t) handle;
    l = strlen(str) + strlen(str) + 1;
    if (vp->v.p.size <= l) {
      vp->v.p.size = l + 128;
      vp->v.p.ptr = realloc(vp->v.p.ptr, vp->v.p.size);
    }
    strcat((char *) vp->v.p.ptr, str);
    break;
  case PV_NET:
    net_print((socket_t) handle, (const char *)str);
    break;
  default:
    dev_print(str);
  }
}

/*
 * just prints the value of variable 'var'
 */
void pv_writevar(var_t *var, int method, int handle) {
  char tmpsb[64];

  // start with a clean buffer
  memset(tmpsb, 0, sizeof(tmpsb));

  switch (var->type) {
  case V_STR:
    pv_write((char *)var->v.p.ptr, method, handle);
    break;
  case V_ARRAY:
  case V_MAP:
    map_write(var, method, handle);
    break;
  case V_PTR:
    ltostr(var->v.ap.p, tmpsb);
    pv_write(tmpsb, method, handle);
    break;
  case V_INT:
    ltostr(var->v.i, tmpsb);
    pv_write(tmpsb, method, handle);
    break;
  case V_NUM:
    ftostr(var->v.n, tmpsb);
    pv_write(tmpsb, method, handle);
    break;
  case V_REF:
    pv_writevar(var->v.ref, method, handle);
    break;
  }
}

/*
 * write a variable to console
 */
void print_var(var_t *v) {
  pv_writevar(v, PV_CONSOLE, 0);
}

/*
 * write a variable to a file
 */
void fprint_var(int handle, var_t *v) {
  pv_writevar(v, PV_FILE, handle);
}

/*
 * write a variable to log-file
 */
void logprint_var(var_t *v) {
  pv_writevar(v, PV_LOG, 0);
}

/*
 * skip parameter
 */
void par_skip() {
  byte exitf = 0, code;
  dword len;
  int level = 0;

  do {
    code = code_peek();
    switch (code) {
    case kwTYPE_INT:           // integer
      prog_ip += OS_INTSZ + 1;
      break;
    case kwTYPE_NUM:           // number
      prog_ip += OS_REALSZ + 1;
      break;
    case kwTYPE_STR:           // string: [2/4B-len][data]
      prog_ip++;
      memcpy(&len, prog_source + prog_ip, OS_STRLEN);
      len += OS_STRLEN;
      prog_ip += len;
      break;
    case kwTYPE_VAR:           // [addr|id]
      prog_ip += ADDRSZ + 1;
      break;
    case kwTYPE_CALLF:
      prog_ip += CODESZ + 1;
      break;
    case kwTYPE_CALLEXTF:
      prog_ip += (ADDRSZ * 2) + 1;
      break;
    case kwTYPE_LEVEL_BEGIN:   // left parenthesis
      level++;
      prog_ip++;
      break;
    case kwTYPE_LEVEL_END:     // right parenthesis
      prog_ip++;
      level--;
      if (level <= 0)
        exitf = 1;
      break;
    case kwTYPE_LOGOPR:
    case kwTYPE_CMPOPR:
    case kwTYPE_ADDOPR:
    case kwTYPE_MULOPR:
    case kwTYPE_POWOPR:
    case kwTYPE_UNROPR:        // [1B data]
      prog_ip += 2;
      break;
    case kwTYPE_CALL_UDF:      // [true-ip][false-ip]
      prog_ip += BC_CTRLSZ + 1;
      break;
    case kwTYPE_LINE:
    case kwTYPE_EOC:
    case kwUSE:
      if (level != 0)
        rt_raise("Block error!");
      exitf = 1;
      break;
    case kwTYPE_SEP:
      if (level <= 0)
        exitf = 1;
      else
        prog_ip += 2;
      break;
    default:
      prog_ip++;
    }
  } while (!exitf);
}

/*
 * get next parameter as var_t
 */
void par_getvar(var_t *var) {
  byte code;

  code = code_peek();
  switch (code) {
  case kwTYPE_LINE:
  case kwTYPE_EOC:
  case kwTYPE_SEP:
    err_syntax(-1, "%P");
    return;
  default:
    eval(var);
    break;
  };
}

/*
 * get next parameter as var_t/array
 */
var_t *par_getvarray() {
  byte code;
  var_t *var;

  code = code_peek();
  switch (code) {
  case kwTYPE_LINE:
  case kwTYPE_EOC:
  case kwTYPE_SEP:
    err_syntax(-1, "%P");
    return NULL;
  case kwTYPE_VAR:
    var = code_getvarptr();
    if (!prog_error) {
      if (var->type != V_ARRAY) {
        return NULL;
      }
    }
    return var;
  default:
    err_typemismatch();
    break;
  };
  return NULL;
}

/*
 * get next parameter as var_t
 */
var_t *par_getvar_ptr() {
  if (code_peek() != kwTYPE_VAR) {
    err_syntax(-1, "%P");
    return NULL;
  }
  return code_getvarptr();
}

/*
 * get next parameter as var_t
 */
void par_getstr(var_t *var) {
  byte code;

  code = code_peek();
  switch (code) {
  case kwTYPE_LINE:
  case kwTYPE_EOC:
  case kwTYPE_SEP:
    err_syntax(-1, "%S");
    return;
  default:
    eval(var);
    break;
  };

  if (var->type != V_STR) {
    v_tostr(var);
  }
}

/*
 * get next parameter as long
 */
var_int_t par_getint() {
  var_t var;
  var_int_t i;

  v_init(&var);
  par_getvar(&var);
  i = v_getint(&var);
  v_free(&var);

  return i;
}

/*
 * get next parameter as double
 */
var_num_t par_getnum() {
  var_t var;
  var_num_t f;

  v_init(&var);
  par_getvar(&var);
  f = v_getval(&var);
  v_free(&var);

  return f;
}

var_int_t par_getval(var_int_t def) {
  var_int_t result;
  byte code = code_peek();
  if (code == kwTYPE_VAR) {
    result = par_getint();
  } else if (code == kwTYPE_INT) {
    code_skipnext();
    result = code_getint();
  } else if (code == kwTYPE_NUM) {
    code_skipnext();
    result = code_getreal();
  } else {
    result = def;
  }
  return result;
}

/*
 * no-error if the next byte is separator
 * returns the separator
 */
int par_getsep() {
  int last_op;
  byte code;

  code = code_peek();
  switch (code) {
  case kwTYPE_SEP:
    code_skipnext();
    last_op = code_getnext();
    return last_op;
  default:
    err_missing_sep();
  };

  return 0;
}

/*
 * no-error if the next byte is the separator ','
 */
void par_getcomma() {
  if (par_getsep() != ',') {
    if (!prog_error) {
      err_missing_comma();
    }
  }
}

/*
 * no-error if the next byte is the separator ';'
 */
void par_getsemicolon() {
  if (par_getsep() != ';') {
    if (!prog_error) {
      err_syntaxsep(";");
    }
  }
}

/*
 * no-error if the next byte is the separator '#'
 */
void par_getsharp() {
  if (par_getsep() != '#') {
    if (!prog_error) {
      err_syntaxsep("#");
    }
  }
}

/*
 * retrieve a 2D point (double)
 */
pt_t par_getpt() {
  pt_t pt;
  var_t *var;
  byte alloc = 0;

  pt.x = pt.y = 0;

  // first parameter
  if (code_isvar()) {
    var = code_getvarptr();
  } else {
    alloc = 1;
    var = v_new();
    eval(var);
  }

  if (!prog_error) {
    if (var->type == V_ARRAY) {
      // array
      if (var->v.a.size != 2) {
        rt_raise(ERR_POLY_POINT);
      } else {
        pt.x = v_getreal(v_elem(var, 0));
        pt.y = v_getreal(v_elem(var, 1));
      }
    } else {
      // non-arrays
      pt.x = v_getreal(var);
      par_getcomma();
      if (!prog_error) {
        var_t v2;

        eval(&v2);
        if (!prog_error) {
          pt.y = v_getreal(&v2);
        }
        v_free(&v2);
      }
    }
  }
  // clean-up
  if (alloc) {
    v_free(var);
    free(var);
  }

  return pt;
}

/*
 * retrieve a 2D point (integer)
 */
ipt_t par_getipt() {
  ipt_t pt;
  var_t *var;
  byte alloc = 0;

  pt.x = pt.y = 0;

  // first parameter
  if (code_isvar()) {
    var = code_getvarptr();
  } else {
    alloc = 1;
    var = v_new();
    eval(var);
  }

  if (!prog_error) {
    if (var->type == V_ARRAY) {
      // array
      if (var->v.a.size != 2) {
        rt_raise(ERR_POLY_POINT);
      } else {
        pt.x = v_getint(v_elem(var, 0));
        pt.y = v_getint(v_elem(var, 1));
      }
    } else {
      // non-arrays
      pt.x = v_getint(var);
      par_getcomma();
      if (!prog_error) {
        var_t v2;

        eval(&v2);
        if (!prog_error)
          pt.y = v_getint(&v2);
        v_free(&v2);
      }
    }
  }
  // clean-up
  if (alloc) {
    v_free(var);
    free(var);
  }

  return pt;
}

/*
 * retrieve a 2D polyline
 */
int par_getpoly(pt_t **poly_pp) {
  pt_t *poly = NULL;
  var_t *var, *el;
  int count = 0;
  byte style = 0, alloc = 0;

  // get array
  if (code_isvar()) {
    var = par_getvarray();
    if (prog_error) {
      return 0;
    }
  } else {
    var = v_new();
    eval(var);
    alloc = 1;
  }

  // zero-length array
  if (var->v.a.size == 0) {
    if (alloc) {
      v_free(var);
      free(var);
    }
    return 0;
  }
  //
  el = v_elem(var, 0);
  if (el->type == V_ARRAY) {
    style = 1;                  // nested --- [ [x1,y1], [x2,y2], ... ]
  }
  // else
  // style = 0; // 2x2 or 1x --- [ x1, y1, x2, y2, ... ]

  // error check
  if (style == 1) {
    if (el->v.a.size != 2) {
      err_parsepoly(-1, 1);
      if (alloc) {
        v_free(var);
        free(var);
      }
      return 0;
    }

    count = var->v.a.size;
  } else if (style == 0) {
    if ((var->v.a.size % 2) != 0) {
      err_parsepoly(-1, 2);
      if (alloc) {
        v_free(var);
        free(var);
      }
      return 0;
    }

    count = var->v.a.size >> 1;
  }
  // build array
  *poly_pp = poly = malloc(sizeof(pt_t) * count);

  if (style == 1) {
    int i;

    for (i = 0; i < count; i++) {
      // get point
      el = v_elem(var, i);

      // error check
      if (el->type != V_ARRAY
        )
        err_parsepoly(i, 3);
      else if (el->v.a.size != 2)
        err_parsepoly(i, 4);
      if (prog_error)
        break;

      // store point
      poly[i].x = v_getreal(v_elem(el, 0));
      poly[i].y = v_getreal(v_elem(el, 1));
    }
  } else if (style == 0) {
    int i, j;

    for (i = j = 0; i < count; i++, j += 2) {
      // error check
      if (prog_error) {
        break;
      }
      // store point
      poly[i].x = v_getreal(v_elem(var, j));
      poly[i].y = v_getreal(v_elem(var, j + 1));
    }
  }
  // clean-up
  if (prog_error) {
    free(poly);
    *poly_pp = NULL;
    count = 0;
  }
  if (alloc) {
    v_free(var);
    free(var);
  }

  return count;
}

/*
 * retrieve a 2D polyline (integers)
 */
int par_getipoly(ipt_t **poly_pp) {
  ipt_t *poly = NULL;
  var_t *var, *el;
  int count = 0;
  byte style = 0, alloc = 0;

  // get array
  if (code_isvar()) {
    var = par_getvarray();
    if (prog_error) {
      return 0;
    }
  } else {
    var = v_new();
    eval(var);
    alloc = 1;
  }

  // zero-length array
  if (var->v.a.size == 0) {
    if (alloc) {
      v_free(var);
      free(var);
    }
    return 0;
  }
  //
  el = v_elem(var, 0);
  if (el && el->type == V_ARRAY) {
    style = 1;   // nested --- [ [x1,y1], [x2,y2], ... ]
  }
  // else
  // style = 0; // 2x2 or 1x --- [ x1, y1, x2, y2, ... ]

  // error check
  if (style == 1) {
    if (el->v.a.size != 2) {
      err_parsepoly(-1, 1);
      if (alloc) {
        v_free(var);
        free(var);
      }
      return 0;
    }

    count = var->v.a.size;
  } else if (style == 0) {
    if ((var->v.a.size % 2) != 0) {
      err_parsepoly(-1, 2);
      if (alloc) {
        v_free(var);
        free(var);
      }
      return 0;
    }

    count = var->v.a.size >> 1;
  }
  // build array
  *poly_pp = poly = malloc(sizeof(ipt_t) * count);

  if (style == 1) {
    int i;

    for (i = 0; i < count; i++) {
      // get point
      el = v_elem(var, i);

      // error check
      if (el->type != V_ARRAY) {
        err_parsepoly(i, 3);
      } else if (el->v.a.size != 2) {
        err_parsepoly(i, 4);
      }
      if (prog_error) {
        break;
      }
      // store point
      poly[i].x = v_getint(v_elem(el, 0));
      poly[i].y = v_getint(v_elem(el, 1));
    }
  } else if (style == 0) {
    int i, j;

    for (i = j = 0; i < count; i++, j += 2) {
      // error check
      if (prog_error) {
        break;
      }
      // store point
      poly[i].x = v_getint(v_elem(var, j));
      poly[i].y = v_getint(v_elem(var, j + 1));
    }
  }
  // clean-up
  if (prog_error) {
    free(poly);
    *poly_pp = NULL;
    count = 0;
  }
  if (alloc) {
    v_free(var);
    free(var);
  }

  return count;
}

/*
 * returns true if the following code is descibing one var code
 * usefull for optimization
 * (one var can be used by the pointer; more than one it must be evaluated)
 */
int par_isonevar() {
  return code_isvar();
}

/*
 * destroy a parameter-table which was created by par_getparlist
 */
void par_freepartable(par_t **ptable_pp, int pcount) {
  int i;
  par_t *ptable;

  ptable = *ptable_pp;
  if (ptable) {
    for (i = 0; i < pcount; i++) {
      if (ptable[i].flags & PAR_BYVAL) {
        v_free(ptable[i].var);
        free(ptable[i].var);
      }
    }

    free(ptable);
  }
  *ptable_pp = NULL;
}

/*
 * builds a parameter table
 *
 * ptable_pp = pointer to an ptable
 * valid_sep = valid separators (';)
 *
 * returns the number of the parameters, OR, -1 on error
 *
 * YOU MUST FREE THAT TABLE BY USING par_freepartable()
 * IF THERE IS NO ERROR, CALL TO par_freepartable IS NOT NEEDED
 */
int par_getpartable(par_t **ptable_pp, const char *valid_sep) {
  byte ready, last_sep = 0;
  par_t *ptable;
  bcip_t ofs;
  char vsep[8];
  var_t *par = NULL;
  int pcount = 0;

  /*
   *      initialize
   */
  ptable = *ptable_pp = malloc(sizeof(par_t) * 256);

  if (valid_sep) {
    strcpy(vsep, valid_sep);
  } else {
    strcpy(vsep, ",");
  }
  /*
   *      start
   */
  ready = 0;
  do {
    switch (code_peek()) {
    case kwTYPE_EOC:
    case kwTYPE_LINE:
    case kwUSE:
    case kwTYPE_LEVEL_END:     // end of parameters
      ready = 1;
      break;
    case kwTYPE_SEP:           // separator
      code_skipnext();

      // check parameters separator
      if (strchr(vsep, (last_sep = code_getnext())) == NULL) {
        if (strlen(vsep) <= 1) {
          err_missing_comma();
        } else {
          err_syntaxsep(vsep);
        }
        par_freepartable(ptable_pp, pcount);
        return -1;
      }
      // update par.next_sep
      if (pcount) {
        ptable[pcount - 1].next_sep = last_sep;
      }
      break;
    case kwTYPE_VAR:           // variable
      ofs = prog_ip;            // store IP

      ptable[pcount].flags = 0;
      ptable[pcount].prev_sep = last_sep;
      ptable[pcount].next_sep = 0;

      if (code_isvar()) {
        // push parameter
        ptable[pcount].var = code_getvarptr();
        pcount++;
        break;
      }
      // Its no a single variable, its an expression
      // restore IP
      prog_ip = ofs;

      // no 'break' here
    default:
      // default --- expression (BYVAL ONLY)
      par = v_new();
      eval(par);
      if (!prog_error) {
        // push parameter
        ptable[pcount].var = par;
        ptable[pcount].flags |= PAR_BYVAL;
        pcount++;
      } else {
        v_free(par);
        free(par);
        par_freepartable(ptable_pp, pcount);
        return -1;
      }
    }

  } while (!ready);

  return pcount;
}

/*
 */
int par_massget_type_check(char fmt, par_t *par) {
  switch (fmt) {
  case 'S':
  case 's':
    return (par->var->type == V_STR);
  case 'I':
  case 'i':
  case 'F':
  case 'f':
    return (par->var->type == V_NUM || par->var->type == V_INT);
  }
  return 0;
}

/*
 * Parsing parameters with scanf-style
 * returns the parameter-count or -1 (error)
 *
 * Format:
 * --------
 * capital character = the parameter is required
 * small character   = optional parameter
 *
 * I = var_int_t       (var_int_t*)
 * F = var_num_t       (var_num_t*)
 * S = string          (char*)
 * P = variable's ptr  (var_t*)
 *
 * Example:
 * --------
 * var_int_t i1, i2 = -1;    // -1 is the default value for i2
 * char      *s1 = NULL;     // NULL is the default value for s1
 * var_t     *v  = NULL;     // NULL is the default value for v
 *
 * // the first integer is required, the second is optional
 * // the string is optional too
 * pc = par_massget("Iis", &i1, &i2, &s1, &v);
 *
 * if ( pc != -1 ) {
 *   // no error; also, you can use prog_error because par_massget() will call rt_raise() on error
 *   printf("required integer = %d\n", i1);
 *
 *   // if there is no optional parameters, the default value will be returned
 *   if  ( i2 != -1 )    printf("optional integer found = %d\n", i2);
 *   if  ( s1 )          printf("optional string found = %s\n", s1);
 *   if  ( v )       {   printf("optional variable's ptr found");    v_free(v);  }
 *   }
 *
 * pfree2(s1, v);
 */
int par_massget(const char *fmt, ...) {
  char *fmt_p = NULL;
  int pcount, rqcount, optcount, curpar;
  int opt = 0, ignore = 0;
  va_list ap;
  par_t *ptable;

  char **s;
  var_int_t *i;
  var_num_t *f;
  var_t **vt;

  // get ptable
  pcount = par_getpartable(&ptable, NULL);
  if (pcount == -1) {
    free(ptable);
    return -1;
  }

  /*
   *      count pars
   */
  fmt_p = (char *)fmt;
  rqcount = optcount = 0;
  while (*fmt_p) {
    if (*fmt_p >= 'a') {
      optcount++;
    } else {
      rqcount++;
    }
    fmt_p++;
  }

  if (rqcount > pcount) {
    err_parfmt(fmt);
  }
  else {
    /*
     *      parse
     */
    va_start(ap, fmt);
    curpar = 0;
    fmt_p = (char *)fmt;
    while (*fmt_p) {
      if (*fmt_p >= 'a' && optcount &&
          ((curpar < pcount) ? par_massget_type_check(*fmt_p, &ptable[curpar]) : 0)) {
        (optcount--, opt = 1, ignore = 0);
      }
      else if (*fmt_p >= 'a' && optcount) {
        (optcount--, opt = 0, ignore = 1);
      }
      else if (*fmt_p < 'a' && rqcount) {
        (rqcount--, opt = 0, ignore = 0);
      }
      else {
        err_parfmt(fmt);
        break;
      }

      if (pcount <= curpar && ignore == 0) {
        err_parfmt(fmt);
        // rt_raise("%s\nb: pc=%d, oc=%d, rc=%d", fmt, pcount, optcount, rqcount);
        break;
      }

      switch (*fmt_p) {
      case 's':
        // optional string
        if (!opt) {
          s = va_arg(ap, char**);
          break;
        }
      case 'S':
        // string
        s = va_arg(ap, char**);
        *s = strdup(v_getstr(ptable[curpar].var));
        curpar++;
        break;
      case 'i':
        // optional integer
        if (!opt) {
          i = va_arg(ap, var_int_t*);
          break;
        }
      case 'I':
        // integer
        i = va_arg(ap, var_int_t*);
        *i = v_getint(ptable[curpar].var);
        curpar++;
        break;
      case 'f':
        // optional real (var_num_t)
        if (!opt) {
          f = va_arg(ap, var_num_t*);
          break;
        }
      case 'F':
        // real (var_num_t)
        f = va_arg(ap, var_num_t*);
        *f = v_getnum(ptable[curpar].var);
        curpar++;
        break;
      case 'p':
        // optional variable
        if (!opt) {
          vt = va_arg(ap, var_t**);
          break;
        }
      case 'P':
        // variable
        vt = va_arg(ap, var_t**);
        if (ptable[curpar].flags == 0)// byref
          *vt = ptable[curpar].var;
        else {
          err_syntax(-1, "%P");
          break;
        }
        curpar++;
        break;
      }

      fmt_p++;
    }
    va_end(ap);
  }

  par_freepartable(&ptable, pcount);
  if (prog_error) {
    return -1;
  }
  return pcount;
}
