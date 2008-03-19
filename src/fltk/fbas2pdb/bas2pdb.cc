/*
*	BAS (text) to PDB
*
*	Nicholas Christopoulos
*/

/*
*	Each DOC is a basic module
*	Each module has 
*		record 0 - type: info_t, the header
*		record 1 - type: sec_t,  the main code section
*		record n - type: sec_t,  code section
*
* 	Each section can be a function or a procedure.
* 	The maximum size of each section is 32KB (FIELD limit).
*/

#define	MAX_SEC		255
#define	MAX_SBTEXT	0x100000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#if defined(_UnixOS)
	#include <unistd.h>
#else
	#if defined(__GNUC__)
		#include <unistd.h>
	#else
		#include <stdlib.h>
		#include <io.h>
	#endif
#endif
#include <time.h>
#include <sys/stat.h>
//#include "pmem.h"

#if !defined(O_BINARY)
	#define O_BINARY	0
#endif

typedef	unsigned char	byte;
typedef int				int32;
typedef	int short		int16;
typedef	char *			char_p;

#define	DT_UTDIF	12345678

// DATE FIELDs
// expressed as the number of seconds since January 1, 1904.
// The database will not install if this value is zero. (PalmOS 1.0.6) 

int32	PalmNOW()
{
/*	time_t	now;

	time(&now);
	now += DT_UTDIF;
	return now;
*/
	return -1;
}

void	write_i16(int handle, int16 x)
{
	byte	y;

	y = x >> 8;
	write(handle, &y, 1); 
	y = x & 0xFF;
	write(handle, &y, 1);
}

void	write_i32(int handle, int32 x)
{
	write_i16(handle, x >> 16);
	write_i16(handle, x & 0xFFFF);
}

void	read_i16(int handle, int16& x)
{
	byte		a, b;

	read(handle, &a, 1); 
	read(handle, &b, 1);
	x = (a << 8) | b;
}

void	read_i32(int handle, int32& x)
{
	int16		a, b;

	read_i16(handle, a);
	read_i16(handle, b);
	x = (a << 16) | b;
}

void	change_file_ext(char *file, char *ext)
{
	char	*p = strrchr(file, '.');
	char	*e = ext;

	if	( p )
		p ++;
	if	( *e == '.' )
		e ++;

	if	( p )	{
		*p = '\0';
		strcat(file, e);
		}
	else	{
		strcat(file, ".");
		strcat(file, e);
		}
}

/*
*	PDB FILE HEADER
*/
class PDBHeader	{
public:

	char	name[32];
	int16	attr;			// 0x2 RO, 0x4 dirty, 0x8 backup, 0x10 ok to install newer,
							// 0x20 reset after install, 0x40 dont allow copy/beam
	int16	ver;			// app
	int32	dt_created;	
	int32	dt_modified;
	int32	dt_backup;
	int32	mod_num;		// always 0 ?
	int32	app_info_pos;	// 0 = no appinf
	int32	sort_info_pos;	// 0 = no ?
	byte	type[4];
	byte	creator[4];
	byte	uniq[4];		// 0 ??
	int32	next;			// 0
	int16	counter;		// record counter

	PDBHeader();
	void	Write(int handle);
	void	Read(int handle);
	};


PDBHeader::PDBHeader()
{
	memset(name, 0, 32);
	attr = 0;
	ver = 0;
	strncpy((char*)&dt_created, "\x06\xD1\x44\xAE", 4);
	strncpy((char*)&dt_modified, "\x06\xD1\x44\xAE", 4);
	dt_backup = 0;
	mod_num = 0;
	app_info_pos = sort_info_pos = 0;
	memcpy(type, "TEXT", 4);
	memcpy(creator, "SmBa", 4);
	memset(uniq, 0, 4);
	next = 0;
	counter = 0;
}

void	PDBHeader::Write(int handle)
{
	write(handle, &name, 32);
	write_i16(handle, attr);
	write_i16(handle, ver);
	write(handle, &dt_created, 4);	
	write(handle, &dt_modified, 4);
	write_i32(handle, dt_backup);
	write_i32(handle, mod_num);
	write_i32(handle, app_info_pos);
	write_i32(handle, sort_info_pos);
	write(handle, &type, 4);
	write(handle, &creator, 4);
	write(handle, &uniq, 4);
	write_i32(handle, next);
	write_i16(handle, counter);
}

void	PDBHeader::Read(int handle)
{
	read(handle, &name, 32);
	read_i16(handle, attr);
	read_i16(handle, ver);
	read(handle, &dt_created, 4);	
	read(handle, &dt_modified, 4);
	read_i32(handle, dt_backup);
	read_i32(handle, mod_num);
	read_i32(handle, app_info_pos);
	read_i32(handle, sort_info_pos);
	read(handle, &type, 4);
	read(handle, &creator, 4);
	read(handle, &uniq, 4);
	read_i32(handle, next);
	read_i16(handle, counter);
}

/*
*	PDB RECORD HEADER
*/
class PDBRecordHeader	{
public:
	int32	offset;
	int32	index;

	PDBRecordHeader()	{ offset = 0; index = (0x40 << 24) | 0x59A000; }
	void Write(int handle)	{ write_i32(handle, offset); write_i32(handle, index); }
	void Read(int handle)	{ read_i32(handle, offset); read_i32(handle, index); }
	};

/*
*	SmallBASIC - doc section
*/
class SMBasDocSec {
public:
	byte	sign;					// always = 'S'
	byte	unused;					//
	int16	version;				// 
	int16	flags;					// 1 for main section
	char	name[64];

	SMBasDocSec()		{ sign = 'S'; unused = 0; version = 1; flags = 1; strcpy(name, "Main"); }
	void	Write(int handle);	
	void	Read(int handle);	
	};

/*
*/
void	SMBasDocSec::Write(int handle)
{
	write(handle, &sign, 1);
	write(handle, &unused, 1);
	write_i16(handle, version);
	write_i16(handle, flags);
	write(handle, name, 64);
}

/*
*/
void	SMBasDocSec::Read(int handle)
{
	read(handle, &sign, 1);
	read(handle, &unused, 1);
	read_i16(handle, version);
	read_i16(handle, flags);
	read(handle, name, 64);
}

/*
*	create a SmallBASIC PDB file
*
*	returns 0 on success
*	-1 can't create file
*	-2 file i/o error
*	-3 text > 32KB
*/
struct txtsec_s	{
	char	name[33];
	char	*sp;
	char	*ep;
	char	*tp;
	int		len;
	};
typedef struct txtsec_s txtsec_t;

int		SaveSBPDB(const char *fname, const char *text)
{
	int			handle;
	PDBHeader	head;
	PDBRecordHeader	rec;
	int32		info;
	SMBasDocSec	sec;
	int16		filler=0;
	char		*final_text, *src, *dst, *ps, *lp;
	txtsec_t	tsec[MAX_SEC+1];
	char		lnx_head[512], lc;
	int			sec_count = 0;
	char		file_name[256];
	int			err_code = 0, i, count;

	// remove '\r'
	final_text = (char *) malloc(strlen(text)+1);
	src = (char *) text;
	dst = final_text;
	while ( *src )	{
		if	( *src != '\r' )	
			*dst ++ = *src;
		src ++;
		}
	*dst = '\0';

	// section list
	src = (char *) final_text;
	if	( strncmp(src, "#!", 2) == 0 )	{
		dst = lnx_head;
		while ( *src != '\0' && *src != '\n' )	
			*dst ++ = *src ++;
		*dst = '\0';
		if	( *src == '\n' )
			src ++;
		strcat(lnx_head, "\n");
		}
	else
		strcpy(lnx_head, "");

	ps = src;
	while ( *src )	{

		if	( strncmp(src, "#sec:", 5) == 0 )	{
			count = 0;
			dst = tsec[sec_count].name;
			tsec[sec_count].tp = src;
			src += 5;
			while ( *src != '\0' && *src != '\n' )	{
				if	( count < 32 )
					*dst ++ = *src ++;
				else
					src ++;
				count ++;
				}
			*dst = '\0';
			if	( *src == '\n' )
				src ++;

			tsec[sec_count].sp = src;
			tsec[sec_count].ep = src;
			sec_count ++;
			ps = src;
			}
		else	{
			// skip text-line
			while ( *src != '\0' && *src != '\n' )	src ++;
			if	( *src == '\n' )
				src ++;
			}
		}

	if	( sec_count == 0 )	{
		strcpy(tsec[0].name, "Main");
		tsec[0].tp = ps;
		tsec[0].sp = ps;
		tsec[0].ep = ps + strlen(ps);
		sec_count ++;
		}

//	printf("\n");
	for ( i = 0; i < sec_count; i ++ )	{
		tsec[i].len = 0;

		if	( (i+1) < sec_count )
			lp = tsec[i+1].tp;
		else
			lp = tsec[i].sp + strlen(tsec[i].sp);

		lc = *lp;
		*lp = '\0';

		if	( i == 0 && strlen(lnx_head) )	
			tsec[i].len += strlen(lnx_head);
		tsec[i].len += strlen(tsec[i].sp);
		tsec[i].len ++;

		if	( tsec[i].len > 32767 )	
			fprintf(stderr, "\nWarning: section '%s'; size %d > 32KB\n", tsec[i].name, tsec[i].len);

//		printf("=== %-32s ================================\n", tsec[i].name);
//		if	( i == 0 && strlen(lnx_head) )	
//			printf("%s", lnx_head);
//		printf("%s", tsec[i].sp);

		*lp = lc;
		}

	// base file name < 28 chars + .bas
	#if !defined(_UnixOS)
	src = strrchr((char *) fname, '\\');
	#else
	src = strrchr((char *) fname, '/');
	#endif
	if	( src )	
		src ++;
	else
		src = (char *) fname;

	memset(file_name, 0, 32);	// debug pdb

	strcpy(file_name, src);
	src = strrchr(file_name, '.');
	if	( src )	
		*src = '\0';
	file_name[27] = '\0';		// extention

	strcat(src, ".bas");
	strcpy(head.name, file_name);

	// create
	remove(fname);
	handle = open(fname, O_CREAT|O_BINARY|O_RDWR, S_IREAD|S_IWRITE);
	if	( handle == -1 )
		err_code = -1;
	else	{
		memcpy(head.type, "TEXT", 4);
		memcpy(head.creator, "SmBa", 4);
		head.counter = sec_count + 1;
		head.Write(handle);

		rec.offset = sizeof(PDBHeader) + sizeof(PDBRecordHeader) * head.counter;
		rec.index ++;
		rec.Write(handle);

		for ( i = 1; i <= sec_count; i ++ )	{
			if	( i == 1 )
				rec.offset += 4;
			else
				rec.offset += (tsec[i-2].len + sizeof(SMBasDocSec));
			rec.index ++;
			rec.Write(handle);
			}

		write_i16(handle, filler);

		info = 0x48030000;		// sign-ver-unused-category
		write_i32(handle, info);

		for ( i = 0; i < sec_count; i ++ )	{
			if	( (i+1) < sec_count )
				lp = tsec[i+1].tp;
			else
				lp = tsec[i].sp + strlen(tsec[i].sp);
			lc = *lp;
			*lp = '\0';

			sec.sign = 'S';
			sec.version = 1;
			sec.flags = (i==0) ? 1 : 0;
			strcpy(sec.name, tsec[i].name);
			sec.Write(handle);

			if	( i == 0 && strlen(lnx_head) )	
				write(handle, lnx_head, strlen(lnx_head));
			write(handle, tsec[i].sp, strlen(tsec[i].sp)+1);

			*lp = lc;
			}

		close(handle);
		}

	free(final_text);
	return err_code;
}


/*
*	Load a SmallBASIC PDB file
*
*	returns 0 on success
*	-1 can't open file
*	-2 file i/o error
*	-3 text > 32KB
*	-4 bad signature
*/
int		LoadSBPDB(const char *fname, char_p *rtext)
{
	int			handle;
	PDBHeader	head;
	PDBRecordHeader	rec_inf[MAX_SEC+1];
	int32		info;
	SMBasDocSec	sec;
	char		*src, *dst, *buf, *cvbuf, *text;
	int			err_code = 0, count, i, rcount;

	text = (char *) malloc(MAX_SBTEXT);	
	*text = '\0';
	*rtext = text;

	handle = open(fname, O_BINARY|O_RDWR, S_IREAD|S_IWRITE);
	if	( handle == -1 )
		err_code = -1;
	else	{
		head.Read(handle);
		rcount = head.counter;

		for ( i = 0; i < rcount; i ++ )	
			rec_inf[i].Read(handle);

		lseek(handle, rec_inf[0].offset, SEEK_SET);
		read_i32(handle, info);

		for ( i = 1; i < rcount; i ++ )	{
			lseek(handle, rec_inf[i].offset, SEEK_SET);

			if	( ((info & 0xFFFF0000) == 0x48030000) || ((info & 0xFFFF0000) == 0x48040000) )	{

				sec.Read(handle);

				buf   = (char *) malloc(0x10000);	// 64KB
				cvbuf = (char *) malloc(0x11000);	// 64KB
				count = read(handle, buf, 0x10000);
				buf[count] = '\0';

				#if	!defined(_UnixOS)
				// add '\r'
				dst = cvbuf;
				src = buf;
				while ( *src )	{
					if	( *src == '\n' )	{
						*dst ++ = '\r';
						*dst ++ = *src ++;
						}
					else
						*dst ++ = *src ++;
					}
				*dst = '\0';
				#else
				strcpy(cvbuf, buf);
				#endif

				// check unix header
				if	( i == 1 )	{
					if	( strncmp(cvbuf, "#!", 2) == 0 )	{
						src = cvbuf;
						dst = text;
						while ( *src != '\0' && *src != '\n' )	
							*dst ++ = *src ++;

						if	( *src )
							src ++;
						*dst ++ = '\n';
						*dst ++ = '\0';
						}
					else
						src = cvbuf;
					}
				else
					src = cvbuf;

				// add section name
				strcat(text, "#sec:");
				strcat(text, sec.name);
				strcat(text, "\n");

				strcat(text, src);
				free(cvbuf);
				free(buf);
				}
			else
				err_code = -4;	// bad signature
			}

		close(handle);
		}

	return err_code;
}

/*
*/
void	usage()
{
#if	defined(_PDB2BAS)
	printf("SmallBASIC utilities: PDB to BAS (text), version 0.6\n");
	printf("%% pdb2bas file\n");
#else
	printf("SmallBASIC utilities: BAS (text) to PDB, version 0.6\n");
	printf("%% bas2pdb file\n");
#endif
}

/*
*/
#if !defined(_GUI)
int	main(int argc, char *argv[])
{
	char	file[1024];
	char	*buf;
#if	defined(_PDB2BAS)
	char	*p;
#else
	int		len;
#endif
	int		errcode = 0, i;
	int		handle;
	struct stat st;

	if	( argc == 1 )	{
		usage();
		errcode = 1;
		}
	else	{
		for	( i = 1; i < argc; i ++ )	{
			strcpy(file, argv[i]);
#if	defined(_BAS2PDB)
			/*
			*/
			handle = open(file, O_BINARY|O_RDWR, S_IREAD|S_IWRITE);
			if	( handle != -1 )	{
				// load
				len = lseek(handle, 0, SEEK_END);
				printf("bas2pdb: %s, size %d -> ", file, len);
				lseek(handle, 0, SEEK_SET);

				buf = new char[len+1];
				read(handle, buf, len);
				buf[len] = '\0';
				close(handle);

				change_file_ext(file, ".pdb");
				printf("%s, size ", file);

				// save
				switch ( SaveSBPDB(file, buf) )	{
				case	-1:
				case	-2:
					fprintf(stderr, "cant create file\n");
					errcode = 3;
					break;
				case	-3:
					fprintf(stderr, "text > 32KB does not supported, yet\n");
					errcode = 4;
					break;
					}

				stat(file, &st);
				printf("%d\n", (int) st.st_size);

				//
				delete[] buf;
				}
			else	{
				fprintf(stderr, "cant open file\n");
				errcode = 2;
				}
#elif defined(_PDB2BAS)
			/*
			*/

			stat(file, &st);
			printf("pdb2bas: %s, size %d -> ", file, (int) st.st_size);

			switch ( LoadSBPDB(file, &buf) )	{
			case	-1:
			case	-2:
				fprintf(stderr, "cant create file\n");
				errcode = 3;
				break;
			case	-3:
				fprintf(stderr, "text > 1MB does not supported, yet\n");
				errcode = 4;
				break;
			case	-4:
				fprintf(stderr, "bad signature\n");
				errcode = 5;
				break;
				}

			if	( errcode == 0 )	{
				if	( (p = strstr(file, ".bas.pdb")) != NULL )	{
					*p = '\0';
					strcat(file, ".bas");
					}
				else
					change_file_ext(file, ".bas");

				handle = open(file, O_CREAT|O_BINARY|O_RDWR, S_IREAD|S_IWRITE);

				if	( handle != -1 )	{
					write(handle, buf, strlen(buf));
					close(handle);

					stat(file, &st);
					printf(" %s, size %d\n", file, (int) st.st_size);
					}
				else	{
					fprintf(stderr, "cant create file\n");
					errcode = 2;
					}

				delete[] buf;
				}
#endif
			}
		}
	return errcode;
}
#endif
