//---------------------------------------------------------------------------

#ifndef gmainH
#define gmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ImgList.hpp>
#include <ToolWin.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TFMain : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlMain;
	TMainMenu *MainMenu1;
	TMenuItem *mnuFile;
	TMenuItem *mnuOpen;
	TMenuItem *mnuSave;
	TMenuItem *N1;
	TMenuItem *mnuExit;
	TPanel *pnlMainWA;
	TPanel *pnlWLeft;
	TPanel *pnlWRight;
	TSplitter *Splitter1;
	TControlBar *ControlBar1;
	TPageControl *pgMain;
	TTabSheet *pgEdit;
	TMenuItem *mnuSaveAS;
	TMenuItem *N2;
	TMenuItem *mnuRun;
	TMenuItem *mnuBreak;
	TImageList *ImageList1;
	TToolBar *ToolBar1;
	TToolButton *tbOpen;
	TToolButton *tbSave;
	TToolButton *ToolButton1;
	TToolButton *ToolButton2;
	TToolButton *ToolButton3;
	TMemo *txtERR;
	TPanel *pnlOut;
	TImage *imgOutput;
	TOpenDialog *dlgOpen;
	TSaveDialog *dlgSave;
	TMenuItem *Help1;
	TMenuItem *mnuIndex;
	TMenuItem *N3;
	TMenuItem *mnuAbout;
	TBevel *Bevel1;
	TRichEdit *txtCode;
	TStatusBar *StatusBar1;
	TMenuItem *Edit1;
	TMenuItem *mnuGoTo;
	void __fastcall mnuExitClick(TObject *Sender);
	void __fastcall OnShow(TObject *Sender);
	void __fastcall mnuRunClick(TObject *Sender);
	void __fastcall mnuBreakClick(TObject *Sender);
	void __fastcall mnuSaveClick(TObject *Sender);
	void __fastcall mnuOpenClick(TObject *Sender);
	void __fastcall mnuSaveASClick(TObject *Sender);
	void __fastcall OnClose(TObject *Sender, TCloseAction &Action);
	void __fastcall txtCodeOnChange(TObject *Sender);
	void __fastcall txtCodeOnKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall mnuGoToClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFMain(TComponent* Owner);
	void UpdatePos();
};
//---------------------------------------------------------------------------
extern PACKAGE TFMain *FMain;
//---------------------------------------------------------------------------
#endif
