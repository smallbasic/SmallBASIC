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
#define MAX_PARAM 16
#define TABLE_GROW_SIZE 16
#define NAME_SIZE 256
#define PATH_SIZE 1024

typedef int (*sblib_exec_fn)(int, int, slib_par_t *, var_t *);

typedef struct {
  char name[NAME_SIZE];
  char fullname[PATH_SIZE];
  void *handle;
  sblib_exec_fn sblib_proc_exec;
  sblib_exec_fn sblib_func_exec;
  uint32_t id;
  uint32_t flags;
  uint32_t first_proc;
  uint32_t first_func;
  uint8_t  imported;
} slib_t;

static slib_t slib_table[MAX_SLIBS];
static int slib_count;
static int extprocsize;
static int extproccount;
static int extfuncsize;
static int extfunccount;
static ext_func_node_t *extfunctable;
static ext_proc_node_t *extproctable;

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
  // scan for conflicts
  for (int i = 0; i < extproccount; i++) {
    if (strcmp(extproctable[i].name, proc_name) == 0) {
      sc_raise("LIB: duplicate proc %s", proc_name);
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
  strlcpy(extproctable[extproccount].name, proc_name, sizeof(extproctable[extproccount].name));
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
  // scan for conflicts
  for (int i = 0; i < extfunccount; i++) {
    if (strcmp(extfunctable[i].name, func_name) == 0) {
      sc_raise("LIB: duplicate func %s", func_name);
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
  strlcpy(extfunctable[extfunccount].name, func_name, sizeof(extfunctable[extfunccount].name));
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
int slib_get_kid(int lib_id, const char *name) {
  const char *dot = strchr(name, '.');
  const char *field = (dot != NULL ? dot + 1 : name);
  for (int i = 0; i < extproccount; i++) {
    if (extproctable[i].lib_id == lib_id &&
        strcmp(extproctable[i].name, field) == 0) {
      return i;
    }
  }
  for (int i = 0; i < extfunccount; i++) {
    if (extfunctable[i].lib_id == lib_id &&
        strcmp(extfunctable[i].name, field) == 0) {
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

void slib_import_routines(slib_t *lib, int comp) {
  char buf[SB_KEYWORD_SIZE];
  int (*fgetname) (int, char *);
  int (*fcount) (void);

  lib->first_proc = extproccount;
  lib->first_func = extfunccount;
  lib->sblib_func_exec = slib_getoptptr(lib, "sblib_func_exec");
  lib->sblib_proc_exec = slib_getoptptr(lib, "sblib_proc_exec");

  fcount = slib_getoptptr(lib, "sblib_proc_count");
  fgetname = slib_getoptptr(lib, "sblib_proc_getname");

  if (fcount && fgetname) {
    int count = fcount();
    for (int i = 0; i < count; i++) {
      if (fgetname(i, buf)) {
        strupper(buf);
        if (slib_add_external_proc(buf, lib->id) == -1) {
          break;
        } else if (comp) {
          char name[NAME_SIZE];
          strlcpy(name, lib->name, sizeof(name));
          strlcat(name, ".", sizeof(name));
          strlcat(name, buf, sizeof(name));
          strupper(name);
          comp_add_external_proc(name, lib->id);
        }
      }
    }
  }

  fcount = slib_getoptptr(lib, "sblib_func_count");
  fgetname = slib_getoptptr(lib, "sblib_func_getname");

  if (fcount && fgetname) {
    int count = fcount();
    for (int i = 0; i < count; i++) {
      if (fgetname(i, buf)) {
        strupper(buf);
        if (slib_add_external_func(buf, lib->id) == -1) {
          break;
        } else if (comp) {
          char name[NAME_SIZE];
          strlcpy(name, lib->name, sizeof(name));
          strlcat(name, ".", sizeof(name));
          strlcat(name, buf, sizeof(name));
          strupper(name);
          comp_add_external_func(name, lib->id);
        }
      }
    }
  }
}

/**
 * returns slib_t* for the given id
 */
slib_t *get_lib(int lib_id) {
  if (lib_id < 0 || lib_id >= slib_count) {
    return NULL;
  }
  return &slib_table[lib_id];
}

/**
 * updates compiler with the module's keywords
 */
void slib_import(int lib_id, int comp) {
  slib_t *lib = get_lib(lib_id);
  if (lib && !lib->imported) {
    slib_import_routines(lib, comp);
    lib->imported = 1;
  }
}

/**
 * opens the library and invokes the init function
 */
void slib_open(const char *fullname, const char *name) {
  int success = 0;
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
  lib->imported = 0;

  if (!opt_quiet) {
    log_printf("LIB: importing %s", fullname);
  }
  if (slib_llopen(lib)) {
    success = 1;

    // init
    int (*minit) (void);
    minit = slib_getoptptr(lib, "sblib_init");
    if (minit) {
      if (!minit()) {
        sc_raise("LIB: %s->sblib_init(), failed", lib->name);
        success = 0;
      }
    }

    // override default name
    const char *(*get_module_name) (void);
    get_module_name = slib_getoptptr(lib, "sblib_get_module_name");
    if (get_module_name) {
      strlcpy(lib->name, get_module_name(), NAME_SIZE);
    }
  } else {
    sc_raise("LIB: can't open %s", fullname);
  }
  if (success) {
    slib_count++;
    if (!opt_quiet) {
      log_printf("... done\n");
    }
  }
}

void slib_open_path(const char *path, const char *name) {
  char *p;
  if (((p = strstr(name, LIB_EXT)) != NULL) && strcmp(p, LIB_EXT) == 0) {
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
    slib_open(full, libname);
  }
}

void slib_scan_path(const char *path) {
  struct stat stbuf;
  if (stat(path, &stbuf) != -1) {
    if (S_ISREG(stbuf.st_mode)) {
      char *name = strrchr(path, '/');
      slib_open_path(path, (name ? name + 1 : path));
    } else {
      DIR *dp = opendir(path);
      if (dp != NULL) {
        struct dirent *e;
        while ((e = readdir(dp)) != NULL) {
          char *name = e->d_name;
          if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
            slib_open_path(path, name);
          }
        }
        closedir(dp);
      }
    }
  } else if (!opt_quiet) {
    log_printf("LIB: module path %s not found.\n", path);
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
  for (int i = 0; i < slib_count; i++) {
    slib_t *lib = &slib_table[i];
    if (lib->handle) {
      void (*mclose) (void);
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
      if (pcount == MAX_PARAM) {
        err_parm_limit(MAX_PARAM);
      }
    } while (!ready && !prog_error);
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
 * execute a function or procedure
 */
int slib_exec(slib_t *lib, var_t *ret, int index, int proc) {
  slib_par_t *ptable;
  int pcount;
  if (code_peek() == kwTYPE_LEVEL_BEGIN) {
    ptable = (slib_par_t *)malloc(sizeof(slib_par_t) * MAX_PARAM);
    pcount = slib_build_ptable(ptable);
  } else {
    ptable = NULL;
    pcount = 0;
  }
  if (prog_error) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
    return 0;
  }

  int success;
  v_init(ret);
  if (proc) {
    success = lib->sblib_proc_exec(index - lib->first_proc, pcount, ptable, ret);
  } else {
    success = lib->sblib_func_exec(index - lib->first_func, pcount, ptable, ret);
  }

  // error
  if (!success) {
    if (ret->type == V_STR) {
      err_throw("LIB:%s: %s\n", lib->name, ret->v.p.ptr);
    } else {
      err_throw("LIB:%s: Unspecified error calling %s\n", lib->name, (proc ? "SUB" : "FUNC"));
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
 * execute a procedure
 */
int slib_procexec(int lib_id, int index) {
  int result;
  slib_t *lib = get_lib(lib_id);
  if (lib && lib->sblib_proc_exec) {
    var_t ret;
    result = slib_exec(lib, &ret, index, 1);
    v_free(&ret);
  } else {
    result = 0;
  }
  return result;
}

/**
 * execute a function
 */
int slib_funcexec(int lib_id, int index, var_t *ret) {
  int result;
  slib_t *lib = get_lib(lib_id);
  if (lib && lib->sblib_func_exec) {
    result = slib_exec(lib, ret, index, 0);
  } else {
    result = 0;
  }
  return result;
}

void *slib_get_func(const char *name) {
  void *result = NULL;
  for (int i = 0; i < slib_count && result == NULL; i++) {
    slib_t *lib = &slib_table[i];
    if (lib->imported) {
      result = slib_getoptptr(lib, name);
    }
  }
  return result;
}

#else
// dummy implementations
int slib_funcexec(int lib_id, int index, var_t *ret) { return 0; }
int slib_procexec(int lib_id, int index) { return 0; }
int slib_get_kid(int lib_id, const char *name) { return -1; }
int slib_get_module_id(const char *name, const char *alias) { return -1; }
void slib_close() {}
int slib_events(int wait_flag) { return 0; }
void slib_init(int mcount, const char *mlist) {}
void slib_setup_comp(int lib_id) {}
void *slib_get_func(const char *name) {}
#endif
