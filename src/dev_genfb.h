/*
*	Generic 'framebuffer' techique graphics driver
*
*	This is a common routines for 'framebuffer' technique (not the linux-driver)
*	This driver stores the image into a buffer (dev_vpage) so the parent driver
*	can use that buffer to dump it into the screen (like bitmap, perhaps on every
*	osd_events() call).
*
*	Notes:
*		Parent driver must setup the palette for 8bpp modes (colors 0..15 to stdvga colors)
*
*	Nicholas Christopoulos
*/

#if !defined(_dev_genfb_h)
#define _dev_genfb_h

#include "sys.h"
#include "osd.h"

int		gfb_init(int width, int height, int bpp);
void	gfb_setcmap(long *newcmap);
int		gfb_close(void);
byte	*gfb_vram(void);
dword   gfb_vramsize(void);
void	gfb_resize(int width, int height);

byte	*gfb_alloc(void);
byte	*gfb_clone(void);
void	gfb_free(byte *buf);
byte	*gfb_switch(byte *buf);

int		gfb_getuf(void);
#define	gfb_modified()	gfb_getuf()
void	gfb_resetuf(void);

#endif

