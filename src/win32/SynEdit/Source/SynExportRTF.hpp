// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynExportRTF.pas' rev: 6.00

#ifndef SynExportRTFHPP
#define SynExportRTFHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditExport.hpp>	// Pascal unit
#include <RichEdit.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synexportrtf
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynExporterRTF;
class PASCALIMPLEMENTATION TSynExporterRTF : public Syneditexport::TSynCustomExporter 
{
	typedef Syneditexport::TSynCustomExporter inherited;
	
private:
	bool fAttributesChanged;
	Classes::TList* fListColors;
	AnsiString __fastcall ColorToRTF(Graphics::TColor AColor);
	int __fastcall GetColorIndex(Graphics::TColor AColor);
	
protected:
	virtual void __fastcall FormatAfterLastAttribute(void);
	virtual void __fastcall FormatAttributeDone(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged);
	virtual void __fastcall FormatAttributeInit(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged);
	virtual void __fastcall FormatBeforeFirstAttribute(bool BackgroundChanged, bool ForegroundChanged, Graphics::TFontStyles FontStylesChanged);
	virtual void __fastcall FormatNewLine(void);
	virtual AnsiString __fastcall GetFooter();
	virtual AnsiString __fastcall GetFormatName();
	virtual AnsiString __fastcall GetHeader();
	virtual AnsiString __fastcall ReplaceMBCS(char Char1, char Char2);
	
public:
	__fastcall virtual TSynExporterRTF(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynExporterRTF(void);
	virtual void __fastcall Clear(void);
	
__published:
	__property Color ;
	__property DefaultFilter ;
	__property Font ;
	__property Highlighter ;
	__property Title ;
	__property UseBackground ;
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synexportrtf */
using namespace Synexportrtf;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynExportRTF
