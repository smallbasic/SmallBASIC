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

#include "common/smbas.h"

#if defined(__MINGW32__)
#include <windows.h>
#include <error.h>
#define WIN_EXTLIB
#define LIB_EXT ".dll"
#elif defined(_UnixOS)
#include <dlfcn.h>
#define LNX_EXTLIB
#define LIB_EXT ".so"
#endif

#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
#include "common/plugins.h"
#include "common/pproc.h"
#include <dirent.h>

#define MAX_SLIBS 64
#define MAX_PARAM 16
#define TABLE_GROW_SIZE 16
#define NAME_SIZE 256
#define PATH_SIZE 1024

typedef int (*sblib_exec_fn)(int, int, slib_par_t *, var_t *);
typedef int (*sblib_getname_fn) (int, char *);
typedef int (*sblib_count_fn) (void);
typedef int (*sblib_init_fn) (const char *);
typedef int (*sblib_free_fn) (int, int);
typedef void (*sblib_close_fn) (void);

typedef struct {
  char _fullname[PATH_SIZE];
  char _name[NAME_SIZE];
  void *_handle;
  sblib_exec_fn _sblib_proc_exec;
  sblib_exec_fn _sblib_func_exec;
  sblib_free_fn _sblib_free;
  ext_func_node_t *_func_list;
  ext_proc_node_t *_proc_list;
  uint32_t _id;
  uint32_t _flags;
  uint32_t _proc_count;
  uint32_t _func_count;
  uint32_t _proc_list_size;
  uint32_t _func_list_size;
  uint8_t  _imported;
} slib_t;

static slib_t *plugins[MAX_SLIBS];

#if defined(LNX_EXTLIB)
int slib_llopen(slib_t *lib) {
  lib->_handle = dlopen(lib->_fullname, RTLD_NOW);
  if (lib->_handle == NULL) {
    sc_raise("LIB: error on loading %s\n%s", lib->_name, dlerror());
  }
  return (lib->_handle != NULL);
}

void *slib_getoptptr(slib_t *lib, const char *name) {
  return dlsym(lib->_handle, name);
}

static int slib_llclose(slib_t *lib) {
  if (!lib->_handle) {
    return 0;
  }
  dlclose(lib->_handle);
  lib->_handle = NULL;
  return 1;
}

#elif defined(WIN_EXTLIB)
static int slib_llopen(slib_t *lib) {
  lib->_handle = LoadLibraryA(lib->_fullname);
  if (lib->_handle == NULL) {
    int error = GetLastError();
    switch (error) {
    case ERROR_MOD_NOT_FOUND:
      sc_raise("LIB: DLL dependency error [%d] loading %s [%s]\n", error, lib->_fullname, lib->_name);
      break;
    case ERROR_DYNLINK_FROM_INVALID_RING:
      sc_raise("LIB: DLL build error [%d] loading %s [%s]\n", error, lib->_fullname, lib->_name);
      break;
    default:
      sc_raise("LIB: error [%d] loading %s [%s]\n", error, lib->_fullname, lib->_name);
      break;
    }
  }
  return (lib->_handle != NULL);
}

static void *slib_getoptptr(slib_t *lib, const char *name) {
  return GetProcAddress((HMODULE) lib->_handle, name);
}

static int slib_llclose(slib_t *lib) {
  if (!lib->_handle) {
    return 0;
  }
  FreeLibrary(lib->_handle);
  lib->_handle = NULL;
  return 1;
}
#endif

//
// returns slib_t* for the given id
//
static slib_t *get_lib(int lib_id) {
  if (lib_id < 0 || lib_id >= MAX_SLIBS) {
    return NULL;
  }
  return plugins[lib_id];
}

//
// opens the library
//
static int slib_open(const char *path, const char *name, const char *alias, int id) {
  char fullname[PATH_SIZE];

  strlcpy(fullname, path, sizeof(fullname));
  if (access(fullname, R_OK) != 0) {
    if (path[strlen(path) - 1] != '/') {
      // add trailing separator
      strlcat(fullname, "/", sizeof(fullname));
    }
    strlcat(fullname, name, sizeof(fullname));
  }

  slib_t *lib = plugins[id];
  memset(lib, 0, sizeof(slib_t));
  strlcpy(lib->_name, alias, NAME_SIZE);
  strlcpy(lib->_fullname, fullname, PATH_SIZE);
  lib->_id = id;
  lib->_imported = 0;

  if (!opt_quiet) {
    log_printf("LIB: registering '%s'", fullname);
  }

  return slib_llopen(lib);
}

//
// init path and file from the given name
//
static void slib_init_path(const char *name, char *path, char *file) {
  if (name[0] == '"') {
    // use quoted string to specify full system path
    strlcpy(path, name + 1, PATH_SIZE);
    int len = strlen(path);
    if (len) {
      path[len - 1] = '\0';
    }
    // separate file-name from path
    char *slash = strrchr(path, '/');
    if (slash) {
      strlcpy(file, slash + 1, PATH_SIZE);
      *slash = '\0';
    }
  } else {
    // non-quoted string to specify portable name
    path[0] = '\0';
    char *slash = strrchr(name, '/');
    strlcpy(file, "lib", PATH_SIZE);
    if (strcmp(name, "android") == 0) {
      strlcat(file, "smallbasic", PATH_SIZE);
    } else if (slash) {
      *slash = '\0';
      strlcat(file, slash + 1, PATH_SIZE);
      strlcpy(path, name, PATH_SIZE);
    } else {
      strlcat(file, name, PATH_SIZE);
    }
    strlcat(file, LIB_EXT, PATH_SIZE);
  }
}

//
// locate the file int the given path or standard locations
//
static int slib_find_path(char *path, const char *file) {
  int result = 0;
  // find in path
  if (path[0]) {
    result = sys_search_path(path, file, path);
  }
  // find in SBASICPATH
  if (!result && getenv("SBASICPATH")) {
    result = sys_search_path(getenv("SBASICPATH"), file, path);
    if (!result && path[0]) {
      char rel_path[PATH_SIZE];
      strlcpy(rel_path, getenv("SBASICPATH"), PATH_SIZE);
      strlcat(rel_path, "/", PATH_SIZE);
      strlcat(rel_path, path, PATH_SIZE);
      result = sys_search_path(rel_path, file, path);
    }
  }
  // find in AppImage
  if (!result && getenv("APPDIR")) {
    char rel_path[PATH_SIZE];
    strlcpy(rel_path, getenv("APPDIR"), PATH_SIZE);
    strlcat(rel_path, "/usr/lib", PATH_SIZE);
    result = sys_search_path(rel_path, file, path);
  }
  // find in modpath
  if (!result && opt_modpath[0]) {
    result = sys_search_path(opt_modpath, file, path);
    if (!result && path[0]) {
      char rel_path[PATH_SIZE];
      strlcpy(rel_path, opt_modpath, PATH_SIZE);
      strlcat(rel_path, "/", PATH_SIZE);
      strlcat(rel_path, path, PATH_SIZE);
      result = sys_search_path(rel_path, file, path);
    }
  }
  // find in program launch directory
  if (!result && gsb_bas_dir[0]) {
    result = sys_search_path(gsb_bas_dir, file, path);
  }
  if (!result) {
    // find in current directory
    result = sys_search_path(".", file, path);
  }

  return result;
}

//
// add an external procedure to the list
//
static int slib_add_external_proc(const char *proc_name, int lib_id) {
  slib_t *lib = get_lib(lib_id);

  if (lib->_proc_list == NULL) {
    lib->_proc_list_size = TABLE_GROW_SIZE;
    lib->_proc_list = (ext_proc_node_t *)malloc(sizeof(ext_proc_node_t) * lib->_proc_list_size);
  } else if (lib->_proc_list_size <= (lib->_proc_count + 1)) {
    lib->_proc_list_size += TABLE_GROW_SIZE;
    lib->_proc_list = (ext_proc_node_t *)realloc(lib->_proc_list, sizeof(ext_proc_node_t) * lib->_proc_list_size);
  }

  lib->_proc_list[lib->_proc_count].lib_id = lib_id;
  lib->_proc_list[lib->_proc_count].symbol_index = 0;
  strlcpy(lib->_proc_list[lib->_proc_count].name, proc_name, sizeof(lib->_proc_list[lib->_proc_count].name));
  strupper(lib->_proc_list[lib->_proc_count].name);

  if (opt_verbose) {
    log_printf("LIB: %d, Idx: %d, PROC '%s'\n", lib_id, lib->_proc_count,
               lib->_proc_list[lib->_proc_count].name);
  }
  lib->_proc_count++;
  return lib->_proc_count - 1;
}

//
// Add an external function to the list
//
static int slib_add_external_func(const char *func_name, uint32_t lib_id) {
  slib_t *lib = get_lib(lib_id);

  if (lib->_func_list == NULL) {
    lib->_func_list_size = TABLE_GROW_SIZE;
    lib->_func_list = (ext_func_node_t *)malloc(sizeof(ext_func_node_t) * lib->_func_list_size);
  } else if (lib->_func_list_size <= (lib->_func_count + 1)) {
    lib->_func_list_size += TABLE_GROW_SIZE;
    lib->_func_list = (ext_func_node_t *)
                      realloc(lib->_func_list, sizeof(ext_func_node_t) * lib->_func_list_size);
  }

  lib->_func_list[lib->_func_count].lib_id = lib_id;
  lib->_func_list[lib->_func_count].symbol_index = 0;
  strlcpy(lib->_func_list[lib->_func_count].name, func_name, sizeof(lib->_func_list[lib->_func_count].name));
  strupper(lib->_func_list[lib->_func_count].name);

  if (opt_verbose) {
    log_printf("LIB: %d, Idx: %d, FUNC '%s'\n", lib_id, lib->_func_count,
               lib->_func_list[lib->_func_count].name);
  }
  lib->_func_count++;
  return lib->_func_count - 1;
}

//
// import functions from the external library
//
static void slib_import_routines(slib_t *lib, int comp) {
  int total = 0;
  char buf[SB_KEYWORD_SIZE];

  lib->_sblib_func_exec = slib_getoptptr(lib, "sblib_func_exec");
  lib->_sblib_proc_exec = slib_getoptptr(lib, "sblib_proc_exec");
  lib->_sblib_free = slib_getoptptr(lib, "sblib_free");
  sblib_count_fn fcount = slib_getoptptr(lib, "sblib_proc_count");
  sblib_getname_fn fgetname = slib_getoptptr(lib, "sblib_proc_getname");

  if (fcount && fgetname) {
    int count = fcount();
    total += count;
    for (int i = 0; i < count; i++) {
      if (fgetname(i, buf)) {
        strupper(buf);
        if (!lib->_imported && slib_add_external_proc(buf, lib->_id) == -1) {
          break;
        } else if (comp) {
          char name[NAME_SIZE];
          strlcpy(name, lib->_name, sizeof(name));
          strlcat(name, ".", sizeof(name));
          strlcat(name, buf, sizeof(name));
          strupper(name);
          comp_add_external_proc(name, lib->_id);
        }
      }
    }
  }

  fcount = slib_getoptptr(lib, "sblib_func_count");
  fgetname = slib_getoptptr(lib, "sblib_func_getname");

  if (fcount && fgetname) {
    int count = fcount();
    total += count;
    for (int i = 0; i < count; i++) {
      if (fgetname(i, buf)) {
        strupper(buf);
        if (!lib->_imported && slib_add_external_func(buf, lib->_id) == -1) {
          break;
        } else if (comp) {
          char name[NAME_SIZE];
          strlcpy(name, lib->_name, sizeof(name));
          strlcat(name, ".", sizeof(name));
          strlcat(name, buf, sizeof(name));
          strupper(name);
          comp_add_external_func(name, lib->_id);
        }
      }
    }
  }

  if (!total) {
    log_printf("LIB: module '%s' has no exports\n", lib->_name);
  }
}

//
// build parameter table
//
static int slib_build_ptable(slib_par_t *ptable) {
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

//
// free parameter table
//
static void slib_free_ptable(slib_par_t *ptable, int pcount) {
  for (int i = 0; i < pcount; i++) {
    if (ptable[i].byref == 0) {
      v_free(ptable[i].var_p);
      v_detach(ptable[i].var_p);
    }
  }
}

//
// execute a function or procedure
//
static int slib_exec(slib_t *lib, var_t *ret, int index, int proc) {
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
    success = lib->_sblib_proc_exec(index, pcount, ptable, ret);
  } else {
    success = lib->_sblib_func_exec(index, pcount, ptable, ret);
  }

  // error
  if (!success) {
    if (ret->type == V_STR) {
      err_throw("LIB:%s: %s\n", lib->_name, ret->v.p.ptr);
    } else {
      err_throw("LIB:%s: Unspecified error calling %s\n", lib->_name, (proc ? "SUB" : "FUNC"));
    }
  }

  // clean-up
  if (ptable) {
    slib_free_ptable(ptable, pcount);
    free(ptable);
  }

  if (success && v_is_type(ret, V_MAP)) {
    ret->v.m.lib_id = lib->_id;
    ret->v.m.ref = 1;
  }

  return success;
}

void plugin_init() {
  for (int i = 0; i < MAX_SLIBS; i++) {
    plugins[i] = NULL;
  }
}

int plugin_import(const char *name, const char *alias) {
  int result = -1;
  char path[PATH_SIZE];
  char file[PATH_SIZE];

  slib_init_path(name, path, file);

  if (slib_find_path(path, file)) {
    for (int i = 0; i < MAX_SLIBS; i++) {
      if (!plugins[i]) {
        // found free slot
        slib_t *lib = plugins[i] = (slib_t *)calloc(sizeof(slib_t), 1);
        if (!lib) {
          sc_raise("LIB: plugin_import failed");
          break;
        }
        if (slib_open(path, file, alias, i)) {
          slib_import_routines(lib, 1);
          lib->_imported = 1;
        } else {
          sc_raise("LIB: plugin_import failed");
        }
        result = i;
        break;
      }
    }
  }
  return result;
}

void plugin_open(const char *name, int lib_id) {
  slib_t *lib = get_lib(lib_id);
  if (!lib && lib_id >= 0 && lib_id < MAX_SLIBS) {
    lib = plugins[lib_id] = (slib_t *)calloc(sizeof(slib_t), 1);
    if (lib) {
      char path[PATH_SIZE];
      char file[PATH_SIZE];
      slib_init_path(name, path, file);
      if (!slib_find_path(path, file)) {
        rt_raise("LIB: can't open %s", name);
      } else if (!slib_open(path, file, file, lib_id)) {
        rt_raise("LIB: can't open %s", name);
      }
    }
  }
  if (lib && !prog_error) {
    if (!lib->_imported) {
      slib_import_routines(lib, 0);
      lib->_imported = 1;
    }
    sblib_init_fn minit = slib_getoptptr(lib, "sblib_init");
    if (minit && !minit(gsb_last_file)) {
      rt_raise("LIB: %s->sblib_init(), failed", lib->_name);
    }
  } else {
    rt_raise("LIB: plugin_open failed");
  }
}

int plugin_get_kid(int lib_id, const char *name) {
  slib_t *lib = get_lib(lib_id);
  if (lib != NULL) {
    const char *dot = strchr(name, '.');
    const char *field = (dot != NULL ? dot + 1 : name);
    for (int i = 0; i < lib->_proc_count; i++) {
      if (lib->_proc_list[i].lib_id == lib_id &&
          strcmp(lib->_proc_list[i].name, field) == 0) {
        return i;
      }
    }
    for (int i = 0; i < lib->_func_count; i++) {
      if (lib->_func_list[i].lib_id == lib_id &&
          strcmp(lib->_func_list[i].name, field) == 0) {
        return i;
      }
    }
  }
  return -1;
}

void *plugin_get_func(const char *name) {
  void *result = NULL;
  for (int i = 0; i < MAX_SLIBS && result == NULL; i++) {
    if (plugins[i]) {
      slib_t *lib = plugins[i];
      if (lib->_imported) {
        result = slib_getoptptr(lib, name);
      }
    }
  }
  return result;
}

int plugin_procexec(int lib_id, int index) {
  int result;
  slib_t *lib = get_lib(lib_id);
  if (lib && lib->_sblib_proc_exec) {
    var_t ret;
    v_init(&ret);
    result = slib_exec(lib, &ret, index, 1);
    v_free(&ret);
  } else {
    result = 0;
  }
  return result;
}

int plugin_funcexec(int lib_id, int index, var_t *ret) {
  int result;
  slib_t *lib = get_lib(lib_id);
  if (lib && lib->_sblib_func_exec) {
    result = slib_exec(lib, ret, index, 0);
  } else {
    result = 0;
  }
  return result;
}

void plugin_free(int lib_id, int cls_id, int id) {
  slib_t *lib = get_lib(lib_id);
  if (lib && lib->_sblib_free) {
    lib->_sblib_free(cls_id, id);
  }
}

void plugin_close() {
  for (int i = 0; i < MAX_SLIBS; i++) {
    if (plugins[i]) {
      slib_t *lib = plugins[i];
      if (lib->_handle) {
        sblib_close_fn mclose = slib_getoptptr(lib, "sblib_close");
        if (mclose) {
          mclose();
        }
        slib_llclose(lib);
      }
      free(lib->_proc_list);
      free(lib->_func_list);
      free(lib);
    }
    plugins[i] = NULL;
  }
}

#else
// dummy implementations
void plugin_init() {}
int plugin_import(const char *name, const char *alias) { return -1; }
void plugin_open(const char *name, int lib_id) { }
int plugin_get_kid(int lib_id, const char *keyword) { return -1; }
void *plugin_get_func(const char *name) { return 0; }
int plugin_procexec(int lib_id, int index) { return -1; }
int plugin_funcexec(int lib_id, int index, var_t *ret) { return -1; }
void plugin_free(int lib_id, int cls_id, int id) {}
void plugin_close() {}
#endif
