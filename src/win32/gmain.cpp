/*
*	SmallBASIC for Win32
*
*	Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "pmem.h"

#include "gmain.h"

HANDLE	hStdFont;
HANDLE	hBoldFont;


extern "C" void	brun(char *file);
extern "C" void	brun_break(void);
extern "C" int	bcb_devinit(void);
extern "C" void	bcb_devrestore(void);
int		SaveSBPDB(const char *fname, const char *text);
int		LoadSBPDB(const char *fname, char**text);

AnsiString	curFileName;
TStringList	*file_list;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFMain *FMain;
//---------------------------------------------------------------------------
__fastcall TFMain::TFMain(TComponent* Owner)
	: TForm(Owner)
{
	file_list = new TStringList;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::OnClose(TObject *Sender, TCloseAction &Action)
{
	delete file_list;	
}
//---------------------------------------------------------------------------
void __fastcall TFMain::mnuExitClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TFMain::OnShow(TObject *Sender)
{
	curFileName = "file.bas";
	txtCode->Lines->LoadFromFile(curFileName);
}
//---------------------------------------------------------------------------
void __fastcall TFMain::mnuRunClick(TObject *Sender)
{
	AnsiString	tmpfile;

	tmpfile = "bcbdef.run";
	txtCode->Lines->SaveToFile(tmpfile);
	bcb_devinit();
	memmgr_init();	
	brun(tmpfile.c_str());
	memmgr_close();	
	bcb_devrestore();	
	remove(tmpfile.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TFMain::mnuBreakClick(TObject *Sender)
{
	brun_break();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::mnuSaveClick(TObject *Sender)
{
	txtCode->Lines->SaveToFile(curFileName);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::mnuOpenClick(TObject *Sender)
{
	if	( dlgOpen->Execute() )	{
		curFileName = dlgOpen->FileName;
		
		if	( dlgOpen->FilterIndex == 1 )	{
			txtCode->Lines->LoadFromFile(curFileName);
			}
		else	{
			char	*buf;
		
			switch ( LoadSBPDB(dlgOpen->FileName.c_str(), &buf) )	{
			case	-1:
			case	-2:
				Application->MessageBoxA("Can't open file", "SBW32", MB_OK);
				break;
			case	-3:
				Application->MessageBoxA("Texts > 32KB does not supported, yet.", "SBW32", MB_OK);
				break;
			case	-4:
				Application->MessageBoxA("Bad signature.", "SBW32", MB_OK);
				break;
			default:				
				txtCode->Text = AnsiString(buf);
				delete[] buf;
				}
			
			//
			curFileName = ChangeFileExt(curFileName, ".bas");
			}

			
		TTabSheet	*curtab = pgMain->Pages[0];
		curtab->Caption = ExtractFileName(curFileName);
		curtab->Hint = curFileName;
		}
}
//---------------------------------------------------------------------------

void __fastcall TFMain::mnuSaveASClick(TObject *Sender)
{
	if	( dlgSave->Execute() )	{
		if	( dlgSave->FilterIndex == 1 )	{
			curFileName = dlgSave->FileName;
			TTabSheet	*curtab = pgMain->Pages[0];
			curtab->Caption = ExtractFileName(curFileName);
			curtab->Hint = curFileName;
			
			txtCode->Lines->SaveToFile(dlgSave->FileName);
			}
		else	{
			AnsiString	s = txtCode->Text;
			switch ( SaveSBPDB(dlgSave->FileName.c_str(), s.c_str()) )	{
			case	-1:
				Application->MessageBoxA("Can't create file", "SBW32", MB_OK);
				break;
			case	-3:
				Application->MessageBoxA("Texts > 32KB does not supported, yet.", "SBW32", MB_OK);
				}
			}
		}
}

//---------------------------------------------------------------------------


void __fastcall TFMain::txtCodeOnChange(TObject *Sender)
{
	UpdatePos();	
}
//---------------------------------------------------------------------------


void TFMain::UpdatePos()
{
	AnsiString	msg;

	msg.printf("Line %d, Col %d", txtCode->CaretPos.y+1, txtCode->CaretPos.x+1);
	StatusBar1->SimpleText = msg;
	StatusBar1->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::txtCodeOnKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	UpdatePos();	
}
//---------------------------------------------------------------------------

void __fastcall TFMain::mnuGoToClick(TObject *Sender)
{
	AnsiString	defPos, txt_line;
	int			line;
	
	defPos.printf("%d", txtCode->CaretPos.y+1);
	txt_line = InputBox("Go to line", "Line", defPos);
	line = txt_line.ToInt();
	if	( line > 0 )	{
		line --;
		txtCode->CaretPos.y = line;
		txtCode->Refresh();
		}
}
//---------------------------------------------------------------------------

