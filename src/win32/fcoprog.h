//---------------------------------------------------------------------------

#ifndef fcoprogH
#define fcoprogH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TFCompProg : public TForm
{
__published:	// IDE-managed Components
	TLabel *labP1;
	TLabel *labP2;
private:	// User declarations
public:		// User declarations
	__fastcall TFCompProg(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFCompProg *FCompProg;
//---------------------------------------------------------------------------
#endif
