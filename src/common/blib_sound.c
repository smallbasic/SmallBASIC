// This file is part of SmallBASIC
//
// SmallBASIC RTL - SOUND
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/str.h"
#include "common/kw.h"
#include "common/var.h"
#include "common/blib.h"
#include "common/pproc.h"

static int tones[] = {
  //c   c#    d     d#    e     f     f#    g     g#    a     a#    b
  65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 116, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233,
  247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784,
  831, 880, 932, 988, 1046, 1109, 1175, 1245, 1318, 1397, 1480, 1568, 1661, 1760, 1865, 1976, 2093, 2217,
  2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951, 4186, 4435, 4699, 4978, 5274, 5587, 5919,
  6271, 6645, 7040, 7459, 7902, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

static int O = 2, bg = 0, vol = 75;
static int period, duration, pitch = 440;
static double L = 4.0, T = 240.0, M = 1.0, TM = 1.0;

#define FILE_PREFIX_LEN 7

//
// BEEP
//
void cmd_beep() {
  dev_beep();
}

//
// SOUND frq, dur [, vol] [BG]
//
void cmd_sound() {
  int frq, ms = 250, vol = 100;
  int bg = 0;

  frq = par_getint();
  if (!prog_error) {
    par_getcomma();
    if (!prog_error) {
      ms = par_getint();
      if (!prog_error) {
        if (code_peek() == kwTYPE_SEP) {
          par_getcomma();
          if (!prog_error) {
            vol = par_getint();
          }
        }
        if (code_peek() == kwBACKG) {
          code_skipnext();
          bg = 1;
        }
        if (!prog_error) {
          dev_sound(frq, ms, vol, bg);
        }
      }
    }
  }
}

void cmd_nosound() {
  dev_clear_sound_queue();
}

void cmd_play_reset() {
  O = 2;
  bg = 0;
  vol = 75;
  period = 0;
  duration = 0;
  pitch = 440;
  L = 4.0;
  T = 240.0;
  M = 1.0;
  TM = 1.0;
}

//
// PLAY str-cmds
//
#define CPLERR(c,a) { if ( (c) ) { rt_raise((a)); free(str); return; } }
void cmd_play() {
  char *p;
  var_t var;
  int n = 0;
  int calc_time_f = 1;
  char *str, *s;
  double TmpL;

  par_getstr(&var);
  if (prog_error) {
    return;
  }
  if (strncmp("file://", var.v.p.ptr, FILE_PREFIX_LEN) == 0) {
    if (dev_fexists(var.v.p.ptr)) {
      dev_audio(var.v.p.ptr + FILE_PREFIX_LEN);
    } else {
      err_file_not_found();
    }
    v_free(&var);
    return;
  }

  str = (char *) malloc(var.v.p.length + 1);

  // copy without spaces
  p = (char *) var.v.p.ptr;
  s = str;
  while (*p) {
    if (*p > 32) {
      *s++ = to_upper(*p);
    }
    p++;
  }
  *s = '\0';
  v_free(&var);

  // run
  p = str;
  while (*p) {
    if (dev_events(0) < 0) {
      break;
    }
    if (calc_time_f) {
      period = (4.0 / L) * (60000 / T) * TM;
      duration = M * period;
      calc_time_f = 0;
    }

    switch (*p) {
      // Volume
    case 'V':
      n = 0;
      while (is_digit(*(p + 1))) {
        p++;
        n = (n * 10) + (*p - '0');
      }

      CPLERR((n < 0 || n > 100), "PLAY: V0-100");
      vol = n;
      break;

      // clear queue
    case 'Q':
      dev_clear_sound_queue();
      break;

      // Octaves
    case '<':
      if (O > 0)
        O--;
      break;
    case '>':
      if (O < 6)
        O++;
      break;
    case 'O':
      O = -1;
      if (is_digit(*(p + 1))) {
        p++;
        O = *p - '0';
      }

      CPLERR((O < 0 || O > 6), "PLAY: O0-6");
      break;

      // Time
    case 'L':
      n = 0;
      while (is_digit(*(p + 1))) {
        p++;
        n = (n * 10) + (*p - '0');
      }

      CPLERR((n < 1 || n > 64), "PLAY: L1-64");

      L = n;
      calc_time_f = 1;
      break;

    case 'T':
      n = 0;
      while (is_digit(*(p + 1))) {
        p++;
        n = (n * 10) + (*p - '0');
      }

      CPLERR((n < 45 || n > 255), "PLAY: T32-255");

      T = n;
      calc_time_f = 1;
      break;

    case 'M':
      p++;
      switch (*p) {
      case 'S':
        M = 0.5;
        break;
      case 'N':
        M = 3.0 / 4.0;
        break;
      case 'L':
        M = 1.0;
        break;
      case 'F':
        bg = 0;
        break;
      case 'B':
        bg = 1;
        break;
      default:
        rt_raise("PLAY: M%c UNSUPPORTED", *p);
        v_free(&var);
      }

      calc_time_f = 1;
      break;

      // Pause
    case 'P':
      n = 0;
      while (is_digit(*(p + 1))) {
        p++;
        n = (n * 10) + (*p - '0');
      }

      if (*(p + 1) == '.') {
        p++;
        TM = 1.5;
      } else {
        TM = 1.0;
      }
      CPLERR((n < 1 || n > 64), "PLAY: P1-64");
      period = (4.0 / n) * (60000 / T) * TM;
      dev_sound(0, period, vol, bg);
      calc_time_f = 1;
      break;

      // Play N
    case 'N':
      n = 0;
      while (is_digit(*(p + 1))) {
        p++;
        n = (n * 10) + (*p - '0');
      }

      CPLERR((n < 0 || n > 84), "PLAY: N0-84");

      if (n) {
        // oct = n / 12;
        // pitch = tones[n - oct * 12] * (1 << oct);
        pitch = tones[n];
      }

      if (n == 0)
        dev_sound(0, period, vol, bg);
      else {
        dev_sound(pitch, duration, vol, bg);
        if (duration < period)
          dev_sound(0, period - duration, vol, bg);
      }

      calc_time_f = 1;
      break;

      // Play note
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
      switch (*p) {
      case 'A':
        n = 13;
        break;
      case 'B':
        n = 15;
        break;
      case 'C':
        n = 4;
        break;
      case 'D':
        n = 6;
        break;
      case 'E':
        n = 8;
        break;
      case 'F':
        n = 9;
        break;
      case 'G':
        n = 11;
        break;
      }
      if (*(p + 1) == '-' || *(p + 1) == '+' || *(p + 1) == '#') {
        p++;
        if (*p == '-')
          n--;
        else
          n++;
      }
      if (is_digit(*(p + 1))) {
        TmpL = 0;
        while (is_digit(*(p + 1))) {
          p++;
          TmpL = (TmpL * 10) + (*p - '0');
        }

        calc_time_f = 1;
      } else {
        TmpL = L;
      }
      if (*(p + 1) == '.') {
        p++;
        TM = 1.5;
      } else {
        TM = 1.0;
      }
      period = (4.0 / TmpL) * (60000 / T) * TM;
      duration = M * period;

      //                      pitch = tones[n] * (1 << O);
      pitch = tones[(n - 4) + O * 12];
      dev_sound(pitch, duration, vol, bg);
      if (duration < period) {
        dev_sound(0, period - duration, vol, bg);
      }
      break;
    default:
      rt_raise("PLAY: '%c' UNSUPPORTED", *p);
      free(str);
      return;
    }

    // next
    if (*p) {
      p++;
    }
  }

  free(str);
}
#undef CPLERR
