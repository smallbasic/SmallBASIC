/*
*	UI utils for PalmOS
*
*	Nicholas Christopoulos
*/

#if !defined(_ui_h)
#define _ui_h

//#include <Pilot.h>
#include "sys.h"

FieldPtr	fld_ptr(word id) SEC(TRASH);
void		setFieldHandle(word id, VoidHand h_text, int redraw)  SEC(TRASH);
void		fld_setText(word id, const char *text, int redraw) SEC(TRASH);
char*		fld_getTextPtr(word id) SEC(TRASH);
Handle		fld_getHandle(word ID) SEC(TRASH);
void		fld_enable(word id) SEC(TRASH);
void		fld_disable(word id) SEC(TRASH);
ListPtr 	GetListPtr(Word ID) SEC(TRASH);
FieldPtr 	GetFieldPtr(Word ID) SEC(TRASH);
int			GetListSel(Word ID) SEC(TRASH);
void		SetListSel(Word ID, int sel) SEC(TRASH);
void		SetPBVal(Word ID, int sel) SEC(TRASH);
void 		UpdatePopup(int trgID, int lstID, UInt selection, char **options) SEC(TRASH);

/*
* Code taken from GutenPalm Project:
*
* GutenPalm: A zTXT format document reader for the Palm Handheld Organizer
* John Gruenenfelder - johng@as.arizona.edu
* http://gutenpalm.sourceforge.net
*/
void		DoAboutForm(void)	SEC(TRASH);
void		ScrollLines(UInt16 fieldID, UInt16 sBarID, Int8 linesToScroll, Boolean redraw) SEC(TRASH);
void		UpdateScrollbar(UInt16 fieldID, UInt16 sBarID) SEC(TRASH);
void		GPlmAlert(DmResID title, DmResID message) SEC(TRASH);
void    	PDOCAlert(const char *filename)             SEC(TRASH);

#endif

