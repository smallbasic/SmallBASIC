/*
*	SmallBASIC platform driver for Unix + SVGALIB
*
*	2001-02-13, Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include "device.h"
#include "osd.h"
#include "str.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vga.h>
#include <vgagl.h>
#include <vgamouse.h>
#include <vgakeyboard.h>
#include "dev_term.h"

static int	cur_x = 0;
static int	cur_y = 0;
static int	uvgaclr = 0;
static int	mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;
static int	tabsize = 32;	// from dev_palm
static int	maxline;
static int	font_h = 16;
static int	font_w = 8;

static int	con_use_bold = 0;
static int	con_use_ul   = 0;
static int	con_use_reverse = 0;

// VGA16 colors in RGB
static unsigned long vga16[] =
{
0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF 
};

// fonts
#include "unix/rom16.c"

static int mouse_hot_x = 0, mouse_hot_y = 0;

/*
*	old mouse cursor (from demos of svgalib)
*/
unsigned long mouse_cursor[] = {
0x3f8,  0x60c,  0x9e2,  0x1a13, 0x1401, 0x1401, 0x1001, 0x1001,
0x1001, 0x1003, 0x1802, 0x340c, 0x6ff8, 0xd800, 0xb000, 0xe000,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0x3f8, 0x60c, 0x9e2, 0x1a13, 0x1401, 0x1401, 0x1001, 0x1001,
0x1001, 0x1003, 0x1802, 0x340c, 0x6ff8, 0xd800, 0xb000, 0xe000,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

/*
*	mouse cursor - pattern (1=black, 2=white(mask))
*/
int			 mouse_cur_org[] = 
  { 
  0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
  2,1,1,2,2,0,0,0,0,0,0,0,0,0,0,0,
  2,1,1,1,1,2,2,0,0,0,0,0,0,0,0,0,
  0,2,1,1,1,1,1,2,2,0,0,0,0,0,0,0,
  0,2,1,1,1,1,1,1,1,2,2,0,0,0,0,0,
  0,0,2,1,1,1,1,1,1,1,1,2,2,0,0,0,
  0,0,2,1,1,1,1,1,1,1,1,1,1,2,0,0,
  0,0,0,2,1,1,1,1,1,2,2,2,2,0,0,0,
  0,0,0,2,1,1,1,1,1,2,0,0,0,0,0,0,
  0,0,0,0,2,1,1,2,2,1,2,0,0,0,0,0,
  0,0,0,0,2,1,1,2,0,2,1,2,0,0,0,0,
  0,0,0,0,0,2,1,2,0,0,2,1,2,0,0,0,
  0,0,0,0,0,2,1,2,0,0,0,2,1,2,0,0,
  0,0,0,0,0,0,2,0,0,0,0,0,2,1,2,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/*
*	build cursor from pattern
*/
void	build_cursor()
{
	unsigned long	*w, *m;
	int				y, x;

	for ( y = 0; y < 16; y ++ )	{
		w = &mouse_cursor[y];
		m = &mouse_cursor[y+32];
		*w = *m = 0;
		for ( x = 0; x < 16; x ++ )	{
			if	( mouse_cur_org[y*16+x] == 1 )
				*w = *w | ((unsigned long) (1<<(31-x)));
			if	( mouse_cur_org[y*16+x] )
				*m = *m | ((unsigned long) (1<<(31-x)));
			}
		}
}

//#define	TAKE_SCREENSHOT

#if defined(TAKE_SCREENSHOT)
void	writeppm(char *name);
#endif

/*
*	SVGALIB: Initialization
*/
int		osd_devinit()
{
	int		vgamode, i;

	vga_init();
	vgamode = vga_getdefaultmode();
    if ( vgamode == -1 )
//		vgamode = G320x200x64K;
//		vgamode = G640x480x64K;
		vgamode = G800x600x64K;
//		vgamode = G1024x768x64K;

    if (!vga_hasmode(vgamode)) {
        fprintf(stderr, "SVGALIB: Mode not available.\n");
		return 0;
	    }

    vga_setmode(vgamode);
    gl_setcontextvga(vgamode);

	os_graf_mx = vga_getxdim();
	os_graf_my = vga_getydim();
	maxline = os_graf_my / font_h;
	os_color = 1;
	switch ( vga_getcolors() )	{
	case	16:
		os_color_depth = 4;
		break;
	case	256:
		os_color_depth = 8;
		for ( i = 0; i < 16; i ++ )	{
			gl_setpalettecolor(
				i,
				(vga16[i] & 0xff0000) >> 16,
				(vga16[i] & 0xff00) >> 8,
				vga16[i] & 0xff );
			}

		break;
	case	65536:
		os_color_depth = 16;
		break;
	default:
		os_color_depth = 32;
		}

    gl_enableclipping();
    gl_setwritemode(FONT_COMPRESSED + WRITEMODE_OVERWRITE);

	///// MOUSE CURSOR: UNDOCUMENTED !!!! WWWHHHHHYYYY ??????????
	build_cursor();
	vga_setmousesupport(1);
	vga_initcursor(1);
	vga_setcursorposition(mouse_getx() + mouse_hot_x, mouse_gety() + mouse_hot_y);
	vga_setcursorimage(0, 0, 0xffffff, 0, (unsigned char *) mouse_cursor);
	vga_selectcursor(0);

	setsysvar_str(SYSVAR_OSNAME, "Unix/SVGA");
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
	osd_write("SVGALIB: Press any key to exit...");
#if defined(TAKE_SCREENSHOT)
	writeppm("svga.ppm");	
#endif
	vga_getch();

    vga_setmode(TEXT);
	return 1;
}

void	osd_settextcolor(long fg, long bg)
{
	osd_setcolor(fg);
	if	( bg != -1 )
		dev_bgcolor = bg;
}

//
void	osd_drawchar(int x, int y, byte ch, int overwrite, int reverse)
{
	int		offset;
	int		bit, i;
	int		fg, bg;

//	fg = gl_rgbcolor( 
//		(vga16[dev_fgcolor] & 0xff0000) >> 16,
//		(vga16[dev_fgcolor] & 0xff00) >> 8,
//		vga16[dev_fgcolor]  & 0xff);
	fg = uvgaclr;

	if	( dev_bgcolor <= 15 && dev_bgcolor >= 0 )	{
		bg = gl_rgbcolor(
			(vga16[dev_bgcolor] & 0xff0000) >> 16,
			(vga16[dev_bgcolor] & 0xff00) >> 8,
			vga16[dev_bgcolor]  & 0xff);
		}
	else
		bg = 0;

	if	( reverse )	{
		int	rev = fg;
		fg = bg;
		bg = rev;
		}

	offset = ch * 16;

	for ( i = 0; i < 16; i ++, offset ++ )	{
		for ( bit = 0; bit < 8; bit ++ )	{
			if	( *(font8x16+offset) & (1 << (8-bit)) )
				gl_setpixel(x+bit, y+i, fg);
			else if ( overwrite )
				gl_setpixel(x+bit, y+i, bg);
			}
		}
}

/*
*	enable or disable PEN code
*/
void	osd_setpenmode(int enable)
{
	mouse_mode = enable;
	vga_showcursor(enable);
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
	if	( mouse_mode )	vga_showcursor(0);
	gl_clearscreen(vga_white());
	if	( mouse_mode )	vga_showcursor(1);
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
		gl_copybox(0, font_h, os_graf_mx, cur_y, 0, 0);
		gl_fillbox(0, cur_y, os_graf_mx, font_h, vga_white());
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
*	\e[nG	move cursor to specified column
*/
void	osd_write(const char *str)
{
	int		len, cx, esc_val, esc_cmd;
	byte	*p, buf[3];

	len = strlen(str);

	if	( len <= 0 )
		return;

	if	( mouse_mode )	vga_showcursor(0);

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
					gl_fillbox(cur_x, cur_y, os_graf_mx - cur_x, font_h, vga_white());
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

					default:
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
			gl_fillbox(cur_x, cur_y, os_graf_mx - cur_x, font_h, vga_white());
			break;
		default:
			//
			//	PRINT THE CHARACTER
			//
			buf[0] = *p;

			cx = 8;
			buf[1] = '\0';

			// new line ?
			if	( cur_x + cx >= os_graf_mx )
				osd_nextln();

			// draw

			// TODO: ??? SJIS on Linux ???
			if	( !con_use_reverse )	{
				osd_drawchar(cur_x, cur_y, *p, 1, 0);
				if	( con_use_bold )	
					osd_drawchar(cur_x-1, cur_y, *p, 0, 0);
				}
			else	{
				osd_drawchar(cur_x, cur_y, *p, 1, 1);
				if	( con_use_bold )	
					osd_drawchar(cur_x-1, cur_y, *p, 0, 1);
				}

			if	( con_use_ul )	{
				osd_setcolor(dev_fgcolor);
				vga_drawline(cur_x, (cur_y+font_h)-1, cur_x+cx, (cur_y+font_h)-1);
				}

			// advance
			cur_x += cx;
			};

		if	( *p == '\0' )
			break;

		p ++;
		}

	if	( mouse_mode )	vga_showcursor(1);
}

/*
*	events loop
*/
int		osd_events()
{
	int		button;
	int		evc = 0;

	evc = term_events();	// keyboard events
	while ( mouse_update() )	{
		mouse_x = mouse_getx();
		mouse_y = mouse_gety();

	   	// !!!
	   	if	( mouse_x < mouse_hot_x )		mouse_x = mouse_hot_x;
	   	if	( mouse_y < mouse_hot_y )		mouse_y = mouse_hot_y;
	   	if	( mouse_x >= os_graf_mx - 1 )	mouse_x = os_graf_mx - 1;
	   	if	( mouse_y >= os_graf_my - 1 )	mouse_y = os_graf_my - 1;
	   	vga_setcursorposition(mouse_x + mouse_hot_x, mouse_y + mouse_hot_y);

	   	button = mouse_getbutton();

	   	mouse_b = 0;
	   	if ( button & MOUSE_LEFTBUTTON )	{
	   		if	( (mouse_b & MOUSE_LEFTBUTTON) == 0 )	{	// new press
	   			mouse_down_x = mouse_x;
	   			mouse_down_y = mouse_y;
	   			}

	   		mouse_upd = 1;

	   		mouse_pc_x = mouse_x;
	   		mouse_pc_y = mouse_y;

	   		mouse_b |= 1;
	   		}
	   	if (button & MOUSE_RIGHTBUTTON)
	   		mouse_b |= 2;
	   	if (button & MOUSE_MIDDLEBUTTON)
	   		mouse_b |= 4;

	   	evc ++;
		}

	return evc;
}

///////////////////////////////////////////////////////////////

void	osd_setcolor(long color)
{
	if	( os_color_depth > 8 )	{
		if ( color < 0 )	
			color = -color;
		else if	( color <= 15 && color >= 0 )	
			color = vga16[color];
		else
			color = 0;

		uvgaclr = gl_rgbcolor(
			(color & 0xff0000) >> 16,
			(color & 0xff00) >> 8,
			color & 0xff);

		vga_setrgbcolor(
			(color & 0xff0000) >> 16,
			(color & 0xff00) >> 8,
			color & 0xff);
		}
	else
		vga_setcolor((uvgaclr=color));
}

void	osd_line(int x1, int y1, int x2, int y2)
{
	if	( mouse_mode )	vga_showcursor(0);
	if	( (x1 == x2) && (y1 == y2) )
		gl_setpixel(x1, y1, uvgaclr);
	else
		gl_line(x1, y1, x2, y2, uvgaclr);
	if	( mouse_mode )	vga_showcursor(1);
}

void	osd_setpixel(int x, int y)
{
	if	( mouse_mode )	vga_showcursor(0);
	gl_setpixel(x, y, uvgaclr);	
	if	( mouse_mode )	vga_showcursor(1);
}

long	osd_getpixel(int x, int y)
{
	long	c; //, i;

	if	( mouse_mode )	vga_showcursor(0);
	c = gl_getpixel(x, y);	
	if	( mouse_mode )	vga_showcursor(1);
//	for ( i = 0; i < 16; i ++ )	{
//		if	( c == vga16[i] )
//			return i;
//		}
	return c;
}

void	osd_rect(int x1, int y1, int x2, int y2, int fill)
{
	int		y;

	if	( mouse_mode )	vga_showcursor(0);
	if	( fill )	{
		for ( y = y1; y <= y2; y ++ )	
			gl_line(x1, y, x2, y, uvgaclr);
		}
	else	{
		gl_line(x1, y1, x1, y2, uvgaclr);
		gl_line(x1, y2, x2, y2, uvgaclr);
		gl_line(x2, y2, x2, y1, uvgaclr);
		gl_line(x2, y1, x1, y1, uvgaclr);
		}
	if	( mouse_mode )	vga_showcursor(1);
}

void	osd_refresh()
{
}

///////////////////////////////////////////////////////////////

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

#if defined(TAKE_SCREENSHOT)
//
void	writeppm(char *name)
{
    int		y, i, j, k;
	FILE	*file;
	byte	*tmp, *tmp2;
	int		width = os_graf_mx;
	int		height = os_graf_my;
	int		bpp = os_color_depth;

	file = fopen(name, "wt");
    fprintf(file, "P6\n%i %i\n255\n", width, height);

	tmp = (byte *) malloc(width*4);
	tmp2 = (byte *) malloc(width*4);

    for ( y = 0; y < height; y++ )	{

        vga_getscansegment(tmp, 0, y, width * 4);

        switch ( bpp ) {
        case 16:
            for ( i = 0; i < width; i++ ) {
                j = tmp[i * 2] + 256 * tmp[i * 2 + 1];
                tmp2[i * 3] = (j & 0xf800) >> 8;
                tmp2[i * 3 + 1] = (j & 0x7e0) >> 3;
                tmp2[i * 3 + 2] = (j & 0x1f) << 3;
	            }
            break;
        case 24:
            memcpy(tmp2, tmp, width * 3);
            break;
	        }

        fwrite(tmp2, width * 3, 1, file);
		}

	free(tmp);
	free(tmp2);

	fclose(file);
}
#endif
