// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynMacroRecorder.pas' rev: 6.00

#ifndef SynMacroRecorderHPP
#define SynMacroRecorderHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditPlugins.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <SynEditKeyCmds.hpp>	// Pascal unit
#include <SynEdit.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synmacrorecorder
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TSynMacroState { msStopped, msRecording, msPlaying, msPaused };
#pragma option pop

#pragma option push -b-
enum TSynMacroCommand { mcRecord, mcPlayback };
#pragma option pop

class DELPHICLASS TSynMacroEvent;
class PASCALIMPLEMENTATION TSynMacroEvent : public System::TObject 
{
	typedef System::TObject inherited;
	
protected:
	Byte fRepeatCount;
	virtual AnsiString __fastcall GetAsString(void) = 0 ;
	virtual void __fastcall InitEventParameters(AnsiString aStr) = 0 ;
	
public:
	__fastcall virtual TSynMacroEvent(void);
	virtual void __fastcall Initialize(Syneditkeycmds::TSynEditorCommand aCmd, char aChar, void * aData) = 0 ;
	virtual void __fastcall LoadFromStream(Classes::TStream* aStream) = 0 ;
	virtual void __fastcall SaveToStream(Classes::TStream* aStream) = 0 ;
	virtual void __fastcall Playback(Synedit::TCustomSynEdit* aEditor) = 0 ;
	__property AnsiString AsString = {read=GetAsString};
	__property Byte RepeatCount = {read=fRepeatCount, write=fRepeatCount, nodefault};
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TSynMacroEvent(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynBasicEvent;
class PASCALIMPLEMENTATION TSynBasicEvent : public TSynMacroEvent 
{
	typedef TSynMacroEvent inherited;
	
protected:
	Syneditkeycmds::TSynEditorCommand fCommand;
	virtual AnsiString __fastcall GetAsString();
	virtual void __fastcall InitEventParameters(AnsiString aStr);
	
public:
	virtual void __fastcall Initialize(Syneditkeycmds::TSynEditorCommand aCmd, char aChar, void * aData);
	virtual void __fastcall LoadFromStream(Classes::TStream* aStream);
	virtual void __fastcall SaveToStream(Classes::TStream* aStream);
	virtual void __fastcall Playback(Synedit::TCustomSynEdit* aEditor);
	__property Syneditkeycmds::TSynEditorCommand Command = {read=fCommand, write=fCommand, nodefault};
public:
	#pragma option push -w-inl
	/* TSynMacroEvent.Create */ inline __fastcall virtual TSynBasicEvent(void) : TSynMacroEvent() { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TSynBasicEvent(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynCharEvent;
class PASCALIMPLEMENTATION TSynCharEvent : public TSynMacroEvent 
{
	typedef TSynMacroEvent inherited;
	
protected:
	char fKey;
	virtual AnsiString __fastcall GetAsString();
	virtual void __fastcall InitEventParameters(AnsiString aStr);
	
public:
	virtual void __fastcall Initialize(Syneditkeycmds::TSynEditorCommand aCmd, char aChar, void * aData);
	virtual void __fastcall LoadFromStream(Classes::TStream* aStream);
	virtual void __fastcall SaveToStream(Classes::TStream* aStream);
	virtual void __fastcall Playback(Synedit::TCustomSynEdit* aEditor);
	__property char Key = {read=fKey, write=fKey, nodefault};
public:
	#pragma option push -w-inl
	/* TSynMacroEvent.Create */ inline __fastcall virtual TSynCharEvent(void) : TSynMacroEvent() { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TSynCharEvent(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynStringEvent;
class PASCALIMPLEMENTATION TSynStringEvent : public TSynMacroEvent 
{
	typedef TSynMacroEvent inherited;
	
protected:
	AnsiString fString;
	virtual AnsiString __fastcall GetAsString();
	virtual void __fastcall InitEventParameters(AnsiString aStr);
	
public:
	virtual void __fastcall Initialize(Syneditkeycmds::TSynEditorCommand aCmd, char aChar, void * aData);
	virtual void __fastcall LoadFromStream(Classes::TStream* aStream);
	virtual void __fastcall SaveToStream(Classes::TStream* aStream);
	virtual void __fastcall Playback(Synedit::TCustomSynEdit* aEditor);
	__property AnsiString Value = {read=fString, write=fString};
public:
	#pragma option push -w-inl
	/* TSynMacroEvent.Create */ inline __fastcall virtual TSynStringEvent(void) : TSynMacroEvent() { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TSynStringEvent(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynPositionEvent;
class PASCALIMPLEMENTATION TSynPositionEvent : public TSynBasicEvent 
{
	typedef TSynBasicEvent inherited;
	
protected:
	#pragma pack(push, 1)
	Types::TPoint fPosition;
	#pragma pack(pop)
	
	virtual AnsiString __fastcall GetAsString();
	virtual void __fastcall InitEventParameters(AnsiString aStr);
	
public:
	virtual void __fastcall Initialize(Syneditkeycmds::TSynEditorCommand aCmd, char aChar, void * aData);
	virtual void __fastcall LoadFromStream(Classes::TStream* aStream);
	virtual void __fastcall SaveToStream(Classes::TStream* aStream);
	virtual void __fastcall Playback(Synedit::TCustomSynEdit* aEditor);
	__property Types::TPoint Position = {read=fPosition, write=fPosition};
public:
	#pragma option push -w-inl
	/* TSynMacroEvent.Create */ inline __fastcall virtual TSynPositionEvent(void) : TSynBasicEvent() { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TSynPositionEvent(void) { }
	#pragma option pop
	
};


class DELPHICLASS TSynDataEvent;
class PASCALIMPLEMENTATION TSynDataEvent : public TSynBasicEvent 
{
	typedef TSynBasicEvent inherited;
	
protected:
	void *fData;
	
public:
	virtual void __fastcall Initialize(Syneditkeycmds::TSynEditorCommand aCmd, char aChar, void * aData);
	virtual void __fastcall LoadFromStream(Classes::TStream* aStream);
	virtual void __fastcall SaveToStream(Classes::TStream* aStream);
	virtual void __fastcall Playback(Synedit::TCustomSynEdit* aEditor);
public:
	#pragma option push -w-inl
	/* TSynMacroEvent.Create */ inline __fastcall virtual TSynDataEvent(void) : TSynBasicEvent() { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TSynDataEvent(void) { }
	#pragma option pop
	
};


class DELPHICLASS TCustomSynMacroRecorder;
typedef void __fastcall (__closure *TSynUserCommandEvent)(TCustomSynMacroRecorder* aSender, Syneditkeycmds::TSynEditorCommand aCmd, TSynMacroEvent* &aEvent);

class PASCALIMPLEMENTATION TCustomSynMacroRecorder : public Syneditplugins::TAbstractSynHookerPlugin 
{
	typedef Syneditplugins::TAbstractSynHookerPlugin inherited;
	
private:
	Classes::TShortCut fShortCuts[2];
	Classes::TNotifyEvent fOnStateChange;
	TSynUserCommandEvent fOnUserCommand;
	AnsiString fMacroName;
	TSynMacroEvent* __fastcall GetEvent(int aIndex);
	int __fastcall GetEventCount(void);
	AnsiString __fastcall GetAsString();
	void __fastcall SetAsString(const AnsiString Value);
	
protected:
	Synedit::TCustomSynEdit* fCurrentEditor;
	TSynMacroState fState;
	Classes::TList* fEvents;
	Syneditkeycmds::TSynEditorCommand fCommandIDs[2];
	void __fastcall SetShortCut(const int Index, const Classes::TShortCut Value);
	bool __fastcall GetIsEmpty(void);
	void __fastcall StateChanged(void);
	virtual void __fastcall DoAddEditor(Synedit::TCustomSynEdit* aEditor);
	virtual void __fastcall DoRemoveEditor(Synedit::TCustomSynEdit* aEditor);
	virtual void __fastcall OnCommand(System::TObject* Sender, bool AfterProcessing, bool &Handled, Syneditkeycmds::TSynEditorCommand &Command, char &aChar, void * Data, void * HandlerData);
	TSynMacroEvent* __fastcall CreateMacroEvent(Syneditkeycmds::TSynEditorCommand aCmd);
	__property Syneditkeycmds::TSynEditorCommand RecordCommandID = {read=fCommandIDs[44], nodefault};
	__property Syneditkeycmds::TSynEditorCommand PlaybackCommandID = {read=fCommandIDs[45], nodefault};
	
public:
	__fastcall virtual TCustomSynMacroRecorder(Classes::TComponent* aOwner);
	__fastcall virtual ~TCustomSynMacroRecorder(void);
	void __fastcall Error(const AnsiString aMsg);
	HIDESBASE void __fastcall AddEditor(Synedit::TCustomSynEdit* aEditor);
	HIDESBASE void __fastcall RemoveEditor(Synedit::TCustomSynEdit* aEditor);
	void __fastcall RecordMacro(Synedit::TCustomSynEdit* aEditor);
	void __fastcall PlaybackMacro(Synedit::TCustomSynEdit* aEditor);
	void __fastcall Stop(void);
	void __fastcall Pause(void);
	void __fastcall Resume(void);
	__property bool IsEmpty = {read=GetIsEmpty, nodefault};
	__property TSynMacroState State = {read=fState, nodefault};
	void __fastcall Clear(void);
	void __fastcall AddEvent(Syneditkeycmds::TSynEditorCommand aCmd, char aChar, void * aData);
	void __fastcall InsertEvent(int aIndex, Syneditkeycmds::TSynEditorCommand aCmd, char aChar, void * aData);
	void __fastcall AddCustomEvent(TSynMacroEvent* aEvent);
	void __fastcall InsertCustomEvent(int aIndex, TSynMacroEvent* aEvent);
	void __fastcall DeleteEvent(int aIndex);
	void __fastcall LoadFromStream(Classes::TStream* aSrc);
	void __fastcall LoadFromStreamEx(Classes::TStream* aSrc, bool aClear);
	void __fastcall SaveToStream(Classes::TStream* aDest);
	void __fastcall LoadFromFile(AnsiString aFilename);
	void __fastcall SaveToFile(AnsiString aFilename);
	__property int EventCount = {read=GetEventCount, nodefault};
	__property TSynMacroEvent* Events[int aIndex] = {read=GetEvent};
	__property Classes::TShortCut RecordShortCut = {read=fShortCuts[26], write=SetShortCut, index=0, nodefault};
	__property Classes::TShortCut PlaybackShortCut = {read=fShortCuts[27], write=SetShortCut, index=1, nodefault};
	__property AnsiString AsString = {read=GetAsString, write=SetAsString};
	__property AnsiString MacroName = {read=fMacroName, write=fMacroName};
	__property Classes::TNotifyEvent OnStateChange = {read=fOnStateChange, write=fOnStateChange};
	__property TSynUserCommandEvent OnUserCommand = {read=fOnUserCommand, write=fOnUserCommand};
};


class DELPHICLASS TSynMacroRecorder;
class PASCALIMPLEMENTATION TSynMacroRecorder : public TCustomSynMacroRecorder 
{
	typedef TCustomSynMacroRecorder inherited;
	
__published:
	__property RecordShortCut ;
	__property PlaybackShortCut ;
	__property OnStateChange ;
	__property OnUserCommand ;
public:
	#pragma option push -w-inl
	/* TCustomSynMacroRecorder.Create */ inline __fastcall virtual TSynMacroRecorder(Classes::TComponent* aOwner) : TCustomSynMacroRecorder(aOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TCustomSynMacroRecorder.Destroy */ inline __fastcall virtual ~TSynMacroRecorder(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE System::ResourceString _sCannotRecord;
#define Synmacrorecorder_sCannotRecord System::LoadResourceString(&Synmacrorecorder::_sCannotRecord)
extern PACKAGE System::ResourceString _sCannotPlay;
#define Synmacrorecorder_sCannotPlay System::LoadResourceString(&Synmacrorecorder::_sCannotPlay)
extern PACKAGE System::ResourceString _sCannotPause;
#define Synmacrorecorder_sCannotPause System::LoadResourceString(&Synmacrorecorder::_sCannotPause)
extern PACKAGE System::ResourceString _sCannotResume;
#define Synmacrorecorder_sCannotResume System::LoadResourceString(&Synmacrorecorder::_sCannotResume)

}	/* namespace Synmacrorecorder */
using namespace Synmacrorecorder;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynMacroRecorder
