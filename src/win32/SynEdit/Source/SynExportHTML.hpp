// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynExportHTML.pas' rev: 6.00

#ifndef SynExportHTMLHPP
#define SynExportHTMLHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditExport.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synexporthtml
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum THTMLFontSize { fs01, fs02, fs03, fs04, fs05, fs06, fs07, fsDefault };
#pragma option pop

class DELPHICLASS TSynExporterHTML;
class PASCALIMPLEMENTATION TSynExporterHTML : public Syneditexport::TSynCustomExporter 
{
	typedef Syneditexport::TSynCustomExporter inherited;
	
private:
	THTMLFontSize fFontSize;
	AnsiString __fastcall ColorToHTML(Graphics::TColor AColor);
	
protected:
	bool fCreateHTMLFragment;
	virtual void __fastcall FormatAfterLastAttribute(void);
	virtual void __fastcall FormatAttributeDone(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged);
	virtual void __fastcall FormatAttributeInit(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged);
	virtual void __fastcall FormatBeforeFirstAttribute(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged);
	virtual void __fastcall FormatNewLine(void);
	virtual AnsiString __fastcall GetFooter();
	virtual AnsiString __fastcall GetFormatName();
	virtual AnsiString __fastcall GetHeader();
	
public:
	__fastcall virtual TSynExporterHTML(Classes::TComponent* AOwner);
	
__published:
	__property Color ;
	__property bool CreateHTMLFragment = {read=fCreateHTMLFragment, write=fCreateHTMLFragment, default=0};
	__property DefaultFilter ;
	__property Font ;
	__property Highlighter ;
	__property THTMLFontSize HTMLFontSize = {read=fFontSize, write=fFontSize, nodefault};
	__property Title ;
	__property UseBackground ;
public:
	#pragma option push -w-inl
	/* TSynCustomExporter.Destroy */ inline __fastcall virtual ~TSynExporterHTML(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synexporthtml */
using namespace Synexporthtml;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynExportHTML
