/*
*	SmallBASIC - GDBM module
*
*	2003/03/06, Nicholas Christopoulos
*/

#include <extlib.h>
#include <gdbm.h>

typedef struct {
	GDBM_FILE	dbf;
	}	dbnode_t;

#define	MAX_OPEN_DB	256

static dbnode_t	table[MAX_OPEN_DB];
static int		hcount;

typedef char* mod_kw;
// SB function names
static mod_kw func_names[] =
{
"OPEN",
"STORE",
"FETCH",
"DELETE",
"FIRSTKEY",
"NEXTKEY",
"REORGANIZE",
"EXISTS",
"STRERROR",
"SETOPT",
"FDESC",
NULL
};

// SB procedure names
static mod_kw proc_names[] =
{
"CLOSE",
"SYNC",
NULL
};

/*
*	find and return a free handle
*/
static int	get_free_handle()
{
	int		i;

	if	( hcount == 0 )	{
		// we avoid to use handle 0, because we want to use 0 for FALSE
		hcount = 2;
		return 1;
		}
		
	for ( i = 1; i < hcount; i ++ )	{
		if	( table[i].dbf == NULL )
			return i;
		}

	if	( hcount + 1 >= MAX_OPEN_DB )	
		return -1;

	hcount ++;
	return hcount - 1;
}

/*
*	returns true if the 'handle' is a valid handle
*/
static int	is_valid_handle(int handle)
{
	if	( handle < 1 || handle >= hcount )
		return 0;
	if	( table[handle].dbf == NULL )
		return 0;
	return 1;
}

/*
*	execute the 'index' function
*/
int		sblib_func_exec(int index, int param_count, slib_par_t *params, var_t *retval)
{
	int		success = 0;

	switch ( index )	{
	case	0:	// handle <- GDBM_OPEN(file[, block_size, flags, mode])
		{
			char	*file;
			int		bsize, flags, mode;

			success = mod_parstr_ptr(0, params, param_count, &file);
			success = mod_opt_parint(1, params, param_count, &bsize, 0);
			success = mod_opt_parint(2, params, param_count, &flags, GDBM_WRCREAT);
			success = mod_opt_parint(3, params, param_count, &mode, 0666);
		
			if	( success )	{
				int		handle;

				handle = get_free_handle();
				if	( handle >= 0 )	{
					table[handle].dbf = gdbm_open(file, bsize, flags, mode, NULL);
					success = (table[handle].dbf != NULL);
					if	( success )	
						v_setint(retval, handle);
					else
						v_setstr(retval, gdbm_strerror(gdbm_errno));
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_OPEN: NO FREE HANDLES");
					}
				}
			else
				v_setstr(retval, "GDBM_OPEN: argument error");
		}
		break;

	case	1:	// handle <- GDBM_STORE(handle, key, data [, flags])
		{
			int		handle, flags;
			char	*key, *data;

			success = mod_parint    (0, params, param_count, &handle);
			success = mod_parstr_ptr(1, params, param_count, &key);
			success = mod_parstr_ptr(2, params, param_count, &data);
			success = mod_opt_parint(3, params, param_count, &flags, GDBM_REPLACE);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					datum	dt_key, dt_data;
					int		r;

					dt_key.dptr   = key;
					dt_key.dsize  = strlen(key) + 1;
					dt_data.dptr  = data;
					dt_data.dsize = strlen(data) + 1;
					
					r = gdbm_store(table[handle].dbf, dt_key, dt_data, flags);
					v_setint(retval, r);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_STORE: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_STORE: argument error");
		}
		break;

	case	2:	// data <- GDBM_FETCH(handle, key)
		{
			int		handle;
			char	*key;

			success = mod_parint    (0, params, param_count, &handle);
			success = mod_parstr_ptr(1, params, param_count, &key);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					datum	dt_key, dt_data;

					dt_key.dptr   = key;
					dt_key.dsize  = strlen(key) + 1;
					
					dt_data = gdbm_fetch(table[handle].dbf, dt_key);
					v_setstr(retval, (char *) dt_data.dptr);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_FETCH: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_FETCH: argument error");
		}
		break;

	case	3:	// status <- GDBM_DELETE(handle, key)
		{
			int		handle;
			char	*key;

			success = mod_parint    (0, params, param_count, &handle);
			success = mod_parstr_ptr(1, params, param_count, &key);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					datum	dt_key;
					int		r;

					dt_key.dptr   = key;
					dt_key.dsize  = strlen(key) + 1;
					
					r = gdbm_delete(table[handle].dbf, dt_key);
					v_setint(retval, r);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_DELETE: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_DELETE: argument error");
		}
		break;

	case	4:	// key <- GDBM_FIRSTKEY(handle)
		{
			int		handle;

			success = mod_parint    (0, params, param_count, &handle);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					datum	dt_key;

					dt_key = gdbm_firstkey(table[handle].dbf);
					v_setstr(retval, (char *) dt_key.dptr);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_FIRSTKEY: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_FIRSTKEY: argument error");
		}
		break;

	case	5:	// key <- GDBM_NEXTKEY(handle, key)
		{
			int		handle;
			char	*key;

			success = mod_parint    (0, params, param_count, &handle);
			success = mod_parstr_ptr(1, params, param_count, &key);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					datum	dt_key;

					dt_key.dptr   = key;
					dt_key.dsize  = strlen(key) + 1;

					dt_key = gdbm_nextkey(table[handle].dbf, dt_key);
					v_setstr(retval, (char *) dt_key.dptr);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_NEXTKEY: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_NEXTKEY: argument error");
		}
		break;

	case	6:	// status <- GDBM_REORGANIZE(handle)
		{
			int		handle;

			success = mod_parint    (0, params, param_count, &handle);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					int		r;

					r = gdbm_reorganize(table[handle].dbf);
					v_setint(retval, r);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_REORGANIZE: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_REORGANIZE: argument error");
		}
		break;

	case	7:	// status <- GDBM_EXISTS(handle, key)
		{
			int		handle;
			char	*key;

			success = mod_parint    (0, params, param_count, &handle);
			success = mod_parstr_ptr(1, params, param_count, &key);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					datum	dt_key;
					int		r;

					dt_key.dptr   = key;
					dt_key.dsize  = strlen(key) + 1;

					r = gdbm_exists(table[handle].dbf, dt_key);
					v_setint(retval, r);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_EXISTS: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_EXISTS: argument error");
		}
		break;

	case	8:	// str <- GDBM_STRERROR()
		v_setstr(retval, gdbm_strerror(gdbm_errno));
		break;

	case	9:	// status <- GDBM_SETOPT(handle, option, value, size)
		{
			int		handle, option, value, size;

			success = mod_parint    (0, params, param_count, &handle);
			success = mod_parint    (1, params, param_count, &option);
			success = mod_parint    (2, params, param_count, &value);
			success = mod_parint    (3, params, param_count, &size);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					int		r;

					r = gdbm_setopt(table[handle].dbf, option, &value, size);
					v_setint(retval, r);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_SETOPT: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_SETOPT: argument error");
		}
		break;

	case	10:	// status <- GDBM_FDESC(handle)
		{
			int		handle;

			success = mod_parint    (0, params, param_count, &handle);
			if	( success )	{
				if	( is_valid_handle(handle) )	{
					int		r;

					r = gdbm_fdesc(table[handle].dbf);
					v_setint(retval, r);
					success = 1;
					}
				else	{
					success = 0;
					v_setstr(retval, "GDBM_FDESC: INVALID HANDLE");
					}
				}
			else
				v_setstr(retval, "GDBM_FDESC: argument error");
		}
		break;

	default:
		v_setstr(retval, "GDBM: function does not exist!");
		}

	return success;
}

/*
*	execute the 'index' procedure
*/
int		sblib_proc_exec(int index, int param_count, slib_par_t *params, var_t *retval)
{
	int		success = 0;
	int		handle;

	switch ( index )	{
	case	0:	// GDBM_CLOSE handle
		success = mod_parint(0, params, param_count, &handle);
		if	( success )	{
			if	( is_valid_handle(handle) )	{
				gdbm_close(table[handle].dbf);
				table[handle].dbf = NULL;
				success = 1;
				}
			else	{
				success = 0;
				v_setstr(retval, "GDBM_CLOSE: INVALID HANDLE");
				}
			}
		else
			v_setstr(retval, "GDBM_CLOSE: argument error");
		break;

	case	1:	// GDBM_SYNC handle
		success = mod_parint(0, params, param_count, &handle);
		if	( success )	{
			if	( is_valid_handle(handle) )	{
				gdbm_sync(table[handle].dbf);
				success = 1;
				}
			else	{
				success = 0;
				v_setstr(retval, "GDBM_SYNC: INVALID HANDLE");
				}
			}
		else
			v_setstr(retval, "GDBM_SYNC: argument error");
		break;

	default:
		v_setstr(retval, "GDBM: procedure does not exist!");
		}

	return success;
}

/* --- Standard code ------------------------------------------------------------------------------------------------------ */

/*
*	returns the number of the procedures
*/
int		sblib_proc_count(void)
{
	int		i;

	for ( i = 0; proc_names[i]; i ++ );
	return i;
}

/*
*	returns the number of the functions
*/
int		sblib_func_count(void)
{
	int		i;

	for ( i = 0; func_names[i]; i ++ );
	return i;
}

/*
*	returns the 'index' procedure name
*/
int		sblib_proc_getname(int index, char *proc_name)
{
	strcpy(proc_name, proc_names[index]);
	return 1;
}

/*
*	returns the 'index' function name
*/
int		sblib_func_getname(int index, char *proc_name)
{
	strcpy(proc_name, func_names[index]);
	return 1;
}

