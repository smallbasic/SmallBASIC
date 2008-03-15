/*
*	low-level platform driver for OFBIS library
*
*	ndc: 2001-02-13
*/

#include "device.h"
#include "osd.h"
#include "str.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#define	FONT_HEIGHT	16

static int curcol = 0;
static int curline = 0;
static int tabsize = 32;        // from dev_palm
static int maxline;
static int text_fgcolor = 0;
static int text_bgcolor = 15;

static int vga16[] = {
  0x0, 0x80, 0x8000, 0x8080, 0x800000, 0x800080, 0x808000, 0x808080,
  0x555555, 0xFF, 0xFF00, 0xFFFF, 0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF
};

static FB *fdev = NULL;

int osd_devinit()
{
//      fdev = FBopen(NULL, FB_OPEN_NEW_VC);
  fdev = FBopen(NULL, FB_KEEP_CURRENT_VC);
  os_graf_mx = 1024;
  os_graf_my = 768;
  maxline = os_graf_my / FONT_HEIGHT;
  setsysvar_str(SYSVAR_OSNAME, "Unix/OFBIS");
  return (fdev != NULL);
}

int osd_devrestore()
{
  curcol = 0;
  curline = maxline - 1;
  osd_write("Press any key to exit...");
  osd_events(1);

  FBclose(fdev);
  return 0;
}

//
void osd_cls()
{
  int y;

  curcol = 0;
  curline = 0;
  for (y = 0; y < os_graf_my; y++)
    FBhline(fdev, 0, os_graf_mx, y, 15);
}

//      returns the current x position
int osd_getx()
{
  return curcol;
}

//      returns the current y position
int osd_gety()
{
  return curline * 16;
}

//
void osd_setxy(int x, int y)
{
  curcol = x / 8;
  curline = y / FONT_HEIGHT;
}

void osd_setpenmode(int enable)
{
}

int osd_getpen(int mode)
{
  return 0;
}

/**
*	next line
*/
void osd_nextln()
{
  curcol = 0;

  if (curline < maxline) {
    curline++;
  }
  else {
    // scroll
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
  int len, cx, escval, esccmd;
  byte *p;

  len = strlen(str);

  if (len <= 0)
    return;

  p = (byte *) str;
  while (*p) {
    switch (*p) {
    case '\a':                 // beep
      printf("\a");
      break;
    case '\t':
      curcol = osd_calctab(curcol);
      break;
    case '\xC':
      dev_cls();
      break;
    case '\033':               // ESC ctrl chars (man console_codes)
      if (*(p + 1) == '[') {
        p += 2;

        escval = 0;
        esccmd = 0;

        if (is_digit(*p)) {
          escval = (*p - '0');
          p++;

          if (is_digit(*p)) {
            escval = (escval * 10) + (*p - '0');
            p++;
          }

          esccmd = *p;
        }
        else
          esccmd = *p;

        switch (esccmd) {
        case 'K':              // cleeol
          break;
        case 'm':
          switch (escval) {
          case 0:              // reset
            break;
          case 1:              // set bold on
            break;
          case 4:              // set underline on
            break;
          case 7:              // reverse video on
            break;
          case 21:             // set bold off
            break;
          case 24:             // set underline off
            break;
          case 27:             // reverse video off
            break;
          };
          break;
        }
      }
      break;
    case '\n':
      osd_nextln();
      break;
    case '\r':
      // clreol
      curcol = 0;
      break;
    default:
      cx = 8;                   // char width
      if (cx + curcol >= os_graf_mx)
        osd_nextln();

      FBputchar(fdev, curcol, curline * FONT_HEIGHT, vga16[text_fgcolor],
                vga16[text_bgcolor], *p);
      curcol += cx;
    };

    if (*p == '\0')
      break;

    p++;
  }
}

void osd_settextcolor(long fg, long bg)
{
  text_fgcolor = fg;
  text_bgcolor = bg;
}

void osd_setcolor(long fg)
{
  text_fgcolor = fg;
}

int osd_textwidth(const char *s)
{
  return 8 * strlen(s);
}

int osd_textheight(const char *s)
{
  return 16;
}
/*
*	gets() && fgets(stdin);
*/
char *osd_gets(char *dest, int size)
{
  word ch, pos;
  word x, y;

  x = curcol;
  y = curline;
  dev_clreol();

  *dest = '\0';
  pos = 0;
  do {
    ch = dev_getch();
    switch (ch) {
    case 0:
    case 10:
    case 13:
      break;
    case 8:                    // backspace
      if (pos)
        pos--;
      dest[pos] = '\0';
      break;
    default:
      dest[pos] = ch;
      pos++;
      dest[pos] = '\0';
    }

    if (pos >= (size - 1))
      break;

    if (ch) {
      curcol = x;
      curline = y;
      dev_clreol();
      dev_print(dest);
    }

  } while ((ch & 0xff) != '\n');
  dest[pos] = '\0';

  curcol = x;
  curline = y;
  dev_clreol();
  dev_print(dest);
  dev_print("\n");
  return dest;
}

int stdunix_events(int wait_flag)
{
  fd_set rfds;
  struct timeval tv;
  int ival;
  byte c;

  FD_ZERO(&rfds);
  FD_SET(0, &rfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  ival = select(1, &rfds, NULL, NULL, &tv);
  if (ival || wait_flag) {
    read(0, &c, 1);
    if (c == SB_KEY_BREAK)      // CTRL+C (break)
      return -2;
    dev_pushkey(c);
    return 1;
  }
  return 0;
}

/*
*/
int ufb_events(int wait_flag)
{
  FBEVENT ev;

  if (wait_flag == 0) {
    FBcheckevent(fdev, &ev, 0);
    switch (ev.type) {
    case FBKeyEvent:
      FBgetevent(fdev, &ev);
      if ((ev.key.ascii >= 32) || (ev.key.ascii & 0x80) || (ev.key.ascii == 9) ||
          (ev.key.ascii == 13) || (ev.key.ascii == 10) || (ev.key.ascii == 8))
        dev_pushkey(ev.key.ascii);
      else if (ev.key.ascii == 3)
        return -1;
      break;
    case FBMouseEvent:         // for the PEN
      break;
    };
    return (ev.type != FBNoEvent);
  }
  else {
    while (ufb_events(0) == 0)
      usleep(50 * 1000);
  }
  return 1;
}

int osd_events(int wait_flag)
{
  if (os_graphics)
    return ufb_events(wait_flag);
  return stdunix_events(wait_flag);
}

void osd_line(int x1, int y1, int x2, int y2)
{
  FBline(fdev, x1, y1, x2, y2, dev_fgcolor);
}

void osd_setpixel(int x, int y)
{
  FBputpixel(fdev, x, y, dev_fgcolor);
}

long osd_getpixel(int x, int y)
{
  return FBgetpixel(fdev, x, y);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill)
{
  int y;

  if (fill) {
    for (y = y1; y <= y2; y++)
      FBline(fdev, x1, y, x2, y, dev_fgcolor);
  }
  else {
    FBline(fdev, x1, y1, x1, y2, dev_fgcolor);
    FBline(fdev, x1, y2, x2, y2, dev_fgcolor);
    FBline(fdev, x2, y2, x2, y1, dev_fgcolor);
    FBline(fdev, x2, y1, x1, y1, dev_fgcolor);
  }
}

void osd_refresh()
{
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
