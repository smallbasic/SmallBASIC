/*
*	DOS GRX driver (DJGPP)
*
*	Written by Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include <stdio.h>
#include <unistd.h>
#include <grx20.h>
#include <grxkeys.h>
#include <conio.h>
#include "device.h"
#include "osd.h"
#include "str.h"

//typedef unsigned short int	word;

static dword	dcolor;

static int	cur_x = 0;
static int	cur_y = 0;
static int	bytespp = 1;
static int	mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;
static int	tabsize = 32;	// from dev_palm
static int	maxline;
static int	font_h = 16;

static int	con_use_bold = 0;
static int	con_use_ul   = 0;
static int	con_use_reverse = 0;

static int	dev_mouse;

// VGA16 colors in RGB
static GrColor *cmap;

// fonts
#include "unix/rom16.c"

#define	TPS		CLOCKS_PER_SEC

/*
*	SB: Initialization
*/
int		osd_devinit()
{
	char	*v, *p;

	if	( getenv("GRX20DRV") != NULL )	{
		if	( !GrSetMode(GR_default_graphics) )
			panic("VGA: User's settings (GRX20DRV) failed");
		}	
	else	{
		if	( (v = getenv("SBGRAF")) != NULL )	{
			int		x, y, b;
			char	var[64];
			
			strcpy(var, v);
			p = strchr(var, 'x');
			if	( !p )	{
				fprintf(stderr, "VGA: SBGRAF variable usage:<width>x<height>x<bits-per-pixel>\nExample:\nset SBGRAF=640x480x4\n");
				return 0;
				}
			*p = '\0';
			x = atoi(var);
			p ++; v = p;
			
			p = strchr(var, 'x');
			if	( !p )	{
				fprintf(stderr, "VGA: SBGRAF variable usage:<width>x<height>x<bits-per-pixel>\nExample:\nset SBGRAF=640x480x4\n");
				return 0;
				}
			*p = '\0';
			y = atoi(v);
			p ++; v = p;
				
			b = atoi(v);
			
			if	( !GrSetMode(GR_width_height_bpp_graphics, x, y, b) )
				panic("VGA: Cannot setup mode %dx%dx%d", x, y, b);
			}
		else	{
			// standard VGA (default)
			if	( GrSetMode(GR_width_height_bpp_graphics, 640, 480, 4) == 0 )
				panic("VGA: Cannot setup standard VGA");
			}
		}

	// 
	os_graf_mx     = GrMaxX()+1;
	os_graf_my     = GrMaxY()+1;
	os_color_depth = 4;
	os_color       = (os_color_depth >= 8);
	bytespp        = os_color_depth / 8;
	maxline        = os_graf_my / font_h;
	
	cmap = GrAllocEgaColors();
	
	dev_mouse = GrMouseDetect();
	if	( dev_mouse )	{
		GrMouseInit();
//		GrMouseSetCursorMode(M_CUR_NORMAL);
		GrMouseEventEnable(0,1);
		GrMouseDisplayCursor();
		}

	osd_cls();
	return 1;
}

/*
*	close
*/
int		osd_devrestore()
{
	cur_x = 0;
	cur_y = (maxline - 1) * font_h;
	osd_write("VGA: Press any key to exit...");
	
	getch();

	if	( dev_mouse )	{
		GrMouseUnInit();
		}

	GrSetMode(GR_default_text);
	return 1;
}

/*
*/
void	osd_refresh()
{
}

/*
*/
long	osd_getpixel(int x, int y)
{
	int		i;
	long	color;

	color = GrPixel(x, y);

	for ( i = 0; i < 16; i ++ )	{
		if	( color == cmap[i] )	
			return i;
		}
	return color;
}

void	direct_line(int x1, int y1, int x2, int y2)
{
	GrLine(x1, y1, x2, y2, dcolor);
}

void	direct_fillrect(int x1, int y1, int x2, int y2)
{
	int		i;

	for ( i = y1; i <= y2; i ++ )	
		GrHLine(x1, x2, i, dcolor);
}

/*
*	drawing bitmap character (8x16)
*/
void	osd_drawchar(int x, int y, byte ch, int overwrite, int fg_rgb, int bg_rgb)
{
	byte	*char_offset;
	int		bit, i;

	char_offset = font8x16 + ch * 16;
	x ++;

	for ( i = 0; i < 16; i ++, char_offset ++ )	{
		for ( bit = 0; bit < 8; bit ++ )	{
			if	( *char_offset & (1 << (8-bit)) )	
				GrPlot(x+bit, y+i, fg_rgb);
			else if ( overwrite )	
				GrPlot(x+bit, y+i, bg_rgb);
			}
		}
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

//
void	osd_cls()
{
	cur_x = cur_y = 0;
	GrClearScreen(cmap[15]);
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
	long	color = dcolor;

	cur_x = 0;
	if	( cur_y < ((maxline-1) * font_h) )	
		cur_y += font_h;
	else	{
		// scroll
		GrImage	*img = GrImageFromContext(GrCurrentContext());
		GrImageDisplay(0, -font_h, img);
		GrImageDestroy(img);
		
		dcolor = cmap[15];
		direct_fillrect(0, cur_y, os_graf_mx-1, cur_y+font_h);
		dcolor = color;
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
	int		len, cx = 8, esc_val, esc_cmd;
	byte	*p, buf[3];
	long	color = dcolor;

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

				while ( is_digit(*p) )	{
					esc_val = (esc_val * 10) + (*p - '0');
					p ++;
					}
				esc_cmd = *p;

				// control characters
				switch ( esc_cmd )	{
				case	'K':			// \e[K - clear to eol
					dcolor = cmap[15];
					direct_fillrect(cur_x, cur_y, os_graf_mx - cur_x, cur_y+font_h);
					dcolor = color;
					break;
				case	'G':
					dev_setxy(esc_val*8, dev_gety());	// default font = 9x16
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

					case	40:	// set black bg
						osd_settextcolor(cmap[dev_fgcolor], 0);
						break;
					case	41:	// set red bg
						osd_settextcolor(cmap[dev_fgcolor], 4);
						break;
					case	42:	// set green bg
						osd_settextcolor(cmap[dev_fgcolor], 2);
						break;
					case	43:	// set brown bg
						osd_settextcolor(cmap[dev_fgcolor], 6);
						break;
					case	44:	// set blue bg
						osd_settextcolor(cmap[dev_fgcolor], 1);
						break;
					case	45:	// set magenta bg
						osd_settextcolor(cmap[dev_fgcolor], 5);
						break;
					case	46:	// set cyan bg
						osd_settextcolor(cmap[dev_fgcolor], 3);
						break;
					case	47:	// set white bg
						osd_settextcolor(cmap[dev_fgcolor], 7);
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
			direct_fillrect(cur_x, cur_y, os_graf_mx - cur_x, cur_y+font_h);
			dcolor = color;
			break;
		default:
			//
			//	PRINT THE CHARACTER
			//
			buf[0] = *p;
			buf[1] = '\0';

			// new line ?
			if	( cur_x + cx >= os_graf_mx )
				osd_nextln();

			// draw

			// TODO: ??? SJIS on Linux ???
			if	( !con_use_reverse )	{
				osd_drawchar(cur_x, cur_y, *p, 1, cmap[dev_fgcolor], cmap[dev_bgcolor]);
				if	( con_use_bold )	
					osd_drawchar(cur_x-1, cur_y, *p, 0, cmap[dev_fgcolor], cmap[dev_bgcolor]);
				}
			else	{
				osd_drawchar(cur_x, cur_y, *p, 1, cmap[dev_bgcolor], cmap[dev_fgcolor]);
				if	( con_use_bold )	
					osd_drawchar(cur_x-1, cur_y, *p, 0, cmap[dev_bgcolor], cmap[dev_fgcolor]);
				}

			if	( con_use_ul )	{
				osd_setcolor(dev_fgcolor);
				direct_line(cur_x, (cur_y+font_h)-1, cur_x+cx, (cur_y+font_h)-1);
				}

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
	dcolor = cmap[dev_fgcolor];
}

void	osd_settextcolor(long fg, long bg)
{
	osd_setcolor(fg);
	if	( bg != -1 )
		dev_bgcolor = bg;
}

void	osd_line(int x1, int y1, int x2, int y2)
{
	if	( (x1 == x2) && (y1 == y2) )
		GrPlot(x1, y1, dcolor);
	else
		direct_line(x1, y1, x2, y2);
}

void	osd_setpixel(int x, int y)
{
	GrPlot(x, y, dcolor);
}

void	osd_rect(int x1, int y1, int x2, int y2, int fill)
{
	int		y;

	if	( fill )	{
		for ( y = y1; y <= y2; y ++ )	
			GrHLine(x1, x2, y, dcolor);
		}
	else	{
		direct_line(x1, y1, x1, y2);
		direct_line(x1, y2, x2, y2);
		direct_line(x2, y2, x2, y1);
		direct_line(x2, y1, x1, y1);
		}
}

int		osd_textwidth(const char *str)
{
	int		l = strlen(str);

	// SJIS ???
	return l * 8;
}

int		osd_textheight(const char *str)
{
	// TODO: count \n
	return font_h;
}

///////////////////////////////////////////////////////////////

/*
*/
int		key_events(int wait_flag)
{
	int		c;

	if	( kbhit() || wait_flag )	{
		c = getch();
		if	( c == SB_KEY_BREAK )	// CTRL+C (break)
			return -2;
		dev_pushkey(c);
		return 1;
		}
	return 0;
}

/*
*/
int		osd_events(int wait_flag)
{
	do	{
		int		r;

		if	( dev_mouse )	{
			
			if	( GrMousePendingEvent() )	{
				GrMouseEvent	ev;
				
				GrMouseGetEvent(GR_M_EVENT, &ev);
				mouse_x = ev.x;
				mouse_y = ev.y;
				
				if ( ev.buttons & 1 )	{
					if	( (mouse_b & 1) == 0 )	{	// new press
						mouse_down_x = mouse_x;
						mouse_down_y = mouse_y;
						}
	
					mouse_upd = 1;
	
					mouse_pc_x = mouse_x;
					mouse_pc_y = mouse_y;

					mouse_b |= 1;
					}	// ev.b
					
				if ( ev.buttons & 2 )
					mouse_b |= 2;
				if ( ev.buttons & 4 )
					mouse_b |= 4;
				return 1;
				} // p
			}
				
		if	( (r = key_events(0)) != 0 )
			return r;

		} while ( wait_flag );

	return 0;
}

//////////

#if !defined(DRV_SOUND)
void	osd_beep()
{
}

void	osd_clear_sound_queue()
{
}

void	osd_sound(int frq, int dur, int vol, int bgplay)
{
}
#endif


