//---------------------------------------------------------------------------

#ifndef conv_viewH
#define conv_viewH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmView : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TPanel *Panel2;
	TRichEdit *txtText;
	TButton *btnClose;
	TButton *btnSave;
	void __fastcall btnSaveClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TfrmView(TComponent* Owner);
	void txt_view(const char * file);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmView *frmView;
//---------------------------------------------------------------------------
#endif
