// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterProgress.pas' rev: 6.00

#ifndef SynHighlighterProgressHPP
#define SynHighlighterProgressHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynHighlighterHashEntries.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlighterprogress
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkEvent, tkIdentifier, tkInclude, tkKey, tkNonReserved, tkNull, tkNumber, tkPreprocessor, tkSpace, tkDataType, tkString, tkSymbol, tkUnknown };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsNone, rsInclude, rsPreprocessorDef, rsPreprocessor, rsComment };
#pragma option pop

#pragma pack(push, 1)
struct TRangeInfo
{
	
	union
	{
		struct 
		{
			Word Range;
			Word Level;
			
		};
		struct 
		{
			void *Ptr;
			
		};
		
	};
} ;
#pragma pack(pop)

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

class DELPHICLASS TSynProgressSyn;
class PASCALIMPLEMENTATION TSynProgressSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	char *fLine;
	int fLineNumber;
	TRangeState fRange;
	int fCommentLevel;
	int fIncludeLevel;
	int fPreProcessorLevel;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	Synedittypes::TSynIdentChars fIdentChars;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind FTokenID;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fEventAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIncludeAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNonReservedKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fPreprocessorAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDataTypeAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synhighlighterhashentries::TSynHashEntryList* fHashList;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	void __fastcall DoAddKeyword(AnsiString AKeyword, int AKind);
	void __fastcall AsciiCharProc(void);
	void __fastcall CommentRangeProc(void);
	void __fastcall IncludeRangeProc(void);
	void __fastcall PreprocessorRangeProc(void);
	void __fastcall PreprocessorDefinitionProc(void);
	void __fastcall PreprocessorDefinitionRangeProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall IdentProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall UnknownProc(void);
	void __fastcall SymbolProc(void);
	
protected:
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynProgressSyn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynProgressSyn(void);
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
	__property Synedittypes::TSynIdentChars IdentChars = {read=GetIdentChars, write=fIdentChars};
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* EventAttri = {read=fEventAttri, write=fEventAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IncludeAttri = {read=fIncludeAttri, write=fIncludeAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NonReservedKeyAttri = {read=fNonReservedKeyAttri, write=fNonReservedKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* PreprocessorAttri = {read=fPreprocessorAttri, write=fPreprocessorAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DataTypeAttri = {read=fDataTypeAttri, write=fDataTypeAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE AnsiString DefaultKeywords;
extern PACKAGE AnsiString DefaultNonReservedKeywords;
extern PACKAGE AnsiString DefaultEvents;
extern PACKAGE AnsiString DefaultDataTypes;

}	/* namespace Synhighlighterprogress */
using namespace Synhighlighterprogress;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterProgress
