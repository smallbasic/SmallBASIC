// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditPrint.pas' rev: 6.00

#ifndef SynEditPrintHPP
#define SynEditPrintHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditMiscProcs.hpp>	// Pascal unit
#include <SynEditPrintMargins.hpp>	// Pascal unit
#include <SynEditPrinterInfo.hpp>	// Pascal unit
#include <SynEditPrintHeaderFooter.hpp>	// Pascal unit
#include <SynEditPrintTypes.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <SynEdit.hpp>	// Pascal unit
#include <Printers.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditprint
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TPageLine;
class PASCALIMPLEMENTATION TPageLine : public System::TObject 
{
	typedef System::TObject inherited;
	
public:
	int FirstLine;
public:
	#pragma option push -w-inl
	/* TObject.Create */ inline __fastcall TPageLine(void) : System::TObject() { }
	#pragma option pop
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TPageLine(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynEditPrint;
class PASCALIMPLEMENTATION TSynEditPrint : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	int FCopies;
	Syneditprintheaderfooter::TFooter* FFooter;
	Syneditprintheaderfooter::THeader* FHeader;
	Classes::TStrings* FLines;
	Syneditprintmargins::TSynEditPrintMargins* FMargins;
	int FPageCount;
	Graphics::TFont* FFont;
	AnsiString FTitle;
	AnsiString FDocTitle;
	Syneditprinterinfo::TSynEditPrinterInfo* FPrinterInfo;
	Classes::TList* FPages;
	Graphics::TCanvas* FCanvas;
	tagTEXTMETRICA FTextMetrics;
	int FCharWidth;
	int FMaxLeftChar;
	int *FETODist;
	bool FWrap;
	Syneditprinttypes::TPrintLineEvent FOnPrintLine;
	Syneditprinttypes::TPrintStatusEvent FOnPrintStatus;
	int FYPos;
	int FLineHeight;
	bool FHighlight;
	bool FColors;
	Synedithighlighter::TSynCustomHighlighter* FHighlighter;
	Graphics::TFont* FOldFont;
	bool FSynOK;
	bool FLineNumbers;
	int FLineNumber;
	int FLineOffset;
	bool FAbort;
	bool FPrinting;
	Graphics::TColor FDefaultBG;
	int FPageOffset;
	bool FRangesOK;
	int FMaxWidth;
	int FMaxCol;
	bool FPagesCounted;
	bool FLineNumbersInMargin;
	int FTabWidth;
	Graphics::TColor fFontColor;
	bool fSelectedOnly;
	bool fSelAvail;
	Synedittypes::TSynSelectionMode fSelMode;
	#pragma pack(push, 1)
	Types::TPoint fBlockBegin;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint fBlockEnd;
	#pragma pack(pop)
	
	void __fastcall CalcPages(void);
	void __fastcall SetLines(const Classes::TStrings* Value);
	void __fastcall SetFont(const Graphics::TFont* Value);
	void __fastcall SetCharWidth(const int Value);
	void __fastcall SetMaxLeftChar(const int Value);
	void __fastcall PrintPage(int Num);
	void __fastcall WriteLine(AnsiString Text);
	void __fastcall WriteLineNumber(void);
	void __fastcall HandleWrap(AnsiString Text, int MaxWidth);
	void __fastcall TextOut(AnsiString Text, Classes::TList* AList);
	void __fastcall SetHighlighter(const Synedithighlighter::TSynCustomHighlighter* Value);
	void __fastcall RestoreCurrentFont(void);
	void __fastcall SaveCurrentFont(void);
	void __fastcall SetPixelsPrInch(void);
	void __fastcall InitPrint(void);
	void __fastcall InitRanges(void);
	int __fastcall GetPageCount(void);
	void __fastcall SetSynEdit(const Synedit::TSynEdit* Value);
	
protected:
	__property int MaxLeftChar = {read=FMaxLeftChar, write=SetMaxLeftChar, nodefault};
	__property int CharWidth = {read=FCharWidth, write=SetCharWidth, nodefault};
	virtual void __fastcall PrintStatus(Syneditprinttypes::TSynPrintStatus Status, int PageNumber, bool &Abort);
	virtual void __fastcall PrintLine(int LineNumber, int PageNumber);
	
public:
	__fastcall virtual TSynEditPrint(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynEditPrint(void);
	void __fastcall UpdatePages(Graphics::TCanvas* ACanvas);
	void __fastcall PrintToCanvas(Graphics::TCanvas* ACanvas, int PageNumber);
	void __fastcall Print(void);
	void __fastcall PrintRange(int StartPage, int EndPage);
	__property Syneditprinterinfo::TSynEditPrinterInfo* PrinterInfo = {read=FPrinterInfo};
	__property int PageCount = {read=GetPageCount, nodefault};
	__property Synedit::TSynEdit* SynEdit = {write=SetSynEdit};
	void __fastcall LoadFromStream(Classes::TStream* AStream);
	void __fastcall SaveToStream(Classes::TStream* AStream);
	
__published:
	__property int Copies = {read=FCopies, write=FCopies, nodefault};
	__property Syneditprintheaderfooter::THeader* Header = {read=FHeader, write=FHeader};
	__property Syneditprintheaderfooter::TFooter* Footer = {read=FFooter, write=FFooter};
	__property Syneditprintmargins::TSynEditPrintMargins* Margins = {read=FMargins, write=FMargins};
	__property Classes::TStrings* Lines = {read=FLines, write=SetLines};
	__property Graphics::TFont* Font = {read=FFont, write=SetFont};
	__property AnsiString Title = {read=FTitle, write=FTitle};
	__property AnsiString DocTitle = {read=FDocTitle, write=FDocTitle};
	__property bool Wrap = {read=FWrap, write=FWrap, default=1};
	__property bool Highlight = {read=FHighlight, write=FHighlight, default=1};
	__property bool SelectedOnly = {read=fSelectedOnly, write=fSelectedOnly, default=0};
	__property bool Colors = {read=FColors, write=FColors, default=0};
	__property bool LineNumbers = {read=FLineNumbers, write=FLineNumbers, default=0};
	__property int LineOffset = {read=FLineOffset, write=FLineOffset, default=0};
	__property int PageOffset = {read=FPageOffset, write=FPageOffset, default=0};
	__property Syneditprinttypes::TPrintLineEvent OnPrintLine = {read=FOnPrintLine, write=FOnPrintLine};
	__property Syneditprinttypes::TPrintStatusEvent OnPrintStatus = {read=FOnPrintStatus, write=FOnPrintStatus};
	__property Synedithighlighter::TSynCustomHighlighter* Highlighter = {read=FHighlighter, write=SetHighlighter};
	__property bool LineNumbersInMargin = {read=FLineNumbersInMargin, write=FLineNumbersInMargin, default=0};
	__property int TabWidth = {read=FTabWidth, write=FTabWidth, nodefault};
	__property Graphics::TColor Color = {read=FDefaultBG, write=FDefaultBG, nodefault};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditprint */
using namespace Syneditprint;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditPrint
