// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterDml.pas' rev: 6.00

#ifndef SynHighlighterDmlHPP
#define SynHighlighterDmlHPP

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

namespace Synhighlighterdml
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkBlock, tkComment, tkForm, tkFunction, tkIdentifier, tkKey, tkNull, tkNumber, tkQualifier, tkSpace, tkSpecial, tkString, tkSymbol, tkUnknown, tkVariable };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsANil, rsAdd, rsFind, rsUnKnown };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

class DELPHICLASS TSynDmlSyn;
class PASCALIMPLEMENTATION TSynDmlSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TRangeState fRange;
	char *fLine;
	int fLineNumber;
	TProcTableProc fProcTable[256];
	int Run;
	char *Temp;
	int fStringLen;
	char *fToIdent;
	TIdentFuncTableFunc fIdentFuncTable[328];
	int fTokenPos;
	TtkTokenKind FTokenID;
	Synedithighlighter::TSynHighlighterAttributes* fFormAttri;
	Synedithighlighter::TSynHighlighterAttributes* fBlockAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fQualiAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fFunctionAttri;
	Synedithighlighter::TSynHighlighterAttributes* fVariableAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpecialAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func9(void);
	TtkTokenKind __fastcall Func15(void);
	TtkTokenKind __fastcall Func17(void);
	TtkTokenKind __fastcall Func19(void);
	TtkTokenKind __fastcall Func22(void);
	TtkTokenKind __fastcall Func23(void);
	TtkTokenKind __fastcall Func24(void);
	TtkTokenKind __fastcall Func26(void);
	TtkTokenKind __fastcall Func27(void);
	TtkTokenKind __fastcall Func28(void);
	TtkTokenKind __fastcall Func29(void);
	TtkTokenKind __fastcall Func30(void);
	TtkTokenKind __fastcall Func31(void);
	TtkTokenKind __fastcall Func32(void);
	TtkTokenKind __fastcall Func33(void);
	TtkTokenKind __fastcall Func34(void);
	TtkTokenKind __fastcall Func35(void);
	TtkTokenKind __fastcall Func36(void);
	TtkTokenKind __fastcall Func37(void);
	TtkTokenKind __fastcall Func38(void);
	TtkTokenKind __fastcall Func40(void);
	TtkTokenKind __fastcall Func41(void);
	TtkTokenKind __fastcall Func42(void);
	TtkTokenKind __fastcall Func43(void);
	TtkTokenKind __fastcall Func45(void);
	TtkTokenKind __fastcall Func47(void);
	TtkTokenKind __fastcall Func48(void);
	TtkTokenKind __fastcall Func49(void);
	TtkTokenKind __fastcall Func50(void);
	TtkTokenKind __fastcall Func51(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func53(void);
	TtkTokenKind __fastcall Func54(void);
	TtkTokenKind __fastcall Func56(void);
	TtkTokenKind __fastcall Func57(void);
	TtkTokenKind __fastcall Func58(void);
	TtkTokenKind __fastcall Func60(void);
	TtkTokenKind __fastcall Func62(void);
	TtkTokenKind __fastcall Func64(void);
	TtkTokenKind __fastcall Func65(void);
	TtkTokenKind __fastcall Func66(void);
	TtkTokenKind __fastcall Func67(void);
	TtkTokenKind __fastcall Func68(void);
	TtkTokenKind __fastcall Func69(void);
	TtkTokenKind __fastcall Func70(void);
	TtkTokenKind __fastcall Func71(void);
	TtkTokenKind __fastcall Func72(void);
	TtkTokenKind __fastcall Func73(void);
	TtkTokenKind __fastcall Func74(void);
	TtkTokenKind __fastcall Func75(void);
	TtkTokenKind __fastcall Func76(void);
	TtkTokenKind __fastcall Func77(void);
	TtkTokenKind __fastcall Func78(void);
	TtkTokenKind __fastcall Func79(void);
	TtkTokenKind __fastcall Func81(void);
	TtkTokenKind __fastcall Func82(void);
	TtkTokenKind __fastcall Func83(void);
	TtkTokenKind __fastcall Func84(void);
	TtkTokenKind __fastcall Func85(void);
	TtkTokenKind __fastcall Func86(void);
	TtkTokenKind __fastcall Func87(void);
	TtkTokenKind __fastcall Func89(void);
	TtkTokenKind __fastcall Func91(void);
	TtkTokenKind __fastcall Func92(void);
	TtkTokenKind __fastcall Func93(void);
	TtkTokenKind __fastcall Func94(void);
	TtkTokenKind __fastcall Func96(void);
	TtkTokenKind __fastcall Func97(void);
	TtkTokenKind __fastcall Func98(void);
	TtkTokenKind __fastcall Func99(void);
	TtkTokenKind __fastcall Func100(void);
	TtkTokenKind __fastcall Func101(void);
	TtkTokenKind __fastcall Func102(void);
	TtkTokenKind __fastcall Func103(void);
	TtkTokenKind __fastcall Func104(void);
	TtkTokenKind __fastcall Func106(void);
	TtkTokenKind __fastcall Func108(void);
	TtkTokenKind __fastcall Func110(void);
	TtkTokenKind __fastcall Func111(void);
	TtkTokenKind __fastcall Func113(void);
	TtkTokenKind __fastcall Func116(void);
	TtkTokenKind __fastcall Func117(void);
	TtkTokenKind __fastcall Func120(void);
	TtkTokenKind __fastcall Func121(void);
	TtkTokenKind __fastcall Func122(void);
	TtkTokenKind __fastcall Func123(void);
	TtkTokenKind __fastcall Func124(void);
	TtkTokenKind __fastcall Func125(void);
	TtkTokenKind __fastcall Func126(void);
	TtkTokenKind __fastcall Func127(void);
	TtkTokenKind __fastcall Func128(void);
	TtkTokenKind __fastcall Func129(void);
	TtkTokenKind __fastcall Func131(void);
	TtkTokenKind __fastcall Func132(void);
	TtkTokenKind __fastcall Func134(void);
	TtkTokenKind __fastcall Func135(void);
	TtkTokenKind __fastcall Func136(void);
	TtkTokenKind __fastcall Func137(void);
	TtkTokenKind __fastcall Func138(void);
	TtkTokenKind __fastcall Func139(void);
	TtkTokenKind __fastcall Func140(void);
	TtkTokenKind __fastcall Func141(void);
	TtkTokenKind __fastcall Func142(void);
	TtkTokenKind __fastcall Func144(void);
	TtkTokenKind __fastcall Func146(void);
	TtkTokenKind __fastcall Func148(void);
	TtkTokenKind __fastcall Func150(void);
	TtkTokenKind __fastcall Func152(void);
	TtkTokenKind __fastcall Func153(void);
	TtkTokenKind __fastcall Func154(void);
	TtkTokenKind __fastcall Func155(void);
	TtkTokenKind __fastcall Func156(void);
	TtkTokenKind __fastcall Func157(void);
	TtkTokenKind __fastcall Func163(void);
	TtkTokenKind __fastcall Func164(void);
	TtkTokenKind __fastcall Func166(void);
	TtkTokenKind __fastcall Func169(void);
	TtkTokenKind __fastcall Func173(void);
	TtkTokenKind __fastcall Func174(void);
	TtkTokenKind __fastcall Func175(void);
	TtkTokenKind __fastcall Func176(void);
	TtkTokenKind __fastcall Func178(void);
	TtkTokenKind __fastcall Func179(void);
	TtkTokenKind __fastcall Func182(void);
	TtkTokenKind __fastcall Func183(void);
	TtkTokenKind __fastcall Func184(void);
	TtkTokenKind __fastcall Func185(void);
	TtkTokenKind __fastcall Func187(void);
	TtkTokenKind __fastcall Func188(void);
	TtkTokenKind __fastcall Func203(void);
	TtkTokenKind __fastcall Func206(void);
	TtkTokenKind __fastcall Func216(void);
	TtkTokenKind __fastcall Func219(void);
	TtkTokenKind __fastcall Func221(void);
	TtkTokenKind __fastcall Func232(void);
	TtkTokenKind __fastcall Func234(void);
	TtkTokenKind __fastcall Func235(void);
	TtkTokenKind __fastcall Func243(void);
	TtkTokenKind __fastcall Func244(void);
	TtkTokenKind __fastcall Func255(void);
	TtkTokenKind __fastcall Func313(void);
	TtkTokenKind __fastcall Func327(void);
	TtkTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	void __fastcall SymbolProc(void);
	void __fastcall AddressOpProc(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall CRProc(void);
	void __fastcall GreaterProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall LowerProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall PointProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall UnknownProc(void);
	void __fastcall RemProc(void);
	bool __fastcall IsQuali(void);
	bool __fastcall IsSpecial(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynDmlSyn(Classes::TComponent* AOwner);
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
	__property Synedithighlighter::TSynHighlighterAttributes* BlockAttri = {read=fBlockAttri, write=fBlockAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* FormAttri = {read=fFormAttri, write=fFormAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* FunctionAttri = {read=fFunctionAttri, write=fFunctionAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* QualiAttri = {read=fQualiAttri, write=fQualiAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpecialAttri = {read=fSpecialAttri, write=fSpecialAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* VariableAttri = {read=fVariableAttri, write=fVariableAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynDmlSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlighterdml */
using namespace Synhighlighterdml;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterDml
