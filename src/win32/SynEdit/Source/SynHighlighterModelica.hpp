// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterModelica.pas' rev: 6.00

#ifndef SynHighlighterModelicaHPP
#define SynHighlighterModelicaHPP

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

namespace Synhighlightermodelica
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkDirective, tkIdentifier, tkKey, tkNull, tkNumber, tkSpace, tkString, tkSymbol, tkUnknown };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsUnknown, rsString39, rsString34, rsComment };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

class DELPHICLASS TSynModelicaSyn;
class PASCALIMPLEMENTATION TSynModelicaSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TRangeState fRange;
	char *fLine;
	int fLineNumber;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	char *fToIdent;
	TtkTokenKind fTokenID;
	int fTokenPos;
	TIdentFuncTableFunc fIdentFuncTable[138];
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDirectiveAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func17(void);
	TtkTokenKind __fastcall Func22(void);
	TtkTokenKind __fastcall Func25(void);
	TtkTokenKind __fastcall Func26(void);
	TtkTokenKind __fastcall Func30(void);
	TtkTokenKind __fastcall Func35(void);
	TtkTokenKind __fastcall Func39(void);
	TtkTokenKind __fastcall Func42(void);
	TtkTokenKind __fastcall Func45(void);
	TtkTokenKind __fastcall Func47(void);
	TtkTokenKind __fastcall Func48(void);
	TtkTokenKind __fastcall Func51(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func54(void);
	TtkTokenKind __fastcall Func59(void);
	TtkTokenKind __fastcall Func60(void);
	TtkTokenKind __fastcall Func62(void);
	TtkTokenKind __fastcall Func68(void);
	TtkTokenKind __fastcall Func69(void);
	TtkTokenKind __fastcall Func70(void);
	TtkTokenKind __fastcall Func80(void);
	TtkTokenKind __fastcall Func81(void);
	TtkTokenKind __fastcall Func84(void);
	TtkTokenKind __fastcall Func85(void);
	TtkTokenKind __fastcall Func88(void);
	TtkTokenKind __fastcall Func91(void);
	TtkTokenKind __fastcall Func98(void);
	TtkTokenKind __fastcall Func106(void);
	TtkTokenKind __fastcall Func107(void);
	TtkTokenKind __fastcall Func110(void);
	TtkTokenKind __fastcall Func112(void);
	TtkTokenKind __fastcall Func114(void);
	TtkTokenKind __fastcall Func115(void);
	TtkTokenKind __fastcall Func116(void);
	TtkTokenKind __fastcall Func119(void);
	TtkTokenKind __fastcall Func133(void);
	TtkTokenKind __fastcall Func137(void);
	void __fastcall AndSymbolProc(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall CRProc(void);
	void __fastcall ColonProc(void);
	void __fastcall DirectiveProc(void);
	void __fastcall GreaterProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall LowerProc(void);
	void __fastcall MinusProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall OrSymbolProc(void);
	void __fastcall PlusProc(void);
	void __fastcall PointProc(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall SymbolProc(void);
	void __fastcall SymbolProcWithEqual(void);
	void __fastcall UnknownProc(void);
	TtkTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	void __fastcall AnsiCProc(void);
	void __fastcall String34Proc(void);
	void __fastcall String39Proc(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynModelicaSyn(Classes::TComponent* AOwner);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual void * __fastcall GetRange(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual void __fastcall SetRange(void * Value);
	virtual void __fastcall ResetRange(void);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DirectiveAttri = {read=fDirectiveAttri, write=fDirectiveAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynModelicaSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall Register(void);

}	/* namespace Synhighlightermodelica */
using namespace Synhighlightermodelica;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterModelica
