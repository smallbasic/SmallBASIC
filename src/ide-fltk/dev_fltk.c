/*
*	FLTK driver to be used with SBFLTK
*
*	Written by Nicholas Christopoulos
*/

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <linux/fb.h>
#include <sys/vt.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/kd.h>
#include <signal.h>
#include <fcntl.h>
#include "device.h"
#include "osd.h"
#include "str.h"
#include "dev_genfb.h"
#include "dev_term.h"

static int	dev_width, dev_height, dev_depth;

static int	mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;

// VGA16 colors in RGB
static unsigned long vga16[] =
{
0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF 
};

/*
*	SB: Initialization
*/
int		osd_devinit()
{
	os_graf_mx = dev_width;
	os_graf_my = dev_height;
	os_color_depth = dev_depth;
	os_color = 1;
	setsysvar_str(SYSVAR_OSNAME, "Unix/FLTK");

	gfb_init(dev_width, dev_height, dev_depth);

	osd_cls();
	return 1;
}

/*
*	display the current video page
*	(called every ~50ms)
*/
void	osd_refresh()
{
	if	( gfb_getuf() )	{	// if it is modified
//		memcpy(dev_vpage, gfb_vram(), gfb_vramsize());	// redraw
		// write image 
		gfb_resetuf();		// clear modified-flag
		}
}

/*
*	close
*/
int		osd_devrestore()
{
	gfb_close();
	return 1;
}

//	enable or disable PEN code
void	osd_setpenmode(int enable)
{	 	mouse_mode = enable;	}

//	returns the status of the light-pen (mouse here)
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

//	check events
int		osd_events()
{
	int		r;
	#if defined(DRV_MOUSE)
	int		x, y, b;

	if	( suspend )	return 0;
		
	if	( drvmouse_get(&x, &y, &b) )	{
		mouse_x = x;
		mouse_y = y;

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
		if (b & 2)
			mouse_b |= 2;
		if (b & 3)
			mouse_b |= 4;
		return 1;
		}
	#endif

	if	( (r = term_events()) != 0 )	// keyboard events
		return r;
	return 0;	
}

// sound

void	osd_beep()
{
}

void	osd_sound(int frq, int dur, int vol, int bgplay)
{
}

void	osd_clear_sound_queue()
{
}
