// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditPrintMarginsDialog.pas' rev: 6.00

#ifndef SynEditPrintMarginsDialogHPP
#define SynEditPrintMarginsDialogHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditPrintMargins.hpp>	// Pascal unit
#include <SynEditPrintTypes.hpp>	// Pascal unit
#include <SynEditPrint.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <Buttons.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditprintmarginsdialog
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynEditPrintMarginsDlg;
class PASCALIMPLEMENTATION TSynEditPrintMarginsDlg : public Forms::TForm 
{
	typedef Forms::TForm inherited;
	
__published:
	Stdctrls::TButton* OKBtn;
	Stdctrls::TButton* CancelBtn;
	Extctrls::TImage* Image1;
	Stdctrls::TLabel* Label1;
	Stdctrls::TLabel* Label2;
	Stdctrls::TLabel* Label3;
	Stdctrls::TLabel* Label4;
	Stdctrls::TLabel* Label5;
	Stdctrls::TLabel* Label6;
	Stdctrls::TLabel* Label7;
	Stdctrls::TLabel* Label8;
	Stdctrls::TLabel* Label9;
	Stdctrls::TCheckBox* CBMirrorMargins;
	Stdctrls::TLabel* Label10;
	Stdctrls::TLabel* Label11;
	Stdctrls::TEdit* EditLeft;
	Stdctrls::TEdit* EditRight;
	Stdctrls::TEdit* EditTop;
	Stdctrls::TEdit* EditBottom;
	Stdctrls::TEdit* EditGutter;
	Stdctrls::TEdit* EditHeader;
	Stdctrls::TEdit* EditFooter;
	Stdctrls::TEdit* EditHFInternalMargin;
	Stdctrls::TEdit* EditLeftHFTextIndent;
	Stdctrls::TEdit* EditRightHFTextIndent;
	Stdctrls::TComboBox* CBUnits;
	void __fastcall FormCreate(System::TObject* Sender);
	void __fastcall FormDestroy(System::TObject* Sender);
	void __fastcall CBUnitsChange(System::TObject* Sender);
	
private:
	Syneditprintmargins::TSynEditPrintMargins* FMargins;
	bool FInternalCall;
	
public:
	void __fastcall SetMargins(Syneditprintmargins::TSynEditPrintMargins* SynEditMargins);
	void __fastcall GetMargins(Syneditprintmargins::TSynEditPrintMargins* SynEditMargins);
public:
	#pragma option push -w-inl
	/* TCustomForm.Create */ inline __fastcall virtual TSynEditPrintMarginsDlg(Classes::TComponent* AOwner) : Forms::TForm(AOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomForm.CreateNew */ inline __fastcall virtual TSynEditPrintMarginsDlg(Classes::TComponent* AOwner, int Dummy) : Forms::TForm(AOwner, Dummy) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomForm.Destroy */ inline __fastcall virtual ~TSynEditPrintMarginsDlg(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSynEditPrintMarginsDlg(HWND ParentWindow) : Forms::TForm(ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditprintmarginsdialog */
using namespace Syneditprintmarginsdialog;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditPrintMarginsDialog
