// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditHighlighter.pas' rev: 6.00

#ifndef SynEditHighlighterHPP
#define SynEditHighlighterHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditMiscClasses.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <IniFiles.hpp>	// Pascal unit
#include <Registry.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synedithighlighter
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TBetterRegistry;
class PASCALIMPLEMENTATION TBetterRegistry : public Registry::TRegistry 
{
	typedef Registry::TRegistry inherited;
	
public:
	#pragma option push -w-inl
	/* TRegistry.Create */ inline __fastcall TBetterRegistry(void)/* overload */ : Registry::TRegistry() { }
	#pragma option pop
	#pragma option push -w-inl
	/* TRegistry.Destroy */ inline __fastcall virtual ~TBetterRegistry(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynHighlighterAttributes;
class PASCALIMPLEMENTATION TSynHighlighterAttributes : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	Graphics::TColor fBackground;
	Graphics::TColor fBackgroundDefault;
	Graphics::TColor fForeground;
	Graphics::TColor fForegroundDefault;
	AnsiString fName;
	Graphics::TFontStyles fStyle;
	Graphics::TFontStyles fStyleDefault;
	Classes::TNotifyEvent fOnChange;
	virtual void __fastcall Changed(void);
	bool __fastcall GetBackgroundColorStored(void);
	bool __fastcall GetForegroundColorStored(void);
	bool __fastcall GetFontStyleStored(void);
	void __fastcall SetBackground(Graphics::TColor Value);
	void __fastcall SetForeground(Graphics::TColor Value);
	void __fastcall SetStyle(Graphics::TFontStyles Value);
	int __fastcall GetStyleFromInt(void);
	void __fastcall SetStyleFromInt(const int Value);
	
public:
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	__fastcall TSynHighlighterAttributes(AnsiString attribName);
	void __fastcall InternalSaveDefaultValues(void);
	virtual bool __fastcall LoadFromBorlandRegistry(HKEY rootKey, AnsiString attrKey, AnsiString attrName, bool oldStyle);
	bool __fastcall LoadFromRegistry(TBetterRegistry* Reg);
	bool __fastcall SaveToRegistry(TBetterRegistry* Reg);
	bool __fastcall LoadFromFile(Inifiles::TIniFile* Ini);
	bool __fastcall SaveToFile(Inifiles::TIniFile* Ini);
	__property int IntegerStyle = {read=GetStyleFromInt, write=SetStyleFromInt, nodefault};
	__property AnsiString Name = {read=fName};
	__property Classes::TNotifyEvent OnChange = {read=fOnChange, write=fOnChange};
	
__published:
	__property Graphics::TColor Background = {read=fBackground, write=SetBackground, stored=GetBackgroundColorStored, nodefault};
	__property Graphics::TColor Foreground = {read=fForeground, write=SetForeground, stored=GetForegroundColorStored, nodefault};
	__property Graphics::TFontStyles Style = {read=fStyle, write=SetStyle, stored=GetFontStyleStored, nodefault};
public:
	#pragma option push -w-inl
	/* TPersistent.Destroy */ inline __fastcall virtual ~TSynHighlighterAttributes(void) { }
	#pragma option pop
	
};


#pragma option push -b-
enum TSynHighlighterCapability { hcUserSettings, hcRegistry };
#pragma option pop

typedef Set<TSynHighlighterCapability, hcUserSettings, hcRegistry>  TSynHighlighterCapabilities;

class DELPHICLASS TSynCustomHighlighter;
class PASCALIMPLEMENTATION TSynCustomHighlighter : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	Classes::TStringList* fAttributes;
	Syneditmiscclasses::TSynNotifyEventChain* fAttrChangeHooks;
	int fUpdateCount;
	bool fEnabled;
	Synedittypes::TSynIdentChars fWordBreakChars;
	void __fastcall SetEnabled(const bool Value);
	
protected:
	AnsiString fDefaultFilter;
	bool fUpdateChange;
	void __fastcall AddAttribute(TSynHighlighterAttributes* AAttrib);
	void __fastcall DefHighlightChange(System::TObject* Sender);
	void __fastcall FreeHighlighterAttributes(void);
	virtual int __fastcall GetAttribCount(void);
	virtual TSynHighlighterAttributes* __fastcall GetAttribute(int idx);
	virtual TSynHighlighterAttributes* __fastcall GetDefaultAttribute(int Index) = 0 ;
	virtual AnsiString __fastcall GetDefaultFilter();
	virtual Synedittypes::TSynIdentChars __fastcall GetIdentChars();
	virtual void __fastcall SetWordBreakChars(const Synedittypes::TSynIdentChars &AChars);
	virtual AnsiString __fastcall GetSampleSource();
	virtual bool __fastcall IsFilterStored(void);
	void __fastcall SetAttributesOnChange(Classes::TNotifyEvent AEvent);
	virtual void __fastcall SetDefaultFilter(AnsiString Value);
	virtual void __fastcall SetSampleSource(AnsiString Value);
	
public:
	/* virtual class method */ virtual TSynHighlighterCapabilities __fastcall GetCapabilities(TMetaClass* vmt);
	/* virtual class method */ virtual AnsiString __fastcall GetLanguageName(TMetaClass* vmt);
	__fastcall virtual TSynCustomHighlighter(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynCustomHighlighter(void);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	void __fastcall BeginUpdate(void);
	void __fastcall EndUpdate(void);
	virtual bool __fastcall GetEol(void) = 0 ;
	virtual void * __fastcall GetRange(void);
	virtual AnsiString __fastcall GetToken(void) = 0 ;
	virtual TSynHighlighterAttributes* __fastcall GetTokenAttribute(void) = 0 ;
	virtual int __fastcall GetTokenKind(void) = 0 ;
	virtual int __fastcall GetTokenPos(void) = 0 ;
	virtual bool __fastcall IsKeyword(const AnsiString AKeyword);
	virtual void __fastcall Next(void) = 0 ;
	void __fastcall NextToEol(void);
	virtual void __fastcall SetLine(AnsiString NewValue, int LineNumber) = 0 ;
	virtual void __fastcall SetRange(void * Value);
	virtual void __fastcall ResetRange(void);
	virtual bool __fastcall UseUserSettings(int settingIndex);
	virtual void __fastcall EnumUserSettings(Classes::TStrings* Settings);
	virtual bool __fastcall LoadFromRegistry(HKEY RootKey, AnsiString Key);
	virtual bool __fastcall SaveToRegistry(HKEY RootKey, AnsiString Key);
	bool __fastcall LoadFromFile(AnsiString AFileName);
	bool __fastcall SaveToFile(AnsiString AFileName);
	void __fastcall HookAttrChangeEvent(Classes::TNotifyEvent ANotifyEvent);
	void __fastcall UnhookAttrChangeEvent(Classes::TNotifyEvent ANotifyEvent);
	__property Synedittypes::TSynIdentChars IdentChars = {read=GetIdentChars};
	__property Synedittypes::TSynIdentChars WordBreakChars = {read=fWordBreakChars, write=SetWordBreakChars};
//	__property AnsiString LanguageName = {read=GetLanguageName};
	__property int AttrCount = {read=GetAttribCount, nodefault};
	__property TSynHighlighterAttributes* Attribute[int idx] = {read=GetAttribute};
//	__property TSynHighlighterCapabilities Capabilities = {read=GetCapabilities, nodefault};
	__property AnsiString SampleSource = {read=GetSampleSource, write=SetSampleSource};
	__property TSynHighlighterAttributes* CommentAttribute = {read=GetDefaultAttribute, index=0};
	__property TSynHighlighterAttributes* IdentifierAttribute = {read=GetDefaultAttribute, index=1};
	__property TSynHighlighterAttributes* KeywordAttribute = {read=GetDefaultAttribute, index=2};
	__property TSynHighlighterAttributes* StringAttribute = {read=GetDefaultAttribute, index=3};
	__property TSynHighlighterAttributes* SymbolAttribute = {read=GetDefaultAttribute, index=5};
	__property TSynHighlighterAttributes* WhitespaceAttribute = {read=GetDefaultAttribute, index=4};
	
__published:
	__property AnsiString DefaultFilter = {read=GetDefaultFilter, write=SetDefaultFilter, stored=IsFilterStored};
	__property bool Enabled = {read=fEnabled, write=SetEnabled, default=1};
};


typedef TMetaClass*TSynCustomHighlighterClass;

class DELPHICLASS TSynHighlighterList;
class PASCALIMPLEMENTATION TSynHighlighterList : public Classes::TList 
{
	typedef Classes::TList inherited;
	
public:
	TMetaClass* operator[](int idx) { return Items[idx]; }
	
private:
	Classes::TList* hlList;
	TMetaClass* __fastcall GetItem(int idx);
	
public:
	__fastcall TSynHighlighterList(void);
	__fastcall virtual ~TSynHighlighterList(void);
	HIDESBASE int __fastcall Count(void);
	int __fastcall FindByName(AnsiString name);
	int __fastcall FindByClass(Classes::TComponent* comp);
	__property TMetaClass* Items[int idx] = {read=GetItem/*, default*/};
};


//-- var, const, procedure ---------------------------------------------------
static const Shortint SYN_ATTR_COMMENT = 0x0;
static const Shortint SYN_ATTR_IDENTIFIER = 0x1;
static const Shortint SYN_ATTR_KEYWORD = 0x2;
static const Shortint SYN_ATTR_STRING = 0x3;
static const Shortint SYN_ATTR_WHITESPACE = 0x4;
static const Shortint SYN_ATTR_SYMBOL = 0x5;
extern PACKAGE TSynHighlighterList* __fastcall GetPlaceableHighlighters(void);
extern PACKAGE void __fastcall RegisterPlaceableHighlighter(TMetaClass* highlighter);

}	/* namespace Synedithighlighter */
using namespace Synedithighlighter;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditHighlighter
