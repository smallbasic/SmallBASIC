// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditPrintPreview.pas' rev: 6.00

#ifndef SynEditPrintPreviewHPP
#define SynEditPrintPreviewHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Menus.hpp>	// Pascal unit
#include <SynEditPrint.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditprintpreview
{
//-- type declarations -------------------------------------------------------
typedef void __fastcall (__closure *TPreviewPageEvent)(System::TObject* Sender, int PageNumber);

#pragma option push -b-
enum TSynPreviewScale { pscWholePage, pscPageWidth, pscUserScaled };
#pragma option pop

class DELPHICLASS TSynEditPrintPreview;
class PASCALIMPLEMENTATION TSynEditPrintPreview : public Controls::TCustomControl 
{
	typedef Controls::TCustomControl inherited;
	
protected:
	Forms::TFormBorderStyle FBorderStyle;
	Syneditprint::TSynEditPrint* FSynEditPrint;
	TSynPreviewScale FScaleMode;
	int FScalePercent;
	#pragma pack(push, 1)
	Types::TPoint FVirtualSize;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint FVirtualOffset;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint FPageSize;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint FScrollPosition;
	#pragma pack(pop)
	
	Graphics::TColor FPageBG;
	int FPageNumber;
	bool FShowScrollHint;
	TPreviewPageEvent FOnPreviewPage;
	Classes::TNotifyEvent FOnScaleChange;
	int FWheelAccumulator;
	void __fastcall SetBorderStyle(Forms::TBorderStyle Value);
	void __fastcall SetPageBG(Graphics::TColor Value);
	void __fastcall SetSynEditPrint(Syneditprint::TSynEditPrint* Value);
	void __fastcall SetScaleMode(TSynPreviewScale Value);
	void __fastcall SetScalePercent(int Value);
	
private:
	HIDESBASE MESSAGE void __fastcall WMEraseBkgnd(Messages::TWMEraseBkgnd &Msg);
	HIDESBASE MESSAGE void __fastcall WMHScroll(Messages::TWMScroll &Msg);
	HIDESBASE MESSAGE void __fastcall WMSize(Messages::TWMSize &Msg);
	HIDESBASE MESSAGE void __fastcall WMVScroll(Messages::TWMScroll &Msg);
	HIDESBASE MESSAGE void __fastcall WMMouseWheel(Messages::TWMMouseWheel &Message);
	void __fastcall PaintPaper(void);
	int __fastcall GetPageCount(void);
	
protected:
	virtual void __fastcall CreateParams(Controls::TCreateParams &Params);
	int __fastcall GetPageHeightFromWidth(int AWidth);
	int __fastcall GetPageHeight100Percent(void);
	int __fastcall GetPageWidthFromHeight(int AHeight);
	int __fastcall GetPageWidth100Percent(void);
	virtual void __fastcall Notification(Classes::TComponent* AComponent, Classes::TOperation Operation);
	void __fastcall ScrollHorzFor(int Value);
	virtual void __fastcall ScrollHorzTo(int Value);
	void __fastcall ScrollVertFor(int Value);
	virtual void __fastcall ScrollVertTo(int Value);
	virtual void __fastcall UpdateScrollbars(void);
	virtual void __fastcall SizeChanged(void);
	
public:
	__fastcall virtual TSynEditPrintPreview(Classes::TComponent* AOwner);
	virtual void __fastcall Paint(void);
	void __fastcall UpdatePreview(void);
	void __fastcall NextPage(void);
	void __fastcall PreviousPage(void);
	void __fastcall FirstPage(void);
	void __fastcall LastPage(void);
	void __fastcall Print(void);
	__property int PageNumber = {read=FPageNumber, nodefault};
	__property int PageCount = {read=GetPageCount, nodefault};
	
__published:
	__property Align  = {default=5};
	__property Forms::TBorderStyle BorderStyle = {read=FBorderStyle, write=SetBorderStyle, default=1};
	__property Color  = {default=-2147483636};
	__property Cursor  = {default=0};
	__property Graphics::TColor PageBGColor = {read=FPageBG, write=SetPageBG, default=16777215};
	__property PopupMenu ;
	__property Syneditprint::TSynEditPrint* SynEditPrint = {read=FSynEditPrint, write=SetSynEditPrint};
	__property TSynPreviewScale ScaleMode = {read=FScaleMode, write=SetScaleMode, default=2};
	__property int ScalePercent = {read=FScalePercent, write=SetScalePercent, default=100};
	__property Visible  = {default=1};
	__property bool ShowScrollHint = {read=FShowScrollHint, write=FShowScrollHint, default=1};
	__property OnClick ;
	__property OnMouseDown ;
	__property OnMouseUp ;
	__property TPreviewPageEvent OnPreviewPage = {read=FOnPreviewPage, write=FOnPreviewPage};
	__property Classes::TNotifyEvent OnScaleChange = {read=FOnScaleChange, write=FOnScaleChange};
public:
	#pragma option push -w-inl
	/* TCustomControl.Destroy */ inline __fastcall virtual ~TSynEditPrintPreview(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSynEditPrintPreview(HWND ParentWindow) : Controls::TCustomControl(ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Syneditprintpreview */
using namespace Syneditprintpreview;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditPrintPreview
