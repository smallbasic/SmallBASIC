/**
*	@file palmfs.h
*
*	PalmOS file system
*
*	@author Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

/**
*	@defgroup palmfs PalmOS file system utilities
*/

#if !defined(_sb_palmfs_h)
#define _sb_palmfs_h

#include "sys.h"

typedef char*		char_p;
typedef DmOpenRef	file_t;
#define	fatal(c,m)		ErrNonFatalDisplayIf((c), (m))

/**
*	@ingroup palmfs
*
*	returns true if the file exists
*
*	@param file the filename
*	@return non-zero if the file exists
*/
int		db_exist(const char *file) 					SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	changes the extention of the file. The 'text' it must be large enough or sure that has already 
*	an extention which has more or equal size to the new ext.
*
*	@param text the filename
*	@param ext the new extention
*/
void	chg_file_ext(char *text, const char *ext)	SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	opens a database. the file must exists
*
*	@param fileName the filename
*	@return the file handle or 0 or error
*/
file_t	db_open(const char *fileName) 				SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	opens a database. if the file does not exists then a new one is created
*
*	@param fileName the filename
*	@return the file handle or 0 or error
*/
file_t	db_open_alw(const char *fileName, dword creator, dword type) SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	closes a database
*
*	@param f the file handle
*/
void	db_close(file_t f) 							SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	writes a new record or changes an older one
*
*	@param f the file handle
*	@param record is the record index
*	@param new_rec is a pointer to data to be written
*	@param size is the size of the data
*	@return the record index
*/
word	db_write(file_t f, word record, void *new_rec, word size) SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	writes a new record or changes an older one. it writes in speficied section of the record.
*
*	@param f the file handle
*	@param record is the record index
*	@param offset is the offset inside of the existed record
*	@param ptr is a pointer to data to be written
*	@param size is the size of the data
*	@return the record index
*/
word	db_write_seg(file_t f, word record, word offset, void *ptr, word size) SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	returns the size of the record in bytes
*
*	@param f the file handle
*	@param record is the record index
*	@return the size of the record in bytes
*/
word	db_rec_size(file_t f, word record) SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	reads a record
*
*	@param f the file handle
*	@param record is the record index
*	@param buff is a pointer to data
*	@param size is the size of the data
*	@return the bytes that readed
*/
word	db_read(file_t f, word record, void *buff, word size) SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	reads a record
*
*	@param f the file handle
*	@param record is the record index
*	@param offset offset from where to start reading inside the record
*	@param buff is a pointer to data
*	@param size is the size of the data
*	@return the bytes that readed
*/
word	db_read_seg(file_t f, word record, word offset, void *buff, word size) SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	removes a database
*
*	@param file the filename
*	@return 0 on success (or if there is no such file); otherwise returns the error
*/
int		db_remove(const char *fileName) 						SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	returns the number of records in the file.
*
*	@param f the file handle
*	@return the number of the records
*/
word	db_rec_count(file_t f) SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	enables the backup-bit of a database
*
*	@param file the filename
*/
void	db_set_bckbit(const char *filename) SEC(PALMFS);

/**
*	@ingroup palmfs
*
*	true if the file is a script-file (launchable)
*
*	@param file the filename
*	@return non-zero if the file is script
*/
int		db_isscript(const char *file)		SEC(PALMFS);

#endif

