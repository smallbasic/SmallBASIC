// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterMulti.pas' rev: 6.00

#ifndef SynHighlighterMultiHPP
#define SynHighlighterMultiHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlightermulti
{
//-- type declarations -------------------------------------------------------
typedef void __fastcall (__closure *TOnCheckMarker)(System::TObject* Sender, int &StartPos, int &MarkerLen, AnsiString &MarkerText);

class DELPHICLASS TgmScheme;
class PASCALIMPLEMENTATION TgmScheme : public Classes::TCollectionItem 
{
	typedef Classes::TCollectionItem inherited;
	
private:
	AnsiString fEndExpr;
	AnsiString fStartExpr;
	Synedithighlighter::TSynCustomHighlighter* fHighlighter;
	Synedithighlighter::TSynHighlighterAttributes* fMarkerAttri;
	AnsiString fSchemeName;
	bool fCaseSensitive;
	TOnCheckMarker fOnCheckStartMarker;
	TOnCheckMarker fOnCheckEndMarker;
	AnsiString __fastcall ConvertExpression(const AnsiString Value);
	void __fastcall MarkerAttriChanged(System::TObject* Sender);
	void __fastcall SetMarkerAttri(const Synedithighlighter::TSynHighlighterAttributes* Value);
	void __fastcall SetHighlighter(const Synedithighlighter::TSynCustomHighlighter* Value);
	void __fastcall SetEndExpr(const AnsiString Value);
	void __fastcall SetStartExpr(const AnsiString Value);
	void __fastcall SetCaseSensitive(const bool Value);
	
protected:
	virtual AnsiString __fastcall GetDisplayName();
	virtual void __fastcall SetDisplayName(const AnsiString Value);
	
public:
	__fastcall virtual TgmScheme(Classes::TCollection* Collection);
	__fastcall virtual ~TgmScheme(void);
	
__published:
	__property bool CaseSensitive = {read=fCaseSensitive, write=SetCaseSensitive, default=1};
	__property AnsiString StartExpr = {read=fStartExpr, write=SetStartExpr};
	__property AnsiString EndExpr = {read=fEndExpr, write=SetEndExpr};
	__property Synedithighlighter::TSynCustomHighlighter* Highlighter = {read=fHighlighter, write=SetHighlighter};
	__property Synedithighlighter::TSynHighlighterAttributes* MarkerAttri = {read=fMarkerAttri, write=SetMarkerAttri};
	__property AnsiString SchemeName = {read=fSchemeName, write=fSchemeName};
	__property TOnCheckMarker OnCheckStartMarker = {read=fOnCheckStartMarker, write=fOnCheckStartMarker};
	__property TOnCheckMarker OnCheckEndMarker = {read=fOnCheckEndMarker, write=fOnCheckEndMarker};
};


typedef TMetaClass*TgmSchemeClass;

class DELPHICLASS TgmSchemes;
class DELPHICLASS TSynMultiSyn;
class DELPHICLASS TgmMarker;
class PASCALIMPLEMENTATION TgmMarker : public System::TObject 
{
	typedef System::TObject inherited;
	
protected:
	int fScheme;
	int fStartPos;
	int fMarkerLen;
	AnsiString fMarkerText;
	bool fIsOpenMarker;
	
public:
	__fastcall TgmMarker(int aScheme, int aStartPos, int aMarkerLen, bool aIsOpenMarker, const AnsiString aMarkerText);
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TgmMarker(void) { }
	#pragma option pop
	
};


class PASCALIMPLEMENTATION TSynMultiSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	AnsiString fDefaultLanguageName;
	Classes::TList* fMarkers;
	TgmMarker* fMarker;
	int fNextMarker;
	int fCurrScheme;
	AnsiString fTmpLine;
	void *fTmpRange;
	void __fastcall SetDefaultHighlighter(const Synedithighlighter::TSynCustomHighlighter* Value);
	TgmMarker* __fastcall GetMarkers(int aIndex);
	__property TgmMarker* Markers[int aIndex] = {read=GetMarkers};
	void __fastcall DoCheckMarker(TgmScheme* Scheme, int StartPos, int MarkerLen, const AnsiString MarkerText, bool Start);
	
protected:
	TgmSchemes* fSchemes;
	Synedithighlighter::TSynCustomHighlighter* fDefaultHighlighter;
	AnsiString fLine;
	int fLineNumber;
	int fTokenPos;
	int fRun;
	AnsiString fSampleSource;
	void __fastcall SetSchemes(const TgmSchemes* Value);
	void __fastcall ClearMarkers(void);
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual int __fastcall GetAttribCount(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetAttribute(int idx);
	void __fastcall HookHighlighter(Synedithighlighter::TSynCustomHighlighter* aHL);
	void __fastcall UnhookHighlighter(Synedithighlighter::TSynCustomHighlighter* aHL);
	virtual void __fastcall Notification(Classes::TComponent* aComp, Classes::TOperation aOp);
	virtual AnsiString __fastcall GetSampleSource();
	virtual void __fastcall SetSampleSource(AnsiString Value);
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynMultiSyn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynMultiSyn(void);
	virtual bool __fastcall GetEol(void);
	virtual void * __fastcall GetRange(void);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual void __fastcall SetRange(void * Value);
	virtual void __fastcall ResetRange(void);
	__property int CurrScheme = {read=fCurrScheme, write=fCurrScheme, nodefault};
	
__published:
	__property TgmSchemes* Schemes = {read=fSchemes, write=SetSchemes};
	__property Synedithighlighter::TSynCustomHighlighter* DefaultHighlighter = {read=fDefaultHighlighter, write=SetDefaultHighlighter};
	__property AnsiString DefaultLanguageName = {read=fDefaultLanguageName, write=fDefaultLanguageName};
};


class PASCALIMPLEMENTATION TgmSchemes : public Classes::TCollection 
{
	typedef Classes::TCollection inherited;
	
public:
	TgmScheme* operator[](int aIndex) { return Items[aIndex]; }
	
private:
	TSynMultiSyn* fOwner;
	TgmScheme* __fastcall GetItems(int Index);
	void __fastcall SetItems(int Index, const TgmScheme* Value);
	
protected:
	DYNAMIC Classes::TPersistent* __fastcall GetOwner(void);
	virtual void __fastcall Update(Classes::TCollectionItem* Item);
	
public:
	__fastcall TgmSchemes(TSynMultiSyn* aOwner);
	__property TgmScheme* Items[int aIndex] = {read=GetItems, write=SetItems/*, default*/};
public:
	#pragma option push -w-inl
	/* TCollection.Destroy */ inline __fastcall virtual ~TgmSchemes(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
static const Shortint MaxNestedMultiSyn = 0x4;

}	/* namespace Synhighlightermulti */
using namespace Synhighlightermulti;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterMulti
