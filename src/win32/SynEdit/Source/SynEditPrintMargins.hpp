// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditPrintMargins.pas' rev: 6.00

#ifndef SynEditPrintMarginsHPP
#define SynEditPrintMarginsHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditPrinterInfo.hpp>	// Pascal unit
#include <SynEditPrintTypes.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditprintmargins
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSynEditPrintMargins;
class PASCALIMPLEMENTATION TSynEditPrintMargins : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	double FLeft;
	double FRight;
	double FTop;
	double FBottom;
	double FHeader;
	double FFooter;
	double FLeftHFTextIndent;
	double FRightHFTextIndent;
	double FHFInternalMargin;
	double FGutter;
	bool FMirrorMargins;
	Syneditprinttypes::TUnitSystem FUnitSystem;
	double __fastcall ConvertTo(double Value);
	double __fastcall ConvertFrom(double Value);
	double __fastcall GetBottom(void);
	double __fastcall GetFooter(void);
	double __fastcall GetGutter(void);
	double __fastcall GetHeader(void);
	double __fastcall GetLeft(void);
	double __fastcall GetRight(void);
	double __fastcall GetTop(void);
	double __fastcall GetLeftHFTextIndent(void);
	double __fastcall GetRightHFTextIndent(void);
	double __fastcall GetHFInternalMargin(void);
	void __fastcall SetBottom(const double Value);
	void __fastcall SetFooter(const double Value);
	void __fastcall SetGutter(const double Value);
	void __fastcall SetHeader(const double Value);
	void __fastcall SetLeft(const double Value);
	void __fastcall SetRight(const double Value);
	void __fastcall SetTop(const double Value);
	void __fastcall SetLeftHFTextIndent(const double Value);
	void __fastcall SetRightHFTextIndent(const double Value);
	void __fastcall SetHFInternalMargin(const double Value);
	
public:
	int PLeft;
	int PRight;
	int PTop;
	int PBottom;
	int PHeader;
	int PFooter;
	int PLeftHFTextIndent;
	int PRightHFTextIndent;
	int PHFInternalMargin;
	int PGutter;
	__fastcall TSynEditPrintMargins(void);
	void __fastcall InitPage(Graphics::TCanvas* ACanvas, int PageNum, Syneditprinterinfo::TSynEditPrinterInfo* PrinterInfo, bool LineNumbers, bool LineNumbersInMargin, int MaxLineNum);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	void __fastcall LoadFromStream(Classes::TStream* AStream);
	void __fastcall SaveToStream(Classes::TStream* AStream);
	
__published:
	__property Syneditprinttypes::TUnitSystem UnitSystem = {read=FUnitSystem, write=FUnitSystem, default=0};
	__property double Left = {read=GetLeft, write=SetLeft};
	__property double Right = {read=GetRight, write=SetRight};
	__property double Top = {read=GetTop, write=SetTop};
	__property double Bottom = {read=GetBottom, write=SetBottom};
	__property double Header = {read=GetHeader, write=SetHeader};
	__property double Footer = {read=GetFooter, write=SetFooter};
	__property double LeftHFTextIndent = {read=GetLeftHFTextIndent, write=SetLeftHFTextIndent};
	__property double RightHFTextIndent = {read=GetRightHFTextIndent, write=SetRightHFTextIndent};
	__property double HFInternalMargin = {read=GetHFInternalMargin, write=SetHFInternalMargin};
	__property double Gutter = {read=GetGutter, write=SetGutter};
	__property bool MirrorMargins = {read=FMirrorMargins, write=FMirrorMargins, nodefault};
public:
	#pragma option push -w-inl
	/* TPersistent.Destroy */ inline __fastcall virtual ~TSynEditPrintMargins(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditprintmargins */
using namespace Syneditprintmargins;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditPrintMargins
