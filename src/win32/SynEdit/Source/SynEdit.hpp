// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEdit.pas' rev: 6.00

#ifndef SynEditHPP
#define SynEditHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Menus.hpp>	// Pascal unit
#include <SynTextDrawer.hpp>	// Pascal unit
#include <SynEditKbdHandler.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditSearch.hpp>	// Pascal unit
#include <SynEditKeyCmds.hpp>	// Pascal unit
#include <SynEditTextBuffer.hpp>	// Pascal unit
#include <SynEditMiscClasses.hpp>	// Pascal unit
#include <SynEditMiscProcs.hpp>	// Pascal unit
#include <SynEditKeyConst.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Imm.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synedit
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TSynSearchOption { ssoMatchCase, ssoWholeWord, ssoBackwards, ssoEntireScope, ssoSelectedOnly, ssoReplace, ssoReplaceAll, ssoPrompt };
#pragma option pop

typedef Set<TSynSearchOption, ssoMatchCase, ssoPrompt>  TSynSearchOptions;

#pragma option push -b-
enum TSynReplaceAction { raCancel, raSkip, raReplace, raReplaceAll };
#pragma option pop

class DELPHICLASS ESynEditError;
class PASCALIMPLEMENTATION ESynEditError : public Sysutils::Exception 
{
	typedef Sysutils::Exception inherited;
	
public:
	#pragma option push -w-inl
	/* Exception.Create */ inline __fastcall ESynEditError(const AnsiString Msg) : Sysutils::Exception(Msg) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmt */ inline __fastcall ESynEditError(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size) : Sysutils::Exception(Msg, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateRes */ inline __fastcall ESynEditError(int Ident)/* overload */ : Sysutils::Exception(Ident) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmt */ inline __fastcall ESynEditError(int Ident, const System::TVarRec * Args, const int Args_Size)/* overload */ : Sysutils::Exception(Ident, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateHelp */ inline __fastcall ESynEditError(const AnsiString Msg, int AHelpContext) : Sysutils::Exception(Msg, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmtHelp */ inline __fastcall ESynEditError(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size, int AHelpContext) : Sysutils::Exception(Msg, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResHelp */ inline __fastcall ESynEditError(int Ident, int AHelpContext)/* overload */ : Sysutils::Exception(Ident, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmtHelp */ inline __fastcall ESynEditError(System::PResStringRec ResStringRec, const System::TVarRec * Args, const int Args_Size, int AHelpContext)/* overload */ : Sysutils::Exception(ResStringRec, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~ESynEditError(void) { }
	#pragma option pop
	
};


typedef void __fastcall (__closure *TDropFilesEvent)(System::TObject* Sender, int X, int Y, Classes::TStrings* AFiles);

typedef void __fastcall (__closure *THookedCommandEvent)(System::TObject* Sender, bool AfterProcessing, bool &Handled, Syneditkeycmds::TSynEditorCommand &Command, char &AChar, void * Data, void * HandlerData);

typedef void __fastcall (__closure *TPaintEvent)(System::TObject* Sender, Graphics::TCanvas* ACanvas);

typedef void __fastcall (__closure *TProcessCommandEvent)(System::TObject* Sender, Syneditkeycmds::TSynEditorCommand &Command, char &AChar, void * Data);

typedef void __fastcall (__closure *TReplaceTextEvent)(System::TObject* Sender, const AnsiString ASearch, const AnsiString AReplace, int Line, int Column, TSynReplaceAction &Action);

typedef void __fastcall (__closure *TSpecialLineColorsEvent)(System::TObject* Sender, int Line, bool &Special, Graphics::TColor &FG, Graphics::TColor &BG);

#pragma option push -b-
enum TTransientType { ttBefore, ttAfter };
#pragma option pop

typedef void __fastcall (__closure *TPaintTransient)(System::TObject* Sender, Graphics::TCanvas* Canvas, TTransientType TransientType);

#pragma option push -b-
enum TSynEditCaretType { ctVerticalLine, ctHorizontalLine, ctHalfBlock, ctBlock };
#pragma option pop

#pragma option push -b-
enum TSynStateFlag { sfCaretChanged, sfScrollbarChanged, sfLinesChanging, sfIgnoreNextChar, sfCaretVisible, sfDblClicked, sfPossibleGutterClick, sfWaitForDragging, sfInsideRedo };
#pragma option pop

#pragma option push -b-
enum TScrollHintFormat { shfTopLineOnly, shfTopToBottom };
#pragma option pop

typedef Set<TSynStateFlag, sfCaretChanged, sfInsideRedo>  TSynStateFlags;

#pragma option push -b-
enum TSynEditorOption { eoAltSetsColumnMode, eoAutoIndent, eoAutoSizeMaxLeftChar, eoDisableScrollArrows, eoDragDropEditing, eoDropFiles, eoEnhanceHomeKey, eoGroupUndo, eoHalfPageScroll, eoHideShowScrollbars, eoKeepCaretX, eoNoCaret, eoNoSelection, eoRightMouseMovesCursor, eoScrollByOneLess, eoScrollHintFollows, eoScrollPastEof, eoScrollPastEol, eoShowScrollHint, eoShowSpecialChars, eoSmartTabDelete, eoSmartTabs, eoSpecialLineDefaultFg, eoTabIndent, eoTabsToSpaces, eoTrimTrailingSpaces };
#pragma option pop

typedef Set<TSynEditorOption, eoAltSetsColumnMode, eoTrimTrailingSpaces>  TSynEditorOptions;

#pragma option push -b-
enum TSynStatusChange { scAll, scCaretX, scCaretY, scLeftChar, scTopLine, scInsertMode, scModified, scSelection, scReadOnly };
#pragma option pop

typedef Set<TSynStatusChange, scAll, scReadOnly>  TSynStatusChanges;

typedef void __fastcall (__closure *TContextHelpEvent)(System::TObject* Sender, AnsiString word);

typedef void __fastcall (__closure *TStatusChangeEvent)(System::TObject* Sender, TSynStatusChanges Changes);

class DELPHICLASS TSynEditMark;
class DELPHICLASS TCustomSynEdit;
class DELPHICLASS TSynEditMarkList;
class PASCALIMPLEMENTATION TSynEditMark : public System::TObject 
{
	typedef System::TObject inherited;
	
protected:
	int fLine;
	int fColumn;
	int fImage;
	TCustomSynEdit* fEdit;
	bool fVisible;
	bool fInternalImage;
	int fBookmarkNum;
	virtual TCustomSynEdit* __fastcall GetEdit(void);
	virtual void __fastcall SetColumn(const int Value);
	virtual void __fastcall SetImage(const int Value);
	virtual void __fastcall SetLine(const int Value);
	void __fastcall SetVisible(const bool Value);
	void __fastcall SetInternalImage(const bool Value);
	bool __fastcall GetIsBookmark(void);
	
public:
	__fastcall TSynEditMark(TCustomSynEdit* AOwner);
	__property int Line = {read=fLine, write=SetLine, nodefault};
	__property int Column = {read=fColumn, write=SetColumn, nodefault};
	__property TCustomSynEdit* Edit = {read=fEdit};
	__property int ImageIndex = {read=fImage, write=SetImage, nodefault};
	__property int BookmarkNumber = {read=fBookmarkNum, write=fBookmarkNum, nodefault};
	__property bool Visible = {read=fVisible, write=SetVisible, nodefault};
	__property bool InternalImage = {read=fInternalImage, write=SetInternalImage, nodefault};
	__property bool IsBookmark = {read=GetIsBookmark, nodefault};
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TSynEditMark(void) { }
	#pragma option pop
	
};


typedef TSynEditMark* TSynEditMarks[16];

class PASCALIMPLEMENTATION TSynEditMarkList : public Classes::TList 
{
	typedef Classes::TList inherited;
	
public:
	TSynEditMark* operator[](int Index) { return Items[Index]; }
	
protected:
	TCustomSynEdit* fEdit;
	Classes::TNotifyEvent fOnChange;
	void __fastcall DoChange(void);
	HIDESBASE TSynEditMark* __fastcall Get(int Index);
	HIDESBASE void __fastcall Put(int Index, TSynEditMark* Item);
	
public:
	__fastcall TSynEditMarkList(TCustomSynEdit* AOwner);
	__fastcall virtual ~TSynEditMarkList(void);
	HIDESBASE int __fastcall Add(TSynEditMark* Item);
	void __fastcall ClearLine(int line);
	HIDESBASE void __fastcall Delete(int Index);
	HIDESBASE TSynEditMark* __fastcall First(void);
	void __fastcall GetMarksForLine(int line, TSynEditMark* * Marks);
	HIDESBASE void __fastcall Insert(int Index, TSynEditMark* Item);
	HIDESBASE TSynEditMark* __fastcall Last(void);
	void __fastcall Place(TSynEditMark* mark);
	HIDESBASE int __fastcall Remove(TSynEditMark* Item);
	__property TSynEditMark* Items[int Index] = {read=Get, write=Put/*, default*/};
	__property TCustomSynEdit* Edit = {read=fEdit};
	__property Classes::TNotifyEvent OnChange = {read=fOnChange, write=fOnChange};
};


typedef void __fastcall (__closure *TPlaceMarkEvent)(System::TObject* Sender, TSynEditMark* &Mark);

typedef void __fastcall (__closure *TGutterClickEvent)(System::TObject* Sender, int X, int Y, int Line, TSynEditMark* mark);

class PASCALIMPLEMENTATION TCustomSynEdit : public Controls::TCustomControl 
{
	typedef Controls::TCustomControl inherited;
	
private:
	MESSAGE void __fastcall WMCaptureChanged(Messages::TMessage &Msg);
	MESSAGE void __fastcall WMCopy(Messages::TMessage &Message);
	MESSAGE void __fastcall WMCut(Messages::TMessage &Message);
	MESSAGE void __fastcall WMDropFiles(Messages::TMessage &Msg);
	HIDESBASE MESSAGE void __fastcall WMEraseBkgnd(Messages::TMessage &Msg);
	MESSAGE void __fastcall WMGetDlgCode(Messages::TWMNoParams &Msg);
	HIDESBASE MESSAGE void __fastcall WMHScroll(Messages::TWMScroll &Msg);
	MESSAGE void __fastcall WMPaste(Messages::TMessage &Message);
	HIDESBASE MESSAGE void __fastcall WMCancelMode(Messages::TMessage &Message);
	MESSAGE void __fastcall WMImeComposition(Messages::TMessage &Msg);
	MESSAGE void __fastcall WMImeNotify(Messages::TMessage &Msg);
	HIDESBASE MESSAGE void __fastcall WMKillFocus(Messages::TWMKillFocus &Msg);
	HIDESBASE MESSAGE void __fastcall WMMouseWheel(Messages::TMessage &Msg);
	HIDESBASE MESSAGE void __fastcall WMSetCursor(Messages::TWMSetCursor &Msg);
	HIDESBASE MESSAGE void __fastcall WMSetFocus(Messages::TWMSetFocus &Msg);
	HIDESBASE MESSAGE void __fastcall WMSize(Messages::TWMSize &Msg);
	HIDESBASE MESSAGE void __fastcall WMVScroll(Messages::TWMScroll &Msg);
	#pragma pack(push, 1)
	Types::TPoint fBlockBegin;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint fBlockEnd;
	#pragma pack(pop)
	
	int fCaretX;
	int fLastCaretX;
	int fCaretY;
	int fCharsInWindow;
	int fCharWidth;
	Graphics::TFont* fFontDummy;
	int fImeCount;
	bool fMBCSStepAside;
	bool fInserting;
	Classes::TStrings* fLines;
	int fLinesInWindow;
	int fLeftChar;
	int fMaxLeftChar;
	int fPaintLock;
	bool fReadOnly;
	int fRightEdge;
	Graphics::TColor fRightEdgeColor;
	Graphics::TColor fScrollHintColor;
	TScrollHintFormat fScrollHintFormat;
	Stdctrls::TScrollStyle FScrollBars;
	int fTextHeight;
	int fTextOffset;
	int fTopLine;
	Synedithighlighter::TSynCustomHighlighter* fHighlighter;
	Syneditmiscclasses::TSynSelectedColor* fSelectedColor;
	Synedittextbuffer::TSynEditUndoList* fUndoList;
	Synedittextbuffer::TSynEditUndoList* fRedoList;
	TSynEditMark* fBookMarks[10];
	int fMouseDownX;
	int fMouseDownY;
	Syneditmiscclasses::TSynBookMarkOpt* fBookMarkOpt;
	Forms::TFormBorderStyle fBorderStyle;
	bool fHideSelection;
	int fMouseWheelAccumulator;
	TSynEditCaretType fOverwriteCaret;
	TSynEditCaretType fInsertCaret;
	#pragma pack(push, 1)
	Types::TPoint fCaretOffset;
	#pragma pack(pop)
	
	Syneditkeycmds::TSynEditKeyStrokes* fKeyStrokes;
	bool fModified;
	TSynEditMarkList* fMarkList;
	int fExtraLineSpacing;
	Synedittypes::TSynSelectionMode fSelectionMode;
	bool fWantTabs;
	Syneditmiscclasses::TSynGutter* fGutter;
	int fTabWidth;
	Syntextdrawer::TheTextDrawer* fTextDrawer;
	#pragma pack(push, 1)
	Types::TRect fInvalidateRect;
	#pragma pack(pop)
	
	TSynStateFlags fStateFlags;
	TSynEditorOptions fOptions;
	TSynStatusChanges fStatusChanges;
	Word fLastKey;
	Classes::TShiftState fLastShiftState;
	Syneditsearch::TSynEditSearch* fTSearch;
	Syneditsearch::TSynEditSearchOverride fOnSearchEngineOverride;
	Classes::TList* fHookedCommandHandlers;
	Syneditkbdhandler::TSynEditKbdHandler* fKbdHandler;
	Classes::TList* fFocusList;
	Classes::TList* fPlugins;
	Extctrls::TTimer* fScrollTimer;
	int fScrollDeltaX;
	int fScrollDeltaY;
	Classes::TNotifyEvent fOnChange;
	TPlaceMarkEvent fOnClearMark;
	TProcessCommandEvent fOnCommandProcessed;
	TDropFilesEvent fOnDropFiles;
	TGutterClickEvent fOnGutterClick;
	TPaintEvent fOnPaint;
	TPlaceMarkEvent fOnPlaceMark;
	TProcessCommandEvent fOnProcessCommand;
	Classes::TNotifyEvent FOnCancelMode;
	TProcessCommandEvent fOnProcessUserCommand;
	TReplaceTextEvent fOnReplaceText;
	TSpecialLineColorsEvent fOnSpecialLineColors;
	TContextHelpEvent fOnContextHelp;
	TPaintTransient fOnPaintTransient;
	TStatusChangeEvent fOnStatusChange;
	bool fWordWrap;
	bool fShowSpecChar;
	bool fIsAltSetColumnMode;
	bool fIsPasting;
	int FPaintTransientLock;
	void __fastcall BookMarkOptionsChanged(System::TObject* Sender);
	void __fastcall ComputeCaret(int X, int Y);
	void __fastcall DoBlockIndent(void);
	void __fastcall DoBlockUnindent(void);
	void __fastcall DoLinesDeleted(int FirstLine, int Count);
	void __fastcall DoLinesInserted(int FirstLine, int Count);
	void __fastcall DoTabKey(void);
	void __fastcall DoCaseChange(const Syneditkeycmds::TSynEditorCommand Cmd);
	int __fastcall FindHookedCmdEvent(THookedCommandEvent AHandlerProc);
	HIDESBASE void __fastcall FontChanged(System::TObject* Sender);
	Types::TPoint __fastcall GetBlockBegin();
	Types::TPoint __fastcall GetBlockEnd();
	bool __fastcall GetCanPaste(void);
	bool __fastcall GetCanRedo(void);
	bool __fastcall GetCanUndo(void);
	Types::TPoint __fastcall GetCaretXY();
	int __fastcall GetDisplayX(void);
	int __fastcall GetDisplayY(void);
	Types::TPoint __fastcall GetDisplayXY();
	Graphics::TFont* __fastcall GetFont(void);
	int __fastcall GetHookedCommandHandlersCount(void);
	AnsiString __fastcall GetLineText();
	int __fastcall GetMaxUndo(void);
	bool __fastcall GetSelAvail(void);
	bool __fastcall GetSelTabBlock(void);
	bool __fastcall GetSelTabLine(void);
	AnsiString __fastcall GetSelText();
	HIDESBASE AnsiString __fastcall GetText();
	AnsiString __fastcall GetWordAtCursor();
	void __fastcall GutterChanged(System::TObject* Sender);
	void __fastcall InsertBlock(const Types::TPoint &BB, const Types::TPoint &BE, char * ChangeStr);
	void __fastcall InsertBlockEx(const Types::TPoint &BB, const Types::TPoint &BE, char * ChangeStr, bool AddToUndoList);
	bool __fastcall IsPointInSelection(const Types::TPoint &Value);
	int __fastcall LeftSpaces(const AnsiString Line);
	int __fastcall LeftSpacesEx(const AnsiString Line, bool WantTabs);
	AnsiString __fastcall GetLeftSpacing(int CharCount, bool WantTabs);
	void __fastcall LinesChanging(System::TObject* Sender);
	void __fastcall LinesChanged(System::TObject* Sender);
	void __fastcall LockUndo(void);
	void __fastcall MoveCaretAndSelection(const Types::TPoint &ptBefore, const Types::TPoint &ptAfter, bool SelectionCommand);
	void __fastcall MoveCaretHorz(int DX, bool SelectionCommand);
	void __fastcall MoveCaretVert(int DY, bool SelectionCommand);
	void __fastcall PluginsAfterPaint(Graphics::TCanvas* ACanvas, const Types::TRect &AClip, int FirstLine, int LastLine);
	int __fastcall ScanFrom(int Index);
	void __fastcall ScrollTimerHandler(System::TObject* Sender);
	void __fastcall SelectedColorsChanged(System::TObject* Sender);
	void __fastcall SetWordWrap(bool Value);
	void __fastcall SetBlockBegin(const Types::TPoint &Value);
	void __fastcall SetBlockEnd(const Types::TPoint &Value);
	void __fastcall SetBorderStyle(Forms::TBorderStyle Value);
	void __fastcall SetCaretAndSelection(const Types::TPoint &ptCaret, const Types::TPoint &ptBefore, const Types::TPoint &ptAfter);
	void __fastcall SetCaretX(int Value);
	void __fastcall SetCaretY(int Value);
	void __fastcall SetExtraLineSpacing(const int Value);
	HIDESBASE void __fastcall SetFont(const Graphics::TFont* Value);
	void __fastcall SetGutter(const Syneditmiscclasses::TSynGutter* Value);
	void __fastcall SetGutterWidth(int Value);
	void __fastcall SetHideSelection(const bool Value);
	void __fastcall SetHighlighter(const Synedithighlighter::TSynCustomHighlighter* Value);
	void __fastcall SetInsertCaret(const TSynEditCaretType Value);
	void __fastcall SetInsertMode(const bool Value);
	void __fastcall SetKeystrokes(const Syneditkeycmds::TSynEditKeyStrokes* Value);
	void __fastcall SetOnKeyDown(const Controls::TKeyEvent Value);
	Controls::TKeyEvent __fastcall GetOnKeyDown();
	void __fastcall SetOnKeyPress(const Controls::TKeyPressEvent Value);
	Controls::TKeyPressEvent __fastcall GetOnKeyPress();
	void __fastcall SetLeftChar(int Value);
	void __fastcall SetLines(Classes::TStrings* Value);
	void __fastcall SetLineText(AnsiString Value);
	void __fastcall SetMaxLeftChar(int Value);
	void __fastcall SetMaxUndo(const int Value);
	void __fastcall SetModified(bool Value);
	void __fastcall SetOptions(TSynEditorOptions Value);
	void __fastcall SetOverwriteCaret(const TSynEditCaretType Value);
	void __fastcall SetRightEdge(int Value);
	void __fastcall SetRightEdgeColor(Graphics::TColor Value);
	void __fastcall SetScrollBars(const Stdctrls::TScrollStyle Value);
	void __fastcall SetSelectionMode(const Synedittypes::TSynSelectionMode Value);
	void __fastcall SetSelText(const AnsiString Value);
	void __fastcall SetSelTextExternal(const AnsiString Value);
	void __fastcall SetTabWidth(int Value);
	HIDESBASE void __fastcall SetText(const AnsiString Value);
	void __fastcall SetTopLine(int Value);
	void __fastcall SetWantTabs(const bool Value);
	void __fastcall SetWordBlock(const Types::TPoint &Value);
	void __fastcall SizeOrFontChanged(bool bFont);
	void __fastcall StatusChanged(TSynStatusChanges AChanges);
	void __fastcall TrimmedSetLine(int ALine, AnsiString ALineText);
	void __fastcall UndoRedoAdded(System::TObject* Sender);
	void __fastcall UnlockUndo(void);
	void __fastcall UpdateLastCaretX(void);
	void __fastcall UpdateScrollBars(void);
	void __fastcall DoShiftTabKey(void);
	void __fastcall DoHomeKey(bool Selection);
	
protected:
	virtual void __fastcall CreateParams(Controls::TCreateParams &Params);
	virtual void __fastcall CreateWnd(void);
	virtual void __fastcall DestroyWnd(void);
	DYNAMIC void __fastcall DblClick(void);
	void __fastcall DecPaintLock(void);
	DYNAMIC void __fastcall DragOver(System::TObject* Source, int X, int Y, Controls::TDragState State, bool &Accept);
	virtual bool __fastcall GetReadOnly(void);
	void __fastcall HighlighterAttrChanged(System::TObject* Sender);
	void __fastcall IncPaintLock(void);
	void __fastcall InitializeCaret(void);
	DYNAMIC void __fastcall KeyDown(Word &Key, Classes::TShiftState Shift);
	DYNAMIC void __fastcall KeyPress(char &Key);
	void __fastcall ListAdded(int Index, const AnsiString S);
	void __fastcall ListCleared(System::TObject* Sender);
	void __fastcall ListDeleted(int Index);
	void __fastcall ListInserted(int Index, const AnsiString S);
	void __fastcall ListPutted(int Index, const AnsiString S);
	void __fastcall ListScanRanges(System::TObject* Sender);
	virtual void __fastcall Loaded(void);
	void __fastcall MarkListChange(System::TObject* Sender);
	void __fastcall MBCSGetSelRangeInLineWhenColumnSelectionMode(const AnsiString s, int &ColFrom, int &ColTo);
	DYNAMIC void __fastcall MouseDown(Controls::TMouseButton Button, Classes::TShiftState Shift, int X, int Y);
	DYNAMIC void __fastcall MouseMove(Classes::TShiftState Shift, int X, int Y);
	DYNAMIC void __fastcall MouseUp(Controls::TMouseButton Button, Classes::TShiftState Shift, int X, int Y);
	virtual void __fastcall NotifyHookedCommandHandlers(bool AfterProcessing, Syneditkeycmds::TSynEditorCommand &Command, char &AChar, void * Data);
	virtual void __fastcall Paint(void);
	virtual void __fastcall PaintGutter(const Types::TRect &AClip, int FirstLine, int LastLine);
	virtual void __fastcall PaintTextLines(const Types::TRect &AClip, int FirstLine, int LastLine, int FirstCol, int LastCol);
	void __fastcall RecalcCharExtent(void);
	void __fastcall RedoItem(void);
	virtual void __fastcall SetCaretXY(const Types::TPoint &Value);
	virtual void __fastcall SetCaretXYEx(bool CallEnsureCursorPos, const Types::TPoint &Value);
	virtual void __fastcall SetName(const AnsiString Value);
	virtual void __fastcall SetReadOnly(bool Value);
	void __fastcall SetSelTextPrimitive(Synedittypes::TSynSelectionMode PasteMode, char * Value, System::PInteger Tag);
	void __fastcall SetSelTextPrimitiveEx(Synedittypes::TSynSelectionMode PasteMode, char * Value, System::PInteger Tag, bool AddToUndoList);
	Syneditkeycmds::TSynEditorCommand __fastcall TranslateKeyCode(Word Code, Classes::TShiftState Shift, void * &Data);
	void __fastcall UndoItem(void);
	int fGutterWidth;
	Syneditmiscclasses::TSynInternalImage* fInternalImage;
	bool FAlwaysShowCaret;
	__property bool WordWrap = {read=fWordWrap, write=SetWordWrap, nodefault};
	void __fastcall HideCaret(void);
	void __fastcall ShowCaret(void);
	virtual void __fastcall DoOnClearBookmark(TSynEditMark* &Mark);
	virtual void __fastcall DoOnCommandProcessed(Syneditkeycmds::TSynEditorCommand Command, char AChar, void * Data);
	virtual void __fastcall DoOnGutterClick(int X, int Y);
	virtual void __fastcall DoOnPaint(void);
	virtual void __fastcall DoOnPaintTransientEx(TTransientType TransientType, bool Lock);
	virtual void __fastcall DoOnPaintTransient(TTransientType TransientType);
	virtual void __fastcall DoOnPlaceMark(TSynEditMark* &Mark);
	virtual void __fastcall DoOnProcessCommand(Syneditkeycmds::TSynEditorCommand &Command, char &AChar, void * Data);
	virtual TSynReplaceAction __fastcall DoOnReplaceText(const AnsiString ASearch, const AnsiString AReplace, int Line, int Column);
	virtual bool __fastcall DoOnSpecialLineColors(int Line, Graphics::TColor &Foreground, Graphics::TColor &Background);
	virtual void __fastcall DoOnStatusChange(TSynStatusChanges Changes);
	int __fastcall GetSelEnd(void);
	int __fastcall GetSelStart(void);
	void __fastcall SetSelEnd(const int Value);
	void __fastcall SetSelStart(const int Value);
	void __fastcall SetAlwaysShowCaret(const bool Value);
	void __fastcall PrepareIdentChars(Synedittypes::TSynIdentChars &IdentChars, Synedittypes::TSynIdentChars &WhiteChars);
	
public:
	__property Canvas ;
	__property int SelStart = {read=GetSelStart, write=SetSelStart, nodefault};
	__property int SelEnd = {read=GetSelEnd, write=SetSelEnd, nodefault};
	__property bool AlwaysShowCaret = {read=FAlwaysShowCaret, write=SetAlwaysShowCaret, nodefault};
	void __fastcall UpdateCaret(void);
	void __fastcall AddKey(Syneditkeycmds::TSynEditorCommand Command, Word Key1, Classes::TShiftState SS1, Word Key2 = (Word)(0x0), Classes::TShiftState SS2 = System::Set<Classes__1, ssShift, ssDouble> () );
	void __fastcall BeginUndoBlock(void);
	void __fastcall BeginUpdate(void);
	int __fastcall CaretXPix(void);
	int __fastcall CaretYPix(void);
	void __fastcall ClearAll(void);
	void __fastcall ClearBookMark(int BookMark);
	void __fastcall ClearSelection(void);
	virtual void __fastcall CommandProcessor(Syneditkeycmds::TSynEditorCommand Command, char AChar, void * Data);
	void __fastcall ClearUndo(void);
	void __fastcall CopyToClipboard(void);
	__fastcall virtual TCustomSynEdit(Classes::TComponent* AOwner);
	void __fastcall CutToClipboard(void);
	__fastcall virtual ~TCustomSynEdit(void);
	void __fastcall DoCopyToClipboard(const AnsiString SText);
	DYNAMIC void __fastcall DragDrop(System::TObject* Source, int X, int Y);
	void __fastcall EndUndoBlock(void);
	void __fastcall EndUpdate(void);
	void __fastcall EnsureCursorPosVisible(void);
	void __fastcall EnsureCursorPosVisibleEx(bool ForceToMiddle);
	virtual void __fastcall FindMatchingBracket(void);
	virtual Types::TPoint __fastcall GetMatchingBracket();
	virtual Types::TPoint __fastcall GetMatchingBracketEx(const Types::TPoint &APoint, bool AdjustForTabs);
	DYNAMIC bool __fastcall ExecuteAction(Classes::TBasicAction* Action);
	virtual void __fastcall ExecuteCommand(Syneditkeycmds::TSynEditorCommand Command, char AChar, void * Data);
	bool __fastcall GetBookMark(int BookMark, int &X, int &Y);
	bool __fastcall GetHighlighterAttriAtRowCol(const Types::TPoint &XY, AnsiString &Token, Synedithighlighter::TSynHighlighterAttributes* &Attri);
	bool __fastcall GetHighlighterAttriAtRowColEx(const Types::TPoint &XY, AnsiString &Token, int &TokenType, int &Start, Synedithighlighter::TSynHighlighterAttributes* &Attri);
	AnsiString __fastcall GetWordAtRowCol(const Types::TPoint &XY);
	void __fastcall GotoBookMark(int BookMark);
	Synedittypes::TSynIdentChars __fastcall IdentChars();
	void __fastcall InvalidateGutter(void);
	void __fastcall InvalidateGutterLines(int FirstLine, int LastLine);
	void __fastcall InvalidateLine(int Line);
	void __fastcall InvalidateLines(int FirstLine, int LastLine);
	bool __fastcall IsBookmark(int BookMark);
	Types::TPoint __fastcall LogicalToPhysicalPos(const Types::TPoint &p);
	Types::TPoint __fastcall PhysicalToLogicalPos(const Types::TPoint &p);
	virtual Types::TPoint __fastcall NextWordPos();
	virtual void __fastcall Notification(Classes::TComponent* AComponent, Classes::TOperation Operation);
	void __fastcall PasteFromClipboard(void);
	virtual Types::TPoint __fastcall WordStart();
	virtual Types::TPoint __fastcall WordEnd();
	virtual Types::TPoint __fastcall PrevWordPos();
	Types::TPoint __fastcall PixelsToRowColumn(const Types::TPoint &Pixels);
	void __fastcall Redo(void);
	void __fastcall RegisterCommandHandler(THookedCommandEvent AHandlerProc, void * AHandlerData);
	Types::TPoint __fastcall RowColumnToPixels(const Types::TPoint &RowCol);
	int __fastcall SearchReplace(const AnsiString ASearch, const AnsiString AReplace, TSynSearchOptions AOptions);
	void __fastcall SelectAll(void);
	void __fastcall SetBookMark(int BookMark, int X, int Y);
	virtual void __fastcall SetDefaultKeystrokes(void);
	void __fastcall SetOptionFlag(TSynEditorOption Flag, bool Value);
	void __fastcall SetSelWord(void);
	void __fastcall Undo(void);
	void __fastcall UnregisterCommandHandler(THookedCommandEvent AHandlerProc);
	DYNAMIC bool __fastcall UpdateAction(Classes::TBasicAction* Action);
	virtual void __fastcall SetFocus(void);
	void __fastcall AddKeyDownHandler(Syneditkbdhandler::TKeyDownProc* aHandler);
	void __fastcall RemoveKeyDownHandler(Syneditkbdhandler::TKeyDownProc* aHandler);
	void __fastcall AddKeyPressHandler(Syneditkbdhandler::TKeyPressProc* aHandler);
	void __fastcall RemoveKeyPressHandler(Syneditkbdhandler::TKeyPressProc* aHandler);
	void __fastcall AddFocusControl(Controls::TWinControl* aControl);
	void __fastcall RemoveFocusControl(Controls::TWinControl* aControl);
	virtual void __fastcall WndProc(Messages::TMessage &Msg);
	__property Types::TPoint BlockBegin = {read=GetBlockBegin, write=SetBlockBegin};
	__property Types::TPoint BlockEnd = {read=GetBlockEnd, write=SetBlockEnd};
	__property bool CanPaste = {read=GetCanPaste, nodefault};
	__property bool CanRedo = {read=GetCanRedo, nodefault};
	__property bool CanUndo = {read=GetCanUndo, nodefault};
	__property int CaretX = {read=fCaretX, write=SetCaretX, nodefault};
	__property int CaretY = {read=fCaretY, write=SetCaretY, nodefault};
	__property Types::TPoint CaretXY = {read=GetCaretXY, write=SetCaretXY};
	__property int DisplayX = {read=GetDisplayX, nodefault};
	__property int DisplayY = {read=GetDisplayY, nodefault};
	__property Types::TPoint DisplayXY = {read=GetDisplayXY};
	__property int CharsInWindow = {read=fCharsInWindow, nodefault};
	__property int CharWidth = {read=fCharWidth, nodefault};
	__property Color  = {default=-2147483643};
	__property Graphics::TFont* Font = {read=GetFont, write=SetFont};
	__property Synedithighlighter::TSynCustomHighlighter* Highlighter = {read=fHighlighter, write=SetHighlighter};
	__property int LeftChar = {read=fLeftChar, write=SetLeftChar, nodefault};
	__property int LineHeight = {read=fTextHeight, nodefault};
	__property int LinesInWindow = {read=fLinesInWindow, nodefault};
	__property AnsiString LineText = {read=GetLineText, write=SetLineText};
	__property Classes::TStrings* Lines = {read=fLines, write=SetLines};
	__property TSynEditMarkList* Marks = {read=fMarkList};
	__property int MaxLeftChar = {read=fMaxLeftChar, write=SetMaxLeftChar, default=1024};
	__property bool Modified = {read=fModified, write=SetModified, nodefault};
	__property int PaintLock = {read=fPaintLock, nodefault};
	__property bool ReadOnly = {read=GetReadOnly, write=SetReadOnly, default=0};
	__property bool SelAvail = {read=GetSelAvail, nodefault};
	__property bool SelTabBlock = {read=GetSelTabBlock, nodefault};
	__property bool SelTabLine = {read=GetSelTabLine, nodefault};
	__property AnsiString SelText = {read=GetSelText, write=SetSelTextExternal};
	__property AnsiString Text = {read=GetText, write=SetText};
	__property int TopLine = {read=fTopLine, write=SetTopLine, nodefault};
	__property AnsiString WordAtCursor = {read=GetWordAtCursor};
	__property Synedittextbuffer::TSynEditUndoList* UndoList = {read=fUndoList};
	__property Synedittextbuffer::TSynEditUndoList* RedoList = {read=fRedoList};
	__property Controls::TKeyEvent OnKeyDown = {read=GetOnKeyDown, write=SetOnKeyDown};
	__property Controls::TKeyPressEvent OnKeyPress = {read=GetOnKeyPress, write=SetOnKeyPress};
	__property Classes::TNotifyEvent OnCancelMode = {read=FOnCancelMode, write=FOnCancelMode};
	__property TProcessCommandEvent OnProcessCommand = {read=fOnProcessCommand, write=fOnProcessCommand};
	__property TSynEditorOptions Options = {read=fOptions, write=SetOptions, default=53870738};
	__property Syneditmiscclasses::TSynBookMarkOpt* BookMarkOptions = {read=fBookMarkOpt, write=fBookMarkOpt};
	__property Forms::TBorderStyle BorderStyle = {read=fBorderStyle, write=SetBorderStyle, default=1};
	__property int ExtraLineSpacing = {read=fExtraLineSpacing, write=SetExtraLineSpacing, default=0};
	__property Syneditmiscclasses::TSynGutter* Gutter = {read=fGutter, write=SetGutter};
	__property bool HideSelection = {read=fHideSelection, write=SetHideSelection, default=0};
	__property TSynEditCaretType InsertCaret = {read=fInsertCaret, write=SetInsertCaret, default=0};
	__property bool InsertMode = {read=fInserting, write=SetInsertMode, default=1};
	__property Syneditkeycmds::TSynEditKeyStrokes* Keystrokes = {read=fKeyStrokes, write=SetKeystrokes};
	__property int MaxUndo = {read=GetMaxUndo, write=SetMaxUndo, default=1024};
	__property TSynEditCaretType OverwriteCaret = {read=fOverwriteCaret, write=SetOverwriteCaret, default=3};
	__property int RightEdge = {read=fRightEdge, write=SetRightEdge, default=80};
	__property Graphics::TColor RightEdgeColor = {read=fRightEdgeColor, write=SetRightEdgeColor, default=12632256};
	__property Graphics::TColor ScrollHintColor = {read=fScrollHintColor, write=fScrollHintColor, default=-2147483624};
	__property TScrollHintFormat ScrollHintFormat = {read=fScrollHintFormat, write=fScrollHintFormat, default=0};
	__property Stdctrls::TScrollStyle ScrollBars = {read=FScrollBars, write=SetScrollBars, default=3};
	__property Syneditmiscclasses::TSynSelectedColor* SelectedColor = {read=fSelectedColor, write=fSelectedColor};
	__property Synedittypes::TSynSelectionMode SelectionMode = {read=fSelectionMode, write=SetSelectionMode, default=0};
	__property int TabWidth = {read=fTabWidth, write=SetTabWidth, default=8};
	__property bool WantTabs = {read=fWantTabs, write=SetWantTabs, default=0};
	__property Classes::TNotifyEvent OnChange = {read=fOnChange, write=fOnChange};
	__property TPlaceMarkEvent OnClearBookmark = {read=fOnClearMark, write=fOnClearMark};
	__property TProcessCommandEvent OnCommandProcessed = {read=fOnCommandProcessed, write=fOnCommandProcessed};
	__property TContextHelpEvent OnContextHelp = {read=fOnContextHelp, write=fOnContextHelp};
	__property TDropFilesEvent OnDropFiles = {read=fOnDropFiles, write=fOnDropFiles};
	__property TGutterClickEvent OnGutterClick = {read=fOnGutterClick, write=fOnGutterClick};
	__property TPaintEvent OnPaint = {read=fOnPaint, write=fOnPaint};
	__property TPlaceMarkEvent OnPlaceBookmark = {read=fOnPlaceMark, write=fOnPlaceMark};
	__property TProcessCommandEvent OnProcessUserCommand = {read=fOnProcessUserCommand, write=fOnProcessUserCommand};
	__property TReplaceTextEvent OnReplaceText = {read=fOnReplaceText, write=fOnReplaceText};
	__property TSpecialLineColorsEvent OnSpecialLineColors = {read=fOnSpecialLineColors, write=fOnSpecialLineColors};
	__property TStatusChangeEvent OnStatusChange = {read=fOnStatusChange, write=fOnStatusChange};
	__property TPaintTransient OnPaintTransient = {read=fOnPaintTransient, write=fOnPaintTransient};
	__property Syneditsearch::TSynEditSearchOverride OnSearchEngineOverride = {read=fOnSearchEngineOverride, write=fOnSearchEngineOverride};
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TCustomSynEdit(HWND ParentWindow) : Controls::TCustomControl(ParentWindow) { }
	#pragma option pop
	
};



class DELPHICLASS TSynEditPlugin;
class PASCALIMPLEMENTATION TSynEditPlugin : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	TCustomSynEdit* fOwner;
	
protected:
	virtual void __fastcall AfterPaint(Graphics::TCanvas* ACanvas, const Types::TRect &AClip, int FirstLine, int LastLine) = 0 ;
	virtual void __fastcall LinesInserted(int FirstLine, int Count) = 0 ;
	virtual void __fastcall LinesDeleted(int FirstLine, int Count) = 0 ;
	__property TCustomSynEdit* Editor = {read=fOwner};
	
public:
	__fastcall TSynEditPlugin(TCustomSynEdit* AOwner);
	__fastcall virtual ~TSynEditPlugin(void);
};


class DELPHICLASS TSynEdit;
class PASCALIMPLEMENTATION TSynEdit : public TCustomSynEdit 
{
	typedef TCustomSynEdit inherited;
	
__published:
	__property Align  = {default=0};
	__property Anchors  = {default=3};
	__property Constraints ;
	__property Color  = {default=-2147483643};
	__property Ctl3D ;
	__property ParentCtl3D  = {default=1};
	__property Enabled  = {default=1};
	__property Font ;
	__property Height ;
	__property Name ;
	__property ParentColor  = {default=1};
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
	__property OnStartDock ;
	__property OnEndDrag ;
	__property OnEnter ;
	__property OnExit ;
	__property OnKeyDown ;
	__property OnKeyPress ;
	__property OnKeyUp ;
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
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
	__property ScrollHintColor  = {default=-2147483624};
	__property ScrollHintFormat  = {default=0};
	__property ScrollBars  = {default=3};
	__property SelectedColor ;
	__property SelectionMode  = {default=0};
	__property TabWidth  = {default=8};
	__property WantTabs  = {default=0};
	__property OnChange ;
	__property OnClearBookmark ;
	__property OnCommandProcessed ;
	__property OnContextHelp ;
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
	/* TCustomSynEdit.Create */ inline __fastcall virtual TSynEdit(Classes::TComponent* AOwner) : TCustomSynEdit(AOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomSynEdit.Destroy */ inline __fastcall virtual ~TSynEdit(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSynEdit(HWND ParentWindow) : TCustomSynEdit(ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
#define DIGIT (System::Set<char, 0, 255> () << '\x30' << '\x31' << '\x32' << '\x33' << '\x34' << '\x35' << '\x36' << '\x37' << '\x38' << '\x39' )
#define ALPHA_UC (System::Set<char, 0, 255> () << '\x41' << '\x42' << '\x43' << '\x44' << '\x45' << '\x46' << '\x47' << '\x48' << '\x49' << '\x4a' << '\x4b' << '\x4c' << '\x4d' << '\x4e' << '\x4f' << '\x50' << '\x51' << '\x52' << '\x53' << '\x54' << '\x55' << '\x56' << '\x57' << '\x58' << '\x59' << '\x5a' )
#define ALPHA_LC (System::Set<char, 0, 255> () << '\x61' << '\x62' << '\x63' << '\x64' << '\x65' << '\x66' << '\x67' << '\x68' << '\x69' << '\x6a' << '\x6b' << '\x6c' << '\x6d' << '\x6e' << '\x6f' << '\x70' << '\x71' << '\x72' << '\x73' << '\x74' << '\x75' << '\x76' << '\x77' << '\x78' << '\x79' << '\x7a' )
static const Word MAX_SCROLL = 0x7fff;
static const Shortint maxMarks = 0x10;
#define SYNEDIT_CLIPBOARD_FORMAT "SynEdit Control Block Type"
extern PACKAGE unsigned SynEditClipboardFormat;
#define SYNEDIT_DEFAULT_OPTIONS (System::Set<TSynEditorOption, eoAltSetsColumnMode, eoTrimTrailingSpaces> () << TSynEditorOption(1) << TSynEditorOption(4) << TSynEditorOption(7) << TSynEditorOption(17) << TSynEditorOption(18) << TSynEditorOption(20) << TSynEditorOption(21) << TSynEditorOption(24) << TSynEditorOption(25) )

}	/* namespace Synedit */
using namespace Synedit;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEdit
