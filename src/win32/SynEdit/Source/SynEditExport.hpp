// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditExport.pas' rev: 6.00

#ifndef SynEditExportHPP
#define SynEditExportHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Clipbrd.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditexport
{
//-- type declarations -------------------------------------------------------
typedef char *TSynReplaceCharsArray[256];

typedef char * *PSynReplaceCharsArray;

class DELPHICLASS TSynCustomExporter;
class PASCALIMPLEMENTATION TSynCustomExporter : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	Classes::TMemoryStream* fBuffer;
	bool fFirstAttribute;
	void __fastcall AssignFont(Graphics::TFont* Value);
	void __fastcall SetExportAsText(bool Value);
	void __fastcall SetFont(Graphics::TFont* Value);
	void __fastcall SetHighlighter(Synedithighlighter::TSynCustomHighlighter* Value);
	void __fastcall SetTitle(const AnsiString Value);
	
protected:
	Graphics::TColor fBackgroundColor;
	unsigned fClipboardFormat;
	AnsiString fDefaultFilter;
	bool fExportAsText;
	Graphics::TFont* fFont;
	Synedithighlighter::TSynCustomHighlighter* fHighlighter;
	Graphics::TColor fLastBG;
	Graphics::TColor fLastFG;
	Graphics::TFontStyles fLastStyle;
	char *fReplaceReserved[256];
	AnsiString fTitle;
	bool fUseBackground;
	void __fastcall AddData(const AnsiString AText);
	void __fastcall AddDataNewLine(const AnsiString AText);
	void __fastcall AddNewLine(void);
	void __fastcall CopyToClipboardFormat(unsigned AFormat);
	virtual void __fastcall FormatAttributeDone(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged) = 0 ;
	virtual void __fastcall FormatAttributeInit(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged) = 0 ;
	virtual void __fastcall FormatAfterLastAttribute(void) = 0 ;
	virtual void __fastcall FormatBeforeFirstAttribute(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged) = 0 ;
	virtual void __fastcall FormatToken(AnsiString Token);
	virtual void __fastcall FormatNewLine(void) = 0 ;
	int __fastcall GetBufferSize(void);
	virtual unsigned __fastcall GetClipboardFormat(void);
	virtual AnsiString __fastcall GetFooter(void) = 0 ;
	virtual AnsiString __fastcall GetFormatName();
	virtual AnsiString __fastcall GetHeader(void) = 0 ;
	void __fastcall InsertData(int APos, const AnsiString AText);
	virtual AnsiString __fastcall ReplaceMBCS(char Char1, char Char2);
	AnsiString __fastcall ReplaceReservedChars(AnsiString AToken, bool &IsSpace);
	virtual void __fastcall SetTokenAttribute(bool IsSpace, Synedithighlighter::TSynHighlighterAttributes* Attri);
	
public:
	__fastcall virtual TSynCustomExporter(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynCustomExporter(void);
	virtual void __fastcall Clear(void);
	void __fastcall CopyToClipboard(void);
	void __fastcall ExportAll(Classes::TStrings* ALines);
	void __fastcall ExportRange(Classes::TStrings* ALines, const Types::TPoint &Start, const Types::TPoint &Stop);
	void __fastcall SaveToFile(const AnsiString AFileName);
	void __fastcall SaveToStream(Classes::TStream* AStream);
	__property Graphics::TColor Color = {read=fBackgroundColor, write=fBackgroundColor, nodefault};
	__property AnsiString DefaultFilter = {read=fDefaultFilter, write=fDefaultFilter};
	__property bool ExportAsText = {read=fExportAsText, write=SetExportAsText, nodefault};
	__property Graphics::TFont* Font = {read=fFont, write=SetFont};
	__property AnsiString FormatName = {read=GetFormatName};
	__property Synedithighlighter::TSynCustomHighlighter* Highlighter = {read=fHighlighter, write=SetHighlighter};
	__property AnsiString Title = {read=fTitle, write=SetTitle};
	__property bool UseBackground = {read=fUseBackground, write=fUseBackground, nodefault};
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditexport */
using namespace Syneditexport;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditExport
