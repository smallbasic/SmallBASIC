//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "conv_view.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmView *frmView;

extern char lastSource[];

//---------------------------------------------------------------------------
__fastcall TfrmView::TfrmView(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmView::btnSaveClick(TObject *Sender)
{
	txtText->Lines->SaveToFile(lastSource);	
}
//---------------------------------------------------------------------------

void TfrmView::txt_view(const char * file)
{
}
