//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "fsettngs.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFSets *FSets;

//---------------------------------------------------------------------------
__fastcall TFSets::TFSets(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFSets::btnEFClick(TObject *Sender)
{
	String	s;
	char	buf[1024], *p, *szp;

	strcpy(buf, txtEF->Text.c_str());
	p = strchr(buf, ',');
	if	( p )	{
		*p = '\0';
		szp = p + 1;
		dlgFont->Font->Name = buf;
		if	( (p = strchr(szp, ',')) )
			*p = '\0';
		dlgFont->Font->Size = atoi(szp);
		}
	else	{
		dlgFont->Font->Name = buf;
		dlgFont->Font->Size = 9;
		}

	if	( dlgFont->Execute() )	{
		s = dlgFont->Font->Name
			+ String(",") + String(dlgFont->Font->Size)
			+ String(",") + String(dlgFont->Font->Charset);
		txtEF->Text = s;
    	}
}
//---------------------------------------------------------------------------
void __fastcall TFSets::btnXFClick(TObject *Sender)
{
	String	s;
    char	buf[1024], *p, *szp;

    strcpy(buf, txtXF->Text.c_str());
    p = strchr(buf, ',');
	dlgFont->Font->Name = buf;
	if	( p )	{
		*p = '\0';
		szp = p + 1;
		dlgFont->Font->Name = buf;
		if	( (p = strchr(szp, ',')) )
			*p = '\0';
		dlgFont->Font->Size = atoi(szp);
        }
	else	{
		dlgFont->Font->Name = buf;
		dlgFont->Font->Size = 9;
        }

	if	( dlgFont->Execute() )	{
		s = dlgFont->Font->Name
			+ String(",") + String(dlgFont->Font->Size)
			+ String(",") + String(dlgFont->Font->Charset);
		txtXF->Text = s;
    	}
}
//---------------------------------------------------------------------------
void __fastcall TFSets::chkVSClick(TObject *Sender)
{
	int		chk = chkVS->Checked;

    txtVW->Enabled = chk;
    txtVH->Enabled = chk;	
}
//---------------------------------------------------------------------------

