/*
*	SmallBASIC platform driver for Unix|Win32 + SDL
*
*	ndc: 2001-03-03
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
#include <SDL/SDL.h>
#include <ctype.h>
#include "g_bmp.h"

extern char *prog_file;

static int cur_x = 0;
static int cur_y = 0;
static int bytespp = 1;
static int mouse_mode, mouse_x, mouse_y, mouse_b, mouse_upd, mouse_down_x,
  mouse_down_y, mouse_pc_x, mouse_pc_y;
static int tabsize = 32;        // from dev_palm
static int maxline;
static int font_h = 16;

static int con_use_bold = 0;
static int con_use_ul = 0;
static int con_use_reverse = 0;
static long dcolor;

// VGA16 colors in RGB
static unsigned long vga16[] = {
  0x0, 0x7F, 0x7F00, 0x7F7F, 0x7F0000, 0x7F007F, 0x7F7F00, 0x808080,
  0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
};

static long cmap[16];

// fonts
#include "unix/rom16.c"

static SDL_Surface *screen;
static int has_audio = 0;
static int mouse_hot_x = -16, mouse_hot_y = -16;
static SDL_AudioSpec audiospec;
static int scr_update = 0;

struct voice_info {
  float period;
  float toggle;
  int setting;
  int vol;
};

static char *mixbuf;
#define	AUDIO_STACK_MAX		8192*2
static struct voice_info audio_info[AUDIO_STACK_MAX];
static int audio_head = 0;
static int audio_tail = 0;

/*
*/
static int keymap[] = {
  SDLK_CAPSLOCK, -1,
  SDLK_LSHIFT, -1,
  SDLK_RSHIFT, -1,
  SDLK_LALT, -1,
  SDLK_RALT, -1,
  SDLK_LCTRL, -1,
  SDLK_RCTRL, -1,
  SDLK_RETURN, '\n',
#if defined(_UnixOS)
  8, 127,
  127, 8,
#endif
  0x111, SB_KEY_UP,
  0x108, SB_KEY_UP,
  0x112, SB_KEY_DN,
  0x102, SB_KEY_DN,
  0x114, SB_KEY_LEFT,
  0x104, SB_KEY_LEFT,
  0x113, SB_KEY_RIGHT,
  0x106, SB_KEY_RIGHT,
  0x118, SB_KEY_PGUP,
  0x109, SB_KEY_PGUP,
  0x119, SB_KEY_PGDN,
  0x103, SB_KEY_PGDN,
  0x116, SB_KEY_HOME,
  0x107, SB_KEY_HOME,
  0x117, SB_KEY_END,
  0x101, SB_KEY_END,
  0x115, SB_KEY_INSERT,
  0x11A, SB_KEY_F(1),
  0x11B, SB_KEY_F(2),
  0x11C, SB_KEY_F(3),
  0x11D, SB_KEY_F(4),
  0x11E, SB_KEY_F(5),
  0x11F, SB_KEY_F(6),
  0x120, SB_KEY_F(7),
  0x121, SB_KEY_F(8),
  0x122, SB_KEY_F(9),
  0x123, SB_KEY_F(10),
  0x124, SB_KEY_F(11),
  0x125, SB_KEY_F(12),
  0, 0
};

#define	LOCK()		SDL_LockSurface(screen)
#define	UNLOCK()	{ SDL_UnlockSurface(screen); scr_update = 1; }


/*
*	I know nothing about sound.
*	someone to fix it
*/
void audio_callback(void *user, unsigned char *stream, int length)
{
  int volume;
  int mix = 0;
  struct voice_info *info;

  memset(mixbuf, audiospec.silence, length);
  if (audio_head != audio_tail) {
    info = &audio_info[audio_head];
    volume = (info->vol * SDL_MIX_MAXVOLUME) / 100;

    if (info->period >= 1.0) {
      int left = length, j = 0;
      int count;

      mix = 1;
      do {
        count = (info->toggle < left) ? (int)info->toggle : left;
        left -= count;
        info->toggle -= count;
        while (count--)
          mixbuf[j++] += info->setting;

        if (info->toggle < 1.0) {
          info->toggle += info->period;
          info->setting = -info->setting;
        }
      } while (left > 0);

    }

    audio_head++;
    if (audio_head >= AUDIO_STACK_MAX)
      audio_head = 0;

    if (mix)
      SDL_MixAudio(stream, mixbuf, length, volume);
  }
}

/*
*	SDL: Initialization
*/
int osd_devinit()
{
  SDL_Color colors[256];
  int i, w = 800, h = 600, d = 16;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL: Couldn't initialize SDL: %s\n", SDL_GetError());
    return 0;
  }

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

  /*
   * setup video mode 
   */
  screen = SDL_SetVideoMode(w, h, d, 0);
  if (screen == NULL) {
    fprintf(stderr,
            "SDL: Couldn't set %dx%dx%d video mode: %s\nUse SB_SDLMODE environment variable (set SB_SDLMODE=800x600x16)\n",
            w, h, d, SDL_GetError());
    return 0;
  }
  has_audio = (SDL_InitSubSystem(SDL_INIT_AUDIO) >= 0);

  os_graf_mx = screen->w;
  os_graf_my = screen->h;
  maxline = os_graf_my / font_h;
  os_color_depth = screen->format->BitsPerPixel;
  os_color = 1;
  if (os_color_depth != 8 && os_color_depth != 16 && os_color_depth != 32) {
    fprintf(stderr, "SDL: Couldn't set video mode\n");
    return 0;
  }

  {
    char buf[1024];

//              sprintf(buf, "SmallBASIC/SDL %dx%dx%d - %s", w, h, d, prog_file);
    SDL_WM_SetCaption(buf, NULL);
  }

  bytespp = screen->format->BytesPerPixel;

  if (os_color_depth == 8) {
    for (i = 0; i < 256; i++) {
      colors[i].r = i;
      colors[i].g = i;
      colors[i].b = i;
    }
    for (i = 0; i < 16; i++) {
      colors[i].r = (vga16[i] & 0xFF0000) >> 16;
      colors[i].g = (vga16[i] & 0xFF00) >> 8;
      colors[i].b = (vga16[i] & 0xFF);
      cmap[i] = i;
    }
    SDL_SetColors(screen, colors, 0, 256);
  }
  else {
    for (i = 0; i < 16; i++) {
      cmap[i] = SDL_MapRGB(screen->format,
                           (vga16[i] & 0xFF0000) >> 16,
                           (vga16[i] & 0xFF00) >> 8, (vga16[i] & 0xFF));
    }
  }

  if (has_audio) {
    SDL_AudioSpec wanted;

    memset(&wanted, 0, sizeof(SDL_AudioSpec));

    // Set the audio format
    wanted.freq = 44100;
    wanted.format = AUDIO_U8;
    wanted.channels = 1;
    wanted.samples = 4096;
    wanted.callback = audio_callback;
    wanted.userdata = NULL;

    // Open the audio device, forcing the desired format
    if (SDL_OpenAudio(&wanted, &audiospec) < 0)
      has_audio = 0;
    else {
      mixbuf = (char *)malloc(0x10000);
      SDL_PauseAudio(0);
    }
  }

#if defined(_UnixOS)
  setsysvar_str(SYSVAR_OSNAME, "Unix/SDL");
#else
  setsysvar_str(SYSVAR_OSNAME, "Win32/SDL");
#endif
  setsysvar_int(SYSVAR_VIDADR, (int32) screen->pixels);
  osd_settextcolor(0, 15);
  osd_cls();

  return 1;
}

/*
*	close
*/
int osd_devrestore()
{
  cur_x = 0;
  cur_y = (maxline - 1) * font_h;
  osd_write("SDL: Press any key to exit...");
  SDL_PauseAudio(1);
  while (!dev_kbhit());
  SDL_Quit();
  if (mixbuf)
    free(mixbuf);

  return 1;
}

/*
*/
void direct_setpixel(int x, int y, long c)
{
  int offset;

  if (x >= 0 && x < screen->w) {
    if (y >= 0 && y < screen->h) {
      offset = y * screen->pitch;
      switch (bytespp) {
      case 1:
        offset += x;
        *((byte *) (screen->pixels + offset)) = c;
        break;
      case 2:
        offset += (x << 1);
        *((word *) (screen->pixels + offset)) = c;
        break;
      case 3:
        offset += x * 3;
#if defined(CPU_BIGENDIAN)
        *((byte *) (screen->pixels + offset)) = (dcolor & 0xFF0000) >> 16;
        *((byte *) (screen->pixels + offset + 1)) = (dcolor & 0xFF00) >> 8;
        *((byte *) (screen->pixels + offset + 2)) = (dcolor & 0xFF);
#else
        *((byte *) (screen->pixels + offset)) = (dcolor & 0xFF);
        *((byte *) (screen->pixels + offset + 1)) = (dcolor & 0xFF00) >> 8;
        *((byte *) (screen->pixels + offset + 2)) = (dcolor & 0xFF0000) >> 16;
#endif
        break;
      case 4:
        offset += (x << 2);
        *((long *)(screen->pixels + offset)) = c;
        break;
      }
    }
  }
}

/*
*	return's the value of the pixel
*/
long osd_getpixel(int x, int y)
{
  int offset, i;
  long color = 0;

  if (x >= 0 && x < screen->w) {
    if (y >= 0 && y < screen->h) {
      offset = y * screen->pitch;
      switch (bytespp) {
      case 1:
        offset += x;
        color = *((byte *) (screen->pixels + offset));
        break;
      case 2:
        offset += (x << 1);
        color = *((word *) (screen->pixels + offset));
        break;
      case 3:
        {
          byte *p;

          p = screen->pixels + offset + x * 3;
#if defined(CPU_BIGENDIAN)
          color = p[0] << 16 | p[1] << 8 | p[2];
#else
          color = p[0] | p[1] << 8 | p[2] << 16;
#endif
        }
//                      case    4:
      default:
        offset += (x << 2);
        color = *((dword *) (screen->pixels + offset));
        break;
      }

      for (i = 0; i < 16; i++) {
        if (cmap[i] == color)
          return i;
      }
    }
  }
  return color;
}

/*
*/
void direct_hline(int x, int x2, int y)
{
  long offset, i, len;

  if (x > x2) {
    i = x;
    x = x2;
    x2 = i;
  }

  if (x < 0)
    x = 0;
  if (x2 < 0)
    x2 = 0;
  if (x2 >= screen->w)
    x2 = screen->w - 1;
  if (y < 0)
    y = 0;
  if (y >= screen->h)
    y = screen->h - 1;

  len = (x2 - x) + 1;
  switch (os_color_depth) {
  case 8:
    offset = y * screen->w + x;
    memset(screen->pixels + offset, dcolor, len);
    return;
  case 15:
  case 16:
    offset = y * (screen->w << 1) + (x << 1);
    for (i = 0; i < len; i++, offset += 2)
      *((unsigned short int *)(screen->pixels + offset)) = dcolor;
    return;
  case 24:
    offset = y * (screen->w * 3) + (x * 3);
    for (i = 0; i < len; i++) {
      *((byte *) (screen->pixels + offset)) = (dcolor & 0xFF0000) >> 16;
      offset++;
      *((byte *) (screen->pixels + offset)) = (dcolor & 0xFF00) >> 8;
      offset++;
      *((byte *) (screen->pixels + offset)) = (dcolor & 0xFF);
      offset++;
    }
    return;
  case 32:
    offset = y * (screen->w << 2) + (x << 2);
    for (i = 0; i < len; i++, offset += 4)
      *((long *)(screen->pixels + offset)) = dcolor;
    return;
  }
}

//
//#define SDL_FASTPIX(x,y)      *(((byte *)screen->pixels)+(y)*screen->w+(x)) = dev_fgcolor
void g_setpixel(int x, int y)
{
  direct_setpixel(x, y, dcolor);
}

/* Bresenham's algorithm for drawing line */
void direct_line(int x1, int y1, int x2, int y2)
{
  if (y1 == y2) {
    direct_hline(x1, x2, y1);
    return;
  }
  g_line(x1, y1, x2, y2, g_setpixel);
}

void direct_fillrect(int x1, int y1, int x2, int y2, long c)
{
  int i;
  long co = dcolor;

  dcolor = c;
  for (i = y1; i <= y2; i++)
    direct_hline(x1, x2, i);
  dcolor = co;
}

void osd_settextcolor(long fg, long bg)
{
  osd_setcolor(fg);
  if (bg != -1)
    dev_bgcolor = bg;
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
        direct_setpixel(x + bit, y + i, fg_rgb);
      else if (overwrite)
        direct_setpixel(x + bit, y + i, bg_rgb);
    }
  }
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
*	clear screen
*/
void osd_cls()
{
  cur_x = cur_y = 0;

  LOCK();
  direct_fillrect(0, 0, screen->w - 1, screen->h - 1, cmap[15]);
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
*	next line
*/
void osd_nextln()
{
  cur_x = 0;

  if (cur_y < ((maxline - 1) * font_h)) {
    cur_y += font_h;
  }
  else {
    int len, to;

    // scroll
    len = cur_y * bytespp * os_graf_mx;
    to = font_h * bytespp * os_graf_mx;
    if (to + len >= os_graf_my * screen->pitch)
      len = ((os_graf_my * screen->pitch) - bytespp) - to;
    memcpy((char *)screen->pixels, (char *)screen->pixels + to, len);

    direct_fillrect(0, cur_y, os_graf_mx - 1, cur_y + font_h, cmap[15]);
  }
}

/*
*	calc next tab position
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
          direct_fillrect(cur_x, cur_y, os_graf_mx - cur_x, cur_y + font_h,
                          cmap[15]);
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
      direct_fillrect(cur_x, cur_y, os_graf_mx - cur_x, cur_y + font_h, cmap[15]);
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
        direct_line(cur_x, (cur_y + font_h) - 1, cur_x + cx, (cur_y + font_h) - 1);
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
*	check SDL's events
*/
int osd_events()
{
  int ch, button, i;
  int evc = 0;
  SDL_Event ev;

  if (scr_update) {
    // refresh
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    scr_update = 0;
  }

  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
    case SDL_KEYDOWN:
      ch = ev.key.keysym.sym;
      if (ch == SDLK_c && (ev.key.keysym.mod & KMOD_CTRL)) {  // break
        return -2;
      }
      else {
//                              dev_printf("--- K=0x%X ---", ch);
        // scan keymap
        for (i = 0; keymap[i] != 0; i += 2) {
          if (keymap[i] == ch) {  // !!!!!!!!!!!!!!!
            if (keymap[i + 1] != -1)
              dev_pushkey(keymap[i + 1]);
            ch = -1;
            break;
          }
        }

        // not found
        if (ch > 0 && ch < 255) {
          int upr;
          int shift;
          char *p;
          char *us_codes = "`1234567890-=[]\\;,./'";
          char *sf_codes = "~!@#$%^&*()_+{}|:<>?\"";

          shift = (ev.key.keysym.mod & (KMOD_SHIFT));

          upr = (((ev.key.keysym.mod & (KMOD_CAPS))) &&
                 ((ev.key.keysym.mod & (KMOD_SHIFT)) == 0))
            || (((ev.key.keysym.mod & (KMOD_CAPS)) == 0) &&
                ((ev.key.keysym.mod & KMOD_SHIFT)));

          if (((p = strchr(us_codes, ch)) != NULL) && shift)
            dev_pushkey(*(sf_codes + (p - us_codes)));
          else if (upr)
            dev_pushkey(toupper(ch));
          else
            dev_pushkey(ch);
        }
      }

      evc++;
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEMOTION:
      button = SDL_GetMouseState(&mouse_x, &mouse_y);

      if (mouse_x < mouse_hot_x)
        mouse_x = mouse_hot_x;
      if (mouse_y < mouse_hot_y)
        mouse_y = mouse_hot_y;
      if (mouse_x >= os_graf_mx - 1)
        mouse_x = os_graf_mx - 1;
      if (mouse_y >= os_graf_my - 1)
        mouse_y = os_graf_my - 1;
      mouse_b = 0;              // / bug
      if (button & SDL_BUTTON(1)) {
        if ((mouse_b & SDL_BUTTON(1)) == 0) { // new press
          mouse_down_x = mouse_x;
          mouse_down_y = mouse_y;
        }

        mouse_upd = 1;

        mouse_pc_x = mouse_x;
        mouse_pc_y = mouse_y;

        mouse_b |= 1;
      }
      if (button & SDL_BUTTON(2))
        mouse_b |= 2;
      if (button & SDL_BUTTON(3))
        mouse_b |= 4;

      evc++;
    }
  }

  return evc;
}

///////////////////////////////////////////////////////////////

void osd_setcolor(long color)
{
  dev_fgcolor = color;
  if (color >= 0 && color <= 15)
    dcolor = cmap[color];
  else if (color < 0) {
    dcolor = -color;
    dcolor = SDL_MapRGB(screen->format,
                        (dcolor & 0xFF0000) >> 16,
                        (dcolor & 0xFF00) >> 8, (dcolor & 0xFF));
  }
}

void osd_line(int x1, int y1, int x2, int y2)
{
  LOCK();

  if ((x1 == x2) && (y1 == y2))
    direct_setpixel(x1, y1, dcolor);
  else
    direct_line(x1, y1, x2, y2);

  UNLOCK();
}

void osd_setpixel(int x, int y)
{
  LOCK();

  direct_setpixel(x, y, dcolor);

  UNLOCK();
}

void osd_rect(int x1, int y1, int x2, int y2, int fill)
{
  int y;

  LOCK();

  if (fill) {
    for (y = y1; y <= y2; y++)
      direct_hline(x1, x2, y);
  }
  else {
    direct_line(x1, y1, x1, y2);
    direct_line(x1, y2, x2, y2);
    direct_line(x2, y2, x2, y1);
    direct_line(x2, y1, x1, y1);
  }

  UNLOCK();
//      SDL_UpdateRect(screen, 0, 0, 0, 0);
}

///////////////////////////////////////////////////////////////

void osd_sound(int freq, int ms, int vol, int bgplay)
{
  struct voice_info *info;
  int i, loops;

  if (has_audio) {

    if (!freq)
      freq = audiospec.freq / 2;

    loops = ((ms * audiospec.freq / freq / 2.0) / audiospec.samples) / 1000;

    SDL_LockAudio();

    info = &audio_info[audio_tail];
    info->vol = vol;
    info->setting = 0;
    info->period = 0;
    info->toggle = 1;

    if ((freq < audiospec.freq / 2) && (freq != 0)) {
      info->period = (float)(audiospec.freq / freq / 2.0);
      info->setting = (info->setting > 0) ? info->vol : -info->vol;
    }
    else
      info->period = 0.0;

    // 
    audio_tail++;
    if (audio_tail >= AUDIO_STACK_MAX)
      audio_tail = 0;

    for (i = 1; i < loops; i++) {
      audio_info[audio_tail] = *info;
      audio_info[audio_tail].toggle = 0;

      audio_tail++;
      if (audio_tail >= AUDIO_STACK_MAX)
        audio_tail = 0;
    }

    SDL_UnlockAudio();

    if (!bgplay)
      SDL_Delay(ms);
  }
}

void osd_beep()
{
  if (has_audio)
    osd_sound(440, 250, 75, 0);
  else
    printf("\a");
}

void osd_clear_sound_queue()
{
  if (has_audio) {
    SDL_LockAudio();
    audio_head = audio_tail;
    SDL_UnlockAudio();
  }
}

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
  if (scr_update) {
    // refresh
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    scr_update = 0;
  }
}

/// bitmap

/*
*	the size of the bitmap
*/
long osd_bmpsize(int x1, int y1, int x2, int y2)
{
  return bmp_size(x1, y1, x2, y2, bytespp * 8);
}

/*
*	stores video-rect to buf
*/
void osd_bmpget(int x1, int y1, int x2, int y2, char *buf)
{
  bmp_get((bmp_header_t *) buf, x1, y1, x2, y2, bytespp * 8, osd_getpixel);
}

/*
*	writes image to video
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
    bmp_put(newbmp, x, y, direct_setpixel);
    free(newbmp);
  }
  else
    bmp_put(bmp, x, y, direct_setpixel);
}
