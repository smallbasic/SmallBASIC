/*
*	DOS video ram direct
*
*	Written by Nicholas Christopoulos
*/

#include <stdio.h>
#include <unistd.h>
#include <dpmi.h>
#include <pc.h>
#include <conio.h>
#include <bios.h>
#include <go32.h>
#include <dos.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include "device.h"
#include "osd.h"
#include "str.h"
#if defined(DRV_MOUSE)
#include "drvmouse.h"
#endif
#include "dev_genfb.h"

//typedef unsigned short int	word;

static dword	dev_width, dev_height, dev_depth;
static dword	dev_dos_vseg, dev_bank_count;
//static dword	cmap[16];

static int	mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x, mouse_down_y, mouse_pc_x, mouse_pc_y;

static int	dev_oldmode;
static int	dev_vesa;
static int	dev_vwrite_type;	// 0 = not mapped, 1 = mapping banked, 3 = linear

static int	dev_bgra;

static char dos_pal[768];

// EGA/VGA16 colors in RGB
static dword vga16[] =
{
0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF 
};

struct VBEINFO {
	char	signature[4];     
	word	version;
	dword	oemptr;
	byte	cap[4];
	dword	modeptr;
	word	totmem;
	word	oemrev;
	dword	oemname;
	dword	product;
	dword	productrev;
	char	shit[222];
	char	data[256];
	};

struct VBEMODEINFO {
   // Mandatory information for all VBE revision
	word	attr;	     // Mode attributes
   BYTE  winaattributes;     // Window A attributes
   BYTE  winbattributes;     // Window B attributes
   WORD  wingranularity;     // Window granularity
   WORD  winsize;            // Window size
   WORD  winasegment;        // Window A start segment
   WORD  winbsegment;        // Window B start segment
   DWORD winfuncptr;         // pointer to window function

	word	bpsl;				// bytes per scan line

   // Mandatory information for VBE 1.2 and above
	word		width;			// Horizontal resolution in pixel or chars
	word		height;			// Vertical resolution in pixel or chars
   BYTE  xcharsize;           // Character cell width in pixel
   BYTE  ycharsize;           // Character cell height in pixel
   BYTE  numberofplanes;      // Number of memory planes
	byte		bpp;			// Bits per pixel
   BYTE  numberofbanks;       // Number of banks
   BYTE  memorymodel;         // Memory model type
   BYTE  banksize;            // Bank size in KB
   BYTE  numberofimagepages;  // Number of images
   BYTE  reserved1;           // Reserved for page function

   // Direct Color fields (required for direct/6 and YUV/7 memory models)
   BYTE  redmasksize;         // Size of direct color red mask in bits
   BYTE  redfieldposition;    // Bit position of lsb of red bask
   BYTE  greenmasksize;       // Size of direct color green mask in bits
   BYTE  greenfieldposition;  // Bit position of lsb of green bask
   BYTE  bluemasksize;        // Size of direct color blue mask in bits
   BYTE  bluefieldposition;   // Bit position of lsb of blue bask
   BYTE  rsvdmasksize;        // Size of direct color reserved mask in bits
   BYTE  rsvdfieldposition;   // Bit position of lsb of reserved bask
   BYTE  directcolormodeinfo; // Direct color mode attributes

   // Mandatory information for VBE 2.0 and above
	dword	base_addr;         // Physical address for flat frame buffer
   DWORD offscreenmemoffset;  // Pointer to start of off screen memory
   WORD  offscreenmemsize;    // Amount of off screen memory in 1Kb units
   char  reserved2[206];      // Remainder of ModeInfoBlock
	};

void	dos_getpalette(char *pal)
{
	int		i;

	/* get the palette */
	outportb(0x3c7, 0); /* want to read */
	for (i = 0; i < 768; i++)
		pal[i] = inportb(0x3c9);
}

void	dos_setpalette(char *pal)
{
	int		i;
	
	/* set palette */
	outportb(0x3c8, 0); /* want to write */
	for (i = 0; i < 768; i++)
		outportb(0x3c9, pal[i]);
}

void	dos_buildpalette()
{
	int		i;
	char	pal[768]; /* 256 rgb triplets */

	dos_getpalette(dos_pal);
	dos_getpalette(pal);
	for ( i = 0; i < 16; i ++ )	{
		pal[i*3+0] = (vga16[i] >> 16);
		pal[i*3+1] = ((vga16[i] & 0xFF00) >> 8);
		pal[i*3+2] = (vga16[i] & 0xFF);
		}
	dos_setpalette(pal);
}

/*
*	returns true for success (actually 2 for VBE2, 1 for VBE < 2, 0 for error)
*/
int		vesa_avail(struct VBEINFO *vi)
{
	_go32_dpmi_seginfo	mi;
	__dpmi_regs 		regs;
	int					retval;

	mi.size = 64;
	_go32_dpmi_allocate_dos_memory(&mi);

	memcpy(vi->signature, "VBE2", 4);
	dosmemput(vi, sizeof(struct VBEINFO), mi.rm_segment*16);
	memset(&regs, 0, sizeof(regs));
	regs.x.ax = 0x4F00;
	regs.x.es = mi.rm_segment;
	__dpmi_int(0x10, &regs);

	retval = ( regs.x.ax == 0x4F );
	if	( retval )	{
		dosmemget(mi.rm_segment*16, sizeof(struct VBEINFO), vi);
		if	( vi->signature[3] >= '2' && vi->signature[3] <= '9' )
			retval = vi->signature[3] - '0';
		}

	_go32_dpmi_free_dos_memory(&mi);
	return retval;
}

/*
*	returns true for success
*/
int		vesa_getmodeinfo(int mode, struct VBEMODEINFO *vi)
{
	_go32_dpmi_seginfo	mi;
	__dpmi_regs 		regs;
	int					retval;

	mi.size = 64;
	_go32_dpmi_allocate_dos_memory(&mi);

	memset(&regs, 0, sizeof(regs));
	regs.x.ax = 0x4F01;
	regs.x.cx = mode;
	regs.x.es = mi.rm_segment;
	__dpmi_int(0x10, &regs);

	retval = ( regs.x.ax == 0x4F );
	if	( retval )
		dosmemget(mi.rm_segment*16, sizeof(struct VBEMODEINFO), vi);

	_go32_dpmi_free_dos_memory(&mi);
	return retval;
}

/*
*	returns true for success
*/
int		vesa_setmode(int mode)
{
	__dpmi_regs 	regs;

	memset(&regs, 0, sizeof(regs));
	regs.x.ax = 0x4F02;
	regs.x.bx = mode;
	__dpmi_int(0x10, &regs);
	return ( regs.x.ax == 0x4F );
}

void	vesa_setbank(int bank)
{
	__dpmi_regs 	regs;
	memset(&regs, 0, sizeof(regs));
	regs.x.ax = 0x4F05;
	regs.x.bx = 1;
	regs.x.dx = bank * (64 / dev_bgra);
	__dpmi_int(0x10, &regs);
	regs.x.bx = 0;
	__dpmi_int(0x10, &regs);
}

//#define	R16CPY(d, s, n)		_dosmemputl((s), (n)>>2, (d))	
#define	R16CPY(d, s, n)		movedata(_my_ds(), (dword) (s), _dos_ds, (d), (n))

void	dos_write_vram()
{
	if	( dev_vwrite_type == 0 )	{
		if	( gfb_vramsize() < 0x10000 )	{
			switch ( dev_depth )	{
			case	1:
				outportb(0x3CE, 0x01);		outportb(0x3CF, 0x00);
				outportb(0x3CE, 0x08);		outportb(0x3CF, 0xFF);
				outportb(0x3C4, 0x02);		outportb(0x3C5, 0x0F);

				R16CPY(dev_dos_vseg, gfb_vram(), gfb_vramsize());

				outportb(0x3C5, 0x0F);
				outportb(0x3CE, 0x01);		outportb(0x3CF, 0x0F);
				break;
			case	2:	
			case	4:
				break;
			case	8:
				R16CPY(dev_dos_vseg, gfb_vram(), gfb_vramsize());
				}
			}
		else	{
			int		i;

			for ( i = 0; i < dev_bank_count; i ++ )	{
				vesa_setbank(i);
				R16CPY(dev_dos_vseg, gfb_vram(), 0x10000);
				}
			}
		}
}

void	osd_refresh()
{
	dos_write_vram();
}

/*
*	SB: Initialization
*/
#define	VMODEP(s, x, y, b)	(dev_dos_vseg=((s) << 4), dev_width = (x), dev_height = (y), dev_depth = (b))
int		osd_devinit()
{
	int		dev_mode = 0x13;
	char	*p;
	__dpmi_regs 	regs;

	if	( (p = getenv("SBGRAFMODE")) != NULL )	
		dev_mode = atoi(p);

	//
	switch ( dev_mode )	{
	case	0x4:	// CGA
	case	0x5:	// CGA GRAY
		VMODEP(0xB800, 320, 200, 2);
		break;
	case	0x6:
		VMODEP(0xB800, 640, 200, 1);
		break;
	case	0x8:	// HERCULES 720x348 mono
		VMODEP(0xB000, 720, 348, 1);
		break;
	case	0xD:	// EGA
		VMODEP(0xA000, 320, 200, 4);
		break;
	case	0xE:	// EGA
		VMODEP(0xA000, 640, 200, 4);
		break;
	case	0xF:	// EGA
		VMODEP(0xA000, 640, 350, 1);
		break;
	case	0x10:	// EGA
		VMODEP(0xA000, 640, 350, 4);
		break;
	case	0x11:	// VGA
		VMODEP(0xA000, 640, 480, 1);
		break;
	case	0x12:	// VGA
		VMODEP(0xA000, 640, 480, 4);
		break;
	case	0x13:	// VGA
		VMODEP(0xA000, 320, 200, 8);
		break;
	case	0x100:	// VESA
		VMODEP(0xA000, 640, 400, 8);
		break;
	case	0x101:
		VMODEP(0xA000, 640, 480, 8);
		break;
	case	0x103:
		VMODEP(0xA000, 800, 600, 8);
		break;
	case	0x105:
		VMODEP(0xA000, 1024, 768, 8);
		break;
	case	0x107:
		VMODEP(0xA000, 1280, 1024, 8);
		break;
	case	0x110:
		VMODEP(0xA000, 640, 480, 16);
		break;
	case	0x112:
		VMODEP(0xA000, 640, 480, 32);
		break;
	case	0x113:
		VMODEP(0xA000, 800, 600, 16);
		break;
	case	0x115:
		VMODEP(0xA000, 800, 600, 32);
		break;
	case	0x116:
		VMODEP(0xA000, 1024, 768, 16);
		break;
	case	0x118:
		VMODEP(0xA000, 1024, 768, 32);
		break;
	case	0x119:
		VMODEP(0xA000, 1280, 1024, 16);
		break;
	case	0x11B:
		VMODEP(0xA000, 1280, 1024, 32);
		break;
	case	0x11C:
		VMODEP(0xA000, 1600, 1200, 8);
		break;
	case	0x11D:
		VMODEP(0xA000, 1600, 1200, 16);
		break;
	default:
		return 0;
		};

	// change mode
	dev_oldmode = _farpeekb(_dos_ds, 0x449);
	if	( dev_mode < 0x100 )	{
		memset(&regs, 0, sizeof(regs));
		regs.x.ax = dev_mode;
		__dpmi_int(0x10, &regs);
		}
	else	{
		// VESA
 		struct VBEINFO		vinfo;
 		struct VBEMODEINFO	vmodeinfo;
		
		if	( (dev_vesa = vesa_avail(&vinfo)) != 0 )	{
			if 	( vesa_getmodeinfo(0x4000 | dev_mode, &vmodeinfo) == 0 )	{
				if 	( vesa_getmodeinfo(dev_mode, &vmodeinfo) == 0 )	
					panic("VGA: VESA %d PROBLEM WITH MODE 0x%X\n", dev_vesa, dev_mode);
				}
			else
				dev_mode |= 0x4000;

			dev_bgra = vmodeinfo.wingranularity;
			if	( dev_vesa >= 2 )	{
				dev_width  = vmodeinfo.width;
				dev_height = vmodeinfo.height;
				dev_depth  = vmodeinfo.bpp;
				}
			
			/////////////////////////////////////////////////////////
			vesa_setmode(dev_mode);
			vesa_setbank(0);

			dev_vwrite_type = 0;

/*			if	( dev_vesa >= 2 )	{
				if	( vmodeinfo.attr & 0x80 )	{
					dev_dos_vseg = vmodeinfo.base_addr;
					dev_video = dev_vpage = (byte *) vmodeinfo.base_addr;
					dev_vwrite_type = 3;
					printf("VGA: LINEAR MODE\n");
					}
				}
*/
			} // vesa_avail()
		}

/*
	if	( dev_vwrite_type == 0 )	{
		__dpmi_meminfo	meminfo;

		meminfo.size = dev_video_size;
		meminfo.handle = 0;
		meminfo.address = dev_dos_vseg;
		if	( __dpmi_physical_address_mapping(&meminfo) == 0 )	{
			unsigned long	base;
			int				ds;

	        __asm__ __volatile__ (" movw %%ds, %%ax" \
			        :"=a" (ds));

			dev_dos_vseg = meminfo.address;
			__dpmi_get_segment_base_address(ds, &base);
			meminfo.address = (dev_dos_vseg - base);
			dev_dos_vseg = meminfo.address;
	        if ( (int) __dpmi_get_segment_limit(ds) != -1 ) 
				__dpmi_set_segment_limit(ds, -1);
 
			dev_video = dev_vpage = (byte *) meminfo.address;
			dev_vwrite_type = 1;
			printf("VGA: MAPPING MODE\n");
			}
		}
*/
	gfb_init(dev_width, dev_height, dev_depth);

	if	( dev_vwrite_type == 0 )	{
		dev_bank_count = (gfb_vramsize() / 0x10000) + 1;
		printf("VGA: WARNING! USING SLOWEST MODE");
		}

	if	( dev_depth == 8 )
		dos_buildpalette();

	// 
	os_graf_mx     = dev_width;
	os_graf_my     = dev_height;
	os_color_depth = dev_depth;
	os_color       = (dev_depth >= 8);

	osd_cls();
	return 1;
}

/*
*	close
*/
int		osd_devrestore()
{
	__dpmi_regs regs;

	osd_setxy(0, (dev_height - osd_textheight("A")) - 1);
	osd_write("VGA: Press any key to exit...");
	dos_write_vram();
	
	getch();

	if	( dev_depth == 8 )
		dos_setpalette(dos_pal);

	memset(&regs, 0, sizeof(regs));
	regs.x.ax = dev_oldmode;
	__dpmi_int(0x10, &regs);

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
		#if defined(DRV_MOUSE)
		int		x, y, b;

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

void	osd_sound(int frq, int dur, int vol, int bgplay)
{
}

#endif

