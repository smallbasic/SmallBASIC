/*
 * SmallBASIC platform driver for Unix lcgi
 *
 * ndc: 2001-03-03
 */

#include "device.h"
#include "osd.h"
#include "str.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(_UnixOS)
#include <unistd.h>
#endif
#include <lcgi/graphics.h>
#include <ctype.h>
#include "g_bmp.h"

extern char *prog_file;

static int cur_x = 0;
static int cur_y = 0;
static int bytespp = 1;
static int tabsize = 32;        // from dev_palm
static int maxline;
static int font_h = 16;

static int con_use_bold = 0;
static int con_use_ul = 0;
static int con_use_reverse = 0;
static long dcolor, dcolor_bg;
static int mouse_mode = 0;

// VGA16 colors in RGB
static unsigned long vga16[] = {
  0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
  0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
};

static long cmap[16];

// fonts
#include "unix/rom16.c"

static int scr_update = 0;

#define LOCK    mouse_hide
#define UNLOCK    mouse_visible

/*
 */
struct key_t {
  int code;
  int sbkey;
};
static struct key_t keytable[] = {
  //{ Key_Escape,         SB_KEY_ESC      },
  {Key_Tab, 9},
  //{ Key_Backtab,        8                       },
  {Key_Backspace, 8},
  {Key_Return, 10},
  {Key_Enter, 10},
  {Key_Insert, SB_KEY_INSERT},
  {Key_Delete, 127},
  //{ Key_Pause,          0               },
  //{ Key_Print,          0               },
  //{ Key_Sysreq,         0               },
  {Key_Home, SB_KEY_HOME},
  {Key_End, SB_KEY_END},
  {Key_Left, SB_KEY_LEFT},
  {Key_Up, SB_KEY_UP},
  {Key_Right, SB_KEY_RIGHT},
  {Key_Down, SB_KEY_DOWN},
  {Key_Pageup, SB_KEY_PGUP},
  {Key_Pagedown, SB_KEY_PGDN},

  //    Key_Shift,    Key_Control, Key_Meta,       Key_Alt,
  //    Key_Capslock, Key_Numlock, Key_Scrolllock,

  {Key_F1, SB_KEY_F(1)},
  {Key_F2, SB_KEY_F(2)},
  {Key_F3, SB_KEY_F(3)},
  {Key_F4, SB_KEY_F(4)},
  {Key_F5, SB_KEY_F(5)},
  {Key_F6, SB_KEY_F(6)},
  {Key_F7, SB_KEY_F(7)},
  {Key_F8, SB_KEY_F(8)},
  {Key_F9, SB_KEY_F(9)},
  {Key_F10, SB_KEY_F(10)},
  {Key_F11, SB_KEY_F(11)},
  {Key_F12, SB_KEY_F(12)},

  /*
  //...... 7-Bit druckbare ASCII-Tasten
  Key_Space,     Key_Exclam,     Key_Quotedbl,  Key_Numbersign,
  Key_Dollar,    Key_Percent,    Key_Ampersand, Key_Apostrophe,
  Key_Parenleft, Key_Parenright, Key_Asterisk,  Key_Plus,
  Key_Comma,     Key_Minus,      Key_Period,    Key_Slash,
  Key_0, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9,
  Key_Colon, Key_Semicolon,
  Key_Less,  Key_Equal,     Key_Greater, Key_Question, Key_At,
  Key_A, Key_B, Key_C, Key_D, Key_E, Key_F, Key_G, Key_H, Key_I, Key_J,
  Key_K, Key_L, Key_M, Key_N, Key_O, Key_P, Key_Q, Key_R, Key_S, Key_T,
  Key_U, Key_V, Key_W, Key_X, Key_Y, Key_Z,
  Key_Bracketleft, Key_Backslash, Key_Bracketright, Key_Asciicircum,
  Key_Underscore,  Key_Quoteleft, Key_Braceleft,    Key_Bar,
  Key_Braceright,  Key_Asciitilde
  */
  {0, 0}
};

/*
 * Initialization
 */
int osd_devinit()
{
  int w = 800, h = 600, d = 16;

  /*
   * default video mode
   */
  if (getenv("SB_SDLMODE") || getenv("SBGRAF")) {
    char *buf, *p, *ps;

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
  }

  initgraph_(g_argc, g_argv, w, h);

  os_graf_mx = getmaxx();
  os_graf_my = getmaxy();
  maxline = os_graf_my / font_h;
  os_color_depth = getmaxcolor() / 8;
  os_color = 1;
  bytespp = os_color_depth;

  setsysvar_str(SYSVAR_OSNAME, "Unix/lcgi");
  setsysvar_int(SYSVAR_VIDADR, 0);
  osd_settextcolor(0, 15);
  osd_cls();

  return 1;
}

/*
 * close
 */
int osd_devrestore()
{
  cur_x = 0;
  cur_y = (maxline - 1) * font_h;
  osd_write("LCGI: Press any key to exit...");

  while (!kbhit());
  closegraph();

  return 1;
}

/*
 * return's the value of the pixel
 */
long osd_getpixel(int x, int y)
{
  return getpixel(x, y);
}

void osd_settextcolor(long fg, long bg)
{
  osd_setcolor(fg);
  if (bg != -1) {
    if (bg >= 0) {
      dev_bgcolor = bg;
      dcolor_bg = cmap[bg];
    }
    else
      dcolor_bg = -bg;
  }
}


//
void osd_fillrect(int x1, int y1, int x2, int y2, long color)
{
  int y;

  setcolor(color);
  for (y = y1; y <= y2; y++)
    line(x1, y, y1, y);
  setcolor(dcolor);
}

//
void osd_drawchar(int x, int y, byte ch, int overwrite, int fg_rgb, int bg_rgb)
{
  int offset;
  int bit, i;

  offset = ch * 16;

  for (i = 0; i < 16; i++, offset++) {
    for (bit = 0; bit < 8; bit++) {
      if (*(font8x16 + offset) & (1 << (8 - bit)))
        putpixel(x + bit, y + i, fg_rgb);
      else if (overwrite)
        putpixel(x + bit, y + i, bg_rgb);
    }
  }
}

/*
 * enable or disable PEN code
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
  static int pmx, pmy, pmb;
  int mx, my, mb;
  static int ppbx, ppby;

  osd_events();
  mouse_getpos(&mx, &my);
  mb = mouse_button();

  if (mouse_mode) {
    switch (code) {
    case 0:                    // bool: status changed
      if (pmx != mx || pmy != my || pmb != mb)
        r = 1;
      break;
    case 1:                    // last pen-down x
      r = ppbx;
      break;
    case 2:                    // last pen-down y
      r = ppby;
      break;
    case 3:                    // vert. 1 = down, 0 = up .... unsupported
      r = (mb & 1);
      break;
    case 4:                    // last x
      if (mb & 1)
        r = pmx;
      break;
    case 5:                    // last y
      if (mb & 1)
        r = pmy;
      break;
    case 10:
      r = mx;
      break;
    case 11:
      r = my;
      break;
    case 12:
      r = mouse_left();
      break;
    case 13:
      r = mouse_right();
      break;
    case 14:
      r = mouse_mid();
      break;
    }

    pmx = mx;
    pmy = my;
    pmb = mb;

    if (mb) {
      ppbx = mx;
      ppby = my;
    }
  }
  return r;
}

/*
 * clear screen
 */
void osd_cls()
{
  cur_x = cur_y = 0;

  LOCK();
  cleardevice(dcolor_bg);
  UNLOCK();
}

//      returns the current x position
int osd_getx()
{
  return cur_x;
}

//      returns the current y position
int osd_gety()
{
  return cur_y;
}

//
void osd_setxy(int x, int y)
{
  cur_x = x;
  cur_y = y;
}

/**
 * next line
 */
void osd_nextln()
{
  cur_x = 0;

  if (cur_y < ((maxline - 1) * font_h)) {
    cur_y += font_h;
  }
  else {
    /*
      int   len, to;

      // scroll
      len = cur_y * bytespp * os_graf_mx;
      to  = font_h * bytespp * os_graf_mx;
      if  ( to + len >= os_graf_my * screen->pitch )
      len = ((os_graf_my * screen->pitch) - bytespp) - to;
      memcpy((char*) screen->pixels, (char*) screen->pixels + to, len);

      direct_fillrect(0, cur_y, os_graf_mx-1, cur_y+font_h, cmap[15]);
    */
  }
}

/*
 * calc next tab position
 */
int osd_calctab(int x)
{
  int c = 1;

  while (x > tabsize) {
    x -= tabsize;
    c++;
  }
  return c * tabsize;
}

/**
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
void osd_write(const char *str)
{
  int len, cx = 8, esc_val, esc_cmd;
  byte *p, buf[3];

  len = strlen(str);

  if (len <= 0)
    return;

  LOCK();

  p = (byte *) str;
  while (*p) {
    switch (*p) {
    case '\a':                 // beep
      osd_beep();
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
        }
        else
          esc_cmd = *p;

        // control characters
        switch (esc_cmd) {
        case 'K':              // \e[K - clear to eol
          osd_fillrect(cur_x, cur_y, os_graf_mx - cur_x, cur_y + font_h, cmap[15]);
          break;
        case 'G':
          dev_setxy(esc_val * 8, dev_gety()); // default font = 9x16
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
            osd_settextcolor(dev_fgcolor, 0);
            break;
          case 41:             // set red bg
            osd_settextcolor(dev_fgcolor, 4);
            break;
          case 42:             // set green bg
            osd_settextcolor(dev_fgcolor, 2);
            break;
          case 43:             // set brown bg
            osd_settextcolor(dev_fgcolor, 6);
            break;
          case 44:             // set blue bg
            osd_settextcolor(dev_fgcolor, 1);
            break;
          case 45:             // set magenta bg
            osd_settextcolor(dev_fgcolor, 5);
            break;
          case 46:             // set cyan bg
            osd_settextcolor(dev_fgcolor, 3);
            break;
          case 47:             // set white bg
            osd_settextcolor(dev_fgcolor, 7);
            break;

          };
          break;
        }
      }
      break;
    case '\n':                 // new line
      UNLOCK();
      osd_nextln();
      LOCK();
      break;
    case '\r':                 // return
      cur_x = 0;
      osd_fillrect(cur_x, cur_y, os_graf_mx - cur_x, cur_y + font_h, cmap[15]);
      break;
    default:
      //
      // PRINT THE CHARACTER
      //
      buf[0] = *p;
      buf[1] = '\0';

      // new line ?
      if (cur_x + cx >= os_graf_mx)
        osd_nextln();

      // draw

      // TODO: ??? SJIS on Linux ???
      if (!con_use_reverse) {
        osd_drawchar(cur_x, cur_y, *p, 1, cmap[dev_fgcolor], cmap[dev_bgcolor]);
        if (con_use_bold)
          osd_drawchar(cur_x - 1, cur_y, *p, 0, cmap[dev_fgcolor],
                       cmap[dev_bgcolor]);
      }
      else {
        osd_drawchar(cur_x, cur_y, *p, 1, cmap[dev_bgcolor], cmap[dev_fgcolor]);
        if (con_use_bold)
          osd_drawchar(cur_x - 1, cur_y, *p, 0, cmap[dev_bgcolor],
                       cmap[dev_fgcolor]);
      }

      if (con_use_ul) {
        osd_setcolor(dev_fgcolor);
        line(cur_x, (cur_y + font_h) - 1, cur_x + cx, (cur_y + font_h) - 1);
      }

      // advance
      cur_x += cx;
    };

    if (*p == '\0')
      break;

    p++;
  }

  UNLOCK();
}

/*
 * check events
 */
int osd_events()
{
  int ch, i, evc = 0;

  while (kbhit()) {
    ch = getch();
    if (ch == 0) {
      ch = getch();
      for (i = 0; keytable[i].code; i++) {
        if (keytable[i].code == ch) {
          dev_pushkey(keytable[i].sbkey);
          break;
        }
      }
    }
    else
      dev_pushkey(ch);
    evc++;
  }

  return evc;
}

///////////////////////////////////////////////////////////////

void osd_setcolor(long color)
{
  dev_fgcolor = color;
  if (color >= 0 && color <= 15)
    dcolor = cmap[color];
  else if (color < 0)
    dcolor = -color;
  setcolor(dcolor);
}

void osd_line(int x1, int y1, int x2, int y2)
{
  LOCK();
  if ((x1 == x2) && (y1 == y2))
    putpixel(x1, y1, dcolor);
  else
    line(x1, y1, x2, y2);
  UNLOCK();
}

void osd_setpixel(int x, int y)
{
  LOCK();
  putpixel(x, y, dcolor);
  UNLOCK();
}

void osd_rect(int x1, int y1, int x2, int y2, int fill)
{
  int y;

  LOCK();

  if (fill) {
    for (y = y1; y <= y2; y++)
      line(x1, y, x2, y);
  }
  else {
    line(x1, y1, x1, y2);
    line(x1, y2, x2, y2);
    line(x2, y2, x2, y1);
    line(x2, y1, x1, y1);
  }

  UNLOCK();
}

///////////////////////////////////////////////////////////////

#if !defined(DRV_SOUND)
void osd_sound(int freq, int ms, int vol, int bgplay)
{
}

void osd_beep()
{
}

void osd_clear_sound_queue()
{
}
#endif

///////////////////////////////////////////////////////////////

int osd_textwidth(const char *str)
{
  int l = strlen(str);

  // SJIS ???
  return l * 8;
}

int osd_textheight(const char *str)
{
  // TODO: count \n
  return font_h;
}

void osd_refresh()
{
}

/// bitmap

/*
 * the size of the bitmap
 */
long osd_bmpsize(int x1, int y1, int x2, int y2)
{
  return bmp_size(x1, y1, x2, y2, bytespp * 8);
}

/*
 * stores video-rect to buf
 */
void osd_bmpget(int x1, int y1, int x2, int y2, char *buf)
{
  bmp_get((bmp_header_t *) buf, x1, y1, x2, y2, bytespp * 8, osd_getpixel);
}

/*
 * writes image to video
 */
void osd_bmpput(int x, int y, int write_mode, char *buf)
{
  bmp_t *bmp, *newbmp;

  bmp = (bmp_t *) buf;
  if (write_mode) {
    newbmp = malloc(bmp_size(x, y, x + bmp->dx, y + bmp->dy, bytespp * 8));
    bmp_get((bmp_header_t *) buf, x, y, x + bmp->dx, y + bmp->dy, bytespp * 8,
            osd_getpixel);
    bmp_combine((byte *) (newbmp + sizeof(bmp_header_t)),
                (byte *) (bmp + sizeof(bmp_header_t)), bytespp * bmp->dx, bmp->dy,
                write_mode);
    bmp_put(newbmp, x, y, osd_setpixel);
    free(newbmp);
  }
  else
    bmp_put(bmp, x, y, osd_setpixel);
}
