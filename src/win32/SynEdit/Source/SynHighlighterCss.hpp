// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterCss.pas' rev: 6.00

#ifndef SynHighlighterCssHPP
#define SynHighlighterCssHPP

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

namespace Synhighlightercss
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkIdentifier, tkKey, tkNull, tkNumber, tkSpace, tkString, tkSymbol, tkUnknown };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsUnknown, rsCStyle };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

class DELPHICLASS TSynCssSyn;
class PASCALIMPLEMENTATION TSynCssSyn : public Synedithighlighter::TSynCustomHighlighter 
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
	int fTokenPos;
	TtkTokenKind FTokenID;
	TIdentFuncTableFunc fIdentFuncTable[256];
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func16(void);
	TtkTokenKind __fastcall Func18(void);
	TtkTokenKind __fastcall Func19(void);
	TtkTokenKind __fastcall Func23(void);
	TtkTokenKind __fastcall Func24(void);
	TtkTokenKind __fastcall Func26(void);
	TtkTokenKind __fastcall Func29(void);
	TtkTokenKind __fastcall Func30(void);
	TtkTokenKind __fastcall Func32(void);
	TtkTokenKind __fastcall Func34(void);
	TtkTokenKind __fastcall Func36(void);
	TtkTokenKind __fastcall Func39(void);
	TtkTokenKind __fastcall Func40(void);
	TtkTokenKind __fastcall Func43(void);
	TtkTokenKind __fastcall Func45(void);
	TtkTokenKind __fastcall Func51(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func54(void);
	TtkTokenKind __fastcall Func55(void);
	TtkTokenKind __fastcall Func56(void);
	TtkTokenKind __fastcall Func57(void);
	TtkTokenKind __fastcall Func58(void);
	TtkTokenKind __fastcall Func59(void);
	TtkTokenKind __fastcall Func60(void);
	TtkTokenKind __fastcall Func61(void);
	TtkTokenKind __fastcall Func62(void);
	TtkTokenKind __fastcall Func63(void);
	TtkTokenKind __fastcall Func64(void);
	TtkTokenKind __fastcall Func65(void);
	TtkTokenKind __fastcall Func67(void);
	TtkTokenKind __fastcall Func69(void);
	TtkTokenKind __fastcall Func70(void);
	TtkTokenKind __fastcall Func72(void);
	TtkTokenKind __fastcall Func74(void);
	TtkTokenKind __fastcall Func76(void);
	TtkTokenKind __fastcall Func78(void);
	TtkTokenKind __fastcall Func79(void);
	TtkTokenKind __fastcall Func80(void);
	TtkTokenKind __fastcall Func81(void);
	TtkTokenKind __fastcall Func82(void);
	TtkTokenKind __fastcall Func83(void);
	TtkTokenKind __fastcall Func85(void);
	TtkTokenKind __fastcall Func86(void);
	TtkTokenKind __fastcall Func87(void);
	TtkTokenKind __fastcall Func88(void);
	TtkTokenKind __fastcall Func90(void);
	TtkTokenKind __fastcall Func91(void);
	TtkTokenKind __fastcall Func93(void);
	TtkTokenKind __fastcall Func94(void);
	TtkTokenKind __fastcall Func95(void);
	TtkTokenKind __fastcall Func96(void);
	TtkTokenKind __fastcall Func97(void);
	TtkTokenKind __fastcall Func98(void);
	TtkTokenKind __fastcall Func99(void);
	TtkTokenKind __fastcall Func100(void);
	TtkTokenKind __fastcall Func101(void);
	TtkTokenKind __fastcall Func102(void);
	TtkTokenKind __fastcall Func103(void);
	TtkTokenKind __fastcall Func105(void);
	TtkTokenKind __fastcall Func106(void);
	TtkTokenKind __fastcall Func107(void);
	TtkTokenKind __fastcall Func108(void);
	TtkTokenKind __fastcall Func110(void);
	TtkTokenKind __fastcall Func111(void);
	TtkTokenKind __fastcall Func112(void);
	TtkTokenKind __fastcall Func113(void);
	TtkTokenKind __fastcall Func114(void);
	TtkTokenKind __fastcall Func115(void);
	TtkTokenKind __fastcall Func116(void);
	TtkTokenKind __fastcall Func117(void);
	TtkTokenKind __fastcall Func118(void);
	TtkTokenKind __fastcall Func120(void);
	TtkTokenKind __fastcall Func121(void);
	TtkTokenKind __fastcall Func122(void);
	TtkTokenKind __fastcall Func124(void);
	TtkTokenKind __fastcall Func126(void);
	TtkTokenKind __fastcall Func128(void);
	TtkTokenKind __fastcall Func129(void);
	TtkTokenKind __fastcall Func130(void);
	TtkTokenKind __fastcall Func131(void);
	TtkTokenKind __fastcall Func134(void);
	TtkTokenKind __fastcall Func136(void);
	TtkTokenKind __fastcall Func137(void);
	TtkTokenKind __fastcall Func138(void);
	TtkTokenKind __fastcall Func139(void);
	TtkTokenKind __fastcall Func140(void);
	TtkTokenKind __fastcall Func141(void);
	TtkTokenKind __fastcall Func144(void);
	TtkTokenKind __fastcall Func148(void);
	TtkTokenKind __fastcall Func149(void);
	TtkTokenKind __fastcall Func150(void);
	TtkTokenKind __fastcall Func151(void);
	TtkTokenKind __fastcall Func152(void);
	TtkTokenKind __fastcall Func154(void);
	TtkTokenKind __fastcall Func156(void);
	TtkTokenKind __fastcall Func158(void);
	TtkTokenKind __fastcall Func160(void);
	TtkTokenKind __fastcall Func164(void);
	TtkTokenKind __fastcall Func166(void);
	TtkTokenKind __fastcall Func167(void);
	TtkTokenKind __fastcall Func169(void);
	TtkTokenKind __fastcall Func172(void);
	TtkTokenKind __fastcall Func173(void);
	TtkTokenKind __fastcall Func174(void);
	TtkTokenKind __fastcall Func178(void);
	TtkTokenKind __fastcall Func182(void);
	TtkTokenKind __fastcall Func187(void);
	TtkTokenKind __fastcall Func190(void);
	TtkTokenKind __fastcall Func194(void);
	TtkTokenKind __fastcall Func195(void);
	TtkTokenKind __fastcall Func199(void);
	TtkTokenKind __fastcall Func200(void);
	TtkTokenKind __fastcall Func210(void);
	TtkTokenKind __fastcall Func213(void);
	TtkTokenKind __fastcall Func220(void);
	TtkTokenKind __fastcall Func250(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall CRProc(void);
	void __fastcall CStyleCommentProc(void);
	void __fastcall DashProc(void);
	void __fastcall IdentProc(void);
	void __fastcall IntegerProc(void);
	void __fastcall LFProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall RoundOpenProc(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall UnknownProc(void);
	TtkTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynCssSyn(Classes::TComponent* AOwner);
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
	virtual void __fastcall ReSetRange(void);
	__property IdentChars ;
	int __fastcall KeyHash2(char * ToHash);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynCssSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlightercss */
using namespace Synhighlightercss;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterCss
