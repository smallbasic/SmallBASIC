// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterAWK.pas' rev: 6.00

#ifndef SynHighlighterAWKHPP
#define SynHighlighterAWKHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditTypes.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
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

namespace Synhighlighterawk
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkIdentifier, tkInterFunc, tkKey, tkNull, tkNumber, tkSpace, tkString, tkSymbol, tkSysVar, tkUnknown };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

class DELPHICLASS TSynAWKSyn;
class PASCALIMPLEMENTATION TSynAWKSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	Classes::TStringList* AWKSyntaxList;
	char *fLine;
	TProcTableProc fProcTable[256];
	int Run;
	int fTokenPos;
	TtkTokenKind FTokenID;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fInterFuncAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSysVarAttri;
	int fLineNumber;
	void __fastcall AndProc(void);
	void __fastcall CommentProc(void);
	void __fastcall CRProc(void);
	void __fastcall ExclamProc(void);
	void __fastcall FieldRefProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall MakeMethodTables(void);
	void __fastcall MakeSyntaxList(void);
	void __fastcall MinusProc(void);
	void __fastcall NullProc(void);
	void __fastcall OpInputProc(void);
	void __fastcall OrProc(void);
	void __fastcall PlusProc(void);
	void __fastcall QuestionProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall SymbolProc(void);
	void __fastcall NumberProc(void);
	void __fastcall BraceProc(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynAWKSyn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynAWKSyn(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* InterFuncAttri = {read=fInterFuncAttri, write=fInterFuncAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SysVarAttri = {read=fSysVarAttri, write=fSysVarAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlighterawk */
using namespace Synhighlighterawk;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterAWK
