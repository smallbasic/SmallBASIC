//---------------------------------------------------------------------------

//
//	Update it from unix version:
//
//	Copy form pdb2bas.cc and paste to pdb.cpp
//
//	In PDB.CPP:
//
//	Declare:
//
//	void	err_printf(const char *fmt, ...);	// message-box
//	void	con_printf(const char *fmt, ...);	// console output (text window)
//
//	Remove routines usage() and main()
//	
//

#include <vcl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#pragma hdrstop

#include "conv_main.h"
#include "conv_view.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFCSMain *FCSMain;

char	lastSource[1024];

typedef char * char_p;

int		SaveSBPDB(const char *fname, const char *text);
int		LoadSBPDB(const char *fname, char_p *text);

void	err_printf(const char *fmt, ...)
{
	char	buf[8192];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

	Application->MessageBoxA(buf, "BAS2PDB/PDB2BAS", MB_OK);
}

void	con_printf(const char *fmt, ...)
{
	char	buf[8192];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

	FCSMain->txtConsole->Lines->Add(buf);
}

bool	filetype(const char *src, const char *cnt_ext)
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


//---------------------------------------------------------------------------
__fastcall TFCSMain::TFCSMain(TComponent* Owner)
	: TForm(Owner)
{
	strcpy(lastSource, "");
}
//---------------------------------------------------------------------------
void __fastcall TFCSMain::btnBSRCClick(TObject *Sender)
{
	if	( dlgOpen->Execute() )	{
		txtSRC->Text = dlgOpen->FileName;
		}
}
//---------------------------------------------------------------------------
void __fastcall TFCSMain::btnBTRGClick(TObject *Sender)
{
	if	( dlgSave->Execute() )	{
		txtTRG->Text = dlgSave->FileName;
		}
}
//---------------------------------------------------------------------------
void __fastcall TFCSMain::btnConvertClick(TObject *Sender)
{
	bool		t2p;
	struct stat st1, st2;

	if	( filetype(txtSRC->Text.c_str(), ".pdb") && filetype(txtTRG->Text.c_str(), ".bas") )
		t2p = false;
	else if	( filetype(txtTRG->Text.c_str(), ".pdb") && filetype(txtSRC->Text.c_str(), ".bas") )	
		t2p = true;
	else	{
		con_printf("Wrong file types.\nSelect filenames with extention \".bas\" or \".pdb\"\n\nExample:\nSource: file.pdb\nTarget: file.bas\n");
		return;
		}
	
	if	( t2p )	{
		txtTMP->Lines->LoadFromFile(txtSRC->Text);

		AnsiString	s = txtTMP->Text;
		switch ( SaveSBPDB(txtTRG->Text.c_str(), s.c_str()) )	{
		case	-1:
			con_printf("BAS2PDB-ERROR: Can't create file");
			break;
		case	-3:
			con_printf("BAS2PDB-ERROR: Texts > 32KB does not supported, yet.");
			}
			
		strcpy(lastSource, txtSRC->Text.c_str());
		}
	else	{
		char	*buf;
		
		switch ( LoadSBPDB(txtSRC->Text.c_str(), &buf) )	{
		case	-1:
		case	-2:
			con_printf("PDB2BAS-ERROR: Can't open file");
			break;
		case	-3:
			con_printf("PDB2BAS-ERROR: Texts > 32KB does not supported, yet.");
			break;
		case	-4:
			con_printf("PDB2BAS-ERROR: Bad signature.");
			break;
		default:				
			txtTMP->Text = AnsiString(buf);
			delete[] buf;
			}

		txtTMP->Lines->SaveToFile(txtTRG->Text);
		strcpy(lastSource, txtTRG->Text.c_str());
		}
		
	stat(txtSRC->Text.c_str(), &st1);
	stat(txtTRG->Text.c_str(), &st2);
	con_printf("\r\nSource:\t%s size %d\r\nTarget:\t%s size %d\r\n* DONE *\r\n",
		 txtSRC->Text.c_str(), st1.st_size,
		 txtTRG->Text.c_str(), st2.st_size);
}
//---------------------------------------------------------------------------
void __fastcall TFCSMain::OnChangeSRC(TObject *Sender)
{
	char	*p;
	char	buf[1024];
	char	ext[5];

	strcpy(buf, txtSRC->Text.c_str());
	strcpy(ext, "");
	p = strrchr(buf, '.');
	if	( !p )	
		p = strrchr(buf, '\\');
	else	{
		if	( stricmp(p, ".bas") == 0 )
			strcpy(ext, ".pdb");
		else if	( stricmp(p, ".pdb") == 0 )
			strcpy(ext, ".bas");
		}

	if	( p )	{
		*p = '\0';
		strcat(buf, ext);
		txtTRG->Text = AnsiString(buf);
		}

	txt_view(txtSRC->Text.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TFCSMain::btnViewClick(TObject *Sender)
{
	FILE	*fp;
	
	if	( strlen(lastSource) == 0 )	{
		if ( FileExists(txtSRC->Text) )	{
			strcpy(lastSource, txtSRC->Text.c_str());
			}
		}

	if	( strlen(lastSource) )	{
		;
		TfrmView *f = new TfrmView(this);

		f->txtText->Lines->LoadFromFile(lastSource);
		f->ShowModal();
		delete f;
		}
	else	{
		con_printf("No source file");
		}
}
//---------------------------------------------------------------------------


void TFCSMain::txt_view(const char * file)
{
	if ( FileExists(file) )	{
		if	( filetype(file, ".pdb") )	{
			char	*buf;
		
			switch ( LoadSBPDB(txtSRC->Text.c_str(), &buf) )	{
			case	-1:	case	-2:
				con_printf("PDB2BAS-ERROR: Can't open file");
				break;
			case	-3:
				con_printf("PDB2BAS-ERROR: Texts > 32KB does not supported, yet.");
				break;
			case	-4:
				con_printf("PDB2BAS-ERROR: Bad signature.");
				break;
			default:				
				txtSrcView->Text = AnsiString(buf);
				delete[] buf;
				}
			}
        else if	( filetype(file, ".bas") )	
			txtSrcView->Lines->LoadFromFile(file);
		else
    	   	txtSrcView->Lines->Clear();
		}
	else
   	   	txtSrcView->Lines->Clear();
}
