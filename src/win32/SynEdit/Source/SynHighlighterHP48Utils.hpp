// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynHighlighterHP48Utils.pas' rev: 6.00

#ifndef SynHighlighterHP48UtilsHPP
#define SynHighlighterHP48UtilsHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synhighlighterhp48utils
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TSpeedListObject;
class DELPHICLASS TSpeedStringList;
class PASCALIMPLEMENTATION TSpeedListObject : public System::TObject 
{
	typedef System::TObject inherited;
	
protected:
	AnsiString FName;
	TSpeedStringList* FSpeedList;
	System::TObject* fobject;
	virtual void __fastcall SetName(const AnsiString Value);
	
public:
	__property AnsiString Name = {read=FName, write=SetName};
	__fastcall TSpeedListObject(AnsiString name);
	__fastcall virtual ~TSpeedListObject(void);
	__property TSpeedStringList* SpeedList = {read=FSpeedList, write=FSpeedList};
	__property System::TObject* pointer = {read=fobject, write=fobject};
};


typedef TSpeedListObject* TSpeedListObjects[1];

typedef TSpeedListObject* *PSpeedListObjects;

class PASCALIMPLEMENTATION TSpeedStringList : public System::TObject 
{
	typedef System::TObject inherited;
	
public:
	AnsiString operator[](int Index) { return Strings[Index]; }
	
private:
	AnsiString __fastcall GetText();
	void __fastcall SetText(const AnsiString Value);
	System::TObject* __fastcall GetInObject(int Index);
	void __fastcall SetInObject(int Index, const System::TObject* Value);
	
protected:
	Classes::TNotifyEvent FOnChange;
	int SumOfUsed[128];
	int datasUsed[128];
	TSpeedListObject* *datas[128];
	int lengthDatas[128];
	virtual void __fastcall Changed(void);
	virtual AnsiString __fastcall Get(int Index);
	TSpeedListObject* __fastcall GetObject(int Index);
	int __fastcall GetCount(void);
	Classes::TStrings* __fastcall GetStringList(void);
	void __fastcall SetStringList(const Classes::TStrings* value);
	
public:
	void __fastcall NameChange(const TSpeedListObject* obj, const AnsiString NewName);
	void __fastcall ObjectDeleted(const TSpeedListObject* obj);
	__fastcall virtual ~TSpeedStringList(void);
	__fastcall TSpeedStringList(void);
	int __fastcall Add(const TSpeedListObject* Value)/* overload */;
	TSpeedListObject* __fastcall Add(const AnsiString Value)/* overload */;
	void __fastcall Clear(void);
	TSpeedListObject* __fastcall Find(const AnsiString name);
	__property Classes::TNotifyEvent OnChange = {read=FOnChange, write=FOnChange};
	__property TSpeedListObject* Objects[int Index] = {read=GetObject};
	__property System::TObject* inobject[int Index] = {read=GetInObject, write=SetInObject};
	__property AnsiString Strings[int Index] = {read=Get/*, default*/};
	__property int count = {read=GetCount, nodefault};
	__property Classes::TStrings* StringList = {read=GetStringList, write=SetStringList};
	__property AnsiString text = {read=GetText, write=SetText};
};



//-- var, const, procedure ---------------------------------------------------
static const Byte NbSubList = 0x80;

}	/* namespace Synhighlighterhp48utils */
using namespace Synhighlighterhp48utils;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynHighlighterHP48Utils
