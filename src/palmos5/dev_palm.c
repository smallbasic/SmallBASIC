/*
*	low-level device driver for PALM OS
*
*	ndc: 2001-02-13
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include "sys.h"
#include "device.h"
#include "osd.h"
#include "pmem.h"
#include "pproc.h"
#include "smbas.h"

#include <PenMgr.h>


#define sysVersion35    sysMakeROMVersion(3,5,0,sysROMStageRelease,0)

int	BasicInputBox(char *title, char *dest, int size, int frmid); 	// from sbpad

static int	pdev_prev_keymask;
static word	pdev_keydelay[3];
static byte pdev_keyq;
static dword	TPS;

static dbt_t audio_queue;
static int	audio_qhead;
static int	audio_qtail;
struct audio_node_s	{
	word	frq;
	word	dur;
	byte	vol;
	dword	start;
	dword	end;
	byte	status;
	};
typedef struct audio_node_s audio_node;
#define	AUDIO_QSIZE		256


static UInt16   saved_coordinates_system;
static dword    wmVersion;



static int	cur_y = 0;		// current y position
static int	cur_x = 0;		// current x position

int        maxline = (160/11);	// max. console line  ((160-15)/11 = 13.18 = 0..12)
static int useinvert = 0;	// use invert routines
static int tabsize = 32;	// tab size in pixels (160/32 = 5)
int	   font_h = 11;		// font height (needed not-static for Sony's bug)

// PEN STATUS (used with do_events/PEN() function)
static int16	pen_x;
static int16	pen_y;
static int16	pen_down_x;
static int16	pen_down_y;
static int16	pen_update;
static byte	pen_down = 0;	// boolean
static int16	pen_state = 0;

static byte*	video_base = NULL;		// Video RAM pointer

static int	vga35[16];				// PalmOS 3.5 color-index value


/* ---- Set the system in graphic coordinates system */
static void SetCoordGraph()
{
if (wmVersion >= 4)
  WinSetCoordinateSystem(kCoordinatesNative);
}

/* ---- Restore the system to it's normal coordinates system */
static void SetCoordOrg()
{
if (wmVersion >= 4)
  WinSetCoordinateSystem(saved_coordinates_system);
}


/*
*	PalmOS device initialization
*/
int		osd_devinit()
{
	dword	romVersion;
        Coord   wmx, wmy;



	/* OS Version INFO */
        FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
        FtrGet(sysFtrCreator, sysFtrNumWinVersion, &wmVersion);       
        
        
	os_ver = (sysGetROMVerMajor(romVersion) << 8) | (sysGetROMVerMinor(romVersion) << 4) | sysGetROMVerFix(romVersion);
	// ---- TODO: stop if not at least Palm OS 3.5
        
        /* System clock init */
        TPS = SysTicksPerSecond();


        /* Audio init */
	audio_queue = dbt_create("SBI-AUDIO", 0);
	dbt_prealloc(audio_queue, AUDIO_QSIZE, sizeof(audio_node));
	audio_qhead = audio_qtail = 0;

	/* initialize keyboard (hardware keys) */
	pdev_prev_keymask = KeySetMask(0x7E);
	KeyRates(0, &pdev_keydelay[0], &pdev_keydelay[1], &pdev_keydelay[2], &pdev_keyq);


	/* initialize colors */
        os_color_depth = 1;  // Assume no color
        os_color       = 0;
    
        
        if ( romVersion >= sysVersion35 )  {

                dword	sup_depth;
                dword   sup_mx, sup_my;
                Boolean sup_color;
                int     i;


                // ---- if modern device, maybe there are colors
                // so get highest supported depth and resolution
                // TODO: remove virtual Graffiti zone if any
                ScrDisplayMode(scrDisplayModeGet, &sup_mx, &sup_my, NULL, &sup_color);
                ScrDisplayMode(scrDisplayModeGetSupportedDepths, NULL, NULL, &sup_depth, NULL);

                for(i=15; (sup_depth & (1 << i)) == 0; i--);
                os_color_depth = i+1;
                os_color = sup_color;

                // ---- If we have a possibly high density system, let's get the maximum size

                if (wmVersion >= 4) {

                        // ---- Get actual pixel size of the screen
                        WinScreenGetAttribute(winScreenWidth, &sup_mx);
                        WinScreenGetAttribute(winScreenHeight, &sup_my);

                        // ---- Use the screen without any scaling (hi-res)
                        saved_coordinates_system = WinGetCoordinateSystem();
                        WinSetCoordinateSystem(kCoordinatesNative);

                }

                // ---- Set the screen in the requested mode
                ScrDisplayMode(winScreenModeSet, &sup_mx, &sup_my, &os_color_depth, &sup_color);


        }
        
        // ---- Gets final display size
	WinGetDisplayExtent(&wmx, &wmy);
	os_graf_mx = wmx;
	os_graf_my = wmy;

	///////// No direct access because there is to much variety within palmOS devices ///////////////////////////////
	
	video_base = NULL;
	setsysvar_int(SYSVAR_VIDADR, (int32) video_base);

	
        // ---- if the device is a color one, set up the VGA like palette with RGB values for the 16 first indexes
	if ( os_color && os_color_depth >= 4 )	{

		int		color;
		RGBColorType	rgb;

		for ( color = 0; color < 16; color ++ )	{
			int		lo, hi;

			MemSet(&rgb, sizeof(RGBColorType), 0);

			if	( color == 15 )
				rgb.r = rgb.g = rgb.b = 0xFF;
			else if ( color != 0 )	{
				if	( color > 8 )
					( hi = 1, lo = color & 0x7 );
				else
					( hi = 0, lo = color );

				if	( color == 8 )
					rgb.r = rgb.g = rgb.b = 0x55;
				else if	( color == 7 )
					rgb.r = rgb.g = rgb.b = 0xAA;
				else	{
				if	( lo == CLR_BLUE || lo == CLR_CYAN || lo == CLR_MAGENTA || lo == CLR_BLACK )
					rgb.b = (hi) ? 0xFF : 0x7F;
				if	( lo == CLR_GREEN || lo == CLR_CYAN || lo == CLR_BROWN || lo == CLR_BLACK )
						rgb.g = (hi) ? 0xFF : 0x7F;
					if	( lo == CLR_RED || lo == CLR_MAGENTA || lo == CLR_BROWN || lo == CLR_BLACK )
						rgb.r = (hi) ? 0xFF : 0x7F;
					}
				}

			vga35[color] = WinRGBToIndex(&rgb);

			WinSetForeColor(vga35[color]);
			vga35[color] = WinRGBToIndex(&rgb);

		}

        }



        // ---- Setup base fonts
        // TODO: add a call to a function that setups a font using an SB semantic
	FntSetFont(stdFont);
	font_h = FntLineHeight();
	maxline = os_graf_my/font_h;
        
        // ---- Internal inits
	pen_state     = 0;
	pen_update    = 0;
	cur_x = cur_y = 0;

	osd_setcolor(0);
	
	dev_clrkb();
	dev_cls();
        SetCoordOrg();
	return 1;
}

/*
*	PalmOS device - restore to defaults
*/
int		osd_devrestore()
{
        /* restore screen */
        SetCoordOrg();

	FntSetFont(stdFont);
	osd_settextcolor(0, 15);
	font_h = FntLineHeight();
	maxline = os_graf_my/font_h;
	osd_setcolor(0);
	cur_x = cur_y = 0;

        /* restore audio */
	audio_qhead = audio_qtail = 0;
	dbt_close(audio_queue);

	/* restore keyboard */
	KeySetMask(pdev_prev_keymask);
	KeyRates(-1, &pdev_keydelay[0], &pdev_keydelay[1], &pdev_keydelay[2], &pdev_keyq);
	os_color = 0;



	return 1;

}

/*
*/
void	osd_realsound(int frq, int ms, int vol) SEC(BIO);
void	osd_realsound(int frq, int ms, int vol)
{
	SndCommandType	beepPB;

	beepPB.cmd = sndCmdFrqOn;
	beepPB.param1 = frq;
	beepPB.param2 = ms;	// in ms
	beepPB.param3 = (sndMaxAmp * vol) / 100;
	SndDoCmd(NULL, &beepPB, 1);
}

/*
*/
void	osd_backsound(dword dif)	SEC(BIO);
void	osd_backsound(dword dif)
{
	audio_node	node;
	dword		now;

	if	( audio_qhead != audio_qtail )	{
		dbt_read(audio_queue, audio_qhead, &node, sizeof(audio_node));

		if	( node.status == 1 )	{	// I am playing
			now = TimGetTicks();
			if	( now >= node.end )	{
				osd_realsound(0, 0, 0);	// stop

				audio_qhead ++;
				if	( audio_qhead >= AUDIO_QSIZE )
					audio_qhead = 0;

				if	( now > node.end )
					osd_backsound(now - node.end);	// read next NOW
				else
					osd_backsound(0);	// read next NOW
				}
			}
		else	{	// next cmd
			node.start = TimGetTicks() + dif;
			node.end = node.start + ((node.dur * TPS) / 1000);

			if	( node.frq )
				osd_realsound(node.frq, node.dur, node.vol);	// start play
			else
				osd_realsound(0, 0, 0);	// stop

			node.status = 1;
			dbt_write(audio_queue, audio_qhead, &node, sizeof(audio_node));
			}
		}
}

/*
*	events
*/
int		BRUNHandleEvent(EventPtr e);
Boolean ApplicationHandleEvent(EventPtr e);

/*
*/
int		osd_catch(EventType *ev)	SEC(TRASH);
int		osd_catch(EventType *ev)
{
	dword	state;
	int		r = 0;
        Int16   evt_x, evt_y;
        Boolean evt_pd;

	if	( ev->eType == penUpEvent )	
		pen_down = 0;
	else if	( ev->eType == penMoveEvent || ev->eType == penDownEvent )	{
	
        
                evt_x  = ev->screenX;
                evt_y  = ev->screenY;
                evt_pd = ev->penDown;
                
		/* ---- If high density, use native coordinates */
		if (wmVersion >= 4) 		
			EvtGetPenNative(WinGetDrawWindow(), &evt_x, &evt_y, &evt_pd);
		
		
		
	
		if	( evt_y <= os_graf_my && evt_x <= os_graf_mx )	{
			pen_x = evt_x;
			pen_y = evt_y;

			if	( (ev->eType == penDownEvent) && (pen_down == 0) )	{
				pen_down_x = pen_x;
				pen_down_y = pen_y;
				pen_down = 1;
			}

			pen_update = pen_state;
		}
	}

	if	( ev->eType == keyDownEvent )	{

		state = KeyCurrentState();
		if	( state & keyBitHard4 )	{	// BREAK
			pen_state = 0;					
			return -2;
			}
		if	( state & keyBitHard1 )	{
			dev_pushkey(SB_KEY_PALM_BTN1);
			return 1;
			}
		if	( state & keyBitHard2 )	{
			dev_pushkey(SB_KEY_PALM_BTN2);
			return 1;
			}
		if	( state & keyBitHard3 )	{
			dev_pushkey(SB_KEY_PALM_BTN3);
			return 1;
			}
		if	( state & keyBitPower )	{
			// Power ON/OFF
			return 0;	// let the system to check it
			}


		if ( ev->data.keyDown.modifiers & commandKeyMask )	{
			switch ( ev->data.keyDown.chr )	{
			case	pageUpChr:
				dev_pushkey(SB_KEY_PGUP);
				r ++;
				return r;
			case	pageDownChr:
				dev_pushkey(SB_KEY_PGDN);
				r ++;
				return r;
			case vchrLaunch:
				pen_state = 0;					
				EvtAddEventToQueue(ev);
				return -2;
			case vchrPowerOff:
			case vchrAlarm:                       // back to for alarm events
			case vchrMenu:
				return 0;
			case vchrLowBattery:
//				if	( os_ver >= 0x350 )
//					inf_low_battery();
				return 0;
			case vchrFind:
				dev_pushkey(SB_KEY_PALM_FIND);
				r ++;
				return r;
			case vchrCalc:
				dev_pushkey(SB_KEY_PALM_CALC);
				r ++;
				return r;
			case vchrKeyboard:
			case vchrKeyboardAlpha:
			case vchrKeyboardNumeric:				// popup keyboard
				{
					char	*tmp;

					tmp = tmp_alloc(256);

					EvtAddEventToQueue(ev);				// resend popup-keyboard event
					if	( BasicInputBox("Keyboard buffer (255c)", tmp, 255, 1) )	{
						char	*p;

						p = tmp;
						while ( *p )	{
							dev_pushkey(*p);
							r ++; p ++;
							}
						}
					tmp_free(tmp);
				}
				return r;
			case vchrGraffitiReference:           // popup the Graffiti reference
				break;
                }
			}
		else	{
			if	( (word) ev->data.keyDown.chr >= 32 )	
				dev_pushkey(ev->data.keyDown.chr);
	
			if	( ev->data.keyDown.chr == '\b' || ev->data.keyDown.chr == '\t' || ev->data.keyDown.chr == '\n' )	
				dev_pushkey(ev->data.keyDown.chr);

			r ++;
			return r;
			}
		}

	return r;
}

/*
*	system event-loop
*	returns -1,-2 if stop rq.
*	returns the number of events
*/
int		osd_events()
{
	EventType	ev;
	Err			err;
	int			retcode = 0;

	osd_backsound(0);
	EvtGetEvent(&ev, 0);

	if	( ev.eType != nilEvent )	{	
		retcode ++;
		if	( (retcode = osd_catch(&ev)) )	
			return retcode;

		if	( !SysHandleEvent(&ev) )	{
			if	( !MenuHandleEvent(NULL, &ev, &err) )	{
				if	( !ApplicationHandleEvent(&ev) )	{
					if	( BRUNHandleEvent(&ev) )	
						return -1;
					} // app
				} // menu
			} // sys
		}

	return retcode;
}

//	returns the current position in pixels
int		osd_getx()	{ return cur_x; }
int		osd_gety()	{ return cur_y; }

// ---- returns the current value of a given pixel
long	osd_getpixel(int x, int y)
{
long ret;
SetCoordGraph();
ret = WinGetPixel(x, y);
SetCoordOrg();

return ret;
}

//

/*
*	enable or disable PEN code
*/
void	osd_setpenmode(int enable)
{
pen_state = enable ? 2 : 0;
}

/*
*	returns the pen events
*	(see device.h)
*/
int		osd_getpen(int code)
{
	int		r = 0;

	if	( pen_state == 0 )
		return 0;

	switch ( code )	{
	case 	0:	// bool: status changed
		if	( pen_update == 0 )	{
			r = osd_events();
			if	( r < 0 )	{
				brun_break();
				return 0;
				}
			}
		if ( pen_update && pen_state == 2 )	{	// after PEN ON
			pen_down_x = pen_x;
			pen_down_y = pen_y;
			pen_state = 1;
			}
		r = pen_update;
		break;		
	case	1:	// last pen-down x
		r = pen_down_x;
		break;		
	case	2:	// last pen-down y
		r = pen_down_y;
		break;		
	case	3:	// vert. 1 = down, 0 = up
		r = pen_down;
		break;
	case	4:	// last x
		r = pen_x;
		break;
	case	5:	// last y
		r = pen_y;
		break;
		}
	pen_update = 0;
	return	r;
}

/*
*	Clear Screen
*/
void	osd_cls()
{
	RectangleType	r;

        SetCoordGraph();
        
	r.topLeft.x = 0;
	r.topLeft.y = 0; 
	r.extent.x = os_graf_mx;
	r.extent.y = os_graf_my;

	WinEraseRectangle(&r, 0);

	cur_x = cur_y = 0;
	useinvert = 0;

	FntSetFont(stdFont);
	WinSetUnderlineMode(noUnderline);
        
        SetCoordOrg();
}

/*
*	set text x,y (in pixels)
*/
void	osd_setxy(int x, int y)
{
	cur_y = y;
	cur_x = x;
}

/*
*	osd_write: next line
*/
void	osd_nextln() SEC(BIO);
void	osd_nextln()
{
	RectangleType	r;
	int				dy;

        
        SetCoordGraph();
        
	cur_x = 0;

	if	( cur_y < ((maxline-1) * font_h) )	{
		cur_y += font_h;
		}
	else	{
		// scroll
		dy = font_h;
		r.topLeft.x = 0;
		r.topLeft.y = 0;
		r.extent.x = os_graf_mx;
		r.extent.y = os_graf_my;

		WinScrollRectangle(&r, winUp, dy, &r);

		cur_y = (maxline-1) * font_h;

		r.topLeft.x = 0;
		r.topLeft.y = cur_y;
		r.extent.x = os_graf_mx;
		r.extent.y = os_graf_my - cur_y;

		WinEraseRectangle(&r, 0);
		}
                
        SetCoordOrg();
}

/*
*	osd_write: calc next tab position
*/
int		osd_calctab(int x) SEC(BIO);
int		osd_calctab(int x)
{
	int		c = 1;

	while ( x > tabsize )	{
		x -= tabsize;
		c ++;
		}
	return c * tabsize;
}

/*
*	osd_write: basic output
*
*	Supported control codes:
*	\t		tab (20 px)
*	\a		beep
*	\r		return
*	\n		next line
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
	int		len, cx, esc_val, esc_cmd;
	byte	*p, buf[3];
	int		ch, char_len = 1, mbf;
	RectangleType	r;

        
        SetCoordGraph();
        
	len = strlen(str);

	if	( len <= 0 )
		return;

	p = (byte *) str;
	while ( *p )	{
		switch ( *p )	{
		case	'\a':	// beep
			osd_beep();
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

				while ( is_digit(*p) )	{
					esc_val = (esc_val * 10) + (((int) *p) - 48);
					p ++;
					}

				esc_cmd = *p;

				// control characters
				switch ( esc_cmd )	{
				case	'K':			// \e[K - clear to eol
					r.topLeft.x = cur_x;
					r.topLeft.y = cur_y;
					r.extent.x = os_graf_mx - cur_x;
					r.extent.y = font_h;

					WinEraseRectangle(&r, 0);
					break;
				case	'G':
					cur_x = esc_val * osd_textwidth("0");
					break;
				case	'm':			// \e[...m	- ANSI terminal
					switch ( esc_val )	{
					case	0:	// reset
						WinSetUnderlineMode(noUnderline);
						useinvert = 0;
						osd_setcolor(0);
						osd_settextcolor(0, 15);
						break;
					case	1:	// set bold on
						FntSetFont(boldFont);
						break;
					case	4:	// set underline on
						WinSetUnderlineMode(solidUnderline);
						break;
					case	7:	// reverse video on
						useinvert = 1;
						break;
					case	21:	// set bold off
						FntSetFont(stdFont);
						break;
					case	24:	// set underline off
						WinSetUnderlineMode(noUnderline);
						break;
					case	27:	// reverse video off
						useinvert = 0;
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

					case	40:	// set black bg
						osd_settextcolor(dev_fgcolor, 0);
						break;
					case	41:	// set red bg
						osd_settextcolor(dev_fgcolor, 4);
						break;
					case	42:	// set green bg
						osd_settextcolor(dev_fgcolor, 2);
						break;
					case	43:	// set brown bg
						osd_settextcolor(dev_fgcolor, 6);
						break;
					case	44:	// set blue bg
						osd_settextcolor(dev_fgcolor, 1);
						break;
					case	45:	// set magenta bg
						osd_settextcolor(dev_fgcolor, 5);
						break;
					case	46:	// set cyan bg
						osd_settextcolor(dev_fgcolor, 3);
						break;
					case	47:	// set white bg
						osd_settextcolor(dev_fgcolor, 7);
						break;

					// extra palmos (tiny fonts)
					case	70:	// select font 1	std
					case	71:	// select font 2	bold
					case	72:	// select font 3	large
					case	73:	// select font 4	symbol
					case	74:	// select font 5	symbol 11
					case	75:	// select font 6	symbol 7
					case	76:	// select font 7	led
					case	77:	// select font 8	large bold
						break;

					// extra palmos
					case	80:	// select font 1	std
					case	81:	// select font 2	bold
					case	82:	// select font 3	large
					case	83:	// select font 4	symbol
					case	84:	// select font 5	symbol 11
					case	85:	// select font 6	symbol 7
					case	86:	// select font 7	led
					case	87:	// select font 8	large bold
						FntSetFont(esc_val-80);
						font_h = FntCharHeight();
						maxline = os_graf_my/font_h;
						break;
					case	90:	// select custon font 1
					case	91:	// select custom font 2
					case	92:	// select custom font 3
					case	93:	// select custom font 4
						if	( os_charset == enc_utf8 )	{
							FntSetFont(128+(esc_val-90));
							font_h = FntLineHeight();
							maxline = os_graf_my/font_h;
							}
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
			r.topLeft.x = 0;
			r.topLeft.y = cur_y;
			r.extent.x = os_graf_mx;
			r.extent.y = font_h; 

			WinEraseRectangle(&r, 0);
			cur_x = 0;
			break;
		default:
			//
			//	PRINT THE CHARACTER
			//
			buf[0] = *p;
			mbf = 0;

			switch ( os_charset )	{
			case	enc_sjis:				// Japan
				if ( IsJIS1Font(*p) )	{
					ch = (*p << 8) + *(p + 1);
					mbf = IsJISFont(ch);
					}
				break;

			case	enc_big5:				// TW
				if ( IsBig51Font(*p) )	{
					ch = (*p << 8) + *(p + 1);
					mbf = IsBig5Font(ch);
					}
				break;

			case	enc_gmb:				// Generic multibyte
				if ( IsGMB1Font(*p) )	{
					ch = (*p << 8) + *(p + 1);
					mbf = IsGMBFont(ch);
					}
				break;

			case	enc_unicode: 			// Unicode
				mbf = 1;
				ch = (*p << 8) + *(p + 1);
				break;
				}

			if	( mbf )	{	// its a multibyte char
				p ++;	// skip one character (needed for the loop)
				cx = FntCharsWidth((byte *) &ch, 2);
				buf[1] = *p;
				buf[2] = '\0';
				char_len = 2;
				}
			else	{
				cx = FntCharWidth(*p);
				buf[1] = '\0';
				char_len = 1;
				}

			// new line ?
			if	( cur_x + cx >= /* dev_Vx2 */ os_graf_mx )
				osd_nextln();

			// draw

			if	( !useinvert )	
				WinDrawChars(buf, char_len, cur_x, cur_y);
			else	
				WinDrawInvertedChars(buf, char_len, cur_x, cur_y);


			// advance
			cur_x += cx;
			};

		if	( *p == '\0' )
			break;

		p ++;
		}
                
                
        SetCoordOrg();       
}

/*
*/
void	osd_spec_setcolor(long color, long bg) SEC(BIO);
void	osd_spec_setcolor(long color, long bg)
{
	RGBColorType	rgb;

	if	( os_color_depth <= 8 )	{
		if	( color > 15 )	color = 15;
		if	( color < 0 )	color = 0;
		}

	if	( bg )
		dev_bgcolor = color;
	else	{
		dev_fgcolor = color;
		}

	MemSet(&rgb, sizeof(rgb), 0);
	switch ( color )	{
	case	0:
		rgb.r = rgb.g = rgb.b = 0;
		if	( bg )
			WinSetColors(0, 0, &rgb, 0);
		else
			WinSetColors(&rgb, 0, 0, 0);
		break;
	case	15:
		rgb.r = rgb.g = rgb.b = 0xFF;

		if	( bg )	
			WinSetColors(0, 0, &rgb, 0);
		else
			WinSetColors(&rgb, 0, 0, 0);
		break;
	default:
		if	( os_color_depth == 2 )	{			// PalmOS ver 3.1+
			// 2bit GRAY
			if	( color < 7 || color == 8 )
				{ rgb.r = rgb.g = rgb.b = 0x55; }
			else
				{ rgb.r = rgb.g = rgb.b = 0xAA; }

			if	( bg )
				WinSetColors(0, 0, &rgb, 0);
			else
				WinSetColors(&rgb, 0, 0, 0);
			}
		else if	( os_color_depth == 4 )	{		// PalmOS ver 3.3+
			// 4bit GRAY
			if	( color == 8 )
				rgb.r = rgb.g = rgb.b = 119;
			else if	( color == 7 )
				rgb.r = rgb.g = rgb.b = 136;
			else
				rgb.r = rgb.g = rgb.b = color * 255/15;

			if	( bg )
				WinSetColors(0, 0, &rgb, 0);
			else
				WinSetColors(&rgb, 0, 0, 0);
			}
		else  if ( os_color_depth == 8 )	{		// PalmOS ver 3.5+
			if	( bg )
				WinSetBackColor(vga35[color]);
			else	{
				WinSetTextColor(vga35[color]);
				WinSetForeColor(vga35[color]);
				}
			}
                else {
			color = -color;
                        rgb.r = (color >> 16) & 0xFF; rgb.g = (color >> 8) & 0xFF; rgb.b = color & 0xFF;
                        if	( bg )
				WinSetColors(0, 0, &rgb, 0);
			else
				WinSetColors(&rgb, 0, 0, 0);

                       }

		};

}

/*
*	set current color
*/
void	osd_setcolor(long color)
{
	osd_spec_setcolor(color, 0);
}

/*
*	set text color (fg used for graphics too)
*/
void	osd_settextcolor(long fg, long bg)
{
	osd_spec_setcolor(fg, 0);
	if	( bg != -1 )
		osd_spec_setcolor(bg, 1);
}



// ---- Draws a line between two points
void	osd_line(int X1, int Y1, int X2, int Y2)
{
SetCoordGraph();
WinDrawLine(X1, Y1, X2, Y2);
SetCoordOrg();
}



/*
*	draw pixel
*/
void	osd_setpixel(int x, int y)
{
SetCoordGraph();
WinDrawPixel(x, y);
SetCoordOrg();
}

/*
*	draw rect
*/
void	osd_rect(int x1, int y1, int x2, int y2, int fill)
{
RectangleType	r;

SetCoordGraph();

r.topLeft.x = x1;
r.topLeft.y = y1;
r.extent.x = (x2 - x1)+1;
r.extent.y = (y2 - y1)+1;

if (fill)
        WinDrawRectangle(&r, 0);
else
        WinDrawRectangleFrame(rectangleFrame, &r);

SetCoordOrg();
}

/*
*	BEEP :)
*/
void	osd_beep()
{
	SndCommandType	beepPB;
	int				vol;

	vol = PrefGetPreference(prefSysSoundVolume);
	if	( vol )	{
		beepPB.cmd = sndCmdFreqDurationAmp;
		beepPB.param1 = 440;
		beepPB.param2 = 125;	// in ms
		beepPB.param3 = vol;
		SndDoCmd(NULL, &beepPB, true);	// true=nowait
		}
}

/*
*	play a tone.
*/
void	osd_sound(int frq, int ms, int vol, int bgplay)
{
	if	( bgplay )	{
		audio_node	node;

		node.status = 0;
		node.frq = frq;
		node.dur = ms;
		node.vol = vol;
		dbt_write(audio_queue, audio_qtail, &node, sizeof(audio_node));
		
		audio_qtail ++;
		if ( audio_qtail >= AUDIO_QSIZE )
			audio_qtail = 0;
		}
	else	{
		if	( frq )	{
			SndCommandType			beepPB;

			beepPB.cmd = sndCmdFreqDurationAmp;
			beepPB.param1 = frq;
			beepPB.param2 = ms;	// in ms
			beepPB.param3 = (sndMaxAmp * vol) / 100;
			SndDoCmd(NULL, &beepPB, 0);	// true=nowait
			}
		else	{
			dev_delay(ms);
			}
		}
}

/*
*/
void	osd_clear_sound_queue()
{
	audio_qhead = audio_qtail;
}

int		osd_textwidth(const char *str)
{
int ret;
SetCoordGraph();    
ret = FntCharsWidth(str, strlen(str));
SetCoordOrg();
return ret;
}

int		osd_textheight(const char *str)
{
	// TODO: count \n
	return font_h;
}

void	osd_refresh()
{
}
