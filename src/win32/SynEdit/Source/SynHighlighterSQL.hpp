// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterSQL.pas' rev: 6.00

#ifndef SynHighlighterSQLHPP
#define SynHighlighterSQLHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynHighlighterHashEntries.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Registry.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlightersql
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkDatatype, tkDefaultPackage, tkException, tkFunction, tkIdentifier, tkKey, tkNull, tkNumber, tkSpace, tkPLSQL, tkSQLPlus, tkString, tkSymbol, tkTableName, tkUnknown, tkVariable };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsUnknown, rsComment, rsString };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

#pragma option push -b-
enum TSQLDialect { sqlStandard, sqlInterbase6, sqlMSSQL7, sqlMySQL, sqlOracle, sqlSybase, sqlIngres, sqlMSSQL2K };
#pragma option pop

typedef Byte TIdentifierTable[256];

typedef Byte *PIdentifierTable;

typedef int THashTable[256];

typedef int *PHashTable;

class DELPHICLASS TSynSQLSyn;
class PASCALIMPLEMENTATION TSynSQLSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TRangeState fRange;
	char *fLine;
	int fLineNumber;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind fTokenID;
	Synhighlighterhashentries::TSynHashEntryList* fKeywords;
	Classes::TStrings* fTableNames;
	TSQLDialect fDialect;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDataTypeAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDefaultPackageAttri;
	Synedithighlighter::TSynHighlighterAttributes* fExceptionAttri;
	Synedithighlighter::TSynHighlighterAttributes* fFunctionAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fPLSQLAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSQLPlusAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fTableNameAttri;
	Synedithighlighter::TSynHighlighterAttributes* fVariableAttri;
	Byte *fIdentifiersPtr;
	int *fmHashTablePtr;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	void __fastcall AndSymbolProc(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall CRProc(void);
	void __fastcall EqualProc(void);
	void __fastcall GreaterProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall LowerProc(void);
	void __fastcall MinusProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall OrSymbolProc(void);
	void __fastcall PlusProc(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall SymbolProc(void);
	void __fastcall SymbolAssignProc(void);
	void __fastcall VariableProc(void);
	void __fastcall UnknownProc(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	void __fastcall AnsiCProc(void);
	void __fastcall DoAddKeyword(AnsiString AKeyword, int AKind);
	void __fastcall SetDialect(TSQLDialect Value);
	void __fastcall SetTableNames(const Classes::TStrings* Value);
	void __fastcall TableNamesChanged(System::TObject* Sender);
	void __fastcall InitializeKeywordLists(void);
	void __fastcall PutTableNamesInKeywordList(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynSQLSyn(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynSQLSyn(void);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual void * __fastcall GetRange(void);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual bool __fastcall IsKeyword(const AnsiString AKeyword);
	virtual void __fastcall Next(void);
	virtual void __fastcall ResetRange(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual void __fastcall SetRange(void * Value);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DataTypeAttri = {read=fDataTypeAttri, write=fDataTypeAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DefaultPackageAttri = {read=fDefaultPackageAttri, write=fDefaultPackageAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* ExceptionAttri = {read=fExceptionAttri, write=fExceptionAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* FunctionAttri = {read=fFunctionAttri, write=fFunctionAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* PLSQLAttri = {read=fPLSQLAttri, write=fPLSQLAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SQLPlusAttri = {read=fSQLPlusAttri, write=fSQLPlusAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* TableNameAttri = {read=fTableNameAttri, write=fTableNameAttri};
	__property Classes::TStrings* TableNames = {read=fTableNames, write=SetTableNames};
	__property Synedithighlighter::TSynHighlighterAttributes* VariableAttri = {read=fVariableAttri, write=fVariableAttri};
	__property TSQLDialect SQLDialect = {read=fDialect, write=SetDialect, nodefault};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlightersql */
using namespace Synhighlightersql;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterSQL
