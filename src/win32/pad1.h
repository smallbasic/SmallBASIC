//---------------------------------------------------------------------------

#ifndef pad1H
#define pad1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ImgList.hpp>
#include <ToolWin.hpp>
#include "SynEdit.hpp"
#include "SynEditHighlighter.hpp"
#include "SynHighlighterGeneral.hpp"
#include "SynHighlighterMulti.hpp"
#include <OleCtrls.hpp>
#include "SHDocVw_OCX.h"
//---------------------------------------------------------------------------
class TFMain : public TForm
{
__published:	// IDE-managed Components
    TPanel *Panel1;
    TMainMenu *MainMenu1;
    TMenuItem *File1;
    TMenuItem *Load1;
    TMenuItem *Save1;
    TMenuItem *N1;
    TMenuItem *Quit1;
    TMenuItem *Program1;
    TMenuItem *Restart1;
    TMenuItem *Break1;
	TOpenDialog *dlgOpen;
	TTimer *Timer1;
	TMenuItem *Edit1;
	TMenuItem *Find1;
	TMenuItem *Finx1;
	TToolBar *ToolBar1;
	TImageList *ImageList1;
	TToolButton *ToolButton1;
	TToolButton *ToolButton2;
	TToolButton *ToolButton3;
	TToolButton *ToolButton4;
	TToolButton *ToolButton5;
	TToolButton *ToolButton6;
	TToolButton *ToolButton7;
	TToolButton *ToolButton8;
	TImageList *ImageList2;
	TMenuItem *Saveas1;
	TMenuItem *About1;
	TSaveDialog *dlgSave;
	TMenuItem *N3;
	TMenuItem *Settings1;
	TMenuItem *New1;
	TToolButton *ToolButton9;
	TToolButton *ToolButton10;
	TToolButton *ToolButton11;
	TSynGeneralSyn *gsyn;
	TPageControl *pgMain;
	TTabSheet *pgEdit;
	TSynEdit *editor;
	TTabSheet *pgHelp;
	TTabSheet *pgExec;
	TPanel *Panel11;
	TScrollBox *sclOut;
	TPaintBox *imgOut;
	TMenuItem *mnuReopen;
	TMenuItem *mnuReopen1;
	TMenuItem *mnuReopen2;
	TMenuItem *mnuReopen3;
	TMenuItem *mnuReopen4;
	TMenuItem *mnuReopen5;
	TMenuItem *mnuReopen6;
	TMenuItem *mnuReopen7;
	TMenuItem *mnuReopen8;
	TPanel *Panel2;
	TCppWebBrowser *browser;
	TTabSheet *pgTextOut;
	TPanel *Panel3;
	TButton *btnLogClear;
	TCheckBox *chkCapture;
	TPanel *Panel4;
	TMemo *txtLog;
	TStatusBar *StatBar;
	void __fastcall Quit1Click(TObject *Sender);
	void __fastcall Load1Click(TObject *Sender);
	void __fastcall srcOnKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall srcOnMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall Save1Click(TObject *Sender);
	void __fastcall Restart1Click(TObject *Sender);
	void __fastcall Break1Click(TObject *Sender);
	void __fastcall outOnMouseMove(TObject *Sender, TShiftState Shift, int X,
		  int Y);
	void __fastcall outOnKeyPress(TObject *Sender, char &Key);
	void __fastcall OnClose(TObject *Sender, TCloseAction &Action);
	void __fastcall outOnMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall About1Click(TObject *Sender);
	void __fastcall Saveas1Click(TObject *Sender);
	void __fastcall Settings1Click(TObject *Sender);
	void __fastcall New1Click(TObject *Sender);
	void __fastcall OnShow(TObject *Sender);
	void __fastcall outOnPaint(TObject *Sender);
	void __fastcall pgMainChange(TObject *Sender);
	void __fastcall mnuReopenX(TObject *Sender);
	void __fastcall Finx1Click(TObject *Sender);
	void __fastcall Find1Click(TObject *Sender);
	void __fastcall btnLogClearClick(TObject *Sender);
	void __fastcall chkCaptureClick(TObject *Sender);
	void __fastcall outOnKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall outOnKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
private:	// User declarations
	char	dir_sb[1024];
	char	username[512];
	TStringList	*lastUsedFiles;

public:		// User declarations
	__fastcall TFMain(TComponent* Owner);
	void update_sbar();
	void load_source();
	void save_source();
	void copy_file(const char * src, const char * dst);
	void help_search(const char * key);
	void rebuildLastUsedFiles();
	void updateLastUsedFiles(const char * name);
	void __fastcall textSearchAndReplace(bool replace, bool back);
	void lwrite(const char * s);
	void CompProg(int pass, int pmin, int pmax);
};
//---------------------------------------------------------------------------
extern PACKAGE TFMain *FMain;
//---------------------------------------------------------------------------
#endif
