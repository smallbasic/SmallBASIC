// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterHC11.pas' rev: 6.00

#ifndef SynHighlighterHC11HPP
#define SynHighlighterHC11HPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditTypes.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
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

namespace Synhighlighterhc11
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkDirective, tkIdentifier, tkKey, tkNull, tkNumber, tkSpace, tkString, tkSymbol, tkUnknown };
#pragma option pop

#pragma option push -b-
enum TkwKeyWordType { kwNone, kwOperand, kwOperandOver, kwNoOperand };
#pragma option pop

struct THashListEntry;
typedef THashListEntry *PHashListEntry;

#pragma pack(push, 4)
struct THashListEntry
{
	THashListEntry *Next;
	AnsiString Token;
	TtkTokenKind Kind;
	bool Op;
} ;
#pragma pack(pop)

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef THashListEntry SynHighlighterHC11__2[156];

class DELPHICLASS TSynHC11Syn;
class PASCALIMPLEMENTATION TSynHC11Syn : public Synedithighlighter::TSynCustomHighlighter 
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
	TtkTokenKind FTokenID;
	TkwKeyWordType FKeyWordType;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDirecAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fInvalidAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	THashListEntry fHashArray[156];
	THashListEntry *fHashList[256];
	int fHashArrayIndex;
	char __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	void __fastcall SymAsciiCharProc(void);
	void __fastcall SymbolProc(void);
	void __fastcall SymDollarProc(void);
	void __fastcall SymCRProc(void);
	void __fastcall SymIdentProc(void);
	void __fastcall SymLFProc(void);
	void __fastcall SymPercentProc(void);
	void __fastcall SymNullProc(void);
	void __fastcall SymNumberProc(void);
	void __fastcall SymSpaceProc(void);
	void __fastcall SymStarProc(void);
	void __fastcall SymStringProc(void);
	void __fastcall SymUnknownProc(void);
	void __fastcall InitIdent(void);
	void __fastcall MakeMethodTables(void);
	void __fastcall AddHashEntry(AnsiString NewToken, TtkTokenKind NewKind);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynHC11Syn(Classes::TComponent* AOwner);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DirecAttri = {read=fDirecAttri, write=fDirecAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* InvalidAttri = {read=fInvalidAttri, write=fInvalidAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynHC11Syn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
static const Byte KeyWordCount = 0x95;
static const Shortint DirectiveCount = 0x6;

}	/* namespace Synhighlighterhc11 */
using namespace Synhighlighterhc11;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterHC11
