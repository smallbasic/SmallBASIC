/*
*	SmallBASIC Helio Interface Functions (_VTOS)
*
*/


#include <system.h>
#include <setjmp.h>
#include "../device.h"
#include "../osd.h"
#include "../str.h"
#include "res/resource.h"
#include "res/menu.h"
#include "fnt.h"


extern jmp_buf env;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct Pref {
  int fontID;
  char fontName[64];
} Pref;

static Pref pref;


void LoadPreferences()
{
  FILE *fp;
  fp = fopen("SmallBASIC Prefs", "rb");
  if (fp) {
    fread(&pref, 1, sizeof(Pref), fp);
    fclose(fp);
  } else {
    pref.fontID = smallFixedFont;
    pref.fontName[0] = 0;
  }

  if (pref.fontID==userFont)
    FntInstallFont(pref.fontName);
  
  FntSetFont(pref.fontID);
}

void SavePreferences()
{
  FILE *fp;
  fp = fopen("SmallBASIC Prefs", "wb");
  if (fp) {
    fwrite(&pref, 1, sizeof(Pref), fp);
    fclose(fp);
  }
}


void SetPreferences(int id, char *name)
{
  pref.fontID = id;
  strcpy(pref.fontName, name);

  if (pref.fontID==userFont)
    FntInstallFont(pref.fontName);
  
  FntSetFont(pref.fontID);

  SavePreferences();
}

extern BOOLEAN ApplicationHandleEvent(EvtType *event);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static int donefont=0;

void DoFont()
{
  EvtType event;

  donefont=0;
  FormPopupForm(FORM_FONT);
  while(!donefont) {
    EvtGetEvent(&event);
    if(SystemHandleEvent(&event)) continue;
    if(MenuHandleEvent(&event)) continue;
    if(ApplicationHandleEvent(&event)) continue;
    FormDispatchEvent(&event);
    if(event.eventType==EVT_APP_STOP) longjmp(env, 1);
  }
  FormPopupForm(FORM_IFACE);
}


static void ChangeToFont(char *str) {
  int id;
  if (strcmp(str, "Small Fixed Font")==0) id=smallFixedFont;
  else if (strcmp(str, "Standard Fixed Font")==0) id=stdFixedFont;
  else id = userFont;
  
  SetPreferences(id, str);
}


BOOLEAN FormFontHandleEvent(EvtType* Event)
{
  SHORT item;
  BYTE *ptr;
	
  switch(Event->eventType) {
  case EVT_FORM_OPEN:
    FormDrawForm(Event->eventID);
    ListDeleteAllItems(LIST_FONT);
    ListAllDBs("FontPDB", LIST_FONT);
    ListInsertItem(LIST_FONT, 0, "Small Fixed Font");
    ListInsertItem(LIST_FONT, 1, "Standard Fixed Font");
    ListSetSelectedItem(LIST_FONT, NO_SELECTION);
    ListDrawList(LIST_FONT);
    break;

  case EVT_LIST_SELECT:
    ListSetSelectedItem(Event->eventID, Event->para1);
    ListSetHighlightedItem(Event->eventID, Event->para1);
    ListDrawList(Event->eventID);
    return TRUE;
		
  case EVT_INLAY_SELECT:
    switch(Event->para1) {
    case INLAY_OK:
      ListGetTotalItems(LIST_FONT, &item);
      if (item==0) break;
      ListGetSelectedItem(LIST_FONT, &item);
      if (item == NO_SELECTION) return TRUE;
      ListGetListItem(LIST_FONT, item, (BYTE **)&ptr);
      ChangeToFont(ptr);
      donefont = 1;
      break;
    case INLAY_EXIT:
    case INLAY_MAIN_MENU:
      donefont = 1;
      break;
    default:
      return FALSE;
    }
    break;
		
  case EVT_CONTROL_SELECT:
    ListGetTotalItems(LIST_FONT, &item);
    if (item==0) break;
    ListGetSelectedItem(LIST_FONT, &item);
    if (item == NO_SELECTION) return TRUE;
    ListGetListItem(LIST_FONT, item, (BYTE **)&ptr);
    switch (Event->eventID) {
    case OPEN_FONT:
      ChangeToFont(ptr);
      donefont = 1;
      break;
    case DELETE_FONT:
      if (unlink(ptr)) {
	EvtAppendEvt(EVT_FORM_CLOSE, FORM_FONT, 0, 0, NULL);
	FormPopupForm(FORM_FONT);
      }
      break;
    }
    break;
		
  default:
    return FALSE;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static int yesno, doneyesno;
static char *yesnoPrompt, *yesnoTitle;

int DoYesNo(char *title, char *prompt, int form)
{
  EvtType event;
  void *save;

  yesno = 0;
  doneyesno=0;
  yesnoPrompt = prompt;
  yesnoTitle = title;
  save = SaveBackground();
  StippleBackground();
  FormPopupForm(FORM_YESNO);
  do {
    EvtGetEvent(&event);
    if(SystemHandleEvent(&event)) continue;
    if(MenuHandleEvent(&event)) continue;
    if(ApplicationHandleEvent(&event)) continue;
    FormDispatchEvent(&event);
  } while (!doneyesno);
  RestoreBackground(save);
  if (form>=0) FormPopupForm(form);
  return yesno;
}


static void YesNoOK(int OK)
{
  //  FormRestoreBitBehind(FORM_YESNO);
  EvtAppendEvt(EVT_FORM_CLOSE, FORM_YESNO, 0, 0, NULL);
  yesno = OK;
  doneyesno=1;
}


BOOLEAN FormYesNoHandleEvent(EvtType* event)
{
  switch(event->eventType) {
    case EVT_FORM_OPEN:
      //      FormSaveBehindBits(event->eventID);
      StringSetText(STRING_YESNO, yesnoPrompt);
      FormSetDialogTitle(event->eventID, yesnoTitle);
      FormDrawForm(event->eventID);
      break;

    case EVT_INLAY_SELECT:
    switch(event->para1) {
    case INLAY_OK:
      YesNoOK(1);
      break;
    case INLAY_EXIT:
    case INLAY_MAIN_MENU:
      YesNoOK(0);
      break;
    default:
      return FALSE;
    }
  case EVT_CONTROL_SELECT:
    switch (event->eventID) {
    case BUTTON_YESNOOK:
      YesNoOK(1);
      break;
    case BUTTON_YESNOCANCEL:
      YesNoOK(0);
      break;
    }

    default:
      return FALSE;
    }
  return FALSE;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static int doneedit=0;
static char *editname;
static int resume=0;
static char resumename[33];
static int active=0;

void DoEdit(char *name)
{
  EvtType event;

  doneedit=0;
  active=0;
  editname = name;
  FormPopupForm(FORM_EDIT);
  while(!doneedit) {
    EvtGetEvent(&event);
    if(SystemHandleEvent(&event)) continue;
    if(MenuHandleEvent(&event)) continue;
    if(ApplicationHandleEvent(&event)) continue;
    FormDispatchEvent(&event);
    if(event.eventType==EVT_APP_STOP) longjmp(env, 1);
  }
  FormPopupForm(FORM_IFACE);
}


static void LoadFile(char *editname)
{
  FILE *fp;
  int sz, i, c;
  char *ptr, *mem;

  fp = fopen(editname, "rb");
  if (fp) {
    sz = fsize(fp);
    mem = ptr = qmalloc(2*sz+1);
    if (ptr) {
      c=0;
      for (i=0; i<sz; i++) {
	*ptr=fgetc(fp);
	if (*ptr==0x0d) {
	  c=fgetc(fp);
	  if (c!=0x0a && c!=EOF) ungetc(c, fp);
	  else i++;
	} else if (*ptr==0x0a) {
	  *ptr=0x0d;
	  c=fgetc(fp);
	  if (c!=0x0d && c!=EOF) ungetc(c, fp);
	  else i++;
	}
	ptr++;
      }
      *ptr=0;
      FieldSetText(FIELD_EDIT, mem);
      qfree(mem);
    }
    fclose(fp);
  } else {
    FieldSetText(FIELD_EDIT, "");
  }
}


static void SaveFile(char *editname)
{
  FILE *fp;
  int sz, i;
  char *ptr;
 
  filetype("BASIC");
  fp = fopen(editname, "wb");
  if (fp) {
    FieldGetNumOfChars(FIELD_EDIT, &sz);
    FieldGetTextPointer(FIELD_EDIT, (BYTE**)&ptr);
    for (i=0; i<sz; i++) {
      if (ptr[i]==0x0d) fputc(0x0a, fp);
      else fputc(ptr[i], fp);
    }
    fclose(fp);
  }
}

static void Field2SB()
{
  WORD totlines, topline, maxlines, max, pg;

  FieldGetTopLineNum(FIELD_EDIT, &topline);
  FieldGetMaxNumLinesDisplay(FIELD_EDIT, &maxlines);
  FieldGetTotalNumOfLines(FIELD_EDIT, &totlines);

  pg = maxlines-1;
  max = totlines-pg-1;	
  
  if (totlines <= maxlines) { // No scrollbar
    ScrollbarSetScrollbarVisible(SCROLL_EDIT, FALSE);
    ScrollbarEraseScrollbar(SCROLL_EDIT);
  } else { // It's present
    ScrollbarSetScrollbarVisible(SCROLL_EDIT, TRUE);
    ScrollbarSetScrollbarDrawPagesize(SCROLL_EDIT, maxlines);
    if (topline > (totlines - maxlines))
      ScrollbarSetScrollbar(SCROLL_EDIT, max, max, 0, pg, totlines);
    else
      ScrollbarSetScrollbar(SCROLL_EDIT, topline, max, 0, pg, totlines);
    
    ScrollbarDrawScrollbar(SCROLL_EDIT);
  }
}

static void EditSetField()
{
  Field	*field_ptr;
  int inson, hiliteon;
  BYTE	object_type, font_id;
  SHORT	line_height;
  WORD	first, last, count = 0;
  WORD	line_num, insert_pos;
  WORD	temp;
	
  FormGetObjectPointer(FIELD_EDIT, &object_type, (void**)&field_ptr);		
  inson = field_ptr->field_attr.field_insert_pt_visible;
  hiliteon = field_ptr->field_attr.field_highlight;
	
  FieldGetFont(FIELD_EDIT, &font_id);
  line_height = SysGetFontHeight(font_id) + SPACE_LINE;
	
  StrAnalyzeLine(field_ptr);
  FieldGetFirstVisibleChar(FIELD_EDIT, &first);
  FieldGetLastVisibleChar(FIELD_EDIT, &last);
	
  if (inson == TRUE || hiliteon == TRUE) {
      if (inson) {
	for (count = 0; count < field_ptr->field_total_num_lines; count++) {
	  if (field_ptr->field_insert_pt_char_pos >= field_ptr->field_lineinfo[count].start &&
	      field_ptr->field_insert_pt_char_pos <= field_ptr->field_lineinfo[count].start + field_ptr->field_lineinfo[count].length)
	    break;
	}
	
	if (field_ptr->field_insert_pt_char_pos < first)
	  field_ptr->field_top_line_num = count;
	else if (field_ptr->field_insert_pt_char_pos > last)
	  field_ptr->field_top_line_num = count - field_ptr->field_num_lines_displayed;
	
	if (field_ptr->field_num_lines_displayed < (field_ptr->bounds.height/(line_height))) {
	  if (field_ptr->field_top_line_num > (field_ptr->field_total_num_lines - (field_ptr->bounds.height/(line_height))))
	    field_ptr->field_top_line_num = (field_ptr->field_total_num_lines - (field_ptr->bounds.height/(line_height)));
	  
	  if (field_ptr->field_top_line_num < 0)
	    field_ptr->field_top_line_num = 0;
	}
      }
      StrAnalyzeLine(field_ptr);
      StrCharPosToXY(field_ptr, field_ptr->field_insert_pt_char_pos,  &(field_ptr->field_insert_pt_x), &(field_ptr->field_insert_pt_y));

      FieldGetInsertPointPosition(FIELD_EDIT, &insert_pos);
      FieldGetFirstVisibleChar (FIELD_EDIT, &first);
      FieldGetLastVisibleChar (FIELD_EDIT, &last);
      
      if (inson) {
	if(insert_pos > last) {
	  FieldCharPosToLineNum (field_ptr, insert_pos, &line_num);
	  FieldSetTopLineNum(FIELD_EDIT, line_num - (field_ptr->bounds.height/(line_height)) + 1);
	}
	FieldSetInsertPointPositionByCharPos(FIELD_EDIT, insert_pos);
      } else {
	temp = field_ptr->field_highlight_start_char;
	if (temp < field_ptr->field_highlight_end_char)
	  temp = field_ptr->field_highlight_end_char;
	if (temp >= last) {
	  FieldCharPosToLineNum(field_ptr, temp, &line_num);
	  FieldSetTopLineNum(FIELD_EDIT, max(line_num - (field_ptr->bounds.height/(line_height)) + 1,0));
	} else {
	  FieldGetNumOfChars(FIELD_EDIT, &temp);
	  FieldCharPosToLineNum(field_ptr, temp, &line_num);
	  if ((field_ptr->field_top_line_num + (field_ptr->bounds.height/(line_height))) >= line_num)
	    FieldSetTopLineNum(FIELD_EDIT, max(line_num - (field_ptr->bounds.height/(line_height)) + 1, 0));
	}
      }
  } else {
    FieldCharPosToLineNum(field_ptr, last, &line_num);
    if ((field_ptr->field_top_line_num + (field_ptr->bounds.height/(line_height))) >= line_num)
      FieldSetTopLineNum(FIELD_EDIT, max(0, line_num - (field_ptr->bounds.height/(line_height)) + 1));
  }

}

static void FitKeyboard()
{
  unsigned char type;
  Scrollbar *sb;
  ObjectBounds bnds;

  FieldGetFieldBounds(FIELD_EDIT, &bnds);

  if (!KeyboardCheckKeyboardStatus()) bnds.height = 140;
  else bnds.height = 140-58;

  FieldSetBounds(FIELD_EDIT, bnds);
  FormGetObjectPointer(SCROLL_EDIT, &type, (void**)&sb);
  sb->bounds.height = bnds.height;

  // Still need to redraw!
}


void SaveResumeFile()
{
  FILE *fp;
  SaveFile("SmallBASIC.AutoResume.Text");
  filetype("SmallBASIC Resume");
  fp=fopen("SmallBASIC.AutoResume.Name", "wb");
  fwrite(editname, 1, strlen(editname)+1, fp);
  fclose(fp);
}

char *CheckResumeFile()
{
  FILE *fp;

  resumename[0] = 0;
  fp=fopen("SmallBASIC.AutoResume.Name", "rb");
  if (fp) {
    fread(resumename, 1, 32, fp);
    resumename[32]=0;
    fclose(fp);
    //    LoadFile("SmallBASIC.AutoResume.Text");
    unlink("SmallBASIC.AutoResume.Name");
    active=1;
    resume = 1;
    return resumename;
  }
  return NULL;
}

BOOLEAN FormEditHandleEvent(EvtType* event)
{
  BYTE	object_type;
  Field	*fld_addr;
  WORD max,min,value,pagesize;
  WORD scroll_total_lines;
  static int insertpos;
  static int dirty = 0;
  char tmpname[34];

  switch (event->eventType) {
  case EVT_FORM_OPEN:
    dirty = 0;
    FormSetDialogTitle(FORM_EDIT, editname);
    if (resume) {
      dirty = 1;
      resume = 0;
      active=0;
      LoadFile("SmallBASIC.AutoResume.Text");
      unlink("SmallBASIC.AutoResume.Text");
    } else {
      if (!active)
	LoadFile(editname);
    }
    FormGetObjectPointer(FIELD_EDIT, &object_type, (void **)&fld_addr);
    FormSetFormActiveObject(FORM_EDIT, FIELD_EDIT);
    FormObjectSetFocus(FIELD_EDIT);
    FieldSetHighlightSelection(FIELD_EDIT, 0, 0);
    FieldSetTopLineNum(FIELD_EDIT, 0);
    FieldSetInsertPointPositionByCharPos(FIELD_EDIT, 0);
    FieldSetInsertPointOn(FIELD_EDIT);
    FitKeyboard();
    Field2SB();
    FormDrawForm(FORM_EDIT);
    insertpos = 0;
    active=0;
    return TRUE;
		
  case EVT_KEYBOARD_STATUS:
    FitKeyboard();
    EditSetField();
    Field2SB();
    FieldEraseField(FIELD_EDIT);
    FieldDrawField(FIELD_EDIT);
    return TRUE;
		
  case EVT_SCROLLBAR_REPEAT:
    FormSetFormActiveObject(FORM_EDIT, FIELD_EDIT);
    FormGetObjectPointer(FIELD_EDIT, &object_type, (void **)&fld_addr);
    
    if ((fld_addr->field_insert_pt_x >= fld_addr->bounds.xcoord && fld_addr->field_insert_pt_x < fld_addr->bounds.xcoord + fld_addr->bounds.width) &&
	(fld_addr->field_insert_pt_y >= fld_addr->bounds.ycoord && fld_addr->field_insert_pt_y < fld_addr->bounds.ycoord + fld_addr->bounds.height))
      FieldGetInsertPointPosition(FIELD_EDIT, &insertpos);
    
    ScrollbarGetScrollbar(SCROLL_EDIT, &value, &max, &min, &pagesize, &scroll_total_lines);
    ScrollbarSetScrollbar(SCROLL_EDIT, event->para2, max, min, pagesize, scroll_total_lines);
    
    FieldSetTopLineNum(FIELD_EDIT, event->para2);
    FieldDrawField(FIELD_EDIT);
    ScrollbarDrawScrollbar(SCROLL_EDIT);
		
    if (!fld_addr->field_attr.field_highlight)
      FieldSetInsertPointPositionByCharPos(FIELD_EDIT, insertpos);
			
    if ((fld_addr->field_insert_pt_x >= fld_addr->bounds.xcoord && fld_addr->field_insert_pt_x < fld_addr->bounds.xcoord + fld_addr->bounds.width) &&
	(fld_addr->field_insert_pt_y >= fld_addr->bounds.ycoord && fld_addr->field_insert_pt_y < fld_addr->bounds.ycoord + fld_addr->bounds.height)) {
      if (!fld_addr->field_attr.field_highlight)
	FieldSetInsertPointOn(FIELD_EDIT);
    } else
      FieldSetInsertPointOff(FIELD_EDIT);
    return TRUE;
		
  case EVT_SCROLLBAR_SELECT:
    ScrollbarDrawScrollbar(SCROLL_EDIT);
    return TRUE;
    
  case EVT_KEY:
    if (event->eventID == SOFT_KEY) {
      int ipos, st, ed;
      EditSetField();
      Field2SB();
      FieldGetInsertPointPosition(FIELD_EDIT, &ipos);
      FieldGetCurrentHighlightedSelection(FIELD_EDIT, &st, &ed);
      if (ipos>=0 || st>=0 ) {
	FieldGetInsertPointPosition(FIELD_EDIT, &insertpos);
	FieldAddKeyInChar(FIELD_EDIT,(BYTE)(event->para1));
      }
      dirty = 1;
      return TRUE;
    }
    return TRUE;
		
  case EVT_FIELD_CHANGED:
    Field2SB();
    dirty = 1;
    return TRUE;
		
  case EVT_FIELD_MODIFIED:
    FieldGetInsertPointPosition(FIELD_EDIT, &insertpos);
    dirty = 1;
    return TRUE;

  case EVT_APP_STOP:
    SaveResumeFile();
    break;
    
  case EVT_MENU_SELECT_ITEM:
    switch(event->para1) {
    case MENU_COPY: dirty = 1; KeyboardSendEvent(KEY_COPY, FALSE, FALSE, FALSE); return TRUE;
    case MENU_CUT: dirty = 1; KeyboardSendEvent(KEY_CUT, FALSE, FALSE, FALSE); return TRUE;
    case MENU_PASTE: dirty = 1; KeyboardSendEvent(KEY_PASTE, FALSE, FALSE, FALSE); return TRUE;
    case MENU_UNDO: dirty = 1; KeyboardSendEvent(KEY_UNDO, FALSE, FALSE, FALSE); return TRUE;
    case MENU_SAVE: 
      SaveFile(editname);
      dirty = 0;
      doneedit=1;
     return TRUE;
    case MENU_SAVEAS:
      strcpy(tmpname, editname);
      if (DoNew(tmpname)) {
	SaveFile(tmpname);
	dirty = 0;
	doneedit=1;
      } else {
	active=1;
	FormPopupForm(FORM_EDIT);
      }
     return TRUE;
    case MENU_EXIT:
      active=1;
      if (dirty) {
	if (!DoYesNo("File Modified", "Exit Without Saving?", FORM_EDIT)) return TRUE;
      }
      doneedit=1;
      break;
    }
    return FALSE;
    
  case EVT_IO_KEY_CTRL:
    if (event->eventID == EVT_IO_KEY_PRESS || event->eventID == EVT_IO_KEY_REPEAT) {
      if (event->para2 == IO_UP_ARROW)
	ScrollbarHardButtonSetScrollbar(SCROLL_EDIT, SCROLLBAR_UP_ARROW);
      else if (event->para2 == IO_DOWN_ARROW)
	ScrollbarHardButtonSetScrollbar(SCROLL_EDIT, SCROLLBAR_DOWN_ARROW);
      return TRUE;
    }
    return FALSE;
      
  case EVT_INLAY_SELECT:
    switch (event->para1) {
    case INLAY_OK:
      SaveFile(editname);
      dirty = 0;
      doneedit=1;
      return TRUE;
    case INLAY_EXIT:
      active=1;
      if (dirty) {
	if (!DoYesNo("File Modified", "Exit Without Saving?", FORM_EDIT)) return TRUE;
      }
      doneedit=1;
      
      return TRUE;
    default:
      return FALSE;
    }
    return TRUE;
    
  default:
    return FALSE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static char *newname;
static int donenew;
static int newok;
int DoNew(char *name)
{
  EvtType event;

  donenew=0;
  newok = 0;
  newname = name;
  StippleBackground();
  FormPopupForm(FORM_NEW);
  while(!donenew) {
    EvtGetEvent(&event);
    if(SystemHandleEvent(&event)) continue;
    if(MenuHandleEvent(&event)) continue;
    if(ApplicationHandleEvent(&event)) continue;
    FormDispatchEvent(&event);
    if(event.eventType==EVT_APP_STOP) longjmp(env, 1);
  }
  //  FormPopupForm(FORM_IFACE);
  return newok;
}

static void NewOK(int ok)
{
  BYTE *text_ptr;

  TextboxGetTextPointer(TEXT_NEW, &text_ptr);
  strcpy(newname, text_ptr);
  TextboxSetInsertPointOff(TEXT_NEW);
  FormRestoreBitBehind(FORM_NEW);
  EvtAppendEvt(EVT_FORM_CLOSE, FORM_NEW, 0, 0, NULL);
  if (strlen(newname)<1) ok=0;
  else if (ok) {
    if (fexists(newname)) {
      if (!DoYesNo("Warning, File Exists!", "Overwrite File?", -1))
	ok=0;
      else
	unlink(newname);
    }
  }
  newok=ok;
  donenew=1;
}


BOOLEAN FormNewHandleEvent(EvtType* event)
{
  switch(event->eventType) {
  case EVT_FORM_OPEN:
    FormSaveBehindBits(event->eventID);
    FormDrawForm(event->eventID);
    TextboxSetText(TEXT_NEW, newname);
    TextboxSetInsertPointPositionByCharPos(TEXT_NEW, strlen(newname));
    TextboxSetInsertPointOn(TEXT_NEW);
    TextboxDrawTextbox(TEXT_NEW);
    FormSetFormActiveObject(FORM_NEW, TEXT_NEW);
    FormObjectSetFocus(TEXT_NEW);
    break;
	
  case EVT_KEY:
    if (event->eventID == SOFT_KEY) {	
      if ((BYTE)(event->para1) == 13)
	NewOK(1);
      else
	TextboxAddKeyInChar(TEXT_NEW, (BYTE)(event->para1));
    }
    return TRUE;
    break;

  case EVT_INLAY_SELECT:
    switch(event->para1) {
    case INLAY_OK:
      NewOK(1);
      break;
    case INLAY_EXIT:
    case INLAY_MAIN_MENU:
      NewOK(0);
      break;
    default:
      return FALSE;
    }
    break;
	
  default:
    return FALSE;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

BOOLEAN FormAboutHandleEvent(EvtType* Event)
{
  switch(Event->eventType) {
  case EVT_FORM_OPEN:
    StippleBackground();
    FormSaveBehindBits(Event->eventID);
    FormDrawForm(Event->eventID);
    break;

  case EVT_INLAY_SELECT:
    switch(Event->para1) {
    case INLAY_OK:
    case INLAY_EXIT:
    case INLAY_MAIN_MENU:
      FormRestoreBitBehind(FORM_ABOUT);
      EvtAppendEvt(EVT_FORM_CLOSE, FORM_ABOUT, 0, 0, NULL);
      FormPopupForm(FORM_IFACE);
      break;
    default:
      return FALSE;
    }
    break;
	
  default:
    return FALSE;
  }
  return TRUE;
}

