// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditSearch.pas' rev: 6.00

#ifndef SynEditSearchHPP
#define SynEditSearchHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditsearch
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynEditSearch;
typedef void __fastcall (__closure *TSynEditSearchOverride)(TSynEditSearch* &ASynEditSearch);

class PASCALIMPLEMENTATION TSynEditSearch : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	char *Run;
	char *Origin;
	char *TheEnd;
	AnsiString Pat;
	int fCount;
	int fTextLen;
	int Look_At;
	int PatLen;
	int PatLenPlus;
	int Shift[256];
	bool fSensitive;
	bool fWhole;
	Classes::TList* fResults;
	bool fShiftInitialized;
	bool __fastcall GetFinished(void);
	int __fastcall GetResult(int Index);
	int __fastcall GetResultCount(void);
	void __fastcall InitShiftTable(void);
	void __fastcall SetPattern(const AnsiString Value);
	void __fastcall SetSensitive(const bool Value);
	
protected:
	virtual bool __fastcall TestWholeWord(void);
	
public:
	__fastcall TSynEditSearch(void);
	__fastcall virtual ~TSynEditSearch(void);
	int __fastcall FindAll(const AnsiString NewText);
	int __fastcall FindFirst(const AnsiString NewText);
	void __fastcall FixResults(int First, int Delta);
	int __fastcall Next(void);
	__property int Count = {read=fCount, write=fCount, nodefault};
	__property bool Finished = {read=GetFinished, nodefault};
	__property AnsiString Pattern = {read=Pat, write=SetPattern};
	__property int Results[int Index] = {read=GetResult};
	__property int ResultCount = {read=GetResultCount, nodefault};
	__property bool Sensitive = {read=fSensitive, write=SetSensitive, nodefault};
	__property bool Whole = {read=fWhole, write=fWhole, nodefault};
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall MakeCompTable(bool Sensitive);
extern PACKAGE void __fastcall MakeDelimiterTable(void);

}	/* namespace Syneditsearch */
using namespace Syneditsearch;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditSearch
