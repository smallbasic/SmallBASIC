// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditKeyCmdEditor.pas' rev: 6.00

#ifndef SynEditKeyCmdEditorHPP
#define SynEditKeyCmdEditorHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditMiscClasses.hpp>	// Pascal unit
#include <SynEditKeyCmds.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <ComCtrls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditkeycmdeditor
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynEditKeystrokeEditorForm;
class PASCALIMPLEMENTATION TSynEditKeystrokeEditorForm : public Forms::TForm 
{
	typedef Forms::TForm inherited;
	
__published:
	Extctrls::TPanel* pnlAlign;
	Stdctrls::TLabel* Label1;
	Stdctrls::TLabel* Label2;
	Stdctrls::TLabel* Label4;
	Stdctrls::TButton* bntClearKey;
	Stdctrls::TButton* btnOK;
	Stdctrls::TComboBox* cmbCommand;
	Stdctrls::TButton* btnCancel;
	void __fastcall FormShow(System::TObject* Sender);
	void __fastcall bntClearKeyClick(System::TObject* Sender);
	void __fastcall cmbCommandKeyPress(System::TObject* Sender, char &Key);
	void __fastcall cmbCommandExit(System::TObject* Sender);
	void __fastcall btnOKClick(System::TObject* Sender);
	void __fastcall FormCreate(System::TObject* Sender);
	
private:
	bool FExtended;
	void __fastcall SetCommand(const Syneditkeycmds::TSynEditorCommand Value);
	void __fastcall SetKeystroke(const Classes::TShortCut Value);
	void __fastcall AddEditorCommand(const AnsiString S);
	Syneditkeycmds::TSynEditorCommand __fastcall GetCommand(void);
	Classes::TShortCut __fastcall GetKeystroke(void);
	Classes::TShortCut __fastcall GetKeystroke2(void);
	void __fastcall SetKeystroke2(const Classes::TShortCut Value);
	
public:
	Syneditmiscclasses::TSynHotKey* hkKeystroke2;
	Syneditmiscclasses::TSynHotKey* hkKeystroke;
	__property Syneditkeycmds::TSynEditorCommand Command = {read=GetCommand, write=SetCommand, nodefault};
	__property Classes::TShortCut Keystroke = {read=GetKeystroke, write=SetKeystroke, nodefault};
	__property Classes::TShortCut Keystroke2 = {read=GetKeystroke2, write=SetKeystroke2, nodefault};
	__property bool ExtendedString = {read=FExtended, write=FExtended, default=1};
public:
	#pragma option push -w-inl
	/* TCustomForm.Create */ inline __fastcall virtual TSynEditKeystrokeEditorForm(Classes::TComponent* AOwner) : Forms::TForm(AOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomForm.CreateNew */ inline __fastcall virtual TSynEditKeystrokeEditorForm(Classes::TComponent* AOwner, int Dummy) : Forms::TForm(AOwner, Dummy) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomForm.Destroy */ inline __fastcall virtual ~TSynEditKeystrokeEditorForm(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSynEditKeystrokeEditorForm(HWND ParentWindow) : Forms::TForm(ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE TSynEditKeystrokeEditorForm* SynEditKeystrokeEditorForm;

}	/* namespace Syneditkeycmdeditor */
using namespace Syneditkeycmdeditor;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditKeyCmdEditor
