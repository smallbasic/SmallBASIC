// This file is part of SmallBASIC
//
// SmallBASIC platform driver for Unix,
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2001-12-12, Nicholas Christopoulos

#include "common/device.h"
#include "common/keymap.h"
#include "common/str.h"
#include "common/dev_term.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>           // struct timeval
#include <unistd.h>
#if defined(_UnixOS)
#if !defined(__CYGWIN__)
#include <term.h>
#else
#include <ncurses/termcap.h>
#endif
#include <termios.h>
#include <sys/ioctl.h>
#elif defined(_DOS)
#include <conio.h>
#endif

static char pchar = '@';
static int rawmode = 0;         // file i/o style
static int cur_x = 0, cur_y = 0;        // cursor position
static int scr_w = 80, scr_h = 24;      // screen size
static int linux_term = 0, ansi_term = 0, xterm_term = 0;
static int tabsize = 8;

typedef struct {
  unsigned char ch;
  char fg, bg;
} vtchar_t;

static vtchar_t *video;

#if defined(_UnixOS)
static struct termios saved_stdin_attributes;
static struct termios saved_stdout_attributes;
static struct termios saved_stderr_attributes;
#endif

// terminal codes 
#if defined(_UnixOS)
static int termcap_is_alive;    // true = use termcap
static char termcap_buf[8192];  // used for termcap
#if defined(__CYGWIN__)
static char *tctemp_buf;
#endif
#endif

//#define       TAKE_SCREENSHOT

#if defined(TAKE_SCREENSHOT)
void write_vram(char *name);
#endif

/*
*	keyboard mapping table
*/
typedef struct {
  char scode[16];               // string code
  int icode;                    // integer code (SB's key code)
} vtkey_t;

static vtkey_t keytable[512];
static int keytable_count;

/*
*	termcap entries/keys
*/
typedef struct {
  char scode[8];
  int icode;
} vttermcapkey_t;

static vttermcapkey_t termcapkeys[] = {
  {"kb", SB_KEY_BACKSPACE},
  {"ta", SB_KEY_TAB},
  {"@8", SB_KEY_ENTER},

  {"kl", SB_KEY_LEFT},
  {"kr", SB_KEY_RIGHT},
  {"ku", SB_KEY_UP},
  {"kd", SB_KEY_DN},

  {"kh", SB_KEY_HOME},
  {"@7", SB_KEY_END},

  {"kI", SB_KEY_INSERT},
  {"kD", SB_KEY_DELETE},

  {"kP", SB_KEY_PGUP},
  {"kN", SB_KEY_PGDN},

  {"k1", SB_KEY_F(1)},
  {"k2", SB_KEY_F(2)},
  {"k3", SB_KEY_F(3)},
  {"k4", SB_KEY_F(4)},
  {"k5", SB_KEY_F(5)},
  {"k6", SB_KEY_F(6)},
  {"k7", SB_KEY_F(7)},
  {"k8", SB_KEY_F(8)},
  {"k9", SB_KEY_F(9)},
  {"k;", SB_KEY_F(10)},
  {"F1", SB_KEY_F(11)},
  {"F2", SB_KEY_F(12)},

  {"", 0}
};

/*
*	termcap entries/terminal code-strings
*/
typedef struct {
  char scode[8];
  int func;
  char str[64];
} vttermcapstr_t;

enum tfunc {
  tc_null,
  tc_hide_cursor,
  tc_show_cursor,
  tc_move_cursor,
  tc_move_tocol,
  tc_move_right,
  tc_move_right_one,
  tc_save,
  tc_restore,
  tc_clreos,
  tc_cpr_rq,
  tc_cpr,
  tc_eol
};

static vttermcapstr_t termcapstrs[] = {
  {"vs", tc_show_cursor, ""},
  {"vi", tc_hide_cursor, ""},
  {"cm", tc_move_cursor, ""},
  {"ch", tc_move_tocol, ""},
  {"RI", tc_move_right, ""},
  {"nd", tc_move_right_one, ""},
  {"sc", tc_save, ""},
  {"rc", tc_restore, ""},
  {"cd", tc_clreos, ""},
  {"u7", tc_cpr_rq, ""},
  {"u6", tc_cpr, ""},

  {"", tc_null, ""}
};

#if defined(_UnixOS)
#define		TERM_WAIT	(1000000 / 1920)
#endif

/* ----------------------------------------------------------------------------------- */

/*
*	save vt state
*/
void term_savemode() {
#if defined(_UnixOS)
  if (isatty(STDIN_FILENO))
    tcgetattr(STDIN_FILENO, &saved_stdin_attributes);
  if (isatty(STDOUT_FILENO))
    tcgetattr(STDOUT_FILENO, &saved_stdout_attributes);
  if (isatty(STDERR_FILENO))
    tcgetattr(STDERR_FILENO, &saved_stderr_attributes);
#endif
}

/*
*	restore vt state
*/
void term_restoremode() {
#if defined(_UnixOS)
  if (isatty(STDIN_FILENO))
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_stdin_attributes);
  if (isatty(STDOUT_FILENO))
    tcsetattr(STDOUT_FILENO, TCSANOW, &saved_stdout_attributes);
  if (isatty(STDERR_FILENO))
    tcsetattr(STDERR_FILENO, TCSANOW, &saved_stderr_attributes);
#endif
}

/*
*	setup terminal
*/
void term_setup_term() {
#if defined(_UnixOS)
  struct termios tattr;
  char *termtype;
  int success;

  termcap_is_alive = 0;

  /*
   * make sure stdin is a terminal. 
   */
  if (isatty(STDIN_FILENO)) {

    /*
     * Set the funny terminal modes. 
     */
    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO | ECHONL); // Clear ICANON and ECHO.
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);

    /*
     * termcap 
     */
    termcap_is_alive = 1;
    termtype = getenv("TERM");
    if (termtype == 0) {
      printf("\n\aSpecify a terminal type with `setenv TERM <yourtype>'.\n");
      termcap_is_alive = 0;
    } else {
      success = tgetent(termcap_buf, termtype);
      if (success <= 0)
        termcap_is_alive = 0;
      linux_term = (strncasecmp(termtype, "linux", 5) == 0);
      xterm_term = (strncasecmp(termtype, "xterm", 5) == 0) || (strncasecmp(termtype, "x1", 2) == 0);
      ansi_term = (strncasecmp(termtype, "ansi", 4) == 0) || linux_term;
    }

#if defined(TIOCGWINSZ)
    {
      struct winsize ws;

      /*
       * Get and check window size. 
       */
      if (ioctl(0, TIOCGWINSZ, &ws) >= 0 && ws.ws_row != 0 && ws.ws_col != 0) {
        scr_w = ws.ws_col;
        scr_h = ws.ws_row;
      }
    }
#else
    {
      scr_w = tgetnum("co");
      scr_h = tgetnum("li");
      if (scr_w < 8)
        scr_w = 8;
      if (scr_h < 1)
        scr_h = 1;
    }
#endif

  } else
    rawmode = 1;
#endif
}

/*
*	Add a keyboard code to keyboard table
*/
static void term_addkey(const char *scode, int icode) {
  vtkey_t *key;

  if (scode) {
    if (strlen(scode)) {
      key = &keytable[keytable_count];
      strcpy(key->scode, scode);
      key->icode = icode;
      keytable_count++;
    }
  }
}

/*
*	build keyboard table
*/
static void term_build_kbtable() {
#if defined(_UnixOS)
  int i;
  char *altkeys = " `1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./";
  char *altkeys_cap = " ~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?";

#if defined(__CYGWIN__)
  tctemp_buf = (char *)malloc(1024);
#endif

  if (termcap_is_alive) {
    for (i = 0; termcapkeys[i].scode[0] != '\0'; i++) {
      // tgetstr() with NULL parameters does not work on cygwin!
#if !defined(__CYGWIN__)
      if (tgetstr(termcapkeys[i].scode, NULL))
        term_addkey(tgetstr(termcapkeys[i].scode, NULL), termcapkeys[i].icode);
#else
      if (tgetstr(termcapkeys[i].scode, &tctemp_buf))
        term_addkey(tgetstr(termcapkeys[i].scode, &tctemp_buf), termcapkeys[i].icode);
#endif
    }
  }

  /*
   * defaults; if key is not in termcap that must works (linux) 
   */
  term_addkey("\033[A", SB_KEY_UP);
  term_addkey("\033[B", SB_KEY_DN);
  term_addkey("\033[D", SB_KEY_LEFT);
  term_addkey("\033[C", SB_KEY_RIGHT);
  term_addkey("\033[5~", SB_KEY_PGUP);
  term_addkey("\033[6~", SB_KEY_PGDN);
  term_addkey("\033[1~", SB_KEY_HOME);
  term_addkey("\033[4~", SB_KEY_END);
  term_addkey("\033[2~", SB_KEY_INSERT);
  term_addkey("\033[3~", SB_KEY_DELETE);
  term_addkey("\033[P", SB_KEY_BREAK);

  term_addkey("\033[[A", SB_KEY_F(1));
  term_addkey("\033[[B", SB_KEY_F(2));
  term_addkey("\033[[C", SB_KEY_F(3));
  term_addkey("\033[[D", SB_KEY_F(4));
  term_addkey("\033[[E", SB_KEY_F(5));

  term_addkey("\033[11~", SB_KEY_F(1));
  term_addkey("\033[12~", SB_KEY_F(2));
  term_addkey("\033[13~", SB_KEY_F(3));
  term_addkey("\033[14~", SB_KEY_F(4));
  term_addkey("\033[15~", SB_KEY_F(5));

  term_addkey("\033[17~", SB_KEY_F(6));
  term_addkey("\033[18~", SB_KEY_F(7));
  term_addkey("\033[19~", SB_KEY_F(8));
  term_addkey("\033[20~", SB_KEY_F(9));
  term_addkey("\033[21~", SB_KEY_F(10));
  term_addkey("\033[23~", SB_KEY_F(11));
  term_addkey("\033[24~", SB_KEY_F(12));

  for (i = 0; altkeys[i] != '\0'; i++) {
    char buf[8];

    sprintf(buf, "\033%c", altkeys[i]);
    term_addkey(buf, SB_KEY_ALT(altkeys[i]));
    sprintf(buf, "\033%c", altkeys_cap[i]);
    term_addkey(buf, SB_KEY_ALT(altkeys[i]));
  }
#endif
}

/*
*	build command index
*/
static void term_build_cmds() {
#if defined(_UnixOS)
  int i;

  if (termcap_is_alive) {
    for (i = 0; termcapstrs[i].func != tc_null; i++) {
#if !defined(__CYGWIN__)
      if (tgetstr(termcapstrs[i].scode, NULL))
        strcpy(termcapstrs[i].str, tgetstr(termcapstrs[i].scode, NULL));
#else
      if (tgetstr(termcapstrs[i].scode, &tctemp_buf))
        strcpy(termcapstrs[i].str, tgetstr(termcapstrs[i].scode, &tctemp_buf));
#endif
    }
  }
#endif
}

/*
*	debug - display termcap entries
*/
void term_disp_cmds() {
#if defined(_UnixOS)
  int i;

  if (termcap_is_alive) {
    for (i = 0; termcapstrs[i].func != tc_null; i++) {
      if (strlen(termcapstrs[i].str))
        printf("TC: %s %d\t\\e%s\n", termcapstrs[i].scode, i, termcapstrs[i].str + 1);
      else
        printf("TC: %s %d\t%s\n", termcapstrs[i].scode, i, "NIL");
    }
  }
#endif
}

/*
*	run a terminal command
*/
static void term_cmd(int fcode, ...) {
#if defined(_UnixOS)
  int i, r, c;
  va_list ap;

  if (!isatty(STDOUT_FILENO))
    return;

  va_start(ap, fcode);
  if (termcap_is_alive) {
    for (i = 0; termcapstrs[i].func != tc_null; i++) {
      if (termcapstrs[i].func == fcode) {
        if (fcode == tc_move_tocol) {
          c = va_arg(ap, int);
          printf("%s", tparm(termcapstrs[i].str, c));
        } else if (fcode == tc_move_right) {
          c = va_arg(ap, int);
          if (termcapstrs[i].str[0])
            printf("%s", tparm(termcapstrs[i].str, c));
          else {
            if (linux_term)
              printf("\033[%dC", c);
            else {
              int j;

              for (j = 0; j < c; j++)
                term_cmd(tc_move_right_one);
            }
          }
        } else if (fcode == tc_cpr_rq && linux_term) {
          if (termcapstrs[i].str[0] == 0)
            printf("\033[6n");
          else
            printf(termcapstrs[i].str);
        } else if (fcode == tc_move_cursor) {
          r = va_arg(ap, int);
          c = va_arg(ap, int);
          printf("%s", tgoto(termcapstrs[i].str, c, r));
        } else {
          vprintf(termcapstrs[i].str, ap);
          break;
        }
      }
    }
  }

  va_end(ap);
#endif
}

/*
*	Initialization
*/
int term_init() {
  keytable_count = 0;

  /*
   * initialize terminal 
   */
  term_savemode();
  term_setup_term();
  term_build_kbtable();
  term_build_cmds();
// term_disp_cmds();

  /*
   * get current x,y 
   */
  term_getx();                  // updated by CPR

  /*
   * update globals 
   */
  os_graf_mx = scr_w;
  os_graf_my = scr_h;

  /*
   * build VRAM 
   */
  video = malloc(sizeof(vtchar_t) * scr_w * scr_h);

  return 1;
}

/*
*	close
*/
int term_restore() {
#if defined(TAKE_SCREENSHOT)
  write_vram("term.txt");
#endif
  cur_x = cur_y = 0;
  term_restoremode();
  free(video);
  return 1;
}

/*
*/
int term_getc() {
#if defined(_UnixOS)
  fd_set rfds;
  struct timeval tv;
  int ival;
  byte c;

  if (!term_israw()) {
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    ival = select(1, &rfds, NULL, NULL, &tv);
    if (ival) {
      read(0, &c, 1);
      return c;
    }
  } else {
    if (feof(stdin))
      return -1;
    return fgetc(stdin);
  }
#elif defined(_DOS)
  int c;
  static int pc;

  if (pc) {
    pc = 0;
    return pc;
  }

  if (kbhit()) {
    c = getch();
    if (c == 0) {
      pc = getch();
      return 27;
    }
    return c;
  }
#endif
  return 0;
}

/*
*	events
*/
int term_events() {
  char buf[16];
  int pos = 0, i, evc = 0;
  int c;

#if defined(_UnixOS) || defined(_DOS)
  if (term_israw())
    return 0;
#endif

  while (1) {
    // get key
    c = term_getc();
    if (c == 0)
      return evc;

    if (c == -1)                // EOF !!!
      return -2;                // 

    // check key
    if (c == SB_KEY_BREAK)      // CTRL+C (break)
      return -2;

    if (c == 27) {              // extended key - check table
      c = term_getc();
      if (c) {
        buf[0] = c;
        pos = 1;
        do {
          c = term_getc();
          if (c == 0 || pos > 15)
            break;

          buf[pos] = c;
          pos++;
        } while (1);
        buf[pos] = '\0';

        if (buf[0] == '[' && buf[pos - 1] == 'R') {
          // CPR (TODO: get format from termcap 'u6')
          sscanf(buf, "[%d;%dR", &cur_y, &cur_x);
          cur_y--;
          cur_x--;
          return 1;             // leave it
        }

        for (i = 0; keytable[i].icode; i++) {
          if (strcmp(buf, keytable[i].scode + 1) == 0) {
            if (keytable[i].icode == SB_KEY_BREAK)      // CTRL+C (break)
              return -2;
            else
              dev_pushkey(keytable[i].icode);

            evc++;
            break;
          }
        }
      } else {
        dev_pushkey(c);
        evc++;
      }
    } else {
      dev_pushkey(c);
      evc++;
    }
  }

  return evc;
}

/*
*/
int term_getch() {
  return dev_getch();
}

/*
* 	return's true if there is working on raw-mode
*/
int term_israw() {
  return rawmode;
}

/*
*	draw string for gets
*
*	we must draw the text in current position
*	and put the cursor in 'pos' position
*
*	tm - 0 = before, 1 = while, 1 = after
*/
void term_getsdraw(char *dest, int pos, int tm) {
  int len;
#if defined(_DOS)
  static int px, py;
#endif

  len = strlen(dest);

  // disable cursor
#if defined(_UnixOS)
  term_cmd(tc_hide_cursor);
  fflush(stdout);
#elif defined(_DOS)
  _setcursortype(_NOCURSOR);    // ???
#endif

  // save or restore pos
  if (tm == 0) {
#if defined(_DOS)
    px = wherex();
    py = wherey();
#else
    term_cmd(tc_save);
    fflush(stdout);
#endif
  } else {
#if defined(_DOS)
    gotoxy(px, py);
#else
    term_cmd(tc_restore);
    fflush(stdout);
#endif
  }

  // print text
#if defined(_DOS)
  cprintf("%s ", dest);
#else
  printf("%s \b", dest);
#endif

  // set cursor pos
#if defined(_DOS)
  gotoxy(px + pos, py);
#else
  term_cmd(tc_restore);
  if (pos)
    term_cmd(tc_move_right, pos);
  fflush(stdout);
#endif

  // enable cursor
#if defined(_UnixOS)
  term_cmd(tc_show_cursor);
  fflush(stdout);
#elif defined(_DOS)
  _setcursortype(_SOLIDCURSOR); // ???
#endif
}

/*
*	print...
*/
void term_print(const char *str) {
#if defined(_DOS)
  int len, esc_val, esc_cmd;
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
    case '\xC':
      clrscr();
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
        } else {
          esc_cmd = *p;
        }

        // control characters
        switch (esc_cmd) {
        case 'K':              // \e[K - clear to eol
          clreol();
          break;
        case 'G':              // \e[nG - go to column
          gotoxy(esc_val, wherey());
          break;
        case 'm':              // \e[...m - ANSI terminal
          switch (esc_val) {
          case 0:              // reset
            term_settextcolor(7, 0);
            break;
          case 1:              // set bold on
            term_settextcolor(14, 0);
            break;
          case 4:              // set underline on
            break;
          case 7:              // reverse video on
            term_settextcolor(dev_bgcolor, dev_fgcolor);
            break;
          case 21:             // set bold off
            term_settextcolor(7, 0);
            break;
          case 24:             // set underline off
            break;
          case 27:             // reverse video off
            term_settextcolor(dev_fgcolor, dev_bgcolor);
            break;

            // colors - 30..37 foreground, 40..47 background
          case 30:             // set black fg
            term_settextcolor(0, -1);
            break;
          case 31:             // set red fg
            term_settextcolor(4, -1);
            break;
          case 32:             // set green fg
            term_settextcolor(2, -1);
            break;
          case 33:             // set brown fg
            term_settextcolor(6, -1);
            break;
          case 34:             // set blue fg
            term_settextcolor(1, -1);
            break;
          case 35:             // set magenta fg
            term_settextcolor(5, -1);
            break;
          case 36:             // set cyan fg
            term_settextcolor(3, -1);
            break;
          case 37:             // set white fg
            term_settextcolor(7, -1);
            break;

          case 40:             // set black bg
            term_settextcolor(dev_fgcolor, 0);
            break;
          case 41:             // set red bg
            term_settextcolor(dev_fgcolor, 4);
            break;
          case 42:             // set green bg
            term_settextcolor(dev_fgcolor, 2);
            break;
          case 43:             // set brown bg
            term_settextcolor(dev_fgcolor, 6);
            break;
          case 44:             // set blue bg
            term_settextcolor(dev_fgcolor, 1);
            break;
          case 45:             // set magenta bg
            term_settextcolor(dev_fgcolor, 5);
            break;
          case 46:             // set cyan bg
            term_settextcolor(dev_fgcolor, 3);
            break;
          case 47:             // set white bg
            term_settextcolor(dev_fgcolor, 7);
            break;

          };
          break;
        }
      }
      break;
    default:
      // 
      // PRINT THE CHARACTER
      // 
      if (*p == '\t') {
        int x = wherex();
        int ts = tabsize, len;

        len = (ts - (x % ts));
        gotoxy(x + len, wherey());
      } else if ((*p > 31) || (*p & 0x80))      // non-control code
        cprintf("%c", *p);
      else
        printf("%c", *p);
    };

    if (*p == '\0')
      break;

    p++;
  }
#else
  printf("%s", str);
  fflush(stdout);
#endif
}

/*
*	returns the x position of cursor (in pixels)
*
*	CPR req.
*/
int term_getx() {
#if defined(_DOS)
  return wherex() - 1;
#else
  if (!term_israw()) {
    int count = 100;            // 100 ms

    term_cmd(tc_cpr_rq);
    fflush(stdout);
    while ((term_events() == 0) && count) {
      dev_delay(1);             // 1 ms
      count--;
    }
  }
  return cur_x;
#endif
}

/*
*	returns the y position of cursor (in pixels)
*
*	CPR req.
*/
int term_gety() {
#if defined(_DOS)
  return wherey() - 1;
#else
  if (!term_israw()) {
    int count = 100;            // 100ms

    term_cmd(tc_cpr_rq);
    fflush(stdout);
    while ((term_events() == 0) && count) {
      dev_delay(1);             // 1ms
      count--;
    }
  }
  return cur_y;
#endif
}

/*
*	set cursor position
*/
void term_setxy(int x, int y) {
#if defined(_DOS)
  gotoxy(x + 1, y + 1);
#else
  if (!term_israw()) {
    term_cmd(tc_move_cursor, y, x);
    fflush(stdout);
  }
#endif
}

/*
*	set text colors
*/
void term_settextcolor(int fg, int bg) {
#if defined(_UnixOS)
  int c = 0, hi = 0;

  if (!isatty(STDOUT_FILENO))
    return;

  // 
  if (fg > 15)
    fg = 15;
  if (fg < 0)
    fg = dev_fgcolor;
  else
    dev_fgcolor = fg;

  if (bg > 15)
    bg = 15;
  if (bg < 0)
    bg = dev_bgcolor;
  else
    dev_bgcolor = bg;

  // 
  if (fg > 7)
    (hi = 1, fg -= 8);
  switch (fg) {
  case 0:
    c = 30;
    break;
  case 1:
    c = 34;
    break;
  case 2:
    c = 32;
    break;
  case 3:
    c = 36;
    break;
  case 4:
    c = 31;
    break;
  case 5:
    c = 35;
    break;
  case 6:
    c = 33;
    break;
  case 7:
    c = 37;
    break;
  };

  if (hi)
    printf("\033[1m");
  else
    printf("\033[21m");

  printf("\033[%dm", c);

  hi = 0;
  c = 0;
  if (bg > 7)
    (hi = 1, bg -= 8);
  switch (bg) {
  case 0:
    c = 40;
    break;
  case 1:
    c = 44;
    break;
  case 2:
    c = 42;
    break;
  case 3:
    c = 46;
    break;
  case 4:
    c = 41;
    break;
  case 5:
    c = 45;
    break;
  case 6:
    c = 43;
    break;
  case 7:
    c = 47;
    break;
  };

  if (hi)
    printf("\033[5m");
  else
    printf("\033[25m");

  printf("\033[%dm", c);
  fflush(stdout);

#elif defined(_DOS)
  textcolor(fg);
  if (bg != -1)
    textbackground(bg);
#endif
}

/*
*	Clear screen
*/
void term_cls() {
#if defined(_DOS)
  clrscr();
#else
#if defined(_UnixOS)
  if (!isatty(STDOUT_FILENO))
    return;
#endif
  term_cmd(tc_move_cursor, 0, 0);
  term_cmd(tc_clreos);
  fflush(stdout);
#endif
}

/*
*	update video structure
*/
void term_update_color(int x, int y, int ch, int fg, int bg) {
  int ofs;

  ofs = scr_w * y + x;
  video[ofs].ch = ch;
  video[ofs].fg = fg;
  video[ofs].bg = bg;
}

/*
*	setpixel emulation
*/
void term_drawpoint(int x, int y) {
#if defined(_DOS)
  int px, py;
#endif

#if defined(_UnixOS)
  term_cmd(tc_save);
  term_cmd(tc_move_cursor, y, x);
  printf("%c", pchar);
  term_cmd(tc_restore);
  fflush(stdout);
#elif defined(_DOS)
  px = wherex();
  py = wherey();
  gotoxy(x + 1, y + 1);
  cprintf("%c", pchar);
  gotoxy(px, py);
#endif
  term_update_color(x, y, pchar, dev_fgcolor, dev_bgcolor);
}

/*
*	returns the color of x,y character
*/
int term_getpoint(int x, int y) {
  int ofs;

  ofs = scr_w * y + x;
  return video[ofs].fg;
}

/*
*	The classic line algorithm
*/
void term_drawline(int x1, int y1, int x2, int y2) {
  g_line(x1, y1, x2, y2, term_drawpoint);
}

/*
*/
void term_drawrect(int x1, int y1, int x2, int y2, int fill) {
  int y, x;

  if (!fill) {
    for (x = x1 + 1; x < x2; x++) {
      term_setxy(x, y1);
      term_print("-");
      term_update_color(x, y1, '-', dev_fgcolor, dev_bgcolor);

      term_setxy(x, y2);
      term_print("-");
      term_update_color(x, y2, '-', dev_fgcolor, dev_bgcolor);
    }

    for (y = y1 + 1; y < y2; y++) {
      term_setxy(x1, y);
      term_print("|");
      term_update_color(x1, y, '|', dev_fgcolor, dev_bgcolor);

      term_setxy(x2, y);
      term_print("|");
      term_update_color(x2, y, '|', dev_fgcolor, dev_bgcolor);
    }

    term_setxy(x1, y1);
    term_print("+");
    term_update_color(x1, y1, '+', dev_fgcolor, dev_bgcolor);
    term_setxy(x1, y2);
    term_print("+");
    term_update_color(x1, y2, '+', dev_fgcolor, dev_bgcolor);
    term_setxy(x2, y1);
    term_print("+");
    term_update_color(x2, y1, '+', dev_fgcolor, dev_bgcolor);
    term_setxy(x2, y2);
    term_print("+");
    term_update_color(x2, y2, '+', dev_fgcolor, dev_bgcolor);
  } else {
    term_settextcolor(dev_bgcolor, dev_fgcolor);
    for (y = y1; y <= y2; y++) {
      for (x = x1; x <= x2; x++) {
        term_setxy(x, y);
        term_update_color(x, y, ' ', dev_bgcolor, dev_fgcolor);
        term_print(" ");
      }
    }
    term_settextcolor(dev_bgcolor, dev_fgcolor);
  }

  fflush(stdout);
}

/**
*/
void term_settab(int tabsz) {
  tabsize = tabsz;
}

/**
*/
int term_cols() {
  return scr_w;
}

/**
*/
int term_rows() {
  return scr_h;
}

/*
*	style = 0 - off
*	style = 1 - on
*	style = 2 - half (not yet)
*/
void term_setcursor(int style) {
  switch (style) {
  case 0:
    // disable cursor
#if defined(_UnixOS)
    term_cmd(tc_hide_cursor);
#elif defined(_DOS)
    _setcursortype(_NOCURSOR);  // ???
#endif
    break;
  default:
    // enable cursor
#if defined(_UnixOS)
    term_cmd(tc_show_cursor);
#elif defined(_DOS)
    _setcursortype(_SOLIDCURSOR);       // ???
#endif
    break;
  }
}

void term_recalc_size() {
#if	defined(_UnixOS)
#if defined(TIOCGWINSZ)
  {
    struct winsize ws;

    /*
     * Get and check window size. 
     */
    if (ioctl(0, TIOCGWINSZ, &ws) >= 0 && ws.ws_row != 0 && ws.ws_col != 0) {
      scr_w = ws.ws_col;
      scr_h = ws.ws_row;
    }
  }
#else
  {
    scr_w = tgetnum("co");
    scr_h = tgetnum("li");
    if (scr_w < 8)
      scr_w = 8;
    if (scr_h < 1)
      scr_h = 1;
  }
#endif
#elif defined(_DOS)
// scr_w = peekb(0, 0x449);
// scr_h = peekb(0, 0x449);
#endif
}
