/*
 *	SmallBASIC platform driver for Helio (_VTOS)
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


jmp_buf env;
static int	cur_x = 0;
static int	cur_y = 0;
static int	mouse_mode, mouse_x, mouse_y, mouse_is_down/*mouse_b*/, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;
static int	tabsize = 32;	// from dev_palm
static int	maxline;

// font data
static int	font_w = 8;
static int	font_h = 16;

static int	con_use_bold = 0;
static int	con_use_ul   = 0;
static int	con_use_reverse = 0;

// VGA16 colors in RGB
static unsigned long vga16[] =
  {
    0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
    0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF 
  };
static unsigned long cmap[16];


BOOLEAN (*FormDispatchEvent)(EvtType *event);
BOOLEAN ApplicationHandleEvent(EvtType *event);
BOOLEAN FormMainHandleEvent(EvtType* event);
BOOLEAN FormIfaceHandleEvent(EvtType* event);
BOOLEAN FormEditHandleEvent(EvtType* event);
BOOLEAN FormYesNoHandleEvent(EvtType* event);
BOOLEAN FormNewHandleEvent(EvtType* event);
BOOLEAN FormAboutHandleEvent(EvtType* event);
BOOLEAN FormFontHandleEvent(EvtType* event);
extern void LoadPreferences();
extern int DoYesNo(char *title, char *msg, int form);
extern void CpuChangeSpeed(int speed);
extern void DoFont();
extern int DoNew(char *name);
extern void my_cleanup();
extern void brun(char *, int);
extern void DoEdit(char *name);

void	osd_nextln();

/*
 *	build color map
 */
void	build_colors()
{
  int			i;
  int red, green, blue;
  for ( i = 0; i < 16; i ++ )	{
    red   = ((vga16[i] & 0xff0000) >> 16) ;
    green = ((vga16[i] & 0xff00) >> 8) ;
    blue  = (vga16[i] & 0xff) ;
    if (((red+green+blue)/3)>127) 
      cmap[i] = COLOR_WHITE;
    else
      cmap[i] = COLOR_BLACK;
  }
}
int ready;

/*
 */
int		osd_devinit()
{
  cur_x = 0;
  cur_y = 0;
  mouse_mode=0;
  mouse_x=0;
  mouse_y=0;
  mouse_is_down=0;
  mouse_upd=0;
  mouse_down_x=0;
  mouse_down_y=0;
  mouse_pc_x=0;
  mouse_pc_y=0;
  tabsize = 32;	// from dev_palm
  maxline=0;
  con_use_bold = 0;
  con_use_ul   = 0;
  con_use_reverse = 0;

  LoadPreferences();
  //  FntSetFont(smallFixedFont);
  font_h = FntLineHeight();
  font_w = FntCharWidth('W');

  FormPopupForm(FORM_MAIN);

  os_graf_my = 160;
  os_graf_mx = 160;
  mouse_is_down = 0;


  // setup fonts & sizes
  maxline = os_graf_my / font_h;

  // setup palette
  build_colors();

  if (ready) {
    setsysvar_int(SYSVAR_BPP, 1);
  }

  osd_cls();
  return 1;
}

/*
 */
int		osd_devrestore()
{
  return 1;
}
int eol=0;
void PauseForKey()
{
  int osd_events_sleep(void);

  eol=1;
  osd_nextln();
  osd_write("Press any key to exit...");
  while(osd_events());
  dev_events(1);
  eol=0;
}

static int buttcover = 0;

int hard_exit = 0;
int osd_events()
{
  EvtType event;

  if (!buttcover)
    EvtAppendEvt(0xdead, 0, 0, 0, NULL);
  buttcover = 1;
  while(!hard_exit) {
    EvtGetEvent(&event);
    if (SystemHandleEvent(&event)) if (event.eventType!=EVT_KEY) continue;
    if (MenuHandleEvent(&event)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    if (FormDispatchEvent(&event)) break;
    if(event.eventType==EVT_APP_STOP) longjmp(env, 1);
  }
  if (event.eventType==0xdead) buttcover = 0;
  if (hard_exit) {
    buttcover=0;
    longjmp(env,1);
  }
  return buttcover;
}

int osd_events_sleep()
{
  EvtType event;

  LcdSetPixel(159, 159, COLOR_BLACK);
  while(!hard_exit) {
    EvtGetEvent(&event);
    if (SystemHandleEvent(&event)) if (event.eventType!=EVT_KEY) continue;
    if (MenuHandleEvent(&event)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    if (FormDispatchEvent(&event)) break;
    if(event.eventType==EVT_APP_STOP) longjmp(env, 1);
  }
  if (hard_exit) {
    longjmp(env,1);
  }
  LcdSetPixel(159, 159, COLOR_WHITE);
  return 1;
}



/*
 */
void	osd_settextcolor(long fg, long bg)
{
  osd_setcolor(fg);
}

/*
 *	enable or disable PEN code
 */
void	osd_setpenmode(int enable)
{
  mouse_mode = enable;
}

/*
 */
int		osd_getpen(int code)
{
  int		r = 0;

  osd_events();	
  if	( mouse_mode )	{
    switch ( code )	{
    case 	0:	// bool: status changed
      r = mouse_upd;
      break;		
    case	1:	// last pen-down x
      r = mouse_down_x;
      break;		
    case	2:	// last pen-down y
      r = mouse_down_y;
      break;		
    case	3:	// vert. 1 = down, 0 = up .... unsupported
      r = mouse_is_down;
      break;
    case	4:	// last x
      r = mouse_pc_x;
      break;
    case	5:	// last y
      r = mouse_pc_y;
      break;
    case	10:
      r = mouse_x;
      break;
    case	11:
      r = mouse_y;
      break;
    case	12:
    case	13:
    case	14:
      r = mouse_is_down; //(mouse_b & (1 << (code - 11))) ? 1 : 0;
      break;
    }

    mouse_upd = 0;
  }
  return	r;
}

//
void	osd_cls()
{
  ObjectBounds bnds;

  cur_x = cur_y = 0;
	
  bnds.xcoord = 0;
  bnds.ycoord = 0;
  bnds.width = os_graf_mx;
  bnds.height = os_graf_my;
  LcdEraseRegion(&bnds);
}

//	returns the current x position
int		osd_getx()
{
  return cur_x;
}

//	returns the current y position
int		osd_gety()
{
  return cur_y;
}

//
void	osd_setxy(int x, int y)
{
  cur_x = x;
  cur_y = y;
}

/**
 *	next line
 */
void	osd_nextln()
{
  ObjectBounds bnds;
  BitmapTemplate b;

  cur_x = 0;

  if	( cur_y < ((maxline-1) * font_h) )	{
    cur_y += font_h;
  }
  else	{
    b.xcoord = 0;
    b.ycoord = font_h;
    b.width = os_graf_mx;
    b.height = os_graf_my-font_h;
    LcdGetBitmap(&b);
    b.ycoord = 0;
    LcdDrawBitmap(&b, FALSE);
    qfree(b.bitmap_data);

    bnds.xcoord = 0;
    bnds.ycoord = os_graf_my-font_h;
    bnds.width = os_graf_mx;
    bnds.height = font_h;
    LcdEraseRegion(&bnds);
    cur_y = ((maxline-1) * font_h);
  }
}

/*
 *	calc next tab position
 */
int		osd_calctab(int x)
{
  int		c = 1;

  while ( x > tabsize )	{
    x -= tabsize;
    c ++;
  }
  return c * tabsize;
}

/**
 *	Basic output
 *
 *	Supported control codes:
 *	\t		tab (20 px)
 *	\a		beep
 *	\n		next line (cr/lf)
 *	\xC		clear screen
 *	\e[K	clear to end of line
 *	\e[0m	reset all attributes to their defaults
 *	\e[1m	set bold on
 *	\e[4m	set underline on
 *	\e[7m	reverse video
 *	\e[21m	set bold off
 *	\e[24m	set underline off
 *	\e[27m	set reverse off
 */
void	osd_write(const char *str)
{
  int	len, cx, esc_val, esc_cmd;
  byte	*p, buf[3];
  int	 char_len = 1;
  ObjectBounds bnds;

  len = strlen(str);

  if	( len <= 0 )
    return;

  p = (byte *) str;
  while ( *p )	{
    switch ( *p )	{
    case	'\a':	// beep
      dev_beep();
      break;
    case	'\t':
      cur_x = osd_calctab(cur_x+1);
      break;
    case	'\xC':
      osd_cls();
      break;
    case	'\033':		// ESC ctrl chars (man console_codes)
      if	( *(p+1) == '[' )	{
	p += 2;
	esc_val = esc_cmd = 0;

	if	( is_digit(*p) )	{
	  esc_val = (*p - '0');
	  p ++;

	  if	( is_digit(*p) )	{
	    esc_val = (esc_val * 10) + (*p - '0');
	    p ++;
	  }

	  esc_cmd = *p;
	}
	else	
	  esc_cmd = *p;

	// control characters
	switch ( esc_cmd )	{
	case	'G':			// \e[nG - go to column
	  cur_x = esc_val*font_w;
	  break;
	case	'K':			// \e[K - clear to eol
	  bnds.xcoord = cur_x;
	  bnds.ycoord = cur_y;
	  bnds.width = os_graf_mx - cur_x;
	  bnds.height = font_h;
	  LcdDrawBox(&bnds,cmap[dev_bgcolor],cmap[dev_bgcolor], TRUE);
	  break;
	case	'm':			// \e[...m	- ANSI terminal
	  switch ( esc_val )	{
	  case	0:	// reset
	    con_use_bold = 0;
	    con_use_ul = 0;
	    con_use_reverse = 0;
	    osd_setcolor(0);
	    osd_settextcolor(0, 15);
	    break;
	  case	1:	// set bold on
	    con_use_bold = 1;
	    break;
	  case	4:	// set underline on
	    con_use_ul = 1;
	    break;
	  case	7:	// reverse video on
	    con_use_reverse = 1;
	    break;
	  case	21:	// set bold off
	    con_use_bold = 0;
	    break;
	  case	24:	// set underline off
	    con_use_ul = 0;
	    break;
	  case	27:	// reverse video off
	    con_use_reverse = 0;
	    break;

	    // colors - 30..37 foreground, 40..47 background
	  case	30:	// set black fg
	    osd_setcolor(0);
	    break;
	  case	31:	// set red fg
	    osd_setcolor(4);
	    break;
	  case	32:	// set green fg
	    osd_setcolor(2);
	    break;
	  case	33:	// set brown fg
	    osd_setcolor(6);
	    break;
	  case	34:	// set blue fg
	    osd_setcolor(1);
	    break;
	  case	35:	// set magenta fg
	    osd_setcolor(5);
	    break;
	  case	36:	// set cyan fg
	    osd_setcolor(3);
	    break;
	  case	37:	// set white fg
	    osd_setcolor(7);
	    break;
	  };
	  break;
	}
      }
      break;
    case	'\n':		// new line
      osd_nextln();
      break;
    case	'\r':		// return
      cur_x = 0;
      bnds.xcoord = cur_x;
      bnds.ycoord = cur_y;
      bnds.width = os_graf_mx - cur_x;
      bnds.height = font_h;
      LcdDrawBox(&bnds,cmap[dev_bgcolor],cmap[dev_bgcolor], TRUE);
      break;
    default:
      //
      //	PRINT THE CHARACTER
      //
      buf[0] = *p;

      cx = font_w;
      buf[1] = '\0';

      // new line ?
      if ( cur_x + cx >= os_graf_mx )
	osd_nextln();

      // draw
      if ( con_use_ul )
	FntSetUnderlineMode(solidUnderline);
      else
	FntSetUnderlineMode(noUnderline);

      if ( !con_use_reverse )
	FntDrawChars( buf, char_len, cur_x, cur_y, FALSE);
      else
	FntDrawChars( buf, char_len, cur_x, cur_y, TRUE);

      // advance
      cur_x += cx;
    };

    if	( *p == '\0' )
      break;

    p ++;
  }
}

void	osd_setcolor(long color)
{
  dev_fgcolor = color;
}

void	osd_line(int x1, int y1, int x2, int y2)
{
  if	( (x1 == x2) && (y1 == y2) )
    LcdSetPixel(x1, y1, cmap[dev_fgcolor]);
  else
    LcdDrawLine(x1, y1, x2, y2, 1, NON_DOTTED_LINE, cmap[dev_fgcolor], COLOR_WHITE);
}

void	osd_setpixel(int x, int y)
{
  LcdSetPixel(x, y, cmap[dev_fgcolor]);
}

long	osd_getpixel(int x, int y)
{
  BYTE color;
  LcdGetPixel(x, y, &color);
  if (color==COLOR_WHITE)
    return 15;
  else
    return 0;
}

void	osd_rect(int x1, int y1, int x2, int y2, int fill)
{
  int		y;

  if	( fill )	{
    for ( y = y1; y <= y2; y ++ )	
      osd_line(x1, y, x2, y);
  }
  else	{
    osd_line(x1, y1, x1, y2);
    osd_line(x1, y2, x2, y2);
    osd_line(x2, y2, x2, y1);
    osd_line(x2, y1, x1, y1);
  }
}

void	osd_refresh()
{
}


int		osd_textwidth(const char *str)
{
  int		l = strlen(str);

  return l * font_w;
}

int		osd_textheight(const char *str)
{
  return font_h;
}


void	osd_beep()
{
  FntBeep();
}

void	osd_sound(int frq, int ms, int vol, int bgplay)
{
}




BOOLEAN ApplicationHandleEvent(EvtType *event)
{
  BYTE object_type;
  Form *form_ptr;
  void* evtHandler;
	
  switch(event->eventType) {
  case EVT_FORM_LOAD:
    if((event->para1==1)||(UISearchForAddress((ObjectID)event->eventID,&object_type,(void**)&form_ptr)!=TRUE))
      FormInitAllFormObjects((ObjectID)event->eventID);
    if(UISearchForAddress((ObjectID)event->eventID,&object_type,(void**)&form_ptr)!=TRUE)
      return FALSE;
    switch((ObjectID)event->eventID) {
    case FORM_MAIN: evtHandler = (void*)FormMainHandleEvent; break;
    case FORM_IFACE: evtHandler = (void*)FormIfaceHandleEvent; break;
    case FORM_EDIT: evtHandler = (void*)FormEditHandleEvent; break;
    case FORM_YESNO: evtHandler = (void*)FormYesNoHandleEvent; break;
    case FORM_NEW: evtHandler = (void*)FormNewHandleEvent; break;
    case FORM_ABOUT: evtHandler = (void*)FormAboutHandleEvent; break;
    case FORM_FONT: evtHandler = (void*)FormFontHandleEvent; break;
    default: return FALSE;
    }
    FormSetEventHandler((ObjectID)event->eventID,(void**)&FormDispatchEvent,evtHandler);
    FormSetActiveForm((ObjectID)event->eventID);
    break;
  default:
    return FALSE;
  }
  return TRUE;
}

BOOLEAN FormMainHandleEvent(EvtType* event)
{
  switch(event->eventType) {
  case 0xdead:
    return TRUE;

  case EVT_FORM_OPEN:
    FormSetFormActiveObject(FORM_MAIN, TEXTBOX_MAIN);
    FormObjectSetFocus(TEXTBOX_MAIN);
    TextboxSetText(TEXTBOX_MAIN, "");
    TextboxSetInsertPointPositionByCharPos(TEXTBOX_MAIN, 0);
    TextboxDrawTextbox(TEXTBOX_MAIN);
    TextboxSetInsertPointOn(TEXTBOX_MAIN);
    break;
    
  case EVT_KEY:
    FormObjectSetFocus(TEXTBOX_MAIN);
    if (!eol) dev_pushkey(event->para1);
    return TRUE;
    
  case PEN_EVENT:
    switch (event->eventID) {
    case PEN_DOWN:
      mouse_is_down = 1;
      mouse_down_x = event->para1;
      mouse_down_y = event->para2;
      mouse_pc_x = mouse_x = mouse_down_x;
      mouse_pc_y = mouse_y = mouse_down_y;
      mouse_upd = 1;
      break;
      
    case PEN_MOVE:
      mouse_is_down = 1;
      mouse_pc_x = mouse_x = event->para1;
      mouse_pc_y = mouse_y = event->para2;
      mouse_upd = 1;
      break;
      
    case PEN_UP:
      mouse_is_down = 0;
      mouse_pc_x = mouse_x = event->para1;
      mouse_pc_y = mouse_y = event->para2;
      mouse_upd = 1;
      break;
    }
    if (!eol) return TRUE;

  case EVT_INLAY_SELECT:
    switch(event->para1) {
    case INLAY_EXIT:
    case INLAY_MAIN_MENU:
      hard_exit=1;
      return TRUE;
    case INLAY_OK:
      if (!eol) dev_pushkey('\n');
      return TRUE;
    }
    break;
  
  default:
    return FALSE;
  }
  
  return FALSE;
}


void MessageBox(char *title, char *msg, int fatal)
{
  DoYesNo(title,msg,-1);
  if (fatal)
    longjmp(env,1);
}

char g_file[32];
void sbasicmain(char *fname)
{
  strcpy(g_file, fname);

  FntClearScreen();
  ready = 0;
  osd_devinit();
  ready = 1;
  buttcover=0;

  memmgr_init();
  
  brun(g_file, 0);

  my_cleanup();
}

int go, doneiface;

void EventLoop(void)
{
  EvtType event;

  doneiface=0;
  go=0;
  while(!doneiface) {
    EvtGetEvent(&event);
    if(SystemHandleEvent(&event)) continue;
    if(MenuHandleEvent(&event)) continue;
    if(ApplicationHandleEvent(&event)) continue;
    FormDispatchEvent(&event);
    if(event.eventType==EVT_APP_STOP) longjmp(env, 1);
  }
}

BOOLEAN FormIfaceHandleEvent(EvtType* event)
{
  SHORT item;
  BYTE *ptr;
  extern char *CheckResumeFile();

  switch(event->eventType) {
  case EVT_FORM_OPEN:
    ptr=CheckResumeFile();
    if (ptr) {
      DoEdit(ptr);
    } else {
      FormSetFormActiveObject(FORM_IFACE, LIST_IFACE);
      FormObjectSetFocus(LIST_IFACE);
      ListDeleteAllItems(LIST_IFACE);
      ListAllDBs("BASIC", LIST_IFACE);
      ListSetSelectedItem(LIST_IFACE, NO_SELECTION);
      FormDrawForm(event->eventID);
    }
    break;

  case EVT_LIST_SELECT:
    ListSetSelectedItem(event->eventID, event->para1);
    ListSetHighlightedItem(event->eventID, event->para1);
    ListDrawList(event->eventID);
    return TRUE;
		
  case EVT_MENU_SELECT_ITEM:
    if (event->para1==MENU_ABOUT) FormPopupForm(FORM_ABOUT);
    else if (event->para1==MENU_FONT) DoFont();
    else if (event->para1==MENU_ALLDBS) {
      ListDeleteAllItems(LIST_IFACE);
      ListAllDBs(NULL, LIST_IFACE);
      ListSetSelectedItem(LIST_IFACE, NO_SELECTION);
      ListDrawList(LIST_IFACE);
    } else if (event->para1==MENU_DELETE) {
      char buff[64];
      ListGetTotalItems(LIST_IFACE, &item);
      if (item==0) break;
      ListGetSelectedItem(LIST_IFACE, &item);
      if (item == NO_SELECTION) return TRUE;
      ListGetListItem(LIST_IFACE, item, (BYTE **)&ptr);
      sprintf(buff, "Really Delete '%s'?", ptr);
      if (DoYesNo("Delete Warning", buff, FORM_IFACE))
	unlink(ptr);
    }
    break;

  case EVT_INLAY_SELECT:
    switch(event->para1) {
    case INLAY_EXIT:
    case INLAY_MAIN_MENU:
      doneiface=1;
      go=0;
      break;
    case INLAY_OK:
      EvtAppendEvt(EVT_CONTROL_SELECT, RUN_IFACE, 0, 0, NULL);
      break;
    default:
      return FALSE;
    }
    break;
		
  case EVT_CONTROL_SELECT:
    if (event->eventID==NEW_IFACE) {
      g_file[0] = 0;
      if (DoNew(g_file)) {
	DoEdit(g_file);
	return TRUE;
      } else
	FormPopupForm(FORM_IFACE);
    }
    ListGetTotalItems(LIST_IFACE, &item);
    if (item==0) break;
    ListGetSelectedItem(LIST_IFACE, &item);
    if (item == NO_SELECTION) return TRUE;
    ListGetListItem(LIST_IFACE, item, (BYTE **)&ptr);
    switch (event->eventID) {
    case RUN_IFACE:
      strcpy(g_file, ptr);
      doneiface=1;
      go = 1;
      break;
    case EDIT_IFACE:
      DoEdit(ptr);
      break;
    }
    break;
		
  default:
    return FALSE;
  }
  return TRUE;
}

void FormGotoApp(BYTE* app_name)
{
  ObjectID objid;
  FormGetActiveFormID(&objid);
  EvtAppendEvt(EVT_FORM_CLOSE, objid, 0, 0, NULL);
  SysGetAppID(app_name,&objid);
  EvtAppendEvt(EVT_APP_STOP,0,0,0,NULL);
  EvtAppendEvt(EVT_APP_LAUNCH,objid,0,0,NULL);
}

int __main(WORD cmd, void *cmd_arg)
{
  switch(cmd) {
  case LAUNCH_CMD_NORMAL_LAUNCH:
    UIApplicationInit();		//initialize UI resources
    if (setjmp(env)==0) {
      do {
	FormPopupForm(FORM_IFACE);
	EventLoop();			//run main event loop
	if (go) {
	  sbasicmain(g_file);
	  PauseForKey();
	}
      } while(go);
    }
    my_cleanup();
    FormGotoApp("Mainmenu");
    UIDeleteAllAppObjects();	        //main event loop has stopped, cleanup
    break;
  case LAUNCH_CMD_GOTO_REC:
    if(((GotoRec*)cmd_arg)->find_string)
      pfree(((GotoRec*)cmd_arg)->find_string);
  case LAUNCH_CMD_FIND:
  case LAUNCH_CMD_ALARM_HIT:
    pfree(cmd_arg);
    break;
  default:
    return FALSE;
  }
  return TRUE;
}
