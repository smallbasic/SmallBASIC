// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditKbdHandler.pas' rev: 6.00

#ifndef SynEditKbdHandlerHPP
#define SynEditKbdHandlerHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditkbdhandler
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TKeyboardControl;
class PASCALIMPLEMENTATION TKeyboardControl : public Controls::TWinControl 
{
	typedef Controls::TWinControl inherited;
	
public:
	__property OnKeyDown ;
	__property OnKeyPress ;
public:
	#pragma option push -w-inl
	/* TWinControl.Create */ inline __fastcall virtual TKeyboardControl(Classes::TComponent* AOwner) : Controls::TWinControl(AOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TKeyboardControl(HWND ParentWindow) : Controls::TWinControl(ParentWindow) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TWinControl.Destroy */ inline __fastcall virtual ~TKeyboardControl(void) { }
	#pragma option pop
	
};


class DELPHICLASS TKeyDownProc;
class PASCALIMPLEMENTATION TKeyDownProc : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	Controls::TKeyEvent fKeyDownProc;
	
public:
	__fastcall TKeyDownProc(Controls::TKeyEvent aKeyDownProc);
	__property Controls::TKeyEvent OnKeyDown = {read=fKeyDownProc, write=fKeyDownProc};
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TKeyDownProc(void) { }
	#pragma option pop
	
};


class DELPHICLASS TKeyPressProc;
class PASCALIMPLEMENTATION TKeyPressProc : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	Controls::TKeyPressEvent fKeyPressProc;
	
public:
	__fastcall TKeyPressProc(Controls::TKeyPressEvent aKeyPressProc);
	__property Controls::TKeyPressEvent OnKeyPress = {read=fKeyPressProc, write=fKeyPressProc};
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TKeyPressProc(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynEditKbdHandler;
class PASCALIMPLEMENTATION TSynEditKbdHandler : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	TKeyboardControl* fControl;
	Classes::TList* fKeyPressChain;
	Controls::TKeyPressEvent fOldKeyPress;
	bool fInKeyPress;
	Classes::TList* fKeyDownChain;
	Controls::TKeyEvent fOldKeyDown;
	bool fInKeyDown;
	void __fastcall SetOnKeyPress(const Controls::TKeyPressEvent Value);
	void __fastcall SetOnKeyDown(const Controls::TKeyEvent Value);
	
protected:
	void __fastcall EditorKeyPress(System::TObject* Sender, char &Key);
	void __fastcall EditorKeyDown(System::TObject* Sender, Word &Key, Classes::TShiftState Shift);
	
public:
	__fastcall TSynEditKbdHandler(Controls::TWinControl* aControl);
	__fastcall virtual ~TSynEditKbdHandler(void);
	void __fastcall ExecuteKeyPress(System::TObject* Sender, char &Key);
	void __fastcall ExecuteKeyDown(System::TObject* Sender, Word &Key, Classes::TShiftState Shift);
	void __fastcall AddKeyDownHandler(TKeyDownProc* aHandler);
	void __fastcall RemoveKeyDownHandler(TKeyDownProc* aHandler);
	void __fastcall AddKeyPressHandler(TKeyPressProc* aHandler);
	void __fastcall RemoveKeyPressHandler(TKeyPressProc* aHandler);
	__property Controls::TKeyPressEvent OnKeyPress = {read=fOldKeyPress, write=SetOnKeyPress};
	__property Controls::TKeyEvent OnKeyDown = {read=fOldKeyDown, write=SetOnKeyDown};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditkbdhandler */
using namespace Syneditkbdhandler;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditKbdHandler
