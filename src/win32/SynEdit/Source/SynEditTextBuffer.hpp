// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditTextBuffer.pas' rev: 6.00

#ifndef SynEditTextBufferHPP
#define SynEditTextBufferHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditMiscProcs.hpp>	// Pascal unit
#include <SynEditTypes.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synedittextbuffer
{
//-- type declarations -------------------------------------------------------
typedef void *TSynEditRange;

#pragma option push -b-
enum TSynEditStringFlag { sfHasTabs, sfHasNoTabs, sfExpandedLengthUnknown };
#pragma option pop

typedef Set<TSynEditStringFlag, sfHasTabs, sfExpandedLengthUnknown>  TSynEditStringFlags;

struct TSynEditStringRec;
typedef TSynEditStringRec *PSynEditStringRec;

#pragma pack(push, 4)
struct TSynEditStringRec
{
	AnsiString fString;
	System::TObject* fObject;
	void *fRange;
	bool fWraped;
	int fExpandedLength;
	TSynEditStringFlags fFlags;
} ;
#pragma pack(pop)

typedef TSynEditStringRec TSynEditStringRecList[89478485];

typedef TSynEditStringRec *PSynEditStringRecList;

typedef void __fastcall (__closure *TStringListIndexEvent)(int Index);

typedef void __fastcall (__closure *TStringListInsertedEvent)(int Index, const AnsiString s);

#pragma option push -b-
enum TSynEditFileFormat { sffDos, sffUnix, sffMac };
#pragma option pop

class DELPHICLASS TSynEditStringList;
class PASCALIMPLEMENTATION TSynEditStringList : public Classes::TStrings 
{
	typedef Classes::TStrings inherited;
	
private:
	bool fWordWrap;
	int fWordWrapWidth;
	TSynEditStringRec *fList;
	int fCount;
	int fCapacity;
	TSynEditFileFormat fFileFormat;
	bool fAppendNewLineAtEOF;
	Syneditmiscprocs::TConvertTabsProcEx fConvertTabsProc;
	int fIndexOfLongestLine;
	int fTabWidth;
	Classes::TNotifyEvent fOnChange;
	Classes::TNotifyEvent fOnChanging;
	AnsiString __fastcall WrapString(AnsiString &InputString);
	void __fastcall WrapLine(const int LineNumber);
	AnsiString __fastcall ExpandedString(int Index);
	AnsiString __fastcall GetExpandedString(int Index);
	int __fastcall GetLengthOfLongestLine(void);
	void * __fastcall GetRange(int Index);
	void __fastcall Grow(void);
	void __fastcall InsertItem(int Index, const AnsiString S);
	void __fastcall PutRange(int Index, void * ARange);
	void __fastcall SetWordWrap(const bool Value);
	
protected:
	int fLongestLineIndex;
	TStringListInsertedEvent fOnAdded;
	Classes::TNotifyEvent fOnCleared;
	TStringListIndexEvent fOnDeleted;
	TStringListInsertedEvent fOnInserted;
	TStringListInsertedEvent fOnPutted;
	virtual AnsiString __fastcall Get(int Index);
	virtual int __fastcall GetCapacity(void);
	virtual int __fastcall GetCount(void);
	virtual System::TObject* __fastcall GetObject(int Index);
	virtual void __fastcall Put(int Index, const AnsiString S);
	virtual void __fastcall PutObject(int Index, System::TObject* AObject);
	virtual void __fastcall SetCapacity(int NewCapacity);
	void __fastcall SetTabWidth(int Value);
	virtual void __fastcall SetUpdateState(bool Updating);
	
public:
	__fastcall TSynEditStringList(void);
	__fastcall virtual ~TSynEditStringList(void);
	virtual int __fastcall Add(const AnsiString S);
	virtual void __fastcall AddStrings(Classes::TStrings* Strings);
	virtual void __fastcall Clear(void);
	virtual void __fastcall Delete(int Index);
	void __fastcall DeleteLines(int Index, int NumLines);
	virtual void __fastcall Exchange(int Index1, int Index2);
	virtual void __fastcall Insert(int Index, const AnsiString S);
	void __fastcall InsertLines(int Index, int NumLines);
	void __fastcall InsertStrings(int Index, Classes::TStrings* NewStrings);
	virtual void __fastcall LoadFromFile(const AnsiString FileName);
	virtual void __fastcall SaveToFile(const AnsiString FileName);
	virtual void __fastcall SaveToStream(Classes::TStream* Stream);
	virtual void __fastcall LoadFromStream(Classes::TStream* Stream);
	void __fastcall DoWrapLines(const int FromLine, const int ToLine);
	void __fastcall DoUnWrapLines(const int FromLine, const int ToLine);
	int __fastcall ReWrapLine(const int LineNumber);
	bool __fastcall IsLineWraped(const int LineNumber);
	void __fastcall DoWordWrap(void);
	void __fastcall DoWordUnWrap(void);
	__property bool AppendNewLineAtEOF = {read=fAppendNewLineAtEOF, write=fAppendNewLineAtEOF, nodefault};
	__property bool WordWrap = {read=fWordWrap, write=SetWordWrap, nodefault};
	__property int WordWrapWidth = {read=fWordWrapWidth, write=fWordWrapWidth, nodefault};
	__property TSynEditFileFormat FileFormat = {read=fFileFormat, write=fFileFormat, nodefault};
	__property AnsiString ExpandedStrings[int Index] = {read=GetExpandedString};
	__property int LengthOfLongestLine = {read=GetLengthOfLongestLine, nodefault};
	__property void * Ranges[int Index] = {read=GetRange, write=PutRange};
	__property int TabWidth = {read=fTabWidth, write=SetTabWidth, nodefault};
	__property TStringListInsertedEvent OnAdded = {read=fOnAdded, write=fOnAdded};
	__property Classes::TNotifyEvent OnChange = {read=fOnChange, write=fOnChange};
	__property Classes::TNotifyEvent OnChanging = {read=fOnChanging, write=fOnChanging};
	__property Classes::TNotifyEvent OnCleared = {read=fOnCleared, write=fOnCleared};
	__property TStringListIndexEvent OnDeleted = {read=fOnDeleted, write=fOnDeleted};
	__property TStringListInsertedEvent OnInserted = {read=fOnInserted, write=fOnInserted};
	__property TStringListInsertedEvent OnPutted = {read=fOnPutted, write=fOnPutted};
};


class DELPHICLASS ESynEditStringList;
class PASCALIMPLEMENTATION ESynEditStringList : public Sysutils::Exception 
{
	typedef Sysutils::Exception inherited;
	
public:
	#pragma option push -w-inl
	/* Exception.Create */ inline __fastcall ESynEditStringList(const AnsiString Msg) : Sysutils::Exception(Msg) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmt */ inline __fastcall ESynEditStringList(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size) : Sysutils::Exception(Msg, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateRes */ inline __fastcall ESynEditStringList(int Ident)/* overload */ : Sysutils::Exception(Ident) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmt */ inline __fastcall ESynEditStringList(int Ident, const System::TVarRec * Args, const int Args_Size)/* overload */ : Sysutils::Exception(Ident, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateHelp */ inline __fastcall ESynEditStringList(const AnsiString Msg, int AHelpContext) : Sysutils::Exception(Msg, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmtHelp */ inline __fastcall ESynEditStringList(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size, int AHelpContext) : Sysutils::Exception(Msg, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResHelp */ inline __fastcall ESynEditStringList(int Ident, int AHelpContext)/* overload */ : Sysutils::Exception(Ident, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmtHelp */ inline __fastcall ESynEditStringList(System::PResStringRec ResStringRec, const System::TVarRec * Args, const int Args_Size, int AHelpContext)/* overload */ : Sysutils::Exception(ResStringRec, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~ESynEditStringList(void) { }
	#pragma option pop
	
};


#pragma option push -b-
enum TSynChangeReason { crInsert, crPaste, crDragDropInsert, crDeleteAfterCursor, crDelete, crLineBreak, crIndent, crUnindent, crSilentDelete, crSilentDeleteAfterCursor, crAutoCompleteBegin, crAutoCompleteEnd, crSpecial1Begin, crSpecial1End, crSpecial2Begin, crSpecial2End, crNothing, crGroupBreak, crDeleteAll, crWrap, crUnWrap, crWhiteSpaceAdd };
#pragma option pop

class DELPHICLASS TSynEditUndoItem;
class PASCALIMPLEMENTATION TSynEditUndoItem : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
protected:
	TSynChangeReason fChangeReason;
	Synedittypes::TSynSelectionMode fChangeSelMode;
	#pragma pack(push, 1)
	Types::TPoint fChangeStartPos;
	#pragma pack(pop)
	
	#pragma pack(push, 1)
	Types::TPoint fChangeEndPos;
	#pragma pack(pop)
	
	AnsiString fChangeStr;
	int fChangeNumber;
	
public:
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	__property TSynChangeReason ChangeReason = {read=fChangeReason, nodefault};
	__property Synedittypes::TSynSelectionMode ChangeSelMode = {read=fChangeSelMode, nodefault};
	__property Types::TPoint ChangeStartPos = {read=fChangeStartPos};
	__property Types::TPoint ChangeEndPos = {read=fChangeEndPos};
	__property AnsiString ChangeStr = {read=fChangeStr};
	__property int ChangeNumber = {read=fChangeNumber, nodefault};
public:
	#pragma option push -w-inl
	/* TPersistent.Destroy */ inline __fastcall virtual ~TSynEditUndoItem(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Create */ inline __fastcall TSynEditUndoItem(void) : Classes::TPersistent() { }
	#pragma option pop
	
};


class DELPHICLASS TSynEditUndoList;
class PASCALIMPLEMENTATION TSynEditUndoList : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	int fBlockChangeNumber;
	int fBlockCount;
	bool fFullUndoImposible;
	Classes::TList* fItems;
	int fLockCount;
	int fMaxUndoActions;
	int fNextChangeNumber;
	Classes::TNotifyEvent fOnAddedUndo;
	void __fastcall EnsureMaxEntries(void);
	bool __fastcall GetCanUndo(void);
	int __fastcall GetItemCount(void);
	void __fastcall SetMaxUndoActions(int Value);
	
public:
	__fastcall TSynEditUndoList(void);
	__fastcall virtual ~TSynEditUndoList(void);
	void __fastcall AddChange(TSynChangeReason AReason, const Types::TPoint &AStart, const Types::TPoint &AEnd, AnsiString ChangeText, Synedittypes::TSynSelectionMode SelMode);
	void __fastcall BeginBlock(void);
	void __fastcall Clear(void);
	void __fastcall EndBlock(void);
	void __fastcall Lock(void);
	TSynEditUndoItem* __fastcall PeekItem(void);
	TSynEditUndoItem* __fastcall PopItem(void);
	void __fastcall PushItem(TSynEditUndoItem* Item);
	void __fastcall Unlock(void);
	TSynChangeReason __fastcall GetChangeReason(void);
	virtual void __fastcall Assign(Classes::TPersistent* Source);
	void __fastcall AddGroupBreak(void);
	__property int BlockChangeNumber = {read=fBlockChangeNumber, write=fBlockChangeNumber, nodefault};
	__property bool CanUndo = {read=GetCanUndo, nodefault};
	__property bool FullUndoImpossible = {read=fFullUndoImposible, nodefault};
	__property int ItemCount = {read=GetItemCount, nodefault};
	__property int MaxUndoActions = {read=fMaxUndoActions, write=SetMaxUndoActions, nodefault};
	__property Classes::TNotifyEvent OnAddedUndo = {read=fOnAddedUndo, write=fOnAddedUndo};
};


//-- var, const, procedure ---------------------------------------------------
static const int SynEditStringRecSize = 0x18;
static const int MaxSynEditStrings = 0x5555555;
#define NullRange (void *)(0xffffffff)

}	/* namespace Synedittextbuffer */
using namespace Synedittextbuffer;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditTextBuffer
