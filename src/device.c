/*
*	lowlevel device (OS) I/O
*/

#include "sys.h"
#include "str.h"
#include "var.h"
#define	DEVICE_MODULE
#include "device.h"
#include "osd.h"
#include "smbas.h"
#include "sberr.h"
#include "messages.h"
#if !defined(_PalmOS)
	#include <signal.h>
	#include <stdio.h>
	#if defined(_UnixOS)
		#include <sys/types.h>
		#include <sys/stat.h>
		#include <sys/time.h>		// struct timeval
		#include <unistd.h>
//		#include <sys/utsname.h>
		extern char **environ;
	#elif defined(_DOS)
		#include <sys/time.h>		// struct timeval
		#include <unistd.h>
		#include <conio.h>
		extern char **environ;
	#elif defined(_Win32)
		#include <windows.h>
		#include <process.h>
		#include <dir.h>
		extern char **environ;
	#endif
#endif

// ADD-ON DRIVERS
#if defined(DRV_SOUND)
#include "drvsound.h"
static int drvsound_ok;
#endif

#if defined(DRV_MOUSE)
#include "drvmouse.h"
static int drvmouse_ok;
#endif

#include "dev_term.h"

//	Keyboard buffer
#define	PCKBSIZE	256
static word keybuff[PCKBSIZE];
static int	keyhead;
static int	keytail;

///////////////////////////////////////////////
//////////////////////////////// INIT & RESTORE

/*
*	os_graphics:
*		0 = no graphics (console)
*		1 = ...
*	os_graf_mx, os_graf_my = graphics display max. x, max. y
*/

#if defined(_FRANKLIN_EBM)
	///
	/// EBM
	///
	byte    os_graphics = 1;
	int             os_graf_mx = 200;
	int             os_graf_my = 240;
	dword   os_ver = 0x20000;
	byte    os_color = 1;
	dword   os_color_depth = 16;
#elif defined(_PalmOS)
	///
	/// PalmOS
	///
	dword	os_ver = 0x30100;
	dword	os_color_depth = 1;
	byte	os_color = 0;
	#if defined(SONY_CLIE)
		byte	use_sony_clie = 0;
		UInt16	sony_refHR;
		int		os_graf_mx = 320;
		int		os_graf_my = 320;
	#else
		int		os_graf_mx = 160;
		int		os_graf_my = 160;
	#endif
	byte	os_graphics = 1;
#elif defined(_VTOS)
	///
	/// VTOS
	///
	dword   os_ver = OS_VER;
	dword	os_color_depth = 1;
	byte	os_color = 0;
	byte	os_graphics = 1;
	int		os_graf_mx = 160;
	int		os_graf_my = 160;
#else
	///
	/// Unix/DOS/Windows
	///
	dword	os_ver = 0x40000;
	byte	os_color = 1;
	dword	os_color_depth = 16;
	#if defined(_WinBCB)
		byte	os_graphics = 1;
	#else
		byte	os_graphics = 0;			// CONSOLE
	#endif
	int		os_graf_mx = 80;
	int		os_graf_my = 25;
#endif

// cache
word	os_cclabs1 = 256;			
word	os_ccpass2 = 256;

// graphics - viewport
int32	dev_Vx1;
int32	dev_Vy1;
int32	dev_Vx2;
int32	dev_Vy2;

int32	dev_Vdx;
int32	dev_Vdy;

// graphics - window world coordinates
int32	dev_Wx1;
int32	dev_Wy1;
int32	dev_Wx2;
int32	dev_Wy2;

int32	dev_Wdx;
int32	dev_Wdy;

//
long	dev_fgcolor = 0;
long	dev_bgcolor = 15;

/*
*	BREAK SIGNAL
*/
#if defined(_UnixOS) || defined(_DOS)
void	termination_handler (int signum)
{
	static int		ctrlc_count;

	prog_error = -2;
	ctrlc_count ++;
	if	( ctrlc_count == 1 )	
		dev_printf("\n\n\033[0m\033[7m\a * %s %d * \033[0m\n", WORD_BREAK_AT, prog_line);
	else if ( ctrlc_count == 3 )	{
		dev_restore();
	   	memmgr_setabort(1);
		exit(1);
		}		
}
#endif

/*
*	INITIALIZE ALL DRIVERS
*/
int		dev_init(int mode, int flags)
{
#if defined(_UnixOS)
	int		verfd;
//	struct utsname uts;
#endif

#if defined(DRV_SOUND)
	drvsound_ok = drvsound_init();
#endif
#if defined(DRV_MOUSE)
	if	( os_graphics )
		drvmouse_ok = drvmouse_init();
#endif
	dev_initfs();
	dev_fgcolor = 0;
	#if defined(_PalmOS)
	dev_bgcolor = 15;
	#else
	dev_bgcolor = (os_graphics) ? 15 : 0;
	#endif
	#if defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	osd_devinit();
	#else
	os_graphics = mode;
	term_init();	// by default
	if	( mode )	{
		#if defined(_UnixOS)
		char	buf[256];

		if	( term_israw() )	
			setsysvar_str(SYSVAR_OSNAME, "Unix/RAW");
		else	{
			if	( getenv("TERM") )	{
				strcpy(buf, "Unix/Terminal:");
				strcat(buf, getenv("TERM"));
				setsysvar_str(SYSVAR_OSNAME, buf);
				}
			else
				setsysvar_str(SYSVAR_OSNAME, "Unix/Stream");
			}
		#endif

		if	( osd_devinit() == 0 )
        	#if defined(_WinBCB)
            panic("osd_devinit() failed");
            #else
			exit(1);
            #endif
		}
	#endif

	dev_viewport(0,0,0,0);
	dev_window(0,0,0,0);

	///////
	if	( os_graphics )	{
		dev_fgcolor = 0;
		dev_bgcolor = 15;
		osd_settextcolor(dev_fgcolor, dev_bgcolor);
		osd_setcolor(dev_fgcolor);
		}
	else	{
		dev_fgcolor = 7;
		dev_bgcolor = 0;
		#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
		#else
//		term_settextcolor(dev_fgcolor, dev_bgcolor);
		#endif
		}

#if defined(_UnixOS)
/*
	if	( uname(&uts) == 0 )	{
		// will use the POSIX's uname()
		strcpy(tmp, "Unix/");
		strcat(tmp, uts.machine);
		strcat(tmp, "/");
		strcat(tmp, uts.sysname);
		setsysvar_str(SYSVAR_OSNAME, tmp);
		}
	else	{
*/
		// will try to read /proc/version

		verfd = open("/proc/version", O_RDONLY);
		if	( verfd != -1 )	{
			char	*p;
			char	verstr[256];
			char	tmp[300];
			int		bytes;

			memset(verstr, 0, 256);
			bytes = read(verfd, verstr, 255);
			verstr[(bytes<256)?bytes:255] = '\0';
			p = strchr(verstr, '\n');
			if	( p )	*p = '\0';
			close(verfd);

			// store name to system constant
			strcpy(tmp, "Unix/");
			strcat(tmp, verstr);
			setsysvar_str(SYSVAR_OSNAME, tmp);

			// build OSVER
			if	( (p = strstr(verstr, "ersion")) != NULL )	{
				long	vi = 0;
				int		dg = 0;

				p += 6;
				while ( *p == ' ' || *p == '\t' )	p ++;

				while ( *p )	{
					if	( is_digit(*p) )	{
						vi = (vi << 4) + (*p - '0');
						dg ++;
						}
					else if	( *p == '.' )	{
						switch ( dg )	{
						case	0:
							vi = vi << 8;
							break;
						case	1:
							vi = vi << 4;
							break;
							};

						dg = 0;
						}
					else
						break;

					p ++;
					} // while (*p)

				os_ver = vi;
				} // if ver
			} // verfd
//		}

	setsysvar_int(SYSVAR_OSVER, os_ver);
#elif defined(_DOS)
	os_ver = ((_osmajor << 16) | (_osminor)) << 8;
	setsysvar_int(SYSVAR_OSVER, os_ver);
#elif defined(_WinBCB)
    if	( flags == 0 )
		setsysvar_int(SYSVAR_OSVER, os_ver);
#else
		setsysvar_int(SYSVAR_OSVER, os_ver);
#endif

	#if defined(_WinBCB)
    if	( flags == 0 )	{
    #endif
	setsysvar_int(SYSVAR_XMAX, os_graf_mx-1);
	setsysvar_int(SYSVAR_YMAX, os_graf_my-1);
	if	( os_graphics )
		setsysvar_int(SYSVAR_BPP, os_color_depth);
	else
		setsysvar_int(SYSVAR_BPP, 4);
	#if defined(_WinBCB)
    }
    #endif

#if defined(_UnixOS) || defined(_DOS)
	signal(SIGINT, termination_handler);
	signal(SIGQUIT, termination_handler);
#endif
	return 1;
}

/*
*	RESTORE DEVICE'S MODE
*/
int		dev_restore()
{
	if	( os_graphics )
		osd_refresh();
		
#if defined(DRV_SOUND)
	if	( drvsound_ok )
		drvsound_close();
#endif
#if defined(DRV_MOUSE)
	if	( os_graphics )	{
		if	( drvmouse_ok )
			drvmouse_close();
		}
#endif
	dev_closefs();
	if	( os_graphics )
		osd_devrestore();
	#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	#else
	term_restore();	// by default
	#endif

#if defined(_UnixOS) || defined(_DOS)
   	signal(SIGINT, SIG_DFL);
   	signal(SIGQUIT, SIG_DFL);
#endif
	return 1;
}

/*
*	CHECK FOR EVENTS
*
*	wait == 0	check & return
*		 != 0	wait for an event
*
*	returns 0 for no events in queue
*/
int		dev_events(int waitf)
{
	#if !defined(_PalmOS) && !defined(_FRANKLIN_EBM)
	if	( os_graphics )
		osd_refresh();
	#endif

	#if defined(_UnixOS) || defined(_DOS)
	//
	//	standard input case
	//
	if	( !os_graphics )	{
		if	( term_israw() )	
			return !feof(stdin);
		}
	#endif

	#if	defined(_FRANKLIN_EBM)
	return osd_events(waitf);
	#else

	if	( waitf )	{
		int		evc;

 		#if defined(_VTOS)
		{
		extern int osd_events_sleep();
		evc = osd_events_sleep();
		}
		#else 
		while ( (evc = dev_events(0)) == 0 )	
			#if defined(_UnixOS)
			usleep(31250);
			#elif defined(_Win32)
	        	;
			#elif defined(_PalmOS)
				;					//
			#else
			dev_delay(31);		// sleep for 1000/32 and try again
			#endif
		#endif
		return evc;
		}
	else	{
		#if defined(DRV_SOUND)
		drvsound_event();
		#endif
		#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) 
		return	osd_events();
		#else
		if	( os_graphics )
			return	osd_events();
		return term_events();
		#endif
		}

	#endif // _FRANKLIN_EBM
}

/*
*	delay for a specified amount of milliseconds
*/
void	dev_delay(dword ms)
{
	#if defined(_PalmOS)
	dword	start, tps, now;
	int		evc;

	start = TimGetTicks();
	tps   = SysTicksPerSecond();
	while ( 1 )	{
		SysTaskDelay(1);

		switch ( (evc = dev_events(0)) )	{
		case	0:	// no event
			break;
		case	-2:	// break
			brun_break();
		case	-1:	// break
			return;
			}

		now = TimGetTicks();
			
		if	( now > (start + (tps * ms) / 1000L) )
			return;
		}
	#elif defined(_Win32)
	Sleep(ms);
	#elif defined(_DOS)
	delay(ms);
	#else	// Unix
	usleep(ms * 1000);
	#endif
}

///////////////////////////////////////////////
////////////////////////////////////// KEYBOARD

/*
*	clear keyboard buffer
*/
void	dev_clrkb()
{
	keyhead = keytail = 0;
}

/*
*	stores a key in keyboard buffer
*/
void	dev_pushkey(word key)
{
	keybuff[keytail] = key;
	keytail ++;
	if	( keytail >= PCKBSIZE )
		keytail = 0;
}
			
/*
*	returns true if there is an key in keyboard buffer
*/
int		dev_kbhit()
{
	int		code;

#if defined(_UnixOS) || defined(_DOS)
	if	( !os_graphics && term_israw() )
		return !feof(stdin);
#endif
	
	if ( keytail != keyhead )
		return 1;

#ifdef _FRANKLIN_EBM
    // conserve battery power
	code = dev_events(1);
#else
	code = dev_events(0);
#endif
	if	( code < 0)
		brun_break();

	return (keytail != keyhead);
}

/*
*	returns the next key in keyboard buffer (and removes it)
*/
long int	dev_getch()
{
	word	ch = 0;

#if defined(_UnixOS) || defined(_DOS)
	if	( !os_graphics && term_israw() )
		return fgetc(stdin);
#endif
	
	while ( (dev_kbhit() == 0) && (prog_error == 0) )	{
		int		evc;

		evc = dev_events(1);
		if	( evc < 0 || prog_error )
			return 0xFFFF;
		}

	ch = keybuff[keyhead];
	keyhead ++;
	if	( keyhead >= PCKBSIZE )
		keyhead = 0;

	return ch;
}

/*
*	draw the cursor
*/
void	dev_drawcursor(int x, int y)			SEC(BIO);
void	dev_drawcursor(int x, int y)
{
	if	( os_graphics )	{
		osd_setpixel(x, y);
		osd_setpixel(x, y+1);
		osd_setpixel(x, y+2);
		}
}

/*
*	return the character (multibyte charsets support)
*/
int		dev_input_char2str(int ch, byte *cstr)	SEC(BIO);
int		dev_input_char2str(int ch, byte *cstr)
{
	memset(cstr, 0, 3);

	switch ( os_charset )	{
	case	enc_sjis:				// Japan
		if	( IsJISFont(ch) )	
			cstr[0] = ch >> 8;
		cstr[1] = ch & 0xFF;
		break;
	case	enc_big5:				// China/Taiwan (there is another charset for China but I don't know the difference)
		if	( IsBig5Font(ch) )
			cstr[0] = ch >> 8;
		cstr[1] = ch & 0xFF;
		break;
	case	enc_gmb:				// Generic multibyte
		if	( IsGMBFont(ch) )		
			cstr[0] = ch >> 8;
		cstr[1] = ch & 0xFF;
		break;
	case	enc_unicode: 			// Unicode
		cstr[0] = ch >> 8;
		cstr[1] = ch & 0xFF;
		break;
	default:
		cstr[0] = ch;
		};
	return strlen((char *) cstr);
}

/*
*	return the character size at pos! (multibyte charsets support)
*/
int		dev_input_count_char(byte *buf, int pos)	SEC(BIO);
int		dev_input_count_char(byte *buf, int pos)
{
	int		count, ch;
	byte	cstr[3];

	if ( os_charset != enc_utf8 )	{
		ch = buf[0];
		ch = ch << 8;
		ch = ch + buf[1];
		count = dev_input_char2str(ch, cstr);
		}
	else
		count = 1;
	return count;
}

/*
*	stores a character at 'pos' position
*/
int		dev_input_insert_char(int ch, byte *dest, int pos, int replace_mode) SEC(BIO);
int		dev_input_insert_char(int ch, byte *dest, int pos, int replace_mode)
{
	byte	cstr[3];
	int		count, remain;
	byte	*buf;

	count = dev_input_char2str(ch, cstr);

	// store character into buffer
	if	( replace_mode )	{
		// overwrite mode
		remain = strlen((char *) (dest+pos));
		buf = tmp_alloc(remain+1);
		strcpy(buf, dest+pos);
		memcpy(dest+pos, cstr, count);
		dest[pos+count] = '\0';

		if ( os_charset != enc_utf8 )	
			count = dev_input_char2str(((buf[0]<<8)|buf[1]), cstr);
		else
			count = 1;

		if	( buf[0] )	// not a '\0'
			strcat((char *) dest, (char *) (buf+count));
		tmp_free(buf);
		}
	else	{
		// insert mode
		remain = strlen((char *) (dest+pos));
		buf = tmp_alloc(remain+1);
		strcpy(buf, dest+pos);
		memcpy(dest+pos, cstr, count);
		dest[pos+count] = '\0';
		strcat((char *) dest, (char *) buf);
		tmp_free(buf);
		}

	return count;
}

/*
*	removes the character at 'pos' position
*/
int		dev_input_remove_char(byte *dest, int pos)	SEC(BIO);
int		dev_input_remove_char(byte *dest, int pos)
{
	byte	cstr[3];
	int		count, remain;
	byte	*buf;

	if	( dest[pos] )	{
		if ( os_charset != enc_utf8 )	
			count = dev_input_char2str(((dest[pos]<<8)|dest[pos+1]), cstr);
		else
			count = 1;

		remain = strlen((char *) (dest+pos+1));
		buf = tmp_alloc(remain+1);
		strcpy(buf, dest+pos+count);

		dest[pos] = '\0';
		strcat((char *) dest, (char *) buf);
		tmp_free(buf);
		return count;
		}
	return 0;
}

/*
*	clears right of the cursor
*/
void	dev_input_clreol(int cx, int cy)	SEC(BIO);
void	dev_input_clreol(int cx, int cy)
{
	int		x, y;
	int		color = dev_fgcolor;

	x = dev_getx();
	y = dev_gety();
	if	( x + cx + 1 >= os_graf_mx )
		dev_clreol();
	else	{
		if	( os_graphics )	{
			osd_setcolor(dev_bgcolor);
			osd_rect(x, y, x+cx+1, y+cy, 1);
			osd_setcolor(color);
			}
		else	{
			dev_clreol();
			}
		}
}

#if !defined(_FRANKLIN_EBM)

/*
*	gets a string (INPUT)
*/
char	*dev_gets(char *dest, int size)
{
	long int	ch = 0;
	word		pos, len = 0;
	int			prev_x = 1, prev_y = 1;
	int			tmp_lines, lines = 0, disp_x, disp_y;
	int			lpp = 24, cy = 1, w, replace_mode = 0;
	char		prev_ch;
	int			code;
	int			cx = 1;

	#if defined(_DOS) || defined(_UnixOS)
	if	( !os_graphics )	{
		if	( term_israw() )	{
			//	standard input
			char	*p;
			int		c;

			if	( feof(stdin) )	{
				strcpy(dest, "");
				return dest;
				}

			p = dest;
			while ( (c = fgetc(stdin)) != -1 )	{
				if	( c == '\r' )
					continue;		// ignore
	   			if	( c == '\n' )	
					break;
				if	( size <= ((int) (p-dest))+1 )
					break;

				*p ++ = c;
				}

			*p = '\0';
			return dest;
			}
		}
	#endif

	/*
	*	the 'input'
	*
	*	warning: getx/y routines are required
	*/
	*dest = '\0';

	if	( os_graphics )	{
		prev_x = dev_getx();
		prev_y = dev_gety();
		cx     = dev_textwidth("W");
		cy     = dev_textheight("Q");
		lpp    = os_graf_my / cy;
		}
	
	dev_clrkb();

	pos = 0;
	#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	#else
	if	( !os_graphics )
		term_getsdraw(dest, 0, 0);
	#endif

	do	{

		len = strlen(dest);

		// draw
		#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
		#else
		if	( !os_graphics )
			term_getsdraw(dest, pos, 1);
		else	{
		#endif
			dev_setxy(prev_x, prev_y);
			dev_print(dest);
			dev_input_clreol(cx, cy);

			//
			tmp_lines = (prev_x + dev_textwidth(dest)) / os_graf_mx;
			if	( tmp_lines > lines )	{
				lines = tmp_lines;
				while ( (lines * cy) + prev_y >= (lpp * cy) )
					prev_y -= cy;
				}

			//
			prev_ch = dest[pos];
			dest[pos] = '\0';
			w = dev_textwidth(dest);
			dest[pos] = prev_ch;

			tmp_lines = (prev_x + w) / os_graf_mx;

			disp_y    = prev_y + tmp_lines * cy;
			disp_x    = (prev_x + w) - (tmp_lines * os_graf_mx) +
					(tmp_lines * dev_textwidth(" "));	// TODO: + width of chars at the end of prev lines

			dev_setxy(disp_x, disp_y);
			dev_drawcursor(disp_x, disp_y);
		#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
		#else
			}
		#endif

		// wait for event
		code = dev_events(1);
		
		// remove cursor
		// ...

		if	( code < 0 )	{		// BREAK event
			*dest = '\0';
			brun_break();
			return dest;
			}

		while ( dev_kbhit() )	{	// we have keys
			ch = dev_getch();

			switch ( ch )	{
			case -1:
			case -2:
			case 0xFFFF:
				dest[pos] = '\0';
				return dest;
			case 0: case 10: case 13:	// ignore
				break;
			case SB_KEY_HOME:
				pos = 0;
				break;
			case SB_KEY_END:
				pos = len;
				break;
			case SB_KEY_BACKSPACE: 		// backspace
				if	( pos > 0 )		{
					pos -= dev_input_remove_char((byte *) dest, pos-1);
					len = strlen(dest);
					}
				else
					dev_beep();
				break;
#if defined(_PalmOS) 
			case SB_KEY_PALM_BTN1:
#endif
			case SB_KEY_DELETE: 		// delete
				if	( pos < len )		{
					dev_input_remove_char((byte *) dest, pos);
					len = strlen(dest);
					}
				else
					dev_beep();
				break;
			case SB_KEY_INSERT:
				replace_mode = !replace_mode;
				break;
			case SB_KEY_LEFT:
				if	( pos > 0 )
					pos -= dev_input_count_char((byte *) dest, pos);
				else
					dev_beep();
				break;
			case SB_KEY_RIGHT:
				if	( pos < len )
					pos += dev_input_count_char((byte *) dest, pos);
				else
					dev_beep();
				break;
			default:
				if	( (ch & 0xFF00) != 0xFF00 )			// Not an hardware key
					pos += dev_input_insert_char(ch, (byte *) dest, pos, replace_mode);
				else
					ch = 0;

				// check the size
				len = strlen(dest);
				if	( len >= (size-2) )
					break;
				}
			}	// dev_kbhit() loop

		} while ( ch != '\n' && ch != '\r' );
	dest[len] = '\0';

	#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	#else
	if	( !os_graphics )	
		term_getsdraw(dest, strlen(dest), 2);
	else	{
	#endif
		dev_setxy(prev_x, prev_y);
		dev_print(dest);
		dev_input_clreol(cx, cy);
	#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	#else
		}
	#endif
	return dest;	 
}
#endif // _FRANKLIN_EBM

/*
*	enable/disable default pointing device (pen or mouse)
*/
void	dev_setpenmode(int enable)
{
	if	( os_graphics )
		osd_setpenmode(enable);
}

/*
*	returns data from pointing-device
*	(see PEN(x), osd_getpen(x))
*/
int		dev_getpen(int code)
{
	if	( os_graphics )
		return osd_getpen(code);
	return 0;
}

///////////////////////////////////////////////
//////////////////////////////////////// SCREEN

/*
*	clear to eol
*/
void	dev_clreol()
{
	dev_print("\033[K");	// ANSI
}

/*
*	returns the x position of cursor (in pixels)
*/
int		dev_getx()
{
	#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	return osd_getx();
	#else
	if ( os_graphics )	
		return osd_getx();
	return term_getx();
	#endif
}

/*
*	returns the y position of cursor (in pixels)
*/
int		dev_gety()
{
	#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	return osd_gety();
	#else
	if ( os_graphics )	
		return osd_gety();
	return term_gety();
	#endif
}

/*
*	sets the position of cursor
*	x,y are in pixels
*/
void	dev_setxy(int x, int y)
{
	if	( x < 0 || x > os_graf_mx )
		return;
	if	( y < 0 || y > os_graf_my )
		return;

	#if	defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	osd_setxy(x, y);
	#else
	if ( os_graphics )	
		osd_setxy(x, y);
	else
		term_setxy(x, y);
	#endif
}

/*
*	sets the currect foreground & background color
*	the background color is used only for texts
*/
void	dev_settextcolor(long fg, long bg)
{
	#if !defined(_Win32) && !defined(_PalmOS) && !defined(_WinBCB) && !defined(_VTOS) && !defined(_FRANKLIN_EBM)
 	if ( os_graphics )	{
	#endif
		if	( bg == -1 )
			bg = dev_bgcolor;
			
		if	( (fg <= 15) && (bg <= 15) && (fg >= 0) && (bg >= 0) )	{	// VGA
			if	( bg != -1 )	
				dev_bgcolor = bg;
			osd_settextcolor(dev_fgcolor=fg, dev_bgcolor);	
			}
		else
			osd_settextcolor((dev_fgcolor=fg), (dev_bgcolor=bg));	

	#if !defined(_Win32) && !defined(_PalmOS) && !defined(_WinBCB) && !defined(_VTOS) && !defined(_FRANKLIN_EBM)
		}
	else
		term_settextcolor(fg, bg);
	#endif
}


//	prints a string
void	dev_print(const char *str)
{
	#if defined(_Win32) || defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
 	osd_write(str);
	#else
	if ( os_graphics )	
		osd_write(str);
	else
		term_print(str);
	#endif
}

/*
*	printf
*
*	WARNING: PalmOS ver is limited to 256 bytes
*	WARNING: Win32/Unix ver is limited to 1024 bytes
*/
void	dev_printf(const char *fmt, ...)
{
	char	*buf;
	va_list ap;

	va_start(ap, fmt);

	#if defined(_PalmOS)
	buf = tmp_alloc(256);
	StrVPrintF(buf, fmt, ap);
	va_end(ap);

	#else
	buf = tmp_alloc(1024);
	#if defined(_DOS) || defined(_Win32) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	vsprintf(buf, fmt, ap);
	#else
	vsnprintf(buf, 1024, fmt, ap);
	#endif
	va_end(ap);

	#endif
	dev_print(buf);
	tmp_free(buf);

}

/*
*	clears the screen
*/
void	dev_cls()
{
	#if defined(_PalmOS) || defined(_Win32) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	osd_cls();
	#else
	if	( os_graphics )
		osd_cls();
	else	
		term_cls();
	#endif
}

///////////////////////////////////////////////
////////////////////////////////////// GRAPHICS

#define	W2X(x)	( ( (((x)-dev_Wx1)*dev_Vdx)/dev_Wdx ) + dev_Vx1 )
#define	W2Y(y)	( ( (((y)-dev_Wy1)*dev_Vdy)/dev_Wdy ) + dev_Vy1 )
#define	W2D2(x,y)	{ (x) = W2X((x)); (y) = W2Y((y));	}
#define	W2D4(x1,y1,x2,y2)	{ W2D2((x1),(y1)); W2D2((x2),(y2));	}

/*
*	returns the width of 'str' in pixels
*/
int		dev_textwidth(const char *str)
{
	if	( os_graphics )
		return osd_textwidth(str);
	return strlen(str);		// console
}

/*
*	returns the height of 'str' in pixels
*/
int		dev_textheight(const char *str)
{
	if	( os_graphics )
		return osd_textheight(str);
	return 1; // console
}

/*
*	changes the current foreground color
*/
void	dev_setcolor(long color)
{
	#if defined(_Win32) || defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
 	#else
	if ( os_graphics )	{
	#endif
		if	( color <= 15 && color >= 0 )
			osd_setcolor(dev_fgcolor = color);
		else if ( color < 0 )
			osd_setcolor((dev_fgcolor = color));
	#if defined(_Win32) || defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	#else
		}
	else	{
		if	( color <= 15 && color >= 0 )
			term_settextcolor(color, -1);
		}
	#endif
}

/*
*	draw a pixel
*/
void	dev_setpixel(int x, int y)
{
	x = W2X(x);
	y = W2Y(y);
	if	( x >= dev_Vx1 && x <= dev_Vx2 )	{
		if	( y >= dev_Vy1 && y <= dev_Vy2 )	{
			#if defined(_Win32) || defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
 			osd_setpixel(x,y);
			#else
			if ( os_graphics )	
	 			osd_setpixel(x,y);
			else
				term_drawpoint(x,y);
			#endif
			}
		}
}

/*
*	returns the value of a pixel
*/
long	dev_getpixel(int x, int y)
{
	x = W2X(x);
	y = W2Y(y);
	if	( x >= dev_Vx1 && x <= dev_Vx2 )	{
		if	( y >= dev_Vy1 && y <= dev_Vy2 )	{
			#if defined(_Win32) || defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
 			return osd_getpixel(x,y);
			#else
			if ( os_graphics )	
	 			return osd_getpixel(x,y);
			else
				return term_getpoint(x, y);
			#endif
			}
		}
	return 0;
}

/// clipping ////////////////////////////////////////

#define	CLIPENCODE(x,y,c)	{ c = (x < dev_Vx1); c |= ((y < dev_Vy1) << 1); c |= ((x > dev_Vx2) << 2); c |= ((y > dev_Vy2) << 3); }
#define	CLIPIN(c) 			((c & 0xF) == 0)

// Cohen-Sutherland clipping
void	dev_clipline(int *x1, int *y1, int *x2, int *y2, int *visible)
{
	int		done, in1, in2, sw;
	int		c1, c2;

	*visible = done = 0;
	do	{
		CLIPENCODE(*x1, *y1, c1);
		CLIPENCODE(*x2, *y2, c2);
		in1 = CLIPIN(c1);
		in2 = CLIPIN(c2);
		if	( in1 && in2 )	
			*visible = done = 1;
		else if ( 
					(c1 & c2 & 0x1) ||
					(c1 & c2 & 0x2) ||
					(c1 & c2 & 0x4) ||
					(c1 & c2 & 0x8) 
				)
			done = 1;	// visible = false
		else	{
			// at least one point is outside
			if	( in1 )	{
				// swap
				sw = *x1; *x1 = *x2; *x2 = sw;
				sw = *y1; *y1 = *y2; *y2 = sw;
				sw =  c1;  c1 =  c2;  c2 = sw;
				}

			if	( *x1 == *x2 )	{
				if	( c1 & 0x2 )
					*y1 = dev_Vy1;
				else
					*y1 = dev_Vy2;
				}
			else	{
				if	( c1 & 0x1 )	{
					*y1 += (*y2 - *y1) * (dev_Vx1 - *x1) / (*x2- *x1);
					*x1 = dev_Vx1;
					}
				else if ( c1 & 0x4 )	{
					*y1 += (*y2 - *y1) * (dev_Vx2 - *x1) / (*x2- *x1);
					*x1 = dev_Vx2;
					}
				else if ( c1 & 0x2 )	{
					*x1 += (*x2 - *x1) * (dev_Vy1 - *y1) / (*y2 - *y1);
					*y1 = dev_Vy1;
					}
				else if ( c1 & 0x8 )	{
					*x1 += (*x2 - *x1) * (dev_Vy2 - *y1) / (*y2 - *y1);
					*y1 = dev_Vy2;
					}
				}
			}
		} while ( !done );
}

/*
*	draw line
*/
void	dev_line(int x1, int y1, int x2, int y2)
{
	int		visible;

	W2D4(x1,y1,x2,y2);

	// clip_line
	dev_clipline(&x1, &y1, &x2, &y2, &visible);
	if	( visible )	{
		#if defined(_Win32) || defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
		osd_line(x1, y1, x2, y2);
		#else
		if ( os_graphics )	
			osd_line(x1, y1, x2, y2);
		else	
			term_drawline(x1, y1, x2, y2);
		#endif
		}
}

/*
*	draw rectangle (filled or not)
*/
void	dev_rect(int x1, int y1, int x2, int y2, int fill)
{
	int		px1, py1, px2, py2;
	int		c1, c2, in1, in2;

	px1 = x1;	py1 = y1;
	px2 = x2;	py2 = y2;

	W2D4(x1,y1,x2,y2);

	if	( x1 == x2 )	{
		dev_line(px1, py1, px2, py2);
		return;
		}
	if	( y1 == y2 )	{
		dev_line(px1, py1, px2, py2);
		return;
		}

	/*
	*	check inside
	*/
	CLIPENCODE(x1, y1, c1);
	CLIPENCODE(x2, y2, c2);
	in1 = CLIPIN(c1);
	in2 = CLIPIN(c2);
	if	( in1 && in2 )	{
		/*
		*	its inside
		*/
		#if defined(_Win32) || defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS) || defined(_FRANKLIN_EBM)
		osd_rect(x1, y1, x2, y2, fill);
		#else
		if ( os_graphics )	
			osd_rect(x1, y1, x2, y2, fill);
		else
			term_drawrect(x1, y1, x2, y2, fill);
		#endif
		}
	else {
		/*
		*	partial inside 
		*	TODO: something fast
		*/
		int		y;

		if	( fill )	{
			for ( y = py1; y <= py2; y ++ )	
				dev_line(px1, y, px2, y);
			}
		else	{
			dev_line(px1, py1, px1, py2);
			dev_line(px1, py2, px2, py2);
			dev_line(px2, py2, px2, py1);
			dev_line(px2, py1, px1, py1);
			}
		}
}

/*
*	set viewport
*/
void	dev_viewport(int x1, int y1, int x2, int y2)
{
	if	( x1 == x2 || y1 == y2 )	{
		// reset
		dev_Vx1 = 0;
		dev_Vy1 = 0;
		dev_Vx2 = os_graf_mx-1;
		dev_Vy2 = os_graf_my-1;

		dev_Vdx = os_graf_mx-1;
		dev_Vdy = os_graf_my-1;
		}
	else	{
		if	(
			( x1 < 0 ) || ( x2 < 0 ) ||
			( y1 < 0 ) || ( y2 < 0 ) ||
			( x1 >= os_graf_mx ) ||	( x2 >= os_graf_mx ) ||
			( y1 >= os_graf_my ) ||	( y2 >= os_graf_my )
			)	{

			rt_raise(ERR_VP_POS);
			}

		dev_Vx1 = x1;
		dev_Vy1 = y1;
		dev_Vx2 = x2;
		dev_Vy2 = y2;

		dev_Vdx = ABS(x2-x1);
		dev_Vdy = ABS(y2-y1);

		if	( dev_Vdx == 0 || dev_Vdy == 0 )
			rt_raise(ERR_VP_ZERO); 
		}

	// reset window
	dev_Wx1 = dev_Vx1;	dev_Wy1 = dev_Vy1;
	dev_Wx2 = dev_Vx2;	dev_Wy2 = dev_Vy2;

	dev_Wdx = dev_Vdx;
	dev_Wdy = dev_Vdy;
}

/*
*	set window
*/
void	dev_window(int x1, int y1, int x2, int y2)
{
	if	( x1 == x2 || y1 == y2 )	{
		// reset
		dev_Wx1 = dev_Vx1;		dev_Wy1 = dev_Vy1;
		dev_Wx2 = dev_Vx2;		dev_Wy2 = dev_Vy2;

		dev_Wdx = dev_Vdx;
		dev_Wdy = dev_Vdy;
		}
	else	{
		dev_Wx1 = x1;	dev_Wy1 = y1;
		dev_Wx2 = x2;	dev_Wy2 = y2;

		dev_Wdx = x2-x1;
		dev_Wdy = y2-y1;

		if	( dev_Wdx == 0 || dev_Wdy == 0 )
			rt_raise(ERR_WIN_ZERO);
		}
}


///////////////////////////////////////////////
///////////////////////////////////////// SOUND

/*
*	BEEP :)
*/
void	dev_beep()
{
	if	( os_graphics )
		osd_refresh();

	#if defined(_PalmOS) || defined(_VTOS) || defined(_FRANKLIN_EBM)
 	osd_beep();
	#else
		#if defined(DRV_SOUND)
		drvsound_beep();
		#else
		if	( os_graphics )
			osd_beep();
		else
			printf("\a");
		#endif
	#endif
}

/*
*	plays a sound 
*/
void	dev_sound(int frq, int ms, int vol, int bgplay)
{
	#if defined(_PalmOS) || defined(_VTOS) || defined(_FRANKLIN_EBM)
	osd_sound(frq, ms, vol, bgplay);
	#else

	#if defined(DRV_SOUND)
	drvsound_sound(frq, ms, vol, bgplay);
	#else
	if	( os_graphics )
		osd_sound(frq, ms, vol, bgplay);
	// Linux only ???
	else	{
		#if defined(USE_LINCONCODES)
		/*
		*	Linux console codes - PC Speaker
		*/
		printf("\033[10;%d]", frq);
		printf("\033[11;%d]", ms);
		fflush(stdout);
		printf("\a");
		fflush(stdout);
		dev_delay(ms);
		printf("\033[10;%d]", 440);
		printf("\033[11;%d]", 250);
		fflush(stdout);
		#else
		if	( !bgplay )
			dev_delay(ms);
		#endif
		}
	#endif

	#endif
}

/*
*	clear background sound queue
*/
void	dev_clear_sound_queue()
{
	#if defined(DRV_SOUND)
	drvsound_clear_queue();
	#else
	if	( os_graphics )
		osd_clear_sound_queue();
	#endif
}

///////////////////////////////////////////////
//////////////////////////////////////// SYSTEM

/*
*	run an external program
*/
#if defined(_Win32)
/**
*	#$$%#$~@##@@#$!@#$%$#$!$!@#$%$!@#$@#!$!@#!@#$@!#%$#$%^!@$%^!#^#!%$#$%!@#
*
*	© ndc
*
*	Run a program in the Win32 environment and captures the stdout/stderr
*
*	cmd     = the command line (ex: dir /w)
*	infile  = the stdin (can be NULL)
*	outfile = the stdout/stderr
*
*	(like "rm -f < infile > outfile")
*
*	Warning: you can't use DOS built-in commands (like dir, del, etc)
*	Warning: there is no way to take the real output (stdout/stderr as one file)
*			I have test it (the same file for stdout & stderr) on Win95 box
*			and I take the output of stdout at the top and the output of stderr at the bottom !!!
*/
int shellw2(const char *cmd, const char *inbuf, char **outbuf, int priority)
{
    static char tmp_dir[1024];
	static int sh2_count;

	HANDLE	new_stdin, new_stdout, new_stderr;
	int		attr = 0, failed = 0, backg = 0, clen;
	DWORD	exit_code;
	FILE	*fp;

	STARTUPINFO				si;
	SECURITY_ATTRIBUTES		sap, sat;
	PROCESS_INFORMATION		pip;

    char	fname_in[1024], fname_out[1024], fname_err[1024];

    //
    if	( (clen=strlen(cmd)) == 0 )
    	return -1;

    //
    if	( cmd[clen-1] == '&' )
    	backg = 1;

    // temporary directory
    if	( tmp_dir[0] == '\0' )	{
    	GetTempPath(1024, tmp_dir);
        if	( tmp_dir[strlen(tmp_dir)-1] == '\\' )
			tmp_dir[strlen(tmp_dir)-1] = '\0';
        if	( strlen(tmp_dir) < 4 )	{
        	mkdir("c:\\tmp");
            strcpy(tmp_dir, "c:\\tmp");
        	}
    	}

	// new handles - filenames
	sh2_count ++;
    sprintf(fname_in,  "%s\\shellw2.0x%X.%d.in",  tmp_dir, getpid(), sh2_count);
    sprintf(fname_out, "%s\\shellw2.0x%X.%d.out", tmp_dir, getpid(), sh2_count);
    sprintf(fname_err, "%s\\shellw2.0x%X.%d.err", tmp_dir, getpid(), sh2_count);

	// fill the new stdin
	if	( inbuf && !backg )	{
		if	( (fp = fopen(fname_in, "wt")) != NULL )	{
			fwrite(inbuf, strlen(inbuf), 1, fp);
			fclose(fp);
			}
		}

    if	( !backg )	{
        // create the new standard handles
        new_stdin  = CreateFile(fname_in,  GENERIC_READ,  0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        new_stdout = CreateFile(fname_out, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        new_stderr = CreateFile(fname_err, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }

	// fill stupid structures

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	sap.nLength = sizeof(sap);
	sap.lpSecurityDescriptor = NULL;
	sap.bInheritHandle = TRUE;
	sat.nLength = sizeof(sat);
	sat.lpSecurityDescriptor = NULL;
	sat.bInheritHandle = TRUE;

	// create process attributes (priority, etc).
	attr |= priority;
	attr |= DETACHED_PROCESS;

	// show window args
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	// redirection standard handles
    if	( !backg )	{
		si.hStdInput  = new_stdin;
		si.hStdOutput = new_stdout;
		si.hStdError  = new_stderr;
		si.dwFlags = STARTF_USESTDHANDLES;
        }
    else	{
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.dwFlags = STARTF_USESTDHANDLES;
    	}

	/**** RUN IT ****/
	if	( CreateProcess(NULL, (char *) cmd,
				&sap, &sat,
				TRUE, attr,
				NULL, NULL,
				&si, &pip) )	{

		// wait the process
        if	( !backg )
			while ( WaitForSingleObject(pip.hProcess, 1000) == WAIT_TIMEOUT );

		// ok, the process is finished, close handle

/**
from GetExitCodeProcess():

If the function succeeds, the return value is nonzero.
If the function fails, the return value is zero. To get extended error information, call GetLastError.

Remarks
If the specified process has not terminated, the termination status returned is STILL_ACTIVE.
If the process has terminated, the termination status returned may be one of the following:

* The exit value specified in the ExitProcess or TerminateProcess function.
* The return value from the main or WinMain function of the process.
* The exception value for an unhandled exception that caused the process to terminate.

*/

        if	( !backg )
			GetExitCodeProcess(pip.hProcess, &exit_code);
		CloseHandle(pip.hProcess);
		}
	else
		failed = 1;

	// close files
    if	( !backg )	{
		CloseHandle(new_stdin);
		CloseHandle(new_stdout);
		CloseHandle(new_stderr);
        }

	if	( !failed )	{
		// copy stdout & stderr to output file
		// don't use the ultrashit API ReadFile func
		if	( outbuf && !backg )	{
        	*outbuf = NULL;
			if ( (fp = fopen(fname_out, "rt")) != NULL ) {
            	char	*buf;
                int		size;

            	size = filelength(fileno(fp));
                buf = (char *) tmp_alloc(size+1);
                if	( size )
					fread(buf, size, 1, fp);
                buf[size] = '\0';
                *outbuf = buf;
				fclose(fp);	// STD-C is beautiful.
							// You dont want to see this with Win32-API commands
				}
			} // outbuf
		} // !failed
	else if	( outbuf )
    	*outbuf = NULL;

	//
    if	( !backg )	{
		remove(fname_in);
		remove(fname_out);
		remove(fname_err);
        }

    if	( !failed )
    	return exit_code;
 	return -1;
}

/**
*	w32 run process and capture stdout/stderr using pipes
*	
*	returns a newly allocated string with the result or NULL
*
*	warning: if the cmd is a GUI process, the pw_shell will hang
*/
#define	BUFSIZE	1024
char	*pw_shell(const char *cmd)
{
	HANDLE	h_inppip, h_outpip, h_errpip, h_pid;
  	char	buf[BUFSIZE+1], cv_buf[BUFSIZE+1];
  	char	*result = NULL;
  	int		block_count = 0, bytes;
    
	SECURITY_ATTRIBUTES	sa;
	STARTUPINFO			si;
  	PROCESS_INFORMATION	pi;

	memset(&sa, 0, sizeof(sa));
	sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
  
	if ( !CreatePipe(&h_inppip, &h_outpip, &sa, BUFSIZE) )	
		return NULL;	// failed
  
	h_pid = GetCurrentProcess();
  
  	DuplicateHandle(h_pid, h_inppip, h_pid, &h_inppip, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
    DuplicateHandle(h_pid, h_outpip, h_pid, &h_errpip, 0, TRUE,  DUPLICATE_SAME_ACCESS);

	// run  
	memset(&si, 0, sizeof(si));
  	si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    
    si.hStdOutput = h_outpip;
    si.hStdError = h_errpip;

	if ( CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) )	{
    	
    	// close streams
    	CloseHandle(pi.hThread);
    	CloseHandle(h_outpip);
		CloseHandle(h_errpip);
      	h_errpip = h_outpip = NULL;
    
 		// read stdout/err
		while ( ReadFile(h_inppip, buf, BUFSIZE, &bytes, NULL) )	{

        	buf[bytes] = '\0';
        	memset(cv_buf, 0, BUFSIZE+1);
        	OemToCharBuff(buf, cv_buf, bytes);
      
      		block_count ++;
      		if	( result )
      			result = (char *) tmp_realloc(result, block_count * BUFSIZE + 1);
      		else	{
      			result = (char *) tmp_alloc(BUFSIZE + 1);
      			*result = '\0';
      			}
      			
      		strcat(result, cv_buf);      		
			}      
    
    	//
    	CloseHandle(pi.hProcess);
		}
	else
		result = NULL;	// could not run it
   
    
  	// clean up
  	CloseHandle(h_inppip);
  	if ( h_outpip ) CloseHandle(h_outpip);
  	if ( h_errpip ) CloseHandle(h_errpip);
  	
  	return result;
}
#undef BUFSIZE

#endif

/*
*	run a program (if retflg wait and return; otherwise just exec())
*/
int		dev_run(const char *src, int retflg)
{
	#if defined(_PalmOS)
	LocalID	lid;
	dword	progid;
	word	card;
	DmSearchStateType	state;

	progid  = ((dword) src[0] << 24) + ((dword) src[1] << 16) + ((dword) src[2] << 8) + (dword) src[3];
	if	( DmGetNextDatabaseByTypeCreator(true, &state, 0x6170706C, progid, true, &card, &lid) == 0 )
		return (SysUIAppSwitch(card, lid, sysAppLaunchCmdNormalLaunch, NULL) == 0);
	return 0;
    #elif defined(_VTOS) || defined(_FRANKLIN_EBM)
	return 0;
    #elif defined(_Win32)
	int		r;
    char	*out;
    
	r = ((out = pw_shell(src)) != NULL);
    if	( r )	tmp_free(out);
    
	if	( r && !retflg )
		exit(1);	// ok, what to do now ?
	return r;
	#else
	if	( retflg )
		return (system(src) != -1);
	else	{
		execl(src, src, NULL);
		return 0;
		}
	#endif
}

/*
*	ENVIRONMENT VARIABLES
*/

/**
*	GNU Manual:
*
*	The  putenv() function adds or changes the value of environment variables.  The argument string is of the form name=value.
*	If name does not already exist in the environment, then string is added to the environment.  If name does exist, then  the
*	value of name in the environment is changed to value.  The string pointed to by string becomes part of the environment, so
*	altering the string changes the environment.
*
*	The putenv() function returns zero on success, or -1 if an error occurs.
*
*	SmallBASIC:
*	If the value is zero-length then the variable must be deleted. (libc4,libc5 compatible version)
*/
int		dev_putenv(const char *str)
{
	#if defined(_VTOS)
	return -1;
	#elif defined(_PalmOS)
	return putenv(str);
	#else
	char	*p;
	
	p = strdup(str);	// no free()
	return putenv(p);
	#endif
}

/**
*	GNU Manual:
*
*	The  getenv() function searches the environment list for a string that matches the string pointed to by name. The strings
*	are of the form name = value.
*
*	The getenv() function returns a pointer to the value in the environment, or NULL if there is no match.
*/
char	*dev_getenv(const char *str)
{
	#if defined(_VTOS)
	return -1;
	#else
	return getenv(str);
	#endif
}

#ifndef _FRANKLIN_EBM

/*
*	returns the number of environment variables
*/
int		dev_env_count()
{
	#if defined(_PalmOS) 
	return dbt_count(env_table);
	#elif defined(_VTOS)
	return 0;
	#else
	int		count = 0;

	while ( environ[count] )	count ++;
	return count;
	#endif
}

/*
*	returns the value of the n-th system's environment variable
*/
char	*dev_getenv_n(int n)
{
	#if defined(_PalmOS) 
	dbt_var_t	nd;
	char		*buf;
	char		*retptr;

	if	( n < dev_env_count() )	{
		dbt_read(env_table, n, &nd, sizeof(nd));
		buf = tmp_alloc(nd.node_len);
		dbt_read(env_table, n, buf, nd.node_len);
		retptr = dev_getenv(buf+sizeof(nd));	// use getenv's static-buffer
		tmp_free(buf);
		return retptr;
		}
	return NULL;
	#elif defined(_VTOS)
	return NULL;
	#else
	int		count = 0;

	while ( environ[count] )	{
		if	( n == count )
			return environ[count];
		count ++;
		}
	return NULL;
	#endif
}

// empty implementations
void dev_html(const char* html, const char* title, int x, int y, int w, int h) {
}

void dev_image(int handle, int index, int x, int y, 
               int sx, int sy, int w, int h) {
}

int dev_image_width(int handle, int index) {
    return -1;
}

int dev_image_height(int handle, int index) {
    return -1;
}

#endif

