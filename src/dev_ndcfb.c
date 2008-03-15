/*
 * linux framebuffer driver
 *
 * Written by Nicholas Christopoulos
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
#if defined(DRV_MOUSE)
#include "drvmouse.h"
#endif
#include "dev_genfb.h"
#include "dev_term.h"

//typedef unsigned short int    word;

static int dev_fd;
static int dev_width, dev_height, dev_depth;
static int dev_smemlen;
static unsigned char *dev_video;
static unsigned char *dev_vpage;
//static struct fb_cmap *fbcmap, *old_cmap;
static long cmap[16];
//static int    dev_tty = 0;

static int mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x,
  mouse_down_y, mouse_pc_x, mouse_pc_y;

static int VTFD, vtnum, suspend;

// VGA16 colors in RGB
static unsigned long vga16[] = {
  0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
  0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
};

// get vt num
int vt_getnum(int tty)
{
  struct vt_stat vs;

  ioctl(tty, VT_GETSTATE, &vs);
  return vs.v_active;
}

/* take appropriate action to release VT */
void vt_relsig(int signo)
{
  sigset_t signalset;

  suspend = 1;

  sigemptyset(&signalset);
  sigaddset(&signalset, SIGUSR2);
  sigprocmask(SIG_SETMASK, &signalset, NULL);
  ioctl(VTFD, VT_RELDISP, 1);
}

/* take appropriate action to to aquire VT */
void vt_acqsig(int signo)
{
  sigset_t signalset;

  suspend = 0;

  sigemptyset(&signalset);
  sigaddset(&signalset, SIGUSR1);
  sigprocmask(SIG_SETMASK, &signalset, NULL);
  ioctl(VTFD, VT_RELDISP, VT_ACKACQ);
  ioctl(VTFD, VT_ACTIVATE, vtnum);
  ioctl(VTFD, VT_WAITACTIVE, vtnum);
}

/**
 * initialize vt-switch technicque
 */
void vt_switch_init()
{
  struct sigaction sact;

  // globals
  VTFD = open("/dev/tty", O_RDWR, 0);
  vtnum = vt_getnum(VTFD);

  //
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
  sact.sa_handler = vt_relsig;
  sigaction(SIGUSR2, &sact, NULL);
  sact.sa_handler = vt_acqsig;
  sigaction(SIGUSR1, &sact, NULL);
}

/**
 * stop vt-switch sys
 */
void vt_switch_destroy()
{
  close(VTFD);

  // todo: restore signals
}

/*
 * initialize frame-buffer device
 */
void fb_init(char *device)
{
  char dev[64];
  struct fb_fix_screeninfo finfo;
  struct fb_var_screeninfo vinfo;
  int voffset, i, r, g, b;
  dword dev_linelen, dev_xoffset, dev_yoffset, dev_video_size;

  if (device == NULL)
    strcpy(dev, "/dev/fb0");
  else
    strcpy(dev, device);

  dev_fd = open(dev, O_RDWR);   // root access rq
  if (dev_fd < 0)
    panic("can't open framebuffer device %s", dev);

  if (ioctl(dev_fd, FBIOGET_FSCREENINFO, &finfo))
    panic("error reading finfo");

  if (ioctl(dev_fd, FBIOGET_VSCREENINFO, &vinfo))
    panic("error reading vinfo");

  //
  dev_width = vinfo.xres;
  dev_height = vinfo.yres;
  dev_depth = vinfo.bits_per_pixel;
#if !defined(CPU_ARMLCD)
  if (dev_depth < 8)
    panic("framebuffer depth < 8bit not supported");
#endif

  dev_linelen = finfo.line_length;
  dev_xoffset = vinfo.xoffset;
  dev_yoffset = vinfo.yoffset;

  voffset = dev_yoffset * dev_linelen + dev_xoffset * dev_depth / 8;
  dev_video_size = dev_height * dev_linelen;

  // map pages into memory
  dev_smemlen = finfo.smem_len;
  dev_video =
  (unsigned char *)mmap(0, dev_smemlen, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd,
                        0);
  dev_video += voffset;

  if ((int)dev_video == -1)
    panic("mapping fbdev error");

  // TODO: create video pages
  dev_vpage = dev_video;

  // VGA palette
  switch (dev_depth) {
  case 8:
    /*
      fbcmap   = (struct fb_cmap *) malloc(sizeof(struct fb_cmap));
      fbcmap->start = 0;
      fbcmap->len   = 256;

      fbcmap->red    = (word *) malloc(512);
      fbcmap->green  = (word *) malloc(512);
      fbcmap->blue   = (word *) malloc(512);
      fbcmap->transp = (word *) malloc(512);

      ioctl(dev_fd, FBIOGETCMAP, fbcmap);

      for ( i = 0; i < 16; i ++ ) {
      fbcmap->red[i]    = (vga16[i] & 0xFF        ) << 8;
      fbcmap->green[i]  = ((vga16[i] & 0xFF00) >> 8   ) << 8;
      fbcmap->blue[i]   = ((vga16[i] & 0xFF0000) >> 16  ) << 8;
      fbcmap->transp[i] = 0;
      }

      ioctl(dev_fd, FBIOPUTCMAP, fbcmap);

      free(fbcmap->red);
      free(fbcmap->green);
      free(fbcmap->blue);
      free(fbcmap->transp);
      free(fbcmap);
    */
    for (i = 0; i < 16; i++)
      cmap[i] = i;
    break;
  case 15:
    for (i = 0; i < 16; i++) {
      r = ((vga16[i] >> 16) << 5) >> 8;
      g = (((vga16[i] & 0xFF00) >> 8) << 5) >> 8;
      b = ((vga16[i] & 0xFF) << 5) >> 8;
      cmap[i] = (r << 10) | (g << 5) | b;
    }
    break;
  case 16:
    for (i = 0; i < 16; i++) {
      r = ((vga16[i] >> 16) << 5) >> 8;
      g = (((vga16[i] & 0xFF00) >> 8) << 6) >> 8;
      b = ((vga16[i] & 0xFF) << 5) >> 8;
      cmap[i] = (r << 11) | (g << 5) | b;
    }
    break;
  case 24:
  case 32:
    for (i = 0; i < 16; i++)
      cmap[i] = vga16[i];
    break;
  };

  // vt-switch code
  vt_switch_init();
}

/*
 */
void fb_close()
{
  munmap((char *)dev_video, dev_smemlen);
  close(dev_fd);
  // vt-switch close
  vt_switch_destroy();
}

/*
 * SB: Initialization
 */
int osd_devinit()
{
  fb_init("/dev/fb0");

  os_graf_mx = dev_width;
  os_graf_my = dev_height;
  os_color_depth = dev_depth;
  os_color = 1;
  setsysvar_str(SYSVAR_OSNAME, "Unix/FB");

  gfb_init(dev_width, dev_height, dev_depth);

  osd_cls();
  return 1;
}

/*
 * display the current video page
 * (called every ~50ms)
 */
void osd_refresh()
{
  if (suspend)
    return;
  if (gfb_getuf()) {            // if it is modified
    memcpy(dev_vpage, gfb_vram(), gfb_vramsize());  // redraw
    gfb_resetuf();              // clear modified-flag
  }
}

/*
 * close
 */
int osd_devrestore()
{
  gfb_close();
  fb_close();
  return 1;
}

/*
 * enable or disable PEN code
 */
void osd_setpenmode(int enable)
{
  mouse_mode = enable;
}

/*
 * returns the status of the light-pen (mouse here)
 */
int osd_getpen(int code)
{
  int r = 0;

  osd_events();
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

/*
 * check events
 */
int osd_events()
{
  int r;
#if defined(DRV_MOUSE)
  int x, y, b;

  if (suspend)
    return 0;

  if (drvmouse_get(&x, &y, &b)) {
    mouse_x = x;
    mouse_y = y;

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
    return 1;
  }
#endif

  if (suspend)
    return 0;

  if ((r = term_events()) != 0) // keyboard events
    return r;
  return 0;
}

//////////

#if !defined(DRV_SOUND)

void osd_beep()
{
}

void osd_sound(int frq, int dur, int vol, int bgplay)
{
}

void osd_clear_sound_queue()
{
}

#endif
