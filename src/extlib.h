/**
*	@file extlib.h
*	SmallBASIC module (shared-lib) manager
*
*	@author Nicholas Christopoulos
*	@date 2001/12/07
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

/**
*	@ingroup mod
*	@page moddoc Modules
*
*	The modules are common shared-libraries which can call back the SB's code.
*
*	The module-manager loads the modules at the startup and unloads them
*	before SB's exit. Which module will be loaded is specified by the
*	user in SB's command-line parameters (option -m). The path of the modules
*	is predefined to /usr/lib/sbasic/modules and/or /usr/local/lib/sbasic/modules
*
*	<b>Standard interface</b>
*
*	All modules must provides at least the three following functions:
*	sblib_init(), sblib_close, sblib_type().
*
*	the sblib_init() called on startup, the sblib_close() called on SB's exit,
*	and the sblib_type() called after sblib_init() to inform the module-manager
*	about the type of the module (slib_tp).
*
*	<b>Library-modules</b>
*
*	The library modules must returns as type (sblib_type()) the lib_lang_ext 
*	and provide the library-related functions (@ref modlib).
*
*	The SB before compiles the .bas program, the module-manager
*	asks every module about the number of the functions and procedures
*	which are supported. (sblib_proc_count, sblib_func_count)
*
*	It continues by asking the name of each function and procedure and
*	updates the compiler. (sblib_proc_getname, sblib_func_getname)
*
*	On the execution time, if the program wants to execute a library's
*	procedure or function, the module-manager builds the parameter table
*	and calls the sblib_proc_exec or the sblib_func_exec.
*
*	See modules/example1.c
*
*	<b>VFS-driver-modules</b>
*
*	The vfs-driver modules are similar to library with the exception that
*	must using a specified set of functions/procedures with specified
*	parameters.
*
*	That set of functions/procedures is the same with the build-in 
*	file-system (see fs_stream.c)
*
*	<b>Notes:</b>
*
*	Procedure & functions names are limited to 32 characters (33 with \0)
*/

/*
*	VFS extention its not working, yet!
*/
/**
*	@defgroup mod Module Manager
*/
/**
*	@defgroup modstd Module interface - Standard
*/
/**
*	@defgroup modlib Module interface - Library
*/
/**
*	@defgroup modvfs Module interface - VFS driver
*/

#if !defined(_sb_extlib_h)
#define	_sb_extlib_h

#include "sys.h"
#include "var.h"
#include "device.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
*	@ingroup mod
*	@typedef slib_tp
*	shared-lib supported API 
*/
typedef enum {
	lib_lang_ext,		/**< Language extention */
	lib_vfs_driver,		/**< VFS driver */

	lib_null
	} slib_tp;

/**
*	@ingroup mod
*	@typedef slib_t
*	shared-lib information structure
*/
typedef struct {
	int		id;				/**< library ID */
#if defined(OS_LIMITED)
	char	name[32];		/**< name of library (basename) */
#else
	char	name[256];		/**< name of library (basename) */
	char	fullname[1024];	/**< full pathname */
#endif
	void	*handle;		/**< handle to the lib */
	slib_tp type;			/**< type of the API */
	dword	flags;			/**< flags */

	/* ------------------  Language  ------------------ */

	int		proc_count;		/**< if lang.ext.; number of procedures */
	int		func_count;		/**< if lang.ext.; number of functions */

	/* ------------------ VFS driver ------------------ */

	char	vfs_drvname[6]; 	/**< if vfs; VFS driver name (ex: MEMO:, PDOC:, etc) */

	}	slib_t;

/**
*	@ingroup modvfs
*	@enum slib_vfs_idx_t
*	function 'index' values for VFS drivers
*/
enum slib_vfs_idx_t	{

	/* ---- required ---- */

	lib_vfs_open,					// open a file	
	lib_vfs_close,					// close a file	
	lib_vfs_read,					// read			
	lib_vfs_write,					// write
	lib_vfs_eof,					// EOF

	/* ---- optional ---- */

	lib_vfs_tell,					// position in file
	lib_vfs_length,					// length of file
	lib_vfs_seek,					// seek

	lib_vfs_chmod,					// chmod
	lib_vfs_access,					// returns the file attributes 
	lib_vfs_attr,					// 
	lib_vfs_exist,					// 
	lib_vfs_remove,					// remove a file

	lib_vfs_list,					// returns array of filenames

	lib_vfs_chdir,					// change current directory
	lib_vfs_mkdir,					// create a new directory
	lib_vfs_rmdir,					// remove a directory

	lib_vfs_null					// not used
	};	

/*
* 	slib_t flags
*/
//#define	SBL_LOADED		1

/**
*	@ingroup mod
*
*	Initialize module-manager
*
*	default path /usr/lib/sbasic/modules/:/usr/local/lib/sbasic/modules/
*
*	@param mcount non-zero for check for modules
*	@param list the list of the modules or "" for auto-search/load-all
*/
void	sblmgr_init(int mcount, const char *list)			SEC(TRASH);

/**
*	@ingroup mod
*
*	close module manager
*/
void	sblmgr_close(void)									SEC(TRASH);

/**
*	@ingroup mod
*
*	returns a library's ID 
*
*	@param name is the name of the library (without the file-extention)
*	@return the id or -1 for error
*/
int		slib_get_module_id(const char *name)				SEC(TRASH);

/**
*	@ingroup mod
*
*	updates the compiler with the module (mid) keywords
*/
void	slib_setup_comp(int mid)							SEC(TRASH);

/**
*	@ingroup mod
*	
*	returns the ID of the keyword. used at run-time to assign BCs ID with slib_mgr's one
*/
int		slib_get_kid(const char *name)						SEC(TRASH);

/**
*	@ingroup mod
*
*	returns a library's function name
*
*	@param lib is the lib-id
*	@param index is the index of the function
*	@param buf is the buffer to store the name
*	@return non-zero on success
*/
int		sblmgr_getfuncname(int lib, int index, char *buf)	SEC(TRASH);

/**
*	@ingroup mod
*
*	returns a library's procedure name 
*
*	@param lib is the lib-id
*	@param index is the index of the procedure
*	@param buf is the buffer to store the name
*	@return non-zero on success
*/
int		sblmgr_getprocname(int lib, int index, char *buf)	SEC(TRASH);

/**
*	@ingroup mod
*
*	execute a library's procedure
*
*	@param lib is the lib-id
*	@param index is the index of the procedure
*	@return non-zero on success
*/
int		sblmgr_procexec(int lib, int index)					SEC(TRASH);

/**
*	@ingroup mod
*
*	execute a library's function
*
*	@param lib is the lib-id
*	@param index is the index of the function
*	@param ret is the variable to store the result
*	@return non-zero on success
*/
int		sblmgr_funcexec(int lib, int index, var_t *ret)		SEC(TRASH);

/**
*	@ingroup mod
*
*	search modules for a vfsmodule with that driver-name.
*
*	@param name the name of the driver (char[6], like "COM1:")
*	@return lib-id on success; otherwise -1
*/
int		sblmgr_getvfs(const char *name)						SEC(TRASH);

/**
*	@ingroup mod
*
*	executes a vfs standard function
*
*	@param func the function's index
*	@param f the file structure
*	@return it is depended on 'func'
*/
long	sblmgr_vfsexec(enum slib_vfs_idx_t func, dev_file_t *f, ...)		SEC(TRASH);

/**
*	@ingroup mod
*
*	executes a vfs directory-function
*
*	@param func the function's index
*	@param lib the lib's id
*	@return it is depended on 'func'
*/
long	sblmgr_vfsdirexec(enum slib_vfs_idx_t func, int lib, ...)		SEC(TRASH);

/* --------------------------------------------------- Common interface --------------------------------------------------- */

/**
*	@ingroup modstd
*	@typedef slib_par_t
*
*	Parameter structure
*/
typedef struct	{
	var_t		*var_p;		/**< the parameter itself */
	byte		byref;		/**< parameter can be used as byref */
	} slib_par_t;

/**
*	@ingroup modstd
*
*	Initialize the library. Called by module manager on loading.
*
*	@return non-zero on success
*/
int		sblib_init(void);	

/**
*	@ingroup modstd
*
*	Closes the library. Called by module manager on unload.
*/
void	sblib_close(void);

/**
*	@ingroup modstd
*
*	returns the type of the library (slib_tp)
*
*	@return the type of the library
*/
int		sblib_type(void);	// returns the 'slib_tp' value (type of API)

/* ------------------------------------------------------  Language  ------------------------------------------------------ */

/**
*	@ingroup modlib
*
*	returns the number of procedures that are supported by the library
*
*	@return the number of the procedures
*/
int		sblib_proc_count(void);

/**
*	@ingroup modlib
*
*	returns the name of the procedure 'index'
*
*	@param index the procedure's index
*	@param proc_name the buffer to store the name
*	@return non-zero on success
*/
int		sblib_proc_getname(int index, char *proc_name);		

/**
*	@ingroup modlib
*
*	executes a procedure
*
*	the retval can be used to returns an error-message
*	in case of an error.
*
*	@param index the procedure's index
*	@param param_count the number of the parameters
*	@param params the parameters table
*	@param retval a var_t object to set the return value
*	@return non-zero on success
*/
int		sblib_proc_exec(int index, int param_count,	   		// executes the 'index' procedure
			slib_par_t *params, var_t *retval);

/**
*	@ingroup modlib
*
*	returns the number of functions that are supported by the library
*
*	@return the number of the functions
*/
int		sblib_func_count(void);

/**
*	@ingroup modlib
*
*	returns the name of the function 'index'
*
*	@param index the function's index
*	@param func_name the buffer to store the name
*	@return non-zero on success
*/
int		sblib_func_getname(int index, char *func_name);		// returns the 'index' function name

/**
*	@ingroup modlib
*
*	executes a function
*
*	@param index the procedure's index
*	@param param_count the number of the parameters
*	@param params the parameters table
*	@param retval a var_t object to set the return value
*	@return non-zero on success
*/
int		sblib_func_exec(int index, int param_count,			// executes the 'index' function
			slib_par_t *params, var_t *retval);

/* ------------------------------------------------------ VFS Driver ------------------------------------------------------ */

/**
*	@ingroup modvfs
*
*	returns the driver name (char[6]).
*	(like COM1:, MEMO:, etc)
*
*	@param dest is a buffer of 6 bytes size to store the name
*/
void	sblib_vfsname(char*dest);

/**
*	@ingroup modvfs
*
*	executes a VFS function
*
*	@param index (slib_vfs_idx_t) the function's index
*	@param param_count the number of the parameters
*	@param params the parameters table
*	@param retval a var_t object to set the return value
*	@return non-zero on success
*/
int		sblib_vfs_exec(int index, int param_count,			// executes the 'index' function
			slib_par_t *params, slib_par_t *retval);

/* ------------------------------------------------ Extra Module Support Routines ----------------------------------------- */

#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)

typedef	char* mod_keyword_t;

int		mod_parint(int n, slib_par_t *params, int param_count, int *val);
int		mod_opt_parint(int n, slib_par_t *params, int param_count, int *val, int def_val);
int		mod_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr);
int		mod_opt_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr, const char *def_val);

#endif

#if defined(__cplusplus)
}
#endif

#endif

