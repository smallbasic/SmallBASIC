// This file is part of SmallBASIC
//
// SmallBASIC - External library support (modules)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2001 Nicholas Christopoulos

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(__CYGWIN__)
#include <w32api/windows.h>
#include <sys/cygwin.h>
#define WIN_EXTLIB
#elif defined(__MINGW32__)
#include <windows.h>
#define WIN_EXTLIB
#endif

#include "common/smbas.h"
#include "common/extlib.h"
#include "common/pproc.h"

#if defined(__linux__) && defined(_UnixOS)
#define LNX_EXTLIB
#endif

#ifdef LNX_EXTLIB
#define LIB_EXT ".so"
#else
#define LIB_EXT ".dll"
#endif

#if defined(LNX_EXTLIB)
#include <dlfcn.h>
#endif

#include <dirent.h>

#define MAX_SLIB_N 256
#define MAX_PARAM 64

#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
static slib_t slib_table[MAX_SLIB_N]; /**< module index */
static int slib_count; /**< module count */
static ext_proc_node_t *extproctable; /**< external procedure table       */
static int extprocsize; /**< ext-proc table allocated size  */
static int extproccount; /**< ext-proc table count           */
static ext_func_node_t *extfunctable; /**< external function table        */
static int extfuncsize; /**< ext-func table allocated size  */
static int extfunccount; /**< ext-func table count           */

/**
 * add an external procedure to the list
 */
static int slib_add_external_proc(const char *proc_name, int lib_id) {
  // TODO: scan for conflicts
  char buf[256];

  sprintf(buf, "%s.%s", slib_table[lib_id].name, proc_name);
  strupper(buf);

  if (extproctable == NULL) {
    extprocsize = 16;
    extproctable =
      (ext_proc_node_t *) malloc(sizeof(ext_proc_node_t) * extprocsize);
  }
  else if (extprocsize <= (extproccount + 1)) {
    extprocsize += 16;
    extproctable =
      (ext_proc_node_t *) realloc(extproctable,
                                  sizeof(ext_proc_node_t) * extprocsize);
  }

  extproctable[extproccount].lib_id = lib_id;
  extproctable[extproccount].symbol_index = 0;
  strcpy(extproctable[extproccount].name, buf);
  strupper(extproctable[extproccount].name);

  if (opt_verbose) {
    log_printf("LID: %d, Idx: %d, PROC '%s'\n", lib_id, extproccount,
               extproctable[extproccount].name);
  }
  extproccount++;
  return extproccount - 1;
}

/**
 * Add an external function to the list
 */
static int slib_add_external_func(const char *func_name, int lib_id) {
  char buf[256];

  sprintf(buf, "%s.%s", slib_table[lib_id].name, func_name);
  strupper(buf);

  // TODO: scan for conflicts
  if (extfunctable == NULL) {
    extfuncsize = 16;
    extfunctable =
      (ext_func_node_t *) malloc(sizeof(ext_func_node_t) * extfuncsize);
  }
  else if (extfuncsize <= (extfunccount + 1)) {
    extfuncsize += 16;
    extfunctable =
      (ext_func_node_t *) realloc(extfunctable,
                                  sizeof(ext_func_node_t) * extfuncsize);
  }

  extfunctable[extfunccount].lib_id = lib_id;
  extfunctable[extfunccount].symbol_index = 0;
  strcpy(extfunctable[extfunccount].name, buf);
  strupper(extfunctable[extfunccount].name);

  if (opt_verbose) {
    log_printf("LID: %d, Idx: %d, FUNC '%s'\n", lib_id, extfunccount,
               extfunctable[extfunccount].name);
  }
  extfunccount++;
  return extfunccount - 1;
}

#endif

/**
 * returns the ID of the keyword
 */
int slib_get_kid(const char *name) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  int i;

  for (i = 0; i < extproccount; i++) {
    if (strcmp(extproctable[i].name, name) == 0) {
      return i;
    }
  }
  for (i = 0; i < extfunccount; i++) {
    if (strcmp(extfunctable[i].name, name) == 0) {
      return i;
    }
  }
#endif
  return -1;
}

/**
 * returns the library-id (index of library of the current process)
 */
int slib_get_module_id(const char *name) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  int i;
  char xname[OS_FILENAME_SIZE + 1];
  slib_t *lib;

  strcpy(xname, name);
  strcat(xname, LIB_EXT);
  for (i = 0; i < slib_count; i++) {
    lib = &slib_table[i];
    if (strcasecmp(lib->name, name) == 0) {
      return i;
    }
  }
#endif
  // not found
  return -1;
}

/**
 * updates compiler with the module's keywords
 */
void slib_setup_comp(int mid) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  int i;

  for (i = 0; i < extproccount; i++) {
    if (extproctable[i].lib_id == mid) {
      comp_add_external_proc(extproctable[i].name, mid);
    }
  }
  for (i = 0; i < extfunccount; i++) {
    if (extfunctable[i].lib_id == mid) {
      comp_add_external_func(extfunctable[i].name, mid);
    }
  }
#endif
}

/**
 * retrieve the function pointer
 */
void *slib_getoptptr(slib_t *lib, const char *name) {
#if defined(LNX_EXTLIB)
  return dlsym(lib->handle, name);
#elif defined(WIN_EXTLIB)
  return GetProcAddress((HMODULE) lib->handle, name);
#else
  return NULL;
#endif
}

/**
 * open library (basic open)
 */
int slib_llopen(slib_t *lib) {
#if defined(LNX_EXTLIB)
  lib->handle = dlopen(lib->fullname, RTLD_NOW);
  if (lib->handle == NULL) {
    log_printf("lib: error on loading %s\n%s", lib->name, dlerror());
  }
  return (lib->handle != NULL);
#elif defined(__CYGWIN__)
  char win32Path[1024];
  cygwin_conv_path(CCP_POSIX_TO_WIN_A, lib->fullname, win32Path, sizeof(win32Path));
  lib->handle = LoadLibrary(win32Path);
  if (lib->handle == NULL) {
    log_printf("lib: error on loading %s\n", win32Path);
  }
  return (lib->handle != NULL);
#elif defined(WIN_EXTLIB)
  lib->handle = LoadLibrary(lib->fullname);
  if (lib->handle == NULL) {
    log_printf("lib: error on loading %s\n", lib->name);
  }
  return (lib->handle != NULL);
#else
  return 0;
#endif
}

/**
 * close library (basic close)
 */
int slib_llclose(slib_t *lib) {
#if defined(LNX_EXTLIB)
  if (!lib->handle) {
    return 0;
  }
  dlclose(lib->handle);
  lib->handle = NULL;
  return 1;
#elif defined(WIN_EXTLIB)
  if (!lib->handle) {
    return 0;
  }
  FreeLibrary(lib->handle);
  lib->handle = NULL;
  return 1;
#else
  return 1;
#endif
}

void slib_import_routines(slib_t *lib) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  int i, count;
  char buf[SB_KEYWORD_SIZE];
  int (*fgetname) (int, char *);
  int (*fcount) (void);

  lib->first_proc = extproccount;
  lib->first_func = extfunccount;

  fcount = slib_getoptptr(lib, "sblib_proc_count");
  fgetname = slib_getoptptr(lib, "sblib_proc_getname");
  if (fcount) {
    count = fcount();
    for (i = 0; i < count; i++) {
      if (fgetname(i, buf)) {
        slib_add_external_proc(buf, lib->id);
      }
    }
  }
  fcount = slib_getoptptr(lib, "sblib_func_count");
  fgetname = slib_getoptptr(lib, "sblib_func_getname");
  if (fcount) {
    count = fcount();
    for (i = 0; i < count; i++) {
      if (fgetname(i, buf)) {
        slib_add_external_func(buf, lib->id);
      }
    }
  }
#endif
}

/**
 * load a lib
 */
void slib_import(const char *name, const char *fullname) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  slib_t *lib;
  int (*minit) (void);
  const char *(*get_module_name) (void);
  int mok = 0;
  int name_index = 0;

  if (strncmp(name, "lib", 3) == 0) {
    // libmysql -> store mysql
    name_index = 3;
  }

  lib = &slib_table[slib_count];
  memset(lib, 0, sizeof(slib_t));
  strncpy(lib->name, name + name_index, 255);
  strncpy(lib->fullname, fullname, 1023);
  lib->id = slib_count;

  if (!opt_quiet) {
    log_printf("lib: importing %s", fullname);
  }
  if (slib_llopen(lib)) {
    mok = 1;

    // init
    minit = slib_getoptptr(lib, "sblib_init");
    if (minit) {
      if (!minit()) {
        mok = 0;
        log_printf("lib: %s->sblib_init(), failed", lib->name);
      }
    }

    // override default name
    get_module_name = slib_getoptptr(lib, "sblib_get_module_name");
    if (get_module_name) {
      strncpy(lib->name, get_module_name(), 255);
    }

    slib_import_routines(lib);
    mok = 1;
  }
  else {
    log_printf("lib: can't open %s", fullname);
  }
  if (mok) {
    slib_count++;
    if (!opt_quiet) {
      log_printf("... done\n");
    }
  }
  else {
    if (!opt_quiet) {
      log_printf("... error\n");
    }
  }
#endif
}

/**
 * scan libraries
 */
void sblmgr_scanlibs(const char *path) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  DIR *dp;
  struct dirent *e;
  char *name, *p;
  char full[1024], libname[256];

  if ((dp = opendir(path)) == NULL) {
    if (!opt_quiet) {
      log_printf("lib: module path %s not found.\n", path);
    }
    return;
  }

  while ((e = readdir(dp)) != NULL) {
    name = e->d_name;
    if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0)) {
      continue;
    }
    if ((p = strstr(name, LIB_EXT)) != NULL) {
      if (strcmp(p, LIB_EXT) == 0) {
        // store it
        strcpy(libname, name);
        p = strchr(libname, '.');
        *p = '\0';
        strcpy(full, path);
        if (path[strlen(path) - 1] != '/') {
          // add trailing separator
          strcat(full, "/");
        }
        strcat(full, name);
        slib_import(libname, full);
      }
    }
  }
  closedir(dp);
#endif
}

/**
 * slib-manager: initialize manager
 */
void sblmgr_init(int mcount, const char *mlist) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  slib_count = 0;
  if (mcount && mlist && mlist[0] != '\0') {
    if (!opt_quiet) {
      log_printf("lib: scanning for modules...\n");
    }
    // the -m argument specifies the location of all modules
    sblmgr_scanlibs(mlist);
  }
#endif
}

/**
 * slib-manager: close everything
 */
void sblmgr_close() {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  int i;
  slib_t *lib;
  void (*mclose) (void);

  for (i = 0; i < slib_count; i++) {
    lib = &slib_table[i];
    if (lib->handle) {
      mclose = slib_getoptptr(lib, "sblib_close");
      if (mclose) {
        mclose();
      }
      slib_llclose(lib);
    }
  }
  if (slib_count) {
    slib_count = 0;
    extproctable = NULL;
    extprocsize = 0;
    extproccount = 0;
    extfunctable = NULL;
    extfuncsize = 0;
    extfunccount = 0;
  }
#endif
}

/**
 * returns the 'index' function-name of the 'lib'
 */
int sblmgr_getfuncname(int lib_id, int index, char *buf) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  slib_t *lib;
  int (*mgf) (int, char *);

  buf[0] = '\0';
  if (lib_id < 0 || lib_id >= slib_count) {
    return 0;
  }
  lib = &slib_table[lib_id];
  mgf = slib_getoptptr(lib, "sblib_func_getname");
  if (mgf == NULL) {
    return 0;
  }
  return mgf(index, buf);
#else
  return 0;
#endif
}

/**
 * returns the 'index' procedure-name of the 'lib'
 */
int sblmgr_getprocname(int lib_id, int index, char *buf) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  slib_t *lib;
  int (*mgp) (int, char *);

  buf[0] = '\0';
  if (lib_id < 0 || lib_id >= slib_count) {
    return 0;
  }
  lib = &slib_table[lib_id];
  mgp = slib_getoptptr(lib, "sblib_proc_getname");
  if (mgp == NULL) {
    return 0;
  }
  return mgp(index, buf);
#else
  return 0;
#endif
}

/**
 * build parameter table
 */
int slib_build_ptable(slib_par_t *ptable) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  int pcount = 0;
  var_t *arg = NULL;
  byte ready, code;
  bcip_t ofs;

  if (code_peek() == kwTYPE_LEVEL_BEGIN) {
    code_skipnext();
    ready = 0;
    do {
      code = code_peek();
      switch (code) {
      case kwTYPE_EOC:
        code_skipnext();
        break;
      case kwTYPE_SEP:
        code_skipsep();
        break;
      case kwTYPE_LEVEL_END:
        ready = 1;
        break;
      case kwTYPE_VAR:
        // variable
        ofs = prog_ip;

        if (code_isvar()) {
          // push parameter
          ptable[pcount].var_p = code_getvarptr();
          ptable[pcount].byref = 1;
          pcount++;
          break;
        }

        // restore IP
        prog_ip = ofs;
        // no 'break' here
      default:
        // default --- expression (BYVAL ONLY)
        arg = v_new();
        eval(arg);
        if (!prog_error) {
          // push parameter
          ptable[pcount].var_p = arg;
          ptable[pcount].byref = 0;
          pcount++;
        }
        else {
          v_free(arg);
          v_detach(arg);
          return pcount;
        }
      }
    } while (!ready);
    // kwTYPE_LEVEL_END
    code_skipnext();
  }

  return pcount;
#else
  return 0;
#endif
}

/**
 * free parameter table
 */
void slib_free_ptable(slib_par_t *ptable, int pcount) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  int i;

  for (i = 0; i < pcount; i++) {
    if (ptable[i].byref == 0) {
      v_free(ptable[i].var_p);
      v_detach(ptable[i].var_p);
    }
  }
#endif
}

/**
 * execute a procedure
 */
int sblmgr_procexec(int lib_id, int index) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  slib_t *lib;
  var_t ret;
  slib_par_t *ptable = NULL;
  int (*pexec) (int, int, slib_par_t *, var_t *);
  int pcount = 0;
  int success = 0;

  if (lib_id < 0 || lib_id >= slib_count) {
    return 0;
  }

  lib = &slib_table[lib_id];
  pexec = slib_getoptptr(lib, "sblib_proc_exec");
  if (pexec == NULL) {
    return 0;
  }

  // build parameter table
  ptable = malloc(sizeof(slib_par_t) * MAX_PARAM);
  pcount = slib_build_ptable(ptable);
  if (prog_error) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
    return 0;
  }

  // exec
  v_init(&ret);
  success = pexec(index - lib->first_proc, pcount, ptable, &ret);

  // error
  if (!success) {
    if (ret.type == V_STR) {
      rt_raise("lib:%s: %s\n", lib->name, ret.v.p.ptr);
    } else {
      rt_raise("lib:%s: Unspecified error\n", lib->name);
    }
  }
  // clean-up
  if (ptable) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
  }

  v_free(&ret);
  return success;
#else
  return 0;
#endif
}

/**
 * execute a function
 */
int sblmgr_funcexec(int lib_id, int index, var_t *ret) {
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
  slib_t *lib;
  slib_par_t *ptable = NULL;
  int (*pexec) (int, int, slib_par_t *, var_t *);
  int pcount = 0;
  int success = 0;

  if (lib_id < 0 || lib_id >= slib_count) {
    return 0;
  }

  lib = &slib_table[lib_id];
  pexec = slib_getoptptr(lib, "sblib_func_exec");
  if (pexec == NULL) {
    return 0;
  }

  // build parameter table
  ptable = malloc(sizeof(slib_par_t) * MAX_PARAM);
  pcount = slib_build_ptable(ptable);
  if (prog_error) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
    return 0;
  }

  // exec
  v_init(ret);
  success = pexec(index - lib->first_func, pcount, ptable, ret);

  // error
  if (!success) {
    if (ret->type == V_STR) {
      rt_raise("lib:%s: %s\n", lib->name, ret->v.p.ptr);
    } else {
      rt_raise("lib:%s: Unspecified error\n", lib->name);
    }
  }
  // clean-up
  if (ptable) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
  }

  return success;
#else
  return 0;
#endif
}
