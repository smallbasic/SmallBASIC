// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditPrintHeaderFooter.pas' rev: 6.00

#ifndef SynEditPrintHeaderFooterHPP
#define SynEditPrintHeaderFooterHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditPrintMargins.hpp>	// Pascal unit
#include <SynEditPrintTypes.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditprintheaderfooter
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS THeaderFooterItem;
class PASCALIMPLEMENTATION THeaderFooterItem : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	AnsiString FText;
	Graphics::TFont* FFont;
	int FLineNumber;
	Classes::TAlignment FAlignment;
	int FIndex;
	AnsiString __fastcall GetAsString();
	void __fastcall SetAsString(const AnsiString Value);
	void __fastcall SetFont(const Graphics::TFont* Value);
	
public:
	__fastcall THeaderFooterItem(void);
	__fastcall virtual ~THeaderFooterItem(void);
	AnsiString __fastcall GetText(int NumPages, int PageNum, bool Roman, AnsiString Title, AnsiString ATime, AnsiString ADate);
	void __fastcall LoadFromStream(Classes::TStream* AStream);
	void __fastcall SaveToStream(Classes::TStream* AStream);
	__property Classes::TAlignment Alignment = {read=FAlignment, write=FAlignment, nodefault};
	__property AnsiString AsString = {read=GetAsString, write=SetAsString};
	__property Graphics::TFont* Font = {read=FFont, write=SetFont};
	__property int LineNumber = {read=FLineNumber, write=FLineNumber, nodefault};
	__property AnsiString Text = {read=FText, write=FText};
};


#pragma option push -b-
enum THeaderFooterType { hftHeader, hftFooter };
#pragma option pop

class DELPHICLASS TLineInfo;
class PASCALIMPLEMENTATION TLineInfo : public System::TObject 
{
	typedef System::TObject inherited;
	
public:
	int LineHeight;
	int MaxBaseDist;
public:
	#pragma option push -w-inl
	/* TObject.Create */ inline __fastcall TLineInfo(void) : System::TObject() { }
	#pragma option pop
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TLineInfo(void) { }
	#pragma option pop
	
};


class DELPHICLASS THeaderFooter;
class PASCALIMPLEMENTATION THeaderFooter : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	THeaderFooterType FType;
	Syneditprinttypes::TFrameTypes FFrameTypes;
	Graphics::TColor FShadedColor;
	Graphics::TColor FLineColor;
	Classes::TList* FItems;
	Graphics::TFont* FDefaultFont;
	AnsiString FDate;
	AnsiString FTime;
	int FNumPages;
	AnsiString FTitle;
	Syneditprintmargins::TSynEditPrintMargins* FMargins;
	int FFrameHeight;
	Graphics::TPen* FOldPen;
	Graphics::TBrush* FOldBrush;
	Graphics::TFont* FOldFont;
	bool FRomanNumbers;
	Classes::TList* FLineInfo;
	int FLineCount;
	bool FMirrorPosition;
	void __fastcall SetDefaultFont(const Graphics::TFont* Value);
	void __fastcall DrawFrame(Graphics::TCanvas* ACanvas);
	void __fastcall CalcHeight(Graphics::TCanvas* ACanvas);
	void __fastcall SaveFontPenBrush(Graphics::TCanvas* ACanvas);
	void __fastcall RestoreFontPenBrush(Graphics::TCanvas* ACanvas);
	AnsiString __fastcall GetAsString();
	void __fastcall SetAsString(const AnsiString Value);
	
public:
	__fastcall THeaderFooter(void);
	__fastcall virtual ~THeaderFooter(void);
	int __fastcall Add(AnsiString Text, Graphics::TFont* Font, Classes::TAlignment Alignment, int LineNumber);
	void __fastcall Delete(int Index);
	void __fastcall Clear(void);
	int __fastcall Count(void);
	THeaderFooterItem* __fastcall Get(int Index);
	void __fastcall SetPixPrInch(int Value);
	void __fastcall InitPrint(Graphics::TCanvas* ACanvas, int NumPages, AnsiString Title, Syneditprintmargins::TSynEditPrintMargins* Margins);
	void __fastcall Print(Graphics::TCanvas* ACanvas, int PageNum);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	void __fastcall FixLines(void);
	__property AnsiString AsString = {read=GetAsString, write=SetAsString};
	void __fastcall LoadFromStream(Classes::TStream* AStream);
	void __fastcall SaveToStream(Classes::TStream* AStream);
	
__published:
	__property Syneditprinttypes::TFrameTypes FrameTypes = {read=FFrameTypes, write=FFrameTypes, default=1};
	__property Graphics::TColor ShadedColor = {read=FShadedColor, write=FShadedColor, default=12632256};
	__property Graphics::TColor LineColor = {read=FLineColor, write=FLineColor, default=0};
	__property Graphics::TFont* DefaultFont = {read=FDefaultFont, write=SetDefaultFont};
	__property bool RomanNumbers = {read=FRomanNumbers, write=FRomanNumbers, default=0};
	__property bool MirrorPosition = {read=FMirrorPosition, write=FMirrorPosition, default=0};
};


class DELPHICLASS THeader;
class PASCALIMPLEMENTATION THeader : public THeaderFooter 
{
	typedef THeaderFooter inherited;
	
public:
	__fastcall THeader(void);
public:
	#pragma option push -w-inl
	/* THeaderFooter.Destroy */ inline __fastcall virtual ~THeader(void) { }
	#pragma option pop
	
};


class DELPHICLASS TFooter;
class PASCALIMPLEMENTATION TFooter : public THeaderFooter 
{
	typedef THeaderFooter inherited;
	
public:
	__fastcall TFooter(void);
public:
	#pragma option push -w-inl
	/* THeaderFooter.Destroy */ inline __fastcall virtual ~TFooter(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditprintheaderfooter */
using namespace Syneditprintheaderfooter;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditPrintHeaderFooter
