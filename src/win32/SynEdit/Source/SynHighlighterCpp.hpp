// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterCpp.pas' rev: 6.00

#ifndef SynHighlighterCppHPP
#define SynHighlighterCppHPP

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

namespace Synhighlightercpp
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkAsm, tkComment, tkDirective, tkIdentifier, tkKey, tkNull, tkNumber, tkSpace, tkString, tkSymbol, tkUnknown, tkChar, tkFloat, tkHex, tkOctal };
#pragma option pop

#pragma option push -b-
enum TxtkTokenKind { xtkAdd, xtkAddAssign, xtkAnd, xtkAndAssign, xtkArrow, xtkAssign, xtkBitComplement, xtkBraceClose, xtkBraceOpen, xtkColon, xtkComma, xtkDecrement, xtkDivide, xtkDivideAssign, xtkEllipse, xtkGreaterThan, xtkGreaterThanEqual, xtkIncOr, xtkIncOrAssign, xtkIncrement, xtkLessThan, xtkLessThanEqual, xtkLogAnd, xtkLogComplement, xtkLogEqual, xtkLogOr, xtkMod, xtkModAssign, xtkMultiplyAssign, xtkNotEqual, xtkPoint, xtkQuestion, xtkRoundClose, xtkRoundOpen, xtkScopeResolution, xtkSemiColon, xtkShiftLeft, xtkShiftLeftAssign, xtkShiftRight, xtkShiftRightAssign, xtkSquareClose, xtkSquareOpen, xtkStar, xtkSubtract, xtkSubtractAssign, xtkXor, xtkXorAssign };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsUnknown, rsAnsiC, rsAnsiCAsm, rsAnsiCAsmBlock, rsAsm, rsAsmBlock, rsDirective, rsDirectiveComment, rsString34, rsString39, rsMultiLineString, rsMultiLineDirective };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

class DELPHICLASS TSynCppSyn;
class PASCALIMPLEMENTATION TSynCppSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	bool fAsmStart;
	TRangeState fRange;
	char *fLine;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind FTokenID;
	TxtkTokenKind FExtTokenID;
	int fLineNumber;
	TIdentFuncTableFunc fIdentFuncTable[207];
	Synedithighlighter::TSynHighlighterAttributes* fAsmAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDirecAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fInvalidAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fFloatAttri;
	Synedithighlighter::TSynHighlighterAttributes* fHexAttri;
	Synedithighlighter::TSynHighlighterAttributes* fOctalAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCharAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func17(void);
	TtkTokenKind __fastcall Func21(void);
	TtkTokenKind __fastcall Func32(void);
	TtkTokenKind __fastcall Func34(void);
	TtkTokenKind __fastcall Func36(void);
	TtkTokenKind __fastcall Func40(void);
	TtkTokenKind __fastcall Func42(void);
	TtkTokenKind __fastcall Func45(void);
	TtkTokenKind __fastcall Func46(void);
	TtkTokenKind __fastcall Func48(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func54(void);
	TtkTokenKind __fastcall Func57(void);
	TtkTokenKind __fastcall Func58(void);
	TtkTokenKind __fastcall Func59(void);
	TtkTokenKind __fastcall Func60(void);
	TtkTokenKind __fastcall Func61(void);
	TtkTokenKind __fastcall Func62(void);
	TtkTokenKind __fastcall Func64(void);
	TtkTokenKind __fastcall Func65(void);
	TtkTokenKind __fastcall Func66(void);
	TtkTokenKind __fastcall Func67(void);
	TtkTokenKind __fastcall Func68(void);
	TtkTokenKind __fastcall Func69(void);
	TtkTokenKind __fastcall Func71(void);
	TtkTokenKind __fastcall Func74(void);
	TtkTokenKind __fastcall Func75(void);
	TtkTokenKind __fastcall Func76(void);
	TtkTokenKind __fastcall Func78(void);
	TtkTokenKind __fastcall Func79(void);
	TtkTokenKind __fastcall Func81(void);
	TtkTokenKind __fastcall Func82(void);
	TtkTokenKind __fastcall Func85(void);
	TtkTokenKind __fastcall Func86(void);
	TtkTokenKind __fastcall Func88(void);
	TtkTokenKind __fastcall Func89(void);
	TtkTokenKind __fastcall Func92(void);
	TtkTokenKind __fastcall Func97(void);
	TtkTokenKind __fastcall Func98(void);
	TtkTokenKind __fastcall Func100(void);
	TtkTokenKind __fastcall Func101(void);
	TtkTokenKind __fastcall Func102(void);
	TtkTokenKind __fastcall Func104(void);
	TtkTokenKind __fastcall Func105(void);
	TtkTokenKind __fastcall Func106(void);
	TtkTokenKind __fastcall Func107(void);
	TtkTokenKind __fastcall Func109(void);
	TtkTokenKind __fastcall Func110(void);
	TtkTokenKind __fastcall Func115(void);
	TtkTokenKind __fastcall Func116(void);
	TtkTokenKind __fastcall Func123(void);
	TtkTokenKind __fastcall Func125(void);
	TtkTokenKind __fastcall Func141(void);
	TtkTokenKind __fastcall Func206(void);
	void __fastcall AnsiCProc(void);
	void __fastcall AndSymbolProc(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall AtSymbolProc(void);
	void __fastcall BraceCloseProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall CRProc(void);
	void __fastcall ColonProc(void);
	void __fastcall CommaProc(void);
	void __fastcall DirectiveProc(void);
	void __fastcall DirectiveEndProc(void);
	void __fastcall EqualProc(void);
	void __fastcall GreaterProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall LowerProc(void);
	void __fastcall MinusProc(void);
	void __fastcall ModSymbolProc(void);
	void __fastcall NotSymbolProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall OrSymbolProc(void);
	void __fastcall PlusProc(void);
	void __fastcall PointProc(void);
	void __fastcall QuestionProc(void);
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
	void __fastcall StringEndProc(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	TxtkTokenKind __fastcall GetExtTokenID(void);
	virtual AnsiString __fastcall GetSampleSource();
	
public:
	/* virtual class method */ virtual Synedithighlighter::TSynHighlighterCapabilities __fastcall GetCapabilities(TMetaClass* vmt);
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynCppSyn(Classes::TComponent* AOwner);
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
	virtual bool __fastcall UseUserSettings(int settingIndex);
	virtual void __fastcall EnumUserSettings(Classes::TStrings* settings);
	__property TxtkTokenKind ExtTokenID = {read=GetExtTokenID, nodefault};
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* AsmAttri = {read=fAsmAttri, write=fAsmAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DirecAttri = {read=fDirecAttri, write=fDirecAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* InvalidAttri = {read=fInvalidAttri, write=fInvalidAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* FloatAttri = {read=fFloatAttri, write=fFloatAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* HexAttri = {read=fHexAttri, write=fHexAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* OctalAttri = {read=fOctalAttri, write=fOctalAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CharAttri = {read=fCharAttri, write=fCharAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynCppSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlightercpp */
using namespace Synhighlightercpp;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterCpp
