//---------------------------------------------------------------------------

#include <vcl.h>
#include <inifiles.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <dir.h>
#include <time.h>
#pragma hdrstop

#define	SEC(x)
#define	MIN(a,b)	( ((a) < (b)) ? (a) : (b) )

#include "fabt.h"
#include "fsrc.h"
#include "fsettngs.h"
#include "pad1.h"
#include "sbpad_bcb.h"
#include "panic.h"
#include "unx_memmgr.h"
#include "keys.h"
#include "smbas.h"
#include "sbapp.h"
#include "fcoprog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SynEdit"
#pragma link "SynEditHighlighter"
#pragma link "SynHighlighterGeneral"
#pragma link "SynHighlighterMulti"
#pragma link "SHDocVw_OCX"
#pragma resource "*.dfm"

#define	F_OK	0
#define	X_OK	1
#define	R_OK	4
#define	W_OK	2

static char	dir_user[1024];
static char bcb_dir[1024];
static char bcb_srcfile[1024];		// current filename
static char bcb_search[1024];
static int	bcb_cur_line;
static int	bcb_cur_col;
static char *help_full_text;

String 		bcb_ef;					// editor font
String 		bcb_xf;					// executor font

int bcb_vs;
int bcb_use_snd;
int bcb_use_mm;
int bcb_capture;
int bcb_vwidth;
int bcb_vheight;
int	timer_key;
int	timer_ctrl;
int	timer_time;

#define	HELP_MAX_KW		1024

typedef struct {
	char	key[64];
	char	*pos;
	} hkw_t;

static hkw_t hkw_table[HELP_MAX_KW];
static int	hkw_count;

#include "../help_subsys.c"

//---------------------------------------------------------------------------
TFMain *FMain;
//---------------------------------------------------------------------------

#define	EVLIST_SIZE		1024
volatile event_t	evlist[EVLIST_SIZE];
volatile int		ev_head = 0, ev_tail = 0;
volatile int		ev_record = 0;
volatile int		stdout_init = 0;
volatile int		running = 0;
Graphics::TBitmap	*bmp = NULL;
typedef char * char_p;

// dev_bcb.cpp:
extern void	w32_stdout_open();
extern void	w32_stdout_close();
extern void osd_bcb_breakoff();
extern void osd_bcb_breakon();

// pdb
extern int	SaveSBPDB(const char *fname, const char *text);
extern int	LoadSBPDB(const char *fname, char_p *text);

// drvsound
extern void	drvsound_soundcard_driveroff();
extern void	drvsound_soundcard_driveron();
extern void	drvsound_nosound_at_all();

/*
*/
extern "C" {	// ??? bug BC6 _cdecl!!!!
void bcb_lwrite(char *s)
{
	FMain->lwrite(s);
}

void bcb_comp(int pass, int pmin, int pmax)	// Win32GUI progress
{
	FMain->CompProg(pass, pmin, pmax);
}
}

/*
*	converts string of font to values
*/
void	bcb_strfont(String s, String &name, int &size, int &charset)
{
	char	*buf = strdup(s.c_str());
	char	*p, *szp;

	name = buf;		// font name
	size = 9;		// default size
	charset = 1;	// default charset

	// get size
	p = strchr(buf, ',');
	if	( p )	{
		*p = '\0';
		szp = p + 1;
		name = buf;

		// get charset
		p = strchr(p+1, ',');
		if	( p )	{
			*p = '\0';
			charset = atoi(p+1);
			}

		size = atoi(szp);
		}

	free(buf);
}

/*
*/
void _cdecl w32_copybmp()
{
	if	( stdout_init )
		FMain->imgOut->Canvas->Draw(0,0,bmp);
}

/*
*	refresh request from dev_bcb
*/
void _cdecl w32_invalidate_rect()
{
	w32_copybmp();
}

/*
*	return the canvas of bmp
*/
TCanvas* _cdecl	w32_canvas(void)
{
	if	( bmp )
		return bmp->Canvas;
	return NULL;
}

/*
*	start/stop storing events
*/
void _cdecl w32_evstate(int enable)
{
	if	( enable )
		ev_head = ev_tail = 0;
	ev_record = enable;
	if	( !enable )
		ev_head = ev_tail = 0;
}

/*
*	stores an event to queue
*/
void _cdecl w32_evpush(event_t *ev)
{
	if	( ev_record )	{
		evlist[ev_tail] = *ev;
		if	( ev_tail+1 >= EVLIST_SIZE )
			ev_tail = 0;
		else
			ev_tail ++;
		}
}

/*
*	returns true if there is an event in queue (event is COPIED in the 'ev')
*/
int _cdecl w32_evget(event_t *ev)
{
	if ( ev_tail == ev_head )
		return 0;

	*ev = evlist[ev_head];
	if	( ev_head+1 >= EVLIST_SIZE )
		ev_head = 0;
	else
		ev_head ++;
	return 1;
}

/*
*/
void _cdecl w32_refresh()
{
	w32_copybmp();
}

bool	bcb_filetype(const char *src, const char *cnt_ext)
{
	char	*p, *e;
	char	*buf = new char[strlen(src)+1];
	char	*ext = new char[strlen(cnt_ext)+1];
	bool	rv = false;

	strcpy(buf, src);
	strcpy(ext, cnt_ext);
	strlwr(buf);
	strlwr(ext);

	p = strrchr(buf, '.');
	e = ext;
	if	( p )	{
		p ++;
		if	( *e == '.' )
			e ++;
		rv = (strcmp(e, p) == 0);
		}

	delete[] buf;
	delete[] ext;
	return rv;
}

/* ------------------------------------------------------------------------------------------------------------------- */

void _cdecl sbr_shell(const char *program)
{
	char	file[1024];
	FILE	*fp;

	running = 1;

	strcpy(file, bcb_srcfile);
	strcat(file, ".tmp.run");
	fp = fopen(file, "wt");
	if	( !fp )	{
		sprintf(file, "%s\\%s.run", dir_user, baseof(bcb_srcfile, '\\'));
		fp = fopen(file, "wt");
		
		if	( !fp )	{
			running = 0;
			Application->MessageBox("Can't create temporary file! Process aborted.", "Error");
			return;
			}
		}
	fwrite(program, strlen(program), 1, fp);
	fclose(fp);

	sbasic_main(file);

	w32_refresh();

	remove(file);

	running = 0;
}

/* ------------------------------------------------------------------------------------------------------------------- */

void	close_dev()
{
	if	( stdout_init )	{
		stdout_init = 0;
		dev_restore();
		w32_stdout_close();
		delete bmp;
		bmp = NULL;
		if	( !bcb_vs )
			FMain->imgOut->Align = alClient;
		}
}

void	reset_dev()
{
	TRect	rbmp;

	if	( stdout_init )
		close_dev();

	bmp = new Graphics::TBitmap;
	if	( !bcb_vs )	{
		rbmp.Left = 0; rbmp.Right  = FMain->imgOut->Width - 1;
		rbmp.Top  = 0; rbmp.Bottom = FMain->imgOut->Height - 1;
		}
	else	{
		rbmp.Left = 0; rbmp.Right  = bcb_vwidth - 1;
		rbmp.Top  = 0; rbmp.Bottom = bcb_vheight - 1;
		FMain->imgOut->Align = alNone;
		FMain->imgOut->Width = rbmp.Width();
		FMain->imgOut->Height = rbmp.Height();
		}

	bmp->Height = rbmp.Height();
	bmp->Width  = rbmp.Width();
	bmp->Canvas->Brush->Color = (TColor) 0xFFFFFF;
	bmp->Canvas->FillRect(rbmp);

	w32_stdout_open();
	dev_init(1,1);
	stdout_init = 1;
}

/*
*	Send BREAK key to executor
*/
void	sbbreak()
{
	event_t	ev;

	osd_bcb_breakon();
	
	if	( running )	{
		// send CTRL+C
		ev.type = EVKEY;
		ev.ch = 3;
		w32_evpush(&ev);
		}
}

/*
*	run (or restart) a program
*/
void	sbrun(const char *text)
{
	if	( running )	{
		Application->MessageBoxA("Program is running", "SBPad", MB_OK);
		return;
		}

	AnsiString	program = AnsiString(text);

	if	( bcb_use_snd )	{
		if	( bcb_use_mm )
			drvsound_soundcard_driveron();
		else
			drvsound_soundcard_driveroff();
		}
	else
		drvsound_nosound_at_all();
		
	osd_bcb_breakoff();
	reset_dev();
	if	( bcb_srcfile[1] == ':' )	{
		char	*newdir = strdup(bcb_srcfile);
		char	*p = strrchr(newdir, '\\');

		if	( p )	{
			*p = '\0';
			chdir(newdir);
			}
		else
			chdir(bcb_dir);

		free(newdir);
		}
	else
		chdir(bcb_dir);

	if	( strlen(bcb_srcfile) )
		sbr_shell(program.c_str());
}

/*
*	register file types
*/
void	reg_file_types()
{
	TRegIniFile		*r = new TRegIniFile("");
	AnsiString		app, appcmd;
	AnsiString		appico;

	app = AnsiString("\"") + Application->ExeName + AnsiString("\"");
	appico = app + AnsiString(",0");

	// SB
//	appcmd = app + AnsiString(" -tsc \"%1\"");
	appcmd = app + AnsiString("\"%1\"");
	r->RootKey = HKEY_CLASSES_ROOT;
	r->WriteString(".sb","","SmallBASIC");
	r->WriteString("SmallBASIC","","SmallBASIC Source File");
	r->WriteString("SmallBASIC\\DefaultIcon", "", appico);
	r->WriteString("SmallBASIC\\Shell","","Open");
	r->WriteString("SmallBASIC\\Shell\\Open","","Open with SmallBASIC");
	r->WriteString("SmallBASIC\\Shell\\Open\\command","", appcmd);
	
	delete r;
}


//---------------------------------------------------------------------------
__fastcall TFMain::TFMain(TComponent* Owner)
	: TForm(Owner)
{
	char		*p;
	String		s, entry, defVal;
	int			n, cs, i;
	TIniFile	*ini;

	reg_file_types();

	//
	lastUsedFiles = new TStringList;

	//
	sclOut->DoubleBuffered = true;
	strcpy(bcb_dir, Application->ExeName.c_str());
	p = strrchr(bcb_dir, '\\');
	*p = '\0';
	chdir(bcb_dir);

	//
	strcpy(bcb_srcfile, "untitled.sb");
	s.printf("SBPad - %s", bcb_srcfile);
	this->Caption = s;

	// read default vals
	ini = new TIniFile(ChangeFileExt(Application->ExeName, ".INI"));
	bcb_ef = ini->ReadString("PAD", "EF", "Courier New,9,1");
	bcb_xf = ini->ReadString("PAD", "XF", "Courier New,9,1");
	bcb_vs = ini->ReadInteger("PAD", "VS", 0);
	bcb_use_snd = ini->ReadInteger("PAD", "Sound", 1);
	bcb_use_mm = ini->ReadInteger("PAD", "MMSound", 1);
	bcb_capture = ini->ReadInteger("PAD", "Capture", 0);
	bcb_vwidth  = ini->ReadInteger("PAD", "VW", 1024);
	bcb_vheight = ini->ReadInteger("PAD", "VH", 768);

	chkCapture->Checked = bcb_capture;

	// last used files
	lastUsedFiles->Clear();
	for ( i = 0; i < 8; i ++ )	{
		entry.printf("%d", i);
		defVal = "";
		s = ini->ReadString("LastUsedFiles", entry, defVal);
		s = s.Trim();
		if	( s.Length() )	{
			if	( FileExists(s) )
				lastUsedFiles->Add(s);
			}
		}
	rebuildLastUsedFiles();

	delete ini;

	bcb_strfont(bcb_ef, s, n, cs);
	editor->Font->Name = s;
	editor->Font->Size = n;
	editor->Font->Charset = cs;
	txtLog->Font->Name = s;
	txtLog->Font->Size = n;
	txtLog->Font->Charset = cs;

	//
	wchar_t	wcbuf[1024];
	char	filename[1024];
	
	sprintf(filename, "file://%s\\doc\\guide.html", bcb_dir);
	mbstowcs(wcbuf, filename, 1024);
	browser->Navigate(wcbuf);
}
//---------------------------------------------------------------------------
void __fastcall TFMain::OnClose(TObject *Sender, TCloseAction &Action)
{
	int		r = IDYES, i;
	TIniFile	*ini;
	String	s, entry;

	if	( running )	{
		Application->MessageBoxA("Program is running", "SBPad", MB_OK);
		Action = caNone;
		return;
		}
	if	( editor->Modified )	{
		s.printf("Save changes to %s ?", bcb_srcfile);
		if	( (r = Application->MessageBoxA(s.c_str(), "SBPad", MB_YESNOCANCEL)) == IDYES )
			save_source();
		}

	if	( r != IDCANCEL )	{
		if	( stdout_init )
			close_dev();

		Action = caFree;

		// write default vals
		ini = new TIniFile(ChangeFileExt(Application->ExeName, ".INI"));
		ini->WriteString("PAD", "EF", bcb_ef);
		ini->WriteString("PAD", "XF", bcb_xf);
		ini->WriteInteger("PAD", "VS", bcb_vs);
		ini->WriteInteger("PAD", "MMSound", bcb_use_mm);
		ini->WriteInteger("PAD", "Sound", bcb_use_snd);
		ini->WriteInteger("PAD", "Capture", bcb_capture);
		ini->WriteInteger("PAD", "VW", bcb_vwidth);
		ini->WriteInteger("PAD", "VH", bcb_vheight);

		// last used files
		for ( i = 0; i < 8; i ++ )	{
			entry.printf("%d", i);
			if	( i < lastUsedFiles->Count )	{
				if	( FileExists(lastUsedFiles->Strings[i]) )
					ini->WriteString("LastUsedFiles", entry, lastUsedFiles->Strings[i]);
				else
					ini->WriteString("LastUsedFiles", entry, "");
				}
			else
				ini->WriteString("LastUsedFiles", entry, "");
			}

		delete ini;

		delete lastUsedFiles;
		}
	else
		Action = caNone;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Quit1Click(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Load1Click(TObject *Sender)
{
	TIniFile *ini;
    String	 dir, s;
    char	*p;
    int		r;

	if	( editor->Modified )	{
		String	s;

		s.printf("Save changes to %s ?", bcb_srcfile);
		if	( (r = Application->MessageBoxA(s.c_str(), "SBPad", MB_YESNOCANCEL)) == IDYES )
			save_source();
		if	( r == IDCANCEL )
			return;
		}

	// read default dir
	ini = new TIniFile(ChangeFileExt(Application->ExeName, ".INI"));
	dir = ini->ReadString("PAD", "LoadDir", "");
	delete ini;

	if	( dir.Length() )
		dlgOpen->InitialDir = dir;

	if	( dlgOpen->Execute() )	{
		strcpy(bcb_srcfile, dlgOpen->FileName.c_str());

		// store default dir
		p = strrchr(bcb_srcfile, '\\');
		if	( p )	{
			*p = '\0';
			ini = new TIniFile(ChangeFileExt(Application->ExeName, ".INI"));
			ini->WriteString("PAD", "LoadDir", bcb_srcfile);
			delete ini;
			*p = '\\';
			}

		load_source();
		pgMain->ActivePageIndex = 0;
		}
}
//---------------------------------------------------------------------------

void __fastcall TFMain::srcOnKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
/*
	switch ( Key )	{
    
	case	VK_F1:	
            {
            char 	word[64], *p;
           	char	*text, *sp;

			text = strdup(get_cur_line());
			sp = text + txtSource->SelStart;
                
				// find the start of the word
				while ( sp > text )	{
					if	( !isalnum(*sp) )	{
						sp ++;
						break;
						}
					sp --;
					}
				strncpy(word, sp, 32);

				free(text);
				}

			word[31] = '\0';

			// find the end of the word
			p = word;
			while ( *p )	{
				if	( !isalnum(*p) )	{
					*p = '\0';
					break;
					}
				p ++;
				}

			pgMain->ActivePageIndex = 1;
			txtHlpSrcTxt->Text = word;
			txtHlpSrcTxt->SetFocus();
			}
		break;
		};

	update_sbar();
*/
}
//---------------------------------------------------------------------------

void TFMain::update_sbar()
{
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Timer1Timer(TObject *Sender)
{
	if	( timer_time )
		timer_time = 0;
}
//---------------------------------------------------------------------------


void __fastcall TFMain::srcOnMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	update_sbar();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Save1Click(TObject *Sender)
{
	String	s;

	if	( strcmp(bcb_srcfile, "untitled.sb") == 0 )	{
    	Saveas1Click(Sender);
    	}
	else	{
    	save_source();

	    s.printf("SBPad - %s", bcb_srcfile);
		this->Caption = s;
        }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Restart1Click(TObject *Sender)
{
	pgMain->ActivePageIndex = 2;
	sbrun(editor->Text.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Break1Click(TObject *Sender)
{
	sbbreak();
}
//---------------------------------------------------------------------------

/**
*	this create delays
*/
void __fastcall TFMain::outOnMouseMove(TObject *Sender, TShiftState Shift,
	  int X, int Y)
{
	event_t	ev;

	ev.type = EVMOUSE;
	ev.button = 0;
	if	( Shift.Contains(ssLeft) )
		ev.button |= 1;
	if	( Shift.Contains(ssRight) )
		ev.button |= 2;
	if	( Shift.Contains(ssMiddle) )
		ev.button |= 4;

	ev.x = X;
	ev.y = Y;
	w32_evpush(&ev);
}
//---------------------------------------------------------------------------


void __fastcall TFMain::outOnKeyPress(TObject *Sender, char &Key)
{
	if	( running )	{
		event_t	ev;

		ev.type = EVKEY;
		ev.ch = Key;
		w32_evpush(&ev);
		}
}
//---------------------------------------------------------------------------



void __fastcall TFMain::outOnMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
	event_t	ev;

    ev.type = EVMOUSE;
    ev.button = 0;
    if	( Shift.Contains(ssLeft) )
    	ev.button |= 1;
    if	( Shift.Contains(ssRight) )
    	ev.button |= 2;
    if	( Shift.Contains(ssMiddle) )
    	ev.button |= 4;
        
    if	( ev.button )
        imgOut->OnMouseMove = outOnMouseMove;
	else
        imgOut->OnMouseMove = NULL;

    ev.x = X;
    ev.y = Y;
    w32_evpush(&ev);
}
//---------------------------------------------------------------------------


void __fastcall TFMain::About1Click(TObject *Sender)
{
	TFAbout	*f = new TFAbout(this);

    f->ShowModal();
    delete f;	
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Saveas1Click(TObject *Sender)
{
	TIniFile *ini;
    String	 dir, s;
    char	*p;

	// read default dir
	ini = new TIniFile(ChangeFileExt(Application->ExeName, ".INI"));
	dir = ini->ReadString("PAD", "LoadDir", "");
	delete ini;

    if	( dir.Length() )
    	dlgSave->InitialDir = dir;

	if	( dlgSave->Execute() )	{
    	strcpy(bcb_srcfile, dlgSave->FileName.c_str());

        // store default dir
		p = strrchr(bcb_srcfile, '\\');
        if	( p )	{
        	*p = '\0';
			ini = new TIniFile(ChangeFileExt(Application->ExeName, ".INI"));
			ini->WriteString("PAD", "LoadDir", bcb_srcfile);
			delete ini;
        	*p = '\\';
            }

		// save file
        save_source();

	    s.printf("SBPad - %s", bcb_srcfile);
	    this->Caption = s;
    	}
}
//---------------------------------------------------------------------------


void __fastcall TFMain::Settings1Click(TObject *Sender)
{
	TFSets *frm;
	String	s;
	int		n, cs;

	frm = new TFSets(this);
    frm->txtEF->Text = bcb_ef;
    frm->txtXF->Text = bcb_xf;
	frm->chkVS->Checked = bcb_vs;
	frm->txtVW->Text = String(bcb_vwidth);
	frm->txtVH->Text = String(bcb_vheight);
	frm->chkMMSound->Checked = bcb_use_mm;
	frm->chkSound->Checked = bcb_use_snd;
	
	if	( bcb_vs == 0 )	{
		frm->txtVW->Enabled = false;
		frm->txtVH->Enabled = false;
		}
	if	( frm->ShowModal() == mrOk )	{
		bcb_ef = frm->txtEF->Text;
		bcb_xf = frm->txtXF->Text;
		bcb_vs = frm->chkVS->Checked;
		bcb_use_mm = frm->chkMMSound->Checked;
		bcb_use_snd = frm->chkSound->Checked;
		bcb_vwidth = atoi(frm->txtVW->Text.c_str());
		bcb_vheight = atoi(frm->txtVH->Text.c_str());
		
		bcb_strfont(bcb_ef, s, n, cs);
		editor->Font->Name = s;
		editor->Font->Size = n;
		editor->Font->Charset = cs;
		txtLog->Font->Name = s;
		txtLog->Font->Size = n;
		txtLog->Font->Charset = cs;
		}
	delete frm;
}
//---------------------------------------------------------------------------


void __fastcall TFMain::New1Click(TObject *Sender)
{
	String		s;
    TDateTime	dt = TDateTime::CurrentDateTime();
    int		r;

	if	( editor->Modified )	{
		String	s;

		s.printf("Save changes to %s ?", bcb_srcfile);
		if	( (r = Application->MessageBoxA(s.c_str(), "SBPad", MB_YESNOCANCEL)) == IDYES )
			save_source();
		if	( r == IDCANCEL )
			return;
		}

	strcpy(bcb_srcfile, "untitled.sb");
	s = String("' ") + dt.DateTimeString() + String("\n");
	editor->ClearAll();
	editor->Text = s.c_str();
	pgMain->ActivePageIndex = 0;
}
//---------------------------------------------------------------------------

void TFMain::load_source()
{
	String		s;

	if ( bcb_filetype(bcb_srcfile, ".pdb") )	{
		// PDB
		char	*buf;

		switch ( LoadSBPDB(bcb_srcfile, &buf) )	{
		case	-1:
		case	-2:
			warning("PDB2BAS-ERROR: Can't open file");
			break;
		case	-3:
			warning("PDB2BAS-ERROR: Texts > 32KB does not supported, yet.");
			break;
		case	-4:
			warning("PDB2BAS-ERROR: Bad signature.");
			break;
		default:
			editor->ClearAll();
			editor->Text = buf;
			free(buf);
			}
		}
	else	{
		// open file
		editor->Lines->LoadFromFile(bcb_srcfile);
		updateLastUsedFiles(bcb_srcfile);
		}

	s.printf("SBPad - %s", bcb_srcfile);
	this->Caption = s;
}

void TFMain::save_source()
{
	char	bak[1024];

	strcpy(bak, bcb_srcfile);
	strcat(bak, ".bak");
	copy_file(bcb_srcfile, bak);

	if ( bcb_filetype(bcb_srcfile, ".pdb") )	{
		String	s;

		s = editor->Text;
		switch ( SaveSBPDB(bcb_srcfile, s.c_str()) )	{
		case	-1:
			warning("BAS2PDB-ERROR: Can't create file");
			break;
		case	-3:
			warning("BAS2PDB-ERROR: Texts > 32KB does not supported, yet.");
			}
		}
	else
		editor->Lines->SaveToFile(bcb_srcfile);
		
	updateLastUsedFiles(bcb_srcfile);
}

void TFMain::copy_file(const char * src, const char * dst)
{
	FILE	*fin, *fout;
	char	buf[512];

	if	( (fin = fopen(src, "rt")) == NULL )
		return;
	if	( (fout = fopen(dst, "wt")) == NULL )	{
		fclose(fin);
		return;
		}

	while ( fgets(buf, 512, fin) )
		fputs(buf, fout);

	fclose(fin);
	fclose(fout);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::OnShow(TObject *Sender)
{
	int		i, j;
	TListItem	*item;
	char	*help, *pfound, *next, *p;
	int		idx;
	char	key[64];
	DWORD	dwdummy = 512;
	char	filename[1024];
	char	dirname[1024];
	DIR		*dir;
	struct dirent *ent;
	int		opt_ihavename = 0, opt_nomore = 0;
	
	// create local dir
	strcpy(dir_sb, Application->ExeName.c_str());
	p = strrchr(dir_sb, '\\');
	*p = '\0';
	if	( !GetUserName(username, &dwdummy) )
		strcpy(username, "root");
	sprintf(dir_user, "%s\\%s", dir_sb, username);
	if	( access(dir_user, F_OK) )
		mkdir(dir_user);

	// syntax highlight
	for ( j = 0; opr_table[j].name[0] != '\0'; j ++ )
		gsyn->KeyWords->Add(opr_table[j].name);

	// print keywords
	for ( j = 0; keyword_table[j].name[0] != '\0'; j ++ )	{
		if	( keyword_table[j].name[0] != '$' )
			gsyn->KeyWords->Add(keyword_table[j].name);
		}

	// special separators
	for ( j = 0; spopr_table[j].name[0] != '\0'; j ++ )
		gsyn->KeyWords->Add(spopr_table[j].name);

	// functions
	for ( j = 0; func_table[j].name[0] != '\0'; j ++ )
		gsyn->KeyWords->Add(func_table[j].name);

	// procedures
	for ( j = 0; proc_table[j].name[0] != '\0'; j ++ )
		gsyn->KeyWords->Add(proc_table[j].name);

	// command-line
	if	( _argc > 1 ) {
		int		i;

		for ( i = 1; i < _argc; i ++ )	{
			if	( _argv[i][0] == '-' )	{
				if	( opt_nomore )	{
					if	( strlen(opt_command) )
						strcat(opt_command, " ");
					strcat(opt_command, _argv[i]);
					}
				else	{
					switch ( _argv[i][1] )	{
					case '-':
						// the following parameters are going to script (COMMAND$)
						opt_nomore = 1;
						break;
					case 'g':
						// run in graphics mode
						opt_graphics = 2;
						if	( (_argv[i][2] >= '1') && (_argv[i][2] <= '9') )	{
							char	*slash;
                            char	cwd[1024];
							
							// setup graphics mode
							slash = &_argv[i][2];
							sprintf(cwd, "SBGRAF=%s", slash);
							dev_putenv(cwd);
							comp_preproc_grmode(slash);
							opt_graphics = 2;
							}
						break;
					case 'm':
						// load run-time modules (linux only, shared libraries)
						opt_loadmod = 1;
						strcpy(opt_modlist, _argv[i]+2);
						break;
					case 'q':
						// shutup
						opt_quite = 1;
						break;
					default:
						panic("unknown option: %s\n", _argv[i]);
						};
					}
				}
			else	{
				// no - switch
				// this is the filename or script-parameters
				if	( opt_ihavename == 0 )	{
					strcpy(bcb_srcfile, _argv[i]);
					opt_ihavename = 1;
					}
				else	{
					if	( strlen(opt_command) )
						strcat(opt_command, " ");
					strcat(opt_command, _argv[i]);
					}
				}
			}
			
		if	( opt_ihavename )
			load_source();
		}	
}
//---------------------------------------------------------------------------


void TFMain::help_search(const char * key)
{
}

//---------------------------------------------------------------------------
void __fastcall TFMain::outOnPaint(TObject *Sender)
{
	w32_copybmp();	
}
//---------------------------------------------------------------------------

/**
*	page switch
*/
void __fastcall TFMain::pgMainChange(TObject *Sender)
{
	switch ( pgMain->TabIndex )	{
	case	0:	// editor
		editor->Enabled = true;
		FMain->SetFocus();
		FMain->ActiveControl = NULL;
		break;
	case	1:	// help
		editor->Enabled = false;
		break;
	case	2:	// output
		editor->Enabled = false;
		FMain->SetFocus();
		FMain->ActiveControl = NULL;
		break;
		}
}
//---------------------------------------------------------------------------


void TFMain::rebuildLastUsedFiles()
{
	int		i, count = 0;

	mnuReopen1->Visible = false;
	mnuReopen2->Visible = false;
	mnuReopen3->Visible = false;
	mnuReopen4->Visible = false;
	mnuReopen5->Visible = false;
	mnuReopen6->Visible = false;
	mnuReopen7->Visible = false;
	mnuReopen8->Visible = false;

	for ( i = 0; i < 8; i ++ )	{
		if	( i < lastUsedFiles->Count )	{
			if	( FileExists(lastUsedFiles->Strings[i]) )	{
				mnuReopen->Items[count]->Visible = false;
				mnuReopen->Items[count]->Caption = String(i+1) + String(": ") + lastUsedFiles->Strings[i];
				mnuReopen->Items[count]->Visible = true;

				switch ( count )	{
				case	0:
					mnuReopen1->Caption = mnuReopen->Items[count]->Caption; mnuReopen1->Visible = true;
					break;
				case	1:
					mnuReopen2->Caption = mnuReopen->Items[count]->Caption; mnuReopen2->Visible = true;
					break;
				case	2:
					mnuReopen3->Caption = mnuReopen->Items[count]->Caption; mnuReopen3->Visible = true;
					break;
				case	3:
					mnuReopen4->Caption = mnuReopen->Items[count]->Caption; mnuReopen4->Visible = true;
					break;
				case	4:
					mnuReopen5->Caption = mnuReopen->Items[count]->Caption; mnuReopen5->Visible = true;
					break;
				case	5:
					mnuReopen6->Caption = mnuReopen->Items[count]->Caption; mnuReopen6->Visible = true;
					break;
				case	6:
					mnuReopen7->Caption = mnuReopen->Items[count]->Caption; mnuReopen7->Visible = true;
					break;
				case	7:
					mnuReopen8->Caption = mnuReopen->Items[count]->Caption; mnuReopen8->Visible = true;
					break;
					}

				count ++;
				}
			}
		else
			mnuReopen->Items[i]->Visible = false;
		}
	mnuReopen->RethinkHotkeys();
}

//---------------------------------------------------------------------------

void TFMain::updateLastUsedFiles(const char * name)
{
	bool	found;
	int		i;
	String	file(name);

	found = false;
	for ( i = 0; i < MIN(8, lastUsedFiles->Count); i ++ )	{
		if	( lastUsedFiles->Strings[i].UpperCase() == file.UpperCase() )	{
			found = true;
			break;
			}
		}

	if	( !found )	{
		lastUsedFiles->Insert(0, file);
		rebuildLastUsedFiles();
		}
}
//---------------------------------------------------------------------------
void __fastcall TFMain::mnuReopenX(TObject *Sender)
{
	String	file;

	file = ((TMenuItem *) Sender)->Caption;
	file = file.SubString(4, file.Length()-3).Trim();
	strcpy(bcb_srcfile, file.c_str());
	load_source();
}
//---------------------------------------------------------------------------

/*
*	Search and replace
*/
static String	bcb_sr_search;
static String	bcb_sr_replace;
static bool		bcb_sr_backwards;
static bool		bcb_sr_caseSensitive;
static bool		bcb_sr_entire;
static bool		bcb_sr_selectionOnly;
static bool		bcb_sr_words;

void __fastcall TFMain::textSearchAndReplace(bool replace, bool back)
{
	TSynSearchOptions	o;

	if ( replace )	
		o = TSynSearchOptions() << ssoPrompt << ssoReplace << ssoReplaceAll;
	else
		o = TSynSearchOptions();
		
	if ( back )
		o = o << ssoBackwards;
	if	( bcb_sr_caseSensitive )
		o = o << ssoMatchCase;
	if	( bcb_sr_entire )
		o = o << ssoEntireScope;
	if	( bcb_sr_selectionOnly )
		o = o << ssoSelectedOnly;
	if	( bcb_sr_words )
		o = o << ssoWholeWord;
		
	if ( editor->SearchReplace(bcb_sr_search, bcb_sr_replace, o) == 0 )	{
		MessageBeep(MB_ICONASTERISK);
		if ( back )
			editor->BlockEnd = editor->BlockBegin;
		else
			editor->BlockBegin = editor->BlockEnd;
		editor->CaretXY = editor->BlockBegin;
		}
}

// Find/Replace Dialog
void __fastcall TFMain::Finx1Click(TObject *Sender)
{
	TFSearch *dlg = new TFSearch(this);
	
	dlg->txtSearch->Text  = bcb_sr_search;
	dlg->txtReplace->Text = bcb_sr_replace;
	dlg->chkBack->Checked = bcb_sr_backwards;
	dlg->chkCase->Checked = bcb_sr_caseSensitive;
	dlg->chkEntire->Checked = bcb_sr_entire;
	dlg->chkSelOnly->Checked = bcb_sr_selectionOnly;
	dlg->chkWords->Checked = bcb_sr_words;

	dlg->ShowModal();
	if	( dlg->action )	{
		bcb_sr_search 		=	dlg->txtSearch->Text;
		bcb_sr_replace		=	dlg->txtReplace->Text;
		bcb_sr_backwards	=	dlg->chkBack->Checked;
		bcb_sr_caseSensitive=	dlg->chkCase->Checked;
		bcb_sr_entire		=	dlg->chkEntire->Checked;
		bcb_sr_selectionOnly=	dlg->chkSelOnly->Checked;
		bcb_sr_words		=	dlg->chkWords->Checked;

		textSearchAndReplace((dlg->action == 2), bcb_sr_backwards);
		}

	delete dlg;
}

// Find Again
void __fastcall TFMain::Find1Click(TObject *Sender)
{
	textSearchAndReplace(false, bcb_sr_backwards);
}
//---------------------------------------------------------------------------

void TFMain::lwrite(const char * s)
{
	if	( !bcb_capture )
		return;

	const char	*p = s;
	char	dest[0x10000], *d = dest;

	while ( *p )	{
		if	( *p == '\n' )	{
			*d ++ = '\r';
			*d ++ = '\n';
			}
		else if ( *p == 12 )	{
			// cls
			txtLog->Clear();
			d = dest;
			}
		else if ( *p == 27 )	{
			int		esc_val, esc_cmd;
			
			if	( *(p+1) == '[' )	{
				p += 2;
				esc_val = esc_cmd = 0;

				if	( is_digit(*p) )	{
					esc_val = (*p - '0');
					p ++;

					if	( is_digit(*p) )	{
						esc_val = (esc_val * 10) + (*p - '0');
						p ++;
						}

					esc_cmd = *p;
					}
				else
					esc_cmd = *p;

				// control characters
				switch ( esc_cmd )	{
				case	'K':	// \e[K - clear to eol
					break;
				case	'G':	// set pos 'esc_val'
					break;
				case	'm':			// \e[...m	- ANSI terminal
					switch ( esc_val )	{
					case	0:	// reset
						break;
					case	1:	// set bold on
						break;
					case	4:	// set underline on
						break;
					case	7:	// reverse video on
						break;
					case	21:	// set bold off
						break;
					case	24:	// set underline off
						break;
					case	27:	// reverse video off
						break;

					// fonts - palm emu
					case	80:	// normal fonts
						break;
					case	81:	// bold fonts
						break;
					case	82:	// large fonts
					case	83:	case	84:	case	85:
						break;
					case	87:	// large & bold fonts
					case	86:
						break;
					case	90:	// custom - smallest fonts
						break;
					case	91:	// custom - small - non-fixed fonts
						break;
					case	92:	// custom - small - fixed fonts
						break;
					case	93:	// custom - small - fixed - condensed fonts
						break;

					// colors - 30..37 foreground, 40..47 background
					case	30:	// set black fg
						break;
					case	31:	// set red fg
						break;
					case	32:	// set green fg
						break;
					case	33:	// set brown fg
						break;
					case	34:	// set blue fg
						break;
					case	35:	// set magenta fg
						break;
					case	36:	// set cyan fg
						break;
					case	37:	// set white fg
						break;

					case	40:	// set black bg
						break;
					case	41:	// set red bg
						break;
					case	42:	// set green bg
						break;
					case	43:	// set brown bg
						break;
					case	44:	// set blue bg
						break;
					case	45:	// set magenta bg
						break;
					case	46:	// set cyan bg
						break;
					case	47:	// set white bg
						break;
						};
					break;
					}
				}
			}
		else
			*d ++ = *p;
			
		p ++;
		}
	*d = '\0';

	txtLog->Text = txtLog->Text + AnsiString(dest);
}

void __fastcall TFMain::btnLogClearClick(TObject *Sender)
{
	txtLog->Clear();	
}
//---------------------------------------------------------------------------


void __fastcall TFMain::chkCaptureClick(TObject *Sender)
{
	bcb_capture = chkCapture->Checked;
}
//---------------------------------------------------------------------------


void __fastcall TFMain::outOnKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	if	( running )	{
		event_t	ev;
		ev.type = EVKEY;
		ev.ch = 0;

		switch ( Key )	{
		case VK_PRIOR:	ev.ch = SB_KEY_PGUP;	break;
		case VK_NEXT:	ev.ch = SB_KEY_PGDN;	break;
		case VK_INSERT:	ev.ch = SB_KEY_INSERT;	break;
		case VK_DELETE:	ev.ch = SB_KEY_DELETE;	break;
		case VK_HOME:	ev.ch = SB_KEY_HOME;	break;
		case VK_END:	ev.ch = SB_KEY_END;		break;
		
		case VK_F1:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(1)) : (SB_KEY_SF(1));	break;
		case VK_F2:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(2)) : (SB_KEY_SF(2));	break;
		case VK_F3:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(3)) : (SB_KEY_SF(3));	break;
		case VK_F4:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(4)) : (SB_KEY_SF(4));	break;
		case VK_F5:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(5)) : (SB_KEY_SF(5));	break;
		case VK_F6:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(6)) : (SB_KEY_SF(6));	break;
		case VK_F7:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(7)) : (SB_KEY_SF(7));	break;
		case VK_F8:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(8)) : (SB_KEY_SF(8));	break;
		case VK_F9:		ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(9)) : (SB_KEY_SF(9));	break;
		case VK_F10:	ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(10)) : (SB_KEY_SF(10));	break;
		case VK_F11:	ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(11)) : (SB_KEY_SF(11));	break;
		case VK_F12:	ev.ch = (Shift.Contains(ssShift))? (SB_KEY_F(12)) : (SB_KEY_SF(12));	break;
			};

		if	( ev.ch )
			w32_evpush(&ev);
		}
	else {
		if	( Key == VK_F1 && Shift.Contains(ssCtrl) )	{
			// help on word
			}
		}
}
//---------------------------------------------------------------------------

void __fastcall TFMain::outOnKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	// These keys didn't worked on KeyDown

	if	( running )	{
		event_t	ev;
		ev.type = EVKEY;
		ev.ch = 0;

		switch ( Key )	{
		case VK_LEFT:	ev.ch = SB_KEY_LEFT;	break;
		case VK_RIGHT:	ev.ch = SB_KEY_RIGHT;	break;
		case VK_UP:		ev.ch = SB_KEY_UP;		break;
		case VK_DOWN:	ev.ch = SB_KEY_DOWN;	break;
			};

		if	( ev.ch )
			w32_evpush(&ev);
		}
	
}
//---------------------------------------------------------------------------


void TFMain::CompProg(int pass, int pmin, int pmax)
{
	char	buf[1024];

	if	( pass == 0 )	
		sprintf(buf, "Preparing...");
	if	( pass == 1 )	
		sprintf(buf, "Pass1, Line %d", pmin);
	else if	( pass == 2 )	
		sprintf(buf, "Pass2, Node %d / %d", pmin, pmax);
	else if ( pass == 3 )	
		sprintf(buf, "%s", (pmin)? "Success" : "Error");
	else if ( pass == 4 )	
		sprintf(buf, "Running...");
	else if ( pass == 5 )	
		sprintf(buf, "Stopped!");
	else
		sprintf(buf, "");
		
	StatBar->SimpleText = buf;
	Application->ProcessMessages();
}
