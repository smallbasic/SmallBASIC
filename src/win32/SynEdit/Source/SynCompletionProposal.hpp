// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynCompletionProposal.pas' rev: 6.00

#ifndef SynCompletionProposalHPP
#define SynCompletionProposalHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEdit.hpp>	// Pascal unit
#include <SynEditKbdHandler.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditKeyCmds.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syncompletionproposal
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum SynCompletionType { ctCode, ctHint, ctParams };
#pragma option pop

typedef TCustomForm TSynForm;
;

typedef void __fastcall (__closure *TSynBaseCompletionProposalPaintItem)(int Index, Graphics::TCanvas* ACanvas, const Types::TRect &Rect, bool &aCustomDraw);

typedef void __fastcall (__closure *TCodeCompletionEvent)(AnsiString &Value, Classes::TShiftState Shift, int Index, char EndToken);

typedef void __fastcall (__closure *TAfterCodeCompletionEvent)(const AnsiString Value, Classes::TShiftState Shift, int Index, char EndToken);

typedef void __fastcall (__closure *TValidateEvent)(System::TObject* Sender, Classes::TShiftState Shift, char EndToken);

typedef void __fastcall (__closure *TCompletionParameter)(System::TObject* Sender, int CurrentIndex, int &Level, int &IndexToDisplay, char &Key, AnsiString &DisplayString);

typedef void __fastcall (__closure *TCompletionExecute)(SynCompletionType Kind, System::TObject* Sender, AnsiString &AString, int &x, int &y, bool &CanExecute);

typedef void __fastcall (__closure *TCompletionChange)(System::TObject* Sender, int AIndex);

#pragma option push -b-
enum TSynCompletionOption { scoAnsiStrings, scoCaseSensitive, scoLimitToMatchedText, scoTitleIsCentered, scoUseInsertList, scoUsePrettyText, scoUseBuiltInTimer, scoEndCharCompletion };
#pragma option pop

typedef Set<TSynCompletionOption, scoAnsiStrings, scoEndCharCompletion>  TSynCompletionOptions;

class DELPHICLASS TSynBaseCompletionProposalForm;
class PASCALIMPLEMENTATION TSynBaseCompletionProposalForm : public Forms::TCustomForm 
{
	typedef Forms::TCustomForm inherited;
	
protected:
	AnsiString FCurrentString;
	Controls::TKeyPressEvent FOnKeyPress;
	Classes::TNotifyEvent FOnKeyDelete;
	TSynBaseCompletionProposalPaintItem FOnPaintItem;
	TCompletionChange FChangePosition;
	Classes::TStrings* FItemList;
	Classes::TStrings* FInsertList;
	Classes::TStrings* FAssignedList;
	int FPosition;
	int FNbLinesInWindow;
	int FTitleFontHeight;
	int FFontHeight;
	Stdctrls::TScrollBar* Scroll;
	TValidateEvent FOnValidate;
	Classes::TNotifyEvent FOnCancel;
	Graphics::TColor FClSelect;
	Graphics::TColor fClSelectText;
	Graphics::TColor fClBackGround;
	bool FAnsi;
	bool fCase;
	bool FUsePrettyText;
	bool FMatchText;
	int FMouseWheelAccumulator;
	SynCompletionType FDisplayKind;
	TCompletionParameter FParameterToken;
	int FCurrentIndex;
	int FCurrentLevel;
	SynCompletionType FDefaultKind;
	AnsiString FBiggestWord;
	AnsiString FEndOfTokenChr;
	AnsiString fTriggerChars;
	bool OldShowCaret;
	int fHeightBuffer;
	void __fastcall SetCurrentString(const AnsiString Value);
	DYNAMIC void __fastcall KeyDown(Word &Key, Classes::TShiftState Shift);
	DYNAMIC void __fastcall KeyPress(char &Key);
	DYNAMIC void __fastcall Paint(void);
	void __fastcall ScrollGetFocus(System::TObject* Sender);
	DYNAMIC void __fastcall Activate(void);
	DYNAMIC void __fastcall Deactivate(void);
	void __fastcall MoveLine(int cnt);
	void __fastcall ScrollChange(System::TObject* Sender);
	void __fastcall ScrollOnScroll(System::TObject* Sender, Stdctrls::TScrollCode ScrollCode, int &ScrollPos);
	void __fastcall SetItemList(const Classes::TStrings* Value);
	void __fastcall SetInsertList(const Classes::TStrings* Value);
	HIDESBASE void __fastcall SetPosition(const int Value);
	void __fastcall SetNbLinesInWindow(const int Value);
	DYNAMIC void __fastcall MouseDown(Controls::TMouseButton Button, Classes::TShiftState Shift, int X, int Y);
	HIDESBASE MESSAGE void __fastcall WMMouseWheel(Messages::TMessage &Msg);
	void __fastcall StringListChange(System::TObject* Sender);
	void __fastcall DoDoubleClick(System::TObject* Sender);
	AnsiString __fastcall intLowerCase(AnsiString s);
	void __fastcall DoFormShow(System::TObject* Sender);
	void __fastcall DoFormHide(System::TObject* Sender);
	void __fastcall AdjustScrollBarPosition(void);
	
private:
	Graphics::TBitmap* Bitmap;
	Classes::TComponent* fCurrentEditor;
	bool FUseInsertList;
	AnsiString fTitle;
	Graphics::TFont* fTitleFont;
	Graphics::TFont* fFont;
	Graphics::TColor fClTitleBackground;
	bool FCenterTitle;
	HIDESBASE MESSAGE void __fastcall WMActivate(Messages::TWMActivate &Message);
	MESSAGE void __fastcall WMEraseBackgrnd(Messages::TMessage &Message);
	MESSAGE void __fastcall WMGetDlgCode(Messages::TWMNoParams &Message);
	void __fastcall SetTitle(const AnsiString Value);
	HIDESBASE void __fastcall SetFont(const Graphics::TFont* Value);
	void __fastcall SetTitleFont(const Graphics::TFont* Value);
	void __fastcall TitleFontChange(System::TObject* Sender);
	void __fastcall FontChange(System::TObject* Sender);
	
public:
	__fastcall virtual TSynBaseCompletionProposalForm(Classes::TComponent* AOwner);
	virtual void __fastcall CreateParams(Controls::TCreateParams &Params);
	__fastcall virtual ~TSynBaseCompletionProposalForm(void);
	__property SynCompletionType DisplayType = {read=FDisplayKind, write=FDisplayKind, nodefault};
	__property SynCompletionType DefaultType = {read=FDefaultKind, write=FDefaultKind, nodefault};
	__property AnsiString CurrentString = {read=FCurrentString, write=SetCurrentString};
	__property int CurrentIndex = {read=FCurrentIndex, write=FCurrentIndex, nodefault};
	__property int CurrentLevel = {read=FCurrentLevel, write=FCurrentLevel, nodefault};
	__property TCompletionParameter OnParameterToken = {read=FParameterToken, write=FParameterToken};
	__property Controls::TKeyPressEvent OnKeyPress = {read=FOnKeyPress, write=FOnKeyPress};
	__property Classes::TNotifyEvent OnKeyDelete = {read=FOnKeyDelete, write=FOnKeyDelete};
	__property TSynBaseCompletionProposalPaintItem OnPaintItem = {read=FOnPaintItem, write=FOnPaintItem};
	__property TValidateEvent OnValidate = {read=FOnValidate, write=FOnValidate};
	__property Classes::TNotifyEvent OnCancel = {read=FOnCancel, write=FOnCancel};
	__property Classes::TStrings* ItemList = {read=FItemList, write=SetItemList};
	__property Classes::TStrings* InsertList = {read=FInsertList, write=SetInsertList};
	__property Classes::TStrings* AssignedList = {read=FAssignedList, write=FAssignedList};
	__property int Position = {read=FPosition, write=SetPosition, nodefault};
	__property int NbLinesInWindow = {read=FNbLinesInWindow, write=SetNbLinesInWindow, nodefault};
	__property AnsiString BiggestWord = {read=FBiggestWord, write=FBiggestWord};
	__property AnsiString Title = {read=fTitle, write=SetTitle};
	__property Graphics::TColor ClSelect = {read=FClSelect, write=FClSelect, nodefault};
	__property Graphics::TColor ClSelectedText = {read=fClSelectText, write=fClSelectText, nodefault};
	__property Graphics::TColor ClBackground = {read=fClBackGround, write=fClBackGround, nodefault};
	__property Graphics::TColor ClTitleBackground = {read=fClTitleBackground, write=fClTitleBackground, nodefault};
	__property bool UsePrettyText = {read=FUsePrettyText, write=FUsePrettyText, default=0};
	__property bool UseInsertList = {read=FUseInsertList, write=FUseInsertList, default=0};
	__property bool CenterTitle = {read=FCenterTitle, write=FCenterTitle, default=1};
	__property bool AnsiStrings = {read=FAnsi, write=FAnsi, nodefault};
	__property bool CaseSensitive = {read=fCase, write=fCase, nodefault};
	__property Classes::TComponent* CurrentEditor = {read=fCurrentEditor, write=fCurrentEditor};
	__property bool MatchText = {read=FMatchText, write=FMatchText, nodefault};
	__property AnsiString EndOfTokenChr = {read=FEndOfTokenChr, write=FEndOfTokenChr};
	__property AnsiString TriggerChars = {read=fTriggerChars, write=fTriggerChars};
	__property Graphics::TFont* TitleFont = {read=fTitleFont, write=SetTitleFont};
	__property Graphics::TFont* Font = {read=fFont, write=SetFont};
public:
	#pragma option push -w-inl
	/* TCustomForm.CreateNew */ inline __fastcall virtual TSynBaseCompletionProposalForm(Classes::TComponent* AOwner, int Dummy) : Forms::TCustomForm(AOwner, Dummy) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSynBaseCompletionProposalForm(HWND ParentWindow) : Forms::TCustomForm(ParentWindow) { }
	#pragma option pop
	
};


class DELPHICLASS TSynBaseCompletionProposal;
class PASCALIMPLEMENTATION TSynBaseCompletionProposal : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	TSynBaseCompletionProposalForm* FForm;
	TCompletionExecute FOnExecute;
	int FWidth;
	AnsiString FBiggestWord;
	int FDotOffset;
	int FOldPos;
	int FOldLeft;
	AnsiString FOldStr;
	TSynCompletionOptions fOptions;
	Graphics::TColor __fastcall GetClSelect(void);
	void __fastcall SetClSelect(const Graphics::TColor Value);
	AnsiString __fastcall GetCurrentString();
	Classes::TStrings* __fastcall GetItemList(void);
	Classes::TStrings* __fastcall GetInsertList(void);
	int __fastcall GetNbLinesInWindow(void);
	Classes::TNotifyEvent __fastcall GetOnCancel();
	Controls::TKeyPressEvent __fastcall GetOnKeyPress();
	TSynBaseCompletionProposalPaintItem __fastcall GetOnPaintItem();
	TValidateEvent __fastcall GetOnValidate();
	int __fastcall GetPosition(void);
	void __fastcall SetCurrentString(const AnsiString Value);
	void __fastcall SetItemList(const Classes::TStrings* Value);
	void __fastcall SetInsertList(const Classes::TStrings* Value);
	void __fastcall SetNbLinesInWindow(const int Value);
	void __fastcall SetOnCancel(const Classes::TNotifyEvent Value);
	void __fastcall SetOnKeyPress(const Controls::TKeyPressEvent Value);
	void __fastcall SetOnPaintItem(const TSynBaseCompletionProposalPaintItem Value);
	void __fastcall SetPosition(const int Value);
	void __fastcall SetOnValidate(const TValidateEvent Value);
	Classes::TNotifyEvent __fastcall GetOnKeyDelete();
	void __fastcall SetOnKeyDelete(const Classes::TNotifyEvent Value);
	void __fastcall SetWidth(int Value);
	SynCompletionType __fastcall GetDisplayKind(void);
	void __fastcall SetDisplayKind(const SynCompletionType Value);
	TCompletionParameter __fastcall GetParameterToken();
	void __fastcall SetParameterToken(const TCompletionParameter Value);
	SynCompletionType __fastcall GetDefaultKind(void);
	void __fastcall SetDefaultKind(const SynCompletionType Value);
	void __fastcall SetUseBiggestWord(const AnsiString Value);
	bool __fastcall IsEndToken(char AChar);
	Graphics::TColor __fastcall GetClBack(void);
	void __fastcall SetClBack(const Graphics::TColor Value);
	Graphics::TColor __fastcall GetClSelectedText(void);
	void __fastcall SetClSelectedText(const Graphics::TColor Value);
	AnsiString __fastcall GetEndOfTokenChar();
	void __fastcall SetEndOfTokenChar(const AnsiString Value);
	Graphics::TColor __fastcall GetClTitleBackground(void);
	void __fastcall SetClTitleBackground(const Graphics::TColor Value);
	void __fastcall SetTitle(const AnsiString Value);
	AnsiString __fastcall GetTitle();
	Graphics::TFont* __fastcall GetFont(void);
	Graphics::TFont* __fastcall GetTitleFont(void);
	void __fastcall SetFont(const Graphics::TFont* Value);
	void __fastcall SetTitleFont(const Graphics::TFont* Value);
	TSynCompletionOptions __fastcall GetOptions(void);
	AnsiString __fastcall GetTriggerChars();
	void __fastcall SetTriggerChars(const AnsiString Value);
	TCompletionChange __fastcall GetOnChange();
	void __fastcall SetOnChange(const TCompletionChange Value);
	
protected:
	virtual void __fastcall loaded(void);
	virtual void __fastcall SetOptions(const TSynCompletionOptions Value);
	virtual void __fastcall EditorCancelMode(System::TObject* Sender);
	virtual void __fastcall HookedEditorCommand(System::TObject* Sender, bool AfterProcessing, bool &Handled, Syneditkeycmds::TSynEditorCommand &Command, char &AChar, void * Data, void * HandlerData);
	
public:
	__fastcall virtual TSynBaseCompletionProposal(Classes::TComponent* Aowner);
	__fastcall virtual ~TSynBaseCompletionProposal(void);
	void __fastcall Execute(AnsiString s, int x, int y);
	virtual void __fastcall ExecuteEx(AnsiString s, int x, int y, SynCompletionType Kind = (SynCompletionType)(0x0));
	void __fastcall Activate(void);
	void __fastcall Deactivate(void);
	__property Controls::TKeyPressEvent OnKeyPress = {read=GetOnKeyPress, write=SetOnKeyPress};
	__property Classes::TNotifyEvent OnKeyDelete = {read=GetOnKeyDelete, write=SetOnKeyDelete};
	__property TValidateEvent OnValidate = {read=GetOnValidate, write=SetOnValidate};
	__property Classes::TNotifyEvent OnCancel = {read=GetOnCancel, write=SetOnCancel};
	__property AnsiString CurrentString = {read=GetCurrentString, write=SetCurrentString};
	__property int DotOffset = {read=FDotOffset, write=FDotOffset, nodefault};
	__property SynCompletionType DisplayType = {read=GetDisplayKind, write=SetDisplayKind, nodefault};
	__property TSynBaseCompletionProposalForm* Form = {read=FForm, write=FForm};
	__property AnsiString PreviousWord = {read=FOldStr};
	
__published:
	__property SynCompletionType DefaultType = {read=GetDefaultKind, write=SetDefaultKind, nodefault};
	__property TSynCompletionOptions Options = {read=GetOptions, write=SetOptions, nodefault};
	__property TCompletionExecute OnExecute = {read=FOnExecute, write=FOnExecute};
	__property TCompletionParameter OnParameterToken = {read=GetParameterToken, write=SetParameterToken};
	__property TSynBaseCompletionProposalPaintItem OnPaintItem = {read=GetOnPaintItem, write=SetOnPaintItem};
	__property TCompletionChange OnChange = {read=GetOnChange, write=SetOnChange};
	void __fastcall ClearList(void);
	AnsiString __fastcall DisplayItem(int AIndex);
	AnsiString __fastcall InsertItem(int AIndex);
	void __fastcall AddItemAt(int Where, AnsiString ADisplayText, AnsiString AInsertText);
	void __fastcall AddItem(AnsiString ADisplayText, AnsiString AInsertText);
	__property Classes::TStrings* ItemList = {read=GetItemList, write=SetItemList};
	void __fastcall ResetAssignedList(void);
	__property Classes::TStrings* InsertList = {read=GetInsertList, write=SetInsertList};
	__property int Position = {read=GetPosition, write=SetPosition, nodefault};
	__property int NbLinesInWindow = {read=GetNbLinesInWindow, write=SetNbLinesInWindow, nodefault};
	__property Graphics::TColor ClSelect = {read=GetClSelect, write=SetClSelect, nodefault};
	__property Graphics::TColor ClSelectedText = {read=GetClSelectedText, write=SetClSelectedText, nodefault};
	__property Graphics::TColor ClBackground = {read=GetClBack, write=SetClBack, nodefault};
	__property int Width = {read=FWidth, write=SetWidth, nodefault};
	__property AnsiString BiggestWord = {read=FBiggestWord, write=SetUseBiggestWord};
	__property AnsiString EndOfTokenChr = {read=GetEndOfTokenChar, write=SetEndOfTokenChar};
	__property AnsiString TriggerChars = {read=GetTriggerChars, write=SetTriggerChars};
	__property AnsiString Title = {read=GetTitle, write=SetTitle};
	__property Graphics::TColor ClTitleBackground = {read=GetClTitleBackground, write=SetClTitleBackground, nodefault};
	__property Graphics::TFont* Font = {read=GetFont, write=SetFont};
	__property Graphics::TFont* TitleFont = {read=GetTitleFont, write=SetTitleFont};
};


class DELPHICLASS TSynCompletionProposal;
class PASCALIMPLEMENTATION TSynCompletionProposal : public TSynBaseCompletionProposal 
{
	typedef TSynBaseCompletionProposal inherited;
	
private:
	Classes::TList* fEditors;
	Classes::TShortCut FShortCut;
	Syneditkbdhandler::TKeyDownProc* fKeyDownProc;
	Syneditkbdhandler::TKeyPressProc* fKeyPressProc;
	bool fNoNextKey;
	TCodeCompletionEvent FOnCodeCompletion;
	Extctrls::TTimer* fTimer;
	int fTimerInterval;
	int fCurEditor;
	TAfterCodeCompletionEvent FOnAfterCodeCompletion;
	Classes::TNotifyEvent FOnCancelled;
	void __fastcall SetEditor(const Synedit::TCustomSynEdit* Value);
	void __fastcall HandleOnKeyDelete(System::TObject* Sender);
	void __fastcall HandleOnCancel(System::TObject* Sender);
	void __fastcall HandleOnValidate(System::TObject* Sender, Classes::TShiftState Shift, char EndToken);
	void __fastcall HandleOnKeyPress(System::TObject* Sender, char &Key);
	void __fastcall HandleDblClick(System::TObject* Sender);
	void __fastcall EditorKeyDown(System::TObject* Sender, Word &Key, Classes::TShiftState Shift);
	void __fastcall EditorKeyPress(System::TObject* Sender, char &Key);
	void __fastcall TimerExecute(System::TObject* Sender);
	AnsiString __fastcall GetPreviousToken(Synedit::TCustomSynEdit* FEditor);
	int __fastcall GetTimerInterval(void);
	void __fastcall SetTimerInterval(const int Value);
	Synedit::TCustomSynEdit* __fastcall GetFEditor(void);
	Synedit::TCustomSynEdit* __fastcall GetEditor(int i);
	void __fastcall RemoveCurrentEditor(void);
	void __fastcall InternalCancelCompletion(void);
	
protected:
	virtual void __fastcall DoExecute(Synedit::TCustomSynEdit* AEditor);
	virtual void __fastcall Notification(Classes::TComponent* AComponent, Classes::TOperation Operation);
	void __fastcall SetShortCut(Classes::TShortCut Value);
	virtual void __fastcall SetOptions(const TSynCompletionOptions Value);
	virtual void __fastcall EditorCancelMode(System::TObject* Sender);
	virtual void __fastcall HookedEditorCommand(System::TObject* Sender, bool AfterProcessing, bool &Handled, Syneditkeycmds::TSynEditorCommand &Command, char &AChar, void * Data, void * HandlerData);
	
public:
	__fastcall virtual TSynCompletionProposal(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynCompletionProposal(void);
	__property Synedit::TCustomSynEdit* Editors[int i] = {read=GetEditor};
	void __fastcall AddEditor(Synedit::TCustomSynEdit* AEditor);
	bool __fastcall RemoveEditor(Synedit::TCustomSynEdit* AEditor);
	int __fastcall EditorsCount(void);
	virtual void __fastcall ExecuteEx(AnsiString s, int x, int y, SynCompletionType Kind = (SynCompletionType)(0x0));
	void __fastcall ActivateCompletion(void);
	void __fastcall CancelCompletion(void);
	
__published:
	__property Classes::TShortCut ShortCut = {read=FShortCut, write=SetShortCut, nodefault};
	__property Synedit::TCustomSynEdit* Editor = {read=GetFEditor, write=SetEditor};
	__property int TimerInterval = {read=GetTimerInterval, write=SetTimerInterval, nodefault};
	__property EndOfTokenChr ;
	__property TCodeCompletionEvent OnCodeCompletion = {read=FOnCodeCompletion, write=FOnCodeCompletion};
	__property TAfterCodeCompletionEvent OnAfterCodeCompletion = {read=FOnAfterCodeCompletion, write=FOnAfterCodeCompletion};
	__property Classes::TNotifyEvent OnCancelled = {read=FOnCancelled, write=FOnCancelled};
};


class DELPHICLASS TSynAutoComplete;
class PASCALIMPLEMENTATION TSynAutoComplete : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	Classes::TShortCut FShortCut;
	Synedit::TCustomSynEdit* fEditor;
	Classes::TStrings* fAutoCompleteList;
	Syneditkbdhandler::TKeyDownProc* fKeyDownProc;
	Syneditkbdhandler::TKeyPressProc* fKeyPressProc;
	bool fNoNextKey;
	AnsiString FEndOfTokenChr;
	Classes::TNotifyEvent FOnBeforeExecute;
	Classes::TNotifyEvent FOnAfterExecute;
	void __fastcall SetAutoCompleteList(Classes::TStrings* List);
	void __fastcall SetEditor(const Synedit::TCustomSynEdit* Value);
	
protected:
	void __fastcall SetShortCut(Classes::TShortCut Value);
	virtual void __fastcall Notification(Classes::TComponent* AComponent, Classes::TOperation Operation);
	virtual void __fastcall EditorKeyDown(System::TObject* Sender, Word &Key, Classes::TShiftState Shift);
	virtual void __fastcall EditorKeyPress(System::TObject* Sender, char &Key);
	AnsiString __fastcall GetPreviousToken(Synedit::TCustomSynEdit* Editor);
	
public:
	__fastcall virtual TSynAutoComplete(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynAutoComplete(void);
	void __fastcall Execute(AnsiString token, Synedit::TCustomSynEdit* Editor);
	bool __fastcall RemoveEditor(Synedit::TCustomSynEdit* aEditor);
	AnsiString __fastcall GetTokenList();
	AnsiString __fastcall GetTokenValue(AnsiString Token);
	
__published:
	__property Classes::TStrings* AutoCompleteList = {read=fAutoCompleteList, write=SetAutoCompleteList};
	__property AnsiString EndOfTokenChr = {read=FEndOfTokenChr, write=FEndOfTokenChr};
	__property Synedit::TCustomSynEdit* Editor = {read=fEditor, write=SetEditor};
	__property Classes::TShortCut ShortCut = {read=FShortCut, write=SetShortCut, nodefault};
	__property Classes::TNotifyEvent OnBeforeExecute = {read=FOnBeforeExecute, write=FOnBeforeExecute};
	__property Classes::TNotifyEvent OnAfterExecute = {read=FOnAfterExecute, write=FOnAfterExecute};
};


//-- var, const, procedure ---------------------------------------------------
#define DefaultProposalOptions (System::Set<TSynCompletionOption, scoAnsiStrings, scoEndCharCompletion> () << TSynCompletionOption(2) << TSynCompletionOption(7) )
extern PACKAGE void __fastcall PrettyTextOut(Graphics::TCanvas* c, int x, int y, AnsiString s, bool DoAlign, AnsiString BiggestWord, bool Highlighted);

}	/* namespace Syncompletionproposal */
using namespace Syncompletionproposal;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynCompletionProposal
