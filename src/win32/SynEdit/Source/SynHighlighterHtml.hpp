// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterHtml.pas' rev: 6.00

#ifndef SynHighlighterHtmlHPP
#define SynHighlighterHtmlHPP

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

namespace Synhighlighterhtml
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkAmpersand, tkASP, tkComment, tkIdentifier, tkKey, tkNull, tkSpace, tkString, tkSymbol, tkText, tkUndefKey, tkValue };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsAmpersand, rsASP, rsComment, rsKey, rsParam, rsText, rsUnKnown, rsValue };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

class DELPHICLASS TSynHTMLSyn;
class PASCALIMPLEMENTATION TSynHTMLSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	int fAndCode;
	TRangeState fRange;
	char *fLine;
	TProcTableProc fProcTable[256];
	int Run;
	char *Temp;
	int fStringLen;
	char *fToIdent;
	TIdentFuncTableFunc fIdentFuncTable[244];
	int fTokenPos;
	TtkTokenKind fTokenID;
	Synedithighlighter::TSynHighlighterAttributes* fAndAttri;
	Synedithighlighter::TSynHighlighterAttributes* fASPAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fTextAttri;
	Synedithighlighter::TSynHighlighterAttributes* fUndefKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fValueAttri;
	int fLineNumber;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func1(void);
	TtkTokenKind __fastcall Func2(void);
	TtkTokenKind __fastcall Func8(void);
	TtkTokenKind __fastcall Func9(void);
	TtkTokenKind __fastcall Func10(void);
	TtkTokenKind __fastcall Func11(void);
	TtkTokenKind __fastcall Func12(void);
	TtkTokenKind __fastcall Func13(void);
	TtkTokenKind __fastcall Func14(void);
	TtkTokenKind __fastcall Func16(void);
	TtkTokenKind __fastcall Func17(void);
	TtkTokenKind __fastcall Func18(void);
	TtkTokenKind __fastcall Func19(void);
	TtkTokenKind __fastcall Func20(void);
	TtkTokenKind __fastcall Func21(void);
	TtkTokenKind __fastcall Func23(void);
	TtkTokenKind __fastcall Func24(void);
	TtkTokenKind __fastcall Func25(void);
	TtkTokenKind __fastcall Func26(void);
	TtkTokenKind __fastcall Func27(void);
	TtkTokenKind __fastcall Func28(void);
	TtkTokenKind __fastcall Func29(void);
	TtkTokenKind __fastcall Func30(void);
	TtkTokenKind __fastcall Func31(void);
	TtkTokenKind __fastcall Func32(void);
	TtkTokenKind __fastcall Func33(void);
	TtkTokenKind __fastcall Func35(void);
	TtkTokenKind __fastcall Func37(void);
	TtkTokenKind __fastcall Func38(void);
	TtkTokenKind __fastcall Func39(void);
	TtkTokenKind __fastcall Func40(void);
	TtkTokenKind __fastcall Func41(void);
	TtkTokenKind __fastcall Func42(void);
	TtkTokenKind __fastcall Func43(void);
	TtkTokenKind __fastcall Func46(void);
	TtkTokenKind __fastcall Func47(void);
	TtkTokenKind __fastcall Func48(void);
	TtkTokenKind __fastcall Func49(void);
	TtkTokenKind __fastcall Func50(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func53(void);
	TtkTokenKind __fastcall Func55(void);
	TtkTokenKind __fastcall Func56(void);
	TtkTokenKind __fastcall Func57(void);
	TtkTokenKind __fastcall Func58(void);
	TtkTokenKind __fastcall Func61(void);
	TtkTokenKind __fastcall Func62(void);
	TtkTokenKind __fastcall Func64(void);
	TtkTokenKind __fastcall Func65(void);
	TtkTokenKind __fastcall Func66(void);
	TtkTokenKind __fastcall Func67(void);
	TtkTokenKind __fastcall Func70(void);
	TtkTokenKind __fastcall Func76(void);
	TtkTokenKind __fastcall Func78(void);
	TtkTokenKind __fastcall Func80(void);
	TtkTokenKind __fastcall Func81(void);
	TtkTokenKind __fastcall Func82(void);
	TtkTokenKind __fastcall Func83(void);
	TtkTokenKind __fastcall Func84(void);
	TtkTokenKind __fastcall Func85(void);
	TtkTokenKind __fastcall Func87(void);
	TtkTokenKind __fastcall Func89(void);
	TtkTokenKind __fastcall Func90(void);
	TtkTokenKind __fastcall Func91(void);
	TtkTokenKind __fastcall Func92(void);
	TtkTokenKind __fastcall Func93(void);
	TtkTokenKind __fastcall Func94(void);
	TtkTokenKind __fastcall Func105(void);
	TtkTokenKind __fastcall Func107(void);
	TtkTokenKind __fastcall Func114(void);
	TtkTokenKind __fastcall Func121(void);
	TtkTokenKind __fastcall Func123(void);
	TtkTokenKind __fastcall Func124(void);
	TtkTokenKind __fastcall Func130(void);
	TtkTokenKind __fastcall Func131(void);
	TtkTokenKind __fastcall Func132(void);
	TtkTokenKind __fastcall Func133(void);
	TtkTokenKind __fastcall Func134(void);
	TtkTokenKind __fastcall Func135(void);
	TtkTokenKind __fastcall Func136(void);
	TtkTokenKind __fastcall Func138(void);
	TtkTokenKind __fastcall Func139(void);
	TtkTokenKind __fastcall Func140(void);
	TtkTokenKind __fastcall Func141(void);
	TtkTokenKind __fastcall Func143(void);
	TtkTokenKind __fastcall Func145(void);
	TtkTokenKind __fastcall Func146(void);
	TtkTokenKind __fastcall Func149(void);
	TtkTokenKind __fastcall Func150(void);
	TtkTokenKind __fastcall Func151(void);
	TtkTokenKind __fastcall Func152(void);
	TtkTokenKind __fastcall Func153(void);
	TtkTokenKind __fastcall Func154(void);
	TtkTokenKind __fastcall Func155(void);
	TtkTokenKind __fastcall Func157(void);
	TtkTokenKind __fastcall Func159(void);
	TtkTokenKind __fastcall Func160(void);
	TtkTokenKind __fastcall Func161(void);
	TtkTokenKind __fastcall Func162(void);
	TtkTokenKind __fastcall Func163(void);
	TtkTokenKind __fastcall Func164(void);
	TtkTokenKind __fastcall Func168(void);
	TtkTokenKind __fastcall Func169(void);
	TtkTokenKind __fastcall Func170(void);
	TtkTokenKind __fastcall Func171(void);
	TtkTokenKind __fastcall Func172(void);
	TtkTokenKind __fastcall Func174(void);
	TtkTokenKind __fastcall Func175(void);
	TtkTokenKind __fastcall Func177(void);
	TtkTokenKind __fastcall Func178(void);
	TtkTokenKind __fastcall Func179(void);
	TtkTokenKind __fastcall Func180(void);
	TtkTokenKind __fastcall Func183(void);
	TtkTokenKind __fastcall Func186(void);
	TtkTokenKind __fastcall Func187(void);
	TtkTokenKind __fastcall Func188(void);
	TtkTokenKind __fastcall Func192(void);
	TtkTokenKind __fastcall Func198(void);
	TtkTokenKind __fastcall Func200(void);
	TtkTokenKind __fastcall Func202(void);
	TtkTokenKind __fastcall Func203(void);
	TtkTokenKind __fastcall Func204(void);
	TtkTokenKind __fastcall Func205(void);
	TtkTokenKind __fastcall Func207(void);
	TtkTokenKind __fastcall Func209(void);
	TtkTokenKind __fastcall Func211(void);
	TtkTokenKind __fastcall Func212(void);
	TtkTokenKind __fastcall Func213(void);
	TtkTokenKind __fastcall Func214(void);
	TtkTokenKind __fastcall Func215(void);
	TtkTokenKind __fastcall Func216(void);
	TtkTokenKind __fastcall Func227(void);
	TtkTokenKind __fastcall Func229(void);
	TtkTokenKind __fastcall Func236(void);
	TtkTokenKind __fastcall Func243(void);
	TtkTokenKind __fastcall AltFunc(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall InitIdent(void);
	void __fastcall MakeMethodTables(void);
	void __fastcall ASPProc(void);
	void __fastcall TextProc(void);
	void __fastcall CommentProc(void);
	void __fastcall BraceCloseProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall CRProc(void);
	void __fastcall EqualProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall NullProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall AmpersandProc(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynHTMLSyn(Classes::TComponent* AOwner);
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
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* AndAttri = {read=fAndAttri, write=fAndAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* ASPAttri = {read=fASPAttri, write=fASPAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* TextAttri = {read=fTextAttri, write=fTextAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* UndefKeyAttri = {read=fUndefKeyAttri, write=fUndefKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* ValueAttri = {read=fValueAttri, write=fValueAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynHTMLSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
static const Byte MAX_ESCAPEAMPS = 0xf9;
extern PACKAGE char *EscapeAmps[249];

}	/* namespace Synhighlighterhtml */
using namespace Synhighlighterhtml;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterHtml
