//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "fsrc.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFSearch *FSearch;
//---------------------------------------------------------------------------
__fastcall TFSearch::TFSearch(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFSearch::btnSearchClick(TObject *Sender)
{
	action = 1;
	Close();	
}
//---------------------------------------------------------------------------

void __fastcall TFSearch::btnReplaceClick(TObject *Sender)
{
	action = 2;
	Close();	
}
//---------------------------------------------------------------------------

void __fastcall TFSearch::btnCancelClick(TObject *Sender)
{
	action = 0;
	Close();	
}
//---------------------------------------------------------------------------

