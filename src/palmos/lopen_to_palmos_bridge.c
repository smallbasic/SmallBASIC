/**
*	lopen_to_palmos_bridge.c
*
*	open/read/write/close command set for PalmOS
*
*	Nicholas Christopoulos
*/

#include "lopen_bridge.h"

#define	ID_SmBa		0x536D4261		// Creator ID
// Type IDs
#define	ID_DATA		0x44415441		// SB's internal
#define ID_UFST		0x55465354		// User's file ID

int		errno;

#define	MAX_FILE_HANDLES	32
static FileHand	f_handles[MAX_FILE_HANDLES];

/*
*	opens a file and returns the handle. on error returns -1
*/
int		open(const char *pathname, int flags, ...)
{
	long	osflags;
	int		handle, i;
	Err		last_error = 0;

	osflags = fileModeAnyTypeCreator;
	
	if	( flags & O_TRUNC )	{
		if	( DmFindDatabase(0, (char*) pathname) != 0 )
			FileDelete(0, (char*) pathname);
		}

	if	( flags & O_APPEND )	
		osflags |= fileModeAppend;
	else if	( flags & O_RDWR || flags & O_WRONLY )	
	 	osflags |= fileModeUpdate;
	else if	( flags & O_RDONLY )	
	 	osflags |= fileModeReadOnly;
	else
	 	osflags |= (fileModeReadWrite | fileModeDontOverwrite);

	if	( flags & O_EXCL )
		osflags |= fileModeExclusive;

	//
	handle = -1;
	for ( i = 0; i < MAX_FILE_HANDLES; i ++ )	{
		if	( f_handles[i] == 0 )	{
			handle = i;
			break;
			}
		}

	if	( handle == -1 )	{
		errno = fileErrCreateError;	// ???
		return -1;
		}

	//
	if	( strncmp(pathname, "SBI-", 4) == 0 || strstr(pathname, ".sbx") )	
		f_handles[handle] = FileOpen(0, (char *) pathname, ID_DATA, ID_SmBa, osflags, &last_error);
	else
		f_handles[handle] = FileOpen(0, (char *) pathname, ID_UFST, ID_SmBa, osflags, &last_error);

	if	( last_error )	{
		errno = last_error;
		return -1;
		}

	return handle;
}

/*
*	creates a file and returns the handle. on error returns -1
*/
int		creat(const char *pathname, int mode)
{
	return open(pathname, O_CREAT | O_TRUNC);
}

/*
*	reads from a file 'count' bytes
*/
int		read(int handle, void *buf, int count)
{
	Err			last_error;

	if	( handle < 0 )	{
		errno = fileErrInvalidDescriptor;	// EBADF
		return -1;
		}
	
	FileRead(f_handles[handle], buf, count, 1, &last_error);
	if	( last_error )	{
		if	( last_error == fileErrEOF )
			last_error = 0;
		else	{
			errno = last_error;
			return -1;
			}
		}
	return count;
}

/*
*	writes from a file 'count' bytes
*/
int		write(int handle, const void *buf, int count)
{
	Err		last_error;

	if	( handle < 0 )	{
		errno = fileErrInvalidDescriptor;	// EBADF
		return -1;
		}

	FileWrite(f_handles[handle], (void *) buf, count, 1, &last_error);
	if	( last_error )	{
		errno = last_error;
		return -1;
		}
	return count;
}

/*
*	closes a stream
*/
int		close(int handle)
{
//	LocalID		lid;

	if	( handle < 0 )	{
		errno = fileErrInvalidDescriptor;	// EBADF
		return -1;
		}

	errno = FileClose(f_handles[handle]);
	if	( errno )	
		return -1;

	f_handles[handle] = 0;

/*
	// setting the backup-bit
	lid = DmFindDatabase(0, (char *) (f->name));
	if	( lid )	{
		word	attr;

		DmDatabaseInfo(0, lid, NULL, &attr, NULL, NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		attr |= dmHdrAttrBackup;
		DmSetDatabaseInfo(0, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		}
*/
	return 0;
}
 
/*
*	moves file-pointer
*/
long	lseek(int handle, long offset, int whence)
{
	Err		last_error;
	long	newofs;

	if	( handle < 0 )	{
		errno = fileErrInvalidDescriptor;	// EBADF
		return -1;
		}

	if	( !( offset == 0L && whence == SEEK_CUR ) )	{
		switch ( whence )	{
		case	SEEK_SET:
			errno = FileSeek(f_handles[handle], offset, fileOriginBeginning);
			break;
		case	SEEK_CUR:
			errno = FileSeek(f_handles[handle], offset, fileOriginCurrent);
			break;
		case	SEEK_END:
			errno = FileSeek(f_handles[handle], offset, fileOriginEnd);
			break;
		default:
			errno = 1;
			}

		if	( errno )	
			return -1L;
		}

	newofs = FileTell(f_handles[handle], NULL, &last_error);
	if	( last_error )	{
		errno = last_error;
		return -1L;
		}
	return newofs;
}

/*
*/
int		remove(const char *pathname)
{
	errno = FileDelete(0, (char *) pathname);
	return 0;
}

/*
*/
int		rename(const char *oldname, const char *newname)
{
	LocalID		lid;

	lid = DmFindDatabase(0, (char *) (oldname));
	if	( lid )	{
		errno = DmSetDatabaseInfo(0, lid, (char *) newname,
			NULL, NULL, NULL, NULL,	NULL,
			NULL, NULL, NULL, NULL,	NULL );

		if	( errno )
			return -1;
		return 0;
		}
	return -1;
}
