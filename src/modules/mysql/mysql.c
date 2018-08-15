// This file is part of SmallBASIC
//
// SmallBASIC - MySQL client module based on mysql command-line client
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2003 Nicholas Christopoulos

#include <string.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include "mod_utils.h"

typedef struct {
  MYSQL *dbf;
} dbnode_t;

#define MAX_OPEN_DB 256
#define MESSAGE_LEN 128

static dbnode_t table[MAX_OPEN_DB];
static int hcount;

/*
 * find and return a free handle
 */
static int get_free_handle() {
  int i;

  if (hcount == 0) {
    // we avoid to use handle 0, because we want to use 0 for FALSE
    hcount = 2;
    return 1;
  }

  for (i = 1; i < hcount; i++) {
    if (table[i].dbf == NULL) {
      return i;
    }
  }

  if (hcount + 1 >= MAX_OPEN_DB) {
    return -1;
  }
  hcount++;
  return hcount - 1;
}

/*
 * returns true if the 'handle' is a valid handle
 */
static int is_valid_handle(int handle) {
  if (handle < 1 || handle >= hcount) {
    return 0;
  }
  if (table[handle].dbf == NULL) {
    return 0;
  }
  return 1;
}

/*
 * handle <- CONNECT(host, database, user[, password])
 */
int mq_connect(slib_par_t * params, int pcount, var_t * retval) {
  char *host, *dbname, *user, *paswd;
  int success = 1;

  success = mod_parstr_ptr(0, params, pcount, &host);
  success = mod_parstr_ptr(1, params, pcount, &dbname);
  success = mod_parstr_ptr(2, params, pcount, &user);
  success = mod_opt_parstr_ptr(3, params, pcount, &paswd, NULL);

  if (success) {
    int handle;

    handle = get_free_handle();
    if (handle >= 0) {
      table[handle].dbf = malloc(sizeof(MYSQL));
      mysql_init(table[handle].dbf);
      mysql_options(table[handle].dbf, MYSQL_READ_DEFAULT_GROUP, "SmallBASIC");
      if (!mysql_real_connect
          (table[handle].dbf, host, user, paswd, dbname, 0, NULL, 0)) {
        success = 0;
        char buffer[MESSAGE_LEN];
        sprintf(buffer, "MySQL/CONNECT: %s", mysql_error(table[handle].dbf));
        v_setstr(retval, buffer);
        free(table[handle].dbf);
      }
      else
        v_setint(retval, handle);
    }
    else {
      success = 0;
      v_setstr(retval, "MySQL: NO FREE HANDLES");
    }
  }
  else {
    v_setstr(retval, "MySQL: ARGUMENT ERROR");
  }
  return success;
}

/*
 * DISCONNECT handle
 */
int mq_disconnect(slib_par_t * params, int pcount, var_t * retval) {
  int success, handle;

  success = mod_parint(0, params, pcount, &handle);

  if (success) {
    if (is_valid_handle(handle)) {
      mysql_close(table[handle].dbf);
      free(table[handle].dbf);
      table[handle].dbf = NULL;
      success = 1;
    }
    else {
      success = 0;
      v_setstr(retval, "MYSQL: INVALID HANDLE");
    }
  }
  else {
    v_setstr(retval, "MYSQL: ARGUMENT ERROR");
  }
  return success;
}

/*
 * USE handle, database
 */
int mq_use(slib_par_t * params, int pcount, var_t * retval) {
  int success, handle;
  char *dbname;

  success = mod_parint(0, params, pcount, &handle);
  success = mod_parstr_ptr(1, params, pcount, &dbname);

  if (success) {
    if (is_valid_handle(handle)) {
      if (mysql_select_db(table[handle].dbf, dbname)) {
        char buffer[MESSAGE_LEN];
        sprintf(buffer, "MySQL/USE: %s\n", mysql_error(table[handle].dbf));
        v_setstr(retval, buffer);
        success = 0;
      }
    }
    else {
      success = 0;
      v_setstr(retval, "MySQL/USE: INVALID HANDLE");
    }
  }
  else {
    v_setstr(retval, "MySQL/USE: ARGUMENT ERROR");
  }
  return success;
}

/*
 * build the return-variable as array filled with the query-result's rows x fields
 */
void mq_build_result_array(MYSQL * mysql, MYSQL_RES * result, var_t * retval) {
  MYSQL_ROW row;
  int num_fields, i;
  int num_rows, crow = 0;

  num_fields = mysql_num_fields(result);
  num_rows = mysql_num_rows(result);
  v_tomatrix(retval, num_rows, num_fields);

  // retrieve rows, then call mysql_free_result(result)
  while ((row = mysql_fetch_row(result))) {
    unsigned long *lengths;

    lengths = mysql_fetch_lengths(result);
    for (i = 0; i < num_fields; i++) {
      var_t *e;

      e = (var_t *) (retval->v.a.data + (sizeof(var_t) * (crow * num_fields + i)));
      if (row[i]) {
        char *buf;

        buf = (char *)malloc(lengths[i] + 1);
        strncpy(buf, row[i], lengths[i]);
        buf[lengths[i]] = '\0';
        v_setstr(e, buf);
        free(buf);
      }
      else {
        v_setstr(e, "");
      }
    }

    crow++;
  }
}

/*
 * array <- QUERY(handle, query)
 */
int mq_query(slib_par_t * params, int pcount, var_t * retval) {
  int success = 1, handle;
  char *query;
  MYSQL_RES *result;
  MYSQL *mysql;

  success = mod_parint(0, params, pcount, &handle);
  success = mod_parstr_ptr(1, params, pcount, &query);

  if (success) {
    if (is_valid_handle(handle)) {
      int err;

      mysql = table[handle].dbf;
      err = mysql_query(mysql, query);
      if (err) {
        success = 0;
        v_setstr(retval, mysql_error(mysql));
      }
      else {
        result = mysql_store_result(mysql);
        if (result) {           // there are rows
          mq_build_result_array(mysql, result, retval);
        }
        else {                  // mysql_store_result() returned nothing;
                                // should it have?
          if (mysql_field_count(mysql) == 0) {
            // query does not return data (it was not a SELECT)
            int num_rows;
            num_rows = mysql_affected_rows(mysql);
            v_setint(retval, num_rows);
          }
          else {                // mysql_store_result() should have returned
                                // data
            char buffer[MESSAGE_LEN];
            sprintf(buffer, "MySQL/QUERY(): %s\n", mysql_error(mysql));
            v_setstr(retval, buffer);
          }
        }
      }
    }
    else {
      success = 0;
      v_setstr(retval, "MySQL: INVALID HANDLE");
    }
  }
  else {
    v_setstr(retval, "MySQL: ARGUMENT ERROR");
  }
  return success;
}

/*
 * array <- TABLES(handle[, wildcards])
 */
int mq_listtbls(slib_par_t * params, int pcount, var_t * retval) {
  int success = 1, handle;
  char *wc;
  MYSQL_RES *result;
  MYSQL *mysql;

  success = mod_parint(0, params, pcount, &handle);
  success = mod_opt_parstr_ptr(1, params, pcount, &wc, NULL);

  if (success) {
    if (is_valid_handle(handle)) {
      mysql = table[handle].dbf;
      result = mysql_list_tables(mysql, wc);
      if (result) {             // there are rows
        mq_build_result_array(mysql, result, retval);
        mysql_free_result(result);
      }
      else {
        char buffer[MESSAGE_LEN];
        sprintf(buffer, "MySQL/LISTTABLES: %s\n", mysql_error(mysql));
        v_setstr(retval, buffer);
      }
    }
    else {
      success = 0;
      v_setstr(retval, "MySQL/LISTTABLES: INVALID HANDLE");
    }
  }
  else {
    v_setstr(retval, "MySQL/LISTTABLES: ARGUMENT ERROR");
  }
  return success;
}

/*
 * array <- DBS(handle[, wildcards])
 */
int mq_listdbs(slib_par_t * params, int pcount, var_t * retval) {
  int success = 1, handle;
  char *wc;
  MYSQL_RES *result;
  MYSQL *mysql;

  success = mod_parint(0, params, pcount, &handle);
  success = mod_opt_parstr_ptr(1, params, pcount, &wc, NULL);

  if (success) {
    if (is_valid_handle(handle)) {
      mysql = table[handle].dbf;
      result = mysql_list_dbs(mysql, wc);
      if (result) {             // there are rows
        mq_build_result_array(mysql, result, retval);
        mysql_free_result(result);
      }
      else {
        char buffer[MESSAGE_LEN];
        sprintf(buffer, "MySQL/LISTDBS(): %s\n", mysql_error(mysql));
        v_setstr(retval, buffer);
      }
    }
    else {
      success = 0;
      v_setstr(retval, "MySQL/LISTDBS: INVALID HANDLE");
    }
  }
  else {
    v_setstr(retval, "MySQL/LISTDBS: ARGUMENT ERROR");
  }
  return success;
}

/*
 * array <- FIELDS(handle, table[, wildcards])
 */
int mq_listflds(slib_par_t * params, int pcount, var_t * retval) {
  int success = 1, handle;
  char *dbtable, *wc;
  MYSQL_RES *result;
  MYSQL *mysql;

  success = mod_parint(0, params, pcount, &handle);
  success = mod_parstr_ptr(1, params, pcount, &dbtable);
  success = mod_opt_parstr_ptr(2, params, pcount, &wc, NULL);

  if (success) {
    if (is_valid_handle(handle)) {
      mysql = table[handle].dbf;
      result = mysql_list_fields(mysql, dbtable, wc);
      if (result) {             // there are rows
        mq_build_result_array(mysql, result, retval);
        mysql_free_result(result);
      }
      else {
        char buffer[MESSAGE_LEN];
        sprintf(buffer, "MySQL/LISTFIELDS(): %s\n", mysql_error(mysql));
        v_setstr(retval, buffer);
      }
    }
    else {
      success = 0;
      v_setstr(retval, "MySQL/LISTFIELDS: INVALID HANDLE");
    }
  }
  else {
    v_setstr(retval, "MySQL/LISTFIELDS: ARGUMENT ERROR");
  }

  return success;
}

/* --- SmallBASIC interface ----------------------------------------------------------------------------------------------- */

typedef struct {
  char *name;
  int (*command) (slib_par_t *, int, var_t *);
} mod_kw;

// SB function names
static mod_kw func_names[] = {
  {"CONNECT", mq_connect},      // connects/reconnects to the server
  {"QUERY", mq_query},          // Send command to mysql server
  {"DBS", mq_listdbs},          // get a list of the databases
  {"TABLES", mq_listtbls},      // get a list of the tables
  {"FIELDS", mq_listflds},      // get a list of the fields of a table
  {NULL, NULL}
};

// SB procedure names
static mod_kw proc_names[] = {
  {"DISCONNECT", mq_disconnect},  // disconnects
  {"USE", mq_use},              // changes the current database
  {NULL, NULL}
};

/*
 * returns the number of the procedures
 */
int sblib_proc_count(void) {
  int i;

  for (i = 0; proc_names[i].name; i++);
  return i;
}

/*
 * returns the number of the functions
 */
int sblib_func_count(void) {
  int i;

  for (i = 0; func_names[i].name; i++);
  return i;
}

/*
 * returns the 'index' procedure name
 */
int sblib_proc_getname(int index, char *proc_name) {
  strcpy(proc_name, proc_names[index].name);
  return 1;
}

/*
 * returns the 'index' function name
 */
int sblib_func_getname(int index, char *proc_name) {
  strcpy(proc_name, func_names[index].name);
  return 1;
}

/*
 * execute the 'index' procedure
 */
int sblib_proc_exec(int index, int param_count, slib_par_t * params, var_t * retval) {
  return proc_names[index].command(params, param_count, retval);
}

/*
 * execute the 'index' function
 */
int sblib_func_exec(int index, int param_count, slib_par_t * params, var_t * retval) {
  return func_names[index].command(params, param_count, retval);
}
