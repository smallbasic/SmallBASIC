/*
*	SmallBASIC IDE for PalmOS
*
*	2000-05-26, Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
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

#include "sys.h"
#include "smbas.h"
#include "sbpad.h"
#include "panic.h"
#include "str.h"
#include "ui.h"
#include "device.h"
#include "fs_pdoc.h"
#include "fs_memo.h"
#include "palmfs.h"
#include "sbapp.h"

#define	APPL	0x6170706C
#define	SmBa	0x536D4261

#define	idBASC	0x42415343
#define	idTEXT	0x54455854
#define	idDATA	0x44415441
#define	idData	0x44617461
#define idUFST  0x55465354

#define	idLNCH	0x6C6E6368

#define	MAXTEXTSIZE		0x7fff		// 32K
#define	MAXSTR			256

#define sysVersion35    sysMakeROMVersion(3,5,0,sysROMStageRelease,0)
#define sysVersion40    sysMakeROMVersion(4,0,0,sysROMStageRelease,0)

DWord	OldScreenDepth;

/*
*	sbpad prefs
*/
typedef struct {
	char	last_filename[64];
	word	last_cat;
	word	last_doc_sec;
	word	last_doc_pos;
	word	last_doc_font;
	word	last_doc_form;
	word	font;
	word	charset;
	word	flags;
						// bit 0: true = delete log on startup
						// bit 1: true = keep BC files
						// bit 2: true = use safedraw
						// bit 3: true = use VMT by default
	word	cclabs1;
	word	ccpass2;

	dword	binver;		// last version

	char	reserver[64-24];
	} sbpref_t;

/*
*	.bas header 
*/
typedef struct {
		byte	sign;				// always 'H'
		byte	version;			// 
		byte	unused;
		byte	category;			//
		/* --- version 4+ --- */
		dword	flags;				//	0x1 = compressed - not supported yet
									//	0x2 = PalmOS script
		char	reserved[64-4];
		} info_t;

/*
*	.bas section 
*
*	Any .bas document is separated to sections
*	A section can hold 32KB (max-size in the text-field control)
*/
typedef struct {
	byte	sign;					// always = 'S'
	byte	unused;					//
	word	version;				// 
	word	flags;					// 1 for main section
	char	name[64];
	} sec_t;

/*
*	link appinfo
*
*	That info are needed to create 'script' files
*/
typedef struct {
	dword	crid;		// always 'lnch'
	word	pqav;		// always 3?
	word	unknown;	// always 0?
	word	verlen;		// always 2
	char	verstr[4];	// string - always "1.0\0"
	word	namelen;	// x=strlen(filename)+1; x=x/2+(((x/2)%2)?1:0) 
	char	name[34];	// the filename
	word	iconlen;	// len of icon 32x32 data
	byte	icon[144];	// the icon - palm image type - 32*32/8 + 16 = 144 bytes
	word	smilen;		// len of small icon 15x9 data
	byte	smi[34];	// the small icon - palm image type - 15*9/8 + 16 = ~33 + 1(pad) = 34 bytes
	} link_t;

//	---------------------------------------------------------------
//	For any other who wants to trying that:
//
//	icon data
//	[BitmapType] - size 16 bytes, see SDK
//	[BitmapBits] - raw data
//
//	How-to create icon data
//	create the Tbmp bin files with pilrc
//	the final bins is what are you want.
//	There is a difference in pilrc's headers for the smallicon, see the bas_makescript()
//
//	see also: build_sbgo_icons.c 					// converts pilrc resources to C-style array
//
//	see also: void bas_makescript(const char *file)	// takes a simple .bas file and convert it to a 'shortcut'
//	

#include "sbgo_icons.c"

//
static sbpref_t		sbpref;

/* current doc (for editor) */
static file_t		cur_file;
static info_t		cur_file_header;
static char			cur_file_name[64];			// current filename (source file)
static char			cur_uf_name[64];			// current filename (file viewer)
static int			cur_section;
static byte			modified = 0;
static char			editor_status_text[64];

/* current doc section list */
static char_p		*sec_list_str;
static sec_t		*sec_list;
static int			sec_count;

/* doc list variable */
static char_p *doc_list;
static int	doc_count;

/* user-file list  */
static char_p *uf_list = NULL;
static int	uf_count = 0;

static char	last_search_str[MAXSTR];		// last search string
static int  return_to_form;		  			// 
static int  auto_run = 0;		  			// PalmOS script, run it immediately
static int  dont_save_prefs = 0;  			// PalmOS script, run it immediately

static char	temp[MAXSTR];

/// globals of editor's form
static int	last_doc_font;
static int	last_doc_pagesize;
static word	last_doc_pos;
static word	last_err_doc_line;
static word last_doc_sec;
static word last_uf;
static char	smtitle[64];

/// globals for categories
static char_p	*cat_list;
static int		cat_count;
static dword	cat_unfiled_id;
static dword	cur_cat;

// fonts
typedef struct {
	int			id;		// id
	int			rid;	// resource id
	int			lpp;
	MemHandle	h;		// mem handle
	} font_node_t;

static font_node_t font_table[] = {
{ HK40CustomFont  , HK40FONT  , 20, NULL },
{ HK40CustomFont+1, HK40FONT+1, 15, NULL },
{ HK40CustomFont+2, HK40FONT+2, 13, NULL },
{ HK40CustomFont+3, HK40FONT+3, 13, NULL },
{                0,          0,  0, NULL }
};

#if defined(PDOC_HELP)
#define	SBDOC_GENERIC	"SB-REF Basic"
#define	SBDOC_SYSTEM	"SB-REF System"
#define	SBDOC_GRAPHICS	"SB-REF Graphics and Sound"
#define	SBDOC_MISC		"SB-REF Miscellaneous"
#define	SBDOC_FILESYS	"SB-REF File System"
#define	SBDOC_MATHSTR	"SB-REF Mathematics and Strings"
#define	SBDOC_CONSOLE	"SB-REF Console"
#endif

// declarations
void	save_prefs() SEC(IDE);
void	free_catlist() SEC(IDE);
void	load_catlist() SEC(IDE);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*	just a beep :)
*/
void	warn_beep() SEC(IDE);
void	warn_beep()
{
	SndCommandType			beepPB;

	beepPB.cmd = sndCmdFreqDurationAmp;
	beepPB.param1 = 440;
	beepPB.param2 = 100;	// in ms
	beepPB.param3 = (sndMaxAmp * 75) / 100;
	SndDoCmd(NULL, &beepPB, 0);	// true=nowait
}

/*
*	input-box dialog
*/
int	BasicInputBox(char *title, char *dest, int size, int frmid)
{
	FormPtr		frm, prv_frm;
	FieldPtr  	text_box;
	FormActiveStateType	stateP;
	VoidHand	text_h;
	char		*text_p;
	int			r, l;

	*dest = '\0';
	FrmSaveActiveState(&stateP);
	prv_frm = FrmGetActiveForm();

	if	( frmid == 0 )
		frm = FrmInitForm(InputBoxForm);
	else
		frm = FrmInitForm(KeybForm);

	text_box = (FieldPtr) FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fldInputBox));
	FrmSetActiveForm(frm);
	FrmSetTitle(frm, title);
	FrmSetFocus(frm, FrmGetObjectIndex(frm, fldInputBox));
	FldGrabFocus(text_box);
	FldSetInsPtPosition(text_box, 0);
	InsPtEnable(true);

	// set value	
	if	( (r = FrmDoDialog(frm)) == btnOK )	{

		text_h = (VoidHand) FldGetTextHandle(text_box);
		if	( text_h )	{
			text_p = (char *) MemHandleLock(text_h);
												   
			if	( text_p )	{
				l = strlen(text_p);

				if	( l )	{
					if	( l >= size )	{
						StrNCopy(dest, text_p, size-1);
						dest[size] = '\0';
						}
					else
						StrCopy(dest, text_p);
					}

				MemHandleUnlock(text_h);
				}
			}
		}

	if ( prv_frm)	{
		FrmSetActiveForm(prv_frm);
		FrmRestoreActiveState(&stateP);
		}

	FrmDeleteForm(frm);
	return ( r == btnOK );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// BEAM
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*	beam: send a block of data
*/
Err	beam_send_data(const void *data, dword *size, void *userdata)
{
	Err		err;

	*size = ExgSend((ExgSocketPtr) userdata, (void *) data, *size, &err);
	return err;
}

/*
*	beam: send a database file
*/
void beam_send_db(const char *file)
{
	char	*name;
	ExgSocketType	exsock;
	Err		err;
	LocalID	lid;

	lid = DmFindDatabase(0, (char *) file);
	if	( !lid )
		return;

	name = tmp_alloc(128);
	strcpy(name, file);
    DmDatabaseInfo(0, lid, name, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	MemSet(&exsock, sizeof(exsock), 0);
	exsock.description = name;
	exsock.name = name;
	exsock.target = SmBa;
	err = ExgPut(&exsock);
	if	( !err )	{
		err = ExgDBWrite(beam_send_data, &exsock, NULL, lid, 0);
		err = ExgDisconnect(&exsock, err);
		}
	tmp_free(name);
}

/*
*	receive a data block
*/
Err		beam_read_thunk(void *datap, unsigned long *sizep, void *rock)
{
	Err 			err = 0;
	ExgSocketType	*sock = rock;
	dword			count;

	count = *sizep;
	count = ExgReceive(sock, datap, count, &err);
	*sizep = count;
	return err;
}

/*
*/
Boolean	beam_delete_thunk(const char *name, Word version, word card, LocalID lid, void *rock)
{
	Err		err;
	unsigned short attr;

	err = DmDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	if ( !err )	{
		DmDeleteDatabase(0, lid);
		return 1;
		}
	return 0;
}

/*
*	recieve a database
*/
Err		beam_get_db(ExgSocketPtr sock, byte gavail)
{
	Err		err = 0;
	LocalID lid;
	unsigned short card = 0;
	byte	wantreset = 0;

	err = ExgDBRead(beam_read_thunk, beam_delete_thunk, sock, &lid, card, &wantreset, 1);
	err = ExgDisconnect(sock, err);

	sock->goToCreator = 0;

	if ( !err )	{
		if ( wantreset ) {
		    int res = FrmAlert(AlertWantReset);
		    if (res == 1)
		      SysReset();
			}
		}

	if ( gavail )	{
//		refresh doc list
		if	( sbpref.last_doc_form == MainForm )	{
			EventType	ev;

			ev.eType = evtRefreshDOCs;
			EvtAddEventToQueue(&ev);
			}
		} 
	return err;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*	copies a .BAS file
*/
int		bas_file_copy(const char *oldName, const char *newName) SEC(IDE);
int		bas_file_copy(const char *oldName, const char *newName)
{
	file_t		fin, fout;
	word		recs, i, len;
	char		*ptr;

	fin = db_open(oldName);
	if	( db_exist(newName) ) db_remove(newName);
	ptr = strrchr(newName, '.');
	if	( ptr )	{
		if	( StrCaselessCompare(ptr, ".sbx") == 0 )
			fout = db_open_alw(newName, SmBa /* idBASC */, idDATA);
		else
			fout = db_open_alw(newName, SmBa /* idBASC */, idTEXT);
		}
	else
		fout = db_open_alw(newName, SmBa /* idBASC */, idTEXT);

	// copy
	recs = db_rec_count(fin);
	for ( i = 0; i < recs; i ++ )	{
		len = db_rec_size(fin, i);
		ptr = tmp_alloc(len);
		db_read(fin, i, ptr, len);

		db_write(fout, 0xFFFF, ptr, len);
		tmp_free(ptr);
		}

	// clean up
	db_close(fin);
	db_close(fout);
	db_set_bckbit(newName);
	return 1;
}

/*
*/
void	bas_removebin(const char *basfile, int always) SEC(IDE);
void	bas_removebin(const char *basfile, int always)
{
	char	bin[64], name[64];
	int		del = 0;
	UInt16	attr;

	if	( !always )	{
		del = ((sbpref.flags & 2) == 0); 
		if	( del )	{
			LocalID	lid;

			lid = DmFindDatabase(0, (char *) basfile);
			if	( lid )	{
				strcpy(name, basfile);
				if	( DmDatabaseInfo(0, lid, name, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == 0 )	{
					if	( attr & dmHdrAttrLaunchableData )	
						del = 0;
					}
				}
			}
		}
	else
		del = 1;

	if	( del )	{
		strcpy(bin, basfile);
		chg_file_ext(bin, ".sbx");
		db_remove(bin);
		}
}

/*
*	renames a BAS file
*/
void	bas_file_rename(const char *oldfn, const char *newfn) SEC(IDE);
void	bas_file_rename(const char *oldfn, const char *newfn)
{
	if	( bas_file_copy(oldfn, newfn) )	{
		db_remove(oldfn);
		bas_removebin(oldfn, 1);
		}
}

/*
*	remove all .bin (BC files) files
*/
void	vm_delete_all(void)	SEC(IDE);
void	vm_delete_all()
{
	int			count, dlcount, i;
	dword		db_type, db_creator;
	LocalID		LID;
	LocalID		*table;
	char		*p;

	count = DmNumDatabases(0);
	table = (LocalID *) tmp_alloc(count * sizeof(LocalID));
	dlcount = 0;
		
	// collect LIDs
	for ( i = 0; i < count; i++ )	{
		LID = DmGetDatabase(0, i);
		
		if	( LID )	{
			temp[0] = '\0';
			if	( DmDatabaseInfo(0, LID,
				temp,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				&db_type, &db_creator ) == 0 )	{

				if ( (db_creator == SmBa) && db_type == idDATA ) {
					p = strrchr(temp, '.');
					if	( p )	{
						if	( StrCaselessCompare(p, ".sbx") == 0 || StrCaselessCompare(p, ".bin") == 0 )	{
							table[dlcount] = LID;
							dlcount ++;
							}
						}
					} // dbType
				} // DmDatabaseInfo
			} // LID
		} // for

	// delete the files
	for ( i = 0; i < dlcount; i++ )	
		DmDeleteDatabase(0, table[i]);
	tmp_free(table);

	// old VMTs
	db_remove("BCS-STK.VMT");
	db_remove("BCS-LBL.VMT");
	db_remove("AUDIO.VMT");
	db_remove("SYSENV.VMT");

	// new VMTs
	db_remove("SBI-LBL.dbt");	db_remove("SBI-LBL.dbi");
	db_remove("SBI-STK.dbt");	db_remove("SBI-STK.dbi");
	db_remove("SBI-ENV.dbt");	db_remove("SBI-ENV.dbi");
	db_remove("SBI-AUDIO.dbt");	db_remove("SBI-AUDIO.dbi");
	db_remove("SBI-Memo.dbt");	db_remove("SBI-Memo.dbi");
}

/*
*/
void	bas_rm_all_bin_nonscripts(void)	SEC(IDE);
void	bas_rm_all_bin_nonscripts()
{
	int			count, dlcount, i;
	dword		db_type, db_creator;
	LocalID		lidbin, lidbas;
	LocalID		*table;
	char		*p;
	UInt16		attr;

	count = DmNumDatabases(0);
	table = (LocalID *) tmp_alloc(count * sizeof(LocalID));
	dlcount = 0;
		
	// collect LIDs
	for ( i = 0; i < count; i++ )	{
		lidbin = DmGetDatabase(0, i);
		
		if	( lidbin )	{
			temp[0] = '\0';
			if	( DmDatabaseInfo(0, lidbin,
				temp,
				&attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				&db_type, &db_creator ) == 0 )	{

				if ( (db_creator == SmBa) && db_type == idDATA ) {
					p = strrchr(temp, '.');
					if	( p )	{
						if	( StrCaselessCompare(p, ".sbx") == 0 )	{
							strcpy(p, ".bas");
							if	( (lidbas = DmFindDatabase(0, temp)) != 0 )	{
								attr = 0;
								DmDatabaseInfo(0, lidbas, temp, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
								}

							if	( (attr & dmHdrAttrLaunchableData) == 0 )	{ // !script
								table[dlcount] = lidbin;
								dlcount ++;
								}
							}
						}
					} // dbType
				} // DmDatabaseInfo
			} // LID
		} // for

	// delete the files
	for ( i = 0; i < dlcount; i++ )	
		DmDeleteDatabase(0, table[i]);
	tmp_free(table);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void	mkSmallTitle(char *src, char *dst, int size, int font) SEC(IDE);
void	mkSmallTitle(char *src, char *dst, int size, int font)
{
	int		l, x, f;

#if defined(SONY_CLIE)
	f = HRFntGetFont(sony_refHR);
	HRFntSetFont(sony_refHR, font);
	x = FntWordWrap(src, size);
	HRFntSetFont(sony_refHR, f);
#else
	f = FntGetFont();
	FntSetFont(font);
	x = FntWordWrap(src, size);
	FntSetFont(f);
#endif

	l = StrLen(src);
	if	( l > x )	{
		MemMove(dst, src, x);
		dst[x] = '\x85';
		dst[x+1] = '\0';
		}
	else
		StrCopy(dst, src);
}

/*
*	build section list
*/
static void build_seclist() SEC(IDE);
static void build_seclist()
{
	byte	   	*ptr;
	int			recs, i, len;

	ErrNonFatalDisplayIf(!cur_file, "BuildSecList(): database is not opened");

	recs = db_rec_count(cur_file);

	sec_list = tmp_alloc(sizeof(sec_t) * recs);
	sec_list_str = tmp_alloc(sizeof(char_p) * recs);

	for ( i = 0; i < recs; i ++ )	{
		len = db_rec_size(cur_file, i);
		ptr = tmp_alloc(len);
		db_read(cur_file, i, ptr, len);

		if	( *ptr == 'S' )	{
			MemMove(&sec_list[sec_count], ptr, sizeof(sec_t));
			sec_list_str[sec_count] = sec_list[sec_count].name;
			sec_count ++;
			}

		tmp_free(ptr);
		}
}

/*
*	free section list
*/
void free_seclist() SEC(IDE);
void free_seclist()
{
	if	( sec_list )	{
		tmp_free(sec_list_str);
		tmp_free(sec_list);
		sec_list_str = NULL;
		sec_list = NULL;
		sec_count = 0;
		}
}

/*
*	destroy the list of user-filenames
*/
//static void free_uflist() SEC(IDE);
void	free_uflist()
{
	int		i;

	if	( uf_list )	{
		for ( i = 0; i < uf_count; i ++ )
			MemPtrFree(uf_list[i]);
		tmp_free(uf_list);
		uf_list = NULL;
		uf_count = 0;
		}
}

/*
*	build a list with the user's data files
*/
static void build_uflist() SEC(IDE);
static void build_uflist()
{
	int			dbCount, i;
	dword		dbType, dbCreator;
	LocalID		LID;

	free_uflist();

	dbCount = DmNumDatabases(0);
	uf_list = tmp_alloc(dbCount * sizeof(char_p));
	uf_count = 0;
	for ( i = 0; i < dbCount; i++ )	{
		LID = DmGetDatabase(0, i);
		
		if	( LID )	{
			temp[0] = '\0';
			if	( DmDatabaseInfo(0, LID,
				temp,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				&dbType, &dbCreator ) == 0 )	{

				if ( dbCreator == SmBa && dbType == idUFST ) {
					uf_list[uf_count] = (char *) MemPtrNew(StrLen(temp)+1);
					ErrNonFatalDisplayIf(!uf_list[uf_count], "Out of memory");
					StrCopy(uf_list[uf_count], temp);
					uf_count ++;
					} // dbType
				} // DmDatabaseInfo
			} // LID
		} // for

	if	( uf_count == 0 )	{
		uf_list[uf_count] = (char *) MemPtrNew(33);
		ErrNonFatalDisplayIf(!uf_list[uf_count], "Out of memory");
		StrCopy(uf_list[uf_count], "NewFile");
		uf_count ++;
		}
}

/*
*	load a 'section'
*/
mem_t	bas_loadsec(word secIndex) SEC(IDE);
mem_t	bas_loadsec(word secIndex)
{
	word		len, rec_n;
	//sec_t		*sec_p;
	mem_t		h;
	char		*ptr;

	fatal(!cur_file, "bas_loadsec(): database is not opened");
	fatal(secIndex > sec_count, "bas_loadsec(): page out of range");

	rec_n = secIndex + 1;
	//sec_p = &sec_list[secIndex]; dead code ???
	len = db_rec_size(cur_file, rec_n);

	h = mem_alloc(len - sizeof(sec_t));
	ptr = mem_lock(h);
	db_read_seg(cur_file, rec_n, sizeof(sec_t), ptr, len - sizeof(sec_t));
	mem_unlock(h);
	return h;
}

/*
*	load a user data-file to the editor
*/
mem_t	uf_load(const char *name) SEC(IDE);
mem_t	uf_load(const char *name)
{
	mem_t		h = NULL;
	char		*ptr;
	dword		len;
	FileHand	f;
	Err			ferr;

	f = FileOpen(0, (char *)name, idUFST, SmBa, fileModeReadOnly, &ferr);
	if	( ferr == 0 )	{
		FileTell(f, &len, &ferr);
		if	( len > 32767 )	{
			len = 32767;
			warning("File size > 32KB");
			}
		h = mem_alloc(len+1);
		ptr = mem_lock(h);
		FileRead(f, ptr, len, 1, &ferr);
		ptr[len] = '\0';
		FileClose(f);
		mem_unlock(h);
		}
	return h;
}

/*
*	create a user data-file (UserFileViewer)
*/
int		uf_create(const char *name) SEC(IDE);
int		uf_create(const char *name)
{
	FileHand	f;
	Err			ferr;

	f = FileOpen(0, (char *)name, idUFST, SmBa, fileModeAppend, &ferr);
	if	( ferr == 0 )	
		FileClose(f);
	return (ferr != 0);
}

/*
*	save a user data-file (UserFileViewer)
*/
int		uf_save(const char *name) SEC(IDE);
int		uf_save(const char *name)
{
	char		*text;
	FileHand	f;
	Err			ferr;

	FileDelete(0, (char *)name);
	f = FileOpen(0, (char *)name, idUFST, SmBa, fileModeReadWrite | fileModeAnyTypeCreator, &ferr);
	text = FldGetTextPtr(fld_ptr(fldTEXT));
	FileWrite(f, text, StrLen(text), 1, &ferr);
	FileClose(f);
	db_set_bckbit(name);
	return (ferr != 0);
}

/*
*	open a .bas file
*/
void	bas_open(const char *fileName) SEC(IDE);
void	bas_open(const char *fileName)
{
	if	( cur_file )
		db_close(cur_file);

	modified = 0;

	cur_file = db_open(fileName);

	// header
	db_read(cur_file, 0, &cur_file_header, sizeof(cur_file_header));
	if	( cur_file_header.version <= 3 )	{
		// update file-header
		cur_file_header.version = 4;
		cur_file_header.flags   = 0;
		db_write(cur_file, 0, &cur_file_header, sizeof(cur_file_header));
		}

	// build the section list
	free_seclist();
	build_seclist();
}

/*
*	close .bas
*/
void	bas_close() SEC(IDE);
void	bas_close()
{
	if	( cur_file )	{
	
		free_seclist();
		db_close(cur_file);

		if	( modified )	{
			db_set_bckbit(cur_file_name);
			modified = 0;
			}

		cur_file = 0;
		}
}

/*
*	close a user-data-file
*/
void	uf_close() SEC(IDE);
void	uf_close()
{
	free_uflist();
}

/*
*	Creates an empty doc
*/
int		bas_create(const char *fileName) SEC(IDE);
int		bas_create(const char *fileName)
{
	file_t		f;
	sec_t		sec;
	char		*defText;
	ULong		dts;
	DateTimeType cur_date;
	word		rec_n;

	if	( db_exist(fileName) )	{
		if	( FrmCustomAlert(ConfAlertID, "Overwrite '", (char *)fileName, "' ?") == 0 )	{
			db_remove(fileName);
			bas_removebin(fileName, 1);
			}
		else
			return 0;
		}

	// default text 
	defText = tmp_alloc(64);
	dts = TimGetSeconds();
	TimSecondsToDateTime(dts, &cur_date);
	StrPrintF(defText, "' %s\n' %02d/%02d/%04d\n", fileName, (int) cur_date.day, (int) cur_date.month, cur_date.year);

	// create & open
	f = db_open_alw(fileName, SmBa /* idBASC */, idTEXT);

	// write header
	memset(&cur_file_header, 0, sizeof(info_t));
	cur_file_header.sign = 'H';
	cur_file_header.version = 4;
	cur_file_header.category = (cur_cat == cat_unfiled_id) ? 0 : cur_cat;
	db_write(f, 0xFFFF, &cur_file_header, sizeof(cur_file_header));

	// write section
	sec.sign = 'S';
	sec.version = 4;
	sec.flags = 1;
	StrCopy(sec.name, "Main");
	rec_n = db_write_seg(f, 0xFFFF, 0, &sec, sizeof(sec_t));
	db_write_seg(f, rec_n, sizeof(sec_t), defText, StrLen(defText)+1);

	// close
	db_close(f);
	tmp_free(defText);
	return 1;
}

/*
*	returns the document category
*/
dword	bas_getcat(LocalID lid)
{
	file_t		f;
	info_t		hdr;
	
	// open
	f = DmOpenDatabase(0, lid, dmModeReadWrite);
	fatal(f == 0, "bas_getcat: can't open file");
	if	( f )	{
		db_read(f, 0, &hdr, sizeof(info_t));
		db_close(f);
		return hdr.category;
		}
	return 0;
}


/*
*	sets the category of the file
*/
void 	bas_update_cat(const char *file, dword category) SEC(IDE);
void	bas_update_cat(const char *file, dword category)
{
	file_t		f;
	info_t		hdr;

	f = db_open(file);
	if	( f )	{
		db_read(f, 0, &hdr, sizeof(info_t));
		hdr.category = category;
		db_write(f, 0, &hdr, sizeof(info_t));
		db_close(f);
		}
}

/*
*	create PalmOS script
*/
void	bas_makescript(const char *file)	SEC(IDE);
void	bas_makescript(const char *file)
{
	LocalID		lid, ap_lid, si_lid;
	Err			err;
	char		name[33];
	UInt16		attr, ver;
	UInt32		type, crid;
	UInt32		romVersion;

    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if	( romVersion < sysMakeROMVersion(3,2,0,sysROMStageRelease,0) )	{
		warning("This option requires PalmOS 3.2 or newer");
		return;
		}

	lid = DmFindDatabase(0, (char *) file);
	if	( !lid ) 	{
		warning("bas_makescript(): failed to find the file!");
		return;
		}
	err = DmDatabaseInfo(0, lid, name, &attr, &ver, NULL, NULL, NULL, NULL, &ap_lid, &si_lid, &type, &crid);
	if	( err )
		warning("bas_makescript(): DmDatabaseInfo failed (err=%d)", err);
	else	{
		if	( (attr & dmHdrAttrLaunchableData) == 0 )	{
			// enable it
			link_t		lnk, *lnkp;
			MemHandle	h;
			file_t		f;
			long		ofs = 0;

			// change attributes
			crid = SmBa;
			attr |= dmHdrAttrLaunchableData;

			// create app-info
			memset(&lnk, 0, sizeof(link_t));
			lnk.crid = idLNCH;
			lnk.pqav = 3;
			lnk.verlen = 2;
			strcpy(lnk.verstr, "1.0");
			lnk.namelen = strlen(file)+1;
			if ( (lnk.namelen % 2) != 0 )
				lnk.namelen ++;
			lnk.namelen = lnk.namelen >> 1;
			
			memset(lnk.name, 0, 34);
			strncpy(lnk.name, file, 33);

			// big icon
			lnk.iconlen = 144 >> 1;
			memcpy(lnk.icon, sbgo_icon, 144);	// the first 16 bytes is BitmapType. 
												// You can use it to make iconfamily (chain of images)

			// small icon
			sbgosmall_icon[1] = 0x0F;			// @#$@#$@#, width of the icon = 15 pixels, the pilrc wrote 0x10
			lnk.smilen = 34 >> 1;
			memcpy(lnk.smi, sbgosmall_icon, 34);

			//
			f = db_open(file);
			if	( f )	{
				h = DmNewHandle(f, sizeof(link_t));
				lnkp = (link_t *) MemHandleLock(h);

				#define	AW(p,s) DmWrite(lnkp, ofs, (p), (s));		ofs += s;
				ofs = 0;

				AW(&lnk.crid, 4);
				AW(&lnk.pqav, 2);
				AW(&lnk.unknown, 2);
				AW(&lnk.verlen, 2);
				AW(&lnk.verstr, lnk.verlen << 1);
				AW(&lnk.namelen, 2);
				AW(&lnk.name, lnk.namelen << 1);
				AW(&lnk.iconlen, 2);
				AW(&lnk.icon, 144);
				AW(&lnk.smilen, 2);
				AW(&lnk.smi, 34);		

				#undef AW
				MemHandleUnlock(h);

				ap_lid = MemHandleToLocalID(h);
				err = DmSetDatabaseInfo(0, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, 
					&ap_lid, NULL /*sortInfoIDP*/, &type, &crid);

				if	( err )
					warning("bas_makescript(): DmSetDatabaseInfo failed (err=%d)", err);
				db_close(f);
				}
			else
				warning("bas_makescript(): File not found (err=%d)", err);
				
			}
		}
}

/*
*	convert to regular file (instead of script)
*/
void	bas_makeregular(const char *file)	SEC(IDE);
void	bas_makeregular(const char *file)
{
	LocalID		lid, ap_lid, si_lid;
	Err			err;
	char		name[33];
	UInt16		attr, ver;
	UInt32		type, crid;

	lid = DmFindDatabase(0, (char *) file);
	if	( !lid ) 	{
		warning("bas_makeregular(): failed to find the file!");
		return;
		}
	err = DmDatabaseInfo(0, lid, name, &attr, &ver, NULL, NULL, NULL, NULL, &ap_lid, &si_lid, &type, &crid);
	if	( err )
		warning("bas_makeregular(): DmDatabaseInfo failed (err=%d)", err);
	else	{
		if	( attr & dmHdrAttrLaunchableData )	{
			// kill it
			crid = SmBa;
			attr &= ~dmHdrAttrLaunchableData;
			ap_lid = 0;
			err = DmSetDatabaseInfo(0, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, 
				&ap_lid, NULL /*sortInfoIDP*/, &type, &crid);

			if	( err )
				warning("bas_makeregular(): DmSetDatabaseInfo failed (err=%d)", err);
			}
		}
}

/*
*	retrieve the source-file header
*/
void 	bas_getheader(const char *file, info_t *inf) SEC(IDE);
void	bas_getheader(const char *file, info_t *inf)
{
	file_t		f;

	f = db_open(file);
	if	( f )	{
		db_read(f, 0, inf, sizeof(info_t));
		db_close(f);
		}
}

/*
*	update source-file header
*/
void 	bas_updateheader(const char *file, info_t *inf) SEC(IDE);
void	bas_updateheader(const char *file, info_t *inf)
{
	file_t		f;

	f = db_open(file);
	if	( f )	{
		inf->version = 4;
		db_write(f, 0, inf, sizeof(info_t));
		db_close(f);
		}

	if	( inf->flags & 2 )	
		bas_makescript(file);
	else
		bas_makeregular(file);

}

/*
*	Set all DOCS with that category to unfilled
*/
void	bas_setunfiled(dword cat)	SEC(IDE);
void	bas_setunfiled(dword cat)
{
	int			count, i;
	dword		db_type, db_creator;
	LocalID		LID;
	file_t		f;
	info_t		hdr;

	count = DmNumDatabases(0);
	for ( i = 0; i < count; i++ )	{
		LID = DmGetDatabase(0, i);
		
		if	( LID )	{
			temp[0] = '\0';
			if	( DmDatabaseInfo(0, LID,
				temp,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				&db_type, &db_creator ) == 0 )	{

				if ( (db_creator == idBASC || db_creator == SmBa) && db_type == idTEXT ) {
					if ( bas_getcat(LID) == cat )	{
						f = DmOpenDatabase(0, LID, dmModeReadWrite);
						if	( f )	{
							db_read(f, 0, &hdr, sizeof(info_t));
							hdr.category = cat_unfiled_id;
							db_write(f, 0, &hdr, sizeof(info_t));
							db_close(f);
							}
						}
					} // dbType
				} // DmDatabaseInfo
			} // LID
		} // for
}

/*
*/
void	bas_savesec(word secIndex) SEC(IDE);
void	bas_savesec(word secIndex)
{
	char		*text;

	fatal(!cur_file, "bas_savesec(): database is not opened");
	fatal(secIndex > sec_count, "bas_savesec(): page out of range");

	text = FldGetTextPtr(fld_ptr(fldTEXT));
	db_write_seg(cur_file, secIndex + 1, sizeof(sec_t), text, StrLen(text)+1);
	modified = 1;	// needed to set the backup field
}

/*
*/
void	bas_savesec_name(word secIndex, char *name) SEC(IDE);
void	bas_savesec_name(word secIndex, char *name)
{
	db_write_seg(cur_file, secIndex + 1, sizeof(sec_t)-64, name, StrLen(name)+1);
	modified = 1;	// needed to set the backup field
}

/*
*/
void 	bas_createsec(char *name) SEC(IDE);
void	bas_createsec(char *name)
{
	word		rec_n;
	sec_t		sec;
	char		defText[64];

	fatal(!cur_file, "bas_createsection(): database is not opened");

	mkSmallTitle(name, defText, 55, stdFont);
	StrCopy(sec.name, defText);

	StrCopy(defText, "' ");
	StrCat(defText, name);

	// write section
	sec.sign = 'S';
	sec.flags = 0;

	rec_n = db_write_seg(cur_file, 0xFFFF, 0, &sec, sizeof(sec_t));
	db_write_seg(cur_file, rec_n, sizeof(sec_t), defText, StrLen(defText)+1);
}

/*
*/
void	bas_deletesec(word idx) SEC(IDE);
void	bas_deletesec(word idx)
{
	fatal(!cur_file, "bas_deletesec(): database is not opened");
	if	( idx > 0 )	
		DmRemoveRecord(cur_file, idx+1);
	else
		FrmCustomAlert(InfoAlertID, "You cannot delete the main section", "", "");
}

void	bas_savecheck() SEC(IDE);
void	bas_savecheck()
{
	if	( cur_file )	{
		if	( FldDirty(fld_ptr(fldTEXT)) )	{
			if	( FrmCustomAlert(ConfAlertID, "The text has been modified. Save now ?", "", "") == 0 )	{
				FldCompactText(fld_ptr(fldTEXT));
				bas_savesec(cur_section);
				bas_removebin(cur_file_name, 1);
				}
			}
		}
}

void	uf_savecheck() SEC(IDE);
void	uf_savecheck()
{
	if	( FldDirty(fld_ptr(fldTEXT)) )	{
		if	( FrmCustomAlert(ConfAlertID, "The text has been modified. Save now ?", "", "") == 0 )	{
			FldCompactText(fld_ptr(fldTEXT));
			uf_save(cur_uf_name);
			}
		}
}

/*
*	display the section
*/
int		disp_sec(word secIndex) SEC(IDE);
int		disp_sec(word secIndex)
{
	mem_t	new_h;

	fatal(!cur_file, "disp_sec(): database is not opened");

	if ( secIndex < sec_count )	{
		bas_savecheck();

		new_h = bas_loadsec(secIndex);
		cur_section = secIndex;
		setFieldHandle(fldTEXT, new_h, 1);

		return 0;
		}

	return 1;
}

/*
*	display the user's data file
*/
int		uf_display(const char *name) SEC(IDE);
int		uf_display(const char *name)
{
	mem_t	new_h;
	char	*ptr;

	if	( db_exist(name) )	{
		// bas_savecheck();
		new_h = uf_load(name);
//		StrCopy(cur_uf_name, name);
		}
	else	{
		new_h = mem_alloc(128);
		ptr = mem_lock(new_h);
		StrPrintF(ptr, "File '%s' not found!", name);
		mem_unlock(new_h);
		}
	setFieldHandle(fldTEXT, new_h, 1);

	return 0;
}

void	text_setpos(word pos) SEC(IDE);
void	text_setpos(word pos)
{
	FieldPtr	fp;

	fp = fld_ptr(fldTEXT);
	FldSetScrollPosition(fp, pos);
	FldSetInsPtPosition(fp, pos);
//	FldSetInsertionPoint(fp, pos);
	FldDrawField(fp);
	FldGrabFocus(fp);
}

word	text_getpos() SEC(IDE);
word	text_getpos()
{
	return FldGetInsPtPosition(fld_ptr(fldTEXT));
}

/*
*/
int 	bas_copy(const char *oldName, const char *newName) SEC(IDE);
int		bas_copy(const char *oldName, const char *newName)
{
	if	( !db_exist(oldName) )
		return 0;
    if	( !StrCompare(oldName,newName))	{
		FrmCustomAlert(InfoAlertID, "New name '", (char *) newName, "' same as old name.  No action taken.");
		return 0;
		}
	if	( db_exist(newName) )	{
		if	( FrmCustomAlert(ConfAlertID, "Overwrite '", (char *) newName, "' ?") != 0 )
			return 0;
		}

	if	( bas_file_copy(oldName, newName) )	{
		db_set_bckbit(newName);
		return 1;
		}
	return 0;
}

void	bas_rename(const char *oldName, const char *newName) SEC(IDE);
void	bas_rename(const char *oldName, const char *newName)
{
	if	( bas_copy(oldName, newName) )
		DmDeleteDatabase(0, DmFindDatabase(0, (char *) oldName));
}

/*
*  	executes a program
*	returns 0 = fail
*/
int		sys_exec(const char *prog) SEC(IDE);
int		sys_exec(const char *prog)
{
	LocalID	lid;
	dword	progid;
	word	card;
	DmSearchStateType	state;

	progid  = ((dword) prog[0] << 24) + ((dword) prog[1] << 16) + ((dword) prog[2] << 8) + (dword) prog[3];
	if	( progid == idLNCH )	{
		EventType	ev;

		MemSet(&ev, sizeof(EventType), 0);
		ev.eType = keyDownEvent;
		ev.data.keyDown.modifiers = commandKeyMask;
		ev.data.keyDown.chr = vchrLaunch;
		EvtAddEventToQueue(&ev);
		return 1;
		}
	else	{
		if	( DmGetNextDatabaseByTypeCreator(true, &state, APPL, progid, true, &card, &lid) == 0 )
			return (SysUIAppSwitch(card, lid, sysAppLaunchCmdNormalLaunch, NULL) == 0);
		}
	return 0;
}

/*
*	destroy the list of 'basic-source' filenames
*/
//static void free_baslist() SEC(IDE);
void	free_baslist()
{
	int		i;

	if	( doc_list )	{
		for ( i = 0; i < doc_count; i ++ )
			MemPtrFree(doc_list[i]);
		tmp_free(doc_list);
		doc_list = NULL;
		doc_count = 0;
		}
}

/*
*	build the list of 'basic-source' filenames
*/
//static void build_baslist() SEC(IDE);
void	build_baslist()
{
	int			dbCount, i;
	dword		dbType, dbCat, dbCreator;
	LocalID		LID;

	free_baslist();

	dbCount = DmNumDatabases(0);
	doc_list = tmp_alloc(dbCount * sizeof(char_p));
	doc_count = 0;
	for ( i = 0; i < dbCount; i++ )	{
		LID = DmGetDatabase(0, i);
		
		if	( LID )	{
			temp[0] = '\0';
			if	( DmDatabaseInfo(0, LID,
				temp,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				&dbType, &dbCreator ) == 0 )	{

				if ( (dbCreator == idBASC || dbCreator == SmBa) && dbType == idTEXT ) {
					dbCat = bas_getcat(LID);
					if	( cur_cat == 0 || dbCat == cur_cat || (dbCat == 0 && cur_cat == cat_unfiled_id) )	{
						doc_list[doc_count] = (char *) MemPtrNew(StrLen(temp)+1);
						ErrNonFatalDisplayIf(!doc_list[doc_count], "out of memory");
						StrCopy(doc_list[doc_count], temp);
						doc_count ++;
						}
					} // dbType
				} // DmDatabaseInfo
			} // LID
		} // for
}

/*
*	GOTO LINE
*/
void	ide_goto(int line) SEC(IDE);
void	ide_goto(int line)
{
	char	*p, *text;
	int		c = 1, pos = 0, found = 0;

	if	( line > 0 )	{
		if	( line == 1 )	{
			text_setpos(0);
			return;
			}

		text = FldGetTextPtr(fld_ptr(fldTEXT));
		p = text;

		while ( *p )	{
			if	( *p == '\n' )	{
				c ++;
				if	( c == line )	{
					text_setpos(pos+1);
					found ++;
					break;
					}
				}

			p ++;
			pos ++;
			}

		if	( !found )
			text_setpos(pos+1);
		}
}

void	ide_search_for(const char *src) SEC(IDE);
void	ide_search_for(const char *src)
{
	char	*p, *text;
	int		pos = 0;

	StrCopy(last_search_str, src);
	text = FldGetTextPtr(fld_ptr(fldTEXT));
	p = text;

	while ( *p )	{
		if	( StrNCaselessCompare(p, last_search_str, StrLen(last_search_str)) == 0 )	{
			text_setpos(pos+1);
			break;
			}

		p ++;
		pos ++;
		}
}

void	ide_search_again() SEC(IDE);
void	ide_search_again()
{
	char	*p, *text;
	int		pos;

	text = FldGetTextPtr(fld_ptr(fldTEXT));
	p = text;
	p += ((pos = text_getpos()) + 1);

	while ( *p )	{
		if	( StrNCaselessCompare(p, last_search_str, StrLen(last_search_str)) == 0 )	{
			text_setpos(pos+1);
			break;
			}

		p ++;
		pos ++;
		}
}

void	bas_setfileext(char *text)	SEC(IDE);
void	bas_setfileext(char *text)
{
	char	*p;

	p = stristr(text, ".bas");
	if	( p == NULL )	
		StrCat(text, ".bas");
}

//// EDIT /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void	ide_savestatus(const char *file, word lastSec, word lastPos, int font)	SEC(IDE);
void	ide_savestatus(const char *file, word lastSec, word lastPos, int font)
{
	file_t	f;
	int		i, recs, found = 0;
	byte	*buff;

	f = db_open_alw("SmBa.IDE.DAT", SmBa, idDATA);
	buff = tmp_alloc(128);

	recs = DmNumRecords(f);
	for ( i = 0; i < recs; i ++ )	{
		db_read(f, i, buff, 128);
		if	( StrCompare(buff, file) == 0 )	{
			buff[66] = (lastSec >> 8);		buff[67] = lastSec & 0xFF;
			buff[68] = (lastPos >> 8);		buff[69] = lastPos & 0xFF;
			buff[70] = (font >> 8);			buff[71] = font    & 0xFF;
			db_write(f, i, buff, 128);
			found = 1;
			break;
			}
		}

	if	( !found )	{
		StrCopy(buff, file);
		buff[64] = 0;		buff[65] = 1;
		buff[66] = (lastSec >> 8);		buff[67] = lastSec & 0xFF;
		buff[68] = (lastPos >> 8);		buff[69] = lastPos & 0xFF;
		buff[70] = (font >> 8);			buff[71] = font    & 0xFF;
		db_write(f, 0xFFFF, buff, 128);
		}

	tmp_free(buff);
	db_close(f);
}

int		ide_loadstatus(const char *file, word *lastSec, word *lastPos, int *font) SEC(IDE);
int		ide_loadstatus(const char *file, word *lastSec, word *lastPos, int *font)
{
	file_t	f;
	int		i, recs, found = 0;
	byte	*buff;

	f = db_open_alw("SmBa.IDE.DAT", SmBa, idDATA);
	buff = tmp_alloc(128);

	recs = DmNumRecords(f);
	for ( i = 0; i < recs; i ++ )	{
		db_read(f, i, buff, 128);
		if	( StrCompare(buff, file) == 0 )	{
			*lastSec = (buff[66] << 8) + buff[67];
			*lastPos = (buff[68] << 8) + buff[69];
			*font    = (buff[70] << 8) + buff[71];
			found = 1;
			break;
			}
		}

	tmp_free(buff);
	db_close(f);
	return found;
}

int		ide_getcurline(void)	SEC(IDE);		// MAIN
int		ide_getcurline(void)
{
	FieldPtr	fp;
	char	*p, ch;
	int		line;

	fp = fld_ptr(fldTEXT);

	p = FldGetTextPtr(fp);
	ch = p[last_doc_pos];
	p[last_doc_pos] = '\0';
	line = FldCalcFieldHeight(p, 512*8);
	p[last_doc_pos] = ch;
	return line;
}

/*
*	editor: display the current line number
*/
void	ide_dispcurline()	SEC(IDE);
void	ide_dispcurline()
{
	int			line;
	ControlPtr	ctrl;
    FormPtr 	frm;

	frm = FrmGetActiveForm();
	ctrl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, labLN));

	if	( ctrl )	{
		last_doc_pos = text_getpos();
		line = ide_getcurline();

		memset(editor_status_text, 0, 64);
		StrIToA(editor_status_text, line);
		CtlSetLabel(ctrl, editor_status_text);
		}
}

Boolean		editor_eventloopSEG(EventPtr e)	SEC(IDE);
Boolean		editor_eventloopSEG(EventPtr e)
{
    Boolean handled = false;
    FormPtr frm;
	ListPtr	list;
	EventType	ev;
    
    switch (e->eType) {
    case frmOpenEvent:
		frm = FrmGetActiveForm();
		mkSmallTitle(cur_file_name, smtitle, 60, boldFont);
		FrmSetTitle(frm, smtitle);
		FrmDrawForm(frm);

		sbpref.last_doc_form = ViewForm;

		bas_open(cur_file_name);

		if	( !ide_loadstatus(cur_file_name, &last_doc_sec, &last_doc_pos, &last_doc_font) )	{
			last_doc_sec = 0;
			last_doc_pos = 0;
			last_doc_font = sbpref.font;
			}

		FldSetFont(GetFieldPtr(fldTEXT), last_doc_font);
		if	( last_doc_font == HK40CustomFont )	
			last_doc_pagesize = 20;
		else if	( last_doc_font == SM10CustomFont )	
			last_doc_pagesize = 13;
		else if	( last_doc_font == DOSCustomFont )	
			last_doc_pagesize = 13;
		else if	( last_doc_font == DOS4CustomFont )	
			last_doc_pagesize = 13;
		else if	( last_doc_font == 0 || last_doc_font == 1 )
			last_doc_pagesize = 10;
		else if ( last_doc_font == 2 )
			last_doc_pagesize = 8;
		else
			last_doc_pagesize = 7;

		list = GetListPtr(lstSection);
		LstSetListChoices(list, sec_list_str, sec_count);
		UpdatePopup(trgSection, lstSection, last_doc_sec, sec_list_str);

		disp_sec(last_doc_sec);
		text_setpos(last_doc_pos);

		return_to_form = 0;

		FrmSetFocus(frm, FrmGetObjectIndex(frm, fldTEXT));

		if	( last_err_doc_line )	{
			ide_goto(last_err_doc_line);
			last_err_doc_line = 0;
			}
		ide_dispcurline();
		handled = true;
		break;

	case popSelectEvent:
    	switch (e->data.popSelect.controlID) {
	    case trgSection:
			if	( last_doc_sec != e->data.popSelect.selection )	{
				UpdatePopup(trgSection, lstSection, e->data.popSelect.selection, sec_list_str);
				last_doc_sec = e->data.popSelect.selection;
				disp_sec(last_doc_sec);
				text_setpos(0);
				}
			handled = 1;
			break;
			}
		break;

    case menuEvent:
		switch(e->data.menu.itemID) {
		case mnuNew:
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);

			if	( BasicInputBox("New FileName:", temp, 255, 0) )	{
				bas_savecheck();

				bas_setfileext(temp);
				if	( bas_create(temp) )	{
					bas_open(temp);
					
					last_doc_sec = last_doc_pos = 0;
					StrCopy(cur_file_name, temp);

					frm = FrmGetActiveForm();
					mkSmallTitle(cur_file_name, smtitle, 60, boldFont);
					FrmSetTitle(frm, smtitle);
					FrmDrawForm(frm);

					list = GetListPtr(lstSection);
					LstSetListChoices(list, sec_list_str, sec_count);
					UpdatePopup(trgSection, lstSection, last_doc_sec, sec_list_str);

					disp_sec(last_doc_sec);
					text_setpos(last_doc_pos);
					}
				}
			break;
		case mnuBeam:
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
			bas_savecheck();
			beam_send_db(cur_file_name);
			break;
		case mnuEdit:
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
			bas_savecheck();

			bas_close();
			FrmGotoForm(MainForm);
			break;
		case mnuRun:
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
			bas_savecheck();

			return_to_form = ViewForm;
			bas_close();
			FrmGotoForm(SBOutForm);
			break;
		case mnuSave: // save always
			if ( cur_file )	{
				last_doc_pos = text_getpos();
				ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
				bas_savesec(cur_section);
				FldSetDirty(fld_ptr(fldTEXT), 0);
				}
			break;
		case mnuSaveAs:
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
			if ( cur_file )	{
				if	( BasicInputBox("New FileName:", temp, 255, 0) )	{
					frm = FrmGetActiveForm();
					bas_close();

					bas_setfileext(temp);
					bas_copy(cur_file_name, temp);

					StrCopy(cur_file_name, temp);
					bas_open(cur_file_name);

					list = GetListPtr(lstSection);
					LstSetListChoices(list, sec_list_str, sec_count);
					UpdatePopup(trgSection, lstSection, last_doc_sec, sec_list_str);

					mkSmallTitle(cur_file_name, smtitle, 60, boldFont);
					FrmSetTitle(frm, smtitle);
					FrmDrawForm(frm);

					bas_savesec(last_doc_sec);
					text_setpos(last_doc_pos);
					}
				}
			break;
		case mnuClose:
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
			bas_savecheck();

			bas_close();
			FrmGotoForm(MainForm);
			break;
		case mnuGoto:
			if	( BasicInputBox("Go to line...", temp, 64, 0) )	{
				ide_goto(xstrtol(temp));
				last_doc_pos = text_getpos();
				}
	      	break;
		case mnuSearch:
			if	( BasicInputBox("Search for:", temp, 255, 0) )	{
				ide_search_for(temp);
				last_doc_pos = text_getpos();
				}
	      	break;
		case mnuSearchAgain:
			ide_search_again();
			last_doc_pos = text_getpos();
			break;
		case mnuAbout:
			DoAboutForm();
	      	break;
		case mnuHelpC1:
		case mnuHelpC2:
		case mnuHelpC3:
		case mnuHelpC4:
		case mnuHelpC5:
		case mnuHelpC6:
		case mnuHelpC7:
		case mnuHelpC8:
		case mnuHelpC9:
			{
				int dif = e->data.menu.itemID - mnuHelpC1;
				int	tid = HelpC1TID + dif;
				int	sid = HelpC1ID  + dif;

				GPlmAlert(tid, sid);
//				PDOCAlert(SBDOC_GENERIC);
			}
			break;
		case mnuUndo:
			FldUndo(fld_ptr(fldTEXT));
			break;
		case mnuCut:
			FldCut(fld_ptr(fldTEXT));
			break;
		case mnuCopy:
			FldCopy(fld_ptr(fldTEXT));
			break;
		case mnuSelAll:
			FldSetSelection(fld_ptr(fldTEXT), 0, 0xFFFF);
			break;
	   	case mnuPaste:			  
			FldPaste(fld_ptr(fldTEXT));
			break;
	   	case mnuKeyboard:
			SysKeyboardDialog(kbdDefault);
			break;
		 case mnuGraffiti:
			SysGraffitiReferenceDialog(referenceDefault);
			break;
	   	case mnuFont0:
			FldSetFont(fld_ptr(fldTEXT), (last_doc_font = stdFont));
			last_doc_pagesize = 10;
			save_prefs();
			break;
	   	case mnuFont1:
			FldSetFont(fld_ptr(fldTEXT), (last_doc_font = boldFont));
			last_doc_pagesize = 10;
			save_prefs();
			break;
	   	case mnuFont2:
			FldSetFont(fld_ptr(fldTEXT), (last_doc_font = largeFont));
			last_doc_pagesize = 8;
			save_prefs();
			break;
	   	case mnuFont7:
			FldSetFont(fld_ptr(fldTEXT), (last_doc_font = largeBoldFont));
			last_doc_pagesize = 7;
			save_prefs();
			break;
	   	case mnuFontHK:
	   	case mnuFontSM10:
	   	case mnuFontDOS:
	   	case mnuFontDOS4:
			if	( os_charset == enc_utf8 )	{
				int		idx;

				idx = e->data.menu.itemID - mnuFontHK;
				FldSetFont(fld_ptr(fldTEXT), (last_doc_font = font_table[idx].id));
				last_doc_pagesize = font_table[idx].lpp;
				}
			else
				FrmAlert(AlertCFontMB);
			save_prefs();
			break;
		case	mnuNewSec:
			if	( BasicInputBox("Section Name:", temp, 255, 0) )	{
				temp[32] = '\0';
				bas_savecheck();

				bas_createsec(temp);

				free_seclist();
				build_seclist();
				last_doc_sec = sec_count - 1;

				list = GetListPtr(lstSection);
				LstSetListChoices(list, sec_list_str, sec_count);
				UpdatePopup(trgSection, lstSection, last_doc_sec, sec_list_str);

				disp_sec(last_doc_sec);
				text_setpos(last_doc_pos = 0);
				}
			break;
		case	mnuChangeSec:
			if	( BasicInputBox("New name:", temp, 255, 0) )	{
				temp[32] = '\0';
				bas_savesec_name(cur_section, temp);

				free_seclist();
				build_seclist();

				list = GetListPtr(lstSection);
				LstSetListChoices(list, sec_list_str, sec_count);
				UpdatePopup(trgSection, lstSection, last_doc_sec, sec_list_str);
				}
			break;
		case	mnuDelSec:
			if	( FrmCustomAlert(ConfAlertID, "Do you want to delete the current section ?", "", "") == 0 )		{
				disp_sec(0);
				text_setpos((last_doc_pos = 0));

				bas_deletesec(last_doc_sec);

				free_seclist();
				build_seclist();

				last_doc_sec = 0;
				list = GetListPtr(lstSection);
				LstSetListChoices(list, sec_list_str, sec_count);
				UpdatePopup(trgSection, lstSection, last_doc_sec, sec_list_str);
				}
			break;
		  	};
    	handled = true;
		break;

	case keyDownEvent:
		switch ( e->data.keyDown.chr )	{
		case	pageUpChr: /* Prev */
			if ( FldScrollable(fld_ptr(fldTEXT), winUp) ) 
				FldScrollField(fld_ptr(fldTEXT), last_doc_pagesize, winUp);
			handled = true;
			break;
		case pageDownChr: /* next */
			if	( FldScrollable(fld_ptr(fldTEXT), winDown) )	
				FldScrollField(fld_ptr(fldTEXT), last_doc_pagesize, winDown);
			handled = true;
			break;
			};

		ev.eType = evtRefreshDOCs;
		EvtAddEventToQueue(&ev);
		break;

//	case nilEvent:
	case evtRefreshDOCs:
	case fldChangedEvent:
	case penUpEvent:
	case fldEnterEvent:
		ide_dispcurline();
		break;
    case ctlSelectEvent:
		switch(e->data.ctlSelect.controlID) {
		case	labLN:
			if	( BasicInputBox("Go to line...", temp, 64, 0) )	{
				ide_goto(xstrtol(temp));
				last_doc_pos = text_getpos();
				}
			break;
		case	btnSFT:
			FldInsert(fld_ptr(fldTEXT), "\t", 1);
	    	handled = true;
			break;
		case	btnCLOSE:
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
			bas_savecheck();

			bas_close();
			FrmGotoForm(MainForm);
	    	handled = true;
			break;
		case	btnUP:
			if ( FldScrollable(fld_ptr(fldTEXT), winUp) ) 
				FldScrollField(fld_ptr(fldTEXT), 1, winUp);
			ide_dispcurline();
	    	handled = true;
			break;
		case	btnDOWN:
			if	( FldScrollable(fld_ptr(fldTEXT), winDown) )	
				FldScrollField(fld_ptr(fldTEXT), 1, winDown);
			ide_dispcurline();
	    	handled = true;
			break;
		case	btnPGUP:
			if ( FldScrollable(fld_ptr(fldTEXT), winUp) ) 
				FldScrollField(fld_ptr(fldTEXT), last_doc_pagesize, winUp);

			text_setpos(FldGetScrollPosition(fld_ptr(fldTEXT)));
			ide_dispcurline();
	    	handled = true;
			break;
		case	btnPGDN:
			if	( FldScrollable(fld_ptr(fldTEXT), winDown) )	
				FldScrollField(fld_ptr(fldTEXT), last_doc_pagesize, winDown);

			text_setpos(FldGetScrollPosition(fld_ptr(fldTEXT)));
			ide_dispcurline();
	    	handled = true;
			break;
		case	btnSAVE:
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
			if	( FldDirty(fld_ptr(fldTEXT)) )	
				bas_savesec(cur_section);

			FldSetDirty(fld_ptr(fldTEXT), 0);
	    	handled = true;
			break;
		case btnQRUN:
			// SAVE
			last_doc_pos = text_getpos();
			ide_savestatus(cur_file_name, last_doc_sec, last_doc_pos, last_doc_font);
			if	( FldDirty(fld_ptr(fldTEXT)) )	{	// if modified save it
				bas_savesec(cur_section);
				FldSetDirty(fld_ptr(fldTEXT), 0);
				}

			// AND RUN
			return_to_form = ViewForm;
			bas_close();
			FrmGotoForm(SBOutForm);
			break;
			}
		break;		   

    default:
        break;
    	}

    return handled;
}

Boolean		editor_eventloop(EventPtr e)
{
	return editor_eventloopSEG(e);
}

//// FILEVIEW /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void	fv_savestatus(const char *file, word lastPos, int font)	SEC(IDE);
void	fv_savestatus(const char *file, word lastPos, int font)
{
	file_t	f;
	int		recs;
	byte	*buff;

	f = db_open_alw("SmBa.FSV.DAT", SmBa, idDATA);
	buff = tmp_alloc(128);

	recs = DmNumRecords(f);
	StrCopy(buff, file);
	buff[68] = (lastPos >> 8);		buff[69] = lastPos & 0xFF;
	buff[70] = (font >> 8);			buff[71] = font    & 0xFF;
	if	( recs > 0 )	
		db_write(f, 0, buff, 128);
	else
		db_write(f, 0xFFFF, buff, 128);

	tmp_free(buff);
	db_close(f);
}

int		fv_loadstatus(char *file, word *lastPos, int *font) SEC(IDE);
int		fv_loadstatus(char *file, word *lastPos, int *font)
{
	file_t	f;
	int		recs, found = 0;
	byte	*buff;

	f = db_open_alw("SmBa.FSV.DAT", SmBa, idDATA);
	buff = tmp_alloc(128);

	recs = DmNumRecords(f);
	if	( recs > 0 )	{
		db_read(f, 0, buff, 128);
		StrCopy(file, buff);
		*lastPos = (buff[68] << 8) + buff[69];
		*font    = (buff[70] << 8) + buff[71];
		found = 1;
		}
	else	{
		file[0] = '\0';
		*lastPos = 0;
		*font = 0;
		}

	tmp_free(buff);
	db_close(f);
	return found;
}

Boolean		fsv_eventloopSEG(EventPtr e)	SEC(IDE);
Boolean		fsv_eventloopSEG(EventPtr e)
{
    Boolean handled = false;
    FormPtr frm;
	ListPtr	list;
	int		i;
    
    switch (e->eType) {
    case frmOpenEvent:
		frm = FrmGetActiveForm();
		FrmDrawForm(frm);

		build_uflist();

		if	( !fv_loadstatus(cur_uf_name, &last_doc_pos, &last_doc_font) )	{
			last_doc_pos = 0;
			last_doc_font = sbpref.last_doc_font;
			last_uf = 0;
			}
		else	{
			last_uf = 0;
			for ( i = 0; i < uf_count; i ++ )	{
				if	( StrCompare(cur_uf_name, uf_list[i]) == 0 )	{
					last_uf = i;
					break;
					}
				}
			}

		FldSetFont(GetFieldPtr(fldTEXT), last_doc_font);
		if	( last_doc_font == HK40CustomFont )	
			last_doc_pagesize = 20;
		else if	( last_doc_font == SM10CustomFont )	
			last_doc_pagesize = 13;
		else if	( last_doc_font == DOSCustomFont )	
			last_doc_pagesize = 13;
		else if	( last_doc_font == DOS4CustomFont )	
			last_doc_pagesize = 13;
		else if	( last_doc_font == 0 || last_doc_font == 1 )
			last_doc_pagesize = 10;
		else if ( last_doc_font == 2 )
			last_doc_pagesize = 8;
		else
			last_doc_pagesize = 7;

		list = GetListPtr(lstSection);
		LstSetListChoices(list, uf_list, uf_count);
		UpdatePopup(trgSection, lstSection, last_uf, uf_list);

		StrCopy(cur_uf_name, uf_list[last_uf]);
		uf_display(cur_uf_name);
		text_setpos(last_doc_pos);

		return_to_form = 0;

		FrmSetFocus(frm, FrmGetObjectIndex(frm, fldTEXT));

		handled = true;
		break;
	case popSelectEvent:
    	switch (e->data.popSelect.controlID) {
	    case trgSection:
			if	( last_uf != e->data.popSelect.selection )	{
				UpdatePopup(trgSection, lstSection, e->data.popSelect.selection, uf_list);
				last_uf = e->data.popSelect.selection;
				StrCopy(cur_uf_name, uf_list[last_uf]);
				uf_display(cur_uf_name);
				text_setpos(0);
				}
			handled = 1;
			break;
			}
		break;
    case menuEvent:
		switch(e->data.menu.itemID) {
		case mnuNew:
			last_doc_pos = text_getpos();
			fv_savestatus(cur_uf_name, last_doc_pos, last_doc_font);

			if	( BasicInputBox("New FileName:", temp, 255, 0) )	{
				uf_savecheck();

				if	( uf_create(temp) )	{
					last_uf = 0;
					for ( i = 0; i < uf_count; i ++ )	{
						if	( StrCompare(cur_uf_name, uf_list[i]) == 0 )	{
							last_uf = i;
							break;
							}
						}

					list = GetListPtr(lstSection);
					LstSetListChoices(list, uf_list, uf_count);
					UpdatePopup(trgSection, lstSection, last_uf, uf_list);

					StrCopy(cur_uf_name, uf_list[last_uf]);
					uf_display(cur_uf_name);
					text_setpos(last_doc_pos);
					}
				}
			break;
		case mnuBeam:
			last_doc_pos = text_getpos();
			fv_savestatus(cur_uf_name, last_doc_pos, last_doc_font);
			uf_savecheck();
			beam_send_db(cur_uf_name);
			break;
		case mnuEdit:
			last_doc_pos = text_getpos();
			fv_savestatus(cur_uf_name, last_doc_pos, last_doc_font);
			uf_savecheck();

			uf_close();
			FrmGotoForm(MainForm);
			break;
		case mnuSave:
			last_doc_pos = text_getpos();
			fv_savestatus(cur_uf_name, last_doc_pos, last_doc_font);
			uf_save(cur_uf_name);
			FldSetDirty(fld_ptr(fldTEXT), 0);
			break;
		case mnuSaveAs:
			last_doc_pos = text_getpos();
			fv_savestatus(cur_uf_name, last_doc_pos, last_doc_font);
			if	( BasicInputBox("New FileName:", temp, 255, 0) )	{
				frm = FrmGetActiveForm();
				uf_save(temp);

				last_uf = 0;
				for ( i = 0; i < uf_count; i ++ )	{
					if	( StrCompare(cur_uf_name, uf_list[i]) == 0 )	{
						last_uf = i;
						break;
						}
					}

				list = GetListPtr(lstSection);
				LstSetListChoices(list, uf_list, uf_count);
				UpdatePopup(trgSection, lstSection, last_uf, uf_list);

				StrCopy(cur_uf_name, uf_list[last_uf]);
				uf_display(cur_uf_name);
				text_setpos(last_doc_pos);
				}
			break;
		case mnuClose:
			last_doc_pos = text_getpos();
			fv_savestatus(cur_uf_name, last_doc_pos, last_doc_font);
			uf_savecheck();

			uf_close();
			FrmGotoForm(MainForm);
			break;
		case mnuRun:
			last_doc_pos=text_getpos();
			fv_savestatus(cur_uf_name,last_doc_pos,last_doc_font);
			uf_savecheck();
			uf_close();
			return_to_form = MainForm; //is this right?
			StrCopy(cur_file_name,cur_uf_name);
			FrmGotoForm(SBOutForm);
			break;
		case mnuGoto:
			if	( BasicInputBox("Go to line...", temp, 64, 0) )	{
				ide_goto(xstrtol(temp));
				last_doc_pos = text_getpos();
				}
	      	break;
		case mnuSearch:
			if	( BasicInputBox("Search for:", temp, 255, 0) )	{
				ide_search_for(temp);
				last_doc_pos = text_getpos();
				}
	      	break;
		case mnuSearchAgain:
			ide_search_again();
			last_doc_pos = text_getpos();
			break;
		case mnuAbout:
			DoAboutForm();
	      	break;
		case mnuFileInfo:
	      	break;
		case mnuFSVHelp:
//			GPlmAlert(HelpFSVStrTID, HelpFSVStringID);
	      	break;
		case mnuUndo:
			FldUndo(fld_ptr(fldTEXT));
			break;
		case mnuCut:
			FldCut(fld_ptr(fldTEXT));
			break;
		case mnuCopy:
			FldCopy(fld_ptr(fldTEXT));
			break;
		case mnuSelAll:
			FldSetSelection(fld_ptr(fldTEXT), 0, 0xFFFF);
			break;
	   	case mnuPaste:			  
			FldPaste(fld_ptr(fldTEXT));
			break;
	   	case mnuKeyboard:
			SysKeyboardDialog(kbdDefault);
			break;
		 case mnuGraffiti:
			SysGraffitiReferenceDialog(referenceDefault);
			break;
	   	case mnuFont0:
			FldSetFont(fld_ptr(fldTEXT), (last_doc_font = stdFont));
			last_doc_pagesize = 10;
			save_prefs();
			break;
	   	case mnuFont1:
			FldSetFont(fld_ptr(fldTEXT), (last_doc_font = boldFont));
			last_doc_pagesize = 10;
			save_prefs();
			break;
	   	case mnuFont2:
			FldSetFont(fld_ptr(fldTEXT), (last_doc_font = largeFont));
			last_doc_pagesize = 8;
			save_prefs();
			break;
	   	case mnuFont7:
			FldSetFont(fld_ptr(fldTEXT), (last_doc_font = largeBoldFont));
			last_doc_pagesize = 7;
			save_prefs();
			break;
	   	case mnuFontHK:
	   	case mnuFontSM10:
	   	case mnuFontDOS:
	   	case mnuFontDOS4:
			if	( os_charset == enc_utf8 )	{
				int		idx;

				idx = e->data.menu.itemID - mnuFontHK;
				FldSetFont(fld_ptr(fldTEXT), (last_doc_font = font_table[idx].id));
				last_doc_pagesize = font_table[idx].lpp;
				}
			else
				FrmAlert(AlertCFontMB);
			save_prefs();
			break;
		case	mnuDelete:
			FileDelete(0, cur_uf_name);

			last_uf = 0;
			for ( i = 0; i < uf_count; i ++ )	{
				if	( StrCompare(cur_uf_name, uf_list[i]) == 0 )	{
					last_uf = i;
					break;
					}
				}

			list = GetListPtr(lstSection);
			LstSetListChoices(list, uf_list, uf_count);
			UpdatePopup(trgSection, lstSection, last_uf, uf_list);

			StrCopy(cur_uf_name, uf_list[last_uf]);
			uf_display(cur_uf_name);
			text_setpos(last_doc_pos);
			break;
		  	};
    	handled = true;
		break;

	case keyDownEvent:
		switch ( e->data.keyDown.chr )	{
		case	pageUpChr: /* Prev */
			if ( FldScrollable(fld_ptr(fldTEXT), winUp) ) 
				FldScrollField(fld_ptr(fldTEXT), last_doc_pagesize, winUp);
			handled = true;
			break;
		case pageDownChr: /* next */
			if	( FldScrollable(fld_ptr(fldTEXT), winDown) )	
				FldScrollField(fld_ptr(fldTEXT), last_doc_pagesize, winDown);
			handled = 1;
			break;
			};
		break;
    case ctlSelectEvent:
		switch(e->data.ctlSelect.controlID) {
		case	btnSFT:
			FldInsert(fld_ptr(fldTEXT), "\t", 1);
	    	handled = true;
			break;
		case	btnCLOSE:
			last_doc_pos = text_getpos();
			fv_savestatus(cur_uf_name, last_doc_pos, last_doc_font);
			uf_savecheck();

			uf_close();
			FrmGotoForm(MainForm);
	    	handled = true;
			break;
		case	btnUP:
			if ( FldScrollable(fld_ptr(fldTEXT), winUp) ) 
				FldScrollField(fld_ptr(fldTEXT), 1, winUp);
	    	handled = true;
			break;
		case	btnDOWN:
			if	( FldScrollable(fld_ptr(fldTEXT), winDown) )	
				FldScrollField(fld_ptr(fldTEXT), 1, winDown);
	    	handled = true;
			break;
		case	btnPGUP:
			if ( FldScrollable(fld_ptr(fldTEXT), winUp) ) 
				FldScrollField(fld_ptr(fldTEXT), last_doc_pagesize, winUp);
	    	handled = true;
			break;
		case	btnPGDN:
			if	( FldScrollable(fld_ptr(fldTEXT), winDown) )	
				FldScrollField(fld_ptr(fldTEXT), last_doc_pagesize, winDown);
	    	handled = true;
			break;
		case	btnSAVE:
			last_doc_pos = text_getpos();
			fv_savestatus(cur_uf_name, last_doc_pos, last_doc_font);
			uf_save(cur_uf_name);
			FldSetDirty(fld_ptr(fldTEXT), 0);
	    	handled = true;
			break;
			}
		break;		   

    default:
        break;
    	}

    return handled;
}

Boolean		fsv_eventloop(EventPtr e)
{
	return fsv_eventloopSEG(e);
}

//// OUTPUT /////////////////////////////////////////////////////////////////////////////

/*
*	CALLED FROM BRUN
*/
int		BRUNHandleEventSEG(EventPtr e)	SEC(IDE);
int		BRUNHandleEventSEG(EventPtr e)
{
	int		rv = 0;

    switch (e->eType) {
	case frmCloseEvent:
		if	( brun_status() == BRUN_RUNNING )	
			brun_stop();
		break;

    case menuEvent:
		switch(e->data.menu.itemID) {
		case	mnuRun:
			brun_stop();
			EvtAddEventToQueue(e);
			rv = 1;
			break;
		case	mnuBreak:
			brun_break();
			rv = 1;
			break;
		case	mnuClose:
			brun_stop();
			EvtAddEventToQueue(e);
			rv = 1;
			break;
		case mnuAbout:
			DoAboutForm();
	      	break;
		case mnuHelp:
		case mnuHelpC1:
		case mnuHelpC2:
		case mnuHelpC3:
		case mnuHelpC4:
		case mnuHelpC5:
		case mnuHelpC6:
		case mnuHelpC7:
		case mnuHelpC8:
		case mnuHelpC9:
			{
				int dif = e->data.menu.itemID - mnuHelpC1;
				int	tid = HelpC1TID + dif;
				int	sid = HelpC1ID  + dif;

				GPlmAlert(tid, sid);
//				PDOCAlert(SBDOC_GENERIC);
			}
			break;
			}
		break;

	default:
		break;
    	}

    return rv;
}

//
int		BRUNHandleEvent(EventPtr e)
{
	return BRUNHandleEventSEG(e);
}

//
Boolean		OutFormHandleEventSEG(EventPtr e) SEC(IDE);
Boolean		OutFormHandleEventSEG(EventPtr e)
{
	Boolean		handled = false;

    switch (e->eType) {
    case frmOpenEvent:
		dev_cls();
		if	( auto_run )	{
			opt_quite = 1;
			dev_print("\033[4mPlease, wait...\n\033[0m\n");
			}

		sbasic_main(cur_file_name);
		handled = true;
		break;

	case frmCloseEvent:
		if	( brun_status() == BRUN_RUNNING )	
			brun_stop();

		handled = true;
		break;

	case keyDownEvent:
		switch ( e->data.keyDown.chr )	{
		case	'.': 
			if	( brun_status() != BRUN_RUNNING )	{
				EventType	e;

				e.eType = menuEvent;
				e.data.menu.itemID = mnuClose;
				EvtAddEventToQueue(&e);
				}
			break;
			};
    case menuEvent:

		switch(e->data.menu.itemID) {
		case	mnuRun:
			sbasic_main(cur_file_name);
			handled = true;
			break;
		case	SBFinishNotify:
			// '* DONE *' is on display :)
			handled = true;

			if	( !auto_run )	
				break;
			else	{
				if	( gsb_last_error > 0 )	
					break;
				}

			// else continue with 'mnuClose'
		case	mnuClose:
			if	( gsb_last_error > 0 )	{
				//
				//	normal or script, there is an error so jump to editor
				//
				auto_run = 0;
				opt_quite = 0;

				last_err_doc_line = gsb_last_line;

				FrmGotoForm(ViewForm);
				}
			else	{
				//
				//	No errors
				//
				last_err_doc_line = 0;
				if	( auto_run )	{
					//
					//	script : store BC and quit
					//
					opt_quite = 0;
					auto_run = 0;
					sys_exec("lnch");
					}
				else	{
					//
					//	normal : jump to previous form
					//
					if	( return_to_form )
						FrmGotoForm(return_to_form);
					else
						FrmGotoForm(MainForm);
					}
				}
			handled = true;
			break;
		case mnuAbout:
			DoAboutForm();
			handled = true;
			break;
		case mnuHelpC1:
		case mnuHelpC2:
		case mnuHelpC3:
		case mnuHelpC4:
		case mnuHelpC5:
		case mnuHelpC6:
		case mnuHelpC7:
		case mnuHelpC8:
		case mnuHelpC9:
			{
				int dif = e->data.menu.itemID - mnuHelpC1;
				int	tid = HelpC1TID + dif;
				int	sid = HelpC1ID  + dif;

				GPlmAlert(tid, sid);
//				PDOCAlert(SBDOC_GENERIC);
			}
			break;
			}
		break;

	default:
		break;
    	}

	return handled;
}

Boolean		OutFormHandleEvent(EventPtr e)
{
	return	OutFormHandleEventSEG(e);
}

/*
*	Converts a MemoPad entry to .BAS
*/
void	import_memo(char *file)	SEC(IDE);
void	import_memo(char *file)
{
	dword		pos;
	DmOpenRef	f;
	word		rec_n = 1;
	dev_file_t	df;

	memo_mount();

	strcpy(df.name, "MEMO:");
	strcat(df.name, file);
	df.open_flags = DEV_FILE_INPUT;
	if	( !memo_exist(df.name) )	{
		FrmCustomAlert(InfoAlertID, df.name, ": Memo does not exists", "");
		memo_umount();
		return;
		}
	memo_open(&df);

	bas_setfileext(file);
	bas_create(file);

	f = db_open_alw(file, SmBa /* idBASC */, idTEXT);

	pos = sizeof(sec_t);
	db_write_seg(f, rec_n, pos, df.drv_data, strlen(df.drv_data)+1);

	memo_close(&df);
	db_close(f);
	memo_umount();
}

/*
*	Exports the .BAS file 'source' to MemoPad
*/
void	export_memo(char *source, char *file)	SEC(IDE);
void	export_memo(char *source, char *file)
{
	file_t		fin;
	word		recs, i, len;
	dword		total;
	char		*ptr, ch = 0;
	dev_file_t	df;

	memo_mount();

	fin = db_open(source);

	strcpy(df.name, "MEMO:");
	strcat(df.name, file);
	df.open_flags = DEV_FILE_OUTPUT;
	memo_open(&df);

	// copy
	recs = db_rec_count(fin);
	total = 0;
	for ( i = 1; i < recs; i ++ )	{
		len = db_rec_size(fin, i);
		ptr = tmp_alloc(len+1);
		db_read(fin, i, ptr, len);
		ptr[len] = '\0';
		ptr += sizeof(sec_t);
		total += (len = strlen(ptr));
		if	( total < (4000-67) )
			memo_write(&df, ptr, len);
		else	{
			len = (4000-67) - (total - len);
			if	( len > 1 )
				memo_write(&df, ptr, len);
			ptr -= sizeof(sec_t);
			tmp_free(ptr);
			FrmCustomAlert(InfoAlertID, df.name, ": Memo limit reached", "");
			break;
			}
		ptr -= sizeof(sec_t);
		tmp_free(ptr);
		}

	memo_write(&df, &ch, 1);
	memo_close(&df);
	memo_umount();
	db_close(fin);
}

/*
*	Converts a PDOC file to .BAS
*/
void	import_pdoc(char *file)	SEC(IDE);
void	import_pdoc(char *file)
{
	char		pdocfile[33];
	char		txt[128];
	int			i, page = 1;
	dword		pos;
	pdoc_t		doc;
	DmOpenRef	f;
	word		rec_n = 1;

	strcpy(pdocfile, file);
	switch ( pdoc_lopen(pdocfile, &doc) )	{
	case	-1:
		FrmCustomAlert(InfoAlertID, pdocfile, ": File not found", "");
		return;
	case	-2:
		FrmCustomAlert(InfoAlertID, pdocfile, ": Cannot open file", "");
		return;
	case	-4:
		FrmCustomAlert(InfoAlertID, pdocfile, ": Read error", "");
		return;
	case	-5:
		FrmCustomAlert(InfoAlertID, pdocfile, ": Bad signature", "");
		return;
		}

	bas_setfileext(file);
	bas_create(file);

	f = db_open_alw(file, SmBa /* idBASC */, idTEXT);

	pos = sizeof(sec_t);
	for ( i = 1; i <= doc.page_count; i ++ )	{
		pdoc_loadpage(&doc, i);

		if	( (doc.page.len + pos) > 32000 )	{
			// change page
			sec_t		sec;

			page ++;
			StrPrintF(txt, "Page%d", page);
			StrCopy(sec.name, txt);

			// write section
			sec.sign = 'S';
			sec.flags = 0;

			rec_n = db_write_seg(f, 0xFFFF, 0, &sec, sizeof(sec_t));
			pos = sizeof(sec_t);
			}

		// write text
		db_write_seg(f, rec_n, pos, doc.page.ptr, doc.page.len+1);
		pos += doc.page.len;
		}

	pdoc_lclose(&doc);
	db_close(f);
}

/*
*	Exports the .BAS file 'source' to PDOC format ('file')
*/
void	export_pdoc(char *source, char *file)	SEC(IDE);
void	export_pdoc(char *source, char *file)
{
	FileHand	f;
	char		tmpname[33];
	Err			last_error;
	file_t		fin;
	word		recs, i, len;
	char		*ptr, ch = 0;

	strcpy(tmpname, "pdoc_export.tmp");
	f = FileOpen(0, (char *) tmpname, ID_UFST, ID_SmBa, fileModeReadWrite | fileModeAnyTypeCreator, &last_error);

	fin = db_open(source);
	if	( db_exist(file) ) db_remove(file);

	// copy
	recs = db_rec_count(fin);
	for ( i = 1; i < recs; i ++ )	{
		len = db_rec_size(fin, i);
		ptr = tmp_alloc(len+1);
		db_read(fin, i, ptr, len);
		ptr[len] = '\0';
		ptr += sizeof(sec_t);
		FileWrite(f, ptr, strlen(ptr), 1, &last_error);
		ptr -= sizeof(sec_t);
		tmp_free(ptr);
		}
	FileWrite(f, &ch, 1, 1, &last_error);
	db_close(fin);
	FileClose(f);

	// convert
	pdoc_create_pdoc_from_file(tmpname, file);

	// cleanup
	if	( DmFindDatabase(0, (char*) tmpname) != 0 )
		FileDelete(0, (char*) tmpname);
}

//// MAIN/SELECT /////////////////////////////////////////////////////////////////////////////

/*
*	rebuilds source-file list
*/
static void refresh_bas_list() SEC(IDE);
static void refresh_bas_list()
{
	ListPtr	list;
	build_baslist();
	list = GetListPtr(lstBROWSE);
	LstSetListChoices(list, doc_list, doc_count); 
	LstDrawList(list);
}

/*
*	File-details form
*/
static int show_file_details(char *file) SEC(IDE);
static int show_file_details(char *file)
{
	FormPtr	frmPrev, frmDet;
	ListPtr	list;
	ControlPtr	popup, script;
	int		hit;
	info_t	hdr;

	frmPrev = FrmGetActiveForm();
	frmDet = FrmInitForm(DetForm);
	FrmSetActiveForm(frmDet);

	FrmSetTitle(frmDet, file);

	bas_getheader(file, &hdr);

	list = GetListPtr(lstCat);
	LstSetListChoices(list, cat_list, cat_count);
	LstSetSelection(list, cur_cat);
	popup = (ControlPtr) FrmGetObjectPtr(frmDet, FrmGetObjectIndex(frmDet, trgCat));
	CtlSetLabel(popup, cat_list[cur_cat]);
	script = FrmGetObjectPtr(frmDet, FrmGetObjectIndex(frmDet, chkSDKDRAW));
	CtlSetValue(script, ((hdr.flags & 2) != 0));

	hit = FrmDoDialog(frmDet);

	if	( hit == btnOK )	{
		// get vals
		hdr.category = LstGetSelection(list);

		hdr.flags = 0;
		if	( CtlGetValue(script) ) 
			hdr.flags |= 2;

		bas_updateheader(file, &hdr);
		}

	if	( frmPrev )
		FrmSetActiveForm(frmPrev);
	FrmDeleteForm(frmDet);
	return ( hit == btnOK );
}

/*
*	Categories edit-form
*
*	evtRefreshDOCs event is used for the update of the status-line (current line-number)
*/
static Boolean EditCatHandleEventSEG(EventPtr e) SEC(IDE);
static Boolean EditCatHandleEventSEG(EventPtr e)
{
    Boolean handled = false;
    FormPtr frm = NULL;
	ListPtr	list;
	EventType	ev;
	file_t		f;
    
    switch (e->eType) {
    case frmOpenEvent:
		frm = FrmGetActiveForm();
		FrmDrawForm(frm);

		list = GetListPtr(lstCat);
		LstSetListChoices(list, cat_list, cat_count);
		LstDrawList(list);
		SetListSel(lstCat, cur_cat);

		return_to_form = 0;
		handled = true;
		break;

	case frmCloseEvent:
		break;

	case	evtRefreshDOCs:
		free_catlist();
		load_catlist();

		list = GetListPtr(lstCat);
		LstSetListChoices(list, cat_list, cat_count);
		LstDrawList(list);
		SetListSel(lstCat, cur_cat);
		break;

    case lstSelectEvent:
		switch(e->data.lstSelect.listID) {
		case lstCat:
			cur_cat = GetListSel(lstCat);
    		handled = true;
			break;
			}
		break;

    case ctlSelectEvent:
		switch(e->data.ctlSelect.controlID) {
		case	btnOK:
		case	btnCANCEL:
			FrmGotoForm(MainForm);
	    	handled = true;
			break;
		case	btnNEW:
			if	( BasicInputBox("New:", temp, 255, 0) )	{
				f = db_open("SmBa.IDE.CAT");
				db_write(f, 0xFFFF, temp, StrLen(temp)+1);
				db_close(f);
				db_set_bckbit("SmBa.IDE.CAT");

				ev.eType = evtRefreshDOCs;
				EvtAddEventToQueue(&ev);
				}
	    	handled = true;
			break;

		case	btnRUN:	// RENAME
			if	( BasicInputBox("Rename:", temp, 255, 0) )	{
				f = db_open("SmBa.IDE.CAT");
				db_write(f, cur_cat, temp, StrLen(temp)+1);
				db_close(f);
				db_set_bckbit("SmBa.IDE.CAT");

				ev.eType = evtRefreshDOCs;
				EvtAddEventToQueue(&ev);
				}
	    	handled = true;
			break;

		case	btnDELETE:
			if	( cur_cat != 0 && cur_cat != cat_unfiled_id )	{
				dword	oldCat = cur_cat;

				f = db_open("SmBa.IDE.CAT");
				DmRemoveRecord(f, cur_cat);
				db_close(f);
				db_set_bckbit("SmBa.IDE.CAT");

				free_catlist();
				load_catlist();

				bas_setunfiled(oldCat);
				cur_cat = 0;

				ev.eType = evtRefreshDOCs;
				EvtAddEventToQueue(&ev);
				}
			else
				warn_beep();
	    	handled = true;
			break;
			}
		break;

    default:
        break;
    	}

    return handled;
}

Boolean EditCatHandleEvent(EventPtr e)
{
	return EditCatHandleEventSEG(e);
}

static int show_cat_edit() SEC(IDE);
static int show_cat_edit()
{
	FrmGotoForm(EditCatForm);
	return 1;
}

Boolean MainFormHandleEventSEG(EventPtr e) SEC(IDE);
Boolean MainFormHandleEventSEG(EventPtr e)
{
    Boolean handled = false;
    FormPtr frm = NULL;
	FormPtr	frmpop = NULL;
	ListPtr	list;
	char	*fileName;
	static int lastFile;
	LocalID	LID;
	FormActiveStateType	stateP;
    
    switch (e->eType) {
    case frmOpenEvent:
		frm = FrmGetActiveForm();
		FrmDrawForm(frm);

		cur_cat = sbpref.last_cat;

		build_baslist();

		sbpref.last_doc_form = MainForm;

		if	( doc_count )	{
			list = GetListPtr(lstBROWSE);
			LstSetListChoices(list, doc_list, doc_count); 
			LstDrawList(list);
			SetListSel(lstBROWSE, lastFile);
			}

		list = GetListPtr(lstCat);
		LstSetListChoices(list, cat_list, cat_count);
		UpdatePopup(trgCat, lstCat, cur_cat, cat_list);

		return_to_form = 0;
		handled = true;
		break;

	case frmCloseEvent:
		free_baslist();
		break;

	case	evtRefreshDOCs:
		refresh_bas_list();
		lastFile = 0;
		list = GetListPtr(lstBROWSE);
		LstSetListChoices(list, doc_list, doc_count); 
		LstDrawList(list);
		SetListSel(lstBROWSE, lastFile);
		break;

    case menuEvent:
		switch(e->data.menu.itemID) {
		case mnuSBPREF:
			frm = FrmGetActiveForm();
			FrmSaveActiveState(&stateP);

			frmpop = FrmInitForm(SBPREF_FORM);
			FrmSetActiveForm(frmpop);
			{
				ControlPtr	dellog, delsbvm, usevmt;
				ListPtr		list;
				ControlPtr	popup;
				char		num[33];

				dellog = FrmGetObjectPtr(frmpop, FrmGetObjectIndex(frmpop, chkDELLOG));
				CtlSetValue(dellog, ((sbpref.flags & 1) != 0));

				delsbvm = FrmGetObjectPtr(frmpop, FrmGetObjectIndex(frmpop, chkDELSBVM));
				CtlSetValue(delsbvm, ((sbpref.flags & 2) != 0));

				usevmt = FrmGetObjectPtr(frmpop, FrmGetObjectIndex(frmpop, chkSDKDRAW));
				CtlSetValue(usevmt, ((sbpref.flags & 8) != 0));

				list = (ListPtr) FrmGetObjectPtr(frmpop, FrmGetObjectIndex(frmpop, lstCHAR));

				popup = (ControlPtr) FrmGetObjectPtr(frmpop, FrmGetObjectIndex(frmpop, trgCHAR));
				CtlSetLabel(popup, LstGetSelectionText(list, sbpref.charset));

				StrPrintF(num, "%d", sbpref.cclabs1);
				fld_setText(fldCLABS1, num, 0);
				StrPrintF(num, "%d", sbpref.ccpass2);
				fld_setText(fldCPASS2, num, 0);

				if	( FrmDoDialog(frmpop) == btnOK )	{
					sbpref.charset = LstGetSelection(list);
					sbpref.flags = 0;

					if	( CtlGetValue(dellog) )	
						sbpref.flags |= 1;
					if	( CtlGetValue(delsbvm) )	
						sbpref.flags |= 2;
					sbpref.flags |= 4;	// SDK 3.5 draw; ON by default
					if	( CtlGetValue(usevmt) )	{
						opt_usevmt = 1;
						sbpref.flags |= 8;
						}
					else
						opt_usevmt = 0;

					sbpref.cclabs1 = (word) xstrtol(fld_getTextPtr(fldCLABS1));
					sbpref.ccpass2 = (word) xstrtol(fld_getTextPtr(fldCPASS2));
					if	( sbpref.cclabs1 < 64 )		sbpref.cclabs1 = 64;
					if	( sbpref.ccpass2 < 64 )		sbpref.ccpass2 = 64;

					save_prefs();
					}
			}

			FrmRestoreActiveState(&stateP);
			FrmSetActiveForm(frm);
			FrmDeleteForm(frmpop);

			os_charset = sbpref.charset;
			os_cclabs1 = sbpref.cclabs1;
			os_ccpass2 = sbpref.ccpass2;

//			FrmUpdateForm(MainForm, frmRedrawUpdateCode);
			break;
		case mnuRemoveAllBIN:
			vm_delete_all();
			FrmCustomAlert(InfoAlertID, "All binary files deleted", "", "");
			break;
		case mnuClose:
			sys_exec("lnch");
			break;
		case mnuAbout:
			DoAboutForm();
	      	break;
		case mnuHelpC1:
		case mnuHelpC2:
		case mnuHelpC3:
		case mnuHelpC4:
		case mnuHelpC5:
		case mnuHelpC6:
		case mnuHelpC7:
		case mnuHelpC8:
		case mnuHelpC9:
			{
				int dif = e->data.menu.itemID - mnuHelpC1;
				int	tid = HelpC1TID + dif;
				int	sid = HelpC1ID  + dif;

				GPlmAlert(tid, sid);
//				PDOCAlert(SBDOC_GENERIC);
			}
			break;
		case	mnuBeam:
			if	( doc_count )	{
				fileName = doc_list[lastFile];
				beam_send_db(fileName);
				}
			break;
		case	mnuDetails:
			if	( doc_count )	{
				fileName = doc_list[lastFile];
				if	( show_file_details(fileName) && cur_cat != 0 )	{
					refresh_bas_list();

					lastFile = 0;
					list = GetListPtr(lstBROWSE);
					LstSetListChoices(list, doc_list, doc_count); 
					LstDrawList(list);
					SetListSel(lstBROWSE, lastFile);
					}
				}
			break;
		case	mnuFSV:
			FrmGotoForm(FSVForm);
			break;
		case	mnuNew:
			if	( BasicInputBox("New FileName:", temp, 255, 0) )	{
				fileName = temp;
				bas_setfileext(temp);
				bas_create(fileName);
				if	( db_exist(fileName) )	{
					StrCopy(cur_file_name, fileName);
					FrmGotoForm(ViewForm);
					}
				}
			break;
		case mnuPDOCImport:
			if	( BasicInputBox("PDOC:", temp, 255, 0) )	{
				if	( db_exist(temp) )	{
					import_pdoc(temp);
					fileName = temp;
					if	( db_exist(fileName) )	{
						StrCopy(cur_file_name, fileName);
						FrmGotoForm(ViewForm);
						}
					}
				else
					FrmCustomAlert(InfoAlertID, temp, ": File does not exists", "");
				}
			break;
		case	mnuPDOCExport:
			StrCopy(cur_file_name, doc_list[lastFile]);
			if	( BasicInputBox("New PDOC:", temp, 255, 0) )	
				export_pdoc(cur_file_name, temp);
			break;

		case mnuMEMOImport:
			if	( BasicInputBox("Memo:", temp, 255, 0) )	{
				import_memo(temp);
				fileName = temp;
				if	( db_exist(fileName) )	{
					StrCopy(cur_file_name, fileName);
					FrmGotoForm(ViewForm);
					}
				}
			break;
		case	mnuMEMOExport:
			StrCopy(cur_file_name, doc_list[lastFile]);
			if	( BasicInputBox("New MEMO:", temp, 255, 0) )	
				export_memo(cur_file_name, temp);
			break;
		case	mnuEditCat:
			show_cat_edit();
			break;
		case	mnuEdit:
			if	( doc_count )	{
				fileName = doc_list[lastFile];
				if	( db_exist(fileName) )	{
					StrCopy(cur_file_name, fileName);
					FrmGotoForm(ViewForm);
					}
				}
			break;
		case	mnuCopyFile:
			StrCopy(cur_file_name, doc_list[lastFile]);
			if	( BasicInputBox("New FileName:", temp, 255, 0) )	{
				bas_setfileext(temp);
				fileName = temp;
				bas_copy(cur_file_name, fileName);
				refresh_bas_list();
				}
			break;
		case	mnuRename:
			StrCopy(cur_file_name, doc_list[lastFile]);
			if	( BasicInputBox("New FileName:", temp, 255, 0) )	{
				bas_setfileext(temp);
				fileName = temp;

				bas_rename(cur_file_name, fileName);
				refresh_bas_list();

				lastFile = 0;
				list = GetListPtr(lstBROWSE);
				LstSetListChoices(list, doc_list, doc_count); 
				LstDrawList(list);
				SetListSel(lstBROWSE, lastFile);
				}
			break;
		case	mnuRun:
			return_to_form = MainForm;
			if	( doc_count )	{
				fileName = doc_list[lastFile];
				StrCopy(cur_file_name, fileName);
				FrmGotoForm(SBOutForm);
				}
			break;

		case	mnuDelete:
			if	( doc_count )	{
				fileName = doc_list[lastFile];
				if	( FrmCustomAlert(ConfAlertID, "Delete '", fileName, "' ?") == 0 )	{
					fileName = doc_list[lastFile];
					LID = DmFindDatabase(0, fileName);
					if	( LID )	{
						DmDeleteDatabase(0, LID);
						bas_removebin(fileName, 1);

						refresh_bas_list();

						lastFile = 0;
						list = GetListPtr(lstBROWSE);
						LstSetListChoices(list, doc_list, doc_count); 
						LstDrawList(list);
						SetListSel(lstBROWSE, lastFile);
						}
					else
						FrmCustomAlert(InfoAlertID, fileName, ": The PalmOS API routine DmFindDatabase() FAILED!!!", "");
					}
				}
			break;
			}

    	handled = true;
		break;

	case popSelectEvent:
    	switch (e->data.popSelect.controlID) {
	    case trgCat:
			if	( cur_cat != e->data.popSelect.selection )	{
				cur_cat = e->data.popSelect.selection;
				sbpref.last_cat = cur_cat;

				refresh_bas_list();
				lastFile = 0;
				list = GetListPtr(lstBROWSE);
				LstSetListChoices(list, doc_list, doc_count); 
				LstDrawList(list);
				SetListSel(lstBROWSE, lastFile);

				UpdatePopup(trgCat, lstCat, cur_cat, cat_list);
				}
			handled = 1;
			break;
			}
		break;

    case lstSelectEvent:
		switch(e->data.lstSelect.listID) {
		case lstBROWSE:
			lastFile = GetListSel(lstBROWSE);
    		handled = true;
			break;
			}
		break;

    case ctlSelectEvent:
		switch(e->data.ctlSelect.controlID) {
		case	btnCLOSE:
			sys_exec("lnch");
	    	handled = true;
			break;
		case	btnOPEN:
			if	( doc_count )	{
				fileName = doc_list[lastFile];
				if	( db_exist(fileName) )	{
					StrCopy(cur_file_name, fileName);
					FrmGotoForm(ViewForm);
					}
				}
	    	handled = true;
			break;
		case	btnNEW:
			if	( BasicInputBox("New FileName:", temp, 255, 0) )	{
				fileName = temp;
				bas_setfileext(temp);
				bas_create(fileName);
				if	( db_exist(fileName) )	{
					StrCopy(cur_file_name, fileName);
					FrmGotoForm(ViewForm);
					}
				}
	    	handled = true;
			break;
		case	btnRUN:
			return_to_form = MainForm;
			if	( doc_count )	{
				fileName = doc_list[lastFile];
				StrCopy(cur_file_name, fileName);

				FrmGotoForm(SBOutForm);
				}
	    	handled = true;
			break;

		case	btnDELETE:
			if	( doc_count )	{
				fileName = doc_list[lastFile];
				if	( FrmCustomAlert(ConfAlertID, "Delete '", fileName, "' ?") == 0 )	{
					fileName = doc_list[lastFile];
					LID = DmFindDatabase(0, fileName);
					if	( LID )	{
						DmDeleteDatabase(0, LID);
						bas_removebin(fileName, 0);

						refresh_bas_list();

						lastFile = 0;
						list = GetListPtr(lstBROWSE);
						LstSetListChoices(list, doc_list, doc_count); 
						LstDrawList(list);
						SetListSel(lstBROWSE, lastFile);
						}
					else
						FrmCustomAlert(InfoAlertID, fileName, ": The PalmOS API routine DmFindDatabase() FAILED!!!", "");
					}
				}
			
	    	handled = true;
			break;
			}
		break;

    default:
        break;
    	}

    return handled;
}

Boolean MainFormHandleEvent(EventPtr e)
{
	return MainFormHandleEventSEG(e);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Boolean ApplicationHandleEvent(EventPtr e)
{
    FormPtr frm;
    Word    formId;

    if (e->eType == frmLoadEvent) {
		formId = e->data.frmLoad.formID;
		frm = FrmInitForm(formId);
		FrmSetActiveForm(frm);

		switch(formId) {
		case MainForm:
		    FrmSetEventHandler(frm, MainFormHandleEvent);
		    break;
		case ViewForm:
		    FrmSetEventHandler(frm, editor_eventloop);
		    break;
		case FSVForm:
		    FrmSetEventHandler(frm, fsv_eventloop);
		    break;
		case EditCatForm:
		    FrmSetEventHandler(frm, EditCatHandleEvent);
		    break;
		case SBOutForm:
		    FrmSetEventHandler(frm, OutFormHandleEvent);
		    break;
			}
		return true;
	    }

    return false;
}

/*
*	free list of categories
*/
void	free_catlist()
{
	int		i;

	if	( cat_list )	{
		for ( i = 0; i < cat_count; i ++ )	
			tmp_free(cat_list[i]);
		tmp_free(cat_list);

		cat_list = NULL;
		cat_count = 0;
		}
	else
		cat_count = 0;
}

/*
*	Load or build categories
*/
void	load_catlist()
{
	file_t	f;
	int		i, recs;
	byte	*buff;

	free_catlist();

	buff = tmp_alloc(64);
	if	( !db_exist("SmBa.IDE.CAT") )	{
		f = db_open_alw("SmBa.IDE.CAT", SmBa, idDATA);

		StrCopy(buff, "All");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		StrCopy(buff, "Business");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		StrCopy(buff, "Personal");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		StrCopy(buff, "Library");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		StrCopy(buff, "Graphics");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		StrCopy(buff, "Math.");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		StrCopy(buff, "System");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		StrCopy(buff, "Temp");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		StrCopy(buff, "Unfiled");
		db_write(f, 0xFFFF, buff, StrLen(buff)+1);

		db_close(f);
		}

	f = db_open("SmBa.IDE.CAT");

	recs = DmNumRecords(f);
	cat_list = tmp_alloc(sizeof(char_p) * recs);
	cat_count = 0;
	for ( i = 0; i < recs; i ++ )	{
		db_read(f, i, buff, 64);

		if	( StrCompare(buff, "Unfiled") == 0 )	
			cat_unfiled_id = i;

		cat_list[cat_count] = tmp_alloc(StrLen(buff)+1);
		StrCopy(cat_list[cat_count], buff);
		cat_count ++;
		}

	tmp_free(buff);
	db_close(f);
}

/*
*	Load preferences
*/
void load_prefs() SEC(IDE);
void load_prefs()
{
	word	prefSize, prefVer;

	prefSize = sizeof(sbpref_t);
	prefVer = PrefGetAppPreferences(SmBa, 0, &sbpref, &prefSize, 1);
	if	( prefVer == noPreferenceFound )	{
		sbpref.last_doc_sec = 0;
		sbpref.last_doc_pos = 0;
		sbpref.last_doc_form = 0;
		sbpref.font = stdFont;
		sbpref.last_cat = cur_cat;
		sbpref.charset = 0;
		sbpref.flags = 0x1;
		sbpref.cclabs1 = sbpref.ccpass2 = 256;
		}

	if	( sbpref.cclabs1 <= 0 )
		sbpref.cclabs1 = 256;
	if	( sbpref.ccpass2 <= 0 )
		sbpref.ccpass2 = 256;

	os_charset = sbpref.charset;
	opt_safedraw = ((sbpref.flags & 4) != 0);
	opt_usevmt = ((sbpref.flags & 8) != 0);
	os_cclabs1 = sbpref.cclabs1;
	os_ccpass2 = sbpref.ccpass2;
}

/*
*	Save preferences
*/
void save_prefs() SEC(IDE);
void save_prefs()
{
	if	( !dont_save_prefs )	{
		StrCopy(sbpref.last_filename, cur_file_name);
		sbpref.last_cat = cur_cat;
		sbpref.last_doc_sec = last_doc_sec;
		sbpref.last_doc_pos = last_doc_pos;
//		sbpref.last_doc_form = ViewForm;
		sbpref.font = last_doc_font;
		PrefSetAppPreferences(SmBa, 0, 1, &sbpref, sizeof(sbpref_t), 1);
		}
}

/*
*	setup "color" screen mode
*/
//void	init_scrmode() SEC(IDE);
void	init_scrmode()
{
	DWord	NewDepth;
	DWord	SupportDepth = 1;
	Boolean	SupportColor_flag = 0;
	
	#if defined(SONY_CLIE)
	// Check for Sony CLIE
	UInt32	val;
	dword	romVersion;
	Err		error;
	
 	use_sony_clie = 0;
	
           FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	   if	( romVersion >= sysVersion35 )	{
		if ( !FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &val) ) {
			if ( val == sonyHwrOEMCompanyID_Sony )  {
				SonySysFtrSysInfoP	sip;
				Err	status = 0;

				if ( ( error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32 *) &sip)) == 0) {
					if ( sip->libr & sonySysFtrSysInfoLibrHR ) {	// HR lib needed
						if ((error = SysLibFind(sonySysLibNameHR, &sony_refHR))) {
					    	if ( error == sysErrLibNotFound )
						    	error = SysLibLoad('libr', sonySysFileCHRLib, &sony_refHR);
						 	}

						if ( !error)
							use_sony_clie = 1;																   
					   }
					}
				}
			}
		}	
		

	if	( use_sony_clie )	{
		error = HROpen(sony_refHR);
		if ( error )
			use_sony_clie = 0;
		else	{
			UInt32	width, height, depth;

			width = hrWidth;
			height = hrHeight;
//			width = opt_pref_width;
//			height = opt_pref_height;
//			depth = opt_pref_bpp;
			if ( romVersion >= sysVersion40 )
				depth = 16;
			else
				depth = 8;    // support older N710C with OS3.5
			error = HRWinScreenMode(sony_refHR, winScreenModeSet, &width, &height, &depth, NULL);
			HRFntSetFont(sony_refHR, hrStdFont);
			}
		}


	if	( !use_sony_clie )	{
	#endif

	FtrGet(sysFtrCreator, sysFtrNumDisplayDepth, &OldScreenDepth);
	ScrDisplayMode(scrDisplayModeGetSupportedDepths, NULL, NULL, &SupportDepth,NULL);
	ScrDisplayMode(scrDisplayModeGetSupportsColor, NULL, NULL, NULL, &SupportColor_flag);

	if      (SupportDepth & 0x0080) NewDepth = 8;
	else if (SupportDepth & 0x0008) NewDepth = 4;
	else if (SupportDepth & 0x0002) NewDepth = 2;
	else                            NewDepth = 1;

	if ( !FtrSet(sysFtrCreator, sysFtrNumDisplayDepth, NewDepth) ) 	
		ScrDisplayMode(scrDisplayModeSetToDefaults,NULL,NULL,NULL,NULL);
	else	{
		ScrDisplayMode(scrDisplayModeSet,NULL,NULL,&NewDepth,NULL);
		WinScreenMode(winScreenModeSet, 0, 0, &NewDepth, 0);
		}

	#if defined(SONY_CLIE)
	}
	#endif
}

/*
*	restore screen mode
*/
//void	restore_scrmode() SEC(IDE);
void	restore_scrmode()
{
	#if defined(SONY_CLIE)
	if	( use_sony_clie )	{
		HRWinScreenMode(sony_refHR, winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
		HRClose(sony_refHR);
		}
	else	{
	#endif
	FtrSet(sysFtrCreator, sysFtrNumDisplayDepth, OldScreenDepth);
	ScrDisplayMode(scrDisplayModeSetToDefaults, NULL, NULL, NULL, NULL);
	#if defined(SONY_CLIE)
	}
	#endif
}

/*
*	load buildin fonts
*/
void	load_fonts() SEC(IDE);
void	load_fonts()
{
	int		i;

	for ( i = 0; font_table[i].id != 0; i ++ )	{
		if ( i == 0 )
			font_table[i].h = DmGetResource('pFNT', font_table[i].rid);
		else
			font_table[i].h = DmGetResource('NFNT', font_table[i].rid);

		FntDefineFont(font_table[i].id, MemHandleLock(font_table[i].h));
		}
}

/*
*	load buildin fonts
*/
void	unload_fonts() SEC(IDE);
void	unload_fonts()
{
	int		i;

	for ( i = 0; font_table[i].id != 0; i ++ )	{
		MemHandleUnlock(font_table[i].h);
		DmReleaseResource(font_table[i].h);
		}
}

/*
*/
//Word sbpad_init(void) SEC(IDE);
Word sbpad_init(void)
{
	Err	error;

	//
	init_scrmode();
	load_catlist();
	error = SysLibFind(MathLibName, &MathLibRef);
	if	( error )		{
		error = SysLibLoad(LibType, MathLibCreator, &MathLibRef);
		if	( error )	{
			FrmAlert(AlertMathLib);
			restore_scrmode();
			return 0;
			}
		}
	else	{
		if	( error )	{
			FrmAlert(AlertMathLib);
			restore_scrmode();
			return 0;
			}
		}

	error = MathLibOpen(MathLibRef, MathLibVersion);
	if	( error )	{
		FrmAlert(AlertMathLib);
		restore_scrmode();
		return 0;
		}
	
	load_fonts();
	load_prefs();

	if	( sbpref.binver != SB_DWORD_VER )	{
		// first time with that version
		vm_delete_all();

		// update prefs
		sbpref.binver = SB_DWORD_VER;
		save_prefs();
		}
	else if ( (sbpref.flags & 2) == 0 )	
		bas_rm_all_bin_nonscripts();

	if	( sbpref.flags & 1 )
		FileDelete(0, "SB.LOG");

	if	( auto_run )
		FrmGotoForm(SBOutForm);
	else	{
		if	( sbpref.last_doc_form == ViewForm && db_exist(sbpref.last_filename) )	{
			StrCopy(cur_file_name, sbpref.last_filename);
			FrmGotoForm(ViewForm);
			}
		else
		    FrmGotoForm(MainForm);
		}
    return 1;
}

/*
*/
//void sbpad_close(void) SEC(IDE);
void sbpad_close(void)
{
	Int		librefs;

	bas_close();
	save_prefs();
    FrmSaveAllForms();
    FrmCloseAllForms();

	free_catlist();
	
	unload_fonts();

	MathLibClose(MathLibRef, &librefs);
	if	( librefs == 0 )
		SysLibRemove(MathLibRef);

	restore_scrmode();
}

/*
*/
void EventLoop(void)
{
    Word err;
    EventType e;

    do {
		EvtGetEvent(&e, evtWaitForever);
		if (!SysHandleEvent (&e))	{
		    if (!MenuHandleEvent (NULL, &e, &err))	{
				if (!ApplicationHandleEvent (&e))	
				    FrmDispatchEvent (&e);
				}
			}
	    } while (e.eType != appStopEvent);
}

/*
*/
DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    Word	err;

	if ( cmd == sysAppLaunchCmdOpenDB ) {
		/*
		*	Command-line args in PalmOS! :)
		*/
		word 	card = ((SysAppLaunchCmdOpenDBType *) cmdPBP)->cardNo;
		LocalID lid  = ((SysAppLaunchCmdOpenDBType *) cmdPBP)->dbID;
		dword	type, creator;

        char docname[dmDBNameLength];
        DmDatabaseInfo(card, lid, docname, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator);
        if ( !(type == 'TEXT' && creator == 'SmBa') ) {
			FrmAlert(AlertArgOpen);
			return 1;
	        }

		// update prefs - auto load the new file
		load_prefs();
		sbpref.last_doc_form = ViewForm;

		/// Load ////////////////////////////////////////////
		strcpy(cur_file_name, docname);
		bas_open(cur_file_name);
		if	( cur_file_header.flags & 2 )	{	// PalmOS script --> run it
			auto_run = 1;
			dont_save_prefs = 1;
			return_to_form = 0;
			bas_close();
			}
		else	{
			auto_run = 0;
			dont_save_prefs = 0;
			bas_close();

			strcpy(sbpref.last_filename, docname);
			PrefSetAppPreferences(SmBa, 0, 1, &sbpref, sizeof(sbpref_t), 1);
			}

		// normal launch
        cmd = sysAppLaunchCmdNormalLaunch;
		}

    if ( cmd == sysAppLaunchCmdNormalLaunch )	{
		/*
		*	Normal launch
		*/
		ExgRegisterData(SmBa, exgRegExtensionID, "bas");		//	Register .bas files
		if	( sbpad_init() )	{
			EventLoop();
			sbpad_close();
			}
	    } 
	else if (cmd == sysAppLaunchCmdSyncNotify) {
		ExgRegisterData(SmBa, exgRegExtensionID, "bas");		//	Register .bas files
		}
	else if (cmd == sysAppLaunchCmdExgAskUser)    {
		ExgAskParamPtr paramP = (ExgAskParamPtr)cmdPBP;
       	paramP->result = exgAskOk; // paramP->result = exgAskCancel;
        }
	else if (cmd == sysAppLaunchCmdExgReceiveData)    {
		byte prog_running = ((launchFlags & sysAppLaunchFlagSubCall) != 0);

		if	( (err = beam_get_db((ExgSocketPtr)cmdPBP, prog_running)) != 0 )
			return err;
		}

    return 0;
}

