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

#define sysVersion30    sysMakeROMVersion(3,0,0,sysROMStageRelease,0)
#define sysVersion31    sysMakeROMVersion(3,1,0,sysROMStageRelease,0)
#define sysVersion35    sysMakeROMVersion(3,5,0,sysROMStageRelease,0)
#define sysVersion40    sysMakeROMVersion(4,0,0,sysROMStageRelease,0)

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

static int	cur_y = 0;		// current y position
static int	cur_x = 0;		// current x position

int			maxline = (160/11);	// max. console line  ((160-15)/11 = 13.18 = 0..12)
static int useinvert = 0;	// use invert routines 
static int tabsize = 32;	// tab size in pixels (160/32 = 5)
int			font_h = 11;		// font height (needed not-static for Sony's bug)

// PEN STATUS (used with do_events/PEN() function)
static int16	pen_x;
static int16	pen_y;
static int16	pen_down_x;
static int16	pen_down_y;
static int16	pen_update;
static byte		pen_down = 0;	// boolean
static int16	pen_state = 0;

static byte*	video_base = NULL;		// Video RAM pointer

static int		vga35[16];				// PalmOS 3.5 color-index value
static int		vga35tra[16];			// Real color value

// direct access 
static void		(*directsetpixel)(int,int);
static long		dcolor;

#if	PALMOS_SDK_VERSION < 0x0500
static void	palm_directsetpixel_1(int x, int y);
static void	palm_directsetpixel_2(int x, int y);
static void	palm_directsetpixel_4(int x, int y);
static void	palm_directsetpixel_8(int x, int y);
#if defined(SONY_CLIE)
static void	palm_sony_directsetpixel_8(int x, int y);
#endif
static void	palm_directsetpixel_16(int x, int y);
static void	palm_directsetpixel_24(int x, int y);
static void	palm_directsetpixel_32(int x, int y);
#endif
static long	palm_directcolor(long vgacolor);
static void	palm_sdk_setpixel(int x, int y);
static long	palm_sdk_getpixel(int x, int y);

/*
*	PalmOS device initialization
*/
int		osd_devinit()
{
	dword	romVersion;
	dword	sup_depth;
//	word	keydelay[3];
//	byte	keyq;
	word	wmx, wmy;
	#if defined(SONY_CLIE)
	UInt32	width, height;
	#endif

	/* OS INFO */
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if	( romVersion >= sysVersion35 )	
		os_ver = (sysGetROMVerMajor(romVersion) << 8) | (sysGetROMVerMinor(romVersion) << 4) | sysGetROMVerFix(romVersion);
	else if	( romVersion > sysVersion31 )
		os_ver = 0x330;
	else if	( romVersion == sysVersion31 )
		os_ver = 0x310;
	else
		os_ver = 0x300;

	/* initialize keyboard (hardware keys) */
	pdev_prev_keymask = KeySetMask(0x7F);
	KeyRates(0, &pdev_keydelay[0], &pdev_keydelay[1], &pdev_keydelay[2], &pdev_keyq);
//	keydelay[0] = keydelay[1] = keydelay[2] = 0; keyq = 0;
//	KeyRates(-1, &keydelay[0], &keydelay[1], &keydelay[2], &keyq);

	// PalmOS 3.5+, use SDK to access screen
	#if	PALMOS_SDK_VERSION < 0x0500
    if ( os_ver >= 0x350 ) 
		opt_safedraw = 1;
	else // otherwise (PalmOS < 3.5) use direct access
		opt_safedraw = 0;
	#else
	opt_safedraw = 1;
	#endif

	/* initialize colors */
    if ( os_ver < 0x310 ) {
        os_color_depth = 1;  // no system support for gray or colors.
        os_color = 0;
        }
    else {
		ScrDisplayMode(scrDisplayModeGetSupportedDepths, NULL, NULL, &sup_depth, NULL);
		if      (sup_depth & 0x8000) os_color_depth = 16;
		else if (sup_depth & 0x0080) os_color_depth = 8;
		else if (sup_depth & 0x0008) os_color_depth = 4;
		else if (sup_depth & 0x0002) os_color_depth = 2;
		else                         os_color_depth = 1;
	#if defined(SONY_CLIE)
		if	( use_sony_clie )	{
			width = hrWidth;
			height = hrHeight;
//			width = opt_pref_width;
//			height = opt_pref_height;
			HRWinScreenMode(sony_refHR, winScreenModeSet, &width, &height, &os_color_depth, NULL);
//			HRWinScreenMode(sony_refHR, winScreenModeSet, &width, &height, &opt_pref_bpp, NULL);
		        HRWinScreenMode(sony_refHR, winScreenModeGet, &width, &height, &sup_depth, NULL);			}
		else
		{
	#endif
                WinScreenMode(winScreenModeSet, 0, 0, &os_color_depth, 0);
                os_color = (os_color_depth>=8);
	#if defined(SONY_CLIE)
		}
	#endif
        }

	///////// direct access ///////////////////////////////
	#if defined(SONY_CLIE)
	if	( use_sony_clie )		{
		video_base = NULL; // WinGetDrawWindow()->displayAddrV20;
		directsetpixel = palm_sony_directsetpixel_8;
		setsysvar_int(SYSVAR_VIDADR, (int32) WinGetDrawWindow()->displayAddrV20);
		}
	else
		{
	#endif
	#if	PALMOS_SDK_VERSION < 0x0500
	if	( !opt_safedraw )	{
		video_base = (byte *) WinGetDrawWindow()->displayAddrV20;
		setsysvar_int(SYSVAR_VIDADR, (int32) video_base);

		switch ( os_color_depth )	{
		case	1:
			directsetpixel = palm_directsetpixel_1;
			break;
		case	2:
			directsetpixel = palm_directsetpixel_2;
			break;
		case	4:
			directsetpixel = palm_directsetpixel_4;
			break;
		case	8:
			directsetpixel = palm_directsetpixel_8;
			break;
		case	16:
			directsetpixel = palm_directsetpixel_16;
			break;
		case	24:
			directsetpixel = palm_directsetpixel_24;
			break;
		case	32:
			directsetpixel = palm_directsetpixel_32;
			break;
			}
		}
	else	{
	#endif
		video_base = NULL;
		setsysvar_int(SYSVAR_VIDADR, (int32) video_base);

		directsetpixel = palm_sdk_setpixel;
	#if	PALMOS_SDK_VERSION < 0x0500
		}
	#endif
		
	#if defined(SONY_CLIE)
		}
	#endif
	///////// direct access ///////////////////////////////

	//
	#if defined(SONY_CLIE)
	if	( use_sony_clie )	
		HRWinGetDisplayExtent(sony_refHR, &wmx, &wmy);
	else	
	#endif
	WinGetDisplayExtent(&wmx, &wmy);
	os_graf_mx = wmx;
	os_graf_my = wmy;

	TPS = SysTicksPerSecond();

	audio_queue = dbt_create("SBI-AUDIO", 0);
	dbt_prealloc(audio_queue, AUDIO_QSIZE, sizeof(audio_node));
	audio_qhead = audio_qtail = 0;

	pen_state = 0;
	pen_update = 0;
	cur_x = cur_y = 0;

	if ( os_ver >= 0x350 )	{	
		//
		// PalmOS ver 3.5+
		//	Palette
		//
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
			#if defined(SONY_CLIE)
			WinSetForeColor(vga35[color]);
			if	( use_sony_clie )	{
				HRWinDrawPixel(sony_refHR, 0, 0);
				vga35tra[color] = HRWinGetPixel(sony_refHR, 0, 0);
				}
			else	{
			#endif
			WinSetForeColor(vga35[color]);
			WinDrawPixel(0,0);
			vga35[color] = WinRGBToIndex(&rgb);

//			color
//
//			switch ( os_color_depth )	{
//			case	8:
//				vga35tra[color] = video_base[0];
//				break;
//			case	16:
//				vga35tra[color] = (video_base[0] << 8) | video_base[1];
//				break;
//				};
			#if defined(SONY_CLIE)
			}
			#endif
			}

		vga35tra[15] = 0;
		}

	#if defined(SONY_CLIE)
	if	( use_sony_clie )	
		HRFntSetFont(sony_refHR, 8);
	else	
	#endif
	FntSetFont(stdFont);
	font_h = FntCharHeight();
	maxline = os_graf_my/font_h;
	osd_setcolor(0);

	dev_clrkb();
	dev_cls();
	return 1;
}

/*
*	PalmOS device - restore to defaults
*/
int		osd_devrestore()
{
	#if defined(SONY_CLIE)
	if	( use_sony_clie )	
		HRFntSetFont(sony_refHR, hrStdFont);
	else
	#endif
	FntSetFont(stdFont);

	osd_settextcolor(0, 15);
	font_h = FntCharHeight();
	maxline = os_graf_my/font_h;
	osd_setcolor(0);
	cur_x = cur_y = 0;

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

	if	( ev->eType == penUpEvent )	
		pen_down = 0;
	else if	( ev->eType == penMoveEvent || ev->eType == penDownEvent )	{
		if	( ev->screenY <= os_graf_my )	{
			pen_x = ev->screenX;
			pen_y = ev->screenY;

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

//
long	osd_getpixel(int x, int y)
{
	int		offset, mask = 0, shift = 0, i;
	long	co;

	if	( opt_safedraw )
		return palm_sdk_getpixel(x,y);

	#if defined(SONY_CLIE)
	if	( use_sony_clie ){	
		co = HRWinGetPixel(sony_refHR, 2*x, 2*y);	// Clie digitizer is half HR resolution
		for ( i = 0; i < 16; i ++ )	{		// convert index to vga
			if	( vga35tra[i] == co )
				return i;
			}
		}	
	#endif
	switch ( os_color_depth )	{
	case	1:
		offset = (y*os_graf_mx/8)+x/8;
		shift = 7-(x % 8);
		mask = (1 << shift);
		if ((video_base[offset] & mask) >> shift)
			return 0;
		return 15;
	case	2:
		offset = (y*os_graf_mx/4)+x/4;
		switch ( x % 4 )	{
		case	0:
			mask = 0xC0;
			shift = 6;
			break;
		case	1:
			mask = 0x30;
			shift = 4;
			break;
		case	2:
			mask = 0xC;
			shift = 2;
			break;
		case	3:
			mask = 0x3;
			shift = 0;
			break;
			}
		co = (video_base[offset] & mask) >> shift;
		switch ( co )	{
		case	0:
			return 15;		// white
		case	1:
			return 7;		// light gray
		case	2:
			return 8;		// dark gray
			};
		return 0;			// black
	case	4:
		offset = (y*os_graf_mx/2)+x/2;
		switch ( x % 2 )	{
		case	0:
			mask = 0xF0;
			shift = 4;
			break;
		case	1:
			mask = 0xF;
			shift = 0;
			break;
			}
		co = (video_base[offset] & mask) >> shift;
		co = 15-co;
		if	( co == 7 )
			return 8;
		else if ( co == 8 )
			return 7;
		return co;
	case	8:
		offset = (y*os_graf_mx)+x;
		co = video_base[offset];
		for ( i = 0; i < 16; i ++ )	{
			if	( vga35tra[i] == co )
				return i;
			}
		return co;
	case	16:
		offset = (y*os_graf_mx*2)+x*2;
		return (video_base[offset]<<8)|video_base[offset+1];
		};
	return 0;
}

/*
*	enable or disable PEN code
*/
void	osd_setpenmode(int enable)
{
	if	( enable )	
		pen_state = 2;
	else
		pen_state = 0;
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

	r.topLeft.x = 0;
	r.topLeft.y = 0; 
	r.extent.x = os_graf_mx;
	r.extent.y = os_graf_my;
	#if defined(SONY_CLIE)
	if	( use_sony_clie )
		HRWinEraseRectangle(sony_refHR, &r, 0);
	else
	#endif
	WinEraseRectangle(&r, 0);

	cur_x = cur_y = 0;

	useinvert = 0;
	#if defined(SONY_CLIE)
	if	( use_sony_clie )	
		HRFntSetFont(sony_refHR, 8);
	else
	#endif
	FntSetFont(stdFont);
	WinSetUnderlineMode(noUnderline);
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

		#if defined(SONY_CLIE)
		if ( use_sony_clie )	
			HRWinScrollRectangle(sony_refHR, &r, winUp, dy, &r);
		else
		#endif
		WinScrollRectangle(&r, winUp, dy, &r);

		cur_y = (maxline-1) * font_h;

		r.topLeft.x = 0;
		r.topLeft.y = cur_y;
		r.extent.x = os_graf_mx;
		r.extent.y = os_graf_my - cur_y;
		#if defined(SONY_CLIE)
		if ( use_sony_clie )	
			HRWinEraseRectangle(sony_refHR, &r, 0);
		else
		#endif
		WinEraseRectangle(&r, 0);
		}
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
					#if defined(SONY_CLIE)
					if ( use_sony_clie )	
						HRWinEraseRectangle(sony_refHR, &r, 0);
					else
					#endif
					WinEraseRectangle(&r, 0);
					break;
				case	'G':
					cur_x = esc_val * osd_textwidth("0");
					break;
				case	'm':			// \e[...m	- ANSI terminal
					switch ( esc_val )	{
					case	0:	// reset
//						#if defined(SONY_CLIE)
//						if	( use_sony_clie )	
//							HRFntSetFont(sony_refHR, 8);
//						else
//						#endif
//						FntSetFont(stdFont);
						WinSetUnderlineMode(noUnderline);
						useinvert = 0;
						osd_setcolor(0);
						osd_settextcolor(0, 15);
//						font_h = FntCharHeight();
//						maxline = os_graf_my/font_h;
						break;
					case	1:	// set bold on
						#if defined(SONY_CLIE)
						if	( use_sony_clie )	
							HRFntSetFont(sony_refHR, 9);
						else
						#endif
						FntSetFont(boldFont);
						break;
					case	4:	// set underline on
						WinSetUnderlineMode(solidUnderline);
						break;
					case	7:	// reverse video on
						useinvert = 1;
						break;
					case	21:	// set bold off
						#if defined(SONY_CLIE)
						if	( use_sony_clie )	
							HRFntSetFont(sony_refHR, 8);
						else
						#endif
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

					// extra sony-clie palmos (tiny fonts)
					case	70:	// select font 1	std
					case	71:	// select font 2	bold
					case	72:	// select font 3	large
					case	73:	// select font 4	symbol
					case	74:	// select font 5	symbol 11
					case	75:	// select font 6	symbol 7
					case	76:	// select font 7	led
					case	77:	// select font 8	large bold
						#if defined(SONY_CLIE)
						if	( use_sony_clie )	{
							HRFntSetFont(sony_refHR, esc_val-70);
							font_h = FntCharHeight();
							maxline = os_graf_my/font_h;
							}
						#endif
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
						#if defined(SONY_CLIE)
						if	( use_sony_clie )	{
							HRFntSetFont(sony_refHR, esc_val-72);	// +7 for Sony
							font_h = FntCharHeight();
							maxline = os_graf_my/font_h;
							}
						else	{
						#endif
						FntSetFont(esc_val-80);
						font_h = FntCharHeight();
						maxline = os_graf_my/font_h;
						#if defined(SONY_CLIE)
						}
						#endif
						break;
					case	90:	// select custon font 1
					case	91:	// select custom font 2
					case	92:	// select custom font 3
					case	93:	// select custom font 4
						if	( os_charset == enc_utf8 )	{
							#if defined(SONY_CLIE)
							if	( use_sony_clie )
								HRFntSetFont(sony_refHR, 128+(esc_val-90));
							else
							#endif
							FntSetFont(128+(esc_val-90));
							font_h = FntCharHeight();
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
			#if defined(SONY_CLIE)
			if ( use_sony_clie )	
				HRWinEraseRectangle(sony_refHR, &r, 0);
			else
			#endif
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
			#if defined(SONY_CLIE)
			if	( use_sony_clie )	{
				if	( !useinvert )	
					HRWinDrawChars(sony_refHR, buf, char_len, cur_x, cur_y);
				else	
					HRWinDrawInvertedChars(sony_refHR, buf, char_len, cur_x, cur_y);
				}
			else	{
			#endif
			if	( !useinvert )	
				WinDrawChars(buf, char_len, cur_x, cur_y);
			else	
				WinDrawInvertedChars(buf, char_len, cur_x, cur_y);
			#if defined(SONY_CLIE)
			}
			#endif

			// advance
			cur_x += cx;
			};

		if	( *p == '\0' )
			break;

		p ++;
		}
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
		dcolor = palm_directcolor(dev_fgcolor);
		}

	MemSet(&rgb, sizeof(rgb), 0);
	switch ( color )	{
	case	0:
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
		else  if ( os_ver >= 0x350 )	{		// PalmOS ver 3.5+
			if	( bg )
				WinSetBackColor(vga35[color]);
			else	{
				WinSetTextColor(vga35[color]);
				WinSetForeColor(vga35[color]);
				}
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

/******************************************************************************
*	pixel access
*/

/*
*	return the real VRAM color
*/
static long	palm_directcolor(long c)
{
	#if defined(SONY_CLIE)
	if	( use_sony_clie )	
		return c;
	#endif
	switch ( os_color_depth )	{
	case	1:
		if	( c != 15 )
			return 1;	// black
		return 0;	// white
	case	2:
		switch ( c )	{
		case	0:	
			return 3;		// black
		case	15:
			return 0;		// white
		case	7:
			return 1;		// light gray
		case	8:
			return 2;		// dark gray
			};

		if	( c < 7 )
			return 2;		// dark
		return 1;			// light
	case	4:
		c = 15-c;
		if	( c == 8 )
			return 7;
		else if ( c == 7 )
			return 8;
		return c;
		}

	return vga35tra[c];
}

/*
*	use api - setpixel
*/
static void palm_sdk_setpixel(int x, int y)
{
	WinDrawPixel(x, y);
}

/*
*	use api - getpixel
*/
static long palm_sdk_getpixel(int x, int y)
{
	return WinGetPixel(x, y);
}

#if	PALMOS_SDK_VERSION < 0x0500
/*
*	1bit mode - setpixel
*/
static void	palm_directsetpixel_1(int x, int y)
{
	long	offset;
	int		shift, mask;

	offset = (y*(os_graf_mx>>3))+(x>>3);
	shift = 7-(x % 8);
	mask = ((1 << shift) ^ 0xFF);
	video_base[offset] = (video_base[offset] & mask) | (dcolor << shift);
}

/*
*	2bit mode - setpixel
*/
static void	palm_directsetpixel_2(int x, int y)
{
	long	offset;
	byte	shift, mask;

	offset = (y*(os_graf_mx>>2))+(x>>2);
	switch ( x % 4 )	{
	case	0:
		mask = 0x3F;
		shift = 6;
		break;
	case	1:
		mask = 0xCF;
		shift = 4;
		break;
	case	2:
		mask = 0xF3;
		shift = 2;
		break;
	default:	// case	3:
		mask = 0xFC;
		shift = 0;
		break;
		}

	video_base[offset] = (video_base[offset] & mask) | (dcolor << shift);
}

/*
*	4bit mode - setpixel
*/
static void	palm_directsetpixel_4(int x, int y)
{
	long	offset;

	offset = (y*(os_graf_mx>>1))+(x>>1);
	if	( (x % 2) == 0 )	
		video_base[offset] = (video_base[offset] & 0xF) | (dcolor << 4);
	else
		video_base[offset] = (video_base[offset] & 0xF0) | dcolor;
}

/*
*	8bit mode - setpixel
*/
static void	palm_directsetpixel_8(int x, int y)
{
	video_base[(y*os_graf_mx)+x] = dcolor;
}

/*
*	8bit mode - setpixel (SONY CLIE)
*/
#if defined(SONY_CLIE)
static void	palm_sony_directsetpixel_8(int x, int y)
{
	if	( use_sony_clie )
		HRWinDrawPixel(sony_refHR, x, y);
	else
		video_base[(y*os_graf_mx)+x] = dcolor;
}
#endif

/*
*	16bit mode - setpixel
*/
static void	palm_directsetpixel_16(int x, int y)
{
	long	offset;

	offset = (y*(os_graf_mx<<1))+(x<<1);
	video_base[offset] = (dcolor >> 8);
	video_base[offset+1] = (dcolor & 0xFF);
}

/*
*	24bit mode - setpixel
*/
static void	palm_directsetpixel_24(int x, int y)
{
	long	offset;

	offset = (y*(os_graf_mx*3))+(x*3);
	video_base[offset]   = (dcolor >> 16);
	video_base[offset+1] = ((dcolor & 0xFF00) >> 8);
	video_base[offset+2] = (dcolor & 0xFF);
}

/*
*	32bit mode - setpixel
*/
static void	palm_directsetpixel_32(int x, int y)
{
	long	offset;

	offset = (y*(os_graf_mx<<2))+(x<<2);
	memcpy(video_base+offset, &dcolor, 4);
}
#endif	// SDK < 5

/*
*/
void	palm_directhline(int x, int x2, int y)
{
#if	PALMOS_SDK_VERSION < 0x0500
	long	offset, i, len;
	long	co;
	byte	l[2];
#endif

	if	( opt_safedraw )	{
		WinDrawLine(x, y, x2, y);
		return;
		}

	#if defined(SONY_CLIE)
	if	( use_sony_clie )	{
		HRWinDrawLine(sony_refHR, x, y, x2, y);
		return;
		}
	#endif

#if	PALMOS_SDK_VERSION < 0x0500
	if	( x > x2 )	{
		i  = x;
		x  = x2;
		x2 = i;
		}

	switch ( os_color_depth )	{
	case	4:
		co = (dcolor << 4) | dcolor;
		l[0] = (x  % 2);
		l[1] = (x2 % 2);
		if	( l[0] )	{
			directsetpixel(x,y);
			x ++;
			}
		if	( l[1] == 0 )	{
			directsetpixel(x2,y);
			x2 --;
			}

		if	( x2 - x < 1 )	{
			directsetpixel(x,y);
			}
		else	{
			len = ((x2-x)+1) >> 1;
			offset = (y*(os_graf_mx>>1))+(x>>1);
			memset(video_base+offset, co, len);
			}
		return;
	case	8:
		len = (x2-x)+1;
		offset = (y*os_graf_mx)+x;
		memset(video_base+offset, dcolor, len);
		return;
	default:
		WinDrawLine(x, y, x2, y);
		};
#endif
}

/*
*	Bresenham's algorithm for drawing line
*/
#define SDL_FASTPIX(x,y)	directsetpixel(x,y)
//void	palm_directline(int X1, int Y1, int X2, int Y2)
void	osd_line(int X1, int Y1, int X2, int Y2)
{
	int		dX, dY, row, col, final, G, inc1, inc2;
	char	pos_slope;

	if	( opt_safedraw )	{
		WinDrawLine(X1, Y1, X2, Y2);
		return;
		}
	#if defined(SONY_CLIE)
	if	( use_sony_clie )	{
		HRWinDrawLine(sony_refHR, X1, Y1, X2, Y2);
		return;
		}
	#endif
	// 
	if	( X1 >= os_graf_mx )
		X1 = os_graf_mx;
	else if ( X1 < 0 )
		X1 = 0;
	if	( Y1 >= os_graf_my )
		Y1 = os_graf_my;
	else if ( Y1 < 0 )
		Y1 = 0;

	if	( X2 >= os_graf_mx )
		X2 = os_graf_mx;
	else if ( X2 < 0 )
		X2 = 0;
	if	( Y2 >= os_graf_my )
		Y2 = os_graf_my;
	else if ( Y2 < 0 )
		Y2 = 0;

	//
	if	( Y1 == Y2 )	{
		palm_directhline(X1, X2, Y1);
		return;
		}

	dX = X2 - X1;
	dY = Y2 - Y1;
	pos_slope = (dX > 0);
	if ( dY < 0 )
		pos_slope = !pos_slope;

	if ( ABS(dX) > ABS(dY) )	{
		if (dX > 0)    {
			col = X1;
			row = Y1;
			final = X2;
		    }
		else	{
			col = X2;
			row = Y2;
			final = X1;
		    }

	    inc1 = 2 * abs (dY);
	    G = inc1 - abs (dX);
	    inc2 = 2 * (abs (dY) - abs (dX));
	    if ( pos_slope )	{
			while (col <= final)	{
				SDL_FASTPIX(col, row);
				col++;
				if (G >= 0) {
					row ++;
					G += inc2;
					}
				else		
					G += inc1;
				}
			}
		else	{
			while (col <= final)	{
				SDL_FASTPIX(col, row);
				col ++;
				if (G > 0)	{
					row --;
					G += inc2;
					}
				else
					G += inc1;
				}
			}
		}  /* if |dX| > |dY| */
	else	{
		if (dY > 0)	{
			col = X1;
			row = Y1;
			final = Y2;
		    }
		else	{
			col = X2;
			row = Y2;
			final = Y1;
		    }

		inc1 = 2 * abs (dX);
		G = inc1 - abs (dY);
		inc2 = 2 * (abs (dX) - abs (dY));

		if (pos_slope)	{
			while (row <= final) {
				SDL_FASTPIX(col, row);
				row ++;
				if (G >= 0)	{
					col ++;
					G += inc2;
					}
				else
					G += inc1;
				}
			}
		else	{
			while (row <= final)	{
				SDL_FASTPIX(col, row);
				row ++;
				if (G > 0)	{
					col --;
	  				G += inc2;
					}
				else
					G += inc1;
		     	}
			}
		}
}

/*
*	draw pixel
*/
void	osd_setpixel(int x, int y)
{
	directsetpixel(x, y);
}

/*
*	draw rect
*/
void	osd_rect(int x1, int y1, int x2, int y2, int fill)
{
	int				y;
	RectangleType	r;

	r.topLeft.x = x1;
	r.topLeft.y = y1;
	r.extent.x = (x2 - x1)+1;
	r.extent.y = (y2 - y1)+1;

	switch ( dev_fgcolor )	{
	case	0:		/* black */
		if	( fill )	{
			#if defined(SONY_CLIE)
			if	( use_sony_clie )	
				HRWinDrawRectangle(sony_refHR, &r, 0);
			else
			#endif
			WinDrawRectangle(&r, 0);
			}
		else	{
			osd_line(x1, y1, x1, y2);
			osd_line(x1, y2, x2, y2);
			osd_line(x2, y2, x2, y1);
			osd_line(x2, y1, x1, y1);
			}
		break;
	case	15:		/* white */
		if	( fill )	{
			#if defined(SONY_CLIE)
			if	( use_sony_clie )	
				HRWinEraseRectangle(sony_refHR, &r, 0);
			else
			#endif
			WinEraseRectangle(&r, 0);
			}
		else	{
			osd_line(x1, y1, x1, y2);
			osd_line(x1, y2, x2, y2);
			osd_line(x2, y2, x2, y1);
			osd_line(x2, y1, x1, y1);
			}
		break;
	default:
		if	( fill )	{
			for ( y = y1; y <= y2; y ++ )	
				palm_directhline(x1, x2, y);
			}
		else	{
			osd_line(x1, y1, x1, y2);
			osd_line(x1, y2, x2, y2);
			osd_line(x2, y2, x2, y1);
			osd_line(x2, y1, x1, y1);
			}
		}
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
	return FntCharsWidth(str, strlen(str));
}

int		osd_textheight(const char *str)
{
	// TODO: count \n
	return font_h;
}

void	osd_refresh()
{
}
