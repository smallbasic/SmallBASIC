// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynAutoCorrectEditor.pas' rev: 6.00

#ifndef SynAutoCorrectEditorHPP
#define SynAutoCorrectEditorHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynAutoCorrect.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Registry.hpp>	// Pascal unit
#include <Buttons.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synautocorrecteditor
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TfrmAutoCorrectEditor;
class PASCALIMPLEMENTATION TfrmAutoCorrectEditor : public Forms::TForm 
{
	typedef Forms::TForm inherited;
	
__published:
	Stdctrls::TLabel* lblLabel1;
	Stdctrls::TLabel* lblLabel2;
	Stdctrls::TListBox* lbxItems;
	Extctrls::TPanel* pnlSeparator;
	Buttons::TSpeedButton* btnAdd;
	Buttons::TSpeedButton* btnDelete;
	Buttons::TSpeedButton* btnClear;
	Buttons::TSpeedButton* btnEdit;
	Buttons::TSpeedButton* btnExit;
	Extctrls::TBevel* bvlSeparator;
	void __fastcall FormShow(System::TObject* Sender);
	void __fastcall lbxItemsDrawItem(Controls::TWinControl* Control, int Index, const Types::TRect &Rect, Windows::TOwnerDrawState State);
	void __fastcall btnAddClick(System::TObject* Sender);
	void __fastcall btnDeleteClick(System::TObject* Sender);
	void __fastcall btnEditClick(System::TObject* Sender);
	void __fastcall btnExitClick(System::TObject* Sender);
	void __fastcall btnClearClick(System::TObject* Sender);
	void __fastcall lbxItemsClick(System::TObject* Sender);
	void __fastcall FormResize(System::TObject* Sender);
	void __fastcall FormDestroy(System::TObject* Sender);
	
public:
	Synautocorrect::TSynAutoCorrect* SynAutoCorrect;
public:
	#pragma option push -w-inl
	/* TCustomForm.Create */ inline __fastcall virtual TfrmAutoCorrectEditor(Classes::TComponent* AOwner) : Forms::TForm(AOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomForm.CreateNew */ inline __fastcall virtual TfrmAutoCorrectEditor(Classes::TComponent* AOwner, int Dummy) : Forms::TForm(AOwner, Dummy) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomForm.Destroy */ inline __fastcall virtual ~TfrmAutoCorrectEditor(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TfrmAutoCorrectEditor(HWND ParentWindow) : Forms::TForm(ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE TfrmAutoCorrectEditor* frmAutoCorrectEditor;
extern PACKAGE Registry::TRegIniFile* Reg;

}	/* namespace Synautocorrecteditor */
using namespace Synautocorrecteditor;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynAutoCorrectEditor
