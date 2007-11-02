/**
*	SmallBASIC Unit (SB units) manager
*
*	@author Nicholas Christopoulos
*	@date 2002/07/10
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include "sys.h"
#include "kw.h"
#include "var.h"
#include "device.h"
#include "pproc.h"
#include "scan.h"
#include "units.h"

// units table
static unit_t	*units;
static int		unit_count = 0;

int		find_unit_source(const char *name, char *file)		SEC(BEXEC);

/**
*	initialization
*/
void	unit_mgr_init()
{
	unit_count = 0;
	units = NULL;
}

/**
*	close up
*/
void	unit_mgr_close()
{
	int		i;

	for ( i = 0; i < unit_count; i ++ )	{
		if	( units[i].status == unit_loaded )
			close_unit(i);
		}
	unit_count = 0;
	if	( units )
		tmp_free(units);
}

/**
*	returns the full-pathname of unit
*
*	@param name unit's name
*	@param file buffer to store the filename
*	@return non-zero on success
*/
int		find_unit_source(const char *name, char *file)
{
	char	lname[OS_FILENAME_SIZE+1];
	#if defined(_UnixOS)
	char	*def_path =
			"./:"
			"~/:"
			"~/sbasic/units:"
			"~/sbasic:"
			"~/.sbasic/units:"
			"~/.sbasic:"
			"~/.sbide/units:"
			"~/.sbide:"
			"/usr/lib/sbasic/units:"
			"/usr/share/sbasic/units:"
			"/usr/share/lib/sbasic/units:"
			"/usr/local/lib/sbasic/units:"
			"/usr/local/share/sbasic/units:"
			"/usr/local/share/lib/sbasic/units";
	#elif !defined(_PalmOS)
	char	*def_path =
			".\\;"
			".\\units;"
			"~\\;"
			"~\\units";
	#endif

	strcpy(lname, name);
	strlower(lname);
	sprintf(file, "%s.bas", lname);
	
	#if defined(_UnixOS) || defined(_DOS) || defined(_Win32)
	if	( getenv("BASDIR") )	{
		if	( sys_search_path(getenv("BASDIR"), file, file) ) {
			return 1;
		}
    }

	if	( getenv("SB_UNIT_PATH") )	{
		if	( sys_search_path(getenv("SB_UNIT_PATH"), file, file) ) {
			return 1;
		}
    }

	if	( sys_search_path(def_path, file, file) ) {
		return 1;
    }

	#else
	if	( access(file, R_OK) == 0 )	
		return 1;
	#endif

	return 0;
}

/**
*	returns the full-pathname of unit
*
*	@param name unit's name
*	@param file buffer to store the filename
*	@return non-zero on success
*/
int		find_unit(const char *name, char *file)
{
	char	lname[OS_FILENAME_SIZE+1];
	#if defined(_UnixOS)
	char	*def_path =
			"./:"
			"~/:"
			"~/sbasic/units:"
			"~/sbasic:"
			"~/.sbasic/units:"
			"~/.sbasic:"
			"~/.sbide/units:"
			"~/.sbide:"
			"/usr/lib/sbasic/units:"
			"/usr/share/sbasic/units:"
			"/usr/share/lib/sbasic/units:"
			"/usr/local/lib/sbasic/units:"
			"/usr/local/share/sbasic/units:"
			"/usr/local/share/lib/sbasic/units";
	#elif !defined(_PalmOS)
	char	*def_path =
			".\\;"
			".\\units;"
			"~\\;"
			"~\\units";
	#endif

	strcpy(lname, name);
	strlower(lname);
	sprintf(file, "%s.sbu", lname);
	
	#if defined(_UnixOS) || defined(_DOS) || defined(_Win32)
	if	( getenv("SB_UNIT_PATH") )	{
		if	( sys_search_path(getenv("SB_UNIT_PATH"), file, file) )
			return 1;
		}

	if	( sys_search_path(def_path, file, file) )
		return 1;

	#else
	if	( access(file, R_OK) == 0 )	
		return 1;
	#endif

	return 0;
}

/**
*	open unit
*
*	@param file is the filename
*	@return the unit handle or -1 on error
*/
int		open_unit(const char *file)
{
	int			h, i;
	unit_t		u;
	int			uid = -1;

	char		unitname[OS_PATHNAME_SIZE];
	char		usrcname[OS_PATHNAME_SIZE];
	time_t		ut, st;
	int			comp_rq = 0;

	// clean structure please
	memset(&u, 0, sizeof(unit_t));

	find_unit_source(file, usrcname);	// find unit's source file name
	find_unit(file, unitname);			// find unit's executable file name

	if	( (ut = sys_filetime(unitname)) == 0L )			// binary not found
		comp_rq = 1;									// compile it
	else	{
		if	( (st = sys_filetime(usrcname)) )	{		// source found
			if	( ut < st )
				comp_rq = 1;							// executable is older than source; compile it
			}
		}

	// compilation required
	if	( comp_rq )	{
		if	( !comp_compile(usrcname) )
			return -1;
		find_unit(file, unitname);
		}

	// open unit
	h = open(unitname, O_RDWR | O_BINARY, 0660);
	if	( h == -1 )	
		return -1;

	// read file header
	read(h, &u.hdr, sizeof(unit_file_t));
	if	( memcmp(&u.hdr.sign, "SBUn", 4) != 0 )	{
		close(h);
		return -2;
		}

	// load symbol-table
	u.symbols = (unit_sym_t *) tmp_alloc(u.hdr.sym_count * sizeof(unit_sym_t));
	read(h, u.symbols, u.hdr.sym_count * sizeof(unit_sym_t));

	// setup the rest
	strcpy(u.name, unitname);
	u.status = unit_loaded;

	// add unit
	if ( unit_count == 0 )	{
		// this is the first unit
		uid = unit_count;
		unit_count ++;
		units = (unit_t *) tmp_alloc(unit_count * sizeof(unit_t));
		}
	else	{
		// search for an empty entry
		for ( i = 0; i < unit_count; i ++ )	{
			if	( units[i].status == unit_undefined )	{
				uid = i;
				break;
				}
			}

		// resize the table
		if	( uid == -1 )	{
			uid = unit_count;
			unit_count ++;
			units = (unit_t *) tmp_realloc(units, unit_count * sizeof(unit_t));
			}
		}

	// copy unit's data
	memcpy(&units[uid], &u, sizeof(unit_t));

	// cleanup
	close(h);
	return uid;
}

/**
*	closes a unit
*
*	@param uid is the unit's handle
*	@return 0 on success
*/
int		close_unit(int uid)
{
	if	( uid >= 0 )	{
		unit_t	*u;
		
		u = &units[uid];
		if	( u->status == unit_loaded )	{
			u->status = unit_undefined;
			tmp_free(u->symbols);
			}
		else
			return -2;
		}
	else
		return -1;

	return 0;
}

/**
*	imports unit's names
*
*	@param uid unit's handle
*	@return 0 on success
*/
int		import_unit(int uid)
{
	char	buf[SB_KEYWORD_SIZE+1];
	int		i;

	if	( uid >= 0 )	{
		unit_t	*u;
		
		u = &units[uid];
		if	( u->status == unit_loaded )	{

			for ( i = 0; i < u->hdr.sym_count; i ++ )	{
				// build the name
				sprintf(buf, "%s.%s", u->hdr.base, u->symbols[i].symbol);

				switch ( u->symbols[i].type )	{
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
		else
			return -2;
		}
	else
		return -1;

	return 0;
}

/**
*	execute a call to a unit
*/
int		unit_exec(int lib_id, int index, var_t *ret)
{
	unit_sym_t		*us;		// unit's symbol data
	bc_symbol_rec_t	*ps;		// program's symbol data
	int				my_tid;
	stknode_t		udf_rv;

	my_tid = ctask->tid;
	ps = &prog_symtable[index];
	us = &(taskinfo(ps->task_id)->sbe.exec.exptable[ps->exp_idx]);

	//
	switch ( ps->type )	{
	case	stt_variable:
		break;
	case	stt_procedure:
		exec_sync_variables(1);

		cmd_call_unit_udp(kwPROC, ps->task_id, us->address, 0);

		activate_task(ps->task_id);
		if	( prog_error )	{ gsb_last_error = prog_error; taskinfo(my_tid)->error = gsb_last_error; return 0; }
		bc_loop(2);
		if	( prog_error )	{ gsb_last_error = prog_error; taskinfo(my_tid)->error = gsb_last_error; return 0; }
		activate_task(my_tid);
		exec_sync_variables(0);
		break;

	case	stt_function:
		exec_sync_variables(1);

		cmd_call_unit_udp(kwFUNC, ps->task_id, us->address, us->vid);

		activate_task(ps->task_id);
		if	( prog_error )	{ gsb_last_error = prog_error; taskinfo(my_tid)->error = gsb_last_error; return 0; }
		bc_loop(2);
		if	( prog_error )	{ gsb_last_error = prog_error; taskinfo(my_tid)->error = gsb_last_error; return 0; }

		// get last variable from stack
		code_pop(&udf_rv);
		if	( udf_rv.type != kwTYPE_RET )	
			err_stackmess();
		else	{
			v_set(ret, udf_rv.x.vdvar.vptr);
			v_free(udf_rv.x.vdvar.vptr);				// free ret-var
			tmp_free(udf_rv.x.vdvar.vptr);
			}

		//
		activate_task(my_tid);
		exec_sync_variables(0);
		break;
		};

	return (prog_error == 0);
}





