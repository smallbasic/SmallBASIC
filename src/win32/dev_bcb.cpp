/*
*	win32 native - bcb
*
*	Written by Nicholas Christopoulos
*/

#include <vcl.h>
#include <stdio.h>
#include "sbpad_bcb.h"
#include <windows.h>
#include "device.h"
#include "osd.h"
#include "str.h"

// dev_winsnd.cpp:
#include "drvsound.h"

//typedef unsigned short int	word;

typedef int* int_ptr_t;

static dword	dev_width, dev_height, dev_depth;

static int	mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;

static TCanvas	*canvas;
static int_ptr_t *video;
static TColor	cmap[16];
static TColor	dcolor, dbgcolor;
static int		font_h = 16, font_w = 8, maxline = 11;

static int		bcb_break_flag = 0;

// console/fonts
static int	cur_x = 0;
static int	cur_y = 0;
static int	con_use_bold = 0;
static int	con_use_ul   = 0;
static int	con_use_reverse = 0;

static int	bcb_refresh_rq;

// EGA/VGA16 colors in RGB
//static dword vga16[] =
//{
//0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
//0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
//};

// blue-green-red
static dword vga16[] =
{
0x0, 0x7F0000, 0x7F00, 0x7F7F00, 0x7F, 0x7F007F, 0x007F7F, 0x808080,
0x555555, 0xFF0000, 0xFF00, 0xFFFF00, 0xFF, 0xFF00FF, 0xFFFF, 0xFFFFFF
};

extern String bcb_xf;
extern void	bcb_strfont(String s, String &name, int &size, int &charset);
extern void* _cdecl w32_video_ptr(int line);

#define SWAPRB(c)	( (((c) & 0xFF) << 16) | ((c) & 0xFF00) | (((c) & 0xFF0000) >> 16) )

/*
*	write vram to window
*/
void _cdecl	osd_refresh()
{
	if	( bcb_refresh_rq )	{
		w32_invalidate_rect();
		bcb_refresh_rq = 0;
		Application->ProcessMessages();
		}
}

/*
*	update font - global values
*/
void	osd_updateFont()
{
	TSize	size;

	size = canvas->TextExtent("X");
	font_w = size.cx;
	font_h = size.cy;
	maxline = dev_height / font_h;
}

/*
*/
void w32_stdout_open()
{
	int		i, n, cs;
	String	s;
	TRect	r;

	canvas = w32_canvas();

	r = canvas->ClipRect;
	dev_width  = (r.right - r.left);
	dev_height = (r.bottom - r.top);
	dev_depth  = 32;

	// build pens
	for ( i = 0; i < 16; i ++ )	
		cmap[i] = (TColor) vga16[i];
	dbgcolor = cmap[15];

	// select font
	canvas->Brush->Color = (TColor) 0xFFFFFF;
	bcb_strfont(bcb_xf, s, n, cs);

	canvas->Font->Name = s;
	canvas->Font->Size = n;
	canvas->Font->Charset = cs;
	cur_x = cur_y = 0;
	osd_updateFont();

	//
	os_graf_mx     = dev_width;
	os_graf_my     = dev_height;
	os_color_depth = dev_depth;
	os_color       = (dev_depth >= 8);

	video = (int_ptr_t *) malloc(dev_height * sizeof(int_ptr_t));
	for ( i = 0; i < dev_height; i ++ )
		video[i] = (int_ptr_t) w32_video_ptr(i);

	drvsound_init();
	w32_evstate(1);

	osd_settextcolor(0, 15);
	osd_cls();
}

void  w32_stdout_close()
{
	w32_evstate(0);
	drvsound_close();
	free(video);
}

/*
*	SB: Initialization
*/
int _cdecl		osd_devinit()
{
	os_graf_mx     = dev_width;
	os_graf_my     = dev_height;
	os_color_depth = dev_depth;
	os_color       = (dev_depth >= 8);
    
	osd_settextcolor(0, 15);
	osd_cls();
	return 1;
}

/*
*	close
*/
int _cdecl		osd_devrestore()
{
//	bcb_refresh_rq = 1;
//	osd_refresh();
	return 1;
}

// getpixel
long  _cdecl	osd_getpixel(int x, int y)
{
	TColor	color;
	int			i;

	color = canvas->Pixels[x][y];
//	color = video[y][x];
	for ( i = 0; i < 16; i ++ )	{
		if	( cmap[i] == color )
			return i;
		}
	return color;
}

// setpixel
void  _cdecl	osd_setpixel(int x, int y)
{
	video[y][x] = dcolor;
//	canvas->Pixels[x][y] = dcolor;
	bcb_refresh_rq = 1;
}

// cls
void _cdecl	osd_cls()
{
	int		i, mw;

	cur_x = cur_y = 0;
	TRect r = canvas->ClipRect;
//	canvas->Brush->Color = (TColor) dbgcolor;
//	canvas->FillRect(r);
	mw = dev_width * 4;
	for ( i = 0; i < dev_height; i ++ )	
		memset(video[i], 0xff, mw);
	bcb_refresh_rq = 1;
}

// position
int	 _cdecl	osd_getx()
{	return cur_x;	}

int		osd_gety()
{	return cur_y;	}

void _cdecl	osd_setxy(int x, int y)
{
	cur_x = x;
	cur_y = y;
}

/* */
static void direct_fillrect(int x1, int y1, int x2, int y2)
{
	TRect		r;

	r.left = x1; 	r.right  = x2 + 1;
	r.top  = y1;	r.bottom = y2 + 1;
    canvas->Brush->Color = (TColor) dcolor;
	canvas->FillRect(r);
}

/*	next line */
static void	osd_nextln(void)
{
	cur_x = 0;

	if	( cur_y < ((maxline-1) * font_h) )	
		cur_y += font_h;
	else	{
		TRect	rs, rd;

        rd.left = rd.top = 0;
        rd.right = dev_width - 1;
        rd.bottom = (dev_height - font_h) - 1;

        rs.left = 0;  rs.top = font_h;
        rs.right = dev_width - 1;
        rs.bottom = dev_height - 1;

		// scroll
		canvas->CopyRect(rd, canvas, rs);

		// clear
		rs.left = 0;
		rs.right = dev_width - 1;
		rs.top = cur_y;
		rs.bottom = dev_height - 1;
		canvas->Brush->Color = (TColor) dbgcolor;
		canvas->FillRect(rs);
		}
}

/*	calc next tab position */
static int	osd_calctab(int x)
{
	int		c = 1;

	while ( x > 32 )	{
		x -= 32;
		c ++;
		}
	return c * 32;
}

/*	drawing character */
static void	direct_drawchar(int x, int y, byte ch, int overwrite, int fg_rgb, int bg_rgb)
{
	char	buf[2];

	if	( overwrite )	{
		buf[0] = ch;
		buf[1] = '\0';
		canvas->TextOut(x, y, buf);
		}
	// else // bold
}

/**
*	OSD API:
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
void _cdecl	osd_write(const char *str)
{
	int		len, cx = 8, esc_val, esc_cmd;
	byte	*p, buf[3];
	TColor	color = dcolor;
	String	s;
	int		n, cs;

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
				case	'K':			// \e[K - clear to eol
					dcolor = cmap[15];
					direct_fillrect(cur_x, cur_y, dev_width - cur_x, cur_y+font_h);
					dcolor = color;
					break;
				case	'G':
					dev_setxy(esc_val*8, dev_gety());	// default font = 9x16
					break;
				case	'm':			// \e[...m	- ANSI terminal
					switch ( esc_val )	{
					case	0:	// reset
						bcb_strfont(bcb_xf, s, n, cs);
						canvas->Font->Name = s;
						canvas->Font->Size = n;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles();
						osd_updateFont();

						con_use_bold = 0;
						con_use_ul = 0;
						con_use_reverse = 0;
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

					// fonts - palm emu
					case	80:	// normal fonts
						bcb_strfont(bcb_xf, s, n, cs);
						canvas->Font->Name = s;
						canvas->Font->Size = n;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles();
						osd_updateFont();
						break;
					case	81:	// bold fonts
						bcb_strfont(bcb_xf, s, n, cs);
						canvas->Font->Name = s;
						canvas->Font->Size = n;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles() << fsBold;
						osd_updateFont();
						break;
					case	82:	// large fonts
					case	83:	case	84:	case	85:
						bcb_strfont(bcb_xf, s, n, cs);
						canvas->Font->Name = s;
						canvas->Font->Size = n * 1.4;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles();
						osd_updateFont();
						break;
					case	87:	// large & bold fonts
					case	86:
						bcb_strfont(bcb_xf, s, n, cs);
						canvas->Font->Name = s;
						canvas->Font->Size = n * 1.4;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles() << fsBold;
						osd_updateFont();
						break;
					case	90:	// custom - smallest fonts
						bcb_strfont(bcb_xf, s, n, cs);
//						canvas->Font->Name = "Courier New";
						canvas->Font->Size = n / 1.4;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles();
						osd_updateFont();
						break;
					case	91:	// custom - small - non-fixed fonts
						bcb_strfont(bcb_xf, s, n, cs);
//						canvas->Font->Name = "Trebuchet MS";
						canvas->Font->Size = n / 1.2;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles();
						osd_updateFont();
						break;
					case	92:	// custom - small - fixed fonts
						bcb_strfont(bcb_xf, s, n, cs);
//						canvas->Font->Name = "Courier New";
						canvas->Font->Size = n / 1.1;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles();
						osd_updateFont();
						break;
					case	93:	// custom - small - fixed - condensed fonts
						bcb_strfont(bcb_xf, s, n, cs);
//						canvas->Font->Name = "Courier New";
						canvas->Font->Size = n / 1.1;
						canvas->Font->Charset = cs;
						canvas->Font->Style = TFontStyles();
						osd_updateFont();
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
			dcolor = cmap[15];
			direct_fillrect(cur_x, cur_y, dev_width - cur_x, cur_y+font_h);
			dcolor = color;
			break;
		default:
			//
			//	PRINT THE CHARACTER
			//
			buf[0] = *p;
			buf[1] = '\0';
			cx = osd_textwidth(buf);
//			cx = font_w;

			// new line ?
			if	( cur_x + cx >= (int) dev_width )
				osd_nextln();

			// draw

			// TODO: ??? SJIS on Linux ???
			if	( !con_use_reverse )	{
				if	( con_use_bold )	canvas->Font->Style = TFontStyles() << fsBold;
				direct_drawchar(cur_x, cur_y, *p, 1, dcolor, dbgcolor);
				if	( con_use_bold )	canvas->Font->Style = TFontStyles() >> fsBold;
				}
			else	{
				if	( con_use_bold )	canvas->Font->Style = TFontStyles() << fsBold;
				direct_drawchar(cur_x, cur_y, *p, 1, dbgcolor, dcolor);
				if	( con_use_bold )	canvas->Font->Style = TFontStyles() >> fsBold;
				}

			if	( con_use_ul )	{
				osd_setcolor(dev_fgcolor);
				osd_line(cur_x, (cur_y+font_h)-1, cur_x+cx, (cur_y+font_h)-1);
				}

			// advance
			cur_x += cx;
			};

		if	( *p == '\0' )
			break;

		p ++;
		}
	bcb_refresh_rq = 1;
}

/* */
void _cdecl	osd_setcolor(long color)
{
	if	( color >= 0 && color < 16 )	{
		dev_fgcolor = color;
		dcolor = cmap[dev_fgcolor];
		}
	else if ( color < -1 )	
		dcolor = SWAPRB(-color);
	else if ( color > 16 )	
		dcolor = SWAPRB(color);

	canvas->Pen->Color = (TColor) dcolor;
	canvas->Font->Color = (TColor) dcolor;
}

/*	Sets the current drawing color and the background color (used for text) */
void  _cdecl	osd_settextcolor(long fg, long bg)
{
	osd_setcolor(fg);
	if	( bg >= 0 && bg < 16 )	{
		dev_bgcolor = bg;
		dbgcolor = cmap[dev_bgcolor];
		}
	else if ( bg < -1 )
		dbgcolor = SWAPRB(-bg);
	else if ( bg > 16 )
		dbgcolor = SWAPRB(bg);

	canvas->Brush->Color = dbgcolor;
}

// line
void  _cdecl	osd_line(int x1, int y1, int x2, int y2)
{
	canvas->MoveTo(x1, y1);
//	canvas->Pixels[x1][y1] = dcolor;
	video[y1][x1] = dcolor;
	canvas->LineTo(x2, y2);
	bcb_refresh_rq = 1;
}

// rect
void  _cdecl	osd_rect(int x1, int y1, int x2, int y2, int fill)
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
	bcb_refresh_rq = 1;
}

/* returns the width of the text str in pixels */
int	 _cdecl	osd_textwidth(const char *str)
{
	TSize	size;

	size = canvas->TextExtent(str);
	return size.cx;
}

/* returns the height of the text str in pixels */
int	 _cdecl	osd_textheight(const char *str)
{
	return font_h;
}

/*	enable or disable PEN code */
void  _cdecl	osd_setpenmode(int enable)
{
 	mouse_mode = enable;
}

/* */
int	 _cdecl	osd_getpen(int code)
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
			r = mouse_b & 1;
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
			r = (mouse_b & (1 << (code - 11))) ? 1 : 0;
			break;
			}

		mouse_upd = 0;
		}
	return	r;
}

/*
*/
int	 _cdecl	osd_events_wait(int wait_flag)
{
	int		r = 0;

	drvsound_event();

	do	{
		event_t	ev;

		while ( w32_evget(&ev) )	{
			r ++;

			if	( ev.type == EVMOUSE )	{
				// mouse
				mouse_x = ev.x;
				mouse_y = ev.y;

				mouse_b = 0;
				if ( ev.button & 1 )	{
					if	( (mouse_b & 1) == 0 )	{	// new press
						mouse_down_x = mouse_x;
						mouse_down_y = mouse_y;
						}

					mouse_upd = 1;

					mouse_pc_x = mouse_x;
					mouse_pc_y = mouse_y;

					mouse_b |= 1;
					}
				if ( ev.button & 2 )
					mouse_b |= 2;
				if ( ev.button & 4 )
					mouse_b |= 4;
				}
			else	{
				// keyboard
				if	( ev.ch == 3 )	// CTRL+C (break)
					return -2;
				if	( ev.ch == '\r' )
					dev_pushkey('\n');
				else
					dev_pushkey(ev.ch);
				}
			}

		if	( r == 0 )	
	    	Application->ProcessMessages();

		} while ( wait_flag && (r == 0) );

	return r;
}

int	 _cdecl	osd_events()
{
	if	( bcb_break_flag )
		return -2;
	return osd_events_wait(0);
}

void osd_bcb_breakoff()
{ bcb_break_flag = 0; }

void osd_bcb_breakon()
{ bcb_break_flag = 1; }

//////////

void  _cdecl	osd_beep()
{
	drvsound_beep();
}

void  _cdecl	osd_sound(int frq, int dur, int vol, int bgplay)
{
	drvsound_sound(frq, dur, vol, bgplay);
}

void  _cdecl	osd_clear_sound_queue()
{
	drvsound_clear_queue();
}
