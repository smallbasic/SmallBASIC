/**
*	PalmOS file system
*
*	Nicholas Christopoulos
*/

#include "sys.h"
#include "palmfs.h"

#define	APPL	0x6170706C
#define	SmBa	0x536D4261

#define	idBASC	0x42415343
#define	idTEXT	0x54455854
#define	idDATA	0x44415441
#define	idData	0x44617461
#define idUFST  0x55465354

#define	idLNCH	0x6C6E6368

/*
*	returns true if the file exists
*/
int		db_exist(const char *file)
{
	return (DmFindDatabase(0, (char*) file) != 0);
}

/*
*	change file's extension
*/
void	chg_file_ext(char *text, const char *ext)
{
	char	*p;

	p = strrchr(text, '.');
	if	( p )	
		*p = '\0';
	strcat(text, ext);
}

/*
*	opens a palm-database file
*/
file_t	db_open(const char *fileName)
{
	LocalID		lid;
	file_t		ref;
	
	// open
	lid = DmFindDatabase(0, (char *) fileName);
	ref = DmOpenDatabase(0, lid, dmModeReadWrite);
	fatal(ref == 0, "db_open(): can't open file");
	return ref;
}

/*
*	opens a palm-database file; if the file does not exists its creates one
*/
file_t	db_open_alw(const char *fileName, dword creator, dword type)
{
	LocalID		lid;
	file_t		ref;
	int			d;
	
	// open
	lid = DmFindDatabase(0, (char *) fileName);
	if	( !lid )	{ // create
		d = DmCreateDatabase(0, fileName, creator, type, false);
		fatal(d != 0, "db_open_alw(): can't create file");
		lid = DmFindDatabase(0, (char *) fileName);
		}

	ref = DmOpenDatabase(0, lid, dmModeReadWrite);
	fatal(ref == 0, "db_open_alw(): can't open file");
	return ref;
}

/*
*	close a palm database file
*/
void	db_close(file_t f)
{
	DmCloseDatabase(f);
}

/*
*	writes a new record at the end of the file
*/
word	db_write(file_t f, word record, void *new_rec, word size)
{
	word		recIndex;
	mem_t		rec_h;
	void		*pre_rec;
	int		src_sz;
	int		d;

	if	( record >= DmNumRecords(f) )		{	// new record
		recIndex = dmMaxRecordIndex;
		rec_h = DmNewRecord(f, &recIndex, size);
		fatal(rec_h == 0, "db_write(): Can't create record");
		}
	else	{	// Update record
		recIndex = record;
		rec_h = DmGetRecord(f, recIndex);
		fatal(rec_h == 0, "db_write(): Can't open record");
		src_sz = MemHandleSize(rec_h);
		if	( src_sz != size )
			MemHandleResize(rec_h, size);
		}

	pre_rec = mem_lock(rec_h);
	fatal( pre_rec == 0,        "db_write(): Can't lock record!");
	d = DmWrite(pre_rec, 0, new_rec, size);
	fatal(d != 0, "db_write(): DmWrite return error.");
	mem_unlock(rec_h);
	d = DmReleaseRecord(f, recIndex, 1);
	fatal(d != 0, "db_write(): Can't release record.");
	return recIndex;
}

/*
*	writes a new record at the end of the file
*/
word	db_write_seg(file_t f, word record, word offset, void *ptr, word size)
{
	word		recIndex;
	mem_t		rec_h;
	void		*pre_rec;
	int		src_sz;
	int		d;

	if	( record >= DmNumRecords(f) )		{	// new record
		recIndex = dmMaxRecordIndex;
		rec_h = DmNewRecord(f, &recIndex, size);
		fatal(rec_h == 0, "db_write_seg(): Can't create record");
		}
	else	{	// Update record
		recIndex = record;
		rec_h = DmGetRecord(f, recIndex);
		fatal(rec_h == 0, "db_write_seg(): Can't open record");
		src_sz = MemHandleSize(rec_h);
		if	( src_sz < (size+offset) )
			MemHandleResize(rec_h, (size+offset));
		}

	pre_rec = mem_lock(rec_h);
	fatal(pre_rec == NULL, "db_write_seg(): Can't lock record!");
	d = DmWrite(pre_rec, offset, ptr, size);
	fatal( d != 0, "db_write_seg(): DmWrite return error.");
	mem_unlock(rec_h);
	d = DmReleaseRecord(f, recIndex, 1);
	fatal(d != 0, "db_write_seg(): Can't release record.");
	return recIndex;
}

/*
*	returns the size of the record
*/
word	db_rec_size(file_t f, word record)
{
	VoidPtr		rec_p = NULL;
	VoidHand	rec_h = NULL;
	word		len;

	rec_h = DmGetRecord(f, record);
	fatal(rec_h == 0, "db_rec_size(): can't get record");
	rec_p = mem_lock(rec_h);
	fatal(rec_p == NULL, "db_rec_size(): can't lock record");

	len = MemHandleSize(rec_h);
	mem_unlock(rec_h);
	DmReleaseRecord(f, record, 0);
	return len;
}

/*
*	reads from a database file
*/
word	db_read(file_t f, word record, void *buff, word size)
{
	VoidPtr		rec_p = NULL;
	VoidHand	rec_h = NULL;
	word		len;

	rec_h = DmGetRecord(f, record);
	fatal(rec_h == 0, "db_read(): can't get record");
	rec_p = mem_lock(rec_h);
	fatal(rec_p == NULL, "db_read(): can't lock record");

	len = MemHandleSize(rec_h);
	if	( len > size )
		len = size;

	MemMove(buff, rec_p, len);

	mem_unlock(rec_h);
	DmReleaseRecord(f, record, 0);
	return len;
}

/*
*	reads a segment of a database record
*/
word	db_read_seg(file_t f, word record, word offset, void *buff, word size)
{
	VoidPtr		rec_p = NULL;
	VoidHand	rec_h = NULL;
	word		len;

	rec_h = DmGetRecord(f, record);
	fatal(rec_h == 0, "db_read_seg(): can't get record");
	rec_p = mem_lock(rec_h);
	fatal(rec_p == NULL, "db_read_seg(): can't lock record");

	len = MemHandleSize(rec_h);
	if	( len > size )
		len = size;

	MemMove(buff, rec_p+offset, len);

	mem_unlock(rec_h);
	DmReleaseRecord(f, record, 0);
	return len;
}

/*
*	removes a database file
*/
int		db_remove(const char *fileName)
{
	LocalID		lid;

	lid = DmFindDatabase(0, (char *) fileName);
	if	( !lid )
		return 0;
	return DmDeleteDatabase(0, lid);
}

/*
*	returns the number of the records
*/
word	db_rec_count(file_t f)
{
	return DmNumRecords(f);
}

/*
*	setup the backup-bit of a database
*/
void	db_set_bckbit(const char *filename)
{
	LocalID		lid;
	word	attr;

	lid = DmFindDatabase(0, (char *) filename);
	if	( lid )	{
		DmDatabaseInfo(0, lid, NULL, &attr, NULL, NULL,  NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		attr |= dmHdrAttrBackup;
		DmSetDatabaseInfo(0, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		}
}

/*
*	true if the file is a script-file (launchable)
*/
int		db_isscript(const char *file)
{
	LocalID		lid;
	Err			err;
	char		name[33];
	UInt16		attr;

	lid = DmFindDatabase(0, (char *) file);
	if	( lid ) 	{
		err = DmDatabaseInfo(0, lid, name, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		if	( !err )	{
			if	( attr & dmHdrAttrLaunchableData )
				return 1;
			}
		}
	return 0;
}



