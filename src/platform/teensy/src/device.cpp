// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <Arduino.h>

#include <stdarg.h>
#include "config.h"
#include "common/device.h"
#include "common/smbas.h"

void dev_delay(uint32_t timeout) {
  delay(timeout);
}

uint64_t dev_get_millisecond_count() {
  return millis();
}

void dev_trace_line(int lineNo) {
  dev_printf("<%d>", lineNo);
}

int dev_events(int dead_loop) {
  yield();
  return 0;
}

const char *dev_getcwd() {
  return "/tmp";
}

int dev_init(int mode, int flags) {
  os_graf_mx = opt_pref_width;
  os_graf_my = opt_pref_height;
  setsysvar_int(SYSVAR_XMAX, os_graf_mx);
  setsysvar_int(SYSVAR_YMAX, os_graf_my);
  return 1;
}

int get_escape(const char *str, int begin, int end) {
  int result = 0;
  for (int i = begin; i < end; i++) {
    if (isdigit(str[i])) {
      result = (result * 10) + (str[i] - '0');
    } else {
      break;
    }
  }
  return result;
}

void dev_print(const char *str) {
  static int column = 0;
  int len = strlen(str);
  if (len) {
    int escape = 0;
    for (int i = 0; i < len; i++) {
      if (i + 1 < len && str[i] == '\033' && str[i + 1] == '[') {
        i += 2;
        escape = i;
      } else if (escape && str[i] == 'G') {
        // move to column
        int escValue = get_escape(str, escape, i);
        while (escValue > column) {
          Serial.print(' ');
          column++;
        }
        escape = 0;
      } else if (escape && str[i] == 'm') {
        escape = 0;
      } else if (!escape) {
        if (str[i] == '\n') {
          Serial.println();
        } else {
          Serial.print(str[i]);
        }
        column = (str[i] == '\n') ? 0 : column + 1;
      }
    }
  }
}

void dev_printf(const char *format, ...) {
  static char _buffer[1024];
  va_list args;
  va_start(args, format);
  unsigned size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (size < sizeof(_buffer)) {
    _buffer[0] = '\0';
    va_start(args, format);
    vsnprintf((char *)_buffer, size + 1, format, args);
    va_end(args);
    _buffer[size] = '\0';
    dev_print(_buffer);
  }
}

void log_printf(const char *format, ...) {
  if (opt_verbose) {
    va_list args;
    va_start(args, format);
    unsigned size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (size) {
      char *buf = (char *)malloc(size + 3);
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
      dev_print(buf);
      free(buf);
    }
  }
}

void lwrite(const char *buf) {
  if (opt_verbose) {
    Serial.println(buf);
  }
}

void panic(const char *fmt, ...) {
  Serial.println("Fatal error");
  for (;;);
}
