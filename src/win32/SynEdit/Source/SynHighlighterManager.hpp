// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterManager.pas' rev: 6.00

#ifndef SynHighlighterManagerHPP
#define SynHighlighterManagerHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlightermanager
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynHighlighterManager;
class PASCALIMPLEMENTATION TSynHighlighterManager : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
public:
	__fastcall virtual TSynHighlighterManager(Classes::TComponent* AOwner);
public:
	#pragma option push -w-inl
	/* TComponent.Destroy */ inline __fastcall virtual ~TSynHighlighterManager(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall Register(void);

}	/* namespace Synhighlightermanager */
using namespace Synhighlightermanager;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterManager
