/*
*	UI utils for PalmOS
*
*	Nicholas Christopoulos
*/

#include "sys.h"
#include "ui.h"
#include "sbpad.h"
#include "fs_pdoc.h"

/*
*	get fieldptr
*/
FieldPtr	fld_ptr(word id)
{
    FormPtr		frm = FrmGetActiveForm();
	FieldPtr	ptr;

	ptr = (FieldPtr) FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, id));
	ErrNonFatalDisplayIf(!ptr, "Field not in form");
	return ptr;
}

/*
*	setting text in a field 
*/
void	setFieldHandle(word id, VoidHand h_text, int redraw) 
{
	VoidHand	h_old;
	FieldPtr	p_fld;

	p_fld = fld_ptr(id);
	ErrNonFatalDisplayIf(!p_fld, "Field not in form");
	h_old = (VoidHand) FldGetTextHandle(p_fld);

	FldSetTextHandle(p_fld, (Handle) h_text);
	if	( redraw )
		FldDrawField(p_fld);

	if ( h_old ) 
		MemHandleFree(h_old);
}

/*
*	copies the text in the handle and sets it to the field. 
*/
void	fld_setText(word id, const char *text, int redraw)
{
	VoidHand	h_new = 0;
	char		*temp;

	temp = (char *) text;
	h_new = MemHandleNew(sizeof(char) * (StrLen(temp) + 1));
	StrCopy((char *) MemHandleLock(h_new), temp);
	MemHandleUnlock(h_new);
	setFieldHandle(id, h_new, redraw);
}

char*	fld_getTextPtr(word id)
{
	return FldGetTextPtr(fld_ptr(id));
}

Handle	fld_getHandle(word ID)
{
	FieldPtr	fp;

	fp = fld_ptr(ID);
	FldCompactText(fp);
	return FldGetTextHandle(fp);
}

void	fld_enable(word id)
{
	FldSetUsable(fld_ptr(id), 1);
}

void	fld_disable(word id)
{
	FldSetUsable(fld_ptr(id), 0);
}

ListPtr GetListPtr(Word ID)
{
    FormPtr frm;
	ListPtr	list;

	frm = FrmGetActiveForm();
	list = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, ID));
	ErrNonFatalDisplayIf(!list, "List not in form");
	return list;
}

FieldPtr GetFieldPtr(Word ID)
{
    FormPtr frm;
	FieldPtr field;

	frm = FrmGetActiveForm();
	field = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, ID));
	ErrNonFatalDisplayIf(!field, "Field not in form");
	return field;
}

int GetListSel(Word ID)
{
    FormPtr frm;
	ListPtr	list;

	frm = FrmGetActiveForm();
	list = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, ID));
	ErrNonFatalDisplayIf(!list, "List not in form");
	return LstGetSelection(list);
}

void SetListSel(Word ID, int sel)
{
    FormPtr frm;
	ListPtr	list;

	frm = FrmGetActiveForm();
	list = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, ID));
	ErrNonFatalDisplayIf(!list, "List not in form");
	LstSetSelection(list, sel);
}

void SetPBVal(Word ID, int sel)
{
    FormPtr frm;
	ControlPtr	ctrl;

	frm = FrmGetActiveForm();
	ctrl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, ID));
	ErrNonFatalDisplayIf(!ctrl, "Control not in form");
	CtlSetValue(ctrl, sel);
}

void UpdatePopup(int trgID, int lstID, UInt selection, char **options)
{
	FormPtr		frm;
	ListPtr		list;
	ControlPtr	popup;

	frm = FrmGetActiveForm();
	list = (ListPtr) FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, lstID));
	ErrNonFatalDisplayIf(!list, "List not in form");
	LstMakeItemVisible(list, selection);

	LstSetSelection(list, selection);
	popup = (ControlPtr) FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, trgID));
	ErrNonFatalDisplayIf(!popup, "Popup not in form");
	CtlSetLabel(popup, options[selection]);
	CtlDrawControl(popup);
}

/*
* Code taken from GutenPalm Project:
*
* GutenPalm: A zTXT format document reader for the Palm Handheld Organizer
* John Gruenenfelder - johng@as.arizona.edu
* http://gutenpalm.sourceforge.net
*/

/* Local functions */
Boolean  HelpFormHandleEvent(EventPtr);
Boolean  AboutFormHandleEvent(EventPtr);

/*
 * Generic GutenPalm alert.  Fetches STRING resources for the title and
 * the contents.  Puts the text in a field with a scrollbar
 */
void	GPlmAlert(DmResID title, DmResID message)
{
  FormPtr       prevFrm;
  FormPtr       frm;
  FieldPtr      helpTextField;
//  MemHandle     helpTitleHandle;
  MemHandle     helpMessageHandle;
  MemHandle     oldMessageHandle;

  /* Setup form */
  prevFrm = FrmGetActiveForm();
  frm = FrmInitForm(FormHelpID);
  FrmSetActiveForm(frm);
  helpTextField = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, HelpTextField));

  /* Get the string resources for help contents */
//  helpTitleHandle = DmGetResource('tSTR', title);
//  FrmSetTitle(frm, MemHandleLock(helpTitleHandle));
  helpMessageHandle = DmGetResource('tSTR', message);
  oldMessageHandle = FldGetTextHandle(helpTextField);
  FldSetTextHandle(helpTextField, helpMessageHandle);

  /* Set the scrollbar values and the event handler */
  UpdateScrollbar(HelpTextField, HelpScrBar);
  FrmSetEventHandler(frm, HelpFormHandleEvent);
  FrmSetFocus(frm, FrmGetObjectIndex(frm, HelpTextField));

  /* Show the dialog */
  FrmDoDialog(frm);

  FldSetTextHandle(helpTextField, oldMessageHandle);
//  MemHandleUnlock(helpTitleHandle);
//  DmReleaseResource(helpTitleHandle);
  DmReleaseResource(helpMessageHandle);

  if (prevFrm)
    FrmSetActiveForm(prevFrm);
  FrmDeleteForm(frm);
}

/*
*/
void	PDOCAlert(const char *filename)
{
	FormPtr		prevFrm, frm;
	FieldPtr	helpTextField;
	MemHandle	helpMessageHandle, oldMessageHandle;
	char		*ptr;

	/* Setup form */
	prevFrm = FrmGetActiveForm();
	frm = FrmInitForm(FormHelpID);
	FrmSetActiveForm(frm);
	helpTextField = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, HelpTextField));

	helpMessageHandle = pdoc_loadtomem(filename);
	if	( !helpMessageHandle )	{
		helpMessageHandle = MemHandleNew(128);
		ptr = MemHandleLock(helpMessageHandle);
		strcpy(ptr, "File not found!"); 
		MemHandleUnlock(helpMessageHandle);
		}
	oldMessageHandle = FldGetTextHandle(helpTextField);
	FldSetTextHandle(helpTextField, helpMessageHandle);

	/* Set the scrollbar values and the event handler */
	UpdateScrollbar(HelpTextField, HelpScrBar);
	FrmSetEventHandler(frm, HelpFormHandleEvent);
	FrmSetFocus(frm, FrmGetObjectIndex(frm, HelpTextField));

	/* Show the dialog */
	FrmDoDialog(frm);

	// cleanup
	FldSetTextHandle(helpTextField, oldMessageHandle);
	MemHandleFree(helpMessageHandle);

	if (prevFrm)
		FrmSetActiveForm(prevFrm);
	FrmDeleteForm(frm);
}

Boolean HelpFormHandleEventOnTRASH(EventPtr e)	SEC(TRASH);
Boolean HelpFormHandleEventOnTRASH(EventPtr e)
{
  Boolean       handled = false;
  FormPtr       frm;
  Int8          linesToScroll;
  FieldPtr      fld;

  switch (e->eType)
    {
      case fldChangedEvent:
        UpdateScrollbar(HelpTextField, HelpScrBar);
        handled = true;
        break;

      case sclRepeatEvent:
        ScrollLines(HelpTextField, HelpScrBar, e->data.sclRepeat.newValue - e->data.sclRepeat.value, true);
        break;								 

      case keyDownEvent:
        frm = FrmGetActiveForm();
        fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, HelpTextField));
        if (e->data.keyDown.chr == pageUpChr)
          {
            if (FldScrollable(fld, winUp))
              {
                linesToScroll = -(FldGetVisibleLines(fld) - 1);
                ScrollLines(HelpTextField, HelpScrBar, linesToScroll, true);
              }
            handled = true;
          }
        else if (e->data.keyDown.chr == pageDownChr)
          {
            if (FldScrollable(fld, winDown))
              {
                linesToScroll = FldGetVisibleLines(fld) - 1;
                ScrollLines(HelpTextField, HelpScrBar, linesToScroll, true);
              }
            handled = true;
          }
        break;

      default:
        break;
    }

  return handled;
}

/* Event handler for the generic alert form */
Boolean HelpFormHandleEvent(EventPtr e)
{
	return HelpFormHandleEventOnTRASH(e);
}


/*
 * Display the about form with a pretty and colorful logo
 * If your Palm is not already in some color mode, this will attempt
 * to turn on 4bpp greyscale mode.
 */
void DoAboutForm(void)
{
  FormPtr       prevFrm;
  FormPtr       frm;
  FieldPtr      aboutField;
  MemHandle     aboutTextHandle;
  MemHandle     oldMessageHandle;
//  UInt32        oldDepth;
//  UInt32        depth = 4;

//  /* We want a pretty logo, so set the color depth up, if supported */
//  if (CheckROMVerGreaterThan(3,5))
//    {
//      WinScreenMode(winScreenModeGet, NULL, NULL, &oldDepth, NULL);
//      /* If not in 24bpp or 8bpp, switch to 4bpp greyscale */
//      if ((oldDepth != 8) && (oldDepth != 24))
//        WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
//    }
//  else
//    {
//      /* This should work for Palm OS >= 2.0 */
//      ScrDisplayMode(scrDisplayModeGet, NULL, NULL, &oldDepth, NULL);
//      /* If not in 24bpp or 8bpp, switch to 4bpp greyscale */
//      if ((oldDepth != 8) && (oldDepth != 24))
//        WinScreenMode(scrDisplayModeSet, NULL, NULL, &depth, NULL);
//    }

  /* Setup form */
  prevFrm = FrmGetActiveForm();
  frm = FrmInitForm(AboutForm2);
  FrmSetActiveForm(frm);
  aboutField = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, AboutField));

  /* Get the string resources for help contents */
  aboutTextHandle = DmGetResource('tSTR', AboutStrID);
  oldMessageHandle = FldGetTextHandle(aboutField);
  FldSetTextHandle(aboutField, aboutTextHandle);

  /* Set the scrollbar values and the event handler */
  UpdateScrollbar(AboutField, AboutScrBar);
  FrmSetEventHandler(frm, AboutFormHandleEvent);
  FrmSetFocus(frm, FrmGetObjectIndex(frm, AboutField));

  /* Show the dialog */
  FrmDoDialog(frm);

  FldSetTextHandle(aboutField, oldMessageHandle);
  DmReleaseResource(aboutTextHandle);

  if (prevFrm)
    FrmSetActiveForm(prevFrm);
  FrmDeleteForm(frm);

//  /* Set screen back to the way it was */
//  if (CheckROMVerGreaterThan(3,5))
//    WinScreenMode(winScreenModeSet, NULL, NULL, &oldDepth, NULL);
//  else
//    ScrDisplayMode(scrDisplayModeSet, NULL, NULL, &oldDepth, NULL);

//  FrmDrawForm(prevFrm);
}

Boolean AboutFormHandleEventOnTRASH(EventPtr e)	SEC(TRASH);
Boolean AboutFormHandleEventOnTRASH(EventPtr e)
{
  Boolean       handled = false;
  FormPtr       frm;
  Int8          linesToScroll;
  FieldPtr      fld;

  switch (e->eType)
    {
      case fldChangedEvent:
        UpdateScrollbar(AboutField, AboutScrBar);
        handled = true;
        break;

      case sclRepeatEvent:
        ScrollLines(AboutField, AboutScrBar,
                    e->data.sclRepeat.newValue - e->data.sclRepeat.value,
                    true);
        break;

      case keyDownEvent:
        frm = FrmGetActiveForm();
        fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, AboutField));
        if (e->data.keyDown.chr == pageUpChr)
          {
            if (FldScrollable(fld, winUp))
              {
                linesToScroll = -(FldGetVisibleLines(fld) - 1);
                ScrollLines(AboutField, AboutScrBar, linesToScroll, true);
              }
            handled = true;
          }
        else if (e->data.keyDown.chr == pageDownChr)
          {
            if (FldScrollable(fld, winDown))
              {
                linesToScroll = FldGetVisibleLines(fld) - 1;
                ScrollLines(AboutField, AboutScrBar, linesToScroll, true);
              }
            handled = true;
          }
        break;

      default:
        break;
    }

  return handled;
}

/* Event handler for the About form */
Boolean AboutFormHandleEvent(EventPtr e)
{
	return AboutFormHandleEventOnTRASH(e);
}


/* Scroll a few lines in the given text field */
void ScrollLines(UInt16 fieldID, UInt16 sBarID, Int8 linesToScroll, Boolean redraw)
{
  FormPtr       frm;
  FieldPtr      fld;

  frm = FrmGetActiveForm();
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldID));

  if (linesToScroll < 0)
    FldScrollField(fld, -linesToScroll, winUp);
  else
    FldScrollField(fld, linesToScroll, winDown);

  if ((FldGetNumberOfBlankLines(fld) && (linesToScroll < 0)) || redraw)
    UpdateScrollbar(fieldID, sBarID);
}


/* Updates the scrollbar values */
void UpdateScrollbar(UInt16 fieldID, UInt16 sBarID)
{
  FormPtr       frm;
  ScrollBarPtr  sbar;
  FieldPtr      fld;
  UInt16        curPos;
  UInt16        textHeight;
  UInt16        fieldHeight;
  UInt16        maxValue;

  frm = FrmGetActiveForm();
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldID));
  FldGetScrollValues(fld, &curPos, &textHeight, &fieldHeight);

  if (textHeight > fieldHeight)
    maxValue = textHeight - fieldHeight;
  else if (curPos)
    maxValue = curPos;
  else
    maxValue = 0;

  sbar = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, sBarID));
  SclSetScrollBar(sbar, curPos, 0, maxValue, fieldHeight - 1);
}

