//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USEFORM("conv_main.cpp", FCSMain);
USEFORM("conv_view.cpp", frmView);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "SmallBASIC BAS2PDB Convertor";
		Application->CreateForm(__classid(TFCSMain), &FCSMain);
		Application->CreateForm(__classid(TfrmView), &frmView);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
