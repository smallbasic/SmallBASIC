// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynAutoCorrect.pas' rev: 6.00

#ifndef SynAutoCorrectHPP
#define SynAutoCorrectHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <IniFiles.hpp>	// Pascal unit
#include <SynEdit.hpp>	// Pascal unit
#include <SynEditHighlighter.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <SynEditMiscProcs.hpp>	// Pascal unit
#include <Registry.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synautocorrect
{
//-- type declarations -------------------------------------------------------
struct TRecordEditorVars
{
	Controls::TKeyEvent kd;
	Controls::TMouseEvent md;
} ;

typedef TRecordEditorVars *PRecordEditorVars;

class DELPHICLASS TSynAutoCorrect;
class PASCALIMPLEMENTATION TSynAutoCorrect : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	bool FAutoCorrectOnMouseDown;
	bool FBeepOnAutoCorrect;
	bool FEnabled;
	Synedit::TCustomSynEdit* FEditor;
	TRecordEditorVars *FEditVars;
	bool FIgnoreCase;
	Classes::TStrings* FReplaceItems;
	Synedit::TReplaceTextEvent FOnReplaceText;
	int PrevLine;
	bool FMaintainCase;
	int __fastcall CorrectItemStart(AnsiString EditLine, AnsiString SearchString, int StartPos, bool frMatchCase, bool frWholeWord);
	bool __fastcall FindAndCorrect(AnsiString &EditLine, AnsiString SearchString, AnsiString ReplaceText, int &CurrentX);
	AnsiString __fastcall PreviousToken();
	Classes::TStrings* __fastcall GetReplaceItems(void);
	void __fastcall SetReplaceItems(const Classes::TStrings* Value);
	
protected:
	virtual void __fastcall EditorKeyDown(System::TObject* Sender, Word &Key, Classes::TShiftState Shift);
	virtual void __fastcall EditorMouseDown(System::TObject* Sender, Controls::TMouseButton Button, Classes::TShiftState Shift, int X, int Y);
	void __fastcall SetEditor(Synedit::TCustomSynEdit* Value);
	
public:
	__fastcall virtual TSynAutoCorrect(Classes::TComponent* AOwner);
	__fastcall virtual ~TSynAutoCorrect(void);
	void __fastcall Add(AnsiString sReplaceFrom, AnsiString sReplaceTo);
	void __fastcall AutoCorrectAll(void);
	void __fastcall Delete(int iItemIndex);
	void __fastcall Edit(int iItemIndex, AnsiString sReplaceFrom, AnsiString sReplaceTo);
	void __fastcall LoadFromIni(AnsiString FileName, AnsiString Section);
	void __fastcall SaveToIni(AnsiString FileName, AnsiString Section);
	bool __fastcall LoadFromList(AnsiString FileName);
	bool __fastcall SaveToList(AnsiString FileName);
	void __fastcall LoadFromRegistry(unsigned RegistryRoot, AnsiString RegistryKey);
	void __fastcall SaveToRegistry(unsigned RegistryRoot, AnsiString RegistryKey);
	
__published:
	__property bool AutoCorrectOnMouseDown = {read=FAutoCorrectOnMouseDown, write=FAutoCorrectOnMouseDown, nodefault};
	__property bool BeepOnAutoCorrect = {read=FBeepOnAutoCorrect, write=FBeepOnAutoCorrect, nodefault};
	__property bool Enabled = {read=FEnabled, write=FEnabled, nodefault};
	__property Synedit::TCustomSynEdit* Editor = {read=FEditor, write=SetEditor};
	__property bool IgnoreCase = {read=FIgnoreCase, write=FIgnoreCase, nodefault};
	__property Classes::TStrings* ReplaceItems = {read=GetReplaceItems, write=SetReplaceItems};
	__property bool MaintainCase = {read=FMaintainCase, write=FMaintainCase, default=1};
	__property Synedit::TReplaceTextEvent OnReplaceText = {read=FOnReplaceText, write=FOnReplaceText};
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE System::Set<Byte, 0, 255>  DELIMITERS;
extern PACKAGE System::Set<Byte, 0, 255>  NUMBERS;
extern PACKAGE AnsiString __fastcall HalfString(AnsiString Str, bool FirstHalf);
extern PACKAGE AnsiString __fastcall StrLeft(const AnsiString S, int Count);
extern PACKAGE AnsiString __fastcall StringsToStr(const Classes::TStrings* List, AnsiString Sep);
extern PACKAGE void __fastcall StrToStrings(AnsiString S, AnsiString Sep, const Classes::TStrings* List);

}	/* namespace Synautocorrect */
using namespace Synautocorrect;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynAutoCorrect
