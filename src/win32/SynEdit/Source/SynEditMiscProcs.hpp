// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditMiscProcs.pas' rev: 6.00

#ifndef SynEditMiscProcsHPP
#define SynEditMiscProcsHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditTypes.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditmiscprocs
{
//-- type declarations -------------------------------------------------------
typedef int TIntArray[134217727];

typedef int *PIntArray;

typedef AnsiString __fastcall (*TConvertTabsProc)(const AnsiString Line, int TabWidth);

typedef AnsiString __fastcall (*TConvertTabsProcEx)(const AnsiString Line, int TabWidth, bool &HasTabs);

//-- var, const, procedure ---------------------------------------------------
extern PACKAGE int __fastcall Max(int x, int y);
extern PACKAGE int __fastcall Min(int x, int y);
extern PACKAGE int __fastcall MinMax(int x, int mi, int ma);
extern PACKAGE void __fastcall SwapInt(int &l, int &r);
extern PACKAGE Types::TPoint __fastcall maxPoint(const Types::TPoint &P1, const Types::TPoint &P2);
extern PACKAGE Types::TPoint __fastcall minPoint(const Types::TPoint &P1, const Types::TPoint &P2);
extern PACKAGE PIntArray __fastcall GetIntArray(unsigned Count, int InitialValue);
extern PACKAGE void __fastcall InternalFillRect(HDC dc, const Types::TRect &rcPaint);
extern PACKAGE AnsiString __fastcall ConvertTabsEx(const AnsiString Line, int TabWidth, bool &HasTabs);
extern PACKAGE AnsiString __fastcall ConvertTabs(const AnsiString Line, int TabWidth);
extern PACKAGE TConvertTabsProc __fastcall GetBestConvertTabsProc(int TabWidth);
extern PACKAGE TConvertTabsProcEx __fastcall GetBestConvertTabsProcEx(int TabWidth);
extern PACKAGE int __fastcall CharIndex2CaretPos(int Index, int TabWidth, const AnsiString Line);
extern PACKAGE int __fastcall CaretPos2CharIndex(int Position, int TabWidth, const AnsiString Line, bool &InsideTabChar);
extern PACKAGE int __fastcall StrScanForCharInSet(const AnsiString Line, int Start, const Synedittypes::TSynIdentChars &AChars);
extern PACKAGE int __fastcall StrRScanForCharInSet(const AnsiString Line, int Start, const Synedittypes::TSynIdentChars &AChars);
extern PACKAGE int __fastcall StrScanForMultiByteChar(const AnsiString Line, int Start);
extern PACKAGE int __fastcall StrRScanForMultiByteChar(const AnsiString Line, int Start);
extern PACKAGE char * __fastcall GetEOL(char * Line);
extern PACKAGE AnsiString __fastcall EncodeString(AnsiString s);
extern PACKAGE AnsiString __fastcall DecodeString(AnsiString s);

}	/* namespace Syneditmiscprocs */
using namespace Syneditmiscprocs;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditMiscProcs
