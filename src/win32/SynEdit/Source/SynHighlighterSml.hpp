// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterSml.pas' rev: 6.00

#ifndef SynHighlighterSmlHPP
#define SynHighlighterSmlHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
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

namespace Synhighlightersml
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkCharacter, tkComment, tkIdentifier, tkKey, tkNull, tkNumber, tkOperator, tkSpace, tkString, tkSymbol, tkSyntaxError, tkUnknown };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

class DELPHICLASS TSynSMLSyn;
class PASCALIMPLEMENTATION TSynSMLSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	bool fBasis;
	char *fLine;
	int fLineNumber;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind FTokenID;
	TIdentFuncTableFunc fIdentFuncTable[146];
	Synedithighlighter::TSynHighlighterAttributes* fCharacterAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fOperatorAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSyntaxErrorAttri;
	bool __fastcall IsValidMLCharacter(char Ch);
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func15(void);
	TtkTokenKind __fastcall Func19(void);
	TtkTokenKind __fastcall Func20(void);
	TtkTokenKind __fastcall Func21(void);
	TtkTokenKind __fastcall Func23(void);
	TtkTokenKind __fastcall Func26(void);
	TtkTokenKind __fastcall Func28(void);
	TtkTokenKind __fastcall Func31(void);
	TtkTokenKind __fastcall Func35(void);
	TtkTokenKind __fastcall Func37(void);
	TtkTokenKind __fastcall Func41(void);
	TtkTokenKind __fastcall Func43(void);
	TtkTokenKind __fastcall Func44(void);
	TtkTokenKind __fastcall Func47(void);
	TtkTokenKind __fastcall Func50(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func57(void);
	TtkTokenKind __fastcall Func59(void);
	TtkTokenKind __fastcall Func60(void);
	TtkTokenKind __fastcall Func62(void);
	TtkTokenKind __fastcall Func66(void);
	TtkTokenKind __fastcall Func68(void);
	TtkTokenKind __fastcall Func74(void);
	TtkTokenKind __fastcall Func76(void);
	TtkTokenKind __fastcall Func80(void);
	TtkTokenKind __fastcall Func82(void);
	TtkTokenKind __fastcall Func88(void);
	TtkTokenKind __fastcall Func92(void);
	TtkTokenKind __fastcall Func97(void);
	TtkTokenKind __fastcall Func101(void);
	TtkTokenKind __fastcall Func111(void);
	TtkTokenKind __fastcall Func114(void);
	TtkTokenKind __fastcall Func126(void);
	TtkTokenKind __fastcall Func145(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall CRProc(void);
	void __fastcall CharacterProc(void);
	void __fastcall ColonProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall OperatorProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall SymbolProc(void);
	void __fastcall UnknownProc(void);
	void __fastcall BasisOpProc(void);
	void __fastcall RoundBracketOpen(void);
	TtkTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynSMLSyn(Classes::TComponent* AOwner);
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
	__property Synedithighlighter::TSynHighlighterAttributes* CharacterAttri = {read=fCharacterAttri, write=fCharacterAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* OperatorAttri = {read=fOperatorAttri, write=fOperatorAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SyntaxErrorAttri = {read=fSyntaxErrorAttri, write=fSyntaxErrorAttri};
	__property bool Basis = {read=fBasis, write=fBasis, default=1};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynSMLSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlightersml */
using namespace Synhighlightersml;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterSml
