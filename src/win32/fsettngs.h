//---------------------------------------------------------------------------

#ifndef fsettngsH
#define fsettngsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TFSets : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TEdit *txtEF;
	TLabel *Label2;
	TBevel *Bevel1;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *txtXF;
	TButton *Button1;
	TButton *Button2;
	TCheckBox *chkVS;
	TLabel *Label5;
	TEdit *txtVW;
	TLabel *Label6;
	TEdit *txtVH;
	TButton *btnEF;
	TButton *btnXF;
	TPanel *Panel1;
	TImage *Image1;
	TFontDialog *dlgFont;
	TCheckBox *chkSound;
	TBevel *Bevel2;
	TCheckBox *chkMMSound;
	void __fastcall btnEFClick(TObject *Sender);
	void __fastcall btnXFClick(TObject *Sender);
	void __fastcall chkVSClick(TObject *Sender);
	void __fastcall chkSoundClick(TObject *Sender);
	void __fastcall OnShow(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFSets(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFSets *FSets;
//---------------------------------------------------------------------------
#endif
