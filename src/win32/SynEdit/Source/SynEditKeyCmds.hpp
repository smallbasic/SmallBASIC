// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditKeyCmds.pas' rev: 6.00

#ifndef SynEditKeyCmdsHPP
#define SynEditKeyCmdsHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SysUtils.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditkeycmds
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS ESynKeyError;
class PASCALIMPLEMENTATION ESynKeyError : public Sysutils::Exception 
{
	typedef Sysutils::Exception inherited;
	
public:
	#pragma option push -w-inl
	/* Exception.Create */ inline __fastcall ESynKeyError(const AnsiString Msg) : Sysutils::Exception(Msg) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmt */ inline __fastcall ESynKeyError(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size) : Sysutils::Exception(Msg, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateRes */ inline __fastcall ESynKeyError(int Ident)/* overload */ : Sysutils::Exception(Ident) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmt */ inline __fastcall ESynKeyError(int Ident, const System::TVarRec * Args, const int Args_Size)/* overload */ : Sysutils::Exception(Ident, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateHelp */ inline __fastcall ESynKeyError(const AnsiString Msg, int AHelpContext) : Sysutils::Exception(Msg, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmtHelp */ inline __fastcall ESynKeyError(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size, int AHelpContext) : Sysutils::Exception(Msg, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResHelp */ inline __fastcall ESynKeyError(int Ident, int AHelpContext)/* overload */ : Sysutils::Exception(Ident, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmtHelp */ inline __fastcall ESynKeyError(System::PResStringRec ResStringRec, const System::TVarRec * Args, const int Args_Size, int AHelpContext)/* overload */ : Sysutils::Exception(ResStringRec, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~ESynKeyError(void) { }
	#pragma option pop
	
};


typedef Word TSynEditorCommand;

class DELPHICLASS TSynEditKeyStroke;
class PASCALIMPLEMENTATION TSynEditKeyStroke : public Classes::TCollectionItem 
{
	typedef Classes::TCollectionItem inherited;
	
private:
	Word FKey;
	Classes::TShiftState FShift;
	Word FKey2;
	Classes::TShiftState FShift2;
	TSynEditorCommand FCommand;
	Classes::TShortCut __fastcall GetShortCut(void);
	Classes::TShortCut __fastcall GetShortCut2(void);
	void __fastcall SetCommand(const TSynEditorCommand Value);
	void __fastcall SetKey(const Word Value);
	void __fastcall SetKey2(const Word Value);
	void __fastcall SetShift(const Classes::TShiftState Value);
	void __fastcall SetShift2(const Classes::TShiftState Value);
	void __fastcall SetShortCut(const Classes::TShortCut Value);
	void __fastcall SetShortCut2(const Classes::TShortCut Value);
	
protected:
	virtual AnsiString __fastcall GetDisplayName();
	
public:
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	void __fastcall LoadFromStream(Classes::TStream* AStream);
	void __fastcall SaveToStream(Classes::TStream* AStream);
	__property Word Key = {read=FKey, write=SetKey, nodefault};
	__property Word Key2 = {read=FKey2, write=SetKey2, nodefault};
	__property Classes::TShiftState Shift = {read=FShift, write=SetShift, nodefault};
	__property Classes::TShiftState Shift2 = {read=FShift2, write=SetShift2, nodefault};
	
__published:
	__property TSynEditorCommand Command = {read=FCommand, write=SetCommand, nodefault};
	__property Classes::TShortCut ShortCut = {read=GetShortCut, write=SetShortCut, default=0};
	__property Classes::TShortCut ShortCut2 = {read=GetShortCut2, write=SetShortCut2, default=0};
public:
	#pragma option push -w-inl
	/* TCollectionItem.Create */ inline __fastcall virtual TSynEditKeyStroke(Classes::TCollection* Collection) : Classes::TCollectionItem(Collection) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCollectionItem.Destroy */ inline __fastcall virtual ~TSynEditKeyStroke(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynEditKeyStrokes;
class PASCALIMPLEMENTATION TSynEditKeyStrokes : public Classes::TCollection 
{
	typedef Classes::TCollection inherited;
	
public:
	TSynEditKeyStroke* operator[](int Index) { return Items[Index]; }
	
private:
	Classes::TPersistent* FOwner;
	HIDESBASE TSynEditKeyStroke* __fastcall GetItem(int Index);
	HIDESBASE void __fastcall SetItem(int Index, TSynEditKeyStroke* Value);
	
protected:
	DYNAMIC Classes::TPersistent* __fastcall GetOwner(void);
	
public:
	__fastcall TSynEditKeyStrokes(Classes::TPersistent* AOwner);
	HIDESBASE TSynEditKeyStroke* __fastcall Add(void);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	int __fastcall FindCommand(TSynEditorCommand Cmd);
	int __fastcall FindKeycode(Word Code, Classes::TShiftState SS);
	int __fastcall FindKeycode2(Word Code1, Classes::TShiftState SS1, Word Code2, Classes::TShiftState SS2);
	int __fastcall FindShortcut(Classes::TShortCut SC);
	int __fastcall FindShortcut2(Classes::TShortCut SC, Classes::TShortCut SC2);
	void __fastcall LoadFromStream(Classes::TStream* AStream);
	void __fastcall ResetDefaults(void);
	void __fastcall SaveToStream(Classes::TStream* AStream);
	__property TSynEditKeyStroke* Items[int Index] = {read=GetItem, write=SetItem/*, default*/};
public:
	#pragma option push -w-inl
	/* TCollection.Destroy */ inline __fastcall virtual ~TSynEditKeyStrokes(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
static const Shortint ecNone = 0x0;
static const Shortint ecViewCommandFirst = 0x0;
static const Word ecViewCommandLast = 0x1f4;
static const Word ecEditCommandFirst = 0x1f5;
static const Word ecEditCommandLast = 0x3e8;
static const Shortint ecLeft = 0x1;
static const Shortint ecRight = 0x2;
static const Shortint ecUp = 0x3;
static const Shortint ecDown = 0x4;
static const Shortint ecWordLeft = 0x5;
static const Shortint ecWordRight = 0x6;
static const Shortint ecLineStart = 0x7;
static const Shortint ecLineEnd = 0x8;
static const Shortint ecPageUp = 0x9;
static const Shortint ecPageDown = 0xa;
static const Shortint ecPageLeft = 0xb;
static const Shortint ecPageRight = 0xc;
static const Shortint ecPageTop = 0xd;
static const Shortint ecPageBottom = 0xe;
static const Shortint ecEditorTop = 0xf;
static const Shortint ecEditorBottom = 0x10;
static const Shortint ecGotoXY = 0x11;
static const Shortint ecSelection = 0x64;
static const Shortint ecSelLeft = 0x65;
static const Shortint ecSelRight = 0x66;
static const Shortint ecSelUp = 0x67;
static const Shortint ecSelDown = 0x68;
static const Shortint ecSelWordLeft = 0x69;
static const Shortint ecSelWordRight = 0x6a;
static const Shortint ecSelLineStart = 0x6b;
static const Shortint ecSelLineEnd = 0x6c;
static const Shortint ecSelPageUp = 0x6d;
static const Shortint ecSelPageDown = 0x6e;
static const Shortint ecSelPageLeft = 0x6f;
static const Shortint ecSelPageRight = 0x70;
static const Shortint ecSelPageTop = 0x71;
static const Shortint ecSelPageBottom = 0x72;
static const Shortint ecSelEditorTop = 0x73;
static const Shortint ecSelEditorBottom = 0x74;
static const Shortint ecSelGotoXY = 0x75;
static const Byte ecSelectAll = 0xc7;
static const Byte ecCopy = 0xc9;
static const Byte ecScrollUp = 0xd3;
static const Byte ecScrollDown = 0xd4;
static const Byte ecScrollLeft = 0xd5;
static const Byte ecScrollRight = 0xd6;
static const Byte ecInsertMode = 0xdd;
static const Byte ecOverwriteMode = 0xde;
static const Byte ecToggleMode = 0xdf;
static const Byte ecNormalSelect = 0xe7;
static const Byte ecColumnSelect = 0xe8;
static const Byte ecLineSelect = 0xe9;
static const Byte ecMatchBracket = 0xfa;
static const Word ecGotoMarker0 = 0x12d;
static const Word ecGotoMarker1 = 0x12e;
static const Word ecGotoMarker2 = 0x12f;
static const Word ecGotoMarker3 = 0x130;
static const Word ecGotoMarker4 = 0x131;
static const Word ecGotoMarker5 = 0x132;
static const Word ecGotoMarker6 = 0x133;
static const Word ecGotoMarker7 = 0x134;
static const Word ecGotoMarker8 = 0x135;
static const Word ecGotoMarker9 = 0x136;
static const Word ecSetMarker0 = 0x15f;
static const Word ecSetMarker1 = 0x160;
static const Word ecSetMarker2 = 0x161;
static const Word ecSetMarker3 = 0x162;
static const Word ecSetMarker4 = 0x163;
static const Word ecSetMarker5 = 0x164;
static const Word ecSetMarker6 = 0x165;
static const Word ecSetMarker7 = 0x166;
static const Word ecSetMarker8 = 0x167;
static const Word ecSetMarker9 = 0x168;
static const Word ecContextHelp = 0x1ea;
static const Word ecDeleteLastChar = 0x1f5;
static const Word ecDeleteChar = 0x1f6;
static const Word ecDeleteWord = 0x1f7;
static const Word ecDeleteLastWord = 0x1f8;
static const Word ecDeleteBOL = 0x1f9;
static const Word ecDeleteEOL = 0x1fa;
static const Word ecDeleteLine = 0x1fb;
static const Word ecClearAll = 0x1fc;
static const Word ecLineBreak = 0x1fd;
static const Word ecInsertLine = 0x1fe;
static const Word ecChar = 0x1ff;
static const Word ecImeStr = 0x226;
static const Word ecUndo = 0x259;
static const Word ecRedo = 0x25a;
static const Word ecCut = 0x25b;
static const Word ecPaste = 0x25c;
static const Word ecBlockIndent = 0x262;
static const Word ecBlockUnindent = 0x263;
static const Word ecTab = 0x264;
static const Word ecShiftTab = 0x265;
static const Word ecAutoCompletion = 0x28a;
static const Word ecUpperCase = 0x26c;
static const Word ecLowerCase = 0x26d;
static const Word ecToggleCase = 0x26e;
static const Word ecTitleCase = 0x26f;
static const Word ecString = 0x276;
static const Word ecGotFocus = 0x2bc;
static const Word ecLostFocus = 0x2bd;
static const Word ecUserFirst = 0x3e9;
extern PACKAGE void __fastcall GetEditorCommandValues(Classes::TGetStrProc Proc);
extern PACKAGE void __fastcall GetEditorCommandExtended(Classes::TGetStrProc Proc);
extern PACKAGE bool __fastcall IdentToEditorCommand(const AnsiString Ident, int &Cmd);
extern PACKAGE bool __fastcall EditorCommandToIdent(int Cmd, AnsiString &Ident);
extern PACKAGE AnsiString __fastcall EditorCommandToDescrString(TSynEditorCommand Cmd);
extern PACKAGE AnsiString __fastcall EditorCommandToCodeString(TSynEditorCommand Cmd);
extern PACKAGE AnsiString __fastcall ConvertCodeStringToExtended(AnsiString AString);
extern PACKAGE AnsiString __fastcall ConvertExtendedToCodeString(AnsiString AString);
extern PACKAGE int __fastcall IndexToEditorCommand(const int AIndex);
extern PACKAGE TSynEditorCommand __fastcall ConvertExtendedToCommand(AnsiString AString);
extern PACKAGE TSynEditorCommand __fastcall ConvertCodeStringToCommand(AnsiString AString);

}	/* namespace Syneditkeycmds */
using namespace Syneditkeycmds;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditKeyCmds
