// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditPythonBehaviour.pas' rev: 6.00

#ifndef SynEditPythonBehaviourHPP
#define SynEditPythonBehaviourHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditKeyCmds.hpp>	// Pascal unit
#include <SynEdit.hpp>	// Pascal unit
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

namespace Syneditpythonbehaviour
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynEditPythonBehaviour;
class PASCALIMPLEMENTATION TSynEditPythonBehaviour : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	Synedit::TSynEdit* FEditor;
	Controls::TKeyPressEvent FFormerKeyPress;
	Synedit::TProcessCommandEvent FFormProcessUserCommand;
	int fIndent;
	
protected:
	virtual void __fastcall SetEditor(Synedit::TSynEdit* Value);
	virtual void __fastcall doKeyPress(System::TObject* Sender, char &Key);
	virtual void __fastcall doProcessUserCommand(System::TObject* Sender, Syneditkeycmds::TSynEditorCommand &Command, char &AChar, void * Data);
	
public:
	virtual void __fastcall Loaded(void);
	void __fastcall AttachFormerEvents(void);
	__fastcall virtual TSynEditPythonBehaviour(Classes::TComponent* aOwner);
	virtual void __fastcall Notification(Classes::TComponent* AComponent, Classes::TOperation Operation);
	
__published:
	__property Synedit::TSynEdit* Editor = {read=FEditor, write=SetEditor};
	__property int Indent = {read=fIndent, write=fIndent, default=4};
public:
	#pragma option push -w-inl
	/* TComponent.Destroy */ inline __fastcall virtual ~TSynEditPythonBehaviour(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
static const Word ecPythonIndent = 0xb9f;

}	/* namespace Syneditpythonbehaviour */
using namespace Syneditpythonbehaviour;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditPythonBehaviour
