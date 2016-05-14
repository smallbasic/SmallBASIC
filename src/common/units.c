// This file is part of SmallBASIC
//
// SmallBASIC Unit (SB units) manager
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/kw.h"
#include "common/var.h"
#include "common/device.h"
#include "common/pproc.h"
#include "common/scan.h"
#include "common/units.h"

// units table
static unit_t *units;
static int unit_count = 0;

/**
 *   initialization
 */
void unit_mgr_init() {
  unit_count = 0;
  units = NULL;
}

/**
 *   close up
 */
void unit_mgr_close() {
  int i;

  for (i = 0; i < unit_count; i++) {
    if (units[i].status == unit_loaded) {
      close_unit(i);
    }
  }
  unit_count = 0;
  if (units) {
    free(units);
    units = NULL;
  }
}

/**
 * returns the full-pathname of unit
 *
 * @param name unit's name
 * @param file buffer to store the filename
 * @return non-zero on success
 */
int find_unit_path(const char *name, char *file) {
  strcpy(file, name);
  strcat(file, ".bas");

  // find in unitpath
  if (getenv("UNITPATH")) {
    if (sys_search_path(getenv("UNITPATH"), file, file)) {
      return 1;
    }
  }

  // find in program launch directory
  if (gsb_bas_dir[0] && sys_search_path(gsb_bas_dir, file, file)) {
    return 1;
  }

  // find in current directory
  if (sys_search_path(".", file, file)) {
    return 1;
  }

  return 0;
}

/**
 * returns the .sbu file name with the same path as the
 * corresponding .bas file.
 *
 * @param name unit's name
 * @param file buffer to store the filename
 * @return non-zero on success
 */
int find_unit(const char *name, char *file) {
  if (find_unit_path(name, file)) {
    file[strlen(file) - 4] = 0;
    strcat(file, ".sbu");
    return 1;
  }
  return 0;
}

/**
 * open unit
 *
 * @param file is the filename
 * @return the unit handle or -1 on error
 */
int open_unit(const char *file) {
  int h;
  unit_t u;
  int uid = -1;

  char unitname[OS_PATHNAME_SIZE];
  char bas_file[OS_PATHNAME_SIZE];
  time_t ut, st;
  int comp_rq = 0;

  // clean structure please
  memset(&u, 0, sizeof(unit_t));

  // find unit's source file name
  if (!find_unit_path(file, bas_file)) {
    return -1;
  }

  // create corresponding sbu path version
  strcpy(unitname, bas_file);
  unitname[strlen(bas_file) - 4] = 0;
  strcat(unitname, ".sbu");

  if ((ut = sys_filetime(unitname)) == 0L) {
    // binary not found - compile
    comp_rq = 1;
  } else {
    if ((st = sys_filetime(bas_file))) {
      // source found
      if (ut < st) {
        // executable is older than source - compile
        comp_rq = 1;
      }
    }
  }

  // compilation required
  if (comp_rq && !comp_compile(bas_file)) {
    return -1;
  }

  // open unit
  h = open(unitname, O_RDWR | O_BINARY, 0660);
  if (h == -1) {
    return -1;
  }

  // read file header
  int nread = read(h, &u.hdr, sizeof(unit_file_t));
  if (nread != sizeof(unit_file_t) ||
      u.hdr.version != SB_DWORD_VER ||
      memcmp(&u.hdr.sign, "SBUn", 4) != 0) {
    close(h);
    return -1;
  }

  // load symbol-table
  if (u.hdr.sym_count) {
    u.symbols = (unit_sym_t *)malloc(u.hdr.sym_count * sizeof(unit_sym_t));
    read(h, u.symbols, u.hdr.sym_count * sizeof(unit_sym_t));
  }

  // setup the rest
  strcpy(u.name, unitname);
  u.status = unit_loaded;

  // add unit
  uid = unit_count;
  unit_count++;

  if (units == NULL) {
    units = (unit_t *)malloc(unit_count * sizeof(unit_t));
  } else {
    units = (unit_t *)realloc(units, unit_count * sizeof(unit_t));
  }

  // copy unit's data
  memcpy(&units[uid], &u, sizeof(unit_t));

  // cleanup
  close(h);
  return uid;
}

/**
 * closes a unit
 *
 * @param uid is the unit's handle
 * @return 0 on success
 */
int close_unit(int uid) {
  if (uid >= 0) {
    unit_t *u = &units[uid];
    if (u->status == unit_loaded) {
      u->status = unit_undefined;
      free(u->symbols);
    } else {
      return -2;
    }
  } else {
    return -1;
  }

  return 0;
}

/**
 * imports unit's names
 *
 * @param uid unit's handle
 * @return 0 on success
 */
int import_unit(int uid) {
  char buf[SB_KEYWORD_SIZE + 1];int i;

  if (uid >= 0) {
    unit_t *u = &units[uid];
    if (u->status == unit_loaded) {
      for (i = 0; i < u->hdr.sym_count; i++) {
        // build the name
        // with any path component removed from the name
        char *dir_sep = strrchr(u->hdr.base, OS_DIRSEP);
        sprintf(buf, "%s.%s",
            dir_sep ? dir_sep + 1 : u->hdr.base, u->symbols[i].symbol);

        switch (u->symbols[i].type) {
        case stt_function:
          comp_add_external_func(buf, uid | UID_UNIT_BIT);
          break;
        case stt_procedure:
          comp_add_external_proc(buf, uid | UID_UNIT_BIT);
          break;
        case stt_variable:
          comp_add_external_var(buf, uid | UID_UNIT_BIT);
          break;
        };
      }
    }
    else {
      return -2;
    }
  }
  else {
    return -1;
  }

  return 0;
}

/**
 * execute a call to a unit
 */
int unit_exec(int lib_id, int index, var_t *ret) {
  unit_sym_t *us;               // unit's symbol data
  bc_symbol_rec_t *ps;          // program's symbol data
  int my_tid;
  stknode_t udf_rv;

  my_tid = ctask->tid;
  ps = &prog_symtable[index];
  us = &(taskinfo(ps->task_id)->sbe.exec.exptable[ps->exp_idx]);

  switch (ps->type) {
  case stt_variable:
    break;
  case stt_procedure:
    exec_sync_variables(1);
    cmd_call_unit_udp(kwPROC, ps->task_id, us->address, INVALID_ADDR);
    activate_task(ps->task_id);
    if (prog_error) {
      gsb_last_error = prog_error;
      taskinfo(my_tid)->error = gsb_last_error;
      return 0;
    }
    bc_loop(2);
    if (prog_error) {
      gsb_last_error = prog_error;
      taskinfo(my_tid)->error = gsb_last_error;
      return 0;
    }
    activate_task(my_tid);
    exec_sync_variables(0);
    break;

  case stt_function:
    exec_sync_variables(1);
    cmd_call_unit_udp(kwFUNC, ps->task_id, us->address, us->vid);
    activate_task(ps->task_id);
    if (prog_error) {
      gsb_last_error = prog_error;
      taskinfo(my_tid)->error = gsb_last_error;
      return 0;
    }
    bc_loop(2);
    if (prog_error) {
      gsb_last_error = prog_error;
      taskinfo(my_tid)->error = gsb_last_error;
      return 0;
    }
    // get last variable from stack
    code_pop(&udf_rv, kwTYPE_RET);
    if (udf_rv.type != kwTYPE_RET) {
      err_stackmess();
    } else {
      v_set(ret, udf_rv.x.vdvar.vptr);
      v_free(udf_rv.x.vdvar.vptr);  // free ret-var
      free(udf_rv.x.vdvar.vptr);
    }

    activate_task(my_tid);
    exec_sync_variables(0);
    break;
  };

  return (prog_error == 0);
}
