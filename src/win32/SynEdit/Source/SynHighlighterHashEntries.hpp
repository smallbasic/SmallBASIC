// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterHashEntries.pas' rev: 6.00

#ifndef SynHighlighterHashEntriesHPP
#define SynHighlighterHashEntriesHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlighterhashentries
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynHashEntry;
class PASCALIMPLEMENTATION TSynHashEntry : public System::TObject 
{
	typedef System::TObject inherited;
	
protected:
	TSynHashEntry* fNext;
	int fKeyLen;
	AnsiString fKeyword;
	int fKind;
	
public:
	virtual TSynHashEntry* __fastcall AddEntry(TSynHashEntry* NewEntry);
	__fastcall TSynHashEntry(const AnsiString AKey, int AKind);
	__fastcall virtual ~TSynHashEntry(void);
	__property AnsiString Keyword = {read=fKeyword};
	__property int KeywordLen = {read=fKeyLen, nodefault};
	__property int Kind = {read=fKind, nodefault};
	__property TSynHashEntry* Next = {read=fNext};
};


class DELPHICLASS TSynHashEntryList;
class PASCALIMPLEMENTATION TSynHashEntryList : public Classes::TList 
{
	typedef Classes::TList inherited;
	
public:
	TSynHashEntry* operator[](int Index) { return Items[Index]; }
	
protected:
	HIDESBASE TSynHashEntry* __fastcall Get(int HashKey);
	HIDESBASE void __fastcall Put(int HashKey, TSynHashEntry* Entry);
	
public:
	virtual void __fastcall Clear(void);
	__property TSynHashEntry* Items[int Index] = {read=Get, write=Put/*, default*/};
public:
	#pragma option push -w-inl
	/* TList.Destroy */ inline __fastcall virtual ~TSynHashEntryList(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Create */ inline __fastcall TSynHashEntryList(void) : Classes::TList() { }
	#pragma option pop
	
};


typedef void __fastcall (__closure *TEnumerateKeywordEvent)(AnsiString AKeyword, int AKind);

//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall EnumerateKeywords(int AKind, AnsiString KeywordList, const Synedittypes::TSynIdentChars &Identifiers, TEnumerateKeywordEvent AKeywordProc);

}	/* namespace Synhighlighterhashentries */
using namespace Synhighlighterhashentries;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterHashEntries
