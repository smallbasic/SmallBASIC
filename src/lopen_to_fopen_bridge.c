/**
*	lopen_to_fopen_bridge.c
*
*	On some small systems like Helio(VTOS) and PalmOS there is no support for open/close command set
*
*	If your system has supports for fopen/fclose set then you can use this module
*
*	Nicholas Christopoulos
*/

#include <stdio.h>
#include <string.h>
#include "lopen_bridge.h"

/*
*	opens a file and returns the handle. on error returns -1
*/
int		open(const char *pathname, int flags, ...)
{
	FILE	*fp;
	char	mode[4];

	strcpy(mode, "r+");
	if	( (flags & O_CREAT) || (flags & O_TRUNC) )	strcpy(mode, "w+");
	else if ( flags & O_APPEND )	strcpy(mode, "a+");
	else if	( flags & O_RDONLY )	strcpy(mode, "r");
	else if	( flags & O_WRONLY )	strcpy(mode, "w");
	// strcat(mode, "b");

	fp = fopen(pathname, mode);
	if	( !fp )
		return -1;
	return (int) fp;
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
int		read(int fd, void *buf, int count)
{
	return fread(buf, count, 1, (FILE *) fd);
}

/*
*	writes from a file 'count' bytes
*/
int		write(int fd, const void *buf, int count)
{
	return fwrite(buf, count, 1, (FILE *) fd);
}

/*
*	closes a stream
*/
int		close(int fd)
{
	return fclose((FILE *) fd);
}
 
/*
*	moves file-pointer
*/
long	lseek(int fildes, long offset, int whence)
{
	fseek((FILE *)fildes, offset, whence);
	return ftell((FILE*)fildes);
}
