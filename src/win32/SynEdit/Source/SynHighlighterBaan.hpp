// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterBaan.pas' rev: 6.00

#ifndef SynHighlighterBaanHPP
#define SynHighlighterBaanHPP

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

namespace Synhighlighterbaan
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkDirective, tkIdentifier, tkKey, tkNull, tkNumber, tkSpace, tkString, tkSymbol, tkUnknown, tkVariable };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

class DELPHICLASS TSynBaanSyn;
class PASCALIMPLEMENTATION TSynBaanSyn : public Synedithighlighter::TSynCustomHighlighter 
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
	TIdentFuncTableFunc fIdentFuncTable[184];
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDirectiveAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fVariableAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func15(void);
	TtkTokenKind __fastcall Func19(void);
	TtkTokenKind __fastcall Func27(void);
	TtkTokenKind __fastcall Func28(void);
	TtkTokenKind __fastcall Func30(void);
	TtkTokenKind __fastcall Func35(void);
	TtkTokenKind __fastcall Func37(void);
	TtkTokenKind __fastcall Func38(void);
	TtkTokenKind __fastcall Func39(void);
	TtkTokenKind __fastcall Func40(void);
	TtkTokenKind __fastcall Func41(void);
	TtkTokenKind __fastcall Func42(void);
	TtkTokenKind __fastcall Func43(void);
	TtkTokenKind __fastcall Func44(void);
	TtkTokenKind __fastcall Func47(void);
	TtkTokenKind __fastcall Func48(void);
	TtkTokenKind __fastcall Func50(void);
	TtkTokenKind __fastcall Func51(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func53(void);
	TtkTokenKind __fastcall Func54(void);
	TtkTokenKind __fastcall Func56(void);
	TtkTokenKind __fastcall Func57(void);
	TtkTokenKind __fastcall Func58(void);
	TtkTokenKind __fastcall Func59(void);
	TtkTokenKind __fastcall Func63(void);
	TtkTokenKind __fastcall Func64(void);
	TtkTokenKind __fastcall Func69(void);
	TtkTokenKind __fastcall Func70(void);
	TtkTokenKind __fastcall Func71(void);
	TtkTokenKind __fastcall Func72(void);
	TtkTokenKind __fastcall Func73(void);
	TtkTokenKind __fastcall Func74(void);
	TtkTokenKind __fastcall Func77(void);
	TtkTokenKind __fastcall Func78(void);
	TtkTokenKind __fastcall Func79(void);
	TtkTokenKind __fastcall Func80(void);
	TtkTokenKind __fastcall Func81(void);
	TtkTokenKind __fastcall Func82(void);
	TtkTokenKind __fastcall Func83(void);
	TtkTokenKind __fastcall Func84(void);
	TtkTokenKind __fastcall Func86(void);
	TtkTokenKind __fastcall Func87(void);
	TtkTokenKind __fastcall Func89(void);
	TtkTokenKind __fastcall Func91(void);
	TtkTokenKind __fastcall Func92(void);
	TtkTokenKind __fastcall Func93(void);
	TtkTokenKind __fastcall Func96(void);
	TtkTokenKind __fastcall Func98(void);
	TtkTokenKind __fastcall Func99(void);
	TtkTokenKind __fastcall Func100(void);
	TtkTokenKind __fastcall Func101(void);
	TtkTokenKind __fastcall Func102(void);
	TtkTokenKind __fastcall Func103(void);
	TtkTokenKind __fastcall Func104(void);
	TtkTokenKind __fastcall Func105(void);
	TtkTokenKind __fastcall Func106(void);
	TtkTokenKind __fastcall Func107(void);
	TtkTokenKind __fastcall Func108(void);
	TtkTokenKind __fastcall Func114(void);
	TtkTokenKind __fastcall Func117(void);
	TtkTokenKind __fastcall Func118(void);
	TtkTokenKind __fastcall Func122(void);
	TtkTokenKind __fastcall Func123(void);
	TtkTokenKind __fastcall Func124(void);
	TtkTokenKind __fastcall Func125(void);
	TtkTokenKind __fastcall Func126(void);
	TtkTokenKind __fastcall Func129(void);
	TtkTokenKind __fastcall Func133(void);
	TtkTokenKind __fastcall Func135(void);
	TtkTokenKind __fastcall Func141(void);
	TtkTokenKind __fastcall Func145(void);
	TtkTokenKind __fastcall Func153(void);
	TtkTokenKind __fastcall Func154(void);
	TtkTokenKind __fastcall Func160(void);
	TtkTokenKind __fastcall Func183(void);
	void __fastcall AndSymbolProc(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall AtSymbolProc(void);
	void __fastcall BraceCloseProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall CRProc(void);
	void __fastcall ColonProc(void);
	void __fastcall CommaProc(void);
	void __fastcall DirectiveProc(void);
	void __fastcall EqualProc(void);
	void __fastcall ErectProc(void);
	void __fastcall GreaterProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall LowerProc(void);
	void __fastcall MinusProc(void);
	void __fastcall ModSymbolProc(void);
	void __fastcall NotSymbolProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall PlusProc(void);
	void __fastcall RoundCloseProc(void);
	void __fastcall RoundOpenProc(void);
	void __fastcall SemiColonProc(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall SquareCloseProc(void);
	void __fastcall SquareOpenProc(void);
	void __fastcall StarProc(void);
	void __fastcall StringProc(void);
	void __fastcall TildeProc(void);
	void __fastcall XOrSymbolProc(void);
	void __fastcall UnknownProc(void);
	TtkTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynBaanSyn(Classes::TComponent* AOwner);
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
	__property Synedithighlighter::TSynHighlighterAttributes* DirectiveAttri = {read=fDirectiveAttri, write=fDirectiveAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* VariableAttri = {read=fVariableAttri, write=fVariableAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynBaanSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall Register(void);

}	/* namespace Synhighlighterbaan */
using namespace Synhighlighterbaan;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterBaan
