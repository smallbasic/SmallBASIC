// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterXML.pas' rev: 6.00

#ifndef SynHighlighterXMLHPP
#define SynHighlighterXMLHPP

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

namespace Synhighlighterxml
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkAposAttrValue, tkAposEntityRef, tkAttribute, tkCDATA, tkComment, tkElement, tkEntityRef, tkEqual, tkNull, tkProcessingInstruction, tkQuoteAttrValue, tkQuoteEntityRef, tkSpace, tkSymbol, tkText, tknsAposAttrValue, tknsAposEntityRef, tknsAttribute, tknsEqual, tknsQuoteAttrValue, tknsQuoteEntityRef, tkDocType };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsAposAttrValue, rsAPosEntityRef, rsAttribute, rsCDATA, rsComment, rsElement, rsEntityRef, rsEqual, rsProcessingInstruction, rsQuoteAttrValue, rsQuoteEntityRef, rsText, rsnsAposAttrValue, rsnsAPosEntityRef, rsnsEqual, rsnsQuoteAttrValue, rsnsQuoteEntityRef, rsDocType, rsDocTypeSquareBraces };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

class DELPHICLASS TSynXMLSyn;
class PASCALIMPLEMENTATION TSynXMLSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TRangeState fRange;
	char *fLine;
	int Run;
	int fTokenPos;
	TtkTokenKind fTokenID;
	int fLineNumber;
	Synedithighlighter::TSynHighlighterAttributes* fElementAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fTextAttri;
	Synedithighlighter::TSynHighlighterAttributes* fEntityRefAttri;
	Synedithighlighter::TSynHighlighterAttributes* fProcessingInstructionAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCDATAAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDocTypeAttri;
	Synedithighlighter::TSynHighlighterAttributes* fAttributeAttri;
	Synedithighlighter::TSynHighlighterAttributes* fnsAttributeAttri;
	Synedithighlighter::TSynHighlighterAttributes* fAttributeValueAttri;
	Synedithighlighter::TSynHighlighterAttributes* fnsAttributeValueAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	TProcTableProc fProcTable[256];
	bool FWantBracesParsed;
	void __fastcall NullProc(void);
	void __fastcall CarriageReturnProc(void);
	void __fastcall LineFeedProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall LessThanProc(void);
	void __fastcall GreaterThanProc(void);
	void __fastcall CommentProc(void);
	void __fastcall ProcessingInstructionProc(void);
	void __fastcall DocTypeProc(void);
	void __fastcall CDATAProc(void);
	void __fastcall TextProc(void);
	void __fastcall ElementProc(void);
	void __fastcall AttributeProc(void);
	void __fastcall QAttributeValueProc(void);
	void __fastcall AAttributeValueProc(void);
	void __fastcall EqualProc(void);
	void __fastcall IdentProc(void);
	void __fastcall MakeMethodTables(void);
	bool __fastcall NextTokenIs(AnsiString T);
	void __fastcall EntityRefProc(void);
	void __fastcall QEntityRefProc(void);
	void __fastcall AEntityRefProc(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynXMLSyn(Classes::TComponent* AOwner);
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
	__property Synedithighlighter::TSynHighlighterAttributes* ElementAttri = {read=fElementAttri, write=fElementAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* AttributeAttri = {read=fAttributeAttri, write=fAttributeAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NamespaceAttributeAttri = {read=fnsAttributeAttri, write=fnsAttributeAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* AttributeValueAttri = {read=fAttributeValueAttri, write=fAttributeValueAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NamespaceAttributeValueAttri = {read=fnsAttributeValueAttri, write=fnsAttributeValueAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* TextAttri = {read=fTextAttri, write=fTextAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CDATAAttri = {read=fCDATAAttri, write=fCDATAAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* EntityRefAttri = {read=fEntityRefAttri, write=fEntityRefAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* ProcessingInstructionAttri = {read=fProcessingInstructionAttri, write=fProcessingInstructionAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DocTypeAttri = {read=fDocTypeAttri, write=fDocTypeAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property bool WantBracesParsed = {read=FWantBracesParsed, write=FWantBracesParsed, default=1};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynXMLSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlighterxml */
using namespace Synhighlighterxml;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterXML
