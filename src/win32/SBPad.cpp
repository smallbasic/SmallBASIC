//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USEFORM("pad1.cpp", FMain);
USEFORM("fabt.cpp", FAbout);
USEFORM("fsrc.cpp", FSearch);
USEFORM("fsettngs.cpp", FSets);
USEFORM("fcoprog.cpp", FCompProg);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
         Application->Initialize();
         Application->Title = "SBPad for Win32";
         Application->CreateForm(__classid(TFMain), &FMain);
		Application->CreateForm(__classid(TFAbout), &FAbout);
		Application->CreateForm(__classid(TFSearch), &FSearch);
		Application->CreateForm(__classid(TFSets), &FSets);
		Application->CreateForm(__classid(TFCompProg), &FCompProg);
		Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------





