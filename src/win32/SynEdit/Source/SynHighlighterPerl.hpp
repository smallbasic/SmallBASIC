// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterPerl.pas' rev: 6.00

#ifndef SynHighlighterPerlHPP
#define SynHighlighterPerlHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlighterperl
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TtkTokenKind { tkComment, tkIdentifier, tkKey, tkNull, tkNumber, tkOperator, tkPragma, tkSpace, tkString, tkSymbol, tkUnknown, tkVariable };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TtkTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

class DELPHICLASS TSynPerlSyn;
class PASCALIMPLEMENTATION TSynPerlSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	char *fLine;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	char *fToIdent;
	int fTokenPos;
	TtkTokenKind FTokenID;
	TIdentFuncTableFunc fIdentFuncTable[2168];
	int fLineNumber;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fInvalidAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fOperatorAttri;
	Synedithighlighter::TSynHighlighterAttributes* fPragmaAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fVariableAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TtkTokenKind __fastcall Func109(void);
	TtkTokenKind __fastcall Func113(void);
	TtkTokenKind __fastcall Func196(void);
	TtkTokenKind __fastcall Func201(void);
	TtkTokenKind __fastcall Func204(void);
	TtkTokenKind __fastcall Func207(void);
	TtkTokenKind __fastcall Func209(void);
	TtkTokenKind __fastcall Func211(void);
	TtkTokenKind __fastcall Func214(void);
	TtkTokenKind __fastcall Func216(void);
	TtkTokenKind __fastcall Func219(void);
	TtkTokenKind __fastcall Func221(void);
	TtkTokenKind __fastcall Func224(void);
	TtkTokenKind __fastcall Func225(void);
	TtkTokenKind __fastcall Func226(void);
	TtkTokenKind __fastcall Func230(void);
	TtkTokenKind __fastcall Func232(void);
	TtkTokenKind __fastcall Func233(void);
	TtkTokenKind __fastcall Func248(void);
	TtkTokenKind __fastcall Func254(void);
	TtkTokenKind __fastcall Func255(void);
	TtkTokenKind __fastcall Func257(void);
	TtkTokenKind __fastcall Func262(void);
	TtkTokenKind __fastcall Func263(void);
	TtkTokenKind __fastcall Func269(void);
	TtkTokenKind __fastcall Func280(void);
	TtkTokenKind __fastcall Func282(void);
	TtkTokenKind __fastcall Func306(void);
	TtkTokenKind __fastcall Func307(void);
	TtkTokenKind __fastcall Func310(void);
	TtkTokenKind __fastcall Func314(void);
	TtkTokenKind __fastcall Func317(void);
	TtkTokenKind __fastcall Func318(void);
	TtkTokenKind __fastcall Func320(void);
	TtkTokenKind __fastcall Func322(void);
	TtkTokenKind __fastcall Func325(void);
	TtkTokenKind __fastcall Func326(void);
	TtkTokenKind __fastcall Func327(void);
	TtkTokenKind __fastcall Func330(void);
	TtkTokenKind __fastcall Func331(void);
	TtkTokenKind __fastcall Func333(void);
	TtkTokenKind __fastcall Func335(void);
	TtkTokenKind __fastcall Func337(void);
	TtkTokenKind __fastcall Func338(void);
	TtkTokenKind __fastcall Func340(void);
	TtkTokenKind __fastcall Func345(void);
	TtkTokenKind __fastcall Func346(void);
	TtkTokenKind __fastcall Func368(void);
	TtkTokenKind __fastcall Func401(void);
	TtkTokenKind __fastcall Func412(void);
	TtkTokenKind __fastcall Func413(void);
	TtkTokenKind __fastcall Func415(void);
	TtkTokenKind __fastcall Func419(void);
	TtkTokenKind __fastcall Func420(void);
	TtkTokenKind __fastcall Func421(void);
	TtkTokenKind __fastcall Func424(void);
	TtkTokenKind __fastcall Func425(void);
	TtkTokenKind __fastcall Func426(void);
	TtkTokenKind __fastcall Func428(void);
	TtkTokenKind __fastcall Func430(void);
	TtkTokenKind __fastcall Func431(void);
	TtkTokenKind __fastcall Func432(void);
	TtkTokenKind __fastcall Func433(void);
	TtkTokenKind __fastcall Func434(void);
	TtkTokenKind __fastcall Func436(void);
	TtkTokenKind __fastcall Func437(void);
	TtkTokenKind __fastcall Func438(void);
	TtkTokenKind __fastcall Func439(void);
	TtkTokenKind __fastcall Func440(void);
	TtkTokenKind __fastcall Func441(void);
	TtkTokenKind __fastcall Func442(void);
	TtkTokenKind __fastcall Func444(void);
	TtkTokenKind __fastcall Func445(void);
	TtkTokenKind __fastcall Func447(void);
	TtkTokenKind __fastcall Func448(void);
	TtkTokenKind __fastcall Func456(void);
	TtkTokenKind __fastcall Func458(void);
	TtkTokenKind __fastcall Func470(void);
	TtkTokenKind __fastcall Func477(void);
	TtkTokenKind __fastcall Func502(void);
	TtkTokenKind __fastcall Func522(void);
	TtkTokenKind __fastcall Func523(void);
	TtkTokenKind __fastcall Func525(void);
	TtkTokenKind __fastcall Func527(void);
	TtkTokenKind __fastcall Func530(void);
	TtkTokenKind __fastcall Func531(void);
	TtkTokenKind __fastcall Func534(void);
	TtkTokenKind __fastcall Func535(void);
	TtkTokenKind __fastcall Func536(void);
	TtkTokenKind __fastcall Func537(void);
	TtkTokenKind __fastcall Func539(void);
	TtkTokenKind __fastcall Func542(void);
	TtkTokenKind __fastcall Func543(void);
	TtkTokenKind __fastcall Func545(void);
	TtkTokenKind __fastcall Func546(void);
	TtkTokenKind __fastcall Func547(void);
	TtkTokenKind __fastcall Func548(void);
	TtkTokenKind __fastcall Func549(void);
	TtkTokenKind __fastcall Func552(void);
	TtkTokenKind __fastcall Func555(void);
	TtkTokenKind __fastcall Func556(void);
	TtkTokenKind __fastcall Func557(void);
	TtkTokenKind __fastcall Func562(void);
	TtkTokenKind __fastcall Func569(void);
	TtkTokenKind __fastcall Func570(void);
	TtkTokenKind __fastcall Func622(void);
	TtkTokenKind __fastcall Func624(void);
	TtkTokenKind __fastcall Func627(void);
	TtkTokenKind __fastcall Func630(void);
	TtkTokenKind __fastcall Func632(void);
	TtkTokenKind __fastcall Func637(void);
	TtkTokenKind __fastcall Func640(void);
	TtkTokenKind __fastcall Func642(void);
	TtkTokenKind __fastcall Func643(void);
	TtkTokenKind __fastcall Func645(void);
	TtkTokenKind __fastcall Func647(void);
	TtkTokenKind __fastcall Func648(void);
	TtkTokenKind __fastcall Func649(void);
	TtkTokenKind __fastcall Func650(void);
	TtkTokenKind __fastcall Func651(void);
	TtkTokenKind __fastcall Func652(void);
	TtkTokenKind __fastcall Func655(void);
	TtkTokenKind __fastcall Func656(void);
	TtkTokenKind __fastcall Func657(void);
	TtkTokenKind __fastcall Func658(void);
	TtkTokenKind __fastcall Func665(void);
	TtkTokenKind __fastcall Func666(void);
	TtkTokenKind __fastcall Func667(void);
	TtkTokenKind __fastcall Func672(void);
	TtkTokenKind __fastcall Func675(void);
	TtkTokenKind __fastcall Func677(void);
	TtkTokenKind __fastcall Func687(void);
	TtkTokenKind __fastcall Func688(void);
	TtkTokenKind __fastcall Func716(void);
	TtkTokenKind __fastcall Func719(void);
	TtkTokenKind __fastcall Func727(void);
	TtkTokenKind __fastcall Func728(void);
	TtkTokenKind __fastcall Func731(void);
	TtkTokenKind __fastcall Func734(void);
	TtkTokenKind __fastcall Func740(void);
	TtkTokenKind __fastcall Func741(void);
	TtkTokenKind __fastcall Func743(void);
	TtkTokenKind __fastcall Func746(void);
	TtkTokenKind __fastcall Func749(void);
	TtkTokenKind __fastcall Func750(void);
	TtkTokenKind __fastcall Func752(void);
	TtkTokenKind __fastcall Func753(void);
	TtkTokenKind __fastcall Func754(void);
	TtkTokenKind __fastcall Func759(void);
	TtkTokenKind __fastcall Func761(void);
	TtkTokenKind __fastcall Func762(void);
	TtkTokenKind __fastcall Func763(void);
	TtkTokenKind __fastcall Func764(void);
	TtkTokenKind __fastcall Func765(void);
	TtkTokenKind __fastcall Func768(void);
	TtkTokenKind __fastcall Func769(void);
	TtkTokenKind __fastcall Func773(void);
	TtkTokenKind __fastcall Func774(void);
	TtkTokenKind __fastcall Func775(void);
	TtkTokenKind __fastcall Func815(void);
	TtkTokenKind __fastcall Func821(void);
	TtkTokenKind __fastcall Func841(void);
	TtkTokenKind __fastcall Func842(void);
	TtkTokenKind __fastcall Func845(void);
	TtkTokenKind __fastcall Func853(void);
	TtkTokenKind __fastcall Func855(void);
	TtkTokenKind __fastcall Func857(void);
	TtkTokenKind __fastcall Func860(void);
	TtkTokenKind __fastcall Func864(void);
	TtkTokenKind __fastcall Func867(void);
	TtkTokenKind __fastcall Func868(void);
	TtkTokenKind __fastcall Func869(void);
	TtkTokenKind __fastcall Func870(void);
	TtkTokenKind __fastcall Func873(void);
	TtkTokenKind __fastcall Func874(void);
	TtkTokenKind __fastcall Func876(void);
	TtkTokenKind __fastcall Func877(void);
	TtkTokenKind __fastcall Func878(void);
	TtkTokenKind __fastcall Func881(void);
	TtkTokenKind __fastcall Func883(void);
	TtkTokenKind __fastcall Func890(void);
	TtkTokenKind __fastcall Func892(void);
	TtkTokenKind __fastcall Func906(void);
	TtkTokenKind __fastcall Func933(void);
	TtkTokenKind __fastcall Func954(void);
	TtkTokenKind __fastcall Func956(void);
	TtkTokenKind __fastcall Func965(void);
	TtkTokenKind __fastcall Func968(void);
	TtkTokenKind __fastcall Func974(void);
	TtkTokenKind __fastcall Func978(void);
	TtkTokenKind __fastcall Func981(void);
	TtkTokenKind __fastcall Func985(void);
	TtkTokenKind __fastcall Func986(void);
	TtkTokenKind __fastcall Func988(void);
	TtkTokenKind __fastcall Func1056(void);
	TtkTokenKind __fastcall Func1077(void);
	TtkTokenKind __fastcall Func1079(void);
	TtkTokenKind __fastcall Func1084(void);
	TtkTokenKind __fastcall Func1086(void);
	TtkTokenKind __fastcall Func1091(void);
	TtkTokenKind __fastcall Func1093(void);
	TtkTokenKind __fastcall Func1095(void);
	TtkTokenKind __fastcall Func1103(void);
	TtkTokenKind __fastcall Func1105(void);
	TtkTokenKind __fastcall Func1107(void);
	TtkTokenKind __fastcall Func1136(void);
	TtkTokenKind __fastcall Func1158(void);
	TtkTokenKind __fastcall Func1165(void);
	TtkTokenKind __fastcall Func1169(void);
	TtkTokenKind __fastcall Func1172(void);
	TtkTokenKind __fastcall Func1176(void);
	TtkTokenKind __fastcall Func1202(void);
	TtkTokenKind __fastcall Func1211(void);
	TtkTokenKind __fastcall Func1215(void);
	TtkTokenKind __fastcall Func1218(void);
	TtkTokenKind __fastcall Func1223(void);
	TtkTokenKind __fastcall Func1230(void);
	TtkTokenKind __fastcall Func1273(void);
	TtkTokenKind __fastcall Func1277(void);
	TtkTokenKind __fastcall Func1283(void);
	TtkTokenKind __fastcall Func1327(void);
	TtkTokenKind __fastcall Func1343(void);
	TtkTokenKind __fastcall Func1361(void);
	TtkTokenKind __fastcall Func1379(void);
	TtkTokenKind __fastcall Func1396(void);
	TtkTokenKind __fastcall Func1402(void);
	TtkTokenKind __fastcall Func1404(void);
	TtkTokenKind __fastcall Func1409(void);
	TtkTokenKind __fastcall Func1421(void);
	TtkTokenKind __fastcall Func1425(void);
	TtkTokenKind __fastcall Func1440(void);
	TtkTokenKind __fastcall Func1520(void);
	TtkTokenKind __fastcall Func1523(void);
	TtkTokenKind __fastcall Func1673(void);
	TtkTokenKind __fastcall Func1752(void);
	TtkTokenKind __fastcall Func1762(void);
	TtkTokenKind __fastcall Func1768(void);
	TtkTokenKind __fastcall Func2167(void);
	void __fastcall AndSymbolProc(void);
	void __fastcall CRProc(void);
	void __fastcall ColonProc(void);
	void __fastcall CommentProc(void);
	void __fastcall EqualProc(void);
	void __fastcall GreaterProc(void);
	void __fastcall IdentProc(void);
	void __fastcall LFProc(void);
	void __fastcall LowerProc(void);
	void __fastcall MinusProc(void);
	void __fastcall NotSymbolProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall OrSymbolProc(void);
	void __fastcall PlusProc(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StarProc(void);
	void __fastcall StringInterpProc(void);
	void __fastcall StringLiteralProc(void);
	void __fastcall SymbolProc(void);
	void __fastcall XOrSymbolProc(void);
	void __fastcall UnknownProc(void);
	TtkTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TtkTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual AnsiString __fastcall GetSampleSource();
	
public:
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynPerlSyn(Classes::TComponent* AOwner);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	TtkTokenKind __fastcall GetTokenID(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* InvalidAttri = {read=fInvalidAttri, write=fInvalidAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* OperatorAttri = {read=fOperatorAttri, write=fOperatorAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* PragmaAttri = {read=fPragmaAttri, write=fPragmaAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* VariableAttri = {read=fVariableAttri, write=fVariableAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynPerlSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlighterperl */
using namespace Synhighlighterperl;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterPerl
