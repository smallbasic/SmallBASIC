/*
*	win32
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
static dword	dev_dos_vseg, dev_bank_count;
static dword	cmap[16];

static int	mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;

static int	dev_oldmode;
static int	dev_vesa;
static int	dev_vwrite_type;	// 0 = not mapped, 1 = mapping banked, 3 = linear
static int	dev_video_descr;

static int	dev_linelen;

static char dos_pal[768];

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
	HWND	wnd;
	HDC		hdc, hdc_bmp;
	HBITMAP	bmp, obmp;
	BITMAPINFOHEADER bmih;
	BITMAPINFO		 bmi;
	byte			*vr, *vrsrc;
	int				i;

	wnd = w32_getwindow();
	hdc = GetDC(wnd);

	memset(&bmih, 0, sizeof(bmih));
	bmih.biSize     = sizeof(bmih);
	bmih.biWidth    = dev_width;
	bmih.biHeight   = dev_height;
	bmih.biPlanes   = 1;
	bmih.biBitCount = dev_depth;
	bmih.biCompression = BI_RGB;

	bmi.bmiHeader = bmih;

	dev_linelen = dev_width * (dev_depth / 8);

	vrsrc = gfb_vram();
	vr = gfb_alloc();
	for ( i = 0; i < dev_height; i ++ )	
		memcpy(vr + i*dev_linelen, vrsrc + (dev_height-(i+1))*dev_linelen, dev_linelen);
	
    if ( (bmp = CreateDIBitmap(hdc, (LPBITMAPINFOHEADER) &bmi,
			CBM_INIT, vr, &bmi, DIB_RGB_COLORS)) == NULL ) 
			panic("DIB bitmap failed!");

	hdc_bmp = CreateCompatibleDC(hdc);
	obmp = (HBITMAP) SelectObject(hdc_bmp, bmp);
	
	BitBlt(
		hdc, 0, 0, dev_width, dev_height,
		hdc_bmp, 0, 0, SRCCOPY);

	SelectObject(hdc_bmp, obmp);
	DeleteDC(hdc_bmp);

	DeleteObject(bmp);
	ReleaseDC(wnd, hdc);

	gfb_free(vr);
}

/*
*	SB: Initialization
*/
int		osd_devinit()
{
	int		i;

	dev_width  = 320;
	dev_height = 200;
	dev_depth  = 8;
			
	gfb_init(dev_width, dev_height, dev_depth);

//	if	( dev_depth == 8 )
//		dos_buildpalette();

	// 
	os_graf_mx     = dev_width;
	os_graf_my     = dev_height;
	os_color_depth = dev_depth;
	os_color       = (dev_depth >= 8);

	w32_evstate(1);

	osd_cls();
	return 1;
}

/*
*	close
*/
int		osd_devrestore()
{
//	osd_setxy(0, (dev_height - osd_textheight("A")) - 1);
//	osd_write("W32: Press any key to exit...");
	osd_refresh();

//	if	( dev_depth == 8 )
//		dos_setpalette(dos_pal);

	gfb_close();
	return 1;
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
				dev_pushkey(ev.ch);
				}
			}

		} while ( wait_flag && (r == 0) );

	return 0;
}

//////////

#if !defined(DRV_SOUND)

void	osd_beep()
{
}

void	osd_sound(int frq, int dur, int vol, int bgplay)
{
}

#endif

