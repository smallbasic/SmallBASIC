//---------------------------------------------------------------------------

#ifndef conv_mainH
#define conv_mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TFCSMain : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TLabel *Label1;
	TLabel *Label2;
	TEdit *txtSRC;
	TEdit *txtTRG;
	TButton *btnBSRC;
	TButton *btnBTRG;
	TButton *btnConvert;
	TPanel *Panel2;
	TOpenDialog *dlgOpen;
	TImage *Image1;
	TSaveDialog *dlgSave;
	TRichEdit *txtTMP;
	TSplitter *Splitter1;
	TPanel *Panel3;
	TMemo *txtSrcView;
	TPanel *Panel4;
	TPanel *Panel5;
	TPanel *Panel6;
	TMemo *txtConsole;
	void __fastcall btnBSRCClick(TObject *Sender);
	void __fastcall btnBTRGClick(TObject *Sender);
	void __fastcall btnConvertClick(TObject *Sender);
	void __fastcall OnChangeSRC(TObject *Sender);
	void __fastcall btnViewClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFCSMain(TComponent* Owner);
	void txt_view(const char * file);
};
//---------------------------------------------------------------------------
extern PACKAGE TFCSMain *FCSMain;
//---------------------------------------------------------------------------
#endif
