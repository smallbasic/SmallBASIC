// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynTextDrawer.pas' rev: 6.00

#ifndef SynTextDrawerHPP
#define SynTextDrawerHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Graphics.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syntextdrawer
{
//-- type declarations -------------------------------------------------------
typedef Shortint TheStockFontPatterns;

struct TheFontData;
typedef TheFontData *PheFontData;

#pragma pack(push, 4)
struct TheFontData
{
	Graphics::TFontStyles Style;
	HFONT Handle;
	int CharAdv;
	int DBCharAdv;
	int CharHeight;
} ;
#pragma pack(pop)

typedef TheFontData TheFontsData[17];

typedef TheFontData *PheFontsData;

struct TheSharedFontsInfo;
typedef TheSharedFontsInfo *PheSharedFontsInfo;

#pragma pack(push, 4)
struct TheSharedFontsInfo
{
	int RefCount;
	int LockCount;
	Graphics::TFont* BaseFont;
	tagLOGFONTA BaseLF;
	bool IsDBCSFont;
	bool IsTrueType;
	TheFontData FontsData[17];
} ;
#pragma pack(pop)

class DELPHICLASS TheFontsInfoManager;
class PASCALIMPLEMENTATION TheFontsInfoManager : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	Classes::TList* FFontsInfo;
	PheSharedFontsInfo __fastcall FindFontsInfo(const tagLOGFONTA &LF);
	PheSharedFontsInfo __fastcall CreateFontsInfo(Graphics::TFont* ABaseFont, const tagLOGFONTA &LF);
	void __fastcall DestroyFontHandles(PheSharedFontsInfo pFontsInfo);
	void __fastcall RetrieveLogFontForComparison(Graphics::TFont* ABaseFont, tagLOGFONTA &LF);
	
public:
	__fastcall TheFontsInfoManager(void);
	__fastcall virtual ~TheFontsInfoManager(void);
	void __fastcall LockFontsInfo(PheSharedFontsInfo pFontsInfo);
	void __fastcall UnLockFontsInfo(PheSharedFontsInfo pFontsInfo);
	PheSharedFontsInfo __fastcall GetFontsInfo(Graphics::TFont* ABaseFont);
	void __fastcall ReleaseFontsInfo(PheSharedFontsInfo pFontsInfo);
};


typedef void __fastcall (__closure *TheExtTextOutProc)(int X, int Y, unsigned fuOptions, const Types::TRect &ARect, char * Text, int Length);

class DELPHICLASS EheFontStockException;
class PASCALIMPLEMENTATION EheFontStockException : public Sysutils::Exception 
{
	typedef Sysutils::Exception inherited;
	
public:
	#pragma option push -w-inl
	/* Exception.Create */ inline __fastcall EheFontStockException(const AnsiString Msg) : Sysutils::Exception(Msg) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmt */ inline __fastcall EheFontStockException(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size) : Sysutils::Exception(Msg, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateRes */ inline __fastcall EheFontStockException(int Ident)/* overload */ : Sysutils::Exception(Ident) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmt */ inline __fastcall EheFontStockException(int Ident, const System::TVarRec * Args, const int Args_Size)/* overload */ : Sysutils::Exception(Ident, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateHelp */ inline __fastcall EheFontStockException(const AnsiString Msg, int AHelpContext) : Sysutils::Exception(Msg, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmtHelp */ inline __fastcall EheFontStockException(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size, int AHelpContext) : Sysutils::Exception(Msg, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResHelp */ inline __fastcall EheFontStockException(int Ident, int AHelpContext)/* overload */ : Sysutils::Exception(Ident, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmtHelp */ inline __fastcall EheFontStockException(System::PResStringRec ResStringRec, const System::TVarRec * Args, const int Args_Size, int AHelpContext)/* overload */ : Sysutils::Exception(ResStringRec, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~EheFontStockException(void) { }
	#pragma option pop
	
};


class DELPHICLASS TheFontStock;
class PASCALIMPLEMENTATION TheFontStock : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	HDC FDC;
	int FDCRefCount;
	TheSharedFontsInfo *FpInfo;
	bool FUsingFontHandles;
	HFONT FCrntFont;
	Graphics::TFontStyles FCrntStyle;
	TheFontData *FpCrntFontData;
	#pragma pack(push, 1)
	tagLOGFONTA FBaseLF;
	#pragma pack(pop)
	
	Graphics::TFont* __fastcall GetBaseFont(void);
	bool __fastcall GetIsDBCSFont(void);
	bool __fastcall GetIsTrueType(void);
	
protected:
	virtual HDC __fastcall InternalGetDC(void);
	virtual void __fastcall InternalReleaseDC(HDC Value);
	virtual HFONT __fastcall InternalCreateFont(Graphics::TFontStyles Style);
	virtual int __fastcall CalcFontAdvance(HDC DC, System::PInteger pCharHeight, System::PInteger pDBCharAdvance);
	virtual int __fastcall GetCharAdvance(void);
	virtual int __fastcall GetCharHeight(void);
	virtual int __fastcall GetDBCharAdvance(void);
	virtual PheFontData __fastcall GetFontData(int idx);
	void __fastcall UseFontHandles(void);
	void __fastcall ReleaseFontsInfo(void);
	virtual void __fastcall SetBaseFont(Graphics::TFont* Value);
	virtual void __fastcall SetStyle(Graphics::TFontStyles Value);
	__property PheFontData FontData[int idx] = {read=GetFontData};
	__property PheSharedFontsInfo FontsInfo = {read=FpInfo};
	
public:
	__fastcall virtual TheFontStock(Graphics::TFont* InitialFont);
	__fastcall virtual ~TheFontStock(void);
	virtual void __fastcall ReleaseFontHandles(void);
	__property Graphics::TFont* BaseFont = {read=GetBaseFont};
	__property Graphics::TFontStyles Style = {read=FCrntStyle, write=SetStyle, nodefault};
	__property HFONT FontHandle = {read=FCrntFont, nodefault};
	__property int CharAdvance = {read=GetCharAdvance, nodefault};
	__property int CharHeight = {read=GetCharHeight, nodefault};
	__property int DBCharAdvance = {read=GetDBCharAdvance, nodefault};
	__property bool IsDBCSFont = {read=GetIsDBCSFont, nodefault};
	__property bool IsTrueType = {read=GetIsTrueType, nodefault};
};


class DELPHICLASS EheTextDrawerException;
class PASCALIMPLEMENTATION EheTextDrawerException : public Sysutils::Exception 
{
	typedef Sysutils::Exception inherited;
	
public:
	#pragma option push -w-inl
	/* Exception.Create */ inline __fastcall EheTextDrawerException(const AnsiString Msg) : Sysutils::Exception(Msg) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmt */ inline __fastcall EheTextDrawerException(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size) : Sysutils::Exception(Msg, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateRes */ inline __fastcall EheTextDrawerException(int Ident)/* overload */ : Sysutils::Exception(Ident) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmt */ inline __fastcall EheTextDrawerException(int Ident, const System::TVarRec * Args, const int Args_Size)/* overload */ : Sysutils::Exception(Ident, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateHelp */ inline __fastcall EheTextDrawerException(const AnsiString Msg, int AHelpContext) : Sysutils::Exception(Msg, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmtHelp */ inline __fastcall EheTextDrawerException(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size, int AHelpContext) : Sysutils::Exception(Msg, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResHelp */ inline __fastcall EheTextDrawerException(int Ident, int AHelpContext)/* overload */ : Sysutils::Exception(Ident, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmtHelp */ inline __fastcall EheTextDrawerException(System::PResStringRec ResStringRec, const System::TVarRec * Args, const int Args_Size, int AHelpContext)/* overload */ : Sysutils::Exception(ResStringRec, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~EheTextDrawerException(void) { }
	#pragma option pop
	
};


class DELPHICLASS TheTextDrawer;
class PASCALIMPLEMENTATION TheTextDrawer : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	HDC FDC;
	int FSaveDC;
	TheFontStock* FFontStock;
	Graphics::TFontStyles FCalcExtentBaseStyle;
	int FBaseCharWidth;
	int FBaseCharHeight;
	HFONT FCrntFont;
	void *FETODist;
	int FETOSizeInChar;
	Graphics::TColor FColor;
	Graphics::TColor FBkColor;
	int FCharExtra;
	int FDrawingCount;
	
protected:
	virtual void __fastcall ReleaseETODist(void);
	virtual void __fastcall AfterStyleSet(void);
	virtual void __fastcall DoSetCharExtra(int Value);
	__property HDC StockDC = {read=FDC, nodefault};
	__property int DrawingCount = {read=FDrawingCount, nodefault};
	__property TheFontStock* FontStock = {read=FFontStock};
	__property int BaseCharWidth = {read=FBaseCharWidth, nodefault};
	__property int BaseCharHeight = {read=FBaseCharHeight, nodefault};
	
public:
	__fastcall virtual TheTextDrawer(Graphics::TFontStyles CalcExtentBaseStyle, Graphics::TFont* BaseFont);
	__fastcall virtual ~TheTextDrawer(void);
	virtual int __fastcall GetCharWidth(void);
	virtual int __fastcall GetCharHeight(void);
	virtual void __fastcall BeginDrawing(HDC DC);
	virtual void __fastcall EndDrawing(void);
	virtual void __fastcall TextOut(int X, int Y, char * Text, int Length);
	virtual void __fastcall ExtTextOut(int X, int Y, unsigned fuOptions, const Types::TRect &ARect, char * Text, int Length);
	virtual void __fastcall SetBaseFont(Graphics::TFont* Value);
	virtual void __fastcall SetBaseStyle(const Graphics::TFontStyles Value);
	virtual void __fastcall SetStyle(Graphics::TFontStyles Value);
	virtual void __fastcall SetForeColor(Graphics::TColor Value);
	virtual void __fastcall SetBackColor(Graphics::TColor Value);
	virtual void __fastcall SetCharExtra(int Value);
	virtual void __fastcall ReleaseTemporaryResources(void);
	__property int CharWidth = {read=GetCharWidth, nodefault};
	__property int CharHeight = {read=GetCharHeight, nodefault};
	__property Graphics::TFont* BaseFont = {write=SetBaseFont};
	__property Graphics::TFontStyles BaseStyle = {write=SetBaseStyle, nodefault};
	__property Graphics::TColor ForeColor = {write=SetForeColor, nodefault};
	__property Graphics::TColor BackColor = {write=SetBackColor, nodefault};
	__property Graphics::TFontStyles Style = {write=SetStyle, nodefault};
	__property int CharExtra = {read=FCharExtra, write=SetCharExtra, nodefault};
};


class DELPHICLASS TheTextDrawer2;
class PASCALIMPLEMENTATION TheTextDrawer2 : public TheTextDrawer 
{
	typedef TheTextDrawer inherited;
	
private:
	HFONT FFonts[17];
	
public:
	virtual void __fastcall SetStyle(Graphics::TFontStyles Value);
	virtual void __fastcall SetBaseFont(Graphics::TFont* Value);
public:
	#pragma option push -w-inl
	/* TheTextDrawer.Create */ inline __fastcall virtual TheTextDrawer2(Graphics::TFontStyles CalcExtentBaseStyle, Graphics::TFont* BaseFont) : TheTextDrawer(CalcExtentBaseStyle, BaseFont) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TheTextDrawer.Destroy */ inline __fastcall virtual ~TheTextDrawer2(void) { }
	#pragma option pop
	
};


class DELPHICLASS TheTextDrawerEx;
class PASCALIMPLEMENTATION TheTextDrawerEx : public TheTextDrawer 
{
	typedef TheTextDrawer inherited;
	
private:
	int FCrntDx;
	int FCrntDBDx;
	TheExtTextOutProc FExtTextOutProc;
	
protected:
	virtual void __fastcall AfterStyleSet(void);
	virtual void __fastcall DoSetCharExtra(int Value);
	virtual void __fastcall TextOutOrExtTextOut(int X, int Y, unsigned fuOptions, const Types::TRect &ARect, char * Text, int Length);
	virtual void __fastcall ExtTextOutFixed(int X, int Y, unsigned fuOptions, const Types::TRect &ARect, char * Text, int Length);
	virtual void __fastcall ExtTextOutWithETO(int X, int Y, unsigned fuOptions, const Types::TRect &ARect, char * Text, int Length);
	virtual void __fastcall ExtTextOutForDBCS(int X, int Y, unsigned fuOptions, const Types::TRect &ARect, char * Text, int Length);
	
public:
	virtual void __fastcall ExtTextOut(int X, int Y, unsigned fuOptions, const Types::TRect &ARect, char * Text, int Length);
public:
	#pragma option push -w-inl
	/* TheTextDrawer.Create */ inline __fastcall virtual TheTextDrawerEx(Graphics::TFontStyles CalcExtentBaseStyle, Graphics::TFont* BaseFont) : TheTextDrawer(CalcExtentBaseStyle, BaseFont) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TheTextDrawer.Destroy */ inline __fastcall virtual ~TheTextDrawerEx(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE TheFontsInfoManager* __fastcall GetFontsInfoManager(void);

}	/* namespace Syntextdrawer */
using namespace Syntextdrawer;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynTextDrawer
