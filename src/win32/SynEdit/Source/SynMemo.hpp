// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynMemo.pas' rev: 6.00

#ifndef SynMemoHPP
#define SynMemoHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditSearch.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <SynEditKeyCmds.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <SynEditMiscClasses.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <SynEdit.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synmemo
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TCustomSynMemo;
class PASCALIMPLEMENTATION TCustomSynMemo : public Synedit::TCustomSynEdit 
{
	typedef Synedit::TCustomSynEdit inherited;
	
public:
	Types::TPoint __fastcall CharIndexToRowCol(int Index);
	int __fastcall RowColToCharIndex(const Types::TPoint &RowCol);
public:
	#pragma option push -w-inl
	/* TCustomSynEdit.Create */ inline __fastcall virtual TCustomSynMemo(Classes::TComponent* AOwner) : Synedit::TCustomSynEdit(AOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomSynEdit.Destroy */ inline __fastcall virtual ~TCustomSynMemo(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TCustomSynMemo(HWND ParentWindow) : Synedit::TCustomSynEdit(ParentWindow) { }
	#pragma option pop
	
};


class DELPHICLASS TSynMemo;
class PASCALIMPLEMENTATION TSynMemo : public TCustomSynMemo 
{
	typedef TCustomSynMemo inherited;
	
__published:
	__property Align  = {default=0};
	__property Anchors  = {default=3};
	__property Constraints ;
	__property Color  = {default=-2147483643};
	__property Ctl3D ;
	__property Enabled  = {default=1};
	__property Font ;
	__property Height ;
	__property Name ;
	__property ParentColor  = {default=1};
	__property ParentCtl3D  = {default=1};
	__property ParentFont  = {default=1};
	__property ParentShowHint  = {default=1};
	__property PopupMenu ;
	__property ShowHint ;
	__property TabOrder  = {default=-1};
	__property TabStop  = {default=1};
	__property Tag  = {default=0};
	__property Visible  = {default=1};
	__property Width ;
	__property OnClick ;
	__property OnDblClick ;
	__property OnDragDrop ;
	__property OnDragOver ;
	__property OnEndDock ;
	__property OnEndDrag ;
	__property OnEnter ;
	__property OnExit ;
	__property OnKeyDown ;
	__property OnKeyPress ;
	__property OnKeyUp ;
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
	__property OnStartDock ;
	__property OnStartDrag ;
	__property BookMarkOptions ;
	__property BorderStyle  = {default=1};
	__property ExtraLineSpacing  = {default=0};
	__property Gutter ;
	__property HideSelection  = {default=0};
	__property Highlighter ;
	__property InsertCaret  = {default=0};
	__property InsertMode  = {default=1};
	__property Keystrokes ;
	__property Lines ;
	__property MaxLeftChar  = {default=1024};
	__property MaxUndo  = {default=1024};
	__property Options  = {default=53870738};
	__property OverwriteCaret  = {default=3};
	__property ReadOnly  = {default=0};
	__property RightEdge  = {default=80};
	__property RightEdgeColor  = {default=12632256};
	__property ScrollBars  = {default=3};
	__property SelectedColor ;
	__property SelectionMode  = {default=0};
	__property TabWidth  = {default=8};
	__property WantTabs  = {default=0};
	__property OnChange ;
	__property OnClearBookmark ;
	__property OnCommandProcessed ;
	__property OnDropFiles ;
	__property OnGutterClick ;
	__property OnPaint ;
	__property OnPlaceBookmark ;
	__property OnProcessCommand ;
	__property OnProcessUserCommand ;
	__property OnReplaceText ;
	__property OnSpecialLineColors ;
	__property OnStatusChange ;
	__property OnPaintTransient ;
	__property OnSearchEngineOverride ;
public:
	#pragma option push -w-inl
	/* TCustomSynEdit.Create */ inline __fastcall virtual TSynMemo(Classes::TComponent* AOwner) : TCustomSynMemo(AOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomSynEdit.Destroy */ inline __fastcall virtual ~TSynMemo(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSynMemo(HWND ParentWindow) : TCustomSynMemo(ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Synmemo */
using namespace Synmemo;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynMemo
