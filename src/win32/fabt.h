//---------------------------------------------------------------------------

#ifndef fabtH
#define fabtH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TFAbout : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TShape *Shape1;
	TLabel *Label2;
	TPanel *Panel1;
	TImage *Image1;
	TLabel *Version;
	TLabel *Label3;
	TLabel *Label4;
	TButton *Button1;
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall OnCreate(TObject *Sender);
	void __fastcall Label4Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFAbout *FAbout;
//---------------------------------------------------------------------------
#endif
