// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterCPM.pas' rev: 6.00

#ifndef SynHighlighterCPMHPP
#define SynHighlighterCPMHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlightercpm
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkIdentifier, tkKey, tkNull, tkSpace, tkSQLKey, tkString, tkSymbol, tkSpecialVar, tkSystem, tkVariable, tkNumber, tkUnknown };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

#pragma option push -b-
enum TRangeState { rsBraceComment, rsUnKnown };
#pragma option pop

class DELPHICLASS TSynCPMSyn;
class PASCALIMPLEMENTATION TSynCPMSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	char *fLine;
	int fLineNumber;
	TProcTableProc fProcTable[256];
	int Run;
	TRangeState fRange;
	int fCommentLevel;
	int fStringLen;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind fTokenID;
	TIdentFuncTableFunc fIdentFuncTable[292];
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSQLKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpecialVarAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSystemAttri;
	Synedithighlighter::TSynHighlighterAttributes* fVariableAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func15(void);
	TtkTokenKind __fastcall Func21(void);
	TtkTokenKind __fastcall Func23(void);
	TtkTokenKind __fastcall Func28(void);
	TtkTokenKind __fastcall Func29(void);
	TtkTokenKind __fastcall Func30(void);
	TtkTokenKind __fastcall Func37(void);
	TtkTokenKind __fastcall Func41(void);
	TtkTokenKind __fastcall Func43(void);
	TtkTokenKind __fastcall Func44(void);
	TtkTokenKind __fastcall Func47(void);
	TtkTokenKind __fastcall Func48(void);
	TtkTokenKind __fastcall Func49(void);
	TtkTokenKind __fastcall Func53(void);
	TtkTokenKind __fastcall Func54(void);
	TtkTokenKind __fastcall Func55(void);
	TtkTokenKind __fastcall Func56(void);
	TtkTokenKind __fastcall Func57(void);
	TtkTokenKind __fastcall Func58(void);
	TtkTokenKind __fastcall Func62(void);
	TtkTokenKind __fastcall Func63(void);
	TtkTokenKind __fastcall Func65(void);
	TtkTokenKind __fastcall Func66(void);
	TtkTokenKind __fastcall Func68(void);
	TtkTokenKind __fastcall Func69(void);
	TtkTokenKind __fastcall Func72(void);
	TtkTokenKind __fastcall Func74(void);
	TtkTokenKind __fastcall Func75(void);
	TtkTokenKind __fastcall Func76(void);
	TtkTokenKind __fastcall Func79(void);
	TtkTokenKind __fastcall Func83(void);
	TtkTokenKind __fastcall Func85(void);
	TtkTokenKind __fastcall Func86(void);
	TtkTokenKind __fastcall Func88(void);
	TtkTokenKind __fastcall Func89(void);
	TtkTokenKind __fastcall Func91(void);
	TtkTokenKind __fastcall Func92(void);
	TtkTokenKind __fastcall Func94(void);
	TtkTokenKind __fastcall Func95(void);
	TtkTokenKind __fastcall Func96(void);
	TtkTokenKind __fastcall Func99(void);
	TtkTokenKind __fastcall Func100(void);
	TtkTokenKind __fastcall Func101(void);
	TtkTokenKind __fastcall Func104(void);
	TtkTokenKind __fastcall Func105(void);
	TtkTokenKind __fastcall Func106(void);
	TtkTokenKind __fastcall Func107(void);
	TtkTokenKind __fastcall Func108(void);
	TtkTokenKind __fastcall Func109(void);
	TtkTokenKind __fastcall Func112(void);
	TtkTokenKind __fastcall Func113(void);
	TtkTokenKind __fastcall Func114(void);
	TtkTokenKind __fastcall Func116(void);
	TtkTokenKind __fastcall Func117(void);
	TtkTokenKind __fastcall Func118(void);
	TtkTokenKind __fastcall Func119(void);
	TtkTokenKind __fastcall Func120(void);
	TtkTokenKind __fastcall Func122(void);
	TtkTokenKind __fastcall Func125(void);
	TtkTokenKind __fastcall Func126(void);
	TtkTokenKind __fastcall Func127(void);
	TtkTokenKind __fastcall Func128(void);
	TtkTokenKind __fastcall Func130(void);
	TtkTokenKind __fastcall Func131(void);
	TtkTokenKind __fastcall Func133(void);
	TtkTokenKind __fastcall Func134(void);
	TtkTokenKind __fastcall Func136(void);
	TtkTokenKind __fastcall Func137(void);
	TtkTokenKind __fastcall Func138(void);
	TtkTokenKind __fastcall Func139(void);
	TtkTokenKind __fastcall Func141(void);
	TtkTokenKind __fastcall Func142(void);
	TtkTokenKind __fastcall Func143(void);
	TtkTokenKind __fastcall Func146(void);
	TtkTokenKind __fastcall Func147(void);
	TtkTokenKind __fastcall Func148(void);
	TtkTokenKind __fastcall Func149(void);
	TtkTokenKind __fastcall Func153(void);
	TtkTokenKind __fastcall Func154(void);
	TtkTokenKind __fastcall Func156(void);
	TtkTokenKind __fastcall Func157(void);
	TtkTokenKind __fastcall Func160(void);
	TtkTokenKind __fastcall Func162(void);
	TtkTokenKind __fastcall Func164(void);
	TtkTokenKind __fastcall Func165(void);
	TtkTokenKind __fastcall Func166(void);
	TtkTokenKind __fastcall Func170(void);
	TtkTokenKind __fastcall Func174(void);
	TtkTokenKind __fastcall Func178(void);
	TtkTokenKind __fastcall Func186(void);
	TtkTokenKind __fastcall Func187(void);
	TtkTokenKind __fastcall Func188(void);
	TtkTokenKind __fastcall Func198(void);
	TtkTokenKind __fastcall Func210(void);
	TtkTokenKind __fastcall Func211(void);
	TtkTokenKind __fastcall Func212(void);
	TtkTokenKind __fastcall Func213(void);
	TtkTokenKind __fastcall Func271(void);
	TtkTokenKind __fastcall Func273(void);
	TtkTokenKind __fastcall Func291(void);
	void __fastcall CRProc(void);
	void __fastcall LFProc(void);
	void __fastcall SemiColonProc(void);
	void __fastcall SymbolProc(void);
	void __fastcall NumberProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall IdentProc(void);
	void __fastcall VariableProc(void);
	void __fastcall NullProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall UnknownProc(void);
	TtkTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	void __fastcall BraceCommentProc(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	virtual bool __fastcall IsFilterStored(void);
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynCPMSyn(Classes::TComponent* AOwner);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual void * __fastcall GetRange(void);
	virtual void __fastcall ResetRange(void);
	virtual void __fastcall SetRange(void * Value);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SQLKeyAttri = {read=fSQLKeyAttri, write=fSQLKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpecialVarAttri = {read=fSpecialVarAttri, write=fSpecialVarAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SystemAttri = {read=fSystemAttri, write=fSystemAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* VariableAttri = {read=fVariableAttri, write=fVariableAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynCPMSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
static const Word MaxKey = 0x123;

}	/* namespace Synhighlightercpm */
using namespace Synhighlightercpm;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterCPM
