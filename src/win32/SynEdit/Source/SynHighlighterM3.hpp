// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterM3.pas' rev: 6.00

#ifndef SynHighlighterM3HPP
#define SynHighlighterM3HPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynHighlighterHashEntries.hpp>	// Pascal unit
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

namespace Synhighlighterm3
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkIdentifier, tkKey, tkNull, tkNumber, tkPragma, tkReserved, tkSpace, tkString, tkSymbol, tkUnknown, tkSyntaxError };
#pragma option pop

#pragma option push -b-
enum TTokenRange { trNone, trComment, trPragma };
#pragma option pop

#pragma pack(push, 1)
struct TRangeState
{
	
	union
	{
		struct 
		{
			Word TokenRange;
			Word Level;
			
		};
		struct 
		{
			void *p;
			
		};
		
	};
} ;
#pragma pack(pop)

typedef void __fastcall (__closure *TProcTableProc)(void);

class DELPHICLASS TSynM3Syn;
class PASCALIMPLEMENTATION TSynM3Syn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	char *fLine;
	int fLineNumber;
	TProcTableProc fProcTable[256];
	int Run;
	#pragma pack(push, 1)
	TRangeState fRange;
	#pragma pack(pop)
	
	int fStringLen;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind FTokenID;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fPragmaAttri;
	Synedithighlighter::TSynHighlighterAttributes* fReservedAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSyntaxErrorAttri;
	Synhighlighterhashentries::TSynHashEntryList* fKeywords;
	void __fastcall DoAddKeyword(AnsiString AKeyword, int AKind);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	bool __fastcall KeyComp(AnsiString AKey);
	int __fastcall KeyHash(char * ToHash);
	void __fastcall MakeMethodTables(void);
	void __fastcall SymAsciiCharProc(void);
	void __fastcall SymCommentHelpProc(void);
	void __fastcall SymCRProc(void);
	void __fastcall SymIdentProc(void);
	void __fastcall SymLFProc(void);
	void __fastcall SymNestedHelperProc(char AOpenChar, char ACloseChar);
	void __fastcall SymNullProc(void);
	void __fastcall SymNumberProc(void);
	void __fastcall SymPragmaProc(void);
	void __fastcall SymPragmaHelpProc(void);
	void __fastcall SymRoundOpenProc(void);
	void __fastcall SymSpaceProc(void);
	void __fastcall SymStringProc(void);
	void __fastcall SymSymbolProc(void);
	void __fastcall SymUnknownProc(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynM3Syn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynM3Syn(void);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual void * __fastcall GetRange(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual void __fastcall ResetRange(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual void __fastcall SetRange(void * Value);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* PragmaAttri = {read=fPragmaAttri, write=fPragmaAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* ReservedAttri = {read=fReservedAttri, write=fReservedAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SyntaxErrorAttri = {read=fSyntaxErrorAttri, write=fSyntaxErrorAttri};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlighterm3 */
using namespace Synhighlighterm3;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterM3
