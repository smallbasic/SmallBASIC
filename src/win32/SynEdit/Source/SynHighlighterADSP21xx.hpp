// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterADSP21xx.pas' rev: 6.00

#ifndef SynHighlighterADSP21xxHPP
#define SynHighlighterADSP21xxHPP

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

namespace Synhighlighteradsp21xx
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TTokenKind { tkComment, tkCondition, tkIdentifier, tkKey, tkNull, tkNumber, tkRegister, tkSpace, tkString, tkSymbol, tkUnknown };
#pragma option pop

#pragma option push -b-
enum TRangeState { rsUnKnown, rsPascalComment, rsCComment, rsHexNumber, rsBinaryNumber, rsInclude };
#pragma option pop

typedef void __fastcall (__closure *TProcTableProc)(void);

typedef TTokenKind __fastcall (__closure *TIdentFuncTableFunc)(void);

typedef TIdentFuncTableFunc *PIdentFuncTableFunc;

class DELPHICLASS TSynADSP21xxSyn;
class PASCALIMPLEMENTATION TSynADSP21xxSyn : public Synedithighlighter::TSynCustomHighlighter 
{
	typedef Synedithighlighter::TSynCustomHighlighter inherited;
	
private:
	TRangeState fRange;
	char *fLine;
	TProcTableProc fProcTable[256];
	int Run;
	int fStringLen;
	char *fToIdent;
	TIdentFuncTableFunc fIdentFuncTable[192];
	int fTokenPos;
	TTokenKind FTokenID;
	Synedithighlighter::TSynHighlighterAttributes* fNumberAttri;
	Synedithighlighter::TSynHighlighterAttributes* fStringAttri;
	Synedithighlighter::TSynHighlighterAttributes* fKeyAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSymbolAttri;
	Synedithighlighter::TSynHighlighterAttributes* fCommentAttri;
	Synedithighlighter::TSynHighlighterAttributes* fIdentifierAttri;
	Synedithighlighter::TSynHighlighterAttributes* fSpaceAttri;
	Synedithighlighter::TSynHighlighterAttributes* fRegisterAttri;
	Synedithighlighter::TSynHighlighterAttributes* fConditionAttri;
	Synedithighlighter::TSynHighlighterAttributes* fNullAttri;
	Synedithighlighter::TSynHighlighterAttributes* fUnknownAttri;
	int __fastcall KeyHash(char * ToHash);
	bool __fastcall KeyComp(const AnsiString aKey);
	TTokenKind __fastcall Func2(void);
	TTokenKind __fastcall Func4(void);
	TTokenKind __fastcall Func7(void);
	TTokenKind __fastcall Func8(void);
	TTokenKind __fastcall Func9(void);
	TTokenKind __fastcall Func12(void);
	TTokenKind __fastcall Func13(void);
	TTokenKind __fastcall Func15(void);
	TTokenKind __fastcall Func17(void);
	TTokenKind __fastcall Func18(void);
	TTokenKind __fastcall Func19(void);
	TTokenKind __fastcall Func20(void);
	TTokenKind __fastcall Func21(void);
	TTokenKind __fastcall Func22(void);
	TTokenKind __fastcall Func23(void);
	TTokenKind __fastcall Func24(void);
	TTokenKind __fastcall Func25(void);
	TTokenKind __fastcall Func26(void);
	TTokenKind __fastcall Func27(void);
	TTokenKind __fastcall Func28(void);
	TTokenKind __fastcall Func29(void);
	TTokenKind __fastcall Func30(void);
	TTokenKind __fastcall Func31(void);
	TTokenKind __fastcall Func32(void);
	TTokenKind __fastcall Func33(void);
	TTokenKind __fastcall Func35(void);
	TTokenKind __fastcall Func36(void);
	TTokenKind __fastcall Func37(void);
	TTokenKind __fastcall Func38(void);
	TTokenKind __fastcall Func39(void);
	TTokenKind __fastcall Func40(void);
	TTokenKind __fastcall Func41(void);
	TTokenKind __fastcall Func42(void);
	TTokenKind __fastcall Func43(void);
	TTokenKind __fastcall Func44(void);
	TTokenKind __fastcall Func45(void);
	TTokenKind __fastcall Func46(void);
	TTokenKind __fastcall Func47(void);
	TTokenKind __fastcall Func49(void);
	TTokenKind __fastcall Func50(void);
	TTokenKind __fastcall Func52(void);
	TTokenKind __fastcall Func53(void);
	TTokenKind __fastcall Func54(void);
	TTokenKind __fastcall Func55(void);
	TTokenKind __fastcall Func57(void);
	TTokenKind __fastcall Func58(void);
	TTokenKind __fastcall Func60(void);
	TTokenKind __fastcall Func61(void);
	TTokenKind __fastcall Func62(void);
	TTokenKind __fastcall Func63(void);
	TTokenKind __fastcall Func64(void);
	TTokenKind __fastcall Func65(void);
	TTokenKind __fastcall Func66(void);
	TTokenKind __fastcall Func67(void);
	TTokenKind __fastcall Func68(void);
	TTokenKind __fastcall Func69(void);
	TTokenKind __fastcall Func70(void);
	TTokenKind __fastcall Func71(void);
	TTokenKind __fastcall Func72(void);
	TTokenKind __fastcall Func73(void);
	TTokenKind __fastcall Func74(void);
	TTokenKind __fastcall Func75(void);
	TTokenKind __fastcall Func76(void);
	TTokenKind __fastcall Func79(void);
	TTokenKind __fastcall Func80(void);
	TTokenKind __fastcall Func81(void);
	TTokenKind __fastcall Func82(void);
	TTokenKind __fastcall Func83(void);
	TTokenKind __fastcall Func84(void);
	TTokenKind __fastcall Func88(void);
	TTokenKind __fastcall Func89(void);
	TTokenKind __fastcall Func90(void);
	TTokenKind __fastcall Func92(void);
	TTokenKind __fastcall Func94(void);
	TTokenKind __fastcall Func95(void);
	TTokenKind __fastcall Func98(void);
	TTokenKind __fastcall Func99(void);
	TTokenKind __fastcall Func107(void);
	TTokenKind __fastcall Func113(void);
	TTokenKind __fastcall Func145(void);
	TTokenKind __fastcall AltFunc(void);
	void __fastcall InitIdent(void);
	TTokenKind __fastcall IdentKind(char * MayBe);
	void __fastcall MakeMethodTables(void);
	void __fastcall PascalCommentProc(void);
	void __fastcall BraceCloseProc(void);
	void __fastcall BraceOpenProc(void);
	void __fastcall CCommentProc(void);
	void __fastcall CRProc(void);
	void __fastcall ExclamationProc(void);
	void __fastcall IdentProc(void);
	void __fastcall IntegerProc(void);
	void __fastcall IncludeCloseProc(void);
	void __fastcall LFProc(void);
	void __fastcall NullProc(void);
	void __fastcall NumberProc(void);
	void __fastcall BinaryNumber(void);
	void __fastcall HexNumber(void);
	void __fastcall SlashProc(void);
	void __fastcall SpaceProc(void);
	void __fastcall StringProc(void);
	void __fastcall UnknownProc(void);
	
protected:
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	
public:
	/* virtual class method */ virtual Synedithighlighter::TSynHighlighterCapabilities __fastcall GetCapabilities(TMetaClass* vmt);
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynADSP21xxSyn(Classes::TComponent* AOwner);
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index);
	virtual bool __fastcall GetEol(void);
	virtual void * __fastcall GetRange(void);
	TTokenKind __fastcall GetTokenID(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber);
	virtual AnsiString __fastcall GetToken();
	virtual Synedithighlighter::TSynHighlighterAttributes* __fastcall GetTokenAttribute(void);
	virtual int __fastcall GetTokenKind(void);
	virtual int __fastcall GetTokenPos(void);
	virtual void __fastcall Next(void);
	virtual void __fastcall SetRange(void * Value);
	virtual void __fastcall ResetRange(void);
	virtual bool __fastcall UseUserSettings(int settingIndex);
	virtual void __fastcall EnumUserSettings(Classes::TStrings* settings);
	__property IdentChars ;
	
__published:
	__property Synedithighlighter::TSynHighlighterAttributes* CommentAttri = {read=fCommentAttri, write=fCommentAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* ConditionAttri = {read=fConditionAttri, write=fConditionAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* IdentifierAttri = {read=fIdentifierAttri, write=fIdentifierAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* KeyAttri = {read=fKeyAttri, write=fKeyAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* NumberAttri = {read=fNumberAttri, write=fNumberAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* RegisterAttri = {read=fRegisterAttri, write=fRegisterAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* StringAttri = {read=fStringAttri, write=fStringAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SpaceAttri = {read=fSpaceAttri, write=fSpaceAttri};
	__property Synedithighlighter::TSynHighlighterAttributes* SymbolAttri = {read=fSymbolAttri, write=fSymbolAttri};
public:
	#pragma option push -w-inl
	/* TSynCustomHighlighter.Destroy */ inline __fastcall virtual ~TSynADSP21xxSyn(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synhighlighteradsp21xx */
using namespace Synhighlighteradsp21xx;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterADSP21xx
