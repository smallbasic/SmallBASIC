/**
*	standard c-lib (ANSI or POSIX)
*/

#include "syslib.h"
#include "vmt.h"
#include "smbas.h"

/*
*	setups the random-number generator
*/
void		srand(unsigned int seed)
{
	SysRandom(seed);
}

/*
*	returns the time of CPU (in clocks)
*/
clock_t		clock()
{
	return TimGetTicks();
}

/*
*	returns:
*		on success 0; otherwise -1
*/
int		access(const char *file, int mode)
{
	if ( DmFindDatabase(0, (char*) file) != 0 ) 
		return 0;
	return -1;
}

/*
*/
int		stat(const char *filename, struct stat *buf)
{
	LocalID		lid;

	memset(buf, 0, sizeof(struct stat));

	lid = DmFindDatabase(0, (char *) filename);
	if	( lid )	{
		UInt16	attr, ver;
		UInt32	ctime, mtime, btime;
		UInt32	type, creator;

		DmDatabaseInfo(0, lid, NULL, &attr, &ver, &ctime, &mtime, &btime, NULL, NULL, NULL, &type, &creator);

		buf->st_dev = 0;     	/* device */
		buf->st_mode = 0777;   /* protection */
		buf->st_uid = creator;
		buf->st_gid = type;
		buf->st_ino = lid;      /* inode */

		/* total size, in bytes */
		buf->st_size = 1;

	
		buf->st_blksize = 4096;	/* blocksize for filesystem I/O */
		buf->st_blocks  = 0; 	/* number of blocks allocated */

		// probably in Mac format (1904)
		buf->st_atime   = mtime;    /* time of last access */
		buf->st_mtime   = mtime;    /* time of last modification */
		buf->st_ctime   = ctime;    /* time of last change */

		return 0;
		}

	return -1;
}

/*
*/
char	*getenv(const char *name)
{
	char	var[SB_KEYWORD_SIZE+1];
	static char posix_getenv_buf[SB_TEXTLINE_SIZE+1];	// @#$@#!$#@!
	char	*p;

	strncpy(var, name, SB_KEYWORD_SIZE);
	var[SB_KEYWORD_SIZE] = '\0';
	p = (char *) strchr(var, '=');
	if	( p )
		*p = '\0';
	
	p = dbt_getvar(env_table, var);
	if	( p )	{
		strcpy(posix_getenv_buf, p);
		tmp_free(p);
		return posix_getenv_buf;
		}
	return NULL;
}

/*
*/
int		putenv(char *str)
{
	char	var[SB_KEYWORD_SIZE+1];
	char	*val, *p;

	p = (char *) strchr(str, '=');
	if	( !p )	{
		// delete a variable
		return dbt_setvar(env_table, var, NULL);
		}
	else	{
		// variable's value
		val = p + 1;

		// variable's name
		strncpy(var, str, SB_KEYWORD_SIZE);
		var[SB_KEYWORD_SIZE] = '\0';
		p = (char *) strchr(var, '=');
		*p = '\0';

		return dbt_setvar(env_table, var, val);
		}
	return -1;
}
