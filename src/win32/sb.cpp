//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("sb.res");
USEFORM("gmain.cpp", FMain);
USEUNIT("..\unx_memmgr.c");
USEUNIT("..\device.c");
USEUNIT("..\blib_func.c");
USEUNIT("..\blib_graph.c");
USEUNIT("..\blib_sound.c");
USEUNIT("..\brun.c");
USEUNIT("..\ceval.c");
USEUNIT("..\eval.c");
USEUNIT("..\mem.c");
USEUNIT("..\panic.c");
USEUNIT("..\proc.c");
USEUNIT("..\scan.c");
USEUNIT("..\str.c");
USEUNIT("..\blib.c");
USEUNIT("..\var.c");
USEUNIT("pdb.cpp");
USEUNIT("..\dev_bcb.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "SmallBASIC IDE";
		Application->CreateForm(__classid(TFMain), &FMain);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
