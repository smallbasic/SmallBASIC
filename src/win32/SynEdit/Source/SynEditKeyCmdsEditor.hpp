// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditKeyCmdsEditor.pas' rev: 6.00

#ifndef SynEditKeyCmdsEditorHPP
#define SynEditKeyCmdsEditorHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditKeyCmds.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <Buttons.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <ComCtrls.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditkeycmdseditor
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynEditKeystrokesEditorForm;
class PASCALIMPLEMENTATION TSynEditKeystrokesEditorForm : public Forms::TForm 
{
	typedef Forms::TForm inherited;
	
__published:
	Extctrls::TPanel* pnlBottom;
	Stdctrls::TLabel* lnlInfo;
	Stdctrls::TLabel* lnlInfo2;
	Buttons::TSpeedButton* btnAdd;
	Buttons::TSpeedButton* btnEdit;
	Buttons::TSpeedButton* btnDelete;
	Buttons::TSpeedButton* btnClear;
	Buttons::TSpeedButton* btnReset;
	Buttons::TSpeedButton* btnOK;
	Buttons::TSpeedButton* btnCancel;
	Extctrls::TPanel* pnlCommands;
	Comctrls::TListView* KeyCmdList;
	void __fastcall FormResize(System::TObject* Sender);
	void __fastcall btnAddClick(System::TObject* Sender);
	void __fastcall btnEditClick(System::TObject* Sender);
	void __fastcall btnDeleteClick(System::TObject* Sender);
	void __fastcall btnResetClick(System::TObject* Sender);
	void __fastcall FormCreate(System::TObject* Sender);
	void __fastcall btnClearClick(System::TObject* Sender);
	void __fastcall btnOKClick(System::TObject* Sender);
	void __fastcall btnCancelClick(System::TObject* Sender);
	void __fastcall KeyCmdListClick(System::TObject* Sender);
	
private:
	Syneditkeycmds::TSynEditKeyStrokes* FKeystrokes;
	bool FExtended;
	void __fastcall SetKeystrokes(const Syneditkeycmds::TSynEditKeyStrokes* Value);
	void __fastcall UpdateKeystrokesList(void);
	HIDESBASE MESSAGE void __fastcall WMGetMinMaxInfo(Messages::TWMGetMinMaxInfo &Msg);
	
public:
	__fastcall virtual TSynEditKeystrokesEditorForm(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynEditKeystrokesEditorForm(void);
	__property Syneditkeycmds::TSynEditKeyStrokes* Keystrokes = {read=FKeystrokes, write=SetKeystrokes};
	__property bool ExtendedString = {read=FExtended, write=FExtended, nodefault};
public:
	#pragma option push -w-inl
	/* TCustomForm.CreateNew */ inline __fastcall virtual TSynEditKeystrokesEditorForm(Classes::TComponent* AOwner, int Dummy) : Forms::TForm(AOwner, Dummy) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSynEditKeystrokesEditorForm(HWND ParentWindow) : Forms::TForm(ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditkeycmdseditor */
using namespace Syneditkeycmdseditor;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditKeyCmdsEditor
