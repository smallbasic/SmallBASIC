/*
*	SmallBASIC platform driver for X (buffer)
*
*	This driver is very different by classic dev_x because its does not use X to draw.
*	Its use the generic framebuffer driver to produce an bitmap.
*
*	ndc: 2001-08-11
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
#include "dev_genfb.h"

/*
#define icon_width 32
#define icon_height 32
static byte icon_bits[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3F, 0xFF, 0x01, 
0xE0, 0x73, 0x9F, 0x03, 0xE0, 0x73, 0x9F, 0x03, 0xE0, 0x03, 0x9F, 0x03, 0xC0, 0x3F, 0xFF, 0x01, 
0x00, 0x70, 0x9F, 0x03, 0xE0, 0x73, 0x9F, 0x03, 0xE0, 0x73, 0x9F, 0x03, 0xC0, 0x3F, 0xFF, 0x01, 
0x00, 0x00, 0x00, 0x00, 0xA0, 0xEA, 0xFF, 0x03, 0x40, 0xD5, 0xFF, 0x01, 0x80, 0xAA, 0xFF, 0x00, 
0x00, 0x55, 0x7F, 0x00, 0x00, 0xAA, 0x3E, 0x00, 0x00, 0x54, 0x1D, 0x00, 0x00, 0xA8, 0x0A, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00 };
*/

static int mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x,
  mouse_down_y, mouse_pc_x, mouse_pc_y;

// VGA16 colors in RGB
static unsigned long vga16[] = {
  0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
  0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
};
static unsigned long cmap[16];

//static int mouse_hot_x = -16, mouse_hot_y = -16;

// X specific
static Display *x_disp;
static int x_root;
static int x_win;
static int x_screen;
static GC x_gc;
static Colormap colormap;
static long white, black;

static int font_w, font_h, font_ul, font_id;

/*
*	build color map
*/
void build_colors()
{
  int i;
  XcmsColor color;

  colormap = DefaultColormap(x_disp, x_screen);
  XSetWindowColormap(x_disp, x_win, colormap);

  for (i = 0; i < 16; i++) {
    color.format = XcmsRGBiFormat;

    color.spec.RGBi.red = ((vga16[i] & 0xff0000) >> 16) / 255.0;
    color.spec.RGBi.green = ((vga16[i] & 0xff00) >> 8) / 255.0;
    color.spec.RGBi.blue = (vga16[i] & 0xff) / 255.0;
    XcmsAllocColor(x_disp, colormap, &color, XcmsRGBiFormat);
    cmap[i] = color.pixel;
  }
}

/*
*	0=success 
*/
int x_load_font()
{
  const char *fonts[] = { "vga", "9x15", "fixed", NULL }, **p = fonts;
  XFontStruct *font;

  while (1) {
    if ((font = XLoadQueryFont(x_disp, *p)) == NULL) {
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

    if (p[1] != NULL)
      p++;
    else {
      fprintf(stderr, "No fixed font found!\n");
      return -3;
    }
  }

  return 0;
}

/*
*/
int osd_devinit()
{
  int scr_width, scr_height;
  XGCValues gcv;
  char *v, *p;
/*
	Pixmap	icon_pm;
	XClassHint	*classhint;
	XWMHints	*wmhints;
	XSizeHints	*sizehints;
	char	*progname = g_argv[0];
	char	*c_winame = "SmallBASIC/XF";
	char	*c_iconame = "SB/XF";
	XTextProperty	winame, iconame;

	sizehints = XAllocSizeHints();
	wmhints = XAllocWMHints();
	classhint = XAllocClassHint();
*/

  x_disp = XOpenDisplay(0);
  if (x_disp == NULL) {
    perror("Unix/XF:XOpenDisplay");
    exit(10);
  }

  x_root = DefaultRootWindow(x_disp);
  x_screen = DefaultScreen(x_disp);
  scr_width = DisplayWidth(x_disp, x_screen);
  scr_height = DisplayHeight(x_disp, x_screen);

  os_graf_mx = 640;
  os_graf_my = 480;

  /*
   * default video mode 
   */
  if (getenv("SB_SDLMODE") || getenv("SBGRAF")) {
    char *buf, *p, *ps;
    int w = 640, h = 480, d = 16;

    if (getenv("SBGRAF"))
      ps = p = buf = strdup(getenv("SBGRAF"));
    else
      ps = p = buf = strdup(getenv("SB_SDLMODE"));
    p = strchr(buf, 'x');
    if (p) {
      *p = '\0';
      w = atoi(ps);
      ps = ++p;
      p = strchr(ps, 'x');
      if (p) {
        *p = '\0';
        h = atoi(ps);
        ps = ++p;
        if (*p)
          d = atoi(ps);
      }
      else if (*ps)
        h = atoi(ps);
    }

    free(buf);

    // /
    os_graf_mx = w;
    os_graf_my = h;
  }

  black = BlackPixel(x_disp, x_screen);
  white = WhitePixel(x_disp, x_screen);
  x_win = XCreateSimpleWindow(x_disp, DefaultRootWindow(x_disp),
                              (scr_width >> 1) - (os_graf_mx >> 1),
                              (scr_height >> 1) - (os_graf_my >> 1), os_graf_mx,
                              os_graf_my, 0, black, white);

  XSelectInput(x_disp, x_win,
               ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
  XStoreName(x_disp, x_win, "SmallBASIC");
  XMapWindow(x_disp, x_win);

  // setup internal screen buffer
  // .............

  x_load_font();

  gcv.font = font_id;
  gcv.foreground = black;
  gcv.background = white;
  x_gc = XCreateGC(x_disp, x_win, GCFont, &gcv);

/*
	XStringListToTextProperty(&c_winame, 1, &winame);
	XStringListToTextProperty(&c_iconame, 1, &iconame);
	icon_pm = XCreateBitmapFromData(x_disp, x_win, icon_bits, icon_width, icon_height);
	sizehints->flags = PPosition | PSize | PMinSize;
	sizehints->min_width = os_graf_mx;
	sizehints->min_height = os_graf_my;
	wmhints->initial_state = NormalState;
	wmhints->input = 1;
	wmhints->icon_pixmap = icon_pm;
	wmhints->flags = StateHint | IconPixmapHint | InputHint;
	classhint->res_name = progname;
	classhint->res_class = "SBXF";
	XSetWMProperties(x_disp, x_win, &winame, &iconame, g_argv, g_argc, sizehints, wmhints, classhint);
*/

  // setup palette
  build_colors();

  switch (DefaultDepth(x_disp, x_screen)) {
  case 8:
    os_color_depth = 8;         // not supported
    break;
  case 16:
    os_color_depth = 16;
    break;
  case 24:
    os_color_depth = 24;        // not supported
    break;
  case 32:
    os_color_depth = 32;        // not supported
    break;
  default:
    panic("Unix/XF: WINDOW-DEPTH IS NOT SUPPORTED");
  }
  gfb_init(os_graf_mx, os_graf_my, os_color_depth);
  gfb_setcmap(cmap);

  setsysvar_str(SYSVAR_OSNAME, "Unix/XF");
  osd_cls();
  return 1;
}

/*
*/
int osd_devrestore()
{
  osd_setxy(0, (os_graf_my - osd_textheight("A")) - 1);
  osd_write("Unix/XF: Press any key to exit...");
  osd_refresh();
  dev_events(1);

//      XFreeColormap(x_disp, colormap);
  XUnloadFont(x_disp, font_id);
  XFreeGC(x_disp, x_gc);
  XCloseDisplay(x_disp);

  gfb_close();
  return 1;
}

/*
*/
void osd_refresh()
{
  XImage *img;

  img =
    XCreateImage(x_disp, DefaultVisual(x_disp, x_screen), os_color_depth, ZPixmap, 0,
                 NULL, os_graf_mx, os_graf_my, 8, 0);
  if (img) {
    img->data = malloc(gfb_vramsize());
    memcpy(img->data, gfb_vram(), gfb_vramsize());
    XPutImage(x_disp, x_win, x_gc, img, 0, 0, 0, 0, os_graf_mx, os_graf_my);
    XDestroyImage(img);
  }
  else
    panic("XCreateImage: failed");
}

/*
*/
void x_resize(int new_width, int new_height)
{
}

/*
*/
void x_redraw()
{
  osd_refresh();
}

int osd_events()
{
  int evc = 0;
  XEvent ev;
  char *p, buffer[128];
  int bufsize = 128, count;
  KeySym keysym;
  XComposeStatus compose;


  // XSelectInput(x_disp, x_win, ExposureMask | KeyPressMask | ButtonPressMask
  // | StructureNotifyMask);

  // 
  // X - KEYBOARD EVENTS
  // 
  while (XCheckTypedEvent(x_disp, KeyPress, &ev)) {
    count = XLookupString((XKeyEvent *) & ev, buffer, bufsize, &keysym, &compose);
    buffer[count] = '\0';

    if (keysym == SB_KEY_BREAK) // CTRL+C (break)
      return -2;
    if (keysym == 3)            // CTRL+C (break)
      return -2;

    p = buffer;
    while (*p) {
      dev_pushkey(*p);
      p++;
    }

    evc++;
  }

  while (XCheckTypedEvent(x_disp, ButtonPress, &ev)) {
    int b;

    b = ev.xbutton.button;

    mouse_x = ev.xbutton.x;
    mouse_y = ev.xbutton.y;

    mouse_b = 0;
    if (b & 1) {
      if ((mouse_b & 1) == 0) { // new press
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

    evc++;
  }

  // 
  // X - WINDOW EVENTS
  // 
  while (XCheckTypedEvent(x_disp, Expose, &ev)) {
    switch (ev.type) {
    case Expose:               // redraw
      if (ev.xexpose.count != 0)
        break;
      x_redraw();
      break;
    case ConfigureNotify:      // resize
      x_resize(ev.xconfigure.width, ev.xconfigure.height);
      break;
    };
  }

  return evc;
}

/*
*	enable or disable PEN code
*/
void osd_setpenmode(int enable)
{
  mouse_mode = enable;
}

/*
*/
int osd_getpen(int code)
{
  int r = 0;

  osd_events(0);
  if (mouse_mode) {
    switch (code) {
    case 0:                    // bool: status changed
      r = mouse_upd;
      break;
    case 1:                    // last pen-down x
      r = mouse_down_x;
      break;
    case 2:                    // last pen-down y
      r = mouse_down_y;
      break;
    case 3:                    // vert. 1 = down, 0 = up .... unsupported
      r = 0;
      break;
    case 4:                    // last x
      r = mouse_pc_x;
      break;
    case 5:                    // last y
      r = mouse_pc_y;
      break;
    case 10:
      r = mouse_x;
      break;
    case 11:
      r = mouse_y;
      break;
    case 12:
    case 13:
    case 14:
      r = (mouse_b & (1 << (code - 11))) ? 1 : 0;
      break;
    }

    mouse_upd = 0;
  }
  return r;
}

#if !defined(DRV_SOUND)

void osd_beep()
{
}

void osd_sound(int frq, int ms, int vol, int bgplay)
{
}

void osd_clear_sound_queue()
{
}
#endif
