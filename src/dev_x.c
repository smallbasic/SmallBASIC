/*
*	SmallBASIC platform driver for X
*
*	ndc: 2001-02-13
*/

#include "device.h"
#include "osd.h"
#include "str.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xcms.h>

static int	cur_x = 0;
static int	cur_y = 0;
static int	mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;
static int	tabsize = 32;	// from dev_palm
static int	maxline;

// font data
static int	font_id = 0;
static int	font_w = 8;
static int	font_h = 16;
static int	font_ul= 15;		// position of underline

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

static int mouse_hot_x = -16, mouse_hot_y = -16;

// X specific
static Display	*x_disp;
static int		x_root;
static int		x_win;
static int		x_screen;
static GC		x_gc;
static Colormap	colormap;

/*
*	0=success 
*/
int		x_load_font()
{
	const char *fonts[] = {"vga", "9x15", "fixed", NULL}, **p = fonts;
	XFontStruct	*font;

	while ( 1 ) {
		if ((font = XLoadQueryFont(x_disp, *p)) == NULL) 	{
			fprintf(stderr, "X: Unable to open font \"%s\"", *p);
			return -1;
			}
		else if (font->min_bounds.width != font->max_bounds.width) {
			fprintf(stderr, "X: Font \"%s\" isn't monospaced", *p);
			XFreeFont(x_disp, font);
			font = NULL;
			return -2;
			}
		else {
			font_w = font->max_bounds.width;
			font_h = font->max_bounds.ascent + font->max_bounds.descent;
			font_ul = font->max_bounds.ascent;
			font_id = font->fid;
			break;
			}

		if ( p[1] != NULL ) 
			p++;
		else {
			fprintf(stderr, "No fixed font found!\n");
			return -3;
			}
		}

	return 0;
}

/*
*	build color map
*/
void	build_colors()
{
	int			i;
	XcmsColor	color;

	colormap = DefaultColormap(x_disp, x_screen);
	XSetWindowColormap(x_disp, x_win, colormap);

	for ( i = 0; i < 16; i ++ )	{
		color.format = XcmsRGBiFormat;
	
		color.spec.RGBi.red   = ((vga16[i] & 0xff0000) >> 16) / 255.0;
		color.spec.RGBi.green = ((vga16[i] & 0xff00) >> 8) / 255.0;
		color.spec.RGBi.blue  = (vga16[i] & 0xff) / 255.0;
		XcmsAllocColor(x_disp, colormap, &color, XcmsRGBiFormat);
		cmap[i] = color.pixel;
		}
}

/*
*/
int		osd_devinit()
{
	int		white, black;
	int		scr_width, scr_height;
	XGCValues	gcv;

	x_disp = XOpenDisplay(0);
	if ( x_disp == NULL) {
	    perror("SB4X:XOpenDisplay");
	    exit(10);
		}

	x_root     = DefaultRootWindow(x_disp);
	x_screen   = DefaultScreen(x_disp);
	scr_width  = DisplayWidth(x_disp, x_screen);
	scr_height = DisplayHeight(x_disp, x_screen);

	os_graf_my = 320;
	os_graf_mx = 320;

	black      = BlackPixel(x_disp, x_screen);
	white      = WhitePixel(x_disp, x_screen);
    x_win = XCreateSimpleWindow(x_disp, DefaultRootWindow(x_disp), 
				(scr_width >> 1) - (os_graf_mx >> 1), (scr_height >> 1) - (os_graf_my >> 1),
				os_graf_mx, os_graf_my, 
				0, black, white);
    
	XSelectInput(x_disp, x_win, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
    XStoreName(x_disp, x_win, "SmallBASIC");
	XMapWindow(x_disp, x_win);

	// setup fonts & sizes
	if	( x_load_font() )
		return 0;
	maxline = os_graf_my / font_h;

	// setup internal screen buffer
	// .............

	gcv.font = font_id;
	gcv.foreground = black;
	gcv.background = white;
	x_gc = XCreateGC(x_disp, x_win, GCFont, &gcv);
		 
	// setup palette
	build_colors();

	setsysvar_str(SYSVAR_OSNAME, "Unix/X");
	osd_cls();
	return 1;
}

/*
*/
int		osd_devrestore()
{
	cur_x = 0;
	cur_y = (maxline - 1) * font_h;
	osd_write("SB4X: Press any key to exit...");
	dev_events(1);
	
//	XFreeColormap(x_disp, colormap);
	XUnloadFont(x_disp, font_id);
	XFreeGC(x_disp, x_gc);
	XCloseDisplay(x_disp);

	return 1;
}

/*
*/
void	x_resize(int new_width, int new_height)
{
	os_graf_mx = new_width;
	os_graf_my = new_height;
	maxline = new_height/font_h;
}

/*
*/
void	x_redraw()
{
}

int		osd_events()
{
	int		evc = 0;
	XEvent	ev;
	char	*p, buffer[128];
	int		bufsize = 128, count;
	KeySym	keysym;
	XComposeStatus	compose;
	int		new_width, new_height;


		//	XSelectInput(x_disp, x_win, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);

		//
		//	X - KEYBOARD EVENTS
		//
		while ( XCheckTypedEvent(x_disp, KeyPress, &ev) )	{
			count = XLookupString((XKeyEvent *) &ev, buffer, bufsize, &keysym, &compose);
			buffer[count] = '\0';

			if	( keysym == SB_KEY_BREAK )	// CTRL+C (break)
				return -2;
			if	( keysym == 3 )	// CTRL+C (break)
				return -2;

			p = buffer;
			while ( *p )	{
				dev_pushkey(*p);
				p ++;
				}
			
			evc ++;
			}

		//	X - MOUSE EVENTS
		while ( XCheckTypedEvent(x_disp, ButtonPress, &ev) )	{
			int		b;

			b = ev.xbutton.button;

			mouse_x = ev.xbutton.x;
			mouse_y = ev.xbutton.y;

			mouse_b = 0;
			if ( b & 1 )	{
				if	( (mouse_b & 1) == 0 )	{	// new press
					mouse_down_x = mouse_x;
					mouse_down_y = mouse_y;
					}

				mouse_upd = 1;

				mouse_pc_x = mouse_x;
				mouse_pc_y = mouse_y;

				mouse_b |= 1;
				}
			if ( b & 2 )
				mouse_b |= 2;
			if ( b & 3 )
				mouse_b |= 4;
			
			evc ++;
			}

		//
		//	X - WINDOW EVENTS
		//
		while ( XCheckTypedEvent(x_disp, Expose, &ev) )	{
			switch ( ev.type )	{
			case Expose:			// redraw
				if	( ev.xexpose.count != 0 )
					break;
				x_redraw();
				break;
			case ConfigureNotify:	// resize
				x_resize(ev.xconfigure.width, ev.xconfigure.height);
				break;
				};
			}
		}

	return evc;
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
	byte	update;

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
	XClearWindow(x_disp, x_win);
//	XFillRectangle(x_disp, x_win, x_gc, 0, 0, os_graf_mx, os_graf_my);
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
	cur_x = 0;

	if	( cur_y < ((maxline-1) * font_h) )	{
		cur_y += font_h;
		}
	else	{
		// scroll
		XCopyArea(x_disp, x_win, x_win, x_gc, 
			0, font_h, /* src */
			os_graf_mx, os_graf_my-font_h,
			0, 0  /* dst */ );

		cur_y = ((maxline-1) * font_h);

		XSetForeground(x_disp, x_gc, cmap[dev_bgcolor]);
		XFillRectangle(x_disp, x_win, x_gc, 
			0, cur_y, os_graf_mx, font_h);
		XSetForeground(x_disp, x_gc, cmap[dev_fgcolor]);
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
	int		len, cx, esc_val, esc_cmd;
	byte	*p, buf[3];
	int		ch, char_len = 1;

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
					XSetForeground(x_disp, x_gc, cmap[dev_bgcolor]);
					XFillRectangle(x_disp, x_win, x_gc, 
							cur_x, cur_y, os_graf_mx - cur_x, font_h);
					XSetForeground(x_disp, x_gc, cmap[dev_fgcolor]);
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
			XSetForeground(x_disp, x_gc, cmap[dev_bgcolor]);
			XFillRectangle(x_disp, x_win, x_gc, 
				0, cur_y, os_graf_mx, font_h);
			XSetForeground(x_disp, x_gc, cmap[dev_fgcolor]);
			break;
		default:
			//
			//	PRINT THE CHARACTER
			//
			buf[0] = *p;

			cx = font_w;
			buf[1] = '\0';

			// new line ?
			if	( cur_x + cx >= os_graf_mx )
				osd_nextln();

			// draw

			// TODO: ??? SJIS on Linux ???
			if	( !con_use_reverse )	{
				XSetForeground(x_disp, x_gc, cmap[dev_fgcolor]);
				XSetBackground(x_disp, x_gc, cmap[dev_bgcolor]);

				XDrawImageString(x_disp, x_win, x_gc, cur_x, cur_y, buf, char_len);
				if	( con_use_bold )	
					XDrawString(x_disp, x_win, x_gc, cur_x+1, cur_y, buf, char_len);
				}
			else	{
				XSetForeground(x_disp, x_gc, cmap[dev_bgcolor]);
				XSetBackground(x_disp, x_gc, cmap[dev_fgcolor]);

				XDrawImageString(x_disp, x_win, x_gc, cur_x, cur_y, buf, char_len);
				if	( con_use_bold )	
					XDrawString(x_disp, x_win, x_gc, cur_x+1, cur_y, buf, char_len);

				XSetBackground(x_disp, x_gc, cmap[dev_bgcolor]);
				XSetForeground(x_disp, x_gc, cmap[dev_fgcolor]);
				}

			if	( con_use_ul )	{
				osd_setcolor(dev_fgcolor);
				osd_line(cur_x, cur_y+font_ul, cur_x+cx, cur_y+font_ul);
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
	XSetForeground(x_disp, x_gc, cmap[color]);
	dev_fgcolor = color;
}

void	osd_line(int x1, int y1, int x2, int y2)
{
	if	( (x1 == x2) && (y1 == y2) )
		XDrawPoint(x_disp, x_win, x_gc, x1, y1);
	else
	    XDrawLine(x_disp, x_win, x_gc,
				  x1, y1, x2, y2);
}

void	osd_setpixel(int x, int y)
{
	XDrawPoint(x_disp, x_win, x_gc, x, y);
}

long	osd_getpixel(int x, int y)
{
	return 0;	// TODO
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

///////////////////////////////////////////////////////////////

int		osd_textwidth(const char *str)
{
	int		l = strlen(str);

	// SJIS ???
	return l * font_w;
}

int		osd_textheight(const char *str)
{
	// TODO: count \n
	return font_h;
}

#if !defined(DRV_SOUND)

void	osd_beep()
{
}

void	osd_sound(int frq, int ms, int vol, int bgplay)
{
}

void	osd_clear_sound_queue()
{
}
#endif

