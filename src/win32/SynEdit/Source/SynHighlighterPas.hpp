// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterPas.pas' rev: 6.00

#ifndef SynHighlighterPasHPP
#define SynHighlighterPasHPP

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

namespace Synhighlighterpas
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkAsm, tkComment, tkIdentifier, tkKey, tkNull, tkNumber, tkSpace, tkString, tkSymbol, tkUnknown, tkFloat, tkHex, tkDirec, tkChar };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsANil, rsAnsi, rsAnsiAsm, rsAsm, rsBor, rsBorAsm, rsProperty, rsExports, rsUnKnown };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

#pragma option push -b-
enum TDelphiVersion { dvDelphi1, dvDelphi2, dvDelphi3, dvDelphi4, dvDelphi5, dvDelphi6 };
#pragma option pop

class DELPHICLASS TSynPasSyn;
class PASCALIMPLEMENTATION TSynPasSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	bool fAsmStart;
	TRangeState fRange;
	char *fLine;
	int fLineNumber;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	char *fToIdent;
	TIdentFuncTableFunc fIdentFuncTable[192];
	int fTokenPos;
	TtkTokenKind FTokenID;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCharAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fFloatAttri;
	Synedithighlighter::TSynHighlighterAttributes* fHexAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fAsmAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fDirecAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	bool fD4syntax;
	TDelphiVersion fDelphiVersion;
	bool fPackageSource;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func15(void);
	TtkTokenKind __fastcall Func19(void);
	TtkTokenKind __fastcall Func20(void);
	TtkTokenKind __fastcall Func21(void);
	TtkTokenKind __fastcall Func23(void);
	TtkTokenKind __fastcall Func25(void);
	TtkTokenKind __fastcall Func27(void);
	TtkTokenKind __fastcall Func28(void);
	TtkTokenKind __fastcall Func29(void);
	TtkTokenKind __fastcall Func32(void);
	TtkTokenKind __fastcall Func33(void);
	TtkTokenKind __fastcall Func35(void);
	TtkTokenKind __fastcall Func37(void);
	TtkTokenKind __fastcall Func38(void);
	TtkTokenKind __fastcall Func39(void);
	TtkTokenKind __fastcall Func40(void);
	TtkTokenKind __fastcall Func41(void);
	TtkTokenKind __fastcall Func44(void);
	TtkTokenKind __fastcall Func45(void);
	TtkTokenKind __fastcall Func47(void);
	TtkTokenKind __fastcall Func49(void);
	TtkTokenKind __fastcall Func52(void);
	TtkTokenKind __fastcall Func54(void);
	TtkTokenKind __fastcall Func55(void);
	TtkTokenKind __fastcall Func56(void);
	TtkTokenKind __fastcall Func57(void);
	TtkTokenKind __fastcall Func59(void);
	TtkTokenKind __fastcall Func60(void);
	TtkTokenKind __fastcall Func61(void);
	TtkTokenKind __fastcall Func63(void);
	TtkTokenKind __fastcall Func64(void);
	TtkTokenKind __fastcall Func65(void);
	TtkTokenKind __fastcall Func66(void);
	TtkTokenKind __fastcall Func69(void);
	TtkTokenKind __fastcall Func71(void);
	TtkTokenKind __fastcall Func73(void);
	TtkTokenKind __fastcall Func75(void);
	TtkTokenKind __fastcall Func76(void);
	TtkTokenKind __fastcall Func79(void);
	TtkTokenKind __fastcall Func81(void);
	TtkTokenKind __fastcall Func84(void);
	TtkTokenKind __fastcall Func85(void);
	TtkTokenKind __fastcall Func87(void);
	TtkTokenKind __fastcall Func88(void);
	TtkTokenKind __fastcall Func91(void);
	TtkTokenKind __fastcall Func92(void);
	TtkTokenKind __fastcall Func94(void);
	TtkTokenKind __fastcall Func95(void);
	TtkTokenKind __fastcall Func96(void);
	TtkTokenKind __fastcall Func97(void);
	TtkTokenKind __fastcall Func98(void);
	TtkTokenKind __fastcall Func99(void);
	TtkTokenKind __fastcall Func100(void);
	TtkTokenKind __fastcall Func101(void);
	TtkTokenKind __fastcall Func102(void);
	TtkTokenKind __fastcall Func103(void);
	TtkTokenKind __fastcall Func105(void);
	TtkTokenKind __fastcall Func106(void);
	TtkTokenKind __fastcall Func112(void);
	TtkTokenKind __fastcall Func117(void);
	TtkTokenKind __fastcall Func126(void);
	TtkTokenKind __fastcall Func129(void);
	TtkTokenKind __fastcall Func132(void);
	TtkTokenKind __fastcall Func133(void);
	TtkTokenKind __fastcall Func136(void);
	TtkTokenKind __fastcall Func141(void);
	TtkTokenKind __fastcall Func143(void);
	TtkTokenKind __fastcall Func166(void);
	TtkTokenKind __fastcall Func168(void);
	TtkTokenKind __fastcall Func191(void);
	TtkTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	void __fastcall AddressOpProc(void);
	void __fastcall AsciiCharProc(void);
	void __fastcall AnsiProc(void);
	void __fastcall BorProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall ColonOrGreaterProc(void);
	void __fastcall CRProc(void);
	void __fastcall IdentProc(void);
	void __fastcall IntegerProc(void);
	void __fastcall LFProc(void);
	void __fastcall LowerProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall PointProc(void);
	void __fastcall RoundOpenProc(void);
	void __fastcall SemicolonProc(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall SymbolProc(void);
	void __fastcall UnknownProc(void);
	void __fastcall SetD4syntax(const bool Value);
	void __fastcall SetDelphiVersion(const TDelphiVersion Value);
	void __fastcall SetPackageSource(const bool Value);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	virtual bool __fastcall IsFilterStored(void);
	
public:
	/* virtual class method */ virtual Synedithighlighter::TSynHighlighterCapabilities __fastcall GetCapabilities(TMetaClass* vmt);
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynPasSyn(Classes::TComponent* AOwner);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual void * __fastcall GetRange(void);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual void __fastcall ResetRange(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual void __fastcall SetRange(void * Value);
	virtual bool __fastcall UseUserSettings(int settingIndex);
	virtual void __fastcall EnumUserSettings(Classes::TStrings* settings);
	__property IdentChars ;
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* AsmAttri = {read=fAsmAttri, write=fAsmAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* DirectiveAttri = {read=fDirecAttri, write=fDirecAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* FloatAttri = {read=fFloatAttri, write=fFloatAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* HexAttri = {read=fHexAttri, write=fHexAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* CharAttri = {read=fCharAttri, write=fCharAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property bool D4syntax = {read=fD4syntax, write=SetD4syntax, stored=false, nodefault};
	__property TDelphiVersion DelphiVersion = {read=fDelphiVersion, write=SetDelphiVersion, default=5};
	__property bool PackageSource = {read=fPackageSource, write=SetPackageSource, default=1};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynPasSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
#define LastDelphiVersion (TDelphiVersion)(5)

}	/* namespace Synhighlighterpas */
using namespace Synhighlighterpas;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterPas
