// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditPrintTypes.pas' rev: 6.00

#ifndef SynEditPrintTypesHPP
#define SynEditPrintTypesHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditprinttypes
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TFrameType { ftLine, ftBox, ftShaded };
#pragma option pop

typedef Set<TFrameType, ftLine, ftShaded>  TFrameTypes;

#pragma option push -b-
enum TUnitSystem { usMM, usCM, usInch, muThousandthsOfInches };
#pragma option pop

#pragma option push -b-
enum TSynPrintStatus { psBegin, psNewPage, psEnd };
#pragma option pop

typedef void __fastcall (__closure *TPrintStatusEvent)(System::TObject* Sender, TSynPrintStatus Status, int PageNumber, bool &Abort);

typedef void __fastcall (__closure *TPrintLineEvent)(System::TObject* Sender, int LineNumber, int PageNumber);

typedef Set<char, 0, 255>  TSysCharSet;

class DELPHICLASS TWrapPos;
class PASCALIMPLEMENTATION TWrapPos : public System::TObject 
{
	typedef System::TObject inherited;
	
public:
	int Index;
public:
	#pragma option push -w-inl
	/* TObject.Create */ inline __fastcall TWrapPos(void) : System::TObject() { }
	#pragma option pop
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TWrapPos(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
static const Shortint DefLeft = 0x19;
static const Shortint DefRight = 0xf;
static const Shortint DefTop = 0x19;
static const Shortint DefBottom = 0x19;
static const Shortint DefHeader = 0xf;
static const Shortint DefFooter = 0xf;
static const Shortint DefLeftHFTextIndent = 0x2;
static const Shortint DefRightHFTextIndent = 0x2;
#define DefHFInternalMargin  (5.000000E-01)
static const Shortint DefGutter = 0x0;
extern PACKAGE bool __fastcall WrapText(const AnsiString Line, const TSysCharSet &BreakChars, int MaxCol, Classes::TList* AList);
extern PACKAGE AnsiString __fastcall IntToRoman(int Value);

}	/* namespace Syneditprinttypes */
using namespace Syneditprinttypes;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditPrintTypes
