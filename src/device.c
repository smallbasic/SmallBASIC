// $Id$
// This file is part of SmallBASIC
//
// lowlevel device (OS) I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sys.h"
#include "str.h"
#include "var.h"
#include "device.h"
#include "osd.h"
#include "smbas.h"
#include "sberr.h"
#include "messages.h"

// add-on drivers
#if defined(DRV_SOUND)
#include "drvsound.h"
static int drvsound_ok;
#endif

#if defined(DRV_MOUSE)
#include "drvmouse.h"
static int drvmouse_ok;
#endif

#if USE_TERM_IO
#include "dev_term.h"
#endif

#ifdef __MINGW32__
#include <windows.h>
#define usleep(s) Sleep((DWORD)((s+500)/1000))
#endif

void dev_drawcursor(int x, int y) SEC(BIO);
int dev_input_char2str(int ch, byte * cstr) SEC(BIO);
int dev_input_count_char(byte * buf, int pos) SEC(BIO);
int dev_input_insert_char(int ch, byte * dest, int pos, int replace_mode) SEC(BIO);
int dev_input_remove_char(byte * dest, int pos) SEC(BIO);
void dev_input_clreol(int cx, int cy) SEC(BIO);

/**
 * break signal
 */
#if defined(_UnixOS) || defined(_DOS)
void termination_handler(int signum) {
  static int ctrlc_count;

  prog_error = -2;
  ctrlc_count++;
  if (ctrlc_count == 1) {
    dev_printf("\n\n\033[0m\033[7m\a * %s %d * \033[0m\n", WORD_BREAK_AT, prog_line);
  }
  else if (ctrlc_count == 3) {
    dev_restore();
    memmgr_setabort(1);
    exit(1);
  }
}
#endif

#if IMPL_EMPTY_TERM_EVENTS
int term_events() {return 0;}
#endif

/**
 * CHECK FOR EVENTS
 *
 * wait == 0 check & return
 *    != 0 wait for an event
 *
 * returns 0 for no events in queue
 */
int dev_events(int wait_flag) {
#if !defined(_PalmOS) && !defined(DEV_EVENTS_OSD)
  if (os_graphics) {
    osd_refresh();
  }
#endif

#if USE_TERM_IO
  //
  // standard input case
  //
  if (!os_graphics) {
    if (term_israw()) {
      return !feof(stdin);
    }
  }
#endif

#if DEV_EVENTS_OSD
  return osd_events(wait_flag);
#else

  if (wait_flag) {
    int evc;

#if defined(_VTOS)
    {
      extern int osd_events_sleep();
      evc = osd_events_sleep();
    }
#else
    while ((evc = dev_events(0)) == 0)
#if defined(_UnixOS)
      usleep(31250);
#elif defined(_Win32)
    ;
#elif defined(_PalmOS)
    ;                         //
#else
    dev_delay(31);            // sleep for 1000/32 and try again
#endif
#endif
    return evc;
  }
  else {
#if defined(DRV_SOUND)
    drvsound_event();
#endif

#if defined(_PalmOS) || defined(_WinBCB) || defined(_VTOS)
    return osd_events(0);
#else
    if (os_graphics) {
      return osd_events(0);
    }
    return term_events();
#endif
  }

#endif // _FRANKLIN_EBM
}

/**
 * delay for a specified amount of milliseconds
 */
#ifndef IMPL_DEV_DELAY
void dev_delay(dword ms) {
#if defined(_PalmOS)
  dword start, tps, now;
  int evc;

  start = TimGetTicks();
  tps = SysTicksPerSecond();
  while (1) {
    SysTaskDelay(1);

    switch ((evc = dev_events(0))) {
    case 0:                    // no event
      break;
    case -2:                   // break
      brun_break();
    case -1:                   // break
      return;
    }

    now = TimGetTicks();

    if (now > (start + (tps * ms) / 1000L))
      return;
  }
#elif defined(_Win32)
  Sleep(ms);
#elif defined(_DOS)
  delay(ms);
#else // Unix
  usleep(ms * 1000);
#endif
}
#endif

/**
 * draw the cursor
 */
void dev_drawcursor(int x, int y) {
  if (os_graphics) {
    osd_setpixel(x, y);
    osd_setpixel(x, y + 1);
    osd_setpixel(x, y + 2);
  }
}

/**
 * return the character (multibyte charsets support)
 */
int dev_input_char2str(int ch, byte * cstr) {
  memset(cstr, 0, 3);

  switch (os_charset) {
  case enc_sjis:               // Japan
    if (IsJISFont(ch)) {
      cstr[0] = ch >> 8;
    }
    cstr[1] = ch & 0xFF;
    break;
  case enc_big5:               // China/Taiwan (there is another charset for
    // China but I don't know the difference)
    if (IsBig5Font(ch)) {
      cstr[0] = ch >> 8;
    }
    cstr[1] = ch & 0xFF;
    break;
  case enc_gmb:                // Generic multibyte
    if (IsGMBFont(ch)) {
      cstr[0] = ch >> 8;
    }
    cstr[1] = ch & 0xFF;
    break;
  case enc_unicode:            // Unicode
    cstr[0] = ch >> 8;
    cstr[1] = ch & 0xFF;
    break;
  default:
    cstr[0] = ch;
  };
  return strlen((char *)cstr);
}

/**
 * return the character size at pos! (multibyte charsets support)
 */
int dev_input_count_char(byte * buf, int pos) {
  int count, ch;
  byte cstr[3];

  if (os_charset != enc_utf8) {
    ch = buf[0];
    ch = ch << 8;
    ch = ch + buf[1];
    count = dev_input_char2str(ch, cstr);
  }
  else {
    count = 1;
  }
  return count;
}

/**
 * stores a character at 'pos' position
 */
int dev_input_insert_char(int ch, byte * dest, int pos, int replace_mode) {
  byte cstr[3];
  int count, remain;
  byte *buf;

  count = dev_input_char2str(ch, cstr);

  // store character into buffer
  if (replace_mode) {
    // overwrite mode
    remain = strlen((char *)(dest + pos));
    buf = tmp_alloc(remain + 1);
    strcpy(buf, dest + pos);
    memcpy(dest + pos, cstr, count);
    dest[pos + count] = '\0';

    if (os_charset != enc_utf8) {
      count = dev_input_char2str(((buf[0] << 8) | buf[1]), cstr);
    }
    else {
      count = 1;
    }
    if (buf[0]) {                // not a '\0'
      strcat((char *)dest, (char *)(buf + count));
    }
    tmp_free(buf);
  }
  else {
    // insert mode
    remain = strlen((char *)(dest + pos));
    buf = tmp_alloc(remain + 1);
    strcpy(buf, dest + pos);
    memcpy(dest + pos, cstr, count);
    dest[pos + count] = '\0';
    strcat((char *)dest, (char *)buf);
    tmp_free(buf);
  }

  return count;
}

/**
 * removes the character at 'pos' position
 */
int dev_input_remove_char(byte * dest, int pos) {
  byte cstr[3];
  int count, remain;
  byte *buf;

  if (dest[pos]) {
    if (os_charset != enc_utf8) {
      count = dev_input_char2str(((dest[pos] << 8) | dest[pos + 1]), cstr);
    }
    else {
      count = 1;
    }
    remain = strlen((char *)(dest + pos + 1));
    buf = tmp_alloc(remain + 1);
    strcpy(buf, dest + pos + count);

    dest[pos] = '\0';
    strcat((char *)dest, (char *)buf);
    tmp_free(buf);
    return count;
  }
  return 0;
}

/**
 * clears right of the cursor
 */
void dev_input_clreol(int cx, int cy) {
  int x, y;
  int color = dev_fgcolor;

  x = dev_getx();
  y = dev_gety();
  if (x + cx + 1 >= os_graf_mx) {
    dev_clreol();
  }
  else {
    if (os_graphics) {
      osd_setcolor(dev_bgcolor);
      osd_rect(x, y, x + cx + 1, y + cy, 1);
      osd_setcolor(color);
    }
    else {
      dev_clreol();
    }
  }
}

/**
 * gets a string (INPUT)
 */
#ifndef IMPL_DEV_GETS
char *dev_gets(char *dest, int size) {
  long int ch = 0;
  word pos, len = 0;
  int prev_x = 1, prev_y = 1;
  int tmp_lines, lines = 0, disp_x, disp_y;
  int lpp = 24, cy = 1, w, replace_mode = 0;
  char prev_ch;
  int code;
  int cx = 1;

#if USE_TERM_IO
  if (!os_graphics) {
    if (term_israw()) {
      // standard input
      char *p;
      int c;

      if (feof(stdin)) {
        strcpy(dest, "");
        return dest;
      }

      p = dest;
      while ((c = fgetc(stdin)) != -1) {
        if (c == '\r') {
          continue;             // ignore
        }
        if (c == '\n') {
          break;
        }
        if (size <= ((int)(p - dest)) + 1) {
          break;
        }
        *p++ = c;
      }

      *p = '\0';
      return dest;
    }
  }
#endif

  /*
   *      the 'input'
   *
   *      warning: getx/y routines are required
   */
  *dest = '\0';

  if (os_graphics) {
    prev_x = dev_getx();
    prev_y = dev_gety();
    cx = dev_textwidth("W");
    cy = dev_textheight("Q");
    lpp = os_graf_my / cy;
  }

  dev_clrkb();

  pos = 0;

#if USE_TERM_IO
  if (!os_graphics) {
    term_getsdraw(dest, 0, 0);
  }
#endif

  do {
    len = strlen(dest);

    // draw
#if USE_TERM_IO
    if (!os_graphics) {
      term_getsdraw(dest, pos, 1);
    }
    else {
#endif
      dev_setxy(prev_x, prev_y);
      dev_print(dest);
      dev_input_clreol(cx, cy);

      //
      tmp_lines = (prev_x + dev_textwidth(dest)) / os_graf_mx;
      if (tmp_lines > lines) {
        lines = tmp_lines;
        while ((lines * cy) + prev_y >= (lpp * cy)) {
          prev_y -= cy;
        }
      }

      //
      prev_ch = dest[pos];
      dest[pos] = '\0';
      w = dev_textwidth(dest);
      dest[pos] = prev_ch;

      tmp_lines = (prev_x + w) / os_graf_mx;

      disp_y = prev_y + tmp_lines * cy;
      disp_x = (prev_x + w) - (tmp_lines * os_graf_mx) + (tmp_lines * dev_textwidth(" "));  // TODO:
                                                                                            //
      // +
      // width
      // of
      // chars
      // at
      // the
      // end
      // of
      // prev
      // lines

      dev_setxy(disp_x, disp_y);
      dev_drawcursor(disp_x, disp_y);
#if USE_TERM_IO
    }
#endif

    // wait for event
    code = dev_events(1);

    // remove cursor
    // ...

    if (code < 0) {             // BREAK event
      *dest = '\0';
      brun_break();
      return dest;
    }

    while (dev_kbhit()) {       // we have keys
      ch = dev_getch();

      switch (ch) {
      case -1:
      case -2:
      case 0xFFFF:
        dest[pos] = '\0';
        return dest;
      case 0:
      case 10:
      case 13:                 // ignore
        break;
      case SB_KEY_HOME:
        pos = 0;
        break;
      case SB_KEY_END:
        pos = len;
        break;
      case SB_KEY_BACKSPACE:   // backspace
        if (pos > 0) {
          pos -= dev_input_remove_char((byte *) dest, pos - 1);
          len = strlen(dest);
        }
        else
          dev_beep();
        break;
#if defined(_PalmOS)
      case SB_KEY_PALM_BTN1:
#endif
      case SB_KEY_DELETE:      // delete
        if (pos < len) {
          dev_input_remove_char((byte *) dest, pos);
          len = strlen(dest);
        }
        else
          dev_beep();
        break;
      case SB_KEY_INSERT:
        replace_mode = !replace_mode;
        break;
      case SB_KEY_LEFT:
        if (pos > 0) {
          pos -= dev_input_count_char((byte *) dest, pos);
        }
        else {
          dev_beep();
        }
        break;
      case SB_KEY_RIGHT:
        if (pos < len) {
          pos += dev_input_count_char((byte *) dest, pos);
        }
        else {
          dev_beep();
        }
        break;
      default:
        if ((ch & 0xFF00) != 0xFF00) { // Not an hardware key
          pos += dev_input_insert_char(ch, (byte *) dest, pos, replace_mode);
        }
        else {
          ch = 0;
        }
        // check the size
        len = strlen(dest);
        if (len >= (size - 2)) {
          break;
        }
      }
    }                           // dev_kbhit() loop

  } while (ch != '\n' && ch != '\r');
  dest[len] = '\0';

#if USE_TERM_IO
  if (!os_graphics) {
    term_getsdraw(dest, strlen(dest), 2);
  }
  else {
#endif
    dev_setxy(prev_x, prev_y);
    dev_print(dest);
    dev_input_clreol(cx, cy);
#if USE_TERM_IO
  }
#endif
  return dest;
}
#endif // #ifndef IMPL_DEV_GETS

/**
 * returns data from pointing-device
 * (see PEN(x), osd_getpen(x))
 */
int dev_getpen(int code) {
  if (os_graphics) {
    return osd_getpen(code);
  }
  return 0;
}

/**
 * BEEP :)
 */
void dev_beep() {
  if (os_graphics) {
    osd_refresh();
  }

#if defined(DRV_BEEP)
  osd_beep();
#else
#if defined(DRV_SOUND)
  drvsound_beep();
#else
  if (os_graphics) {
    osd_beep();
  }
  else {
    printf("\a");
  }
#endif
#endif
}

/**
 * plays a sound
 */
void dev_sound(int frq, int ms, int vol, int bgplay) {
#if IMPL_OSD_SOUND
  osd_sound(frq, ms, vol, bgplay);
#else

#if defined(DRV_SOUND)
  drvsound_sound(frq, ms, vol, bgplay);
#else
  if (os_graphics) {
    osd_sound(frq, ms, vol, bgplay);
  }
  else {
    // Linux only ???
#if defined(USE_LINCONCODES)
    /*
     *      Linux console codes - PC Speaker
     */
    printf("\033[10;%d]", frq);
    printf("\033[11;%d]", ms);
    fflush(stdout);
    printf("\a");
    fflush(stdout);
    dev_delay(ms);
    printf("\033[10;%d]", 440);
    printf("\033[11;%d]", 250);
    fflush(stdout);
#else
    if (!bgplay) {
      dev_delay(ms);
    }
#endif
  }
#endif

#endif
}

/**
 * clear background sound queue
 */
void dev_clear_sound_queue() {
#if defined(DRV_SOUND)
  drvsound_clear_queue();
#else
  if (os_graphics) {
    osd_clear_sound_queue();
  }
#endif
}

/**
 * printf
 *
 * WARNING: PalmOS ver is limited to 256 bytes
 * WARNING: Win32/Unix ver is limited to 1024 bytes
 */
void dev_printf(const char *fmt, ...) {
  char *buf;
  va_list ap;

  va_start(ap, fmt);

#if defined(_PalmOS)
  buf = tmp_alloc(256);
  StrVPrintF(buf, fmt, ap);
  va_end(ap);

#else
  buf = tmp_alloc(1024);
#if defined(_DOS) || defined(_Win32) || defined(_VTOS) || defined(_FRANKLIN_EBM)
  vsprintf(buf, fmt, ap);
#else
  vsnprintf(buf, 1024, fmt, ap);
#endif
  va_end(ap);

#endif
  dev_print(buf);
  tmp_free(buf);
}

/**
 * In the FLTK build, prints to the LOG window, in other builds
 * prints to the output device as per dev_printf
 */
void log_printf(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof buf - 1, format, args);
  va_end(args);

  while (p > buf && isspace(p[-1])) {
    *--p = '\0';
  }

  *p++ = '\r';
  *p++ = '\n';
  *p = '\0';

#if defined(IMPL_LOG_WRITE)
  lwrite(buf);
#else
  dev_print(buf);
#endif
}

#ifndef IMPL_HTML
void dev_html(const char *html, const char *title, int x, int y, int w, int h)
{
}
#endif

#ifndef IMPL_IMAGE
void dev_image(int handle, int index, int x, int y, int sx, int sy, int w, int h)
{
}

int dev_image_width(int handle, int index)
{
  return -1;
}
int dev_image_height(int handle, int index)
{
  return -1;
}
#endif

