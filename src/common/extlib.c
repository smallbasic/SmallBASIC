// This file is part of SmallBASIC
//
// SmallBASIC - External library support (plugins)
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
 #define LIB_EXT ".dll"
 #define DEFAULT_PATH "c:/sbasic/lib"
#elif defined(__MINGW32__)
 #include <windows.h>
 #define WIN_EXTLIB
 #define LIB_EXT ".dll"
 #define DEFAULT_PATH "c:/sbasic/lib"
#elif defined(__linux__) && defined(_UnixOS)
 #include <dlfcn.h>
 #define LNX_EXTLIB
 #define LIB_EXT ".so"
 #define DEFAULT_PATH "/usr/lib/sbasic/modules:/usr/local/lib/sbasic/modules"
#endif

#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
#include "common/smbas.h"
#include "common/extlib.h"
#include "common/pproc.h"
#include <dirent.h>

#define MAX_SLIBS 256
#define MAX_PARAM 64
#define TABLE_GROW_SIZE 16
#define NAME_SIZE 256
#define PATH_SIZE 1024

typedef int (*sblib_events_fn)(int);

typedef struct {
  char name[NAME_SIZE];
  char fullname[PATH_SIZE];
  void *handle;
  sblib_events_fn dev_events;
  uint32_t id;
  uint32_t flags;
  uint32_t proc_count;
  uint32_t func_count;
  uint32_t first_proc;
  uint32_t first_func;
} slib_t;

static slib_t slib_table[MAX_SLIBS];
static int slib_count;
static ext_proc_node_t *extproctable;
static int extprocsize;
static int extproccount;
static ext_func_node_t *extfunctable;
static int extfuncsize;
static int extfunccount;

#if defined(LNX_EXTLIB)
int slib_llopen(slib_t *lib) {
  lib->handle = dlopen(lib->fullname, RTLD_NOW);
  if (lib->handle == NULL) {
    sc_raise("LIB: error on loading %s\n%s", lib->name, dlerror());
  }
  return (lib->handle != NULL);
}

void *slib_getoptptr(slib_t *lib, const char *name) {
  return dlsym(lib->handle, name);
}

int slib_llclose(slib_t *lib) {
  if (!lib->handle) {
    return 0;
  }
  dlclose(lib->handle);
  lib->handle = NULL;
  return 1;
}

#else // WIN_EXTLIB
#if defined(__CYGWIN__)
int slib_llopen(slib_t *lib) {
  char win32Path[PATH_SIZE];
  cygwin_conv_path(CCP_POSIX_TO_WIN_A, lib->fullname, win32Path, sizeof(win32Path));
  lib->handle = LoadLibrary(win32Path);
  if (lib->handle == NULL) {
    sc_raise("LIB: error on loading %s\n", win32Path);
  }
  return (lib->handle != NULL);
}

#elif defined(WIN_EXTLIB)
int slib_llopen(slib_t *lib) {
  lib->handle = LoadLibrary(lib->fullname);
  if (lib->handle == NULL) {
    sc_raise("LIB: error on loading %s\n", lib->name);
  }
  return (lib->handle != NULL);
}
#endif

void *slib_getoptptr(slib_t *lib, const char *name) {
  return GetProcAddress((HMODULE) lib->handle, name);
}

int slib_llclose(slib_t *lib) {
  if (!lib->handle) {
    return 0;
  }
  FreeLibrary(lib->handle);
  lib->handle = NULL;
  return 1;
}
#endif

/**
 * add an external procedure to the list
 */
int slib_add_external_proc(const char *proc_name, int lib_id) {
  char buf[NAME_SIZE];
  sprintf(buf, "%s.%s", slib_table[lib_id].name, proc_name);
  strupper(buf);

  // scan for conflicts
  for (int i = 0; i < extproccount; i++) {
    if (strcmp(extproctable[i].name, buf) == 0) {
      sc_raise("LIB: duplicate proc %s", buf);
      return -1;
    }
  }
  if (extproctable == NULL) {
    extprocsize = TABLE_GROW_SIZE;
    extproctable = (ext_proc_node_t *)malloc(sizeof(ext_proc_node_t) * extprocsize);
  } else if (extprocsize <= (extproccount + 1)) {
    extprocsize += TABLE_GROW_SIZE;
    extproctable = (ext_proc_node_t *)
                   realloc(extproctable, sizeof(ext_proc_node_t) * extprocsize);
  }

  extproctable[extproccount].lib_id = lib_id;
  extproctable[extproccount].symbol_index = 0;
  strlcpy(extproctable[extproccount].name, buf, sizeof(extproctable[extproccount].name));
  strupper(extproctable[extproccount].name);

  if (opt_verbose) {
    log_printf("LIB: %d, Idx: %d, PROC '%s'\n", lib_id, extproccount,
               extproctable[extproccount].name);
  }
  extproccount++;
  return extproccount - 1;
}

/**
 * Add an external function to the list
 */
int slib_add_external_func(const char *func_name, int lib_id) {
  char buf[NAME_SIZE];
  sprintf(buf, "%s.%s", slib_table[lib_id].name, func_name);
  strupper(buf);

  // scan for conflicts
  for (int i = 0; i < extfunccount; i++) {
    if (strcmp(extfunctable[i].name, buf) == 0) {
      sc_raise("LIB: duplicate func %s", buf);
      return -1;
    }
  }
  if (extfunctable == NULL) {
    extfuncsize = TABLE_GROW_SIZE;
    extfunctable = (ext_func_node_t *)malloc(sizeof(ext_func_node_t) * extfuncsize);
  } else if (extfuncsize <= (extfunccount + 1)) {
    extfuncsize += TABLE_GROW_SIZE;
    extfunctable = (ext_func_node_t *)
                   realloc(extfunctable, sizeof(ext_func_node_t) * extfuncsize);
  }

  extfunctable[extfunccount].lib_id = lib_id;
  extfunctable[extfunccount].symbol_index = 0;
  strlcpy(extfunctable[extfunccount].name, buf, sizeof(extfunctable[extfunccount].name));
  strupper(extfunctable[extfunccount].name);

  if (opt_verbose) {
    log_printf("LIB: %d, Idx: %d, FUNC '%s'\n", lib_id, extfunccount,
               extfunctable[extfunccount].name);
  }
  extfunccount++;
  return extfunccount - 1;
}

/**
 * returns the ID of the keyword
 */
int slib_get_kid(const char *name) {
  for (int i = 0; i < extproccount; i++) {
    if (strcmp(extproctable[i].name, name) == 0) {
      return i;
    }
  }
  for (int i = 0; i < extfunccount; i++) {
    if (strcmp(extfunctable[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

/**
 * returns the library-id (index of library of the current process)
 */
int slib_get_module_id(const char *name, const char *alias) {
  for (int i = 0; i < slib_count; i++) {
    slib_t *lib = &slib_table[i];
    if (strcasecmp(lib->name, name) == 0) {
      strcpy(lib->name, alias);
      return i;
    }
  }
  // not found
  return -1;
}

/**
 * updates compiler with the module's keywords
 */
void slib_setup_comp(int mid) {
  for (int i = 0; i < extproccount; i++) {
    if (extproctable[i].lib_id == mid) {
      comp_add_external_proc(extproctable[i].name, mid);
    }
  }
  for (int i = 0; i < extfunccount; i++) {
    if (extfunctable[i].lib_id == mid) {
      comp_add_external_func(extfunctable[i].name, mid);
    }
  }
}

void slib_import_routines(slib_t *lib) {
  char buf[SB_KEYWORD_SIZE];
  int (*fgetname) (int, char *);
  int (*fcount) (void);

  lib->first_proc = extproccount;
  lib->first_func = extfunccount;
  lib->dev_events = slib_getoptptr(lib, "sblib_events");

  fcount = slib_getoptptr(lib, "sblib_proc_count");
  fgetname = slib_getoptptr(lib, "sblib_proc_getname");

  if (fcount && fgetname) {
    int count = fcount();
    for (int i = 0; i < count; i++) {
      if (fgetname(i, buf)) {
        slib_add_external_proc(buf, lib->id);
      }
    }
  }

  fcount = slib_getoptptr(lib, "sblib_func_count");
  fgetname = slib_getoptptr(lib, "sblib_func_getname");

  if (fcount && fgetname) {
    int count = fcount();
    for (int i = 0; i < count; i++) {
      if (fgetname(i, buf)) {
        slib_add_external_func(buf, lib->id);
      }
    }
  }
}

/**
 * load a lib
 */
void slib_import(const char *name, const char *fullname) {
  int (*minit) (void);
  const char *(*get_module_name) (void);
  int mok = 0;
  int name_index = 0;

  if (strncmp(name, "lib", 3) == 0) {
    // libmysql -> store mysql
    name_index = 3;
  }

  slib_t *lib = &slib_table[slib_count];
  memset(lib, 0, sizeof(slib_t));
  strlcpy(lib->name, name + name_index, NAME_SIZE);
  strlcpy(lib->fullname, fullname, PATH_SIZE);
  lib->id = slib_count;

  if (!opt_quiet) {
    log_printf("LIB: importing %s", fullname);
  }
  if (slib_llopen(lib)) {
    mok = 1;

    // init
    minit = slib_getoptptr(lib, "sblib_init");
    if (minit) {
      if (!minit()) {
        mok = 0;
        sc_raise("LIB: %s->sblib_init(), failed", lib->name);
      }
    }

    // override default name
    get_module_name = slib_getoptptr(lib, "sblib_get_module_name");
    if (get_module_name) {
      strlcpy(lib->name, get_module_name(), NAME_SIZE);
    }

    slib_import_routines(lib);
    mok = 1;
  } else {
    sc_raise("LIB: can't open %s", fullname);
  }
  if (mok) {
    slib_count++;
    if (!opt_quiet) {
      log_printf("... done\n");
    }
  } else {
    if (!opt_quiet) {
      log_printf("... error\n");
    }
  }
}

void slib_import_path(const char *path, const char *name) {
  char *p;
  if ((p = strstr(name, LIB_EXT)) != NULL) {
    if (strcmp(p, LIB_EXT) == 0) {
      // ends with LIB_EXT
      char full[PATH_SIZE];
      char libname[NAME_SIZE];

      // copy name without extension
      strlcpy(libname, name, sizeof(libname));
      p = strchr(libname, '.');
      *p = '\0';

      // copy full path to name
      strlcpy(full, path, sizeof(full));
      if (path[strlen(path) - 1] != '/') {
        // add trailing separator
        strlcat(full, "/", sizeof(full));
      }
      strlcat(full, name, sizeof(full));
      slib_import(libname, full);
    }
  }
}

/**
 * scan libraries
 */
void slib_scan_libs(const char *path) {
  DIR *dp;
  struct dirent *e;

  if ((dp = opendir(path)) == NULL) {
    if (!opt_quiet) {
      log_printf("LIB: module path %s not found.\n", path);
    }
  } else {
    while ((e = readdir(dp)) != NULL) {
      char *name = e->d_name;
      if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
        slib_import_path(path, name);
      }
    }
    closedir(dp);
  }
}

void slib_scan_path(const char *path) {
  struct stat stbuf;
  if (stat(path, &stbuf) != -1) {
    if (S_ISREG(stbuf.st_mode)) {
      char *name = strrchr(path, '/');
      slib_import_path(path, (name ? name + 1 : path));
    } else {
      slib_scan_libs(path);
    }
  }
}

void slib_init_path() {
  char *path = opt_modpath;
  while (path && path[0] != '\0') {
    char *sep = strchr(path, ':');
    if (sep) {
      // null terminate the current path
      *sep = '\0';
      slib_scan_path(path);
      path = sep + 1;
    } else {
      slib_scan_path(path);
      path = NULL;
    }
  }
}

/**
 * slib-manager: initialize manager
 */
void slib_init() {
  slib_count = 0;

  if (!prog_error && opt_loadmod) {
    if (!opt_quiet) {
      log_printf("LIB: scanning for modules...\n");
    }

    if (opt_modpath[0] == '\0') {
      const char *modpath = getenv("SBASICPATH");
      if (modpath != NULL) {
        strlcpy(opt_modpath, modpath, OPT_MOD_SZ);
      }
    }

    if (opt_modpath[0] == '\0') {
      strcpy(opt_modpath, DEFAULT_PATH);
    }

    slib_init_path();
  }
}

/**
 * slib-manager: close everything
 */
void slib_close() {
  void (*mclose) (void);

  for (int i = 0; i < slib_count; i++) {
    slib_t *lib = &slib_table[i];
    if (lib->handle) {
      mclose = slib_getoptptr(lib, "sblib_close");
      if (mclose) {
        mclose();
      }
      slib_llclose(lib);
    }
  }
  if (slib_count) {
    free(extproctable);
    free(extfunctable);
    slib_count = 0;
    extproctable = NULL;
    extprocsize = 0;
    extproccount = 0;
    extfunctable = NULL;
    extfuncsize = 0;
    extfunccount = 0;
  }
}

/**
 * returns the 'index' function-name of the 'lib'
 */
int slib_getfuncname(int lib_id, int index, char *buf) {
  int (*mgf) (int, char *);

  buf[0] = '\0';
  if (lib_id < 0 || lib_id >= slib_count) {
    return 0;
  }
  slib_t *lib = &slib_table[lib_id];
  mgf = slib_getoptptr(lib, "sblib_func_getname");
  return !mgf ? 0 : mgf(index, buf);
}

/**
 * returns the 'index' procedure-name of the 'lib'
 */
int slib_getprocname(int lib_id, int index, char *buf) {
  int (*mgp) (int, char *);

  buf[0] = '\0';
  if (lib_id < 0 || lib_id >= slib_count) {
    return 0;
  }
  slib_t *lib = &slib_table[lib_id];
  mgp = slib_getoptptr(lib, "sblib_proc_getname");
  return !mgp ? 0 : mgp(index, buf);
}

/**
 * build parameter table
 */
int slib_build_ptable(slib_par_t *ptable) {
  int pcount = 0;
  var_t *arg;
  bcip_t ofs;

  if (code_peek() == kwTYPE_LEVEL_BEGIN) {
    code_skipnext();
    byte ready = 0;
    do {
      byte code = code_peek();
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
        } else {
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
}

/**
 * free parameter table
 */
void slib_free_ptable(slib_par_t *ptable, int pcount) {
  for (int i = 0; i < pcount; i++) {
    if (ptable[i].byref == 0) {
      v_free(ptable[i].var_p);
      v_detach(ptable[i].var_p);
    }
  }
}

/**
 * execute a procedure
 */
int slib_procexec(int lib_id, int index) {
  if (lib_id < 0 || lib_id >= slib_count) {
    return 0;
  }

  int (*pexec) (int, int, slib_par_t *, var_t *);
  slib_t *lib = &slib_table[lib_id];
  pexec = slib_getoptptr(lib, "sblib_proc_exec");
  if (pexec == NULL) {
    return 0;
  }

  // build parameter table
  slib_par_t *ptable = (slib_par_t *)malloc(sizeof(slib_par_t) * MAX_PARAM);
  int pcount = slib_build_ptable(ptable);
  if (prog_error) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
    return 0;
  }

  // exec
  var_t ret;
  v_init(&ret);
  int success = pexec(index - lib->first_proc, pcount, ptable, &ret);

  // error
  if (!success) {
    if (ret.type == V_STR) {
      err_throw("LIB:%s: %s\n", lib->name, ret.v.p.ptr);
    } else {
      err_throw("LIB:%s: Unspecified error\n", lib->name);
    }
  }
  // clean-up
  if (ptable) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
  }

  v_free(&ret);
  return success;
}

/**
 * execute a function
 */
int slib_funcexec(int lib_id, int index, var_t *ret) {
  slib_par_t *ptable = NULL;
  int (*pexec) (int, int, slib_par_t *, var_t *);

  if (lib_id < 0 || lib_id >= slib_count) {
    return 0;
  }

  slib_t *lib = &slib_table[lib_id];
  pexec = slib_getoptptr(lib, "sblib_func_exec");
  if (pexec == NULL) {
    return 0;
  }

  // build parameter table
  ptable = malloc(sizeof(slib_par_t) * MAX_PARAM);
  int pcount = slib_build_ptable(ptable);
  if (prog_error) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
    return 0;
  }

  // exec
  v_init(ret);
  int success = pexec(index - lib->first_func, pcount, ptable, ret);

  // error
  if (!success) {
    if (ret->type == V_STR) {
      err_throw("LIB:%s: %s\n", lib->name, ret->v.p.ptr);
    } else {
      err_throw("LIB:%s: Unspecified error\n", lib->name);
    }
  }
  // clean-up
  if (ptable) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
  }

  return success;
}

/**
 * plugin event handling
 */
int slib_events(int wait_flag) {
  int result = 0;
  for (int i = 0; i < slib_count; i++) {
    slib_t *lib = &slib_table[i];
    if (lib->handle && lib->dev_events) {
      int events = lib->dev_events(wait_flag);
      if (events == -2) {
        // BREAK
        result = events;
        break;
      } else if (events != 0) {
        // events exist
        result = events;
      }
    }
  }
  return result;
}

#else
// dummy implementations
int slib_funcexec(int lib_id, int index, var_t *ret) { return 0; }
int slib_getfuncname(int lib_id, int index, char *buf) { return 0; }
int slib_getprocname(int lib_id, int index, char *buf) { return 0; }
int slib_procexec(int lib_id, int index) { return 0; }
int slib_get_kid(const char *name) { return -1; }
int slib_get_module_id(const char *name, const char *alias) { return -1; }
void slib_close() {}
int slib_events(int wait_flag) { return 0; }
void slib_init(int mcount, const char *mlist) {}
void slib_setup_comp(int mid) {}
#endif
