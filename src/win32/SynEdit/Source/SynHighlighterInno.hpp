// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterInno.pas' rev: 6.00

#ifndef SynHighlighterInnoHPP
#define SynHighlighterInnoHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynHighlighterHashEntries.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Registry.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlighterinno
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkConstant, tkIdentifier, tkKey, tkKeyOrParameter, tkNull, tkNumber, tkParameter, tkSection, tkSpace, tkString, tkSymbol, tkUnknown };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

class DELPHICLASS TSynInnoSyn;
class PASCALIMPLEMENTATION TSynInnoSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	char *fLine;
	int fLineNumber;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind fTokenID;
	Synedithighlighter::TSynHighlighterAttributes* fConstantAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSectionAttri;
	Synedithighlighter::TSynHighlighterAttributes* fParamAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fInvalidAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synhighlighterhashentries::TSynHashEntryList* fKeywords;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	void __fastcall SymbolProc(void);
	void __fastcall CRProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall SectionProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall EqualProc(void);
	void __fastcall ConstantProc(void);
	void __fastcall SemiColonProc(void);
	void __fastcall StringProc(void);
	void __fastcall UnknownProc(void);
	void __fastcall DoAddKeyword(AnsiString AKeyword, int AKind);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynInnoSyn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynInnoSyn(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* ConstantAttri = {read=fConstantAttri, write=fConstantAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* InvalidAttri = {read=fInvalidAttri, write=fInvalidAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* ParameterAttri = {read=fParamAttri, write=fParamAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SectionAttri = {read=fSectionAttri, write=fSectionAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlighterinno */
using namespace Synhighlighterinno;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterInno
