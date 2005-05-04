/*
*	SmallBASIC module manager
*
*	2001/12/07, Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*
*	WARNING: extlib is out for now (it will replaces by units/generic-lib-code)
*
*/

#include "smbas.h"
#include "extlib.h"
#include "pproc.h"

#if defined(__linux__) && defined(_UnixOS)
#define	LNX_EXTLIB
#endif

#if defined(__CYGWIN__)
#include <w32api/windows.h>
#include <sys/cygwin.h>
#define WIN_EXTLIB
#elif defined(__MINGW32__)
#include <windows.h>
#define WIN_EXTLIB
#endif

#ifdef LNX_EXTLIB
#define LIB_EXT ".so"
#else
#define LIB_EXT ".dll"
#endif

#if defined(LNX_EXTLIB)
#include <dlfcn.h>
#include <dirent.h>
#endif

#if defined(OS_LIMITED)
#define	MAX_SLIB_N	16
#else
#define	MAX_SLIB_N	256
#endif

#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
/* --- Global Symbol Table ----------------------------------------------------------------------------------------------- */

static slib_t	slib_table[MAX_SLIB_N];		/**< module index */
static int		slib_count;					/**< module count */

static ext_proc_node_t*	extproctable;		/**< external procedure table		*/
static int		   		extprocsize;		/**< ext-proc table allocated size	*/
static int		   		extproccount;		/**< ext-proc table count			*/

static ext_func_node_t*	extfunctable; 		/**< external function table		*/
static int				extfuncsize;		/**< ext-func table allocated size	*/
static int				extfunccount;		/**< ext-func table count			*/

/*
*	reset the external proc/func lists
*/
static void	slib_reset_externals(void)
{
	// reset functions
	if	( extfunctable )
		tmp_free(extfunctable);
	extfunctable = NULL;
	extfunccount = extfuncsize = 0;

	// reset procedures
	if	( extproctable )
		tmp_free(extproctable);
	extproctable = NULL;
	extproccount = extprocsize = 0;
}

/*
*	add an external procedure to the list
*/
static int	slib_add_external_proc(const char *proc_name, int lib_id)
{
	// TODO: scan for conflicts
	char	buf[256];

	sprintf(buf,"%s.%s", slib_table[lib_id].name, proc_name);
	strupper(buf);

	if	( extproctable == NULL )	{
		extprocsize   = 16;
		extproctable  = (ext_proc_node_t*) tmp_alloc(sizeof(ext_proc_node_t) * extprocsize);
		}
	else if ( extprocsize <= (extproccount+1) )	{
		extprocsize  += 16;
		extproctable  = (ext_proc_node_t*) tmp_realloc(extproctable, sizeof(ext_proc_node_t) * extprocsize);
		}

	extproctable[extproccount].lib_id = lib_id;
	extproctable[extproccount].symbol_index = 0;
	strcpy(extproctable[extproccount].name, buf);
	strupper(extproctable[extproccount].name);

	if	( opt_verbose )	
		printf("LID: %d, Idx: %d, PROC '%s'\n", lib_id, extproccount, extproctable[extproccount].name);

	extproccount ++;
	return extproccount-1;
}

/*
*	Add an external function to the list
*/
static int	slib_add_external_func(const char *func_name, int lib_id)
{
	char	buf[256];

	sprintf(buf,"%s.%s", slib_table[lib_id].name, func_name);
	strupper(buf);

	// TODO: scan for conflicts
	if	( extfunctable == NULL )	{
		extfuncsize   = 16;
		extfunctable  = (ext_func_node_t*) tmp_alloc(sizeof(ext_func_node_t) * extfuncsize);
		}
	else if ( extfuncsize <= (extfunccount+1) )	{
		extfuncsize  += 16;
		extfunctable  = (ext_func_node_t*) tmp_realloc(extfunctable, sizeof(ext_func_node_t) * extfuncsize);
		}

	extfunctable[extfunccount].lib_id = lib_id;
	extfunctable[extfunccount].symbol_index = 0;
	strcpy(extfunctable[extfunccount].name, buf);
	strupper(extfunctable[extfunccount].name);

	if	( opt_verbose )	
		printf("LID: %d, Idx: %d, FUNC '%s'\n", lib_id, extfunccount, extfunctable[extfunccount].name);

	extfunccount ++;
	return extfunccount-1;
}

/*
*	returns the external procedure id
*/
static int	slib_is_external_proc(const char *name)
{
	int		i;

	for ( i = 0; i < extproccount; i ++ )	{
		if	( strcmp(extproctable[i].name, name) == 0 )
			return i;
		}
	return -1;
}

/*
*	returns the external function id
*/
static int	slib_is_external_func(const char *name)
{
	int		i;

	for ( i = 0; i < extfunccount; i ++ )	{
		if	( strcmp(extfunctable[i].name, name) == 0 )
			return i;
		}
	return -1;
}
#endif

/**
*	returns the ID of the keyword
*/
int		slib_get_kid(const char *name)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int		i;

	for ( i = 0; i < extproccount; i ++ )	{
		if	( strcmp(extproctable[i].name, name) == 0 )
			return i;
		}
	for ( i = 0; i < extfunccount; i ++ )	{
		if	( strcmp(extfunctable[i].name, name) == 0 )
			return i;
		}
#endif
	return -1;	
}

/**
*	returns the library-id (index of library of the current process)
*/
int		slib_get_module_id(const char *name)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int		i;
	char	xname[OS_FILENAME_SIZE+1];
	slib_t	*lib;

	strcpy(xname, name);
    strcat(xname, LIB_EXT);
	for ( i = 0; i < slib_count; i ++ )	{
		lib = &slib_table[i];
//		printf("slib: %s=%s\n",  lib->name, name);
		if	( strcasecmp(lib->name, name) == 0 )
			return i;
		}
#endif
	return -1;	// not found
}

/**
*	updates compiler with the module's keywords
*/
void	slib_setup_comp(int mid)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int		i;

	for ( i = 0; i < extproccount; i ++ )	{
		if	( extproctable[i].lib_id == mid )	
			comp_add_external_proc(extproctable[i].name, mid);
		}
	for ( i = 0; i < extfunccount; i ++ )	{
		if	( extfunctable[i].lib_id == mid )	
			comp_add_external_func(extfunctable[i].name, mid);
		}
#endif
}

/* --- System Load/Execute ----------------------------------------------------------------------------------------------- */

/*
*	retrieve the function pointer
*/
void    *slib_getoptptr(slib_t *lib, const char *name)
{
#if defined(LNX_EXTLIB)
	return	dlsym(lib->handle, name);
#elif defined(WIN_EXTLIB)
	return	GetProcAddress((HMODULE)lib->handle, name);
#else
	return NULL;
#endif
}

/*
*	retrieve the function pointer; error if its not exists
*/
void    *slib_getptr(slib_t *lib, const char *name)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	void    *ptr;

	ptr = slib_getoptptr(lib->handle, name);
	if   ( !ptr )
		panic("SB-LibMgr: %s, missing function %s\n", lib->name, name);
	return ptr;
#else
	return NULL;
#endif
}

/*
*	open library (basic open)
*/
int		slib_llopen(slib_t *lib)
{
#if defined(LNX_EXTLIB)
  lib->handle = dlopen(lib->fullname, RTLD_NOW);
  if  ( lib->handle == NULL )
    panic("SB-LibMgr: error on loading %s\n%s", lib->name, dlerror());
  return (lib->handle != NULL);
#elif defined(__CYGWIN__)
   char win32Path[1024];
   cygwin_conv_to_full_win32_path(lib->fullname, win32Path);
   lib->handle = LoadLibrary(win32Path);
   if ( lib->handle == NULL )
       panic("SB-LibMgr: error on loading %s\n", win32Path);
   return (lib->handle != NULL);
#elif defined(WIN_EXTLIB)
   lib->handle = LoadLibrary(lib->fullname);
   if ( lib->handle == NULL )
       panic("SB-LibMgr: error on loading %s\n", lib->name);
   return (lib->handle != NULL);
#else
   return 0;
#endif
}

/*
*	close library (basic close)
*/
int		slib_llclose(slib_t *lib)
{
#if defined(LNX_EXTLIB)
	if	( !lib->handle )
		return 0;

	dlclose(lib->handle);
	lib->handle = NULL;
	return 1;
#elif defined(WIN_EXTLIB)
	if	( !lib->handle )
		return 0;

	FreeLibrary(lib->handle);
	lib->handle = NULL;
	return 1;
#else
	return 1;
#endif
}

/*
*/
void	slib_import_routines(slib_t *lib)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int		i, count, fc, pc;
	char	buf[SB_KEYWORD_SIZE];
	int		(*fgetname)(int,char*);
	int		(*fcount)(void);

	lib->first_proc = extproccount;
	lib->first_func = extfunccount;

	fcount   = (int (*)(void)) slib_getoptptr(lib, "sblib_proc_count");
	fgetname = (int	(*)(int,char*)) slib_getoptptr(lib, "sblib_proc_getname");
	if	( fcount )	{
		count = fcount();
		for ( i = 0; i < count; i ++ )	{ 
			if	( fgetname(i, buf) )	
				slib_add_external_proc(buf, lib->id);
			}
		}

	fcount   = (int (*)(void)) slib_getoptptr(lib, "sblib_func_count");
	fgetname = (int	(*)(int,char*)) slib_getoptptr(lib, "sblib_func_getname");
	if	( fcount )	{
		count = fcount();
		for ( i = 0; i < count; i ++ )	{ 
			if	( fgetname(i, buf) )
				slib_add_external_func(buf, lib->id);
			}
		}
#endif
}

/*
*	load a lib
*/
void	slib_import(const char *name, const char *fullname)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	slib_t	*lib;
	int		(*minit)(void);
	int		(*mtype)(void);
	void	(*mdrvname)(char*);
	int		mok = 0;
	
	lib = &slib_table[slib_count];
	memset(lib, 0, sizeof(slib_t));
	strncpy(lib->name, name, 255);
	strncpy(lib->fullname, fullname, 1023);
	lib->id = slib_count;

	if	( !opt_quite )
		printf("SB-LibMgr: importing %s", fullname);

	if	( slib_llopen(lib) )	{
		mok = 1;

		// init
		minit = (int(*)(void)) slib_getoptptr(lib, "sblib_init");
		if	( minit )	{
			if	( !minit() )	{
				mok = 0;
				panic("SB-LibMgr: %s->sblib_init(), failed", lib->name);
				}
			}
		
		// get type
		mtype = (int(*)(void)) slib_getoptptr(lib, "sblib_type");
		if	( mtype )
			lib->type = mtype();
		else
			lib->type = lib_lang_ext;	// default type

		// get info
		switch ( lib->type )	{
		case	lib_lang_ext:
			slib_import_routines(lib);
			mok = 1;
			break;
		case	lib_vfs_driver:
			mdrvname = (void(*)(char*)) slib_getptr(lib, "sblib_vfsname");
			memset(lib->vfs_drvname, 0, 5);
			mdrvname(lib->vfs_drvname);
			mok = 1;
			break;
		default:
			panic("SB-LibMgr: %s->sbmod_type(), type %d is not supported", lib->type);
			mok = 0;
			};
		}
	else
		printf("SB-LibMgr: can't open %s", fullname);

	if	( mok )	{
		slib_count ++;
		if	( !opt_quite )
			printf("... done\n");
		}
	else	{
		if	( !opt_quite )
			printf("... error\n");
		}
#endif
}

/*
*	scan libraries
*/
void	sblmgr_scanlibs(const char *path)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	DIR				*dp;
	struct dirent	*e;
	char			*name, *p;
	char			full[1024], libname[256];

    if  ( (dp = opendir(path)) == NULL ) {
        if  ( !opt_quite) {
            printf("SB-LibMgr: module path %s not found.\n", path);
        }
		return;
    }

	while ( (e = readdir(dp)) != NULL )	{
		name = e->d_name;
		if	( (strcmp(name, ".") == 0) || (strcmp(name, "..") == 0) )
			continue;
        if  ( (p = strstr(name, LIB_EXT)) != NULL )  {
            if  ( strcmp(p, LIB_EXT) == 0 )  {
				// store it
				
				strcpy(libname, name);
				p = strchr(libname, '.');
				*p = '\0';

				strcpy(full, path);
        if (path[strlen(path)-1] != '/') {
            strcat(full, "/"); // add trailing separator
        }
				strcat(full, name);

				slib_import(libname, full);
				}
			}
		}

	closedir(dp);
#endif
}

/*
*	slib-manager: initialize manager
*/
void	sblmgr_init(int mcount, const char *mlist)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int		all=0;

	slib_count = 0;

	if	( !opt_quite && mcount )
		printf("SB-LibMgr: scanning for modules...\n");
	if	( mcount )	{
		if	( mlist )	{
			if	( strlen(mlist) == 0 )
				all = 1;
			else
				all = 1;
			// TODO: else load the specified modules
			}
		else
			all = 1;

		if	( all )	{
#if defined(LNX_EXTLIB)
       sblmgr_scanlibs("/usr/local/lib/sbasic/modules/");
#elif defined(__CYGWIN__) || defined(__MINGW32__)
       // the -m argument specifies the location of all modules
       sblmgr_scanlibs(opt_modlist);
#elif defined(WIN_EXTLIB)
       sblmgr_scanlibs("c:\\sbasic\\modules");
       sblmgr_scanlibs("sbasic\\modules");
#endif
			}
		}
	if	( !opt_quite )
		printf("\n");
#endif
}

/*
*	slib-manager: close everything
*/
void	sblmgr_close()
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int		i;
	slib_t	*lib;
	void	(*mclose)(void);

	for ( i = 0; i < slib_count; i ++ )	{
		lib = &slib_table[i];
		if	( lib->handle )	{
			mclose = (void(*)(void)) slib_getoptptr(lib, "sblib_close");
			if	( mclose )	
				mclose();
			slib_llclose(lib);
			}
		}
#endif
}

/**
*	@ingroup mod
*
*	search modules for a vfsmodule with that driver-name.
*
*	@param name the name of the driver (char[5])
*	@return lib-id on success; otherwise -1
*/
int		sblmgr_getvfs(const char *name)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int		i;
	slib_t	*lib;

	for ( i = 0; i < slib_count; i ++ )	{
		lib = &slib_table[i];
		if	( lib->type == lib_vfs_driver )	{
			if	( strncmp(lib->vfs_drvname, name, 5) == 0 )
				return i;
			}
		}
#endif
	return -1;
}

/*
*	returns the 'index' function-name of the 'lib'
*/
int		sblmgr_getfuncname(int lib_id, int index, char *buf)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	slib_t	*lib;
	int		(*mgf)(int,char*);

	buf[0] = '\0';
	if	( lib_id < 0 || lib_id >= slib_count )	
		return 0;	// error

	lib = &slib_table[lib_id];
	mgf = (int(*)(int,char*)) slib_getoptptr(lib, "sblib_func_getname");
	if	( mgf == NULL )
		return 0;	// error

	return mgf(index, buf);
#else
	return 0;
#endif
}

/*
*	returns the 'index' procedure-name of the 'lib'
*/
int		sblmgr_getprocname(int lib_id, int index, char *buf)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	slib_t	*lib;
	int		(*mgp)(int,char*);

	buf[0] = '\0';
	if	( lib_id < 0 || lib_id >= slib_count )	
		return 0;	// error

	lib = &slib_table[lib_id];
	mgp = (int(*)(int,char*)) slib_getoptptr(lib, "sblib_proc_getname");
	if	( mgp == NULL )
		return 0;	// error

	return mgp(index, buf);
#else
	return 0;
#endif
}

/*
*	build parameter table
*/
int		slib_build_ptable(slib_par_t *ptable)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int			pcount = 0;
	var_t		*arg = NULL;
	byte		ready, code;
	addr_t		ofs;

	if	( code_peek() == kwTYPE_LEVEL_BEGIN )	{
		code_skipnext();		// kwTYPE_LEVEL_BEGIN

		ready = 0;
		do	{
			code = code_peek();
			switch ( code )	{
			case	kwTYPE_EOC:
				code_skipnext();
				break;
			case	kwTYPE_SEP:			// separator 
				code_skipsep();
				break;
			case	kwTYPE_LEVEL_END:	// ) -- end of parameters
				ready = 1;
				break;
			case	kwTYPE_VAR:		// variable
				ofs = prog_ip;		// store IP

				if	( code_isvar() )	{
					// push parameter
					ptable[pcount].var_p = code_getvarptr();
					ptable[pcount].byref = 1;
					pcount ++;
					break;
					}

				prog_ip = ofs;	// restore IP
				// no 'break' here
			default:
				// default --- expression (BYVAL ONLY)
				arg = v_new();
				eval(arg);
				if	( !prog_error )	{
					// push parameter
					ptable[pcount].var_p = arg;
					ptable[pcount].byref = 0;
					pcount ++;
					}
				else	{
					v_free(arg);
					tmp_free(arg);
					return pcount;
					}

				}

			} while ( !ready );

		code_skipnext();		// kwTYPE_LEVEL_END
		}

	return pcount;
#else
	return 0;
#endif
}

/*
*	free parameter table
*/
void	slib_free_ptable(slib_par_t *ptable, int pcount)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	int	i;

	for ( i = 0; i < pcount; i ++ )	{
		if	( ptable[i].byref == 0 )	{
			v_free(ptable[i].var_p);
			tmp_free(ptable[i].var_p);
			}
		}
#endif
}

/*
*	execute a procedure
*/
int		sblmgr_procexec(int lib_id, int index)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	slib_t	*lib;
	var_t	ret;
	slib_par_t *ptable = NULL;
	int		(*pexec)(int, int, slib_par_t *, var_t *);
	int		pcount = 0;
	int		success = 0;

	if	( lib_id < 0 || lib_id >= slib_count )	{
		// rt_raise(...)
		return 0;	// error
		}

	lib = &slib_table[lib_id];
	if	( lib->type == lib_vfs_driver )
		pexec = slib_getoptptr(lib, "sblib_vfs_exec");
	else
		pexec = slib_getoptptr(lib, "sblib_proc_exec");
	if	( pexec == NULL )	{
		// rt_raise(...)
		return 0;	// error
		}

	//	build parameter table
	ptable = tmp_alloc(sizeof(slib_par_t) * 64);	// 64 = maximum parameter
	pcount = slib_build_ptable(ptable);
	if	( prog_error )	{
		slib_free_ptable(ptable, pcount);
		return 0;
		}

	// exec
	v_init(&ret);
	success = pexec(index - lib->first_proc, pcount, ptable, &ret);
	
	// error
	if	( !success )	{
		if	( ret.type == V_STR )
			rt_raise("SB-LibMgr:\n%s: %s\n", lib->name, ret.v.p.ptr);
		else
			rt_raise("SB-LibMgr:\n%s: not specified error\n", lib->name);
		}

	//	clean-up
	if	( ptable )	{
		slib_free_ptable(ptable, pcount);
		tmp_free(ptable);
		}

	v_free(&ret);

	return success;
#else
	return 0;
#endif
}

/*
*	execute a function
*/
int		sblmgr_funcexec(int lib_id, int index, var_t *ret)
{
#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
	slib_t	*lib;
	slib_par_t *ptable = NULL;
	int		(*pexec)(int, int, slib_par_t *, var_t *);
	int		pcount = 0;
	int		success = 0;

	if	( lib_id < 0 || lib_id >= slib_count )	{
		// rt_raise(...)
		return 0;	// error
		}

	lib = &slib_table[lib_id];
	if	( lib->type == lib_vfs_driver )
		pexec = slib_getoptptr(lib, "sblib_vfs_exec");
	else
		pexec = slib_getoptptr(lib, "sblib_func_exec");
	if	( pexec == NULL )	{
		// rt_raise(...)
		return 0;	// error
		}

	//	build parameter table
	ptable = tmp_alloc(sizeof(slib_par_t) * 64);	// 64 = maximum parameter
	pcount = slib_build_ptable(ptable);
	if	( prog_error )	{
		slib_free_ptable(ptable, pcount);
		return 0;
		}

	// exec
	v_init(ret);
	success = pexec(index - lib->first_func, pcount, ptable, ret);
	
	// error
	if	( !success )	{
		if	( ret->type == V_STR )
			rt_raise("SB-LibMgr:\n%s: %s\n", lib->name, ret->v.p.ptr);
		else
			rt_raise("SB-LibMgr:\n%s: (error not specified)\n", lib->name);
		}

	//	clean-up
	if	( ptable )	{
		slib_free_ptable(ptable, pcount);
		tmp_free(ptable);
		}

	return success;
#else
	return 0;
#endif
}

/**
*	@ingroup mod
*
*	executes a vfs standard function
*
*	@param func the function's index
*	@param f the file structure
*	@return it is depended on 'func'
*/
long	sblmgr_vfsexec(enum slib_vfs_idx_t func, dev_file_t *f, ...)
{
	va_list		ap;
	long		retval = 0;

	va_start(ap, f);
	#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
		{
		slib_t	*lib;

		lib = &slib_table[f->vfslib];

		switch ( func )	{
		case	lib_vfs_open:
			{
				int	(*func)(const char *name, int flags);
				func = slib_getoptptr(lib->handle, "vfs_open");
				if	( func )	{
					f->last_error = f->handle = func(f->name+5, f->open_flags);
					retval = (f->handle >= 0);
					}
			}
			break;
		case	lib_vfs_close:
			{
				int	(*func)(int handle);
				func = slib_getoptptr(lib->handle, "vfs_close");
				if	( func )	{
					retval = func(f->handle);
					f->handle = -1;
					}
			}
			break;
		case	lib_vfs_read:
			{
				long	(*func)(int handle, char *data, long size);
				byte	*data;
				dword	size;

				data = va_arg(ap, byte *);
				size = va_arg(ap, dword);
				func = slib_getoptptr(lib->handle, "vfs_read");
				if	( func )	
					retval = (func(f->handle, data, size) == size);
			}
			break;
		case	lib_vfs_write:
			{
				long	(*func)(int handle, char *data, long size);
				byte	*data;
				dword	size;

				data = va_arg(ap, byte *);
				size = va_arg(ap, dword);
				func = slib_getoptptr(lib->handle, "vfs_write");
				if	( func )	
					retval = (func(f->handle, data, size) == size);
			}
			break;
		case	lib_vfs_eof:
			{
				int	(*func)(int handle);
				func = slib_getoptptr(lib->handle, "vfs_eof");
				if	( func )	
					retval = func(f->handle);
			}
		case	lib_vfs_tell:
			{
				long (*func)(int handle);
				func = slib_getoptptr(lib->handle, "vfs_tell");
				if	( func )	
					retval = func(f->handle);
			}
		case	lib_vfs_length:
			{
				long (*func)(int handle);
				func = slib_getoptptr(lib->handle, "vfs_length");
				if	( func )	
					retval = func(f->handle);
			}
		case	lib_vfs_seek:
			{
				long (*func)(int handle, long offset);
				dword offset;

				offset = va_arg(ap, dword);
				func = slib_getoptptr(lib->handle, "vfs_seek");
				if	( func )
					retval = func(f->handle, offset);
			}
		default:
			// error
			;
			}
		}
	#endif
	va_end(ap);
	return retval;
}

/**
*	@ingroup mod
*
*	executes a vfs directory-function
*
*	@param func the function's index
*	@param lib the lib's id
*	@return it is depended on 'func'
*/
long	sblmgr_vfsdirexec(enum slib_vfs_idx_t func, int ilib, ...)
{
	va_list		ap;
	int			retval = 0;

	va_start(ap, ilib);
	#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)
		{
		slib_t	*lib;

		lib = &slib_table[ilib];

		switch ( func )	{
		case lib_vfs_chmod:
			{
				long	(*func)(const char *name, int mode);
				char	*name;
				int		mode;

				name = va_arg(ap, char *);
				mode = va_arg(ap, int);
				func = slib_getoptptr(lib->handle, "vfs_chmod");
				if	( func )	
					retval = (func(name, mode) == 0);
			}
			break;
		case lib_vfs_access:
			{
				long	(*func)(const char *name, int mode);
				char	*name;
				int		mode;

				name = va_arg(ap, char *);
				mode = va_arg(ap, int);
				func = slib_getoptptr(lib->handle, "vfs_access");
				if	( func )	
					retval = func(name, mode);
			}
			break;
		case	lib_vfs_attr:
		case	lib_vfs_exist:
		case	lib_vfs_remove:
		case	lib_vfs_list:
		case	lib_vfs_chdir:
		case	lib_vfs_mkdir:
		case	lib_vfs_rmdir:
		default:
			;
			}
		}
	#endif
	va_end(ap);
	return retval;
}

/* ------------------------------------------------ Extra Module Support Routines ----------------------------------------- */

#if defined(LNX_EXTLIB) || defined(WIN_EXTLIB)

int		mod_parint(int n, slib_par_t *params, int param_count, int *val)
{
	var_t	*param;

	if	( prog_error )					return 0;
	if	( n < 0 || n >= param_count )	{ prog_error = 1; return 0; }
	param = params[n].var_p;
	*val = v_igetval(param);

	return 1;
}

int		mod_opt_parint(int n, slib_par_t *params, int param_count, int *val, int def_val)
{
	var_t	*param;

	if	( prog_error )					return 0;
	if	( n < 0 )						{ prog_error = 1; return 0; }
	if	( n < param_count )	{
		param = params[n].var_p;
		*val = v_igetval(param);
		}
	else
		*val = def_val;

	return 1;
}

int		mod_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr)
{
	var_t	*param;

	if	( prog_error )					return 0;
	if	( n < 0 || n >= param_count )	{ prog_error = 1; return 0; }
	param = params[n].var_p;

	if	( param->type != V_STR )
		v_tostr(param);
	*ptr = param->v.p.ptr;

	return 1;
}

int		mod_opt_parstr_ptr(int n, slib_par_t *params, int param_count, char **ptr, const char *def_val)
{
	var_t	*param;

	if	( prog_error )					return 0;
	if	( n < 0 )						{ prog_error = 1; return 0; }

	if	( n < param_count )	{
		param = params[n].var_p;

		if	( param->type != V_STR )
			v_tostr(param);
		*ptr = param->v.p.ptr;
		}
	else
		*ptr = def_val;

	return 1;
}

#endif


