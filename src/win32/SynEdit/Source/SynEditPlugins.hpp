// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynEditPlugins.pas' rev: 6.00

#ifndef SynEditPluginsHPP
#define SynEditPluginsHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynEditKeyCmds.hpp>	// Pascal unit
#include <SynEdit.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Syneditplugins
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TAbstractSynPlugin;
class PASCALIMPLEMENTATION TAbstractSynPlugin : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	void __fastcall SetEditor(const Synedit::TCustomSynEdit* Value);
	Synedit::TCustomSynEdit* __fastcall GetEditors(int aIndex);
	Synedit::TCustomSynEdit* __fastcall GetEditor(void);
	int __fastcall GetEditorCount(void);
	
protected:
	Classes::TList* fEditors;
	virtual void __fastcall Notification(Classes::TComponent* aComponent, Classes::TOperation aOperation);
	virtual void __fastcall DoAddEditor(Synedit::TCustomSynEdit* aEditor);
	virtual void __fastcall DoRemoveEditor(Synedit::TCustomSynEdit* aEditor);
	int __fastcall AddEditor(Synedit::TCustomSynEdit* aEditor);
	int __fastcall RemoveEditor(Synedit::TCustomSynEdit* aEditor);
	
public:
	__fastcall virtual ~TAbstractSynPlugin(void);
	__property Synedit::TCustomSynEdit* Editors[int aIndex] = {read=GetEditors};
	__property int EditorCount = {read=GetEditorCount, nodefault};
	
__published:
	__property Synedit::TCustomSynEdit* Editor = {read=GetEditor, write=SetEditor};
public:
	#pragma option push -w-inl
	/* TComponent.Create */ inline __fastcall virtual TAbstractSynPlugin(Classes::TComponent* AOwner) : Classes::TComponent(AOwner) { }
	#pragma option pop
	
};


class DELPHICLASS TAbstractSynHookerPlugin;
class PASCALIMPLEMENTATION TAbstractSynHookerPlugin : public TAbstractSynPlugin 
{
	typedef TAbstractSynPlugin inherited;
	
protected:
	void __fastcall HookEditor(Synedit::TCustomSynEdit* aEditor, Syneditkeycmds::TSynEditorCommand aCommandID, Classes::TShortCut aOldShortCut, Classes::TShortCut aNewShortCut);
	void __fastcall UnHookEditor(Synedit::TCustomSynEdit* aEditor, Syneditkeycmds::TSynEditorCommand aCommandID, Classes::TShortCut aShortCut);
	virtual void __fastcall OnCommand(System::TObject* Sender, bool AfterProcessing, bool &Handled, Syneditkeycmds::TSynEditorCommand &Command, char &aChar, void * Data, void * HandlerData) = 0 ;
public:
	#pragma option push -w-inl
	/* TAbstractSynPlugin.Destroy */ inline __fastcall virtual ~TAbstractSynHookerPlugin(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TComponent.Create */ inline __fastcall virtual TAbstractSynHookerPlugin(Classes::TComponent* AOwner) : TAbstractSynPlugin(AOwner) { }
	#pragma option pop
	
};


#pragma option push -b-
enum TPluginState { psNone, psExecuting, psAccepting, psCancelling };
#pragma option pop

class DELPHICLASS TAbstractSynSingleHookPlugin;
class PASCALIMPLEMENTATION TAbstractSynSingleHookPlugin : public TAbstractSynHookerPlugin 
{
	typedef TAbstractSynHookerPlugin inherited;
	
private:
	Syneditkeycmds::TSynEditorCommand fCommandID;
	bool __fastcall IsShortCutStored(void);
	void __fastcall SetShortCut(const Classes::TShortCut Value);
	
protected:
	TPluginState fState;
	Synedit::TCustomSynEdit* fCurrentEditor;
	Classes::TShortCut fShortCut;
	/* virtual class method */ virtual Classes::TShortCut __fastcall DefaultShortCut(TMetaClass* vmt);
	virtual void __fastcall DoAddEditor(Synedit::TCustomSynEdit* aEditor);
	virtual void __fastcall DoRemoveEditor(Synedit::TCustomSynEdit* aEditor);
	virtual void __fastcall DoExecute(void) = 0 ;
	virtual void __fastcall DoAccept(void) = 0 ;
	virtual void __fastcall DoCancel(void) = 0 ;
	
public:
	__fastcall virtual TAbstractSynSingleHookPlugin(Classes::TComponent* aOwner);
	__fastcall virtual ~TAbstractSynSingleHookPlugin(void);
	__property Syneditkeycmds::TSynEditorCommand CommandID = {read=fCommandID, nodefault};
	__property Synedit::TCustomSynEdit* CurrentEditor = {read=fCurrentEditor};
	bool __fastcall Executing(void);
	void __fastcall Execute(Synedit::TCustomSynEdit* aEditor);
	void __fastcall Accept(void);
	void __fastcall Cancel(void);
	
__published:
	__property Classes::TShortCut ShortCut = {read=fShortCut, write=SetShortCut, stored=IsShortCutStored, nodefault};
};


class DELPHICLASS TAbstractSynCompletion;
class PASCALIMPLEMENTATION TAbstractSynCompletion : public TAbstractSynSingleHookPlugin 
{
	typedef TAbstractSynSingleHookPlugin inherited;
	
protected:
	AnsiString fCurrentString;
	virtual void __fastcall SetCurrentString(const AnsiString Value);
	virtual void __fastcall OnCommand(System::TObject* Sender, bool AfterProcessing, bool &Handled, Syneditkeycmds::TSynEditorCommand &Command, char &aChar, void * Data, void * HandlerData);
	virtual void __fastcall DoExecute(void);
	virtual void __fastcall DoAccept(void);
	virtual void __fastcall DoCancel(void);
	virtual AnsiString __fastcall GetCurrentEditorString();
	
public:
	HIDESBASE void __fastcall AddEditor(Synedit::TCustomSynEdit* aEditor);
	__property AnsiString CurrentString = {read=fCurrentString, write=SetCurrentString};
public:
	#pragma option push -w-inl
	/* TAbstractSynSingleHookPlugin.Create */ inline __fastcall virtual TAbstractSynCompletion(Classes::TComponent* aOwner) : TAbstractSynSingleHookPlugin(aOwner) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TAbstractSynSingleHookPlugin.Destroy */ inline __fastcall virtual ~TAbstractSynCompletion(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE Syneditkeycmds::TSynEditorCommand __fastcall NewPluginCommand(void);
extern PACKAGE void __fastcall ReleasePluginCommand(Syneditkeycmds::TSynEditorCommand aCmd);

}	/* namespace Syneditplugins */
using namespace Syneditplugins;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynEditPlugins
