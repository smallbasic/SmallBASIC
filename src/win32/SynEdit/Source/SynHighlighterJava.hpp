// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterJava.pas' rev: 6.00

#ifndef SynHighlighterJavaHPP
#define SynHighlighterJavaHPP

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

namespace Synhighlighterjava
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkDocument, tkIdentifier, tkInvalid, tkKey, tkNull, tkNumber, tkSpace, tkString, tkSymbol, tkUnknown };
#pragma option pop

#pragma option push -b-
enum TxtkTokenKind { xtkAdd, xtkAddAssign, xtkAnd, xtkAndAssign, xtkAssign, xtkBitComplement, xtkBraceClose, xtkBraceOpen, xtkColon, xtkCondAnd, xtkCondOr, xtkDecrement, xtkDivide, xtkDivideAssign, xtkGreaterThan, xtkGreaterThanEqual, xtkIncOr, xtkIncOrAssign, xtkIncrement, xtkLessThan, xtkLessThanEqual, xtkLogComplement, xtkLogEqual, xtkMultiply, xtkMultiplyAssign, xtkNotEqual, xtkPoint, xtkQuestion, xtkRemainder, xtkRemainderAssign, xtkRoundClose, xtkRoundOpen, xtkSemiColon, xtkShiftLeft, xtkShiftLeftAssign, xtkShiftRight, xtkShiftRightAssign, xtkSquareClose, xtkSquareOpen, xtkSubtract, xtkSubtractAssign, xtkUnsignShiftRight, xtkUnsignShiftRightAssign, xtkXor, xtkXorAssign, xtkComma };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsANil, rsComment, rsDocument, rsUnknown };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

class DELPHICLASS TSynJavaSyn;
class PASCALIMPLEMENTATION TSynJavaSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TRangeState fRange;
	char *fLine;
	TProcTableProc fProcTable[256];
	int Run;
	int FRoundCount;
	int FSquareCount;
	int fStringLen;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind FTokenID;
	TxtkTokenKind FExtTokenID;
	bool fEol;
	TIdentFuncTableFunc fIdentFuncTable[173];
	int fLineNumber;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDocumentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fInvalidAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func17(void);
	TtkTokenKind __fastcall Func21(void);
	TtkTokenKind __fastcall Func32(void);
	TtkTokenKind __fastcall Func34(void);
	TtkTokenKind __fastcall Func40(void);
	TtkTokenKind __fastcall Func42(void);
	TtkTokenKind __fastcall Func45(void);
	TtkTokenKind __fastcall Func46(void);
	TtkTokenKind __fastcall Func47(void);
	TtkTokenKind __fastcall Func48(void);
	TtkTokenKind __fastcall Func51(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func54(void);
	TtkTokenKind __fastcall Func56(void);
	TtkTokenKind __fastcall Func59(void);
	TtkTokenKind __fastcall Func60(void);
	TtkTokenKind __fastcall Func61(void);
	TtkTokenKind __fastcall Func62(void);
	TtkTokenKind __fastcall Func63(void);
	TtkTokenKind __fastcall Func65(void);
	TtkTokenKind __fastcall Func66(void);
	TtkTokenKind __fastcall Func68(void);
	TtkTokenKind __fastcall Func69(void);
	TtkTokenKind __fastcall Func71(void);
	TtkTokenKind __fastcall Func76(void);
	TtkTokenKind __fastcall Func77(void);
	TtkTokenKind __fastcall Func78(void);
	TtkTokenKind __fastcall Func84(void);
	TtkTokenKind __fastcall Func85(void);
	TtkTokenKind __fastcall Func86(void);
	TtkTokenKind __fastcall Func88(void);
	TtkTokenKind __fastcall Func89(void);
	TtkTokenKind __fastcall Func90(void);
	TtkTokenKind __fastcall Func92(void);
	TtkTokenKind __fastcall Func97(void);
	TtkTokenKind __fastcall Func98(void);
	TtkTokenKind __fastcall Func102(void);
	TtkTokenKind __fastcall Func104(void);
	TtkTokenKind __fastcall Func109(void);
	TtkTokenKind __fastcall Func115(void);
	TtkTokenKind __fastcall Func116(void);
	TtkTokenKind __fastcall Func119(void);
	TtkTokenKind __fastcall Func129(void);
	TtkTokenKind __fastcall Func136(void);
	TtkTokenKind __fastcall Func172(void);
	void __fastcall CommentProc(void);
	void __fastcall AndSymbolProc(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall AtSymbolProc(void);
	void __fastcall BraceCloseProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall CRProc(void);
	void __fastcall ColonProc(void);
	void __fastcall CommaProc(void);
	void __fastcall EqualProc(void);
	void __fastcall GreaterProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall LowerProc(void);
	void __fastcall MinusProc(void);
	void __fastcall MultiplyProc(void);
	void __fastcall NotSymbolProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall OrSymbolProc(void);
	void __fastcall PlusProc(void);
	void __fastcall PointProc(void);
	void __fastcall PoundProc(void);
	void __fastcall QuestionProc(void);
	void __fastcall RemainderSymbolProc(void);
	void __fastcall RoundCloseProc(void);
	void __fastcall RoundOpenProc(void);
	void __fastcall SemiColonProc(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall SquareCloseProc(void);
	void __fastcall SquareOpenProc(void);
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
	virtual AnsiString __fastcall GetSampleSource();
	TxtkTokenKind __fastcall GetExtTokenID(void);
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynJavaSyn(Classes::TComponent* AOwner);
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
	__property TxtkTokenKind ExtTokenID = {read=GetExtTokenID, nodefault};
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DocumentAttri = {read=fDocumentAttri, write=fDocumentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* InvalidAttri = {read=fInvalidAttri, write=fInvalidAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynJavaSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlighterjava */
using namespace Synhighlighterjava;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterJava
