// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterGeneral.pas' rev: 6.00

#ifndef SynHighlighterGeneralHPP
#define SynHighlighterGeneralHPP

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

namespace Synhighlightergeneral
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkIdentifier, tkKey, tkNull, tkNumber, tkPreprocessor, tkSpace, tkString, tkSymbol, tkUnknown };
#pragma option pop

#pragma option push -b-
enum TCommentStyle { csAnsiStyle, csPasStyle, csCStyle, csAsmStyle, csBasStyle };
#pragma option pop

typedef Set<TCommentStyle, csAnsiStyle, csBasStyle>  CommentStyles;

#pragma option push -b-
enum TRangeState { rsANil, rsAnsi, rsPasStyle, rsCStyle, rsUnKnown };
#pragma option pop

#pragma option push -b-
enum TStringDelim { sdSingleQuote, sdDoubleQuote };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

class DELPHICLASS TSynGeneralSyn;
class PASCALIMPLEMENTATION TSynGeneralSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TRangeState fRange;
	char *fLine;
	TProcTableProc fProcTable[256];
	int Run;
	int fTokenPos;
	TtkTokenKind fTokenID;
	int fLineNumber;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fPreprocessorAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Classes::TStrings* fKeyWords;
	CommentStyles fComments;
	char fStringDelimCh;
	Synedittypes::TSynIdentChars fIdentChars;
	bool fDetectPreprocessor;
	void __fastcall AsciiCharProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall PointCommaProc(void);
	void __fastcall CRProc(void);
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
	void __fastcall MakeMethodTables(void);
	void __fastcall AnsiProc(void);
	void __fastcall PasStyleProc(void);
	void __fastcall CStyleProc(void);
	void __fastcall SetKeyWords(const Classes::TStrings* Value);
	void __fastcall SetComments(CommentStyles Value);
	TStringDelim __fastcall GetStringDelim(void);
	void __fastcall SetStringDelim(const TStringDelim Value);
	AnsiString __fastcall GetIdentifierChars();
	void __fastcall SetIdentifierChars(const AnsiString Value);
	void __fastcall SetDetectPreprocessor(bool Value);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynGeneralSyn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynGeneralSyn(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual void * __fastcall GetRange(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual bool __fastcall IsKeyword(const AnsiString AKeyword);
	virtual void __fastcall Next(void);
	virtual void __fastcall ResetRange(void);
	virtual void __fastcall SetRange(void * Value);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual bool __fastcall SaveToRegistry(HKEY RootKey, AnsiString Key);
	virtual bool __fastcall LoadFromRegistry(HKEY RootKey, AnsiString Key);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property CommentStyles Comments = {read=fComments, write=SetComments, nodefault};
	__property bool DetectPreprocessor = {read=fDetectPreprocessor, write=SetDetectPreprocessor, nodefault};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property AnsiString IdentifierChars = {read=GetIdentifierChars, write=SetIdentifierChars};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Classes::TStrings* KeyWords = {read=fKeyWords, write=SetKeyWords};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* PreprocessorAttri = {read=fPreprocessorAttri, write=fPreprocessorAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property TStringDelim StringDelim = {read=GetStringDelim, write=SetStringDelim, default=0};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlightergeneral */
using namespace Synhighlightergeneral;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterGeneral
