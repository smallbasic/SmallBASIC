// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterGalaxy.pas' rev: 6.00

#ifndef SynHighlighterGalaxyHPP
#define SynHighlighterGalaxyHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
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

namespace Synhighlightergalaxy
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkIdentifier, tkKey, tkNull, tkSpace, tkMessage, tkUnknown };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsUnKnown, rsMessageStyle };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

class DELPHICLASS TSynGalaxySyn;
class PASCALIMPLEMENTATION TSynGalaxySyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TRangeState fRange;
	char *fLine;
	TProcTableProc fProcTable[256];
	int Run;
	int fTokenPos;
	TtkTokenKind FTokenID;
	int fLineNumber;
	Synedithighlighter::TSynHighlighterAttributes* fMessageAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Classes::TStrings* fKeyWords;
	void __fastcall PointCommaProc(void);
	void __fastcall CRProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall NullProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall UnknownProc(void);
	void __fastcall MakeMethodTables(void);
	void __fastcall MessageStyleProc(void);
	void __fastcall SetKeyWords(const Classes::TStrings* Value);
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynGalaxySyn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynGalaxySyn(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual void * __fastcall GetRange(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual bool __fastcall IsKeyword(const AnsiString AKeyword);
	virtual void __fastcall Next(void);
	virtual void __fastcall SetRange(void * Value);
	virtual void __fastcall ReSetRange(void);
	virtual bool __fastcall SaveToRegistry(HKEY RootKey, AnsiString Key);
	virtual bool __fastcall LoadFromRegistry(HKEY RootKey, AnsiString Key);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Classes::TStrings* KeyWords = {read=fKeyWords, write=SetKeyWords};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* MessageAttri = {read=fMessageAttri, write=fMessageAttri};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlightergalaxy */
using namespace Synhighlightergalaxy;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterGalaxy
