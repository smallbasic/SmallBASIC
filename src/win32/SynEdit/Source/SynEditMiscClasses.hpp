// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditMiscClasses.pas' rev: 6.00

#ifndef SynEditMiscClassesHPP
#define SynEditMiscClassesHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SynEditKeyConst.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditmiscclasses
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynSelectedColor;
class PASCALIMPLEMENTATION TSynSelectedColor : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	Graphics::TColor fBG;
	Graphics::TColor fFG;
	Classes::TNotifyEvent fOnChange;
	void __fastcall SetBG(Graphics::TColor Value);
	void __fastcall SetFG(Graphics::TColor Value);
	
public:
	__fastcall TSynSelectedColor(void);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	
__published:
	__property Graphics::TColor Background = {read=fBG, write=SetBG, default=-2147483635};
	__property Graphics::TColor Foreground = {read=fFG, write=SetFG, default=-2147483634};
	__property Classes::TNotifyEvent OnChange = {read=fOnChange, write=fOnChange};
public:
	#pragma option push -w-inl
	/* TPersistent.Destroy */ inline __fastcall virtual ~TSynSelectedColor(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynGutter;
class PASCALIMPLEMENTATION TSynGutter : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	Graphics::TFont* fFont;
	Graphics::TColor fColor;
	int fWidth;
	bool fShowLineNumbers;
	int fDigitCount;
	bool fLeadingZeros;
	bool fZeroStart;
	int fLeftOffset;
	int fRightOffset;
	Classes::TNotifyEvent fOnChange;
	Controls::TCursor fCursor;
	bool fVisible;
	bool fUseFontStyle;
	bool fAutoSize;
	int fAutoSizeDigitCount;
	void __fastcall SetAutoSize(const bool Value);
	void __fastcall SetColor(const Graphics::TColor Value);
	void __fastcall SetDigitCount(int Value);
	void __fastcall SetLeadingZeros(const bool Value);
	void __fastcall SetLeftOffset(int Value);
	void __fastcall SetRightOffset(int Value);
	void __fastcall SetShowLineNumbers(const bool Value);
	void __fastcall SetUseFontStyle(bool Value);
	void __fastcall SetVisible(bool Value);
	void __fastcall SetWidth(int Value);
	void __fastcall SetZeroStart(const bool Value);
	void __fastcall SetFont(Graphics::TFont* Value);
	void __fastcall OnFontChange(System::TObject* Sender);
	
public:
	__fastcall TSynGutter(void);
	__fastcall virtual ~TSynGutter(void);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	void __fastcall AutoSizeDigitCount(int LinesCount);
	AnsiString __fastcall FormatLineNumber(int Line);
	int __fastcall RealGutterWidth(int CharWidth);
	
__published:
	__property bool AutoSize = {read=fAutoSize, write=SetAutoSize, default=0};
	__property Graphics::TColor Color = {read=fColor, write=SetColor, default=-2147483633};
	__property Controls::TCursor Cursor = {read=fCursor, write=fCursor, default=0};
	__property int DigitCount = {read=fDigitCount, write=SetDigitCount, default=4};
	__property Graphics::TFont* Font = {read=fFont, write=SetFont};
	__property bool LeadingZeros = {read=fLeadingZeros, write=SetLeadingZeros, default=0};
	__property int LeftOffset = {read=fLeftOffset, write=SetLeftOffset, default=16};
	__property int RightOffset = {read=fRightOffset, write=SetRightOffset, default=2};
	__property bool ShowLineNumbers = {read=fShowLineNumbers, write=SetShowLineNumbers, default=0};
	__property bool UseFontStyle = {read=fUseFontStyle, write=SetUseFontStyle, default=1};
	__property bool Visible = {read=fVisible, write=SetVisible, default=1};
	__property int Width = {read=fWidth, write=SetWidth, default=30};
	__property bool ZeroStart = {read=fZeroStart, write=SetZeroStart, default=0};
	__property Classes::TNotifyEvent OnChange = {read=fOnChange, write=fOnChange};
};


class DELPHICLASS TSynBookMarkOpt;
class PASCALIMPLEMENTATION TSynBookMarkOpt : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	Controls::TImageList* fBookmarkImages;
	bool fDrawBookmarksFirst;
	bool fEnableKeys;
	bool fGlyphsVisible;
	int fLeftMargin;
	Classes::TComponent* fOwner;
	int fXoffset;
	Classes::TNotifyEvent fOnChange;
	void __fastcall SetBookmarkImages(const Controls::TImageList* Value);
	void __fastcall SetDrawBookmarksFirst(bool Value);
	void __fastcall SetGlyphsVisible(bool Value);
	void __fastcall SetLeftMargin(int Value);
	void __fastcall SetXOffset(int Value);
	
public:
	__fastcall TSynBookMarkOpt(Classes::TComponent* AOwner);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	
__published:
	__property Controls::TImageList* BookmarkImages = {read=fBookmarkImages, write=SetBookmarkImages};
	__property bool DrawBookmarksFirst = {read=fDrawBookmarksFirst, write=SetDrawBookmarksFirst, default=1};
	__property bool EnableKeys = {read=fEnableKeys, write=fEnableKeys, default=1};
	__property bool GlyphsVisible = {read=fGlyphsVisible, write=SetGlyphsVisible, default=1};
	__property int LeftMargin = {read=fLeftMargin, write=SetLeftMargin, default=2};
	__property int Xoffset = {read=fXoffset, write=SetXOffset, default=12};
	__property Classes::TNotifyEvent OnChange = {read=fOnChange, write=fOnChange};
public:
	#pragma option push -w-inl
	/* TPersistent.Destroy */ inline __fastcall virtual ~TSynBookMarkOpt(void) { }
	#pragma option pop
	
};


class DELPHICLASS ESynMethodChain;
class PASCALIMPLEMENTATION ESynMethodChain : public Sysutils::Exception 
{
	typedef Sysutils::Exception inherited;
	
public:
	#pragma option push -w-inl
	/* Exception.Create */ inline __fastcall ESynMethodChain(const AnsiString Msg) : Sysutils::Exception(Msg) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmt */ inline __fastcall ESynMethodChain(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size) : Sysutils::Exception(Msg, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateRes */ inline __fastcall ESynMethodChain(int Ident)/* overload */ : Sysutils::Exception(Ident) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmt */ inline __fastcall ESynMethodChain(int Ident, const System::TVarRec * Args, const int Args_Size)/* overload */ : Sysutils::Exception(Ident, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateHelp */ inline __fastcall ESynMethodChain(const AnsiString Msg, int AHelpContext) : Sysutils::Exception(Msg, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmtHelp */ inline __fastcall ESynMethodChain(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size, int AHelpContext) : Sysutils::Exception(Msg, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResHelp */ inline __fastcall ESynMethodChain(int Ident, int AHelpContext)/* overload */ : Sysutils::Exception(Ident, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmtHelp */ inline __fastcall ESynMethodChain(System::PResStringRec ResStringRec, const System::TVarRec * Args, const int Args_Size, int AHelpContext)/* overload */ : Sysutils::Exception(ResStringRec, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~ESynMethodChain(void) { }
	#pragma option pop
	
};


typedef void __fastcall (__closure *TSynExceptionEvent)(System::TObject* Sender, Sysutils::Exception* E, bool &DoContinue);

class DELPHICLASS TSynMethodChain;
class PASCALIMPLEMENTATION TSynMethodChain : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	Classes::TList* FNotifyProcs;
	TSynExceptionEvent FExceptionHandler;
	
protected:
	virtual void __fastcall DoFire(const System::TMethod &AEvent) = 0 ;
	virtual bool __fastcall DoHandleException(Sysutils::Exception* E);
	__property TSynExceptionEvent ExceptionHandler = {read=FExceptionHandler, write=FExceptionHandler};
	
public:
	__fastcall virtual TSynMethodChain(void);
	__fastcall virtual ~TSynMethodChain(void);
	void __fastcall Add(const System::TMethod &AEvent);
	void __fastcall Remove(const System::TMethod &AEvent);
	void __fastcall Fire(void);
};


class DELPHICLASS TSynNotifyEventChain;
class PASCALIMPLEMENTATION TSynNotifyEventChain : public TSynMethodChain 
{
	typedef TSynMethodChain inherited;
	
private:
	System::TObject* FSender;
	
protected:
	virtual void __fastcall DoFire(const System::TMethod &AEvent);
	
public:
	__fastcall TSynNotifyEventChain(System::TObject* ASender);
	HIDESBASE void __fastcall Add(Classes::TNotifyEvent AEvent);
	HIDESBASE void __fastcall Remove(Classes::TNotifyEvent AEvent);
	__property ExceptionHandler ;
	__property System::TObject* Sender = {read=FSender, write=FSender};
public:
	#pragma option push -w-inl
	/* TSynMethodChain.Create */ inline __fastcall virtual TSynNotifyEventChain(void) : TSynMethodChain() { }
	#pragma option pop
	#pragma option push -w-inl
	/* TSynMethodChain.Destroy */ inline __fastcall virtual ~TSynNotifyEventChain(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynInternalImage;
class PASCALIMPLEMENTATION TSynInternalImage : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	Graphics::TBitmap* fImages;
	int fWidth;
	int fHeight;
	int fCount;
	Graphics::TBitmap* __fastcall CreateBitmapFromInternalList(const AnsiString Name);
	void __fastcall FreeBitmapFromInternalList(void);
	
public:
	__fastcall TSynInternalImage(const AnsiString Name, int Count);
	__fastcall virtual ~TSynInternalImage(void);
	void __fastcall DrawMark(Graphics::TCanvas* ACanvas, int Number, int X, int Y, int LineHeight);
	void __fastcall DrawMarkTransparent(Graphics::TCanvas* ACanvas, int Number, int X, int Y, int LineHeight, Graphics::TColor TransparentColor);
};


class DELPHICLASS TSynHotKey;
class PASCALIMPLEMENTATION TSynHotKey : public Stdctrls::TEdit 
{
	typedef Stdctrls::TEdit inherited;
	
private:
	Classes::TShortCut __fastcall GetHotKey(void);
	void __fastcall SetHotKey(const Classes::TShortCut Value);
	
protected:
	MESSAGE void __fastcall WMGetDlgCode(Messages::TWMNoParams &Message);
	DYNAMIC void __fastcall KeyDown(Word &Key, Classes::TShiftState Shift);
	DYNAMIC void __fastcall KeyUp(Word &Key, Classes::TShiftState Shift);
	DYNAMIC void __fastcall KeyPress(char &Key);
	DYNAMIC void __fastcall DoExit(void);
	
__published:
	__property Classes::TShortCut HotKey = {read=GetHotKey, write=SetHotKey, nodefault};
public:
	#pragma option push -w-inl
	/* TCustomEdit.Create */ inline __fastcall virtual TSynHotKey(Classes::TComponent* AOwner) : Stdctrls::TEdit(AOwner) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSynHotKey(HWND ParentWindow) : Stdctrls::TEdit(ParentWindow) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TWinControl.Destroy */ inline __fastcall virtual ~TSynHotKey(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditmiscclasses */
using namespace Syneditmiscclasses;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditMiscClasses
