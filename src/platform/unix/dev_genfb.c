// This file is part of SmallBASIC
//
// SmallBASIC, Generic 'framebuffer' techique graphics driver
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/*
 * This is a common routines for 'framebuffer' technique (not the linux-driver)
 * This driver stores the image into a buffer (dev_vpage) so the parent driver
 * can use that buffer to dump it into the screen (like bitmap, perhaps on every
 * osd_events() call).
 *
 * Notes:
 *   Parent driver must setup the palette for 8bpp modes (colors 0..15 to stdvga colors)
 *
 * Nicholas Christopoulos
 */

#include "platform/unix/dev_genfb.h"
#include "common/device.h"      // externals dev_fgcolor, dev_bgcolor

static int dev_width, dev_height, dev_depth;    // screen size
static int dev_linelen, dev_video_size, dev_bytespp;    // optimization
static byte *dev_vpage;         // framebuffer itself :)
static word *dev_vpage16;       // framebuffer itself :)
static dword *dev_vpage32;      // framebuffer itself :)
static long cmap[16];           // color map
static long dcolor;             // current color

// console/fonts
static int cur_x = 0;
static int cur_y = 0;
static int tabsize = 32;        // from dev_palm
static int maxline;
static int font_h = 16;
static int con_use_bold = 0;
static int con_use_ul = 0;
static int con_use_reverse = 0;

static int update;

// the font itself
#include "platform/unix/rom16.c"

// EGA/VGA16 colors in RGB
static dword vga16[] = {
  0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x7F7F7F,
  0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
};

// set/get pixel declarations
static void (*direct_setpixel) (int x, int y);
static long (*direct_getpixel) (int x, int y);

static void directsetpixel_1(int x, int y);
static void directsetpixel_2(int x, int y);
static void directsetpixel_4(int x, int y);
static void directsetpixel_8(int x, int y);
static void directsetpixel_16(int x, int y);
static void directsetpixel_24(int x, int y);
static void directsetpixel_32(int x, int y);

static long directgetpixel_1(int x, int y);
static long directgetpixel_2(int x, int y);
static long directgetpixel_4(int x, int y);
static long directgetpixel_8(int x, int y);
static long directgetpixel_16(int x, int y);
static long directgetpixel_24(int x, int y);
static long directgetpixel_32(int x, int y);

/*
* initialization
*/
int gfb_init(int width, int height, int bpp) {
  int i;

  dev_width = width;
  dev_height = height;
  dev_depth = bpp;

  if (bpp > 8)
    dev_bytespp = bpp / 8;
  else
    dev_bytespp = 1;

  if (dev_depth >= 8)
    dev_linelen = (dev_depth / 8) * dev_width;
  else if (dev_depth == 4)
    dev_linelen = dev_width / 2;
  else if (dev_depth == 2)
    dev_linelen = dev_width / 4;
  else if (dev_depth == 1)
    dev_linelen = dev_width / 8;

  dev_video_size = dev_linelen * dev_height;

  // setting up vram
  dev_vpage = tmp_alloc(dev_video_size + (dev_linelen * 17));   // the
  // additional
  // size is
  // added for
  // not checking 
  // drawchar
  // pixels
  dev_vpage16 = (word *) dev_vpage;
  dev_vpage32 = (dword *) dev_vpage;

  // setup driver & colors
  switch (dev_depth) {
  case 1:
    direct_setpixel = directsetpixel_1;
    direct_getpixel = directgetpixel_1;
    for (i = 0; i < 15; i++)
      cmap[i] = 0;
    cmap[15] = 1;
    break;
  case 2:
    direct_setpixel = directsetpixel_2;
    direct_getpixel = directgetpixel_2;

    for (i = 0; i < 16; i++)
      cmap[i] = ((15 - i) / 4);
    break;
  case 4:
    direct_setpixel = directsetpixel_4;
    direct_getpixel = directgetpixel_4;
    for (i = 0; i < 16; i++)
      cmap[i] = i;
    break;
  case 8:
    direct_setpixel = directsetpixel_8;
    direct_getpixel = directgetpixel_8;
    for (i = 0; i < 16; i++)
      cmap[i] = i;
    break;
  case 15:
    direct_setpixel = directsetpixel_16;
    direct_getpixel = directgetpixel_16;
    for (i = 0; i < 16; i++) {
      int r, g, b;

      r = ((vga16[i] >> 16) << 5) >> 8;
      g = (((vga16[i] & 0xFF00) >> 8) << 5) >> 8;
      b = ((vga16[i] & 0xFF) << 5) >> 8;
      cmap[i] = (r << 10) | (g << 5) | b;
    }
    break;
  case 16:
    direct_setpixel = directsetpixel_16;
    direct_getpixel = directgetpixel_16;
    for (i = 0; i < 16; i++) {
      int r, g, b;

      r = ((vga16[i] >> 16) << 5) >> 8;
      g = (((vga16[i] & 0xFF00) >> 8) << 6) >> 8;
      b = ((vga16[i] & 0xFF) << 5) >> 8;
      cmap[i] = ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
    }
    break;
  case 24:
    direct_setpixel = directsetpixel_24;
    direct_getpixel = directgetpixel_24;
    for (i = 0; i < 16; i++)
      cmap[i] = vga16[i];
    break;
  case 32:
    direct_setpixel = directsetpixel_32;
    direct_getpixel = directgetpixel_32;
    for (i = 0; i < 16; i++)
      cmap[i] = vga16[i];
    break;
  };

  // 
  cur_x = cur_y = 0;
  maxline = dev_height / font_h;
  update = 1;

  return 1;
}

/*
* set's the color map (palette)
*/
void gfb_setcmap(long *newcmap) {
  int i;

  for (i = 0; i < 16; i++)
    cmap[i] = newcmap[i];
}

/*
*/
int gfb_close() {
  tmp_free(dev_vpage);
  return 1;
}

/*
* returns a pointer to buffer
*/
byte *gfb_vram() {
  return dev_vpage;
}

/*
* returns the size of buffer (video ram)
*/
dword gfb_vramsize() {
  return dev_video_size;
}

/*
* resize video ram!
*/
void gfb_resize(int width, int height) {
  byte *newbuf;
  int i, min_ll, min_h, vsize, linelen = width;


  if (dev_depth >= 8)
    linelen = (dev_depth / 8) * width;
  else if (dev_depth == 4)
    linelen = width / 2;
  else if (dev_depth == 2)
    linelen = width / 4;
  else if (dev_depth == 1)
    linelen = width / 8;
  vsize = linelen * height;

  newbuf = tmp_alloc(vsize + (linelen * 17));

  min_h = (height < dev_height) ? height : dev_height;
  min_ll = (linelen < dev_linelen) ? linelen : dev_linelen;

  // setup the new vram
  memset(newbuf, 0, vsize);
  for (i = 0; i < min_h; i++)
    memcpy(newbuf + (i * linelen), dev_vpage + (i * dev_linelen), min_ll);

  // update globals
  tmp_free(dev_vpage);
  dev_vpage = newbuf;
  dev_linelen = linelen;
  dev_width = width;
  dev_height = height;
  dev_video_size = vsize;
  maxline = dev_height / font_h;
}

/*
* create new buffer
*/
byte *gfb_alloc() {
  byte *newbuf;

  newbuf = tmp_alloc(dev_video_size + (dev_linelen * 17));      // TODO: fix
  return newbuf;
}

/*
* create new buffer (clone of active buffer)
*/
byte *gfb_clone() {
  byte *newbuf;

  newbuf = tmp_alloc(dev_video_size + (dev_linelen * 17));
  memcpy(newbuf, dev_vpage, dev_video_size);
  return newbuf;
}

/*
* delete buffer
*/
void gfb_free(byte * buf) {
  tmp_free(buf);
}

/*
* set active buffer
*/
byte *gfb_switch(byte * buf) {
  byte *page = dev_vpage;

  if (buf) {
    dev_vpage = buf;
    dev_vpage16 = (word *) dev_vpage;
    dev_vpage32 = (dword *) dev_vpage;
  }
  return page;
}

/*
* returns the update-flag
*
* true = redraw req. (new things had been added)
* false = no redraw is req.
*/
int gfb_getuf() {
  return update;
}

/*
* clears the update-flag
*/
void gfb_resetuf() {
  update = 0;
}

/* ---------------------------------------------------------------------------------------------- */

/*
* 1bit mode 
*/
static void directsetpixel_1(int x, int y) {
  long offset;
  int shift, mask;

  offset = (y * (dev_width >> 3)) + (x >> 3);
  shift = 7 - (x % 8);
  mask = ((1 << shift) ^ 0xFF);
  dev_vpage[offset] = (dev_vpage[offset] & mask) | (dcolor << shift);
}

static long directgetpixel_1(int x, int y) {
  long offset;
  int shift, mask;

  offset = (y * dev_width / 8) + x / 8;
  shift = 7 - (x % 8);
  mask = (1 << shift);
  return ((dev_vpage[offset] & mask) >> shift);
}

/*
* 2bit mode 
*/
static void directsetpixel_2(int x, int y) {
  long offset;
  byte shift, mask;

  offset = (y * (dev_width >> 2)) + (x >> 2);
  switch (x % 4) {
  case 0:
    mask = 0x3F;
    shift = 6;
    break;
  case 1:
    mask = 0xCF;
    shift = 4;
    break;
  case 2:
    mask = 0xF3;
    shift = 2;
    break;
  default:                     // case 3:
    mask = 0xFC;
    shift = 0;
    break;
  }

  dev_vpage[offset] = (dev_vpage[offset] & mask) | (dcolor << shift);
}

static long directgetpixel_2(int x, int y) {
  long offset;
  byte shift, mask;

  offset = (y * dev_width / 4) + x / 4;
  switch (x % 4) {
  case 0:
    mask = 0xC0;
    shift = 6;
    break;
  case 1:
    mask = 0x30;
    shift = 4;
    break;
  case 2:
    mask = 0xC;
    shift = 2;
    break;
//      case    3:
  default:
    mask = 0x3;
    shift = 0;
    break;
  }

  return (dev_vpage[offset] & mask) >> shift;
}

/*
* 4bit mode
*/
static void directsetpixel_4(int x, int y) {
  long offset;

  offset = (y * (dev_width >> 1)) + (x >> 1);
#if defined(CPU_ARMLCD)
  if ((x % 2))
#else
  if ((x % 2) == 0)
#endif
    dev_vpage[offset] = (dev_vpage[offset] & 0xF) | (dcolor << 4);
  else
    dev_vpage[offset] = (dev_vpage[offset] & 0xF0) | dcolor;
}

static long directgetpixel_4(int x, int y) {
  long offset;
  int shift, mask;

  offset = (y * dev_width / 2) + x / 2;
  switch (x % 2) {
#if defined(CPU_ARMLCD)
  case 1:
#else
  case 0:
#endif
    mask = 0xF0;
    shift = 4;
    break;
  default:
    mask = 0xF;
    shift = 0;
    break;
  }
  return (dev_vpage[offset] & mask) >> shift;
}

/*
* 8bit mode
*/
static void directsetpixel_8(int x, int y) {
  dev_vpage[y * dev_linelen + x] = dcolor;
}

static long directgetpixel_8(int x, int y) {
  return dev_vpage[y * dev_linelen + x];
}

/*
* 15/16bit mode
*/
static void directsetpixel_16(int x, int y) {
  dev_vpage16[y * dev_width + x] = dcolor;
}

static long directgetpixel_16(int x, int y) {
  return dev_vpage16[y * dev_width + x];
}

/*
* 24bit mode
*/
static void directsetpixel_24(int x, int y) {
  long offset;

  offset = y * dev_linelen + x * 3;
  *(dev_vpage + offset) = (dcolor & 0xFF0000) >> 16;
  *(dev_vpage + offset + 1) = (dcolor & 0xFF00) >> 8;
  *(dev_vpage + offset + 2) = (dcolor & 0xFF);
}

static long directgetpixel_24(int x, int y) {
  long offset;

  offset = y * dev_linelen + x * 3;
  return (*(dev_vpage + offset) << 16) + (*(dev_vpage + offset + 1) << 8) + ((long)*(dev_vpage + offset + 2));
}

/*
* 32bit mode
*/
static void directsetpixel_32(int x, int y) {
  dev_vpage32[y * dev_width + x] = dcolor;
}

static long directgetpixel_32(int x, int y) {
  return dev_vpage32[y * dev_width + x];
}

/* ------------------------------------------------------------------------------------------------------ */

/*
* OSD API: 
*
* returns the color of the pixel (at x,y)
* The returned color can be the actuall color
* or the color-index (see cmap)
*/
long osd_getpixel(int x, int y) {
  int i;
  long color;

  if (x >= 0 && x < dev_width) {
    if (y >= 0 && y < dev_height) {
      color = direct_getpixel(x, y);

      for (i = 0; i < 16; i++) {
        if (color == cmap[i])
          return i;
      }
      return color;
    }
  }

  return 0;
}

/*
* draw horizontial line
*/
static void direct_hline(int x, int x2, int y) {
  long offset, i, len;
  long co;
  byte l[2];

  if (x > x2) {
    i = x;
    x = x2;
    x2 = i;
  }

  if (x < 0)
    x = 0;
  if (x2 < 0)
    return;
  if (x2 >= dev_width)
    x2 = dev_width - 1;
  if (y < 0)
    return;
  if (y >= dev_height)
    return;

  len = (x2 - x) + 1;
  switch (dev_depth) {
  case 1:
  case 2:
    for (i = x; i <= x2; i++)
      direct_setpixel(i, y);
    break;
  case 4:
    co = (dcolor << 4) | dcolor;
    l[0] = (x % 2);
    l[1] = (x2 % 2);
    if (l[0]) {
      direct_setpixel(x, y);
      x++;
    }
    if (l[1] == 0) {
      direct_setpixel(x2, y);
      x2--;
    }

    if (x2 - x < 1)
      direct_setpixel(x, y);
    else {
      len = ((x2 - x) + 1) >> 1;
      offset = (y * (dev_width >> 1)) + (x >> 1);
      memset(dev_vpage + offset, co, len);
    }
    return;
  case 8:
    offset = y * dev_linelen + x;
    memset(dev_vpage + offset, dcolor, len);
    return;
  case 15:
  case 16:
    offset = (y * dev_linelen) + (x << 1);
    for (i = 0; i < len; i++, offset += 2)
      *((word *) (dev_vpage + offset)) = dcolor;
    return;
  case 24:
    offset = (y * dev_linelen) + (x * 3);
    for (i = 0; i < len; i++) {
      *(dev_vpage + offset) = (dcolor & 0xFF0000) >> 16;
      offset++;
      *(dev_vpage + offset) = (dcolor & 0xFF00) >> 8;
      offset++;
      *(dev_vpage + offset) = (dcolor & 0xFF);
      offset++;
    }
    return;
  case 32:
    offset = (y * dev_linelen) + (x << 2);
    for (i = 0; i < len; i++, offset += 4)
      *((long *)(dev_vpage + offset)) = dcolor;
    return;
  }
}

/*
* Bresenham's algorithm for drawing line 
*/
static void direct_line(int x1, int y1, int x2, int y2) {
  g_line(x1, y1, x2, y2, direct_setpixel);
  update = 1;
}

/*
*/
static void direct_fillrect(int x1, int y1, int x2, int y2) {
  int i;

  for (i = y1; i <= y2; i++)
    direct_hline(x1, x2, i);
}

/*
* OSD API:
*
* Clear screen
*/
void osd_cls() {
  long color = dcolor;

  cur_x = cur_y = 0;
  dcolor = cmap[15];
  direct_fillrect(0, 0, dev_width - 1, dev_height - 1);
  dcolor = color;
  update = 1;
}

/*
* OSD API:
*
* returns the current x position
*/
int osd_getx() {
  return cur_x;
}

/*
* OSD API:
*
* returns the current y position
*/
int osd_gety() {
  return cur_y;
}

/*
* OSD API:
*
* sets x,y position (for text)
*/
void osd_setxy(int x, int y) {
  cur_x = x;
  cur_y = y;
}

/*
* next line
*/
static void osd_nextln() {
  cur_x = 0;

  if (cur_y < ((maxline - 1) * font_h)) {
    cur_y += font_h;
  } else {
    int len, to;
    long color = dcolor;

    // scroll
    len = cur_y * dev_linelen;
    to = font_h * dev_linelen;
    if (to + len >= dev_height * dev_linelen)
      len = ((dev_height * dev_linelen) - dev_bytespp) - to;
    memcpy((char *)dev_vpage, (char *)dev_vpage + to, len);

    // clear
    dcolor = cmap[15];
    direct_fillrect(0, cur_y, dev_width - 1, cur_y + font_h);
    dcolor = color;
  }
}

/*
* calc next tab position
*/
static int osd_calctab(int x) {
  int c = 1;

  while (x > tabsize) {
    x -= tabsize;
    c++;
  }
  return c * tabsize;
}

/*
* drawing bitmap character (8x16)
*/
static void direct_drawchar(int x, int y, byte ch, int overwrite, int fg_rgb, int bg_rgb) {
  byte *char_offset;
  int bit, i;
  long oldcolor = dcolor;

  char_offset = font8x16 + ch * 16;

  for (i = 0; i < 16; i++, char_offset++) {
    for (bit = 0; bit < 8; bit++) {
      if (*char_offset & (1 << (8 - bit))) {
        dcolor = fg_rgb;
        direct_setpixel(x + bit, y + i);
      } else if (overwrite) {
        dcolor = bg_rgb;
        direct_setpixel(x + bit, y + i);
      }
    }
  }

  dcolor = oldcolor;
}

/**
* OSD API:
* Basic output
*
* Supported control codes:
* \t    tab (20 px)
* \a    beep
* \n    next line (cr/lf)
* \xC   clear screen
* \e[K  clear to end of line
* \e[0m reset all attributes to their defaults
* \e[1m set bold on
* \e[4m set underline on
* \e[7m reverse video
* \e[21m  set bold off
* \e[24m  set underline off
* \e[27m  set reverse off
*/
void osd_write(const char *str) {
  int len, cx = 8, esc_val, esc_cmd;
  byte *p, buf[3];
  long color = dcolor;

  len = strlen(str);

  if (len <= 0)
    return;

  p = (byte *) str;
  while (*p) {
    switch (*p) {
    case '\a':                 // beep
      dev_beep();
      break;
    case '\t':
      cur_x = osd_calctab(cur_x + 1);
      break;
    case '\xC':
      osd_cls();
      break;
    case '\033':               // ESC ctrl chars (man console_codes)
      if (*(p + 1) == '[') {
        p += 2;
        esc_val = esc_cmd = 0;

        if (is_digit(*p)) {
          esc_val = (*p - '0');
          p++;

          if (is_digit(*p)) {
            esc_val = (esc_val * 10) + (*p - '0');
            p++;
          }

          esc_cmd = *p;
        } else
          esc_cmd = *p;

        // control characters
        switch (esc_cmd) {
        case 'K':              // \e[K - clear to eol
          dcolor = cmap[15];
          direct_fillrect(cur_x, cur_y, dev_width - cur_x, cur_y + font_h);
          dcolor = color;
          break;
        case 'G':
          dev_setxy(esc_val * 8, dev_gety(), 0);   // default font = 9x16
          break;
        case 'm':              // \e[...m - ANSI terminal
          switch (esc_val) {
          case 0:              // reset
            con_use_bold = 0;
            con_use_ul = 0;
            con_use_reverse = 0;
            osd_setcolor(0);
            osd_settextcolor(0, 15);
            break;
          case 1:              // set bold on
            con_use_bold = 1;
            break;
          case 4:              // set underline on
            con_use_ul = 1;
            break;
          case 7:              // reverse video on
            con_use_reverse = 1;
            break;
          case 21:             // set bold off
            con_use_bold = 0;
            break;
          case 24:             // set underline off
            con_use_ul = 0;
            break;
          case 27:             // reverse video off
            con_use_reverse = 0;
            break;

            // colors - 30..37 foreground, 40..47 background
          case 30:             // set black fg
            osd_setcolor(0);
            break;
          case 31:             // set red fg
            osd_setcolor(4);
            break;
          case 32:             // set green fg
            osd_setcolor(2);
            break;
          case 33:             // set brown fg
            osd_setcolor(6);
            break;
          case 34:             // set blue fg
            osd_setcolor(1);
            break;
          case 35:             // set magenta fg
            osd_setcolor(5);
            break;
          case 36:             // set cyan fg
            osd_setcolor(3);
            break;
          case 37:             // set white fg
            osd_setcolor(7);
            break;

          case 40:             // set black bg
            osd_settextcolor(cmap[dev_fgcolor], 0);
            break;
          case 41:             // set red bg
            osd_settextcolor(cmap[dev_fgcolor], 4);
            break;
          case 42:             // set green bg
            osd_settextcolor(cmap[dev_fgcolor], 2);
            break;
          case 43:             // set brown bg
            osd_settextcolor(cmap[dev_fgcolor], 6);
            break;
          case 44:             // set blue bg
            osd_settextcolor(cmap[dev_fgcolor], 1);
            break;
          case 45:             // set magenta bg
            osd_settextcolor(cmap[dev_fgcolor], 5);
            break;
          case 46:             // set cyan bg
            osd_settextcolor(cmap[dev_fgcolor], 3);
            break;
          case 47:             // set white bg
            osd_settextcolor(cmap[dev_fgcolor], 7);
            break;

          };
          break;
        }
      }
      break;
    case '\n':                 // new line
      osd_nextln();
      break;
    case '\r':                 // return
      cur_x = 0;
      dcolor = cmap[15];
      direct_fillrect(cur_x, cur_y, dev_width - cur_x, cur_y + font_h);
      dcolor = color;
      break;
    default:
      // 
      // PRINT THE CHARACTER
      // 
      buf[0] = *p;
      buf[1] = '\0';

      // new line ?
      if (cur_x + cx >= dev_width)
        osd_nextln();

      // draw

      // TODO: ??? SJIS on Linux ???
      if (!con_use_reverse) {
        direct_drawchar(cur_x, cur_y, *p, 1, cmap[dev_fgcolor], cmap[dev_bgcolor]);
        if (con_use_bold)
          direct_drawchar(cur_x + 1, cur_y, *p, 0, cmap[dev_fgcolor], cmap[dev_bgcolor]);
      } else {
        direct_drawchar(cur_x, cur_y, *p, 1, cmap[dev_bgcolor], cmap[dev_fgcolor]);
        if (con_use_bold)
          direct_drawchar(cur_x + 1, cur_y, *p, 0, cmap[dev_bgcolor], cmap[dev_fgcolor]);
      }

      if (con_use_ul) {
        osd_setcolor(dev_fgcolor);
        direct_line(cur_x, (cur_y + font_h) - 1, cur_x + cx, (cur_y + font_h) - 1);
      }
      // advance
      cur_x += cx;
    };

    if (*p == '\0')
      break;

    p++;
  }

  update = 1;
}

/*
* OSD API:
*
* Sets the current drawing color
*/
void osd_setcolor(long color) {
  dev_fgcolor = color;
  if (dev_fgcolor >= 0 && dev_fgcolor <= 16)
    dcolor = cmap[dev_fgcolor];
  else
    dcolor = ABS(dev_fgcolor);
}

/*
* OSD API:
*
* Sets the current drawing color and the background color (used for text)
*/
void osd_settextcolor(long fg, long bg) {
  osd_setcolor(fg);
  if (bg != -1)
    dev_bgcolor = bg;
}

/*
* OSD API:
*
* draw a line
*/
void osd_line(int x1, int y1, int x2, int y2) {
  if ((x1 == x2) && (y1 == y2))
    direct_setpixel(x1, y1);
  else
    direct_line(x1, y1, x2, y2);
  update = 1;
}

/*
* OSD API:
*
* draw a pixel
*/
void osd_setpixel(int x, int y) {
  direct_setpixel(x, y);
  update = 1;
}

/*
* OSD API:
*
* draw a parallelogram (filled or not)
*/
void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  int y;

  if (fill) {
    for (y = y1; y <= y2; y++)
      direct_hline(x1, x2, y);
  } else {
    direct_line(x1, y1, x1, y2);
    direct_line(x1, y2, x2, y2);
    direct_line(x2, y2, x2, y1);
    direct_line(x2, y1, x1, y1);
  }
  update = 1;
}

/*
* OSD API:
*
* returns the width of the text str in pixels
*/
int osd_textwidth(const char *str) {
  return strlen(str) * 8;
}

/*
* OSD API:
*
* returns the height of the text str in pixels
*/
int osd_textheight(const char *str) {
  return font_h;
}
