/*
*	win32 native
*
*	Written by Nicholas Christopoulos
*/

#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include "device.h"
#include "osd.h"
#include "str.h"
#include "dev_genfb.h"
#include "w32_main.h"

//typedef unsigned short int	word;

static dword	dev_width, dev_height, dev_depth;

static int	mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;

static HDC		hdc;
static HBITMAP 	hbmp;
static HPEN		pen[16], oldpen;
static COLORREF	cmap[16];
static COLORREF	dcolor;
static HFONT	font, fontbold, oldfont;
static int		font_h = 16, font_w = 8, maxline = 11;

// console/fonts
static int	cur_x = 0;
static int	cur_y = 0;
static int	con_use_bold = 0;
static int	con_use_ul   = 0;
static int	con_use_reverse = 0;

static int	update;

// EGA/VGA16 colors in RGB
static dword vga16[] =
{
0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF 
};

/*
*	write vram to window
*/
void	osd_refresh()
{
	if	( update )	{
		RECT	r;
		GetClientRect(w32_getwindow(), &r);
		InvalidateRect(w32_getwindow(), &r, 0);
		update = 0;
		}
}

/*
*/
void	w32_stdout_open()
{
	int		i;
	HWND	wnd;
	RECT	r;
	SIZE	fsize;

	wnd = w32_getwindow();
	GetClientRect(wnd, &r);
	
	dev_width  = (r.right - r.left) + 1;
	dev_height = (r.bottom - r.top) + 1;
	dev_depth  = 8;

	hdc = w32_getdc();
				
	// build pens
	for ( i = 0; i < 16; i ++ )	{
		int	r,g,b;
		r = (vga16[i] & 0xFF0000) >> 16;
		g = (vga16[i] & 0xFF00) >> 8;
		b = (vga16[i] & 0xFF);
		cmap[i] = RGB(r,g,b);
		pen[i] = CreatePen(PS_SOLID, 0, cmap[i]);
		}
	oldpen = (HPEN) SelectObject(hdc, pen[0]);

	// select font
	font = CreateFont(-11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Courier");
	fontbold = CreateFont(-11, 0, 0, 0, 700, 0, 0, 0, 0, 0, 0, 0, 0, "Courier");
//	font = GetStockObject(SYSTEM_FIXED_FONT);
	oldfont = (HFONT) SelectObject(hdc, font);
	GetTextExtentPoint32(hdc, "X", 1, &fsize);
	cur_x = cur_y = 0;
	font_w = fsize.cx;
	font_h = fsize.cy;
	maxline = dev_height / font_h;

	// 
	os_graf_mx     = dev_width;
	os_graf_my     = dev_height;
	os_color_depth = dev_depth;
	os_color       = (dev_depth >= 8);

	w32_evstate(0);

	osd_settextcolor(0, 15);
	osd_cls();
}

/*
*	SB: Initialization
*/
int		osd_devinit()
{
	w32_evstate(1);
	osd_settextcolor(0, 15);
	osd_cls();
	return 1;
}

/*
*	close
*/
int		osd_devrestore()
{
	update = 1;
	osd_refresh();
	w32_evstate(0);
	return 1;
}

void	w32_stdout_close()
{
	int		i;

	w32_evstate(0);
	// delete DC
	SelectObject(hdc, oldfont);
	SelectObject(hdc, oldpen);

	DeleteObject(font);
	DeleteObject(fontbold);

	// delete pens
	for ( i = 0; i < 16; i ++ )	
		DeleteObject(pen[i]);
	return 1;
}

// getpixel
long	osd_getpixel(int x, int y)
{
	COLORREF	color;
	int			i;

	color = GetPixel(hdc, x, y);
	for ( i = 0; i < 16; i ++ )	{
		if	( cmap[i] == color )
			return i;
		}
	return color;
}

// setpixel
void	osd_setpixel(int x, int y)
{
	SetPixel(hdc, x, y, dcolor);
	update = 1;
}

// cls
void	osd_cls()
{
	RECT	r;
	HWND	wnd;
	HBRUSH	brush;

	cur_x = cur_y = 0;
	wnd = w32_getwindow();
	GetClientRect(wnd, &r);
	brush = CreateSolidBrush(cmap[dev_bgcolor]);
	FillRect(hdc, &r, brush);
	DeleteObject(brush);
	update = 1;
}

// position
int		osd_getx()
{	return cur_x;	}

int		osd_gety()
{	return cur_y;	}

void	osd_setxy(int x, int y)
{
	cur_x = x;
	cur_y = y;
}

/* */
static void direct_fillrect(int x1, int y1, int x2, int y2)
{
	RECT		r;
	HBRUSH		brush;

	r.left = x1; 	r.right  = x2 + 1;
	r.top  = y1;	r.bottom = y2 + 1;
	brush = CreateSolidBrush(dcolor);
	FillRect(hdc, &r, brush);
	DeleteObject(brush);
}

/*	next line */
static void	osd_nextln()
{
	cur_x = 0;

	if	( cur_y < ((maxline-1) * font_h) )	
		cur_y += font_h;
	else	{
		RECT	r;
		HBRUSH	brush;

		// scroll
		BitBlt(
			hdc, 0, 0, dev_width, dev_height - font_h,
			hdc, 0, font_h, SRCCOPY);

		// clear
		r.left = 0;
		r.right = dev_width;
		r.top = cur_y;
		r.bottom = dev_height;
		brush = CreateSolidBrush(cmap[dev_bgcolor]);
		FillRect(hdc, &r, brush);
		DeleteObject(brush);
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
		TextOut(hdc, x, y, buf, 1);
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
void	osd_write(const char *str)
{
	int		len, cx = 8, esc_val, esc_cmd;
	byte	*p, buf[3];
	long	color = dcolor;
	HFONT	prev_font;

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
				case	'm':			// \e[...m	- ANSI terminal
					switch ( esc_val )	{
					case	0:	// reset
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
//			cx = osd_textwidth(buf);
			cx = font_w;

			// new line ?
			if	( cur_x + cx >= dev_width )
				osd_nextln();

			// draw

			// TODO: ??? SJIS on Linux ???
			if	( !con_use_reverse )	{
				if	( con_use_bold )	prev_font = SelectObject(hdc, fontbold);
				direct_drawchar(cur_x, cur_y, *p, 1, cmap[dev_fgcolor], cmap[dev_bgcolor]);
				if	( con_use_bold )	SelectObject(hdc, prev_font);
				}
			else	{
				if	( con_use_bold )	prev_font = SelectObject(hdc, fontbold);
				direct_drawchar(cur_x, cur_y, *p, 1, cmap[dev_bgcolor], cmap[dev_fgcolor]);
				if	( con_use_bold )	SelectObject(hdc, prev_font);
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
	update = 1;
}

/* */
void	osd_setcolor(long color)
{
	if	( color >= 0 && color < 16 )	{
		dev_fgcolor = color;
		dcolor = cmap[dev_fgcolor];

		SetTextColor(hdc, dcolor);
		SelectObject(hdc, pen[color]);
		}
}

/*	Sets the current drawing color and the background color (used for text) */
void	osd_settextcolor(long fg, long bg)
{
	osd_setcolor(fg);
	if	( bg >= 0 && bg < 16 )	{
		dev_bgcolor = bg;
		SetBkColor(hdc, cmap[dev_bgcolor]);
		}
}

// line
void	osd_line(int x1, int y1, int x2, int y2)
{
	POINT	pt;

	MoveToEx(hdc, x1, y1, &pt);
	SetPixel(hdc, x1, y1, dcolor);
	LineTo(hdc, x2, y2);
	update = 1;
}

// rect
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
	update = 1;
}

/* returns the width of the text str in pixels */
int		osd_textwidth(const char *str)
{
	SIZE	fsize;

	GetTextExtentPoint32(hdc, str, strlen(str), &fsize);
	return fsize.cx;
}

/* returns the height of the text str in pixels */
int		osd_textheight(const char *str)
{
	return font_h;
}

/*	enable or disable PEN code */
void	osd_setpenmode(int enable)
{
 	mouse_mode = enable;
}

/* */
int		osd_getpen(int code)
{
	int		r = 0;

	osd_events(0);	
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
			r = 0;
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
int		osd_events(int wait_flag)
{
	int		r = 0;

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
				if	( ev.ch == SB_KEY_BREAK )	// CTRL+C (break)
					return -2;
				if	( ev.ch == '\r' )
					dev_pushkey('\n');
				else
					dev_pushkey(ev.ch);
				}
			}

		} while ( wait_flag && (r == 0) );

	return r;
}

//////////

#if !defined(DRV_SOUND)

void	osd_beep()
{
	osd_sound(440, 125, 75, 0);
}

void	osd_sound(int frq, int dur, int vol, int bgplay)
{
	if	( !bgplay )
		Beep(frq, dur);
}

void	osd_clear_sound_queue()
{
}
#endif

