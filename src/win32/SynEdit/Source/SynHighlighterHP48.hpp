// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterHP48.pas' rev: 6.00

#ifndef SynHighlighterHP48HPP
#define SynHighlighterHP48HPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditStrConst.hpp>	// Pascal unit
#include <SynHighlighterHP48Utils.hpp>	// Pascal unit
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

namespace Synhighlighterhp48
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkNull, tkAsmKey, tkAsm, tkAsmComment, tksAsmKey, tksAsm, tksAsmComment, tkRplKey, tkRpl, tkRplComment };
#pragma option pop

typedef AnsiString SynHighlighterHP48__1[10];

#pragma option push -b-
enum TRangeState { rsRpl, rsComRpl, rssasm1, rssasm2, rssasm3, rsAsm, rsComAsm2, rsComAsm1 };
#pragma option pop

class DELPHICLASS TSynHP48Syn;
class PASCALIMPLEMENTATION TSynHP48Syn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TtkTokenKind fTockenKind;
	TRangeState fRange;
	AnsiString fLine;
	int Run;
	int fTokenPos;
	bool fEol;
	Synedithighlighter::TSynHighlighterAttributes* Attribs[10];
	Synhighlighterhp48utils::TSpeedStringList* FRplKeyWords;
	Synhighlighterhp48utils::TSpeedStringList* FAsmKeyWords;
	Synhighlighterhp48utils::TSpeedStringList* FSAsmNoField;
	TRangeState FBaseRange;
	Synedithighlighter::TSynHighlighterAttributes* __fastcall GetAttrib(int Index);
	void __fastcall SetAttrib(int Index, Synedithighlighter::TSynHighlighterAttributes* Value);
	TtkTokenKind __fastcall NullProc(void);
	TtkTokenKind __fastcall SpaceProc(void);
	TtkTokenKind __fastcall ParOpenProc(void);
	TtkTokenKind __fastcall RplComProc(void);
	TtkTokenKind __fastcall AsmComProc(char c);
	TtkTokenKind __fastcall PersentProc(void);
	TtkTokenKind __fastcall IdentProc(void);
	TtkTokenKind __fastcall SlashProc(void);
	TtkTokenKind __fastcall SasmProc1(void);
	TtkTokenKind __fastcall SasmProc2(void);
	TtkTokenKind __fastcall SasmProc3(void);
	void __fastcall EndOfToken(void);
	void __fastcall SetHighLightChange(void);
	TtkTokenKind __fastcall Next1(void);
	void __fastcall Next2(TtkTokenKind tkk);
	TtkTokenKind __fastcall GetTokenFromRange(void);
	TtkTokenKind __fastcall StarProc(void);
	
protected:
	virtual int __fastcall GetAttribCount(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetAttribute(int idx);
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynHP48Syn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynHP48Syn(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual AnsiString __fastcall GetToken();
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual void * __fastcall GetRange(void);
	virtual void __fastcall SetRange(void * Value);
	virtual void __fastcall ReSetRange(void);
	virtual bool __fastcall SaveToRegistry(HKEY RootKey, AnsiString Key);
	virtual bool __fastcall LoadFromRegistry(HKEY RootKey, AnsiString Key);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	__property Synhighlighterhp48utils::TSpeedStringList* AsmKeyWords = {read=FAsmKeyWords};
	__property Synhighlighterhp48utils::TSpeedStringList* SAsmFoField = {read=FSAsmNoField};
	__property Synhighlighterhp48utils::TSpeedStringList* RplKeyWords = {read=FRplKeyWords};
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* AsmKey = {read=GetAttrib, write=SetAttrib, index=1};
	__property Synedithighlighter::TSynHighlighterAttributes* AsmTxt = {read=GetAttrib, write=SetAttrib, index=2};
	__property Synedithighlighter::TSynHighlighterAttributes* AsmComment = {read=GetAttrib, write=SetAttrib, index=3};
	__property Synedithighlighter::TSynHighlighterAttributes* sAsmKey = {read=GetAttrib, write=SetAttrib, index=4};
	__property Synedithighlighter::TSynHighlighterAttributes* sAsmTxt = {read=GetAttrib, write=SetAttrib, index=5};
	__property Synedithighlighter::TSynHighlighterAttributes* sAsmComment = {read=GetAttrib, write=SetAttrib, index=6};
	__property Synedithighlighter::TSynHighlighterAttributes* RplKey = {read=GetAttrib, write=SetAttrib, index=7};
	__property Synedithighlighter::TSynHighlighterAttributes* RplTxt = {read=GetAttrib, write=SetAttrib, index=8};
	__property Synedithighlighter::TSynHighlighterAttributes* RplComment = {read=GetAttrib, write=SetAttrib, index=9};
	__property TRangeState BaseRange = {read=FBaseRange, write=FBaseRange, nodefault};
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE AnsiString tkTokenName[10];

}	/* namespace Synhighlighterhp48 */
using namespace Synhighlighterhp48;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterHP48
