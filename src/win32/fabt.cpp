//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "fabt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFAbout *FAbout;
//---------------------------------------------------------------------------
__fastcall TFAbout::TFAbout(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFAbout::Button1Click(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TFAbout::OnCreate(TObject *Sender)
{
	DWORD	fvsize, dummy;
	unsigned int vs_size = sizeof(VS_FIXEDFILEINFO);
	char	*buf;
	VS_FIXEDFILEINFO	*vs;

	fvsize = GetFileVersionInfoSize(Application->ExeName.c_str(), &dummy);
	if	( fvsize )	{
		buf = new char[fvsize];
		if	( GetFileVersionInfo(
				Application->ExeName.c_str(), 0,
				fvsize, buf) )	{

//			vs = new VS_FIXEDFILEINFO;

			VerQueryValue(buf, TEXT("\\"), (void **) &vs, &vs_size);

			#if defined(_DEBUG)
			Version->Caption = String("Debug version ");
			#else
			Version->Caption = String("Version ");
			#endif
			Version->Caption = Version->Caption +
				 String((int) (vs->dwFileVersionMS >> 16)) + String(".") +
				 String((int) (vs->dwFileVersionMS & 0xFFFF)) + String(".") +
				 String((int) (vs->dwFileVersionLS >> 16)) + String(".") +
				 String((int) (vs->dwFileVersionLS & 0xFFFF));

//			delete vs;
			}
		else
			Version->Caption = String("GetFileVersionInfo (Win32-API) FAILED (όλα είναι δυνατά σ' αυτό τον κόσμο) !!!");

		delete[] buf;
		}
	else
		Version->Caption = String("GetFileVersionInfoSize (Win32-API) FAILED (όλα είναι δυνατά σ' αυτό τον κόσμο) !!!");

}
//---------------------------------------------------------------------------
void __fastcall TFAbout::Label4Click(TObject *Sender)
{
	ShellExecute(Application->MainForm->Handle,
				 "open",
				 "http://smallbasic.sourceforge.net",
				 NULL,NULL,SW_SHOWDEFAULT);
}
//---------------------------------------------------------------------------
