//---------------------------------------------------------------------------

#ifndef fsrcH
#define fsrcH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TFSearch : public TForm
{
__published:	// IDE-managed Components
	TButton *btnSearch;
	TButton *btnCancel;
	TLabel *Label1;
	TEdit *txtSearch;
	TCheckBox *chkBack;
	TCheckBox *chkCase;
	TCheckBox *chkEntire;
	TCheckBox *chkSelOnly;
	TCheckBox *chkWords;
	TLabel *Label2;
	TEdit *txtReplace;
	TButton *btnReplace;
	void __fastcall btnSearchClick(TObject *Sender);
	void __fastcall btnReplaceClick(TObject *Sender);
	void __fastcall btnCancelClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	int		action;
	__fastcall TFSearch(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFSearch *FSearch;
//---------------------------------------------------------------------------
#endif
