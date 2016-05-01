// This file is part of SmallBASIC
//
// lowlevel device (OS) I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/device.h"
#include "common/smbas.h"
#include "common/messages.h"
#include "common/keymap.h"
#include "common/osd.h"
#include "common/inet.h"

#ifdef __MINGW32__
#define usleep(s) Sleep((DWORD)((s+500)/1000))
#endif

/**
 * initialize all drivers
 */
int dev_init(int mode, int flags) {
  dev_initfs();
  dev_fgcolor = 0;
  dev_bgcolor = (os_graphics) ? 15 : 0;
  osd_devinit();

  // init the keyboard map
  keymap_init();

  dev_viewport(0, 0, 0, 0);
  dev_window(0, 0, 0, 0);

  if (os_graphics) {
    // dev_fgcolor + dev_bgcolor can be overridden in osd_devinit()
    // otherwise left as default black text on white background
    osd_settextcolor(dev_fgcolor, dev_bgcolor);
    osd_setcolor(dev_fgcolor);
  } else {
    dev_fgcolor = 7;
    dev_bgcolor = 0;
  }

  return 1;
}

/**
 * restore device's mode
 */
int dev_restore() {
  if (os_graphics) {
    osd_refresh();
  }
  dev_closefs();
  if (os_graphics) {
    osd_devrestore();
  }
  net_close();
  return 1;
}

/**
 * CHECK FOR EVENTS
 *
 * wait == 0 check & return
 *    != 0 wait for an event
 *
 * returns 0 for no events in queue
 */
int dev_events(int wait_flag) {
#if !defined(DEV_EVENTS_OSD)
  if (os_graphics) {
    osd_refresh();
  }
#endif

#if DEV_EVENTS_OSD
  return osd_events(wait_flag);
#else
  if (wait_flag) {
    int evc;
    while ((evc = dev_events(0)) == 0)
 #if defined(_UnixOS)
      usleep(31250);
 #elif defined(_Win32)
      ;
 #else
      dev_delay(31);            // sleep for 1000/32 and try again
 #endif
    return evc;
  } else {
    if (os_graphics) {
      return osd_events(0);
    }
    return 0;
  }
#endif // DEV_EVENTS_OSD
}

#ifndef IMPL_DEV_DELAY

/**
 * delay for a specified amount of milliseconds
 */
void dev_delay(dword ms) {
#if defined(_Win32)
  Sleep(ms);
#elif defined(_DOS)
  delay(ms);
#else // Unix
  usleep(ms * 1000);
#endif
}
#endif

#if defined(BUILD_CONSOLE)

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
int dev_input_count_char(byte *buf, int pos) {
  int count, ch;
  byte cstr[3];

  if (os_charset != enc_utf8) {
    ch = buf[0];
    ch = ch << 8;
    ch = ch + buf[1];
    count = dev_input_char2str(ch, cstr);
  } else {
    count = 1;
  }
  return count;
}

/**
 * stores a character at 'pos' position
 */
int dev_input_insert_char(int ch, char *dest, int pos, int replace_mode) {
  byte cstr[3];
  int count, remain;
  char *buf;

  count = dev_input_char2str(ch, cstr);

  // store character into buffer
  if (replace_mode) {
    // overwrite mode
    remain = strlen((char *)(dest + pos));
    buf = malloc(remain + 1);
    strcpy(buf, dest + pos);
    memcpy(dest + pos, cstr, count);
    dest[pos + count] = '\0';

    if (os_charset != enc_utf8) {
      count = dev_input_char2str(((buf[0] << 8) | buf[1]), cstr);
    } else {
      count = 1;
    }
    if (buf[0]) {                // not a '\0'
      strcat((char *)dest, (char *)(buf + count));
    }
    free(buf);
  } else {
    // insert mode
    remain = strlen((char *)(dest + pos));
    buf = malloc(remain + 1);
    strcpy(buf, dest + pos);
    memcpy(dest + pos, cstr, count);
    dest[pos + count] = '\0';
    strcat((char *)dest, (char *)buf);
    free(buf);
  }

  return count;
}

/**
 * removes the character at 'pos' position
 */
int dev_input_remove_char(char *dest, int pos) {
  byte cstr[3];
  int count, remain;
  char *buf;

  if (dest[pos]) {
    if (os_charset != enc_utf8) {
      count = dev_input_char2str(((dest[pos] << 8) | dest[pos + 1]), cstr);
    } else {
      count = 1;
    }
    remain = strlen((char *)(dest + pos + 1));
    buf = malloc(remain + 1);
    strcpy(buf, dest + pos + count);

    dest[pos] = '\0';
    strcat((char *)dest, (char *)buf);
    free(buf);
    return count;
  }
  return 0;
}

/**
 * gets a string (INPUT)
 */
char *dev_gets(char *dest, int size) {
  long int ch = 0;
  word pos, len = 0;
  int replace_mode = 0;

  *dest = '\0';
  pos = 0;
  do {
    len = strlen(dest);
    ch = fgetc(stdin);
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
        pos -= dev_input_remove_char(dest, pos - 1);
        len = strlen(dest);
      } else {
        dev_beep();
      }
      break;
    case SB_KEY_DELETE:      // delete
      if (pos < len) {
        dev_input_remove_char(dest, pos);
        len = strlen(dest);
      } else
        dev_beep();
      break;
    case SB_KEY_INSERT:
      replace_mode = !replace_mode;
      break;
    case SB_KEY_LEFT:
      if (pos > 0) {
        pos -= dev_input_count_char((byte *)dest, pos);
      } else {
        dev_beep();
      }
      break;
    case SB_KEY_RIGHT:
      if (pos < len) {
        pos += dev_input_count_char((byte *)dest, pos);
      } else {
        dev_beep();
      }
      break;
    default:
      if ((ch & 0xFF00) != 0xFF00) { // Not an hardware key
        pos += dev_input_insert_char(ch, dest, pos, replace_mode);
      } else {
        ch = 0;
      }
      // check the size
      len = strlen(dest);
      if (len >= (size - 2)) {
        break;
      }
    }
  } while (ch != '\n' && ch != '\r');
  dest[len] = '\0';
  dev_print(dest);
  return dest;
}

#endif // #if defined BUILD_CONSOLE

/**
 * BEEP
 */
void dev_beep() {
#if !defined(BUILD_CONSOLE)
  if (!opt_mute_audio) {
    osd_beep();
  }
#endif
}

/**
 * plays a sound
 */
void dev_sound(int frq, int ms, int vol, int bgplay) {
#if !defined(BUILD_CONSOLE)
  if (!opt_mute_audio) {
    osd_sound(frq, ms, vol, bgplay);
  }
#endif
}

/**
 * clear background sound queue
 */
void dev_clear_sound_queue() {
#if !defined(BUILD_CONSOLE)
  if (!opt_mute_audio) {
    osd_clear_sound_queue();
  }
#endif
}

/**
 * printf
 *
 * WARNING: Win32/Unix ver is limited to 1024 bytes
 */
void dev_printf(const char *fmt, ...) {
  char *buf;
  va_list ap;

  va_start(ap, fmt);
  buf = malloc(1024);
#if defined(_DOS) || defined(_Win32)
  vsprintf(buf, fmt, ap);
#else
  vsnprintf(buf, 1024, fmt, ap);
#endif
  va_end(ap);

  dev_print(buf);
  free(buf);
}

/**
 * In the FLTK build, prints to the LOG window, in other builds
 * prints to the output device as per dev_printf
 */
void log_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  unsigned size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (size) {
    char *buf = malloc(size + 3);
    buf[0] = '\0';
    va_start(args, format);
    vsnprintf(buf, size + 1, format, args);
    va_end(args);

    buf[size] = '\0';
    int i = size - 1;
    while (i >= 0 && isspace(buf[i])) {
      buf[i--] = '\0';
    }
    strcat(buf, "\r\n");

#if defined(IMPL_LOG_WRITE)
    lwrite(buf);
#else
    dev_print(buf);
#endif
    free(buf);
  }
}

#if defined(BUILD_CONSOLE)
void v_create_image(var_p_t var) {}
void v_create_form(var_p_t var) {}
void v_create_window(var_p_t var) {}
void dev_show_page() {}
#endif

#if !defined(_SDL)
void dev_trace_line(int lineNo) {
  dev_printf("<%d>", lineNo);
}
#endif
